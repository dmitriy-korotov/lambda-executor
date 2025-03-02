/**
 * @copyright Copyright 2025 MIET-Lambda.
 *            All rights reserved.
 *            Licensed under the Apache-2.0 License and CLA.
 */

#pragma once

#include <userver/components/component_list.hpp>

namespace miet::lambda_executor::handlers {

void AppendExecuteLambda(userver::components::ComponentList& component_list);

}  // namespace miet::lambda_executor::handlers
