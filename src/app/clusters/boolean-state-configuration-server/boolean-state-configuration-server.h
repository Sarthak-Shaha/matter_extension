/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 * @file API declarations for boolean state configuration cluster.
 */

#pragma once

#include "boolean-state-configuration-delegate.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/basic-types.h>
#include <lib/core/DataModelTypes.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace BooleanStateConfiguration {

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate);
Delegate * GetDefaultDelegate(EndpointId endpoint);

CHIP_ERROR SetAlarmsActive(EndpointId ep, chip::BitMask<AlarmModeBitmap> alarms);
CHIP_ERROR SetAllEnabledAlarmsActive(EndpointId ep);
CHIP_ERROR ClearAllAlarms(EndpointId ep);
CHIP_ERROR SuppressAlarms(EndpointId ep, chip::BitMask<BooleanStateConfiguration::AlarmModeBitmap> alarms);
CHIP_ERROR SetCurrentSensitivityLevel(EndpointId ep, uint8_t level);
CHIP_ERROR EmitSensorFault(EndpointId ep, chip::BitMask<BooleanStateConfiguration::SensorFaultBitmap> fault);

inline bool HasFeature(EndpointId ep, Feature feature)
{
    uint32_t map;
    bool success = (Attributes::FeatureMap::Get(ep, &map) == Protocols::InteractionModel::Status::Success);
    return success ? (map & to_underlying(feature)) : false;
}

} // namespace BooleanStateConfiguration
} // namespace Clusters
} // namespace app
} // namespace chip
