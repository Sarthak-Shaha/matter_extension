/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#include "cluster_emulator.hpp"
#include "cluster_revision_table.hpp"
#include "sl_log.h"
#include <attribute_state_cache.hpp>

#include "emulate_groups.hpp"
#include "emulate_identify.hpp"
#include "emulate_level.hpp"
#include "emulate_doorlock.hpp"
#include <app/clusters/identify-server/identify-server.h>

#define LOG_TAG "cluster_emulator"

#include <app-common/zap-generated/callback.h>

using namespace chip::app;

/**
 * @brief Use default values for external attribute storage, this functions overrides a _weak_ symbol on the ember framework.
 *
 * All attributes which has meta data with ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) and is not overridden by an attribute access
 * interface, will be read though this function. Right now this only applies to the attriutes in the Group cluster
 *
 * @param endpoint
 * @param clusterId
 * @param attributeMetadata
 * @param buffer
 * @param maxReadLength
 * @return CHIP_ERROR
 */
CHIP_ERROR emberAfExternalAttributeReadCallback(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                   const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer,
                                                   uint16_t maxReadLength)
{
    sl_log_debug(LOG_TAG, "emberAfExternalAttributeReadCallback: endpoint: %d cluster: %d attribute: %d\n", endpoint, clusterId,
                 attributeMetadata->attributeId);
    memcpy(buffer, &attributeMetadata->defaultValue, attributeMetadata->size);
    return CHIP_NO_ERROR;
}

void OnIdentifyStart(::Identify *)
{
    sl_log_debug(LOG_TAG, "Identify started\n");
}

void OnIdentifyStop(::Identify *)
{
    sl_log_debug(LOG_TAG, "Identify stopped\n");
}

void OnTriggerEffect(::Identify * identify)
{
    switch (identify->mCurrentEffectIdentifier)
    {
    case Clusters::Identify::EffectIdentifierEnum::kBlink:
        sl_log_debug(LOG_TAG, "Blink\n");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kBreathe:
        sl_log_debug(LOG_TAG, "Breathe\n");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kOkay:
        sl_log_debug(LOG_TAG, "Okay\n");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kChannelChange:
        sl_log_debug(LOG_TAG, "Channel Change\n");
        break;
    default:
        sl_log_debug(LOG_TAG, "No identifier effect\n");
        return;
    }
}

static Identify gIdentify1 = {
    chip::EndpointId{ 1 }, OnIdentifyStart, OnIdentifyStop, Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator, OnTriggerEffect,
};

