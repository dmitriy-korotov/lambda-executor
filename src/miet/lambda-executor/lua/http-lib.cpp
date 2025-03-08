#include <miet/lambda-executor/lua/http-lib.hpp>

#include <miet/lambda-executor/lua/utils.hpp>

#include <userver/server/http/http_request_builder.hpp>

#include <LuaCpp/LuaCpp.hpp>

namespace miet::lambda_executor::lua {
constexpr auto kMajor = "major";
constexpr auto kMinor = "minor";
constexpr auto kHttpVersionKey = "http-version";
constexpr auto kMethodKey = "method";
constexpr auto kUrlKey = "url";
constexpr auto kQueryParamsKey = "query-params";
constexpr auto kHeadersKey = "headers";
constexpr auto kBodyKey = "body";

HttpRequest::HttpRequest(
    const userver::server::http::HttpRequest& request) noexcept
    : http_version_(std::make_shared<LuaCpp::Engine::LuaTTable>()),
      method_(std::make_shared<LuaCpp::Engine::LuaTString>(
          userver::server::http::ToString(request.GetMethod()))),
      url_(std::make_shared<LuaCpp::Engine::LuaTString>(request.GetUrl())),
      query_params_(std::make_shared<LuaCpp::Engine::LuaTTable>()),
      headers_(std::make_shared<LuaCpp::Engine::LuaTTable>()),
      body_(
          std::make_shared<LuaCpp::Engine::LuaTString>(request.RequestBody())) {
  http_version_->setValue(
      LuaCpp::Engine::Table::Key(kMajor),
      std::make_shared<LuaCpp::Engine::LuaTNumber>(request.GetHttpMajor()));
  http_version_->setValue(
      LuaCpp::Engine::Table::Key(kMinor),
      std::make_shared<LuaCpp::Engine::LuaTNumber>(request.GetHttpMinor()));
  for (const auto& name : request.ArgNames()) {
    query_params_->setValue(
        LuaCpp::Engine::Table::Key(name),
        std::make_shared<LuaCpp::Engine::LuaTString>(request.GetArg(name)));
  }
  for (const auto& [header, value] : request.GetHeaders()) {
    headers_->setValue(LuaCpp::Engine::Table::Key(header),
                       std::make_shared<LuaCpp::Engine::LuaTString>(value));
  }
}

std::shared_ptr<userver::server::http::HttpRequest> HttpRequest::Build() const {
  userver::server::http::HttpRequestBuilder builder;
  const auto& majorLua =
      http_version_->getValue(LuaCpp::Engine::Table::Key(kMajor));
  const auto& minorLua =
      http_version_->getValue(LuaCpp::Engine::Table::Key(kMinor));
  builder.SetHttpMajor(
      utils::GetValueStrict<const LuaCpp::Engine::LuaTNumber>(&majorLua)
          ->getValue());
  builder.SetHttpMinor(
      utils::GetValueStrict<const LuaCpp::Engine::LuaTNumber>(&minorLua)
          ->getValue());
  builder.SetMethod(
      userver::server::http::HttpMethodFromString(method_->getValue()));
  builder.SetUrl(url_->getValue());
  for (const auto& [key, value] : query_params_->getValues()) {
    builder.AddRequestArg(key.getStringValue(), value->ToString());
  }
  for (const auto& [header, value] : headers_->getValues()) {
    builder.AddHeader(header.getStringValue(), value->ToString());
  }
  builder.SetBody(body_->getValue());
  return builder.Build();
}

std::shared_ptr<HttpRequest::LuaType> HttpRequest::getValue(std::string& key) {
  if (key == kHttpVersionKey) {
    return http_version_;
  } else if (key == kHttpVersionKey) {
    return method_;
  } else if (key == kMethodKey) {
    return method_;
  } else if (key == kUrlKey) {
    return url_;
  } else if (key == kQueryParamsKey) {
    return query_params_;
  } else if (key == kHeadersKey) {
    return headers_;
  } else if (key == kBodyKey) {
    return body_;
  }
  return std::make_shared<LuaCpp::Engine::LuaTNil>();
}

void HttpRequest::setValue(std::string& key, std::shared_ptr<LuaType> value) {
  if (key == kHttpVersionKey) {
    http_version_ = utils::GetValueStrict<LuaCpp::Engine::LuaTTable>(value);
  } else if (key == kMethodKey) {
    method_->setValue(value->ToString());
  } else if (key == kUrlKey) {
    url_->setValue(value->ToString());
  } else if (key == kQueryParamsKey) {
    query_params_ = utils::GetValueStrict<LuaCpp::Engine::LuaTTable>(value);
  } else if (key == kHeadersKey) {
    headers_ = utils::GetValueStrict<LuaCpp::Engine::LuaTTable>(value);
  } else if (key == kBodyKey) {
    body_->setValue(value->ToString());
  } else {
    throw std::runtime_error(
        fmt::format("Invalid request param (key = '{}')", key));
  }
}
}  // namespace miet::lambda_executor::lua
