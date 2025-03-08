#pragma once

#include <userver/server/http/http_request.hpp>

#include <LuaCpp/LuaCpp.hpp>

namespace miet::lambda_executor::lua {
class HttpRequest final : public LuaCpp::LuaMetaObject {
 public:
  explicit HttpRequest(
      const userver::server::http::HttpRequest& request) noexcept;

  std::shared_ptr<userver::server::http::HttpRequest> Build() const;

  std::shared_ptr<LuaType> getValue(std::string& key) override;
  void setValue(std::string& key, std::shared_ptr<LuaType> value) override;

 private:
  std::shared_ptr<LuaCpp::Engine::LuaTTable> http_version_;
  std::shared_ptr<LuaCpp::Engine::LuaTString> method_;
  std::shared_ptr<LuaCpp::Engine::LuaTString> url_;
  std::shared_ptr<LuaCpp::Engine::LuaTTable> query_params_;
  std::shared_ptr<LuaCpp::Engine::LuaTTable> headers_;
  std::shared_ptr<LuaCpp::Engine::LuaTString> body_;
};
}  // namespace miet::lambda_executor::lua