namespace unify::matter_bridge {

#define ON_OFF_LIGHTING_FEATURE_MAP_MASK 0x01
#define LEVEL_ONOFF_DEPENDENCY_FEATURE_MAP_MASK 0x01
#define LEVEL_LIGHTING_FEATURE_MAP_MASK 0x02
#define DOORLOCK_FEATURE_MAP_MASK 0x00

#define THERMOSTAT_HEATING_FEATURE_MAP 0x01
#define THERMOSTAT_COOLING_FEATURE_MAP 0x02

using namespace chip::app;
using namespace chip::app::Clusters;

ClusterEmulator::ClusterEmulator()
{
    std::vector<std::shared_ptr<EmulatorInterface>> emulators = { std::make_shared<EmulateIdentify>(),
                                                                  std::make_shared<EmulateLevelControl>(),
                                                                  std::make_shared<EmulateDoorLock>(),
                                                                  std::make_shared<EmulateGroups>() };
    for (auto e : emulators)
    {
        cluster_emulators_string_map.insert(std::make_pair(e->emulated_cluster_name(), e));
        for (auto a : e->emulated_commands())
        {
            auto key = std::make_pair(e->emulated_cluster(), a);
            cluster_emulators_command_id_map.insert(std::make_pair(key, e));
            ;
        }
        for (auto a : e->emulated_attributes())
        {
            auto key = std::make_pair(e->emulated_cluster(), a);
            cluster_emulators_attribute_id_map.insert(std::make_pair(key, e));
            ;
        }
    }
};

uint32_t ClusterEmulator::read_cluster_revision(const ConcreteReadAttributePath & aPath) const
{
    sl_log_debug(LOG_TAG, "Reading Cluster Revision for cluster %d", aPath.mClusterId);
    switch (aPath.mClusterId)
    {
    case TemperatureMeasurement::Id: {
        return 4;
    }
    }
    if (zap_cluster_revisions.count(aPath.mClusterId))
    {
        return zap_cluster_revisions.at(aPath.mClusterId);
    }
    else
    {
        return 0;
    }
}

Clusters::Globals::Attributes::EventList::TypeInfo::Type
    ClusterEmulator::read_event_list(const ConcreteReadAttributePath & aPath) const
{
    sl_log_debug(LOG_TAG, "Reading Event list for cluster %d", aPath.mClusterId);
    Clusters::Globals::Attributes::EventList::TypeInfo::Type eventList;
    switch (aPath.mClusterId)
    {
    case DoorLock::Id: {
        chip::EventId events[] = { DoorLock::Events::DoorLockAlarm::Id,
                                   DoorLock::Events::LockOperation::Id };
        eventList = events;
    }
    }
    return eventList;
}

uint32_t ClusterEmulator::read_feature_map_revision(const ConcreteReadAttributePath & aPath) const
{
    attribute_state_cache & cache = attribute_state_cache::get_instance();

    sl_log_debug(LOG_TAG, "Reading feature map for cluster %d", aPath.mClusterId);
    switch (aPath.mClusterId)
    {
    case ColorControl::Id: {
        ColorControl::Attributes::ColorCapabilities::TypeInfo::Type colorControlCapabilities;
        if (cache.get<ColorControl::Attributes::ColorCapabilities::TypeInfo::Type>(aPath, colorControlCapabilities))
        {
            return static_cast<uint32_t>(colorControlCapabilities);
        }
        else
        {
            sl_log_warning(LOG_TAG, "Failed to read ColorCapabilities, setting feature map to HueSaturationSupported");
            return 1;
        }
    }
    break;
    case OnOff::Id:
        if (emberAfFindServerCluster(aPath.mEndpointId, LevelControl::Id))
        {
            return ON_OFF_LIGHTING_FEATURE_MAP_MASK;
        }
        break;
    case LevelControl::Id:
        /// Check if OnOff is supported
        if (emberAfFindServerCluster(aPath.mEndpointId, OnOff::Id))
        {
            return (LEVEL_ONOFF_DEPENDENCY_FEATURE_MAP_MASK | LEVEL_LIGHTING_FEATURE_MAP_MASK);
        }
        break;
    case Thermostat::Id: {
        uint32_t result = 0;
        ConcreteAttributePath cooling_atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, 
                                    Thermostat::Attributes::OccupiedCoolingSetpoint::Id);

        ConcreteAttributePath heating_atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, 
                                    Thermostat::Attributes::OccupiedHeatingSetpoint::Id);

        Thermostat::Attributes::OccupiedCoolingSetpoint::TypeInfo::Type coolingCapabilities;
        Thermostat::Attributes::OccupiedHeatingSetpoint::TypeInfo::Type heatingCapabilities;

        auto coolingSupport = cache.get<Thermostat::Attributes::OccupiedCoolingSetpoint::TypeInfo::Type>(cooling_atr_path,
            coolingCapabilities);
        auto heatingSupport = cache.get<Thermostat::Attributes::OccupiedHeatingSetpoint::TypeInfo::Type>(heating_atr_path,
            heatingCapabilities);
        if (coolingSupport)
        {
            result |= THERMOSTAT_COOLING_FEATURE_MAP;
        }
        if (heatingSupport)
        {
            result |= THERMOSTAT_HEATING_FEATURE_MAP;
        }

        sl_log_debug(LOG_TAG, "FeatureMap Cache check: Cooling - %x", coolingSupport );
        sl_log_debug(LOG_TAG, "FeatureMap Cache check: Heating - %x", heatingSupport );
        return result;
    }
    case DoorLock::Id:
        if (emberAfFindServerCluster(aPath.mEndpointId, DoorLock::Id))
        {
            return DOORLOCK_FEATURE_MAP_MASK;
        }
        break;
    }
    return 0;
}

