#pragma once

#include <userver/components/component_list.hpp>

namespace miet::lambda_executor::handlers {

void AppendExecuteLambda(userver::components::ComponentList& component_list);

}  // namespace miet::lambda_executor::handlers
