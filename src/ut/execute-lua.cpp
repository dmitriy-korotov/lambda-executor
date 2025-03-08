#include <miet/lambda-executor/lua/http-lib.hpp>

#include <userver/formats/json/serialize.hpp>
#include <userver/server/http/http_request_builder.hpp>

#include <userver/utest/utest.hpp>

#include <LuaCpp/LuaCpp.hpp>

class TestLuaCpp : public testing::Test {
 protected:
  LuaCpp::LuaContext ctx;

  static int Sum(lua_State* L) noexcept {
    auto n = lua_gettop(L);
    lua_Number sum = 0.0;
    for (int i = 1; i <= n; ++i) {
      if (!lua_isnumber(L, i)) {
        lua_pushliteral(L, "Incorrect argument");
        lua_error(L);
      }
      sum += lua_tonumber(L, i);
    }
    lua_pushnumber(L, sum);
    return 1;
  }
};

UTEST_F(TestLuaCpp, Print) {
  ASSERT_NO_THROW(ctx.CompileString("print", "print('Hello from lua!')"));
  ASSERT_NO_THROW(ctx.Run("print"));
}

UTEST_F(TestLuaCpp, GlobalVariable) {
  const auto sumVar = std::make_shared<LuaCpp::Engine::LuaTNumber>(0);
  const auto aVar = std::make_shared<LuaCpp::Engine::LuaTNumber>(5);
  ctx.AddGlobalVariable("sum", sumVar);
  ctx.AddGlobalVariable("a", aVar);
  ASSERT_NO_THROW(ctx.CompileString("global-variable", "sum = a + 10"));
  ASSERT_NO_THROW(ctx.Run("global-variable"));
  ASSERT_EQ(sumVar->getValue(), 15.0);
}

UTEST_F(TestLuaCpp, Lib) {
  auto lib = std::make_shared<LuaCpp::Registry::LuaLibrary>("mylib");
  lib->AddCFunction("sum", Sum);

  ctx.AddLibrary(lib);

  ASSERT_NO_THROW(ctx.CompileString(
      "lib", "print('Result: ' .. mylib.sum(1, 2, 3, 4, 5))"));
  ASSERT_NO_THROW(ctx.Run("lib"));

  ASSERT_NO_THROW(ctx.CompileString(
      "lib-fail", "print('Result: ' .. mylib.sum(1, 'a', 2))"));
  ASSERT_THROW(ctx.Run("lib-fail"), std::runtime_error);
}

UTEST_F(TestLuaCpp, JsonLib) {
  ASSERT_NO_THROW(ctx.CompileString("json", R"(
    local json = require("dkjson")

    local data = {
        name = "Lua",
        version = "5.4",
        features = {"lightweight", "embeddable", "fast"}
    }

    local json_text = json.encode(data, { indent = true })
    print("Encoded JSON:")
    print(json_text)

    local decoded_data, pos, err = json.decode(json_text, 1, nil)
    if err then
        print("Error:", err)
    else
        print("\nDecoded Data:")
        for key, value in pairs(decoded_data) do
            print(key, value)
        end
    end
  )"));
  ASSERT_NO_THROW(ctx.Run("json"));
}

UTEST_F(TestLuaCpp, ReadHttpRequest) {
  userver::server::http::HttpRequestBuilder builder;
  builder.SetHttpMajor(2);
  builder.SetHttpMinor(0);
  builder.SetMethod(userver::server::http::HttpMethod::kPatch);
  builder.SetUrl("/miet/lambda-executor/lua");
  builder.AddRequestArg("key", "value");
  builder.AddRequestArg("format", "json");
  builder.AddHeader("Content-Type", "application/json");
  builder.AddHeader("X-My-Header", "value");
  builder.SetBody("body");

  const auto request = builder.Build();

  const auto requestVar =
      std::make_shared<miet::lambda_executor::lua::HttpRequest>(*request);
  const auto errorVar = std::make_shared<LuaCpp::Engine::LuaTString>("");
  ctx.AddGlobalVariable("request", requestVar);
  ctx.AddGlobalVariable("error", errorVar);
  ASSERT_NO_THROW(ctx.CompileString("read-http-request", R"(
    local http_version = request['http-version']
    if http_version == nil or http_version['major'] ~= 2 or http_version['minor'] ~= 0 then
      error = 'Incorrect http version'
      return
    end

    local method = request['method']
    if method == nil or method ~= 'PATCH' then
      error = 'Incorrect method'
      return
    end

    local url = request['url']
    if url == nil or url ~= '/miet/lambda-executor/lua' then
      error = 'Incorrect url'
      return
    end

    local query_params = request['query-params']
    if query_params == nil or query_params['key'] ~= 'value' or query_params['format'] ~= 'json' then
      error = 'Incorrect query parameters: ' .. (query_params['key'] or '') .. ', ' .. (query_params['format'] or '')
      return
    end

    local headers = request['headers']
    if headers == nil or headers['Content-Type'] ~= 'application/json' or headers['X-My-Header'] ~= 'value' then
      error = 'Incorrect headers: ' .. (headers['Content-Type'] or '') .. ', ' .. (headers['X-My-Header'] or '')
      return
    end

    local body = request['body']
    if body == nil or body ~= 'body' then
      error = 'Incorrect body'
      return
    end
  )"));
  ASSERT_NO_THROW(ctx.Run("read-http-request"));
  ASSERT_EQ(errorVar->getValue(), "");
}

UTEST_F(TestLuaCpp, BuildHttpRequest) {
  userver::server::http::HttpRequestBuilder builder;
  auto request = builder.Build();
  const auto requestVar =
      std::make_shared<miet::lambda_executor::lua::HttpRequest>(*request);
  ctx.AddGlobalVariable("request", requestVar);
  ASSERT_NO_THROW(ctx.CompileString("build-http-request", R"(
    request['http-version'] = {
      major = 2,
      minor = 0
    }
    request['method'] = 'POST'
    request['url'] = '/v1/lambda-executor/build-http-request'
    request['query-params'] = {
      type = 'bool',
      format = 'json'
    }
    request['headers'] = {
      ['Content-Type'] = 'plain/text',
      ['X-My-Header'] = 'some-value'
    }
    request['body'] = {
      name = 'Alex',
      age = 18
    }
  )"));
  ASSERT_NO_THROW(ctx.Run("build-http-request"));
  request = requestVar->Build();
  ASSERT_EQ(request->GetHttpMajor(), 2);
  ASSERT_EQ(request->GetHttpMinor(), 0);

  ASSERT_EQ(request->GetMethod(), userver::server::http::HttpMethod::kPost);
  ASSERT_EQ(request->GetUrl(), "/v1/lambda-executor/build-http-request");

  ASSERT_TRUE(request->HasArg("type"));
  ASSERT_TRUE(request->HasArg("format"));
  ASSERT_EQ(request->GetArg("type"), "bool");
  ASSERT_EQ(request->GetArg("format"), "json");

  ASSERT_TRUE(request->HasHeader("Content-Type"));
  ASSERT_TRUE(request->HasHeader("X-My-Header"));
  ASSERT_EQ(request->GetHeader("Content-Type"), "plain/text");
  ASSERT_EQ(request->GetHeader("X-My-Header"), "some-value");

  const auto jsonBody =
      userver::formats::json::FromString(request->RequestBody());
  ASSERT_TRUE(jsonBody.IsObject());
  ASSERT_EQ(jsonBody["name"].As<std::string>(), "Alex");
  ASSERT_EQ(jsonBody["age"].As<std::uint8_t>(), 18);
}