void ClusterEmulator::add_emulated_commands_and_attributes(const node_state_monitor::cluster & unify_cluster,
                                                           matter_cluster_builder & cluster_builder)
{
    // We always need to add the feature map and cluster
    cluster_builder.attributes.emplace_back(EmberAfAttributeMetadata{ ZAP_EMPTY_DEFAULT(),
                                                                      chip::app::Clusters::Globals::Attributes::FeatureMap::Id, 4,
                                                                      ZAP_TYPE(BITMAP32), ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) });
    cluster_builder.attributes.emplace_back(EmberAfAttributeMetadata{ ZAP_EMPTY_DEFAULT(),
                                                                      chip::app::Clusters::Globals::Attributes::ClusterRevision::Id,
                                                                      2, ZAP_TYPE(INT16U), ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) });

    cluster_builder.attributes.emplace_back(EmberAfAttributeMetadata{ ZAP_EMPTY_DEFAULT(),
                                                                      chip::app::Clusters::Globals::Attributes::EventList::Id,
                                                                      4, ZAP_TYPE(EVENT_ID), ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) });

    // Add emulation for commands and attributes for the cluster
    auto it = cluster_emulators_string_map.find(unify_cluster.cluster_name);
    if (it != cluster_emulators_string_map.end())
    {
        sl_log_debug(LOG_TAG, "%s emulator is updating the descriptor ", it->second->emulated_cluster_name());

        auto emulated_result = it->second->emulate(unify_cluster, cluster_builder);

        if (emulated_result != CHIP_NO_ERROR)
        {
            sl_log_error(LOG_TAG, "Failed to add emulated commands and attributes for cluster %s",
                         unify_cluster.cluster_name.c_str());
        }
    }
}

bool ClusterEmulator::is_command_emulated(const ConcreteCommandPath & cPath) const
{
    auto it = cluster_emulators_command_id_map.find(std::make_pair(cPath.mClusterId, cPath.mCommandId));
    return (it != cluster_emulators_command_id_map.end());
}

bool ClusterEmulator::is_attribute_emulated(const ConcreteAttributePath & aPath) const
{
    // Global attributes per Core Spec chapter 7.13
    switch (aPath.mAttributeId)
    {
    case chip::app::Clusters::Globals::Attributes::ClusterRevision::Id: // Cluster Revision
    case chip::app::Clusters::Globals::Attributes::FeatureMap::Id: // FeatureMap
    case chip::app::Clusters::Globals::Attributes::EventList::Id: // EventList
        return true;
    default:;
        ;
    }

    auto it = cluster_emulators_attribute_id_map.find(std::make_pair(aPath.mClusterId, aPath.mAttributeId));
    return (it != cluster_emulators_attribute_id_map.end());
}

CHIP_ERROR ClusterEmulator::invoke_command(CommandHandlerInterface::HandlerContext & handlerContext, emulated_cmd_payload & cdata) const
{
    if (is_command_emulated(handlerContext.mRequestPath))
    {
        auto e = cluster_emulators_command_id_map.at(
            std::make_pair(handlerContext.mRequestPath.mClusterId, handlerContext.mRequestPath.mCommandId));

        sl_log_debug(LOG_TAG, "%s emualtor is emulating command 0x%04x ", e->emulated_cluster_name(),
                     handlerContext.mRequestPath.mCommandId);
        return e->command(handlerContext, cdata);
    }

    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ClusterEmulator::read_attribute(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    sl_log_debug(LOG_TAG, "Reading attribute %d", aPath.mAttributeId);
    switch (aPath.mAttributeId)
    {
    case chip::app::Clusters::Globals::Attributes::ClusterRevision::Id: // Cluster Revision
        return aEncoder.Encode(this->read_cluster_revision(aPath));
    case chip::app::Clusters::Globals::Attributes::FeatureMap::Id: // FeatureMap
        return aEncoder.Encode(this->read_feature_map_revision(aPath));
    case chip::app::Clusters::Globals::Attributes::EventList::Id: // EventList
        return aEncoder.Encode(this->read_event_list(aPath));
    default:;
        ;
    }

    sl_log_debug(LOG_TAG, "Cluster specific attribute emulation attribute id %d for cluster id", aPath.mAttributeId,
                 aPath.mClusterId);
    if (is_attribute_emulated(aPath))
    {
        auto e = cluster_emulators_attribute_id_map.at(std::make_pair(aPath.mClusterId, aPath.mAttributeId));
        sl_log_debug(LOG_TAG, "%s emualtor is emulating attribute 0x%04x", e->emulated_cluster_name(), aPath.mAttributeId);
        return e->read_attribute(aPath, aEncoder);
    }

    return CHIP_ERROR_INVALID_ARGUMENT;
}

CHIP_ERROR ClusterEmulator::write_attribute(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) const
{
    if (is_attribute_emulated(aPath))
    {
        return cluster_emulators_attribute_id_map.at(std::make_pair(aPath.mClusterId, aPath.mAttributeId))
            ->write_attribute(aPath, aDecoder);
    }

    return CHIP_ERROR_NOT_IMPLEMENTED;
}
} // namespace unify::matter_bridge
