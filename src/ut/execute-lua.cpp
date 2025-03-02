/**
 * @copyright Copyright 2025 MIET-Lambda.
 *            All rights reserved.
 *            Licensed under the Apache-2.0 License and CLA.
 */

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
  ASSERT_NO_THROW(ctx.CompileString("test-print", "print('Hello from lua!')"));
  ASSERT_NO_THROW(ctx.Run("test-print"));
}

UTEST_F(TestLuaCpp, GlobalVariable) {
  const auto sumVar = std::make_shared<LuaCpp::Engine::LuaTNumber>(0);
  const auto aVar = std::make_shared<LuaCpp::Engine::LuaTNumber>(5);
  ctx.AddGlobalVariable("sum", sumVar);
  ctx.AddGlobalVariable("a", aVar);
  ASSERT_NO_THROW(ctx.CompileString("test-return", "sum = a + 10"));
  ASSERT_NO_THROW(ctx.Run("test-return"));
  ASSERT_EQ(sumVar->getValue(), 15.0);
}

UTEST_F(TestLuaCpp, Lib) {
  auto lib = std::make_shared<LuaCpp::Registry::LuaLibrary>("mylib");
  lib->AddCFunction("sum", Sum);

  ctx.AddLibrary(lib);

  ASSERT_NO_THROW(ctx.CompileString(
      "test-lib", "print('Result: ' .. mylib.sum(1, 2, 3, 4, 5))"));
  ASSERT_NO_THROW(ctx.Run("test-lib"));

  ASSERT_NO_THROW(ctx.CompileString(
      "test-lib-fail", "print('Result: ' .. mylib.sum(1, 'a', 2))"));
  ASSERT_THROW(ctx.Run("test-lib-fail"), std::runtime_error);
}
