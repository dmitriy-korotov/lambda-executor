#include <userver/clients/dns/component.hpp>
#include <userver/clients/http/client.hpp>
#include <userver/clients/http/component.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/server/handlers/tests_control.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/utils/daemon_run.hpp>

#include <miet/lambda-executor/handlers/execute-lambda.hpp>

int main(int argc, char* argv[]) {
  auto component_list = userver::components::MinimalServerComponentList()
                            .Append<userver::server::handlers::Ping>()
                            .Append<userver::server::handlers::TestsControl>()
                            .Append<userver::clients::dns::Component>()
                            .Append<userver::components::HttpClient>()
                            .Append<userver::components::TestsuiteSupport>();

  miet::lambda_executor::handlers::AppendExecuteLambda(component_list);

  return userver::utils::DaemonMain(argc, argv, component_list);
}
