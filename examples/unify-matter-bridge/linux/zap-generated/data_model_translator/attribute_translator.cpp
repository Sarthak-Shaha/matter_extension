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

#define CHIP_USE_ENUM_CLASS_FOR_IM_ENUM

#include "matter.h"
#include <nlohmann/json.hpp>
#include <regex>
#include <sstream>
#include <type_traits>

#include "attribute_translator.hpp"
#include "cluster_emulator.hpp"
#include "matter_device_translator.hpp"
#include <attribute_state_cache.hpp>

#include "app-common/zap-generated/attributes/Accessors.h"
#include "sl_log.h"
#include "uic_mqtt.h"
#define LOG_TAG "attribute_translator"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace unify::matter_bridge;

#include "chip_types_from_json.hpp"
#include "chip_types_to_json.hpp"

CHIP_ERROR
IdentifyAttributeAccess::Read(const ConcreteReadAttributePath& aPath, AttributeValueEncoder& aEncoder)
{
    namespace MN = chip::app::Clusters::Identify::Attributes;
    namespace UN = unify::matter_bridge::Identify::Attributes;
    if (aPath.mClusterId != Clusters::Identify::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    // Do not handle Read for non-unify endpoints
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().read_attribute(aPath, aEncoder);
    }

    try {
        switch (aPath.mAttributeId) {
        case MN::IdentifyTime::Id: { // type is int16u
            MN::IdentifyTime::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::IdentifyType::Id: { // type is IdentifyTypeEnum
            MN::IdentifyType::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        }
    } catch (const std::out_of_range& e) {
        sl_log_info(LOG_TAG,
            "The request attribute Path for endpoint [%i] is not found in the attribute state "
            "container: %s\n",
            atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR IdentifyAttributeAccess::Write(const ConcreteDataAttributePath& aPath, AttributeValueDecoder& aDecoder)
{
    using namespace chip::app::Clusters::Identify;

    if (aPath.mClusterId != Clusters::Identify::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }
    nlohmann::json jsn;

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().write_attribute(aPath, aDecoder);
    }

    switch (aPath.mAttributeId) {
    case Attributes::IdentifyTime::Id: {

        Attributes::IdentifyTime::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["IdentifyTime"] = to_json(value);
        break;
    }
        // IdentifyType is not supported by UCL
        // GeneratedCommandList is not supported by UCL
        // AcceptedCommandList is not supported by UCL
        // EventList is not supported by UCL
        // AttributeList is not supported by UCL
        // FeatureMap is not supported by UCL
        // ClusterRevision is not supported by UCL
    }

    if (!jsn.empty()) {
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) + "/Identify/Commands/WriteAttributes";
        std::string payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NO_MESSAGE_HANDLER;
}

void IdentifyAttributeAccess::reported_updated(const bridged_endpoint* ep, const std::string& cluster,
    const std::string& attribute, const nlohmann::json& unify_value)
{
    namespace MN = chip::app::Clusters::Identify::Attributes;
    namespace UN = unify::matter_bridge::Identify::Attributes;

    auto cluster_id = m_dev_translator.get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::Identify::Id)) {
        return;
    }

    // get attribute id
    auto attribute_id = m_dev_translator.get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value()) {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::Identify::Id, attribute_id.value());
    switch (attribute_id.value()) {
    // type is int16u
    case MN::IdentifyTime::Id: {
        using T = MN::IdentifyTime::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "IdentifyTime attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Identify::Id, MN::IdentifyTime::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
GroupsAttributeAccess::Read(const ConcreteReadAttributePath& aPath, AttributeValueEncoder& aEncoder)
{
    namespace MN = chip::app::Clusters::Groups::Attributes;
    namespace UN = unify::matter_bridge::Groups::Attributes;
    if (aPath.mClusterId != Clusters::Groups::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    // Do not handle Read for non-unify endpoints
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().read_attribute(aPath, aEncoder);
    }

    try {
        switch (aPath.mAttributeId) {
        case MN::NameSupport::Id: { // type is NameSupportBitmap
            MN::NameSupport::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        }
    } catch (const std::out_of_range& e) {
        sl_log_info(LOG_TAG,
            "The request attribute Path for endpoint [%i] is not found in the attribute state "
            "container: %s\n",
            atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupsAttributeAccess::Write(const ConcreteDataAttributePath& aPath, AttributeValueDecoder& aDecoder)
{
    using namespace chip::app::Clusters::Groups;

    if (aPath.mClusterId != Clusters::Groups::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }
    nlohmann::json jsn;

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().write_attribute(aPath, aDecoder);
    }

    switch (aPath.mAttributeId) {
        // NameSupport is not supported by UCL
        // GeneratedCommandList is not supported by UCL
        // AcceptedCommandList is not supported by UCL
        // EventList is not supported by UCL
        // AttributeList is not supported by UCL
        // FeatureMap is not supported by UCL
        // ClusterRevision is not supported by UCL
    }

    if (!jsn.empty()) {
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) + "/Groups/Commands/WriteAttributes";
        std::string payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NO_MESSAGE_HANDLER;
}

void GroupsAttributeAccess::reported_updated(const bridged_endpoint* ep, const std::string& cluster,
    const std::string& attribute, const nlohmann::json& unify_value)
{
    namespace MN = chip::app::Clusters::Groups::Attributes;
    namespace UN = unify::matter_bridge::Groups::Attributes;

    auto cluster_id = m_dev_translator.get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::Groups::Id)) {
        return;
    }

    // get attribute id
    auto attribute_id = m_dev_translator.get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value()) {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::Groups::Id, attribute_id.value());
    switch (attribute_id.value()) {
    // type is NameSupportBitmap
    case MN::NameSupport::Id: {
        using T = MN::NameSupport::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "NameSupport attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Groups::Id, MN::NameSupport::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
OnOffAttributeAccess::Read(const ConcreteReadAttributePath& aPath, AttributeValueEncoder& aEncoder)
{
    namespace MN = chip::app::Clusters::OnOff::Attributes;
    namespace UN = unify::matter_bridge::OnOff::Attributes;
    if (aPath.mClusterId != Clusters::OnOff::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    // Do not handle Read for non-unify endpoints
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().read_attribute(aPath, aEncoder);
    }

    try {
        switch (aPath.mAttributeId) {
        case MN::OnOff::Id: { // type is boolean
            MN::OnOff::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::GlobalSceneControl::Id: { // type is boolean
            MN::GlobalSceneControl::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::OnTime::Id: { // type is int16u
            MN::OnTime::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::OffWaitTime::Id: { // type is int16u
            MN::OffWaitTime::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::StartUpOnOff::Id: { // type is StartUpOnOffEnum
            MN::StartUpOnOff::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        }
    } catch (const std::out_of_range& e) {
        sl_log_info(LOG_TAG,
            "The request attribute Path for endpoint [%i] is not found in the attribute state "
            "container: %s\n",
            atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR OnOffAttributeAccess::Write(const ConcreteDataAttributePath& aPath, AttributeValueDecoder& aDecoder)
{
    using namespace chip::app::Clusters::OnOff;

    if (aPath.mClusterId != Clusters::OnOff::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }
    nlohmann::json jsn;

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().write_attribute(aPath, aDecoder);
    }

    switch (aPath.mAttributeId) {
    // OnOff is not supported by UCL
    // GlobalSceneControl is not supported by UCL
    case Attributes::OnTime::Id: {

        Attributes::OnTime::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["OnTime"] = to_json(value);
        break;
    }
    case Attributes::OffWaitTime::Id: {

        Attributes::OffWaitTime::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["OffWaitTime"] = to_json(value);
        break;
    }
    case Attributes::StartUpOnOff::Id: {

        Attributes::StartUpOnOff::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["StartUpOnOff"] = to_json(value);
        break;
    }
        // GeneratedCommandList is not supported by UCL
        // AcceptedCommandList is not supported by UCL
        // EventList is not supported by UCL
        // AttributeList is not supported by UCL
        // FeatureMap is not supported by UCL
        // ClusterRevision is not supported by UCL
    }

    if (!jsn.empty()) {
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) + "/OnOff/Commands/WriteAttributes";
        std::string payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NO_MESSAGE_HANDLER;
}

void OnOffAttributeAccess::reported_updated(const bridged_endpoint* ep, const std::string& cluster, const std::string& attribute,
    const nlohmann::json& unify_value)
{
    namespace MN = chip::app::Clusters::OnOff::Attributes;
    namespace UN = unify::matter_bridge::OnOff::Attributes;

    auto cluster_id = m_dev_translator.get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::OnOff::Id)) {
        return;
    }

    // get attribute id
    auto attribute_id = m_dev_translator.get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value()) {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::OnOff::Id, attribute_id.value());
    switch (attribute_id.value()) {
    // type is boolean
    case MN::OnOff::Id: {
        using T = MN::OnOff::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "OnOff attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OnOff::Id, MN::OnOff::Id);
        }
        break;
    }
        // type is boolean
    case MN::GlobalSceneControl::Id: {
        using T = MN::GlobalSceneControl::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "GlobalSceneControl attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OnOff::Id, MN::GlobalSceneControl::Id);
        }
        break;
    }
        // type is int16u
    case MN::OnTime::Id: {
        using T = MN::OnTime::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "OnTime attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OnOff::Id, MN::OnTime::Id);
        }
        break;
    }
        // type is int16u
    case MN::OffWaitTime::Id: {
        using T = MN::OffWaitTime::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "OffWaitTime attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OnOff::Id, MN::OffWaitTime::Id);
        }
        break;
    }
        // type is StartUpOnOffEnum
    case MN::StartUpOnOff::Id: {
        using T = MN::StartUpOnOff::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "StartUpOnOff attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OnOff::Id, MN::StartUpOnOff::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
LevelControlAttributeAccess::Read(const ConcreteReadAttributePath& aPath, AttributeValueEncoder& aEncoder)
{
    namespace MN = chip::app::Clusters::LevelControl::Attributes;
    namespace UN = unify::matter_bridge::LevelControl::Attributes;
    if (aPath.mClusterId != Clusters::LevelControl::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    // Do not handle Read for non-unify endpoints
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().read_attribute(aPath, aEncoder);
    }

    try {
        switch (aPath.mAttributeId) {
        case MN::CurrentLevel::Id: { // type is int8u
            MN::CurrentLevel::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RemainingTime::Id: { // type is int16u
            MN::RemainingTime::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MinLevel::Id: { // type is int8u
            MN::MinLevel::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MaxLevel::Id: { // type is int8u
            MN::MaxLevel::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::CurrentFrequency::Id: { // type is int16u
            MN::CurrentFrequency::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MinFrequency::Id: { // type is int16u
            MN::MinFrequency::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MaxFrequency::Id: { // type is int16u
            MN::MaxFrequency::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Options::Id: { // type is OptionsBitmap
            MN::Options::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::OnOffTransitionTime::Id: { // type is int16u
            MN::OnOffTransitionTime::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::OnLevel::Id: { // type is int8u
            MN::OnLevel::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::OnTransitionTime::Id: { // type is int16u
            MN::OnTransitionTime::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::OffTransitionTime::Id: { // type is int16u
            MN::OffTransitionTime::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::DefaultMoveRate::Id: { // type is int8u
            MN::DefaultMoveRate::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::StartUpCurrentLevel::Id: { // type is int8u
            MN::StartUpCurrentLevel::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        }
    } catch (const std::out_of_range& e) {
        sl_log_info(LOG_TAG,
            "The request attribute Path for endpoint [%i] is not found in the attribute state "
            "container: %s\n",
            atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR LevelControlAttributeAccess::Write(const ConcreteDataAttributePath& aPath, AttributeValueDecoder& aDecoder)
{
    using namespace chip::app::Clusters::LevelControl;

    if (aPath.mClusterId != Clusters::LevelControl::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }
    nlohmann::json jsn;

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().write_attribute(aPath, aDecoder);
    }

    switch (aPath.mAttributeId) {
    // CurrentLevel is not supported by UCL
    // RemainingTime is not supported by UCL
    // MinLevel is not supported by UCL
    // MaxLevel is not supported by UCL
    // CurrentFrequency is not supported by UCL
    // MinFrequency is not supported by UCL
    // MaxFrequency is not supported by UCL
    case Attributes::Options::Id: {

        Attributes::Options::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["Options"] = to_json(value);
        break;
    }
    case Attributes::OnOffTransitionTime::Id: {

        Attributes::OnOffTransitionTime::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["OnOffTransitionTime"] = to_json(value);
        break;
    }
    case Attributes::OnLevel::Id: {

        Attributes::OnLevel::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["OnLevel"] = to_json(value);
        break;
    }
    case Attributes::OnTransitionTime::Id: {

        Attributes::OnTransitionTime::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["OnTransitionTime"] = to_json(value);
        break;
    }
    case Attributes::OffTransitionTime::Id: {

        Attributes::OffTransitionTime::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["OffTransitionTime"] = to_json(value);
        break;
    }
    case Attributes::DefaultMoveRate::Id: {

        Attributes::DefaultMoveRate::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["DefaultMoveRate"] = to_json(value);
        break;
    }
    case Attributes::StartUpCurrentLevel::Id: {

        Attributes::StartUpCurrentLevel::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["StartUpCurrentLevel"] = to_json(value);
        break;
    }
        // GeneratedCommandList is not supported by UCL
        // AcceptedCommandList is not supported by UCL
        // EventList is not supported by UCL
        // AttributeList is not supported by UCL
        // FeatureMap is not supported by UCL
        // ClusterRevision is not supported by UCL
    }

    if (!jsn.empty()) {
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) + "/Level/Commands/WriteAttributes";
        std::string payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NO_MESSAGE_HANDLER;
}

void LevelControlAttributeAccess::reported_updated(const bridged_endpoint* ep, const std::string& cluster,
    const std::string& attribute, const nlohmann::json& unify_value)
{
    namespace MN = chip::app::Clusters::LevelControl::Attributes;
    namespace UN = unify::matter_bridge::LevelControl::Attributes;

    auto cluster_id = m_dev_translator.get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::LevelControl::Id)) {
        return;
    }

    // get attribute id
    auto attribute_id = m_dev_translator.get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value()) {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::LevelControl::Id, attribute_id.value());
    switch (attribute_id.value()) {
    // type is int8u
    case MN::CurrentLevel::Id: {
        using T = MN::CurrentLevel::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "CurrentLevel attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::CurrentLevel::Id);
        }
        break;
    }
        // type is int16u
    case MN::RemainingTime::Id: {
        using T = MN::RemainingTime::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RemainingTime attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::RemainingTime::Id);
        }
        break;
    }
        // type is int8u
    case MN::MinLevel::Id: {
        using T = MN::MinLevel::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MinLevel attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::MinLevel::Id);
        }
        break;
    }
        // type is int8u
    case MN::MaxLevel::Id: {
        using T = MN::MaxLevel::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MaxLevel attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::MaxLevel::Id);
        }
        break;
    }
        // type is int16u
    case MN::CurrentFrequency::Id: {
        using T = MN::CurrentFrequency::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "CurrentFrequency attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::CurrentFrequency::Id);
        }
        break;
    }
        // type is int16u
    case MN::MinFrequency::Id: {
        using T = MN::MinFrequency::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MinFrequency attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::MinFrequency::Id);
        }
        break;
    }
        // type is int16u
    case MN::MaxFrequency::Id: {
        using T = MN::MaxFrequency::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MaxFrequency attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::MaxFrequency::Id);
        }
        break;
    }
        // type is OptionsBitmap
    case MN::Options::Id: {
        using T = MN::Options::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Options attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::Options::Id);
        }
        break;
    }
        // type is int16u
    case MN::OnOffTransitionTime::Id: {
        using T = MN::OnOffTransitionTime::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "OnOffTransitionTime attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::OnOffTransitionTime::Id);
        }
        break;
    }
        // type is int8u
    case MN::OnLevel::Id: {
        using T = MN::OnLevel::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "OnLevel attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::OnLevel::Id);
        }
        break;
    }
        // type is int16u
    case MN::OnTransitionTime::Id: {
        using T = MN::OnTransitionTime::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "OnTransitionTime attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::OnTransitionTime::Id);
        }
        break;
    }
        // type is int16u
    case MN::OffTransitionTime::Id: {
        using T = MN::OffTransitionTime::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "OffTransitionTime attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::OffTransitionTime::Id);
        }
        break;
    }
        // type is int8u
    case MN::DefaultMoveRate::Id: {
        using T = MN::DefaultMoveRate::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "DefaultMoveRate attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::DefaultMoveRate::Id);
        }
        break;
    }
        // type is int8u
    case MN::StartUpCurrentLevel::Id: {
        using T = MN::StartUpCurrentLevel::TypeInfo::Type;
        nlohmann::json modified_unify_value = unify_value;

        if (strcmp(modified_unify_value.dump().c_str(), "\"MinimumDeviceValuePermitted\"") == 0) {
            modified_unify_value = 0;
        } else if (strcmp(modified_unify_value.dump().c_str(), "\"SetToPreviousValue\"") == 0) {
            modified_unify_value = 0xFF;
        }

        std::optional<T> value = from_json<T>(modified_unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "StartUpCurrentLevel attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::StartUpCurrentLevel::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
DoorLockAttributeAccess::Read(const ConcreteReadAttributePath& aPath, AttributeValueEncoder& aEncoder)
{
    namespace MN = chip::app::Clusters::DoorLock::Attributes;
    namespace UN = unify::matter_bridge::DoorLock::Attributes;
    if (aPath.mClusterId != Clusters::DoorLock::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    // Do not handle Read for non-unify endpoints
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().read_attribute(aPath, aEncoder);
    }

    try {
        switch (aPath.mAttributeId) {
        case MN::LockState::Id: { // type is DlLockState
            MN::LockState::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::LockType::Id: { // type is DlLockType
            MN::LockType::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ActuatorEnabled::Id: { // type is boolean
            MN::ActuatorEnabled::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::DoorState::Id: { // type is DoorStateEnum
            MN::DoorState::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::DoorOpenEvents::Id: { // type is int32u
            MN::DoorOpenEvents::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::DoorClosedEvents::Id: { // type is int32u
            MN::DoorClosedEvents::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::OpenPeriod::Id: { // type is int16u
            MN::OpenPeriod::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::NumberOfTotalUsersSupported::Id: { // type is int16u
            MN::NumberOfTotalUsersSupported::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::NumberOfPINUsersSupported::Id: { // type is int16u
            MN::NumberOfPINUsersSupported::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::NumberOfRFIDUsersSupported::Id: { // type is int16u
            MN::NumberOfRFIDUsersSupported::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::NumberOfWeekDaySchedulesSupportedPerUser::Id: { // type is int8u
            MN::NumberOfWeekDaySchedulesSupportedPerUser::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::NumberOfYearDaySchedulesSupportedPerUser::Id: { // type is int8u
            MN::NumberOfYearDaySchedulesSupportedPerUser::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::NumberOfHolidaySchedulesSupported::Id: { // type is int8u
            MN::NumberOfHolidaySchedulesSupported::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MaxPINCodeLength::Id: { // type is int8u
            MN::MaxPINCodeLength::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MinPINCodeLength::Id: { // type is int8u
            MN::MinPINCodeLength::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MaxRFIDCodeLength::Id: { // type is int8u
            MN::MaxRFIDCodeLength::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MinRFIDCodeLength::Id: { // type is int8u
            MN::MinRFIDCodeLength::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::CredentialRulesSupport::Id: { // type is DlCredentialRuleMask
            MN::CredentialRulesSupport::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::NumberOfCredentialsSupportedPerUser::Id: { // type is int8u
            MN::NumberOfCredentialsSupportedPerUser::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Language::Id: { // type is char_string
            MN::Language::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::LEDSettings::Id: { // type is int8u
            MN::LEDSettings::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AutoRelockTime::Id: { // type is int32u
            MN::AutoRelockTime::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::SoundVolume::Id: { // type is int8u
            MN::SoundVolume::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::OperatingMode::Id: { // type is OperatingModeEnum
            MN::OperatingMode::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::SupportedOperatingModes::Id: { // type is DlSupportedOperatingModes
            MN::SupportedOperatingModes::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::DefaultConfigurationRegister::Id: { // type is DlDefaultConfigurationRegister
            MN::DefaultConfigurationRegister::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::EnableLocalProgramming::Id: { // type is boolean
            MN::EnableLocalProgramming::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::EnableOneTouchLocking::Id: { // type is boolean
            MN::EnableOneTouchLocking::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::EnableInsideStatusLED::Id: { // type is boolean
            MN::EnableInsideStatusLED::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::EnablePrivacyModeButton::Id: { // type is boolean
            MN::EnablePrivacyModeButton::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::LocalProgrammingFeatures::Id: { // type is DlLocalProgrammingFeatures
            MN::LocalProgrammingFeatures::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::WrongCodeEntryLimit::Id: { // type is int8u
            MN::WrongCodeEntryLimit::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::UserCodeTemporaryDisableTime::Id: { // type is int8u
            MN::UserCodeTemporaryDisableTime::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::SendPINOverTheAir::Id: { // type is boolean
            MN::SendPINOverTheAir::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RequirePINforRemoteOperation::Id: { // type is boolean
            MN::RequirePINforRemoteOperation::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ExpiringUserTimeout::Id: { // type is int16u
            MN::ExpiringUserTimeout::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AliroReaderVerificationKey::Id: { // type is octet_string
            MN::AliroReaderVerificationKey::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AliroReaderGroupIdentifier::Id: { // type is octet_string
            MN::AliroReaderGroupIdentifier::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AliroReaderGroupSubIdentifier::Id: { // type is octet_string
            MN::AliroReaderGroupSubIdentifier::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AliroGroupResolvingKey::Id: { // type is octet_string
            MN::AliroGroupResolvingKey::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AliroBLEAdvertisingVersion::Id: { // type is int8u
            MN::AliroBLEAdvertisingVersion::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::NumberOfAliroCredentialIssuerKeysSupported::Id: { // type is int16u
            MN::NumberOfAliroCredentialIssuerKeysSupported::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::NumberOfAliroEndpointKeysSupported::Id: { // type is int16u
            MN::NumberOfAliroEndpointKeysSupported::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        }
    } catch (const std::out_of_range& e) {
        sl_log_info(LOG_TAG,
            "The request attribute Path for endpoint [%i] is not found in the attribute state "
            "container: %s\n",
            atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR DoorLockAttributeAccess::Write(const ConcreteDataAttributePath& aPath, AttributeValueDecoder& aDecoder)
{
    using namespace chip::app::Clusters::DoorLock;

    if (aPath.mClusterId != Clusters::DoorLock::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }
    nlohmann::json jsn;

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().write_attribute(aPath, aDecoder);
    }

    switch (aPath.mAttributeId) {
    // LockState is not supported by UCL
    // LockType is not supported by UCL
    // ActuatorEnabled is not supported by UCL
    // DoorState is not supported by UCL
    case Attributes::DoorOpenEvents::Id: {

        Attributes::DoorOpenEvents::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["DoorOpenEvents"] = to_json(value);
        break;
    }
    case Attributes::DoorClosedEvents::Id: {

        Attributes::DoorClosedEvents::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["DoorClosedEvents"] = to_json(value);
        break;
    }
    case Attributes::OpenPeriod::Id: {

        Attributes::OpenPeriod::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["OpenPeriod"] = to_json(value);
        break;
    }
    // NumberOfTotalUsersSupported is not supported by UCL
    // NumberOfPINUsersSupported is not supported by UCL
    // NumberOfRFIDUsersSupported is not supported by UCL
    // NumberOfWeekDaySchedulesSupportedPerUser is not supported by UCL
    // NumberOfYearDaySchedulesSupportedPerUser is not supported by UCL
    // NumberOfHolidaySchedulesSupported is not supported by UCL
    // MaxPINCodeLength is not supported by UCL
    // MinPINCodeLength is not supported by UCL
    // MaxRFIDCodeLength is not supported by UCL
    // MinRFIDCodeLength is not supported by UCL
    // CredentialRulesSupport is not supported by UCL
    // NumberOfCredentialsSupportedPerUser is not supported by UCL
    case Attributes::Language::Id: {

        Attributes::Language::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["Language"] = to_json(value);
        break;
    }
    case Attributes::LEDSettings::Id: {

        Attributes::LEDSettings::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["LEDSettings"] = to_json(value);
        break;
    }
    case Attributes::AutoRelockTime::Id: {

        Attributes::AutoRelockTime::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["AutoRelockTime"] = to_json(value);
        break;
    }
    case Attributes::SoundVolume::Id: {

        Attributes::SoundVolume::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["SoundVolume"] = to_json(value);
        break;
    }
    case Attributes::OperatingMode::Id: {

        Attributes::OperatingMode::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["OperatingMode"] = to_json(value);
        break;
    }
    // SupportedOperatingModes is not supported by UCL
    // DefaultConfigurationRegister is not supported by UCL
    case Attributes::EnableLocalProgramming::Id: {

        Attributes::EnableLocalProgramming::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["EnableLocalProgramming"] = to_json(value);
        break;
    }
    case Attributes::EnableOneTouchLocking::Id: {

        Attributes::EnableOneTouchLocking::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["EnableOneTouchLocking"] = to_json(value);
        break;
    }
    case Attributes::EnableInsideStatusLED::Id: {

        Attributes::EnableInsideStatusLED::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["EnableInsideStatusLED"] = to_json(value);
        break;
    }
    case Attributes::EnablePrivacyModeButton::Id: {

        Attributes::EnablePrivacyModeButton::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["EnablePrivacyModeButton"] = to_json(value);
        break;
    }
    case Attributes::LocalProgrammingFeatures::Id: {

        Attributes::LocalProgrammingFeatures::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["LocalProgrammingFeatures"] = to_json(value);
        break;
    }
    case Attributes::WrongCodeEntryLimit::Id: {

        Attributes::WrongCodeEntryLimit::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["WrongCodeEntryLimit"] = to_json(value);
        break;
    }
    case Attributes::UserCodeTemporaryDisableTime::Id: {

        Attributes::UserCodeTemporaryDisableTime::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["UserCodeTemporaryDisableTime"] = to_json(value);
        break;
    }
    case Attributes::SendPINOverTheAir::Id: {

        Attributes::SendPINOverTheAir::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["SendPINOverTheAir"] = to_json(value);
        break;
    }
    case Attributes::RequirePINforRemoteOperation::Id: {

        Attributes::RequirePINforRemoteOperation::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["RequirePINforRFOperation"] = to_json(value);
        break;
    }
    case Attributes::ExpiringUserTimeout::Id: {

        Attributes::ExpiringUserTimeout::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ExpiringUserTimeout"] = to_json(value);
        break;
    }
        // AliroReaderVerificationKey is not supported by UCL
        // AliroReaderGroupIdentifier is not supported by UCL
        // AliroReaderGroupSubIdentifier is not supported by UCL
        // AliroExpeditedTransactionSupportedProtocolVersions is not supported by UCL
        // AliroGroupResolvingKey is not supported by UCL
        // AliroSupportedBLEUWBProtocolVersions is not supported by UCL
        // AliroBLEAdvertisingVersion is not supported by UCL
        // NumberOfAliroCredentialIssuerKeysSupported is not supported by UCL
        // NumberOfAliroEndpointKeysSupported is not supported by UCL
        // GeneratedCommandList is not supported by UCL
        // AcceptedCommandList is not supported by UCL
        // EventList is not supported by UCL
        // AttributeList is not supported by UCL
        // FeatureMap is not supported by UCL
        // ClusterRevision is not supported by UCL
    }

    if (!jsn.empty()) {
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) + "/DoorLock/Commands/WriteAttributes";
        std::string payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NO_MESSAGE_HANDLER;
}

void DoorLockAttributeAccess::trigger_event(const bridged_endpoint* ep, const chip::EventId& eventid,
    const nlohmann::json& unify_value)
{
    namespace ME = chip::app::Clusters::DoorLock::Events;

    EventNumber eventNumber;
    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    bool event_valid = false;

    switch (eventid) {
    case ME::DoorLockAlarm::Id: {
        chip::app::Clusters::DoorLock::Events::DoorLockAlarm::Type event;

        if (strcmp(unify_value.dump().c_str(), "\"ErrorJammed\"") == 0) { // DoorJammed
            event.alarmCode = chip::app::Clusters::DoorLock::AlarmCodeEnum::kLockJammed;
            event_valid = true;
        }
        if (event_valid) {
            if (CHIP_NO_ERROR != LogEvent(event, node_matter_endpoint, eventNumber)) {
                sl_log_warning(LOG_TAG, "DoorLockAlarm: Failed to trigger event");
            }
        }
        break;
    }
    case ME::DoorStateChange::Id: {
        chip::app::Clusters::DoorLock::Events::DoorStateChange::Type event;

        if (event_valid) {
            if (CHIP_NO_ERROR != LogEvent(event, node_matter_endpoint, eventNumber)) {
                sl_log_warning(LOG_TAG, "DoorStateChange: Failed to trigger event");
            }
        }
        break;
    }
    case ME::LockOperation::Id: {
        chip::app::Clusters::DoorLock::Events::LockOperation::Type event;

        if (strcmp(unify_value.dump().c_str(), "\"Locked\"") == 0) { // Door locked event
            event.lockOperationType = chip::app::Clusters::DoorLock::LockOperationTypeEnum::kLock;
            event_valid = true;
        } else if (strcmp(unify_value.dump().c_str(), "\"Unlocked\"") == 0) { // Door unlocked event
            event.lockOperationType = chip::app::Clusters::DoorLock::LockOperationTypeEnum::kUnlock;
            event_valid = true;
        }
        if (event_valid) {
            if (CHIP_NO_ERROR != LogEvent(event, node_matter_endpoint, eventNumber)) {
                sl_log_warning(LOG_TAG, "LockOperation: Failed to trigger event");
            }
        }
        break;
    }
    case ME::LockOperationError::Id: {
        chip::app::Clusters::DoorLock::Events::LockOperationError::Type event;

        if (event_valid) {
            if (CHIP_NO_ERROR != LogEvent(event, node_matter_endpoint, eventNumber)) {
                sl_log_warning(LOG_TAG, "LockOperationError: Failed to trigger event");
            }
        }
        break;
    }
    case ME::LockUserChange::Id: {
        chip::app::Clusters::DoorLock::Events::LockUserChange::Type event;

        if (event_valid) {
            if (CHIP_NO_ERROR != LogEvent(event, node_matter_endpoint, eventNumber)) {
                sl_log_warning(LOG_TAG, "LockUserChange: Failed to trigger event");
            }
        }
        break;
    }
    }
}

void DoorLockAttributeAccess::reported_updated(const bridged_endpoint* ep, const std::string& cluster,
    const std::string& attribute, const nlohmann::json& unify_value)
{
    namespace MN = chip::app::Clusters::DoorLock::Attributes;
    namespace UN = unify::matter_bridge::DoorLock::Attributes;

    auto cluster_id = m_dev_translator.get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::DoorLock::Id)) {
        return;
    }

    // get attribute id
    auto attribute_id = m_dev_translator.get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value()) {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::DoorLock::Id, attribute_id.value());
    switch (attribute_id.value()) {
    // type is DlLockState
    case MN::LockState::Id: {
        using T = MN::LockState::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "LockState attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::LockState::Id);
            trigger_event(ep, chip::app::Clusters::DoorLock::Events::LockOperation::Id, unify_value);
        }
        break;
    }
        // type is DlLockType
    case MN::LockType::Id: {
        using T = MN::LockType::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "LockType attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::LockType::Id);
        }
        break;
    }
        // type is boolean
    case MN::ActuatorEnabled::Id: {
        using T = MN::ActuatorEnabled::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ActuatorEnabled attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::ActuatorEnabled::Id);
        }
        break;
    }
        // type is DoorStateEnum
    case MN::DoorState::Id: {
        using T = MN::DoorState::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "DoorState attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::DoorState::Id);
            trigger_event(ep, chip::app::Clusters::DoorLock::Events::DoorLockAlarm::Id, unify_value);
        }
        break;
    }
        // type is int32u
    case MN::DoorOpenEvents::Id: {
        using T = MN::DoorOpenEvents::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "DoorOpenEvents attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::DoorOpenEvents::Id);
        }
        break;
    }
        // type is int32u
    case MN::DoorClosedEvents::Id: {
        using T = MN::DoorClosedEvents::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "DoorClosedEvents attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::DoorClosedEvents::Id);
        }
        break;
    }
        // type is int16u
    case MN::OpenPeriod::Id: {
        using T = MN::OpenPeriod::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "OpenPeriod attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::OpenPeriod::Id);
        }
        break;
    }
        // type is int16u
    case MN::NumberOfTotalUsersSupported::Id: {
        using T = MN::NumberOfTotalUsersSupported::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "NumberOfTotalUsersSupported attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id,
                MN::NumberOfTotalUsersSupported::Id);
        }
        break;
    }
        // type is int16u
    case MN::NumberOfPINUsersSupported::Id: {
        using T = MN::NumberOfPINUsersSupported::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "NumberOfPINUsersSupported attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::NumberOfPINUsersSupported::Id);
        }
        break;
    }
        // type is int16u
    case MN::NumberOfRFIDUsersSupported::Id: {
        using T = MN::NumberOfRFIDUsersSupported::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "NumberOfRFIDUsersSupported attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id,
                MN::NumberOfRFIDUsersSupported::Id);
        }
        break;
    }
        // type is int8u
    case MN::NumberOfWeekDaySchedulesSupportedPerUser::Id: {
        using T = MN::NumberOfWeekDaySchedulesSupportedPerUser::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "NumberOfWeekDaySchedulesSupportedPerUser attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id,
                MN::NumberOfWeekDaySchedulesSupportedPerUser::Id);
        }
        break;
    }
        // type is int8u
    case MN::NumberOfYearDaySchedulesSupportedPerUser::Id: {
        using T = MN::NumberOfYearDaySchedulesSupportedPerUser::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "NumberOfYearDaySchedulesSupportedPerUser attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id,
                MN::NumberOfYearDaySchedulesSupportedPerUser::Id);
        }
        break;
    }
        // type is int8u
    case MN::NumberOfHolidaySchedulesSupported::Id: {
        using T = MN::NumberOfHolidaySchedulesSupported::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "NumberOfHolidaySchedulesSupported attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id,
                MN::NumberOfHolidaySchedulesSupported::Id);
        }
        break;
    }
        // type is int8u
    case MN::MaxPINCodeLength::Id: {
        using T = MN::MaxPINCodeLength::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MaxPINCodeLength attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::MaxPINCodeLength::Id);
        }
        break;
    }
        // type is int8u
    case MN::MinPINCodeLength::Id: {
        using T = MN::MinPINCodeLength::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MinPINCodeLength attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::MinPINCodeLength::Id);
        }
        break;
    }
        // type is int8u
    case MN::MaxRFIDCodeLength::Id: {
        using T = MN::MaxRFIDCodeLength::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MaxRFIDCodeLength attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::MaxRFIDCodeLength::Id);
        }
        break;
    }
        // type is int8u
    case MN::MinRFIDCodeLength::Id: {
        using T = MN::MinRFIDCodeLength::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MinRFIDCodeLength attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::MinRFIDCodeLength::Id);
        }
        break;
    }
        // type is DlCredentialRuleMask
    case MN::CredentialRulesSupport::Id: {
        using T = MN::CredentialRulesSupport::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "CredentialRulesSupport attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::CredentialRulesSupport::Id);
        }
        break;
    }
        // type is int8u
    case MN::NumberOfCredentialsSupportedPerUser::Id: {
        using T = MN::NumberOfCredentialsSupportedPerUser::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "NumberOfCredentialsSupportedPerUser attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id,
                MN::NumberOfCredentialsSupportedPerUser::Id);
        }
        break;
    }
        // type is char_string
    case MN::Language::Id: {
        using T = MN::Language::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Language attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::Language::Id);
        }
        break;
    }
        // type is int8u
    case MN::LEDSettings::Id: {
        using T = MN::LEDSettings::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "LEDSettings attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::LEDSettings::Id);
        }
        break;
    }
        // type is int32u
    case MN::AutoRelockTime::Id: {
        using T = MN::AutoRelockTime::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "AutoRelockTime attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::AutoRelockTime::Id);
        }
        break;
    }
        // type is int8u
    case MN::SoundVolume::Id: {
        using T = MN::SoundVolume::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "SoundVolume attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::SoundVolume::Id);
        }
        break;
    }
        // type is OperatingModeEnum
    case MN::OperatingMode::Id: {
        using T = MN::OperatingMode::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "OperatingMode attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::OperatingMode::Id);
        }
        break;
    }
        // type is DlSupportedOperatingModes
    case MN::SupportedOperatingModes::Id: {
        using T = MN::SupportedOperatingModes::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "SupportedOperatingModes attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::SupportedOperatingModes::Id);
        }
        break;
    }
        // type is DlDefaultConfigurationRegister
    case MN::DefaultConfigurationRegister::Id: {
        using T = MN::DefaultConfigurationRegister::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "DefaultConfigurationRegister attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id,
                MN::DefaultConfigurationRegister::Id);
        }
        break;
    }
        // type is boolean
    case MN::EnableLocalProgramming::Id: {
        using T = MN::EnableLocalProgramming::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "EnableLocalProgramming attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::EnableLocalProgramming::Id);
        }
        break;
    }
        // type is boolean
    case MN::EnableOneTouchLocking::Id: {
        using T = MN::EnableOneTouchLocking::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "EnableOneTouchLocking attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::EnableOneTouchLocking::Id);
        }
        break;
    }
        // type is boolean
    case MN::EnableInsideStatusLED::Id: {
        using T = MN::EnableInsideStatusLED::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "EnableInsideStatusLED attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::EnableInsideStatusLED::Id);
        }
        break;
    }
        // type is boolean
    case MN::EnablePrivacyModeButton::Id: {
        using T = MN::EnablePrivacyModeButton::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "EnablePrivacyModeButton attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::EnablePrivacyModeButton::Id);
        }
        break;
    }
        // type is DlLocalProgrammingFeatures
    case MN::LocalProgrammingFeatures::Id: {
        using T = MN::LocalProgrammingFeatures::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "LocalProgrammingFeatures attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::LocalProgrammingFeatures::Id);
        }
        break;
    }
        // type is int8u
    case MN::WrongCodeEntryLimit::Id: {
        using T = MN::WrongCodeEntryLimit::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "WrongCodeEntryLimit attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::WrongCodeEntryLimit::Id);
        }
        break;
    }
        // type is int8u
    case MN::UserCodeTemporaryDisableTime::Id: {
        using T = MN::UserCodeTemporaryDisableTime::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "UserCodeTemporaryDisableTime attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id,
                MN::UserCodeTemporaryDisableTime::Id);
        }
        break;
    }
        // type is boolean
    case MN::SendPINOverTheAir::Id: {
        using T = MN::SendPINOverTheAir::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "SendPINOverTheAir attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::SendPINOverTheAir::Id);
        }
        break;
    }
        // type is boolean
    case MN::RequirePINforRemoteOperation::Id: {
        using T = MN::RequirePINforRemoteOperation::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RequirePINforRemoteOperation attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id,
                MN::RequirePINforRemoteOperation::Id);
        }
        break;
    }
        // type is int16u
    case MN::ExpiringUserTimeout::Id: {
        using T = MN::ExpiringUserTimeout::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ExpiringUserTimeout attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::ExpiringUserTimeout::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::FeatureMap::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
BarrierControlAttributeAccess::Read(const ConcreteReadAttributePath& aPath, AttributeValueEncoder& aEncoder)
{
    namespace MN = chip::app::Clusters::BarrierControl::Attributes;
    namespace UN = unify::matter_bridge::BarrierControl::Attributes;
    if (aPath.mClusterId != Clusters::BarrierControl::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    // Do not handle Read for non-unify endpoints
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().read_attribute(aPath, aEncoder);
    }

    try {
        switch (aPath.mAttributeId) {
        case MN::BarrierMovingState::Id: { // type is enum8
            MN::BarrierMovingState::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::BarrierSafetyStatus::Id: { // type is bitmap16
            MN::BarrierSafetyStatus::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::BarrierCapabilities::Id: { // type is bitmap8
            MN::BarrierCapabilities::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::BarrierOpenEvents::Id: { // type is int16u
            MN::BarrierOpenEvents::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::BarrierCloseEvents::Id: { // type is int16u
            MN::BarrierCloseEvents::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::BarrierCommandOpenEvents::Id: { // type is int16u
            MN::BarrierCommandOpenEvents::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::BarrierCommandCloseEvents::Id: { // type is int16u
            MN::BarrierCommandCloseEvents::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::BarrierOpenPeriod::Id: { // type is int16u
            MN::BarrierOpenPeriod::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::BarrierClosePeriod::Id: { // type is int16u
            MN::BarrierClosePeriod::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::BarrierPosition::Id: { // type is int8u
            MN::BarrierPosition::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        }
    } catch (const std::out_of_range& e) {
        sl_log_info(LOG_TAG,
            "The request attribute Path for endpoint [%i] is not found in the attribute state "
            "container: %s\n",
            atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR BarrierControlAttributeAccess::Write(const ConcreteDataAttributePath& aPath, AttributeValueDecoder& aDecoder)
{
    using namespace chip::app::Clusters::BarrierControl;

    if (aPath.mClusterId != Clusters::BarrierControl::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }
    nlohmann::json jsn;

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().write_attribute(aPath, aDecoder);
    }

    switch (aPath.mAttributeId) {
    // barrier moving state is not supported by UCL
    // barrier safety status is not supported by UCL
    // barrier capabilities is not supported by UCL
    case Attributes::BarrierOpenEvents::Id: {

        Attributes::BarrierOpenEvents::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["OpenEvents"] = to_json(value);
        break;
    }
    case Attributes::BarrierCloseEvents::Id: {

        Attributes::BarrierCloseEvents::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["CloseEvents"] = to_json(value);
        break;
    }
    case Attributes::BarrierCommandOpenEvents::Id: {

        Attributes::BarrierCommandOpenEvents::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["CommandOpenEvents"] = to_json(value);
        break;
    }
    case Attributes::BarrierCommandCloseEvents::Id: {

        Attributes::BarrierCommandCloseEvents::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["CommandCloseEvents"] = to_json(value);
        break;
    }
    case Attributes::BarrierOpenPeriod::Id: {

        Attributes::BarrierOpenPeriod::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["OpenPeriod"] = to_json(value);
        break;
    }
    case Attributes::BarrierClosePeriod::Id: {

        Attributes::BarrierClosePeriod::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ClosePeriod"] = to_json(value);
        break;
    }
        // barrier position is not supported by UCL
        // GeneratedCommandList is not supported by UCL
        // AcceptedCommandList is not supported by UCL
        // EventList is not supported by UCL
        // AttributeList is not supported by UCL
        // FeatureMap is not supported by UCL
        // ClusterRevision is not supported by UCL
    }

    if (!jsn.empty()) {
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) + "/BarrierControl/Commands/WriteAttributes";
        std::string payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NO_MESSAGE_HANDLER;
}

void BarrierControlAttributeAccess::reported_updated(const bridged_endpoint* ep, const std::string& cluster,
    const std::string& attribute, const nlohmann::json& unify_value)
{
    namespace MN = chip::app::Clusters::BarrierControl::Attributes;
    namespace UN = unify::matter_bridge::BarrierControl::Attributes;

    auto cluster_id = m_dev_translator.get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::BarrierControl::Id)) {
        return;
    }

    // get attribute id
    auto attribute_id = m_dev_translator.get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value()) {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::BarrierControl::Id, attribute_id.value());
    switch (attribute_id.value()) {
    // type is enum8
    case MN::BarrierMovingState::Id: {
        using T = MN::BarrierMovingState::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "BarrierMovingState attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::BarrierControl::Id, MN::BarrierMovingState::Id);
        }
        break;
    }
        // type is bitmap16
    case MN::BarrierSafetyStatus::Id: {
        using T = MN::BarrierSafetyStatus::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "BarrierSafetyStatus attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::BarrierControl::Id, MN::BarrierSafetyStatus::Id);
        }
        break;
    }
        // type is bitmap8
    case MN::BarrierCapabilities::Id: {
        using T = MN::BarrierCapabilities::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "BarrierCapabilities attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::BarrierControl::Id, MN::BarrierCapabilities::Id);
        }
        break;
    }
        // type is int16u
    case MN::BarrierOpenEvents::Id: {
        using T = MN::BarrierOpenEvents::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "BarrierOpenEvents attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::BarrierControl::Id, MN::BarrierOpenEvents::Id);
        }
        break;
    }
        // type is int16u
    case MN::BarrierCloseEvents::Id: {
        using T = MN::BarrierCloseEvents::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "BarrierCloseEvents attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::BarrierControl::Id, MN::BarrierCloseEvents::Id);
        }
        break;
    }
        // type is int16u
    case MN::BarrierCommandOpenEvents::Id: {
        using T = MN::BarrierCommandOpenEvents::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "BarrierCommandOpenEvents attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::BarrierControl::Id,
                MN::BarrierCommandOpenEvents::Id);
        }
        break;
    }
        // type is int16u
    case MN::BarrierCommandCloseEvents::Id: {
        using T = MN::BarrierCommandCloseEvents::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "BarrierCommandCloseEvents attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::BarrierControl::Id,
                MN::BarrierCommandCloseEvents::Id);
        }
        break;
    }
        // type is int16u
    case MN::BarrierOpenPeriod::Id: {
        using T = MN::BarrierOpenPeriod::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "BarrierOpenPeriod attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::BarrierControl::Id, MN::BarrierOpenPeriod::Id);
        }
        break;
    }
        // type is int16u
    case MN::BarrierClosePeriod::Id: {
        using T = MN::BarrierClosePeriod::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "BarrierClosePeriod attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::BarrierControl::Id, MN::BarrierClosePeriod::Id);
        }
        break;
    }
        // type is int8u
    case MN::BarrierPosition::Id: {
        using T = MN::BarrierPosition::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "BarrierPosition attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::BarrierControl::Id, MN::BarrierPosition::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
ThermostatAttributeAccess::Read(const ConcreteReadAttributePath& aPath, AttributeValueEncoder& aEncoder)
{
    namespace MN = chip::app::Clusters::Thermostat::Attributes;
    namespace UN = unify::matter_bridge::Thermostat::Attributes;
    if (aPath.mClusterId != Clusters::Thermostat::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    // Do not handle Read for non-unify endpoints
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().read_attribute(aPath, aEncoder);
    }

    try {
        switch (aPath.mAttributeId) {
        case MN::LocalTemperature::Id: { // type is temperature
            MN::LocalTemperature::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::OutdoorTemperature::Id: { // type is temperature
            MN::OutdoorTemperature::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Occupancy::Id: { // type is bitmap8
            MN::Occupancy::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AbsMinHeatSetpointLimit::Id: { // type is temperature
            MN::AbsMinHeatSetpointLimit::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AbsMaxHeatSetpointLimit::Id: { // type is temperature
            MN::AbsMaxHeatSetpointLimit::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AbsMinCoolSetpointLimit::Id: { // type is temperature
            MN::AbsMinCoolSetpointLimit::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AbsMaxCoolSetpointLimit::Id: { // type is temperature
            MN::AbsMaxCoolSetpointLimit::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::PICoolingDemand::Id: { // type is int8u
            MN::PICoolingDemand::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::PIHeatingDemand::Id: { // type is int8u
            MN::PIHeatingDemand::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::HVACSystemTypeConfiguration::Id: { // type is bitmap8
            MN::HVACSystemTypeConfiguration::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::LocalTemperatureCalibration::Id: { // type is int8s
            MN::LocalTemperatureCalibration::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::OccupiedCoolingSetpoint::Id: { // type is int16s
            MN::OccupiedCoolingSetpoint::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::OccupiedHeatingSetpoint::Id: { // type is int16s
            MN::OccupiedHeatingSetpoint::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::UnoccupiedCoolingSetpoint::Id: { // type is int16s
            MN::UnoccupiedCoolingSetpoint::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::UnoccupiedHeatingSetpoint::Id: { // type is int16s
            MN::UnoccupiedHeatingSetpoint::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MinHeatSetpointLimit::Id: { // type is int16s
            MN::MinHeatSetpointLimit::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MaxHeatSetpointLimit::Id: { // type is int16s
            MN::MaxHeatSetpointLimit::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MinCoolSetpointLimit::Id: { // type is int16s
            MN::MinCoolSetpointLimit::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MaxCoolSetpointLimit::Id: { // type is int16s
            MN::MaxCoolSetpointLimit::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MinSetpointDeadBand::Id: { // type is int8s
            MN::MinSetpointDeadBand::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RemoteSensing::Id: { // type is RemoteSensingBitmap
            MN::RemoteSensing::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ControlSequenceOfOperation::Id: { // type is ControlSequenceOfOperationEnum
            MN::ControlSequenceOfOperation::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::SystemMode::Id: { // type is SystemModeEnum
            MN::SystemMode::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ThermostatRunningMode::Id: { // type is ThermostatRunningModeEnum
            MN::ThermostatRunningMode::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::StartOfWeek::Id: { // type is StartOfWeekEnum
            MN::StartOfWeek::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::NumberOfWeeklyTransitions::Id: { // type is int8u
            MN::NumberOfWeeklyTransitions::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::NumberOfDailyTransitions::Id: { // type is int8u
            MN::NumberOfDailyTransitions::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::TemperatureSetpointHold::Id: { // type is TemperatureSetpointHoldEnum
            MN::TemperatureSetpointHold::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::TemperatureSetpointHoldDuration::Id: { // type is int16u
            MN::TemperatureSetpointHoldDuration::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ThermostatProgrammingOperationMode::Id: { // type is ProgrammingOperationModeBitmap
            MN::ThermostatProgrammingOperationMode::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ThermostatRunningState::Id: { // type is RelayStateBitmap
            MN::ThermostatRunningState::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::SetpointChangeSource::Id: { // type is SetpointChangeSourceEnum
            MN::SetpointChangeSource::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::SetpointChangeAmount::Id: { // type is int16s
            MN::SetpointChangeAmount::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::SetpointChangeSourceTimestamp::Id: { // type is epoch_s
            MN::SetpointChangeSourceTimestamp::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::OccupiedSetback::Id: { // type is int8u
            MN::OccupiedSetback::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::OccupiedSetbackMin::Id: { // type is int8u
            MN::OccupiedSetbackMin::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::OccupiedSetbackMax::Id: { // type is int8u
            MN::OccupiedSetbackMax::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::UnoccupiedSetback::Id: { // type is int8u
            MN::UnoccupiedSetback::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::UnoccupiedSetbackMin::Id: { // type is int8u
            MN::UnoccupiedSetbackMin::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::UnoccupiedSetbackMax::Id: { // type is int8u
            MN::UnoccupiedSetbackMax::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::EmergencyHeatDelta::Id: { // type is int8u
            MN::EmergencyHeatDelta::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ACType::Id: { // type is ACTypeEnum
            MN::ACType::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ACCapacity::Id: { // type is int16u
            MN::ACCapacity::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ACRefrigerantType::Id: { // type is ACRefrigerantTypeEnum
            MN::ACRefrigerantType::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ACCompressorType::Id: { // type is ACCompressorTypeEnum
            MN::ACCompressorType::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ACErrorCode::Id: { // type is ACErrorCodeBitmap
            MN::ACErrorCode::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ACLouverPosition::Id: { // type is ACLouverPositionEnum
            MN::ACLouverPosition::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ACCoilTemperature::Id: { // type is temperature
            MN::ACCoilTemperature::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ACCapacityformat::Id: { // type is ACCapacityFormatEnum
            MN::ACCapacityformat::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::NumberOfPresets::Id: { // type is int8u
            MN::NumberOfPresets::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::NumberOfSchedules::Id: { // type is int8u
            MN::NumberOfSchedules::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::NumberOfScheduleTransitions::Id: { // type is int8u
            MN::NumberOfScheduleTransitions::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::NumberOfScheduleTransitionPerDay::Id: { // type is int8u
            MN::NumberOfScheduleTransitionPerDay::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ActivePresetHandle::Id: { // type is octet_string
            MN::ActivePresetHandle::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ActiveScheduleHandle::Id: { // type is octet_string
            MN::ActiveScheduleHandle::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::PresetsSchedulesEditable::Id: { // type is boolean
            MN::PresetsSchedulesEditable::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::TemperatureSetpointHoldPolicy::Id: { // type is TemperatureSetpointHoldPolicyBitmap
            MN::TemperatureSetpointHoldPolicy::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::SetpointHoldExpiryTimestamp::Id: { // type is epoch_s
            MN::SetpointHoldExpiryTimestamp::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        }
    } catch (const std::out_of_range& e) {
        sl_log_info(LOG_TAG,
            "The request attribute Path for endpoint [%i] is not found in the attribute state "
            "container: %s\n",
            atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThermostatAttributeAccess::Write(const ConcreteDataAttributePath& aPath, AttributeValueDecoder& aDecoder)
{
    using namespace chip::app::Clusters::Thermostat;

    if (aPath.mClusterId != Clusters::Thermostat::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }
    nlohmann::json jsn;

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().write_attribute(aPath, aDecoder);
    }

    switch (aPath.mAttributeId) {
    // LocalTemperature is not supported by UCL
    // OutdoorTemperature is not supported by UCL
    // Occupancy is not supported by UCL
    // AbsMinHeatSetpointLimit is not supported by UCL
    // AbsMaxHeatSetpointLimit is not supported by UCL
    // AbsMinCoolSetpointLimit is not supported by UCL
    // AbsMaxCoolSetpointLimit is not supported by UCL
    // PICoolingDemand is not supported by UCL
    // PIHeatingDemand is not supported by UCL
    case Attributes::HVACSystemTypeConfiguration::Id: {

        Attributes::HVACSystemTypeConfiguration::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["HVACSystemTypeConfiguration"] = to_json(value);
        break;
    }
    case Attributes::LocalTemperatureCalibration::Id: {

        Attributes::LocalTemperatureCalibration::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["LocalTemperatureCalibration"] = to_json(value);
        break;
    }
    case Attributes::OccupiedCoolingSetpoint::Id: {

        Attributes::OccupiedCoolingSetpoint::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["OccupiedCoolingSetpoint"] = to_json(value);
        break;
    }
    case Attributes::OccupiedHeatingSetpoint::Id: {

        Attributes::OccupiedHeatingSetpoint::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["OccupiedHeatingSetpoint"] = to_json(value);
        break;
    }
    case Attributes::UnoccupiedCoolingSetpoint::Id: {

        Attributes::UnoccupiedCoolingSetpoint::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["UnoccupiedCoolingSetpoint"] = to_json(value);
        break;
    }
    case Attributes::UnoccupiedHeatingSetpoint::Id: {

        Attributes::UnoccupiedHeatingSetpoint::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["UnoccupiedHeatingSetpoint"] = to_json(value);
        break;
    }
    case Attributes::MinHeatSetpointLimit::Id: {

        Attributes::MinHeatSetpointLimit::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["MinHeatSetpointLimit"] = to_json(value);
        break;
    }
    case Attributes::MaxHeatSetpointLimit::Id: {

        Attributes::MaxHeatSetpointLimit::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["MaxHeatSetpointLimit"] = to_json(value);
        break;
    }
    case Attributes::MinCoolSetpointLimit::Id: {

        Attributes::MinCoolSetpointLimit::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["MinCoolSetpointLimit"] = to_json(value);
        break;
    }
    case Attributes::MaxCoolSetpointLimit::Id: {

        Attributes::MaxCoolSetpointLimit::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["MaxCoolSetpointLimit"] = to_json(value);
        break;
    }
    case Attributes::MinSetpointDeadBand::Id: {

        Attributes::MinSetpointDeadBand::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["MinSetpointDeadBand"] = to_json(value);
        break;
    }
    case Attributes::RemoteSensing::Id: {

        Attributes::RemoteSensing::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["RemoteSensing"] = to_json(value);
        break;
    }
    case Attributes::ControlSequenceOfOperation::Id: {

        Attributes::ControlSequenceOfOperation::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ControlSequenceOfOperation"] = to_json(value);
        break;
    }
    case Attributes::SystemMode::Id: {

        Attributes::SystemMode::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["SystemMode"] = to_json(value);
        break;
    }
    // ThermostatRunningMode is not supported by UCL
    // StartOfWeek is not supported by UCL
    // NumberOfWeeklyTransitions is not supported by UCL
    // NumberOfDailyTransitions is not supported by UCL
    case Attributes::TemperatureSetpointHold::Id: {

        Attributes::TemperatureSetpointHold::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["TemperatureSetpointHold"] = to_json(value);
        break;
    }
    case Attributes::TemperatureSetpointHoldDuration::Id: {

        Attributes::TemperatureSetpointHoldDuration::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["TemperatureSetpointHoldDuration"] = to_json(value);
        break;
    }
    case Attributes::ThermostatProgrammingOperationMode::Id: {

        Attributes::ThermostatProgrammingOperationMode::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ThermostatProgrammingOperationMode"] = to_json(value);
        break;
    }
    // ThermostatRunningState is not supported by UCL
    // SetpointChangeSource is not supported by UCL
    // SetpointChangeAmount is not supported by UCL
    // SetpointChangeSourceTimestamp is not supported by UCL
    case Attributes::OccupiedSetback::Id: {

        Attributes::OccupiedSetback::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["OccupiedSetback"] = to_json(value);
        break;
    }
    // OccupiedSetbackMin is not supported by UCL
    // OccupiedSetbackMax is not supported by UCL
    case Attributes::UnoccupiedSetback::Id: {

        Attributes::UnoccupiedSetback::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["UnoccupiedSetback"] = to_json(value);
        break;
    }
    // UnoccupiedSetbackMin is not supported by UCL
    // UnoccupiedSetbackMax is not supported by UCL
    case Attributes::EmergencyHeatDelta::Id: {

        Attributes::EmergencyHeatDelta::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["EmergencyHeatDelta"] = to_json(value);
        break;
    }
    case Attributes::ACType::Id: {

        Attributes::ACType::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ACType"] = to_json(value);
        break;
    }
    case Attributes::ACCapacity::Id: {

        Attributes::ACCapacity::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ACCapacity"] = to_json(value);
        break;
    }
    case Attributes::ACRefrigerantType::Id: {

        Attributes::ACRefrigerantType::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ACRefrigerantType"] = to_json(value);
        break;
    }
    case Attributes::ACCompressorType::Id: {

        Attributes::ACCompressorType::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ACCompressorType"] = to_json(value);
        break;
    }
    case Attributes::ACErrorCode::Id: {

        Attributes::ACErrorCode::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ACErrorCode"] = to_json(value);
        break;
    }
    case Attributes::ACLouverPosition::Id: {

        Attributes::ACLouverPosition::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ACLouverPosition"] = to_json(value);
        break;
    }
    // ACCoilTemperature is not supported by UCL
    case Attributes::ACCapacityformat::Id: {

        Attributes::ACCapacityformat::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ACCapacityFormat"] = to_json(value);
        break;
    }
        // PresetTypes is not supported by UCL
        // ScheduleTypes is not supported by UCL
        // NumberOfPresets is not supported by UCL
        // NumberOfSchedules is not supported by UCL
        // NumberOfScheduleTransitions is not supported by UCL
        // NumberOfScheduleTransitionPerDay is not supported by UCL
        // ActivePresetHandle is not supported by UCL
        // ActiveScheduleHandle is not supported by UCL
        // PresetsSchedulesEditable is not supported by UCL
        // TemperatureSetpointHoldPolicy is not supported by UCL
        // SetpointHoldExpiryTimestamp is not supported by UCL
        // QueuedPreset is not supported by UCL
        // GeneratedCommandList is not supported by UCL
        // AcceptedCommandList is not supported by UCL
        // EventList is not supported by UCL
        // AttributeList is not supported by UCL
        // FeatureMap is not supported by UCL
        // ClusterRevision is not supported by UCL
    }

    if (!jsn.empty()) {
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) + "/Thermostat/Commands/WriteAttributes";
        std::string payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NO_MESSAGE_HANDLER;
}

void ThermostatAttributeAccess::reported_updated(const bridged_endpoint* ep, const std::string& cluster,
    const std::string& attribute, const nlohmann::json& unify_value)
{
    namespace MN = chip::app::Clusters::Thermostat::Attributes;
    namespace UN = unify::matter_bridge::Thermostat::Attributes;

    auto cluster_id = m_dev_translator.get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::Thermostat::Id)) {
        return;
    }

    // get attribute id
    auto attribute_id = m_dev_translator.get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value()) {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::Thermostat::Id, attribute_id.value());
    switch (attribute_id.value()) {
    // type is temperature
    case MN::LocalTemperature::Id: {
        using T = MN::LocalTemperature::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "LocalTemperature attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::LocalTemperature::Id);
        }
        break;
    }
        // type is temperature
    case MN::OutdoorTemperature::Id: {
        using T = MN::OutdoorTemperature::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "OutdoorTemperature attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::OutdoorTemperature::Id);
        }
        break;
    }
        // type is bitmap8
    case MN::Occupancy::Id: {
        using T = MN::Occupancy::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Occupancy attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::Occupancy::Id);
        }
        break;
    }
        // type is temperature
    case MN::AbsMinHeatSetpointLimit::Id: {
        using T = MN::AbsMinHeatSetpointLimit::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "AbsMinHeatSetpointLimit attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::AbsMinHeatSetpointLimit::Id);
        }
        break;
    }
        // type is temperature
    case MN::AbsMaxHeatSetpointLimit::Id: {
        using T = MN::AbsMaxHeatSetpointLimit::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "AbsMaxHeatSetpointLimit attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::AbsMaxHeatSetpointLimit::Id);
        }
        break;
    }
        // type is temperature
    case MN::AbsMinCoolSetpointLimit::Id: {
        using T = MN::AbsMinCoolSetpointLimit::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "AbsMinCoolSetpointLimit attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::AbsMinCoolSetpointLimit::Id);
        }
        break;
    }
        // type is temperature
    case MN::AbsMaxCoolSetpointLimit::Id: {
        using T = MN::AbsMaxCoolSetpointLimit::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "AbsMaxCoolSetpointLimit attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::AbsMaxCoolSetpointLimit::Id);
        }
        break;
    }
        // type is int8u
    case MN::PICoolingDemand::Id: {
        using T = MN::PICoolingDemand::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "PICoolingDemand attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::PICoolingDemand::Id);
        }
        break;
    }
        // type is int8u
    case MN::PIHeatingDemand::Id: {
        using T = MN::PIHeatingDemand::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "PIHeatingDemand attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::PIHeatingDemand::Id);
        }
        break;
    }
        // type is bitmap8
    case MN::HVACSystemTypeConfiguration::Id: {
        using T = MN::HVACSystemTypeConfiguration::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "HVACSystemTypeConfiguration attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id,
                MN::HVACSystemTypeConfiguration::Id);
        }
        break;
    }
        // type is int8s
    case MN::LocalTemperatureCalibration::Id: {
        using T = MN::LocalTemperatureCalibration::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "LocalTemperatureCalibration attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id,
                MN::LocalTemperatureCalibration::Id);
        }
        break;
    }
        // type is int16s
    case MN::OccupiedCoolingSetpoint::Id: {
        using T = MN::OccupiedCoolingSetpoint::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "OccupiedCoolingSetpoint attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::OccupiedCoolingSetpoint::Id);
        }
        break;
    }
        // type is int16s
    case MN::OccupiedHeatingSetpoint::Id: {
        using T = MN::OccupiedHeatingSetpoint::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "OccupiedHeatingSetpoint attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::OccupiedHeatingSetpoint::Id);
        }
        break;
    }
        // type is int16s
    case MN::UnoccupiedCoolingSetpoint::Id: {
        using T = MN::UnoccupiedCoolingSetpoint::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "UnoccupiedCoolingSetpoint attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id,
                MN::UnoccupiedCoolingSetpoint::Id);
        }
        break;
    }
        // type is int16s
    case MN::UnoccupiedHeatingSetpoint::Id: {
        using T = MN::UnoccupiedHeatingSetpoint::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "UnoccupiedHeatingSetpoint attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id,
                MN::UnoccupiedHeatingSetpoint::Id);
        }
        break;
    }
        // type is int16s
    case MN::MinHeatSetpointLimit::Id: {
        using T = MN::MinHeatSetpointLimit::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MinHeatSetpointLimit attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::MinHeatSetpointLimit::Id);
        }
        break;
    }
        // type is int16s
    case MN::MaxHeatSetpointLimit::Id: {
        using T = MN::MaxHeatSetpointLimit::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MaxHeatSetpointLimit attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::MaxHeatSetpointLimit::Id);
        }
        break;
    }
        // type is int16s
    case MN::MinCoolSetpointLimit::Id: {
        using T = MN::MinCoolSetpointLimit::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MinCoolSetpointLimit attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::MinCoolSetpointLimit::Id);
        }
        break;
    }
        // type is int16s
    case MN::MaxCoolSetpointLimit::Id: {
        using T = MN::MaxCoolSetpointLimit::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MaxCoolSetpointLimit attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::MaxCoolSetpointLimit::Id);
        }
        break;
    }
        // type is int8s
    case MN::MinSetpointDeadBand::Id: {
        using T = MN::MinSetpointDeadBand::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MinSetpointDeadBand attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::MinSetpointDeadBand::Id);
        }
        break;
    }
        // type is RemoteSensingBitmap
    case MN::RemoteSensing::Id: {
        using T = MN::RemoteSensing::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RemoteSensing attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::RemoteSensing::Id);
        }
        break;
    }
        // type is ControlSequenceOfOperationEnum
    case MN::ControlSequenceOfOperation::Id: {
        using T = MN::ControlSequenceOfOperation::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ControlSequenceOfOperation attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id,
                MN::ControlSequenceOfOperation::Id);
        }
        break;
    }
        // type is SystemModeEnum
    case MN::SystemMode::Id: {
        using T = MN::SystemMode::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "SystemMode attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::SystemMode::Id);
        }
        break;
    }
        // type is ThermostatRunningModeEnum
    case MN::ThermostatRunningMode::Id: {
        using T = MN::ThermostatRunningMode::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ThermostatRunningMode attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::ThermostatRunningMode::Id);
        }
        break;
    }
        // type is StartOfWeekEnum
    case MN::StartOfWeek::Id: {
        using T = MN::StartOfWeek::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "StartOfWeek attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::StartOfWeek::Id);
        }
        break;
    }
        // type is int8u
    case MN::NumberOfWeeklyTransitions::Id: {
        using T = MN::NumberOfWeeklyTransitions::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "NumberOfWeeklyTransitions attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id,
                MN::NumberOfWeeklyTransitions::Id);
        }
        break;
    }
        // type is int8u
    case MN::NumberOfDailyTransitions::Id: {
        using T = MN::NumberOfDailyTransitions::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "NumberOfDailyTransitions attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id,
                MN::NumberOfDailyTransitions::Id);
        }
        break;
    }
        // type is TemperatureSetpointHoldEnum
    case MN::TemperatureSetpointHold::Id: {
        using T = MN::TemperatureSetpointHold::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "TemperatureSetpointHold attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::TemperatureSetpointHold::Id);
        }
        break;
    }
        // type is int16u
    case MN::TemperatureSetpointHoldDuration::Id: {
        using T = MN::TemperatureSetpointHoldDuration::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "TemperatureSetpointHoldDuration attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id,
                MN::TemperatureSetpointHoldDuration::Id);
        }
        break;
    }
        // type is ProgrammingOperationModeBitmap
    case MN::ThermostatProgrammingOperationMode::Id: {
        using T = MN::ThermostatProgrammingOperationMode::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ThermostatProgrammingOperationMode attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id,
                MN::ThermostatProgrammingOperationMode::Id);
        }
        break;
    }
        // type is RelayStateBitmap
    case MN::ThermostatRunningState::Id: {
        using T = MN::ThermostatRunningState::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ThermostatRunningState attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::ThermostatRunningState::Id);
        }
        break;
    }
        // type is SetpointChangeSourceEnum
    case MN::SetpointChangeSource::Id: {
        using T = MN::SetpointChangeSource::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "SetpointChangeSource attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::SetpointChangeSource::Id);
        }
        break;
    }
        // type is int16s
    case MN::SetpointChangeAmount::Id: {
        using T = MN::SetpointChangeAmount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "SetpointChangeAmount attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::SetpointChangeAmount::Id);
        }
        break;
    }
        // type is epoch_s
    case MN::SetpointChangeSourceTimestamp::Id: {
        using T = MN::SetpointChangeSourceTimestamp::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "SetpointChangeSourceTimestamp attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id,
                MN::SetpointChangeSourceTimestamp::Id);
        }
        break;
    }
        // type is int8u
    case MN::OccupiedSetback::Id: {
        using T = MN::OccupiedSetback::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "OccupiedSetback attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::OccupiedSetback::Id);
        }
        break;
    }
        // type is int8u
    case MN::OccupiedSetbackMin::Id: {
        using T = MN::OccupiedSetbackMin::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "OccupiedSetbackMin attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::OccupiedSetbackMin::Id);
        }
        break;
    }
        // type is int8u
    case MN::OccupiedSetbackMax::Id: {
        using T = MN::OccupiedSetbackMax::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "OccupiedSetbackMax attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::OccupiedSetbackMax::Id);
        }
        break;
    }
        // type is int8u
    case MN::UnoccupiedSetback::Id: {
        using T = MN::UnoccupiedSetback::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "UnoccupiedSetback attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::UnoccupiedSetback::Id);
        }
        break;
    }
        // type is int8u
    case MN::UnoccupiedSetbackMin::Id: {
        using T = MN::UnoccupiedSetbackMin::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "UnoccupiedSetbackMin attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::UnoccupiedSetbackMin::Id);
        }
        break;
    }
        // type is int8u
    case MN::UnoccupiedSetbackMax::Id: {
        using T = MN::UnoccupiedSetbackMax::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "UnoccupiedSetbackMax attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::UnoccupiedSetbackMax::Id);
        }
        break;
    }
        // type is int8u
    case MN::EmergencyHeatDelta::Id: {
        using T = MN::EmergencyHeatDelta::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "EmergencyHeatDelta attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::EmergencyHeatDelta::Id);
        }
        break;
    }
        // type is ACTypeEnum
    case MN::ACType::Id: {
        using T = MN::ACType::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ACType attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::ACType::Id);
        }
        break;
    }
        // type is int16u
    case MN::ACCapacity::Id: {
        using T = MN::ACCapacity::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ACCapacity attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::ACCapacity::Id);
        }
        break;
    }
        // type is ACRefrigerantTypeEnum
    case MN::ACRefrigerantType::Id: {
        using T = MN::ACRefrigerantType::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ACRefrigerantType attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::ACRefrigerantType::Id);
        }
        break;
    }
        // type is ACCompressorTypeEnum
    case MN::ACCompressorType::Id: {
        using T = MN::ACCompressorType::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ACCompressorType attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::ACCompressorType::Id);
        }
        break;
    }
        // type is ACErrorCodeBitmap
    case MN::ACErrorCode::Id: {
        using T = MN::ACErrorCode::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ACErrorCode attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::ACErrorCode::Id);
        }
        break;
    }
        // type is ACLouverPositionEnum
    case MN::ACLouverPosition::Id: {
        using T = MN::ACLouverPosition::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ACLouverPosition attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::ACLouverPosition::Id);
        }
        break;
    }
        // type is temperature
    case MN::ACCoilTemperature::Id: {
        using T = MN::ACCoilTemperature::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ACCoilTemperature attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::ACCoilTemperature::Id);
        }
        break;
    }
        // type is ACCapacityFormatEnum
    case MN::ACCapacityformat::Id: {
        using T = MN::ACCapacityformat::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ACCapacityformat attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::ACCapacityformat::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
FanControlAttributeAccess::Read(const ConcreteReadAttributePath& aPath, AttributeValueEncoder& aEncoder)
{
    namespace MN = chip::app::Clusters::FanControl::Attributes;
    namespace UN = unify::matter_bridge::FanControl::Attributes;
    if (aPath.mClusterId != Clusters::FanControl::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    // Do not handle Read for non-unify endpoints
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().read_attribute(aPath, aEncoder);
    }

    try {
        switch (aPath.mAttributeId) {
        case MN::FanMode::Id: { // type is FanModeEnum
            MN::FanMode::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::FanModeSequence::Id: { // type is FanModeSequenceEnum
            MN::FanModeSequence::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::PercentSetting::Id: { // type is percent
            MN::PercentSetting::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::PercentCurrent::Id: { // type is percent
            MN::PercentCurrent::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::SpeedMax::Id: { // type is int8u
            MN::SpeedMax::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::SpeedSetting::Id: { // type is int8u
            MN::SpeedSetting::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::SpeedCurrent::Id: { // type is int8u
            MN::SpeedCurrent::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RockSupport::Id: { // type is RockBitmap
            MN::RockSupport::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RockSetting::Id: { // type is RockBitmap
            MN::RockSetting::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::WindSupport::Id: { // type is WindBitmap
            MN::WindSupport::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::WindSetting::Id: { // type is WindBitmap
            MN::WindSetting::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AirflowDirection::Id: { // type is AirflowDirectionEnum
            MN::AirflowDirection::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        }
    } catch (const std::out_of_range& e) {
        sl_log_info(LOG_TAG,
            "The request attribute Path for endpoint [%i] is not found in the attribute state "
            "container: %s\n",
            atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR FanControlAttributeAccess::Write(const ConcreteDataAttributePath& aPath, AttributeValueDecoder& aDecoder)
{
    using namespace chip::app::Clusters::FanControl;

    if (aPath.mClusterId != Clusters::FanControl::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }
    nlohmann::json jsn;

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().write_attribute(aPath, aDecoder);
    }

    switch (aPath.mAttributeId) {
    case Attributes::FanMode::Id: {

        Attributes::FanMode::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["FanMode"] = to_json(value);
        break;
    }
        // FanModeSequence is not supported by UCL
        // PercentCurrent is not supported by UCL
        // SpeedMax is not supported by UCL
        // SpeedCurrent is not supported by UCL
        // RockSupport is not supported by UCL
        // WindSupport is not supported by UCL
        // GeneratedCommandList is not supported by UCL
        // AcceptedCommandList is not supported by UCL
        // EventList is not supported by UCL
        // AttributeList is not supported by UCL
        // FeatureMap is not supported by UCL
        // ClusterRevision is not supported by UCL
    }

    if (!jsn.empty()) {
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) + "/FanControl/Commands/WriteAttributes";
        std::string payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NO_MESSAGE_HANDLER;
}

void FanControlAttributeAccess::reported_updated(const bridged_endpoint* ep, const std::string& cluster,
    const std::string& attribute, const nlohmann::json& unify_value)
{
    namespace MN = chip::app::Clusters::FanControl::Attributes;
    namespace UN = unify::matter_bridge::FanControl::Attributes;

    auto cluster_id = m_dev_translator.get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::FanControl::Id)) {
        return;
    }

    // get attribute id
    auto attribute_id = m_dev_translator.get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value()) {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::FanControl::Id, attribute_id.value());
    switch (attribute_id.value()) {
    // type is FanModeEnum
    case MN::FanMode::Id: {
        using T = MN::FanMode::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "FanMode attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::FanControl::Id, MN::FanMode::Id);
        }
        break;
    }
        // type is FanModeSequenceEnum
    case MN::FanModeSequence::Id: {
        using T = MN::FanModeSequence::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "FanModeSequence attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::FanControl::Id, MN::FanModeSequence::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
ThermostatUserInterfaceConfigurationAttributeAccess::Read(const ConcreteReadAttributePath& aPath, AttributeValueEncoder& aEncoder)
{
    namespace MN = chip::app::Clusters::ThermostatUserInterfaceConfiguration::Attributes;
    namespace UN = unify::matter_bridge::ThermostatUserInterfaceConfiguration::Attributes;
    if (aPath.mClusterId != Clusters::ThermostatUserInterfaceConfiguration::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    // Do not handle Read for non-unify endpoints
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().read_attribute(aPath, aEncoder);
    }

    try {
        switch (aPath.mAttributeId) {
        case MN::TemperatureDisplayMode::Id: { // type is TemperatureDisplayModeEnum
            MN::TemperatureDisplayMode::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::KeypadLockout::Id: { // type is KeypadLockoutEnum
            MN::KeypadLockout::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ScheduleProgrammingVisibility::Id: { // type is ScheduleProgrammingVisibilityEnum
            MN::ScheduleProgrammingVisibility::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        }
    } catch (const std::out_of_range& e) {
        sl_log_info(LOG_TAG,
            "The request attribute Path for endpoint [%i] is not found in the attribute state "
            "container: %s\n",
            atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThermostatUserInterfaceConfigurationAttributeAccess::Write(const ConcreteDataAttributePath& aPath,
    AttributeValueDecoder& aDecoder)
{
    using namespace chip::app::Clusters::ThermostatUserInterfaceConfiguration;

    if (aPath.mClusterId != Clusters::ThermostatUserInterfaceConfiguration::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }
    nlohmann::json jsn;

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().write_attribute(aPath, aDecoder);
    }

    switch (aPath.mAttributeId) {
    case Attributes::TemperatureDisplayMode::Id: {

        Attributes::TemperatureDisplayMode::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["TemperatureDisplayMode"] = to_json(value);
        break;
    }
    case Attributes::KeypadLockout::Id: {

        Attributes::KeypadLockout::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["KeypadLockout"] = to_json(value);
        break;
    }
    case Attributes::ScheduleProgrammingVisibility::Id: {

        Attributes::ScheduleProgrammingVisibility::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ScheduleProgrammingVisibility"] = to_json(value);
        break;
    }
        // GeneratedCommandList is not supported by UCL
        // AcceptedCommandList is not supported by UCL
        // EventList is not supported by UCL
        // AttributeList is not supported by UCL
        // FeatureMap is not supported by UCL
        // ClusterRevision is not supported by UCL
    }

    if (!jsn.empty()) {
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) + "/ThermostatUserInterfaceConfiguration/Commands/WriteAttributes";
        std::string payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NO_MESSAGE_HANDLER;
}

void ThermostatUserInterfaceConfigurationAttributeAccess::reported_updated(const bridged_endpoint* ep, const std::string& cluster,
    const std::string& attribute,
    const nlohmann::json& unify_value)
{
    namespace MN = chip::app::Clusters::ThermostatUserInterfaceConfiguration::Attributes;
    namespace UN = unify::matter_bridge::ThermostatUserInterfaceConfiguration::Attributes;

    auto cluster_id = m_dev_translator.get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::ThermostatUserInterfaceConfiguration::Id)) {
        return;
    }

    // get attribute id
    auto attribute_id = m_dev_translator.get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value()) {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::ThermostatUserInterfaceConfiguration::Id, attribute_id.value());
    switch (attribute_id.value()) {
    // type is TemperatureDisplayModeEnum
    case MN::TemperatureDisplayMode::Id: {
        using T = MN::TemperatureDisplayMode::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "TemperatureDisplayMode attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThermostatUserInterfaceConfiguration::Id,
                MN::TemperatureDisplayMode::Id);
        }
        break;
    }
        // type is KeypadLockoutEnum
    case MN::KeypadLockout::Id: {
        using T = MN::KeypadLockout::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "KeypadLockout attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThermostatUserInterfaceConfiguration::Id,
                MN::KeypadLockout::Id);
        }
        break;
    }
        // type is ScheduleProgrammingVisibilityEnum
    case MN::ScheduleProgrammingVisibility::Id: {
        using T = MN::ScheduleProgrammingVisibility::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ScheduleProgrammingVisibility attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThermostatUserInterfaceConfiguration::Id,
                MN::ScheduleProgrammingVisibility::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
ColorControlAttributeAccess::Read(const ConcreteReadAttributePath& aPath, AttributeValueEncoder& aEncoder)
{
    namespace MN = chip::app::Clusters::ColorControl::Attributes;
    namespace UN = unify::matter_bridge::ColorControl::Attributes;
    if (aPath.mClusterId != Clusters::ColorControl::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    // Do not handle Read for non-unify endpoints
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().read_attribute(aPath, aEncoder);
    }

    try {
        switch (aPath.mAttributeId) {
        case MN::CurrentHue::Id: { // type is int8u
            MN::CurrentHue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::CurrentSaturation::Id: { // type is int8u
            MN::CurrentSaturation::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RemainingTime::Id: { // type is int16u
            MN::RemainingTime::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::CurrentX::Id: { // type is int16u
            MN::CurrentX::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::CurrentY::Id: { // type is int16u
            MN::CurrentY::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::DriftCompensation::Id: { // type is enum8
            MN::DriftCompensation::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::CompensationText::Id: { // type is char_string
            MN::CompensationText::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ColorTemperatureMireds::Id: { // type is int16u
            MN::ColorTemperatureMireds::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ColorMode::Id: { // type is enum8
            MN::ColorMode::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Options::Id: { // type is bitmap8
            MN::Options::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::NumberOfPrimaries::Id: { // type is int8u
            MN::NumberOfPrimaries::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Primary1X::Id: { // type is int16u
            MN::Primary1X::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Primary1Y::Id: { // type is int16u
            MN::Primary1Y::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Primary1Intensity::Id: { // type is int8u
            MN::Primary1Intensity::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Primary2X::Id: { // type is int16u
            MN::Primary2X::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Primary2Y::Id: { // type is int16u
            MN::Primary2Y::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Primary2Intensity::Id: { // type is int8u
            MN::Primary2Intensity::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Primary3X::Id: { // type is int16u
            MN::Primary3X::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Primary3Y::Id: { // type is int16u
            MN::Primary3Y::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Primary3Intensity::Id: { // type is int8u
            MN::Primary3Intensity::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Primary4X::Id: { // type is int16u
            MN::Primary4X::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Primary4Y::Id: { // type is int16u
            MN::Primary4Y::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Primary4Intensity::Id: { // type is int8u
            MN::Primary4Intensity::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Primary5X::Id: { // type is int16u
            MN::Primary5X::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Primary5Y::Id: { // type is int16u
            MN::Primary5Y::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Primary5Intensity::Id: { // type is int8u
            MN::Primary5Intensity::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Primary6X::Id: { // type is int16u
            MN::Primary6X::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Primary6Y::Id: { // type is int16u
            MN::Primary6Y::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Primary6Intensity::Id: { // type is int8u
            MN::Primary6Intensity::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::WhitePointX::Id: { // type is int16u
            MN::WhitePointX::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::WhitePointY::Id: { // type is int16u
            MN::WhitePointY::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ColorPointRX::Id: { // type is int16u
            MN::ColorPointRX::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ColorPointRY::Id: { // type is int16u
            MN::ColorPointRY::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ColorPointRIntensity::Id: { // type is int8u
            MN::ColorPointRIntensity::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ColorPointGX::Id: { // type is int16u
            MN::ColorPointGX::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ColorPointGY::Id: { // type is int16u
            MN::ColorPointGY::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ColorPointGIntensity::Id: { // type is int8u
            MN::ColorPointGIntensity::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ColorPointBX::Id: { // type is int16u
            MN::ColorPointBX::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ColorPointBY::Id: { // type is int16u
            MN::ColorPointBY::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ColorPointBIntensity::Id: { // type is int8u
            MN::ColorPointBIntensity::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::EnhancedCurrentHue::Id: { // type is int16u
            MN::EnhancedCurrentHue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::EnhancedColorMode::Id: { // type is enum8
            MN::EnhancedColorMode::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ColorLoopActive::Id: { // type is int8u
            MN::ColorLoopActive::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ColorLoopDirection::Id: { // type is int8u
            MN::ColorLoopDirection::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ColorLoopTime::Id: { // type is int16u
            MN::ColorLoopTime::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ColorLoopStartEnhancedHue::Id: { // type is int16u
            MN::ColorLoopStartEnhancedHue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ColorLoopStoredEnhancedHue::Id: { // type is int16u
            MN::ColorLoopStoredEnhancedHue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ColorCapabilities::Id: { // type is bitmap16
            MN::ColorCapabilities::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ColorTempPhysicalMinMireds::Id: { // type is int16u
            MN::ColorTempPhysicalMinMireds::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ColorTempPhysicalMaxMireds::Id: { // type is int16u
            MN::ColorTempPhysicalMaxMireds::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::CoupleColorTempToLevelMinMireds::Id: { // type is int16u
            MN::CoupleColorTempToLevelMinMireds::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::StartUpColorTemperatureMireds::Id: { // type is int16u
            MN::StartUpColorTemperatureMireds::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        }
    } catch (const std::out_of_range& e) {
        sl_log_info(LOG_TAG,
            "The request attribute Path for endpoint [%i] is not found in the attribute state "
            "container: %s\n",
            atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlAttributeAccess::Write(const ConcreteDataAttributePath& aPath, AttributeValueDecoder& aDecoder)
{
    using namespace chip::app::Clusters::ColorControl;

    if (aPath.mClusterId != Clusters::ColorControl::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }
    nlohmann::json jsn;

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().write_attribute(aPath, aDecoder);
    }

    switch (aPath.mAttributeId) {
    // CurrentHue is not supported by UCL
    // CurrentSaturation is not supported by UCL
    // RemainingTime is not supported by UCL
    // CurrentX is not supported by UCL
    // CurrentY is not supported by UCL
    // DriftCompensation is not supported by UCL
    // CompensationText is not supported by UCL
    // ColorTemperatureMireds is not supported by UCL
    // ColorMode is not supported by UCL
    case Attributes::Options::Id: {

        Attributes::Options::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["Options"] = to_json(value);
        break;
    }
    // NumberOfPrimaries is not supported by UCL
    // Primary1X is not supported by UCL
    // Primary1Y is not supported by UCL
    // Primary1Intensity is not supported by UCL
    // Primary2X is not supported by UCL
    // Primary2Y is not supported by UCL
    // Primary2Intensity is not supported by UCL
    // Primary3X is not supported by UCL
    // Primary3Y is not supported by UCL
    // Primary3Intensity is not supported by UCL
    // Primary4X is not supported by UCL
    // Primary4Y is not supported by UCL
    // Primary4Intensity is not supported by UCL
    // Primary5X is not supported by UCL
    // Primary5Y is not supported by UCL
    // Primary5Intensity is not supported by UCL
    // Primary6X is not supported by UCL
    // Primary6Y is not supported by UCL
    // Primary6Intensity is not supported by UCL
    case Attributes::WhitePointX::Id: {

        Attributes::WhitePointX::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["WhitePointX"] = to_json(value);
        break;
    }
    case Attributes::WhitePointY::Id: {

        Attributes::WhitePointY::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["WhitePointY"] = to_json(value);
        break;
    }
    case Attributes::ColorPointRX::Id: {

        Attributes::ColorPointRX::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ColorPointRX"] = to_json(value);
        break;
    }
    case Attributes::ColorPointRY::Id: {

        Attributes::ColorPointRY::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ColorPointRY"] = to_json(value);
        break;
    }
    case Attributes::ColorPointRIntensity::Id: {

        Attributes::ColorPointRIntensity::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ColorPointRIntensity"] = to_json(value);
        break;
    }
    case Attributes::ColorPointGX::Id: {

        Attributes::ColorPointGX::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ColorPointGX"] = to_json(value);
        break;
    }
    case Attributes::ColorPointGY::Id: {

        Attributes::ColorPointGY::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ColorPointGY"] = to_json(value);
        break;
    }
    case Attributes::ColorPointGIntensity::Id: {

        Attributes::ColorPointGIntensity::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ColorPointGIntensity"] = to_json(value);
        break;
    }
    case Attributes::ColorPointBX::Id: {

        Attributes::ColorPointBX::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ColorPointBX"] = to_json(value);
        break;
    }
    case Attributes::ColorPointBY::Id: {

        Attributes::ColorPointBY::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ColorPointBY"] = to_json(value);
        break;
    }
    case Attributes::ColorPointBIntensity::Id: {

        Attributes::ColorPointBIntensity::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ColorPointBIntensity"] = to_json(value);
        break;
    }
    // EnhancedCurrentHue is not supported by UCL
    // EnhancedColorMode is not supported by UCL
    // ColorLoopActive is not supported by UCL
    // ColorLoopDirection is not supported by UCL
    // ColorLoopTime is not supported by UCL
    // ColorLoopStartEnhancedHue is not supported by UCL
    // ColorLoopStoredEnhancedHue is not supported by UCL
    // ColorCapabilities is not supported by UCL
    // ColorTempPhysicalMinMireds is not supported by UCL
    // ColorTempPhysicalMaxMireds is not supported by UCL
    // CoupleColorTempToLevelMinMireds is not supported by UCL
    case Attributes::StartUpColorTemperatureMireds::Id: {

        Attributes::StartUpColorTemperatureMireds::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["StartUpColorTemperatureMireds"] = to_json(value);
        break;
    }
        // GeneratedCommandList is not supported by UCL
        // AcceptedCommandList is not supported by UCL
        // EventList is not supported by UCL
        // AttributeList is not supported by UCL
        // FeatureMap is not supported by UCL
        // ClusterRevision is not supported by UCL
    }

    if (!jsn.empty()) {
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) + "/ColorControl/Commands/WriteAttributes";
        std::string payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NO_MESSAGE_HANDLER;
}

void ColorControlAttributeAccess::reported_updated(const bridged_endpoint* ep, const std::string& cluster,
    const std::string& attribute, const nlohmann::json& unify_value)
{
    namespace MN = chip::app::Clusters::ColorControl::Attributes;
    namespace UN = unify::matter_bridge::ColorControl::Attributes;

    auto cluster_id = m_dev_translator.get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::ColorControl::Id)) {
        return;
    }

    // get attribute id
    auto attribute_id = m_dev_translator.get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value()) {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::ColorControl::Id, attribute_id.value());
    switch (attribute_id.value()) {
    // type is int8u
    case MN::CurrentHue::Id: {
        using T = MN::CurrentHue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "CurrentHue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::CurrentHue::Id);
        }
        break;
    }
        // type is int8u
    case MN::CurrentSaturation::Id: {
        using T = MN::CurrentSaturation::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "CurrentSaturation attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::CurrentSaturation::Id);
        }
        break;
    }
        // type is int16u
    case MN::RemainingTime::Id: {
        using T = MN::RemainingTime::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RemainingTime attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::RemainingTime::Id);
        }
        break;
    }
        // type is int16u
    case MN::CurrentX::Id: {
        using T = MN::CurrentX::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "CurrentX attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::CurrentX::Id);
        }
        break;
    }
        // type is int16u
    case MN::CurrentY::Id: {
        using T = MN::CurrentY::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "CurrentY attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::CurrentY::Id);
        }
        break;
    }
        // type is enum8
    case MN::DriftCompensation::Id: {
        using T = MN::DriftCompensation::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "DriftCompensation attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::DriftCompensation::Id);
        }
        break;
    }
        // type is char_string
    case MN::CompensationText::Id: {
        using T = MN::CompensationText::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "CompensationText attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::CompensationText::Id);
        }
        break;
    }
        // type is int16u
    case MN::ColorTemperatureMireds::Id: {
        using T = MN::ColorTemperatureMireds::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ColorTemperatureMireds attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id,
                MN::ColorTemperatureMireds::Id);
        }
        break;
    }
        // type is enum8
    case MN::ColorMode::Id: {
        using T = chip::app::Clusters::ColorControl::ColorMode;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ColorMode attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorMode::Id);
        }
        break;
    }
        // type is bitmap8
    case MN::Options::Id: {
        using T = MN::Options::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Options attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Options::Id);
        }
        break;
    }
        // type is int8u
    case MN::NumberOfPrimaries::Id: {
        using T = MN::NumberOfPrimaries::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "NumberOfPrimaries attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::NumberOfPrimaries::Id);
        }
        break;
    }
        // type is int16u
    case MN::Primary1X::Id: {
        using T = MN::Primary1X::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Primary1X attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary1X::Id);
        }
        break;
    }
        // type is int16u
    case MN::Primary1Y::Id: {
        using T = MN::Primary1Y::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Primary1Y attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary1Y::Id);
        }
        break;
    }
        // type is int8u
    case MN::Primary1Intensity::Id: {
        using T = MN::Primary1Intensity::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Primary1Intensity attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary1Intensity::Id);
        }
        break;
    }
        // type is int16u
    case MN::Primary2X::Id: {
        using T = MN::Primary2X::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Primary2X attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary2X::Id);
        }
        break;
    }
        // type is int16u
    case MN::Primary2Y::Id: {
        using T = MN::Primary2Y::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Primary2Y attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary2Y::Id);
        }
        break;
    }
        // type is int8u
    case MN::Primary2Intensity::Id: {
        using T = MN::Primary2Intensity::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Primary2Intensity attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary2Intensity::Id);
        }
        break;
    }
        // type is int16u
    case MN::Primary3X::Id: {
        using T = MN::Primary3X::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Primary3X attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary3X::Id);
        }
        break;
    }
        // type is int16u
    case MN::Primary3Y::Id: {
        using T = MN::Primary3Y::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Primary3Y attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary3Y::Id);
        }
        break;
    }
        // type is int8u
    case MN::Primary3Intensity::Id: {
        using T = MN::Primary3Intensity::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Primary3Intensity attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary3Intensity::Id);
        }
        break;
    }
        // type is int16u
    case MN::Primary4X::Id: {
        using T = MN::Primary4X::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Primary4X attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary4X::Id);
        }
        break;
    }
        // type is int16u
    case MN::Primary4Y::Id: {
        using T = MN::Primary4Y::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Primary4Y attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary4Y::Id);
        }
        break;
    }
        // type is int8u
    case MN::Primary4Intensity::Id: {
        using T = MN::Primary4Intensity::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Primary4Intensity attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary4Intensity::Id);
        }
        break;
    }
        // type is int16u
    case MN::Primary5X::Id: {
        using T = MN::Primary5X::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Primary5X attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary5X::Id);
        }
        break;
    }
        // type is int16u
    case MN::Primary5Y::Id: {
        using T = MN::Primary5Y::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Primary5Y attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary5Y::Id);
        }
        break;
    }
        // type is int8u
    case MN::Primary5Intensity::Id: {
        using T = MN::Primary5Intensity::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Primary5Intensity attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary5Intensity::Id);
        }
        break;
    }
        // type is int16u
    case MN::Primary6X::Id: {
        using T = MN::Primary6X::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Primary6X attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary6X::Id);
        }
        break;
    }
        // type is int16u
    case MN::Primary6Y::Id: {
        using T = MN::Primary6Y::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Primary6Y attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary6Y::Id);
        }
        break;
    }
        // type is int8u
    case MN::Primary6Intensity::Id: {
        using T = MN::Primary6Intensity::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Primary6Intensity attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary6Intensity::Id);
        }
        break;
    }
        // type is int16u
    case MN::WhitePointX::Id: {
        using T = MN::WhitePointX::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "WhitePointX attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::WhitePointX::Id);
        }
        break;
    }
        // type is int16u
    case MN::WhitePointY::Id: {
        using T = MN::WhitePointY::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "WhitePointY attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::WhitePointY::Id);
        }
        break;
    }
        // type is int16u
    case MN::ColorPointRX::Id: {
        using T = MN::ColorPointRX::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ColorPointRX attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorPointRX::Id);
        }
        break;
    }
        // type is int16u
    case MN::ColorPointRY::Id: {
        using T = MN::ColorPointRY::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ColorPointRY attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorPointRY::Id);
        }
        break;
    }
        // type is int8u
    case MN::ColorPointRIntensity::Id: {
        using T = MN::ColorPointRIntensity::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ColorPointRIntensity attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorPointRIntensity::Id);
        }
        break;
    }
        // type is int16u
    case MN::ColorPointGX::Id: {
        using T = MN::ColorPointGX::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ColorPointGX attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorPointGX::Id);
        }
        break;
    }
        // type is int16u
    case MN::ColorPointGY::Id: {
        using T = MN::ColorPointGY::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ColorPointGY attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorPointGY::Id);
        }
        break;
    }
        // type is int8u
    case MN::ColorPointGIntensity::Id: {
        using T = MN::ColorPointGIntensity::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ColorPointGIntensity attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorPointGIntensity::Id);
        }
        break;
    }
        // type is int16u
    case MN::ColorPointBX::Id: {
        using T = MN::ColorPointBX::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ColorPointBX attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorPointBX::Id);
        }
        break;
    }
        // type is int16u
    case MN::ColorPointBY::Id: {
        using T = MN::ColorPointBY::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ColorPointBY attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorPointBY::Id);
        }
        break;
    }
        // type is int8u
    case MN::ColorPointBIntensity::Id: {
        using T = MN::ColorPointBIntensity::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ColorPointBIntensity attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorPointBIntensity::Id);
        }
        break;
    }
        // type is int16u
    case MN::EnhancedCurrentHue::Id: {
        using T = MN::EnhancedCurrentHue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "EnhancedCurrentHue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::EnhancedCurrentHue::Id);
        }
        break;
    }
        // type is enum8
    case MN::EnhancedColorMode::Id: {
        using T = MN::EnhancedColorMode::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "EnhancedColorMode attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::EnhancedColorMode::Id);
        }
        break;
    }
        // type is int8u
    case MN::ColorLoopActive::Id: {
        using T = MN::ColorLoopActive::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ColorLoopActive attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorLoopActive::Id);
        }
        break;
    }
        // type is int8u
    case MN::ColorLoopDirection::Id: {
        using T = MN::ColorLoopDirection::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ColorLoopDirection attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorLoopDirection::Id);
        }
        break;
    }
        // type is int16u
    case MN::ColorLoopTime::Id: {
        using T = MN::ColorLoopTime::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ColorLoopTime attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorLoopTime::Id);
        }
        break;
    }
        // type is int16u
    case MN::ColorLoopStartEnhancedHue::Id: {
        using T = MN::ColorLoopStartEnhancedHue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ColorLoopStartEnhancedHue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id,
                MN::ColorLoopStartEnhancedHue::Id);
        }
        break;
    }
        // type is int16u
    case MN::ColorLoopStoredEnhancedHue::Id: {
        using T = MN::ColorLoopStoredEnhancedHue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ColorLoopStoredEnhancedHue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id,
                MN::ColorLoopStoredEnhancedHue::Id);
        }
        break;
    }
        // type is bitmap16
    case MN::ColorCapabilities::Id: {
        using T = chip::BitMask<chip::app::Clusters::ColorControl::ColorCapabilities>;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ColorCapabilities attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorCapabilities::Id);
        }
        break;
    }
        // type is int16u
    case MN::ColorTempPhysicalMinMireds::Id: {
        using T = MN::ColorTempPhysicalMinMireds::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ColorTempPhysicalMinMireds attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id,
                MN::ColorTempPhysicalMinMireds::Id);
        }
        break;
    }
        // type is int16u
    case MN::ColorTempPhysicalMaxMireds::Id: {
        using T = MN::ColorTempPhysicalMaxMireds::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ColorTempPhysicalMaxMireds attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id,
                MN::ColorTempPhysicalMaxMireds::Id);
        }
        break;
    }
        // type is int16u
    case MN::CoupleColorTempToLevelMinMireds::Id: {
        using T = MN::CoupleColorTempToLevelMinMireds::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "CoupleColorTempToLevelMinMireds attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id,
                MN::CoupleColorTempToLevelMinMireds::Id);
        }
        break;
    }
        // type is int16u
    case MN::StartUpColorTemperatureMireds::Id: {
        using T = MN::StartUpColorTemperatureMireds::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "StartUpColorTemperatureMireds attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id,
                MN::StartUpColorTemperatureMireds::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
IlluminanceMeasurementAttributeAccess::Read(const ConcreteReadAttributePath& aPath, AttributeValueEncoder& aEncoder)
{
    namespace MN = chip::app::Clusters::IlluminanceMeasurement::Attributes;
    namespace UN = unify::matter_bridge::IlluminanceMeasurement::Attributes;
    if (aPath.mClusterId != Clusters::IlluminanceMeasurement::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    // Do not handle Read for non-unify endpoints
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().read_attribute(aPath, aEncoder);
    }

    try {
        switch (aPath.mAttributeId) {
        case MN::MeasuredValue::Id: { // type is int16u
            MN::MeasuredValue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MinMeasuredValue::Id: { // type is int16u
            MN::MinMeasuredValue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MaxMeasuredValue::Id: { // type is int16u
            MN::MaxMeasuredValue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Tolerance::Id: { // type is int16u
            MN::Tolerance::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::LightSensorType::Id: { // type is LightSensorTypeEnum
            MN::LightSensorType::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        }
    } catch (const std::out_of_range& e) {
        sl_log_info(LOG_TAG,
            "The request attribute Path for endpoint [%i] is not found in the attribute state "
            "container: %s\n",
            atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR IlluminanceMeasurementAttributeAccess::Write(const ConcreteDataAttributePath& aPath, AttributeValueDecoder& aDecoder)
{
    using namespace chip::app::Clusters::IlluminanceMeasurement;

    if (aPath.mClusterId != Clusters::IlluminanceMeasurement::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }
    nlohmann::json jsn;

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().write_attribute(aPath, aDecoder);
    }

    switch (aPath.mAttributeId) {
        // MeasuredValue is not supported by UCL
        // MinMeasuredValue is not supported by UCL
        // MaxMeasuredValue is not supported by UCL
        // Tolerance is not supported by UCL
        // LightSensorType is not supported by UCL
        // GeneratedCommandList is not supported by UCL
        // AcceptedCommandList is not supported by UCL
        // EventList is not supported by UCL
        // AttributeList is not supported by UCL
        // FeatureMap is not supported by UCL
        // ClusterRevision is not supported by UCL
    }

    if (!jsn.empty()) {
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) + "/IlluminanceMeasurement/Commands/WriteAttributes";
        std::string payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NO_MESSAGE_HANDLER;
}

void IlluminanceMeasurementAttributeAccess::reported_updated(const bridged_endpoint* ep, const std::string& cluster,
    const std::string& attribute, const nlohmann::json& unify_value)
{
    namespace MN = chip::app::Clusters::IlluminanceMeasurement::Attributes;
    namespace UN = unify::matter_bridge::IlluminanceMeasurement::Attributes;

    auto cluster_id = m_dev_translator.get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::IlluminanceMeasurement::Id)) {
        return;
    }

    // get attribute id
    auto attribute_id = m_dev_translator.get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value()) {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::IlluminanceMeasurement::Id, attribute_id.value());
    switch (attribute_id.value()) {
    // type is int16u
    case MN::MeasuredValue::Id: {
        using T = MN::MeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MeasuredValue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::IlluminanceMeasurement::Id,
                MN::MeasuredValue::Id);
        }
        break;
    }
        // type is int16u
    case MN::MinMeasuredValue::Id: {
        using T = MN::MinMeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MinMeasuredValue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::IlluminanceMeasurement::Id,
                MN::MinMeasuredValue::Id);
        }
        break;
    }
        // type is int16u
    case MN::MaxMeasuredValue::Id: {
        using T = MN::MaxMeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MaxMeasuredValue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::IlluminanceMeasurement::Id,
                MN::MaxMeasuredValue::Id);
        }
        break;
    }
        // type is int16u
    case MN::Tolerance::Id: {
        using T = MN::Tolerance::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Tolerance attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::IlluminanceMeasurement::Id, MN::Tolerance::Id);
        }
        break;
    }
        // type is LightSensorTypeEnum
    case MN::LightSensorType::Id: {
        using T = MN::LightSensorType::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "LightSensorType attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::IlluminanceMeasurement::Id,
                MN::LightSensorType::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
TemperatureMeasurementAttributeAccess::Read(const ConcreteReadAttributePath& aPath, AttributeValueEncoder& aEncoder)
{
    namespace MN = chip::app::Clusters::TemperatureMeasurement::Attributes;
    namespace UN = unify::matter_bridge::TemperatureMeasurement::Attributes;
    if (aPath.mClusterId != Clusters::TemperatureMeasurement::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    // Do not handle Read for non-unify endpoints
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().read_attribute(aPath, aEncoder);
    }

    try {
        switch (aPath.mAttributeId) {
        case MN::MeasuredValue::Id: { // type is temperature
            MN::MeasuredValue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MinMeasuredValue::Id: { // type is temperature
            MN::MinMeasuredValue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MaxMeasuredValue::Id: { // type is temperature
            MN::MaxMeasuredValue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Tolerance::Id: { // type is int16u
            MN::Tolerance::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        }
    } catch (const std::out_of_range& e) {
        sl_log_info(LOG_TAG,
            "The request attribute Path for endpoint [%i] is not found in the attribute state "
            "container: %s\n",
            atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR TemperatureMeasurementAttributeAccess::Write(const ConcreteDataAttributePath& aPath, AttributeValueDecoder& aDecoder)
{
    using namespace chip::app::Clusters::TemperatureMeasurement;

    if (aPath.mClusterId != Clusters::TemperatureMeasurement::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }
    nlohmann::json jsn;

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().write_attribute(aPath, aDecoder);
    }

    switch (aPath.mAttributeId) {
        // MeasuredValue is not supported by UCL
        // MinMeasuredValue is not supported by UCL
        // MaxMeasuredValue is not supported by UCL
        // Tolerance is not supported by UCL
        // GeneratedCommandList is not supported by UCL
        // AcceptedCommandList is not supported by UCL
        // EventList is not supported by UCL
        // AttributeList is not supported by UCL
        // FeatureMap is not supported by UCL
        // ClusterRevision is not supported by UCL
    }

    if (!jsn.empty()) {
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) + "/TemperatureMeasurement/Commands/WriteAttributes";
        std::string payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NO_MESSAGE_HANDLER;
}

void TemperatureMeasurementAttributeAccess::reported_updated(const bridged_endpoint* ep, const std::string& cluster,
    const std::string& attribute, const nlohmann::json& unify_value)
{
    namespace MN = chip::app::Clusters::TemperatureMeasurement::Attributes;
    namespace UN = unify::matter_bridge::TemperatureMeasurement::Attributes;

    auto cluster_id = m_dev_translator.get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::TemperatureMeasurement::Id)) {
        return;
    }

    // get attribute id
    auto attribute_id = m_dev_translator.get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value()) {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::TemperatureMeasurement::Id, attribute_id.value());
    switch (attribute_id.value()) {
    // type is temperature
    case MN::MeasuredValue::Id: {
        using T = MN::MeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MeasuredValue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::TemperatureMeasurement::Id,
                MN::MeasuredValue::Id);
        }
        break;
    }
        // type is temperature
    case MN::MinMeasuredValue::Id: {
        using T = MN::MinMeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MinMeasuredValue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::TemperatureMeasurement::Id,
                MN::MinMeasuredValue::Id);
        }
        break;
    }
        // type is temperature
    case MN::MaxMeasuredValue::Id: {
        using T = MN::MaxMeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MaxMeasuredValue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::TemperatureMeasurement::Id,
                MN::MaxMeasuredValue::Id);
        }
        break;
    }
        // type is int16u
    case MN::Tolerance::Id: {
        using T = MN::Tolerance::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Tolerance attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::TemperatureMeasurement::Id, MN::Tolerance::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
PressureMeasurementAttributeAccess::Read(const ConcreteReadAttributePath& aPath, AttributeValueEncoder& aEncoder)
{
    namespace MN = chip::app::Clusters::PressureMeasurement::Attributes;
    namespace UN = unify::matter_bridge::PressureMeasurement::Attributes;
    if (aPath.mClusterId != Clusters::PressureMeasurement::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    // Do not handle Read for non-unify endpoints
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().read_attribute(aPath, aEncoder);
    }

    try {
        switch (aPath.mAttributeId) {
        case MN::MeasuredValue::Id: { // type is int16s
            MN::MeasuredValue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MinMeasuredValue::Id: { // type is int16s
            MN::MinMeasuredValue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MaxMeasuredValue::Id: { // type is int16s
            MN::MaxMeasuredValue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Tolerance::Id: { // type is int16u
            MN::Tolerance::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ScaledValue::Id: { // type is int16s
            MN::ScaledValue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MinScaledValue::Id: { // type is int16s
            MN::MinScaledValue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MaxScaledValue::Id: { // type is int16s
            MN::MaxScaledValue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ScaledTolerance::Id: { // type is int16u
            MN::ScaledTolerance::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Scale::Id: { // type is int8s
            MN::Scale::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        }
    } catch (const std::out_of_range& e) {
        sl_log_info(LOG_TAG,
            "The request attribute Path for endpoint [%i] is not found in the attribute state "
            "container: %s\n",
            atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR PressureMeasurementAttributeAccess::Write(const ConcreteDataAttributePath& aPath, AttributeValueDecoder& aDecoder)
{
    using namespace chip::app::Clusters::PressureMeasurement;

    if (aPath.mClusterId != Clusters::PressureMeasurement::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }
    nlohmann::json jsn;

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().write_attribute(aPath, aDecoder);
    }

    switch (aPath.mAttributeId) {
        // MeasuredValue is not supported by UCL
        // MinMeasuredValue is not supported by UCL
        // MaxMeasuredValue is not supported by UCL
        // Tolerance is not supported by UCL
        // ScaledValue is not supported by UCL
        // MinScaledValue is not supported by UCL
        // MaxScaledValue is not supported by UCL
        // ScaledTolerance is not supported by UCL
        // Scale is not supported by UCL
        // GeneratedCommandList is not supported by UCL
        // AcceptedCommandList is not supported by UCL
        // EventList is not supported by UCL
        // AttributeList is not supported by UCL
        // FeatureMap is not supported by UCL
        // ClusterRevision is not supported by UCL
    }

    if (!jsn.empty()) {
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) + "/PressureMeasurement/Commands/WriteAttributes";
        std::string payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NO_MESSAGE_HANDLER;
}

void PressureMeasurementAttributeAccess::reported_updated(const bridged_endpoint* ep, const std::string& cluster,
    const std::string& attribute, const nlohmann::json& unify_value)
{
    namespace MN = chip::app::Clusters::PressureMeasurement::Attributes;
    namespace UN = unify::matter_bridge::PressureMeasurement::Attributes;

    auto cluster_id = m_dev_translator.get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::PressureMeasurement::Id)) {
        return;
    }

    // get attribute id
    auto attribute_id = m_dev_translator.get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value()) {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::PressureMeasurement::Id, attribute_id.value());
    switch (attribute_id.value()) {
    // type is int16s
    case MN::MeasuredValue::Id: {
        using T = MN::MeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MeasuredValue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PressureMeasurement::Id, MN::MeasuredValue::Id);
        }
        break;
    }
        // type is int16s
    case MN::MinMeasuredValue::Id: {
        using T = MN::MinMeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MinMeasuredValue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PressureMeasurement::Id,
                MN::MinMeasuredValue::Id);
        }
        break;
    }
        // type is int16s
    case MN::MaxMeasuredValue::Id: {
        using T = MN::MaxMeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MaxMeasuredValue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PressureMeasurement::Id,
                MN::MaxMeasuredValue::Id);
        }
        break;
    }
        // type is int16u
    case MN::Tolerance::Id: {
        using T = MN::Tolerance::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Tolerance attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PressureMeasurement::Id, MN::Tolerance::Id);
        }
        break;
    }
        // type is int16s
    case MN::ScaledValue::Id: {
        using T = MN::ScaledValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ScaledValue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PressureMeasurement::Id, MN::ScaledValue::Id);
        }
        break;
    }
        // type is int16s
    case MN::MinScaledValue::Id: {
        using T = MN::MinScaledValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MinScaledValue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PressureMeasurement::Id, MN::MinScaledValue::Id);
        }
        break;
    }
        // type is int16s
    case MN::MaxScaledValue::Id: {
        using T = MN::MaxScaledValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MaxScaledValue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PressureMeasurement::Id, MN::MaxScaledValue::Id);
        }
        break;
    }
        // type is int16u
    case MN::ScaledTolerance::Id: {
        using T = MN::ScaledTolerance::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ScaledTolerance attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PressureMeasurement::Id,
                MN::ScaledTolerance::Id);
        }
        break;
    }
        // type is int8s
    case MN::Scale::Id: {
        using T = MN::Scale::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Scale attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PressureMeasurement::Id, MN::Scale::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
FlowMeasurementAttributeAccess::Read(const ConcreteReadAttributePath& aPath, AttributeValueEncoder& aEncoder)
{
    namespace MN = chip::app::Clusters::FlowMeasurement::Attributes;
    namespace UN = unify::matter_bridge::FlowMeasurement::Attributes;
    if (aPath.mClusterId != Clusters::FlowMeasurement::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    // Do not handle Read for non-unify endpoints
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().read_attribute(aPath, aEncoder);
    }

    try {
        switch (aPath.mAttributeId) {
        case MN::MeasuredValue::Id: { // type is int16u
            MN::MeasuredValue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MinMeasuredValue::Id: { // type is int16u
            MN::MinMeasuredValue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MaxMeasuredValue::Id: { // type is int16u
            MN::MaxMeasuredValue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Tolerance::Id: { // type is int16u
            MN::Tolerance::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        }
    } catch (const std::out_of_range& e) {
        sl_log_info(LOG_TAG,
            "The request attribute Path for endpoint [%i] is not found in the attribute state "
            "container: %s\n",
            atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR FlowMeasurementAttributeAccess::Write(const ConcreteDataAttributePath& aPath, AttributeValueDecoder& aDecoder)
{
    using namespace chip::app::Clusters::FlowMeasurement;

    if (aPath.mClusterId != Clusters::FlowMeasurement::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }
    nlohmann::json jsn;

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().write_attribute(aPath, aDecoder);
    }

    switch (aPath.mAttributeId) {
        // MeasuredValue is not supported by UCL
        // MinMeasuredValue is not supported by UCL
        // MaxMeasuredValue is not supported by UCL
        // Tolerance is not supported by UCL
        // GeneratedCommandList is not supported by UCL
        // AcceptedCommandList is not supported by UCL
        // EventList is not supported by UCL
        // AttributeList is not supported by UCL
        // FeatureMap is not supported by UCL
        // ClusterRevision is not supported by UCL
    }

    if (!jsn.empty()) {
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) + "/FlowMeasurement/Commands/WriteAttributes";
        std::string payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NO_MESSAGE_HANDLER;
}

void FlowMeasurementAttributeAccess::reported_updated(const bridged_endpoint* ep, const std::string& cluster,
    const std::string& attribute, const nlohmann::json& unify_value)
{
    namespace MN = chip::app::Clusters::FlowMeasurement::Attributes;
    namespace UN = unify::matter_bridge::FlowMeasurement::Attributes;

    auto cluster_id = m_dev_translator.get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::FlowMeasurement::Id)) {
        return;
    }

    // get attribute id
    auto attribute_id = m_dev_translator.get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value()) {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::FlowMeasurement::Id, attribute_id.value());
    switch (attribute_id.value()) {
    // type is int16u
    case MN::MeasuredValue::Id: {
        using T = MN::MeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MeasuredValue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::FlowMeasurement::Id, MN::MeasuredValue::Id);
        }
        break;
    }
        // type is int16u
    case MN::MinMeasuredValue::Id: {
        using T = MN::MinMeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MinMeasuredValue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::FlowMeasurement::Id, MN::MinMeasuredValue::Id);
        }
        break;
    }
        // type is int16u
    case MN::MaxMeasuredValue::Id: {
        using T = MN::MaxMeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MaxMeasuredValue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::FlowMeasurement::Id, MN::MaxMeasuredValue::Id);
        }
        break;
    }
        // type is int16u
    case MN::Tolerance::Id: {
        using T = MN::Tolerance::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Tolerance attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::FlowMeasurement::Id, MN::Tolerance::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
RelativeHumidityMeasurementAttributeAccess::Read(const ConcreteReadAttributePath& aPath, AttributeValueEncoder& aEncoder)
{
    namespace MN = chip::app::Clusters::RelativeHumidityMeasurement::Attributes;
    namespace UN = unify::matter_bridge::RelativeHumidityMeasurement::Attributes;
    if (aPath.mClusterId != Clusters::RelativeHumidityMeasurement::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    // Do not handle Read for non-unify endpoints
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().read_attribute(aPath, aEncoder);
    }

    try {
        switch (aPath.mAttributeId) {
        case MN::MeasuredValue::Id: { // type is int16u
            MN::MeasuredValue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MinMeasuredValue::Id: { // type is int16u
            MN::MinMeasuredValue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MaxMeasuredValue::Id: { // type is int16u
            MN::MaxMeasuredValue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Tolerance::Id: { // type is int16u
            MN::Tolerance::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        }
    } catch (const std::out_of_range& e) {
        sl_log_info(LOG_TAG,
            "The request attribute Path for endpoint [%i] is not found in the attribute state "
            "container: %s\n",
            atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR RelativeHumidityMeasurementAttributeAccess::Write(const ConcreteDataAttributePath& aPath,
    AttributeValueDecoder& aDecoder)
{
    using namespace chip::app::Clusters::RelativeHumidityMeasurement;

    if (aPath.mClusterId != Clusters::RelativeHumidityMeasurement::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }
    nlohmann::json jsn;

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().write_attribute(aPath, aDecoder);
    }

    switch (aPath.mAttributeId) {
        // MeasuredValue is not supported by UCL
        // MinMeasuredValue is not supported by UCL
        // MaxMeasuredValue is not supported by UCL
        // Tolerance is not supported by UCL
        // GeneratedCommandList is not supported by UCL
        // AcceptedCommandList is not supported by UCL
        // EventList is not supported by UCL
        // AttributeList is not supported by UCL
        // FeatureMap is not supported by UCL
        // ClusterRevision is not supported by UCL
    }

    if (!jsn.empty()) {
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) + "/RelativityHumidity/Commands/WriteAttributes";
        std::string payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NO_MESSAGE_HANDLER;
}

void RelativeHumidityMeasurementAttributeAccess::reported_updated(const bridged_endpoint* ep, const std::string& cluster,
    const std::string& attribute, const nlohmann::json& unify_value)
{
    namespace MN = chip::app::Clusters::RelativeHumidityMeasurement::Attributes;
    namespace UN = unify::matter_bridge::RelativeHumidityMeasurement::Attributes;

    auto cluster_id = m_dev_translator.get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::RelativeHumidityMeasurement::Id)) {
        return;
    }

    // get attribute id
    auto attribute_id = m_dev_translator.get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value()) {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::RelativeHumidityMeasurement::Id, attribute_id.value());
    switch (attribute_id.value()) {
    // type is int16u
    case MN::MeasuredValue::Id: {
        using T = MN::MeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MeasuredValue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::RelativeHumidityMeasurement::Id,
                MN::MeasuredValue::Id);
        }
        break;
    }
        // type is int16u
    case MN::MinMeasuredValue::Id: {
        using T = MN::MinMeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MinMeasuredValue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::RelativeHumidityMeasurement::Id,
                MN::MinMeasuredValue::Id);
        }
        break;
    }
        // type is int16u
    case MN::MaxMeasuredValue::Id: {
        using T = MN::MaxMeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MaxMeasuredValue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::RelativeHumidityMeasurement::Id,
                MN::MaxMeasuredValue::Id);
        }
        break;
    }
        // type is int16u
    case MN::Tolerance::Id: {
        using T = MN::Tolerance::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Tolerance attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::RelativeHumidityMeasurement::Id,
                MN::Tolerance::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
OccupancySensingAttributeAccess::Read(const ConcreteReadAttributePath& aPath, AttributeValueEncoder& aEncoder)
{
    namespace MN = chip::app::Clusters::OccupancySensing::Attributes;
    namespace UN = unify::matter_bridge::OccupancySensing::Attributes;
    if (aPath.mClusterId != Clusters::OccupancySensing::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    // Do not handle Read for non-unify endpoints
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().read_attribute(aPath, aEncoder);
    }

    try {
        switch (aPath.mAttributeId) {
        case MN::Occupancy::Id: { // type is OccupancyBitmap
            MN::Occupancy::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::OccupancySensorType::Id: { // type is OccupancySensorTypeEnum
            MN::OccupancySensorType::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::OccupancySensorTypeBitmap::Id: { // type is OccupancySensorTypeBitmap
            MN::OccupancySensorTypeBitmap::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::PIROccupiedToUnoccupiedDelay::Id: { // type is int16u
            MN::PIROccupiedToUnoccupiedDelay::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::PIRUnoccupiedToOccupiedDelay::Id: { // type is int16u
            MN::PIRUnoccupiedToOccupiedDelay::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::PIRUnoccupiedToOccupiedThreshold::Id: { // type is int8u
            MN::PIRUnoccupiedToOccupiedThreshold::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::UltrasonicOccupiedToUnoccupiedDelay::Id: { // type is int16u
            MN::UltrasonicOccupiedToUnoccupiedDelay::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::UltrasonicUnoccupiedToOccupiedDelay::Id: { // type is int16u
            MN::UltrasonicUnoccupiedToOccupiedDelay::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::UltrasonicUnoccupiedToOccupiedThreshold::Id: { // type is int8u
            MN::UltrasonicUnoccupiedToOccupiedThreshold::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::PhysicalContactOccupiedToUnoccupiedDelay::Id: { // type is int16u
            MN::PhysicalContactOccupiedToUnoccupiedDelay::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::PhysicalContactUnoccupiedToOccupiedDelay::Id: { // type is int16u
            MN::PhysicalContactUnoccupiedToOccupiedDelay::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::PhysicalContactUnoccupiedToOccupiedThreshold::Id: { // type is int8u
            MN::PhysicalContactUnoccupiedToOccupiedThreshold::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        }
    } catch (const std::out_of_range& e) {
        sl_log_info(LOG_TAG,
            "The request attribute Path for endpoint [%i] is not found in the attribute state "
            "container: %s\n",
            atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR OccupancySensingAttributeAccess::Write(const ConcreteDataAttributePath& aPath, AttributeValueDecoder& aDecoder)
{
    using namespace chip::app::Clusters::OccupancySensing;

    if (aPath.mClusterId != Clusters::OccupancySensing::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }
    nlohmann::json jsn;

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().write_attribute(aPath, aDecoder);
    }

    switch (aPath.mAttributeId) {
    // Occupancy is not supported by UCL
    // OccupancySensorType is not supported by UCL
    // OccupancySensorTypeBitmap is not supported by UCL
    case Attributes::PIROccupiedToUnoccupiedDelay::Id: {

        Attributes::PIROccupiedToUnoccupiedDelay::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["PIROccupiedToUnoccupiedDelay"] = to_json(value);
        break;
    }
    case Attributes::PIRUnoccupiedToOccupiedDelay::Id: {

        Attributes::PIRUnoccupiedToOccupiedDelay::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["PIRUnoccupiedToOccupiedDelay"] = to_json(value);
        break;
    }
    case Attributes::PIRUnoccupiedToOccupiedThreshold::Id: {

        Attributes::PIRUnoccupiedToOccupiedThreshold::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["PIRUnoccupiedToOccupiedThreshold"] = to_json(value);
        break;
    }
    case Attributes::UltrasonicOccupiedToUnoccupiedDelay::Id: {

        Attributes::UltrasonicOccupiedToUnoccupiedDelay::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["UltrasonicOccupiedToUnoccupiedDelay"] = to_json(value);
        break;
    }
    case Attributes::UltrasonicUnoccupiedToOccupiedDelay::Id: {

        Attributes::UltrasonicUnoccupiedToOccupiedDelay::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["UltrasonicUnoccupiedToOccupiedDelay"] = to_json(value);
        break;
    }
    case Attributes::UltrasonicUnoccupiedToOccupiedThreshold::Id: {

        Attributes::UltrasonicUnoccupiedToOccupiedThreshold::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["UltrasonicUnoccupiedToOccupiedThreshold"] = to_json(value);
        break;
    }
    case Attributes::PhysicalContactOccupiedToUnoccupiedDelay::Id: {

        Attributes::PhysicalContactOccupiedToUnoccupiedDelay::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["PhysicalContactOccupiedToUnoccupiedDelay"] = to_json(value);
        break;
    }
    case Attributes::PhysicalContactUnoccupiedToOccupiedDelay::Id: {

        Attributes::PhysicalContactUnoccupiedToOccupiedDelay::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["PhysicalContactUnoccupiedToOccupiedDelay"] = to_json(value);
        break;
    }
    case Attributes::PhysicalContactUnoccupiedToOccupiedThreshold::Id: {

        Attributes::PhysicalContactUnoccupiedToOccupiedThreshold::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["PhysicalContactUnoccupiedToOccupiedThreshold"] = to_json(value);
        break;
    }
        // GeneratedCommandList is not supported by UCL
        // AcceptedCommandList is not supported by UCL
        // EventList is not supported by UCL
        // AttributeList is not supported by UCL
        // FeatureMap is not supported by UCL
        // ClusterRevision is not supported by UCL
    }

    if (!jsn.empty()) {
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) + "/OccupancySensing/Commands/WriteAttributes";
        std::string payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NO_MESSAGE_HANDLER;
}

void OccupancySensingAttributeAccess::reported_updated(const bridged_endpoint* ep, const std::string& cluster,
    const std::string& attribute, const nlohmann::json& unify_value)
{
    namespace MN = chip::app::Clusters::OccupancySensing::Attributes;
    namespace UN = unify::matter_bridge::OccupancySensing::Attributes;

    auto cluster_id = m_dev_translator.get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::OccupancySensing::Id)) {
        return;
    }

    // get attribute id
    auto attribute_id = m_dev_translator.get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value()) {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::OccupancySensing::Id, attribute_id.value());
    switch (attribute_id.value()) {
    // type is OccupancyBitmap
    case MN::Occupancy::Id: {
        using T = MN::Occupancy::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Occupancy attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OccupancySensing::Id, MN::Occupancy::Id);
        }
        break;
    }
        // type is OccupancySensorTypeEnum
    case MN::OccupancySensorType::Id: {
        using T = MN::OccupancySensorType::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "OccupancySensorType attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OccupancySensing::Id,
                MN::OccupancySensorType::Id);
        }
        break;
    }
        // type is OccupancySensorTypeBitmap
    case MN::OccupancySensorTypeBitmap::Id: {
        using T = MN::OccupancySensorTypeBitmap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "OccupancySensorTypeBitmap attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OccupancySensing::Id,
                MN::OccupancySensorTypeBitmap::Id);
        }
        break;
    }
        // type is int16u
    case MN::PIROccupiedToUnoccupiedDelay::Id: {
        using T = MN::PIROccupiedToUnoccupiedDelay::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "PIROccupiedToUnoccupiedDelay attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OccupancySensing::Id,
                MN::PIROccupiedToUnoccupiedDelay::Id);
        }
        break;
    }
        // type is int16u
    case MN::PIRUnoccupiedToOccupiedDelay::Id: {
        using T = MN::PIRUnoccupiedToOccupiedDelay::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "PIRUnoccupiedToOccupiedDelay attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OccupancySensing::Id,
                MN::PIRUnoccupiedToOccupiedDelay::Id);
        }
        break;
    }
        // type is int8u
    case MN::PIRUnoccupiedToOccupiedThreshold::Id: {
        using T = MN::PIRUnoccupiedToOccupiedThreshold::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "PIRUnoccupiedToOccupiedThreshold attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OccupancySensing::Id,
                MN::PIRUnoccupiedToOccupiedThreshold::Id);
        }
        break;
    }
        // type is int16u
    case MN::UltrasonicOccupiedToUnoccupiedDelay::Id: {
        using T = MN::UltrasonicOccupiedToUnoccupiedDelay::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "UltrasonicOccupiedToUnoccupiedDelay attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OccupancySensing::Id,
                MN::UltrasonicOccupiedToUnoccupiedDelay::Id);
        }
        break;
    }
        // type is int16u
    case MN::UltrasonicUnoccupiedToOccupiedDelay::Id: {
        using T = MN::UltrasonicUnoccupiedToOccupiedDelay::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "UltrasonicUnoccupiedToOccupiedDelay attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OccupancySensing::Id,
                MN::UltrasonicUnoccupiedToOccupiedDelay::Id);
        }
        break;
    }
        // type is int8u
    case MN::UltrasonicUnoccupiedToOccupiedThreshold::Id: {
        using T = MN::UltrasonicUnoccupiedToOccupiedThreshold::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "UltrasonicUnoccupiedToOccupiedThreshold attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OccupancySensing::Id,
                MN::UltrasonicUnoccupiedToOccupiedThreshold::Id);
        }
        break;
    }
        // type is int16u
    case MN::PhysicalContactOccupiedToUnoccupiedDelay::Id: {
        using T = MN::PhysicalContactOccupiedToUnoccupiedDelay::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "PhysicalContactOccupiedToUnoccupiedDelay attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OccupancySensing::Id,
                MN::PhysicalContactOccupiedToUnoccupiedDelay::Id);
        }
        break;
    }
        // type is int16u
    case MN::PhysicalContactUnoccupiedToOccupiedDelay::Id: {
        using T = MN::PhysicalContactUnoccupiedToOccupiedDelay::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "PhysicalContactUnoccupiedToOccupiedDelay attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OccupancySensing::Id,
                MN::PhysicalContactUnoccupiedToOccupiedDelay::Id);
        }
        break;
    }
        // type is int8u
    case MN::PhysicalContactUnoccupiedToOccupiedThreshold::Id: {
        using T = MN::PhysicalContactUnoccupiedToOccupiedThreshold::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "PhysicalContactUnoccupiedToOccupiedThreshold attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OccupancySensing::Id,
                MN::PhysicalContactUnoccupiedToOccupiedThreshold::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
ElectricalMeasurementAttributeAccess::Read(const ConcreteReadAttributePath& aPath, AttributeValueEncoder& aEncoder)
{
    namespace MN = chip::app::Clusters::ElectricalMeasurement::Attributes;
    namespace UN = unify::matter_bridge::ElectricalMeasurement::Attributes;
    if (aPath.mClusterId != Clusters::ElectricalMeasurement::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    // Do not handle Read for non-unify endpoints
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().read_attribute(aPath, aEncoder);
    }

    try {
        switch (aPath.mAttributeId) {
        case MN::MeasurementType::Id: { // type is bitmap32
            MN::MeasurementType::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::DcVoltage::Id: { // type is int16s
            MN::DcVoltage::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::DcVoltageMin::Id: { // type is int16s
            MN::DcVoltageMin::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::DcVoltageMax::Id: { // type is int16s
            MN::DcVoltageMax::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::DcCurrent::Id: { // type is int16s
            MN::DcCurrent::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::DcCurrentMin::Id: { // type is int16s
            MN::DcCurrentMin::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::DcCurrentMax::Id: { // type is int16s
            MN::DcCurrentMax::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::DcPower::Id: { // type is int16s
            MN::DcPower::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::DcPowerMin::Id: { // type is int16s
            MN::DcPowerMin::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::DcPowerMax::Id: { // type is int16s
            MN::DcPowerMax::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::DcVoltageMultiplier::Id: { // type is int16u
            MN::DcVoltageMultiplier::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::DcVoltageDivisor::Id: { // type is int16u
            MN::DcVoltageDivisor::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::DcCurrentMultiplier::Id: { // type is int16u
            MN::DcCurrentMultiplier::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::DcCurrentDivisor::Id: { // type is int16u
            MN::DcCurrentDivisor::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::DcPowerMultiplier::Id: { // type is int16u
            MN::DcPowerMultiplier::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::DcPowerDivisor::Id: { // type is int16u
            MN::DcPowerDivisor::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AcFrequency::Id: { // type is int16u
            MN::AcFrequency::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AcFrequencyMin::Id: { // type is int16u
            MN::AcFrequencyMin::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AcFrequencyMax::Id: { // type is int16u
            MN::AcFrequencyMax::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::NeutralCurrent::Id: { // type is int16u
            MN::NeutralCurrent::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::TotalActivePower::Id: { // type is int32s
            MN::TotalActivePower::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::TotalReactivePower::Id: { // type is int32s
            MN::TotalReactivePower::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::TotalApparentPower::Id: { // type is int32u
            MN::TotalApparentPower::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Measured1stHarmonicCurrent::Id: { // type is int16s
            MN::Measured1stHarmonicCurrent::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Measured3rdHarmonicCurrent::Id: { // type is int16s
            MN::Measured3rdHarmonicCurrent::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Measured5thHarmonicCurrent::Id: { // type is int16s
            MN::Measured5thHarmonicCurrent::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Measured7thHarmonicCurrent::Id: { // type is int16s
            MN::Measured7thHarmonicCurrent::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Measured9thHarmonicCurrent::Id: { // type is int16s
            MN::Measured9thHarmonicCurrent::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Measured11thHarmonicCurrent::Id: { // type is int16s
            MN::Measured11thHarmonicCurrent::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MeasuredPhase1stHarmonicCurrent::Id: { // type is int16s
            MN::MeasuredPhase1stHarmonicCurrent::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MeasuredPhase3rdHarmonicCurrent::Id: { // type is int16s
            MN::MeasuredPhase3rdHarmonicCurrent::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MeasuredPhase5thHarmonicCurrent::Id: { // type is int16s
            MN::MeasuredPhase5thHarmonicCurrent::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MeasuredPhase7thHarmonicCurrent::Id: { // type is int16s
            MN::MeasuredPhase7thHarmonicCurrent::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MeasuredPhase9thHarmonicCurrent::Id: { // type is int16s
            MN::MeasuredPhase9thHarmonicCurrent::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MeasuredPhase11thHarmonicCurrent::Id: { // type is int16s
            MN::MeasuredPhase11thHarmonicCurrent::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AcFrequencyMultiplier::Id: { // type is int16u
            MN::AcFrequencyMultiplier::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AcFrequencyDivisor::Id: { // type is int16u
            MN::AcFrequencyDivisor::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::PowerMultiplier::Id: { // type is int32u
            MN::PowerMultiplier::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::PowerDivisor::Id: { // type is int32u
            MN::PowerDivisor::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::HarmonicCurrentMultiplier::Id: { // type is int8s
            MN::HarmonicCurrentMultiplier::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::PhaseHarmonicCurrentMultiplier::Id: { // type is int8s
            MN::PhaseHarmonicCurrentMultiplier::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::InstantaneousVoltage::Id: { // type is int16s
            MN::InstantaneousVoltage::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::InstantaneousLineCurrent::Id: { // type is int16u
            MN::InstantaneousLineCurrent::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::InstantaneousActiveCurrent::Id: { // type is int16s
            MN::InstantaneousActiveCurrent::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::InstantaneousReactiveCurrent::Id: { // type is int16s
            MN::InstantaneousReactiveCurrent::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::InstantaneousPower::Id: { // type is int16s
            MN::InstantaneousPower::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RmsVoltage::Id: { // type is int16u
            MN::RmsVoltage::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RmsVoltageMin::Id: { // type is int16u
            MN::RmsVoltageMin::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RmsVoltageMax::Id: { // type is int16u
            MN::RmsVoltageMax::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RmsCurrent::Id: { // type is int16u
            MN::RmsCurrent::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RmsCurrentMin::Id: { // type is int16u
            MN::RmsCurrentMin::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RmsCurrentMax::Id: { // type is int16u
            MN::RmsCurrentMax::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ActivePower::Id: { // type is int16s
            MN::ActivePower::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ActivePowerMin::Id: { // type is int16s
            MN::ActivePowerMin::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ActivePowerMax::Id: { // type is int16s
            MN::ActivePowerMax::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ReactivePower::Id: { // type is int16s
            MN::ReactivePower::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ApparentPower::Id: { // type is int16u
            MN::ApparentPower::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::PowerFactor::Id: { // type is int8s
            MN::PowerFactor::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AverageRmsVoltageMeasurementPeriod::Id: { // type is int16u
            MN::AverageRmsVoltageMeasurementPeriod::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AverageRmsUnderVoltageCounter::Id: { // type is int16u
            MN::AverageRmsUnderVoltageCounter::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RmsExtremeOverVoltagePeriod::Id: { // type is int16u
            MN::RmsExtremeOverVoltagePeriod::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RmsExtremeUnderVoltagePeriod::Id: { // type is int16u
            MN::RmsExtremeUnderVoltagePeriod::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RmsVoltageSagPeriod::Id: { // type is int16u
            MN::RmsVoltageSagPeriod::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RmsVoltageSwellPeriod::Id: { // type is int16u
            MN::RmsVoltageSwellPeriod::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AcVoltageMultiplier::Id: { // type is int16u
            MN::AcVoltageMultiplier::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AcVoltageDivisor::Id: { // type is int16u
            MN::AcVoltageDivisor::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AcCurrentMultiplier::Id: { // type is int16u
            MN::AcCurrentMultiplier::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AcCurrentDivisor::Id: { // type is int16u
            MN::AcCurrentDivisor::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AcPowerMultiplier::Id: { // type is int16u
            MN::AcPowerMultiplier::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AcPowerDivisor::Id: { // type is int16u
            MN::AcPowerDivisor::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::OverloadAlarmsMask::Id: { // type is bitmap8
            MN::OverloadAlarmsMask::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::VoltageOverload::Id: { // type is int16s
            MN::VoltageOverload::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::CurrentOverload::Id: { // type is int16s
            MN::CurrentOverload::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AcOverloadAlarmsMask::Id: { // type is bitmap16
            MN::AcOverloadAlarmsMask::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AcVoltageOverload::Id: { // type is int16s
            MN::AcVoltageOverload::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AcCurrentOverload::Id: { // type is int16s
            MN::AcCurrentOverload::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AcActivePowerOverload::Id: { // type is int16s
            MN::AcActivePowerOverload::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AcReactivePowerOverload::Id: { // type is int16s
            MN::AcReactivePowerOverload::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AverageRmsOverVoltage::Id: { // type is int16s
            MN::AverageRmsOverVoltage::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AverageRmsUnderVoltage::Id: { // type is int16s
            MN::AverageRmsUnderVoltage::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RmsExtremeOverVoltage::Id: { // type is int16s
            MN::RmsExtremeOverVoltage::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RmsExtremeUnderVoltage::Id: { // type is int16s
            MN::RmsExtremeUnderVoltage::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RmsVoltageSag::Id: { // type is int16s
            MN::RmsVoltageSag::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RmsVoltageSwell::Id: { // type is int16s
            MN::RmsVoltageSwell::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::LineCurrentPhaseB::Id: { // type is int16u
            MN::LineCurrentPhaseB::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ActiveCurrentPhaseB::Id: { // type is int16s
            MN::ActiveCurrentPhaseB::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ReactiveCurrentPhaseB::Id: { // type is int16s
            MN::ReactiveCurrentPhaseB::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RmsVoltagePhaseB::Id: { // type is int16u
            MN::RmsVoltagePhaseB::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RmsVoltageMinPhaseB::Id: { // type is int16u
            MN::RmsVoltageMinPhaseB::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RmsVoltageMaxPhaseB::Id: { // type is int16u
            MN::RmsVoltageMaxPhaseB::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RmsCurrentPhaseB::Id: { // type is int16u
            MN::RmsCurrentPhaseB::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RmsCurrentMinPhaseB::Id: { // type is int16u
            MN::RmsCurrentMinPhaseB::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RmsCurrentMaxPhaseB::Id: { // type is int16u
            MN::RmsCurrentMaxPhaseB::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ActivePowerPhaseB::Id: { // type is int16s
            MN::ActivePowerPhaseB::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ActivePowerMinPhaseB::Id: { // type is int16s
            MN::ActivePowerMinPhaseB::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ActivePowerMaxPhaseB::Id: { // type is int16s
            MN::ActivePowerMaxPhaseB::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ReactivePowerPhaseB::Id: { // type is int16s
            MN::ReactivePowerPhaseB::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ApparentPowerPhaseB::Id: { // type is int16u
            MN::ApparentPowerPhaseB::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::PowerFactorPhaseB::Id: { // type is int8s
            MN::PowerFactorPhaseB::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AverageRmsVoltageMeasurementPeriodPhaseB::Id: { // type is int16u
            MN::AverageRmsVoltageMeasurementPeriodPhaseB::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AverageRmsOverVoltageCounterPhaseB::Id: { // type is int16u
            MN::AverageRmsOverVoltageCounterPhaseB::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AverageRmsUnderVoltageCounterPhaseB::Id: { // type is int16u
            MN::AverageRmsUnderVoltageCounterPhaseB::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RmsExtremeOverVoltagePeriodPhaseB::Id: { // type is int16u
            MN::RmsExtremeOverVoltagePeriodPhaseB::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RmsExtremeUnderVoltagePeriodPhaseB::Id: { // type is int16u
            MN::RmsExtremeUnderVoltagePeriodPhaseB::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RmsVoltageSagPeriodPhaseB::Id: { // type is int16u
            MN::RmsVoltageSagPeriodPhaseB::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RmsVoltageSwellPeriodPhaseB::Id: { // type is int16u
            MN::RmsVoltageSwellPeriodPhaseB::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::LineCurrentPhaseC::Id: { // type is int16u
            MN::LineCurrentPhaseC::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ActiveCurrentPhaseC::Id: { // type is int16s
            MN::ActiveCurrentPhaseC::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ReactiveCurrentPhaseC::Id: { // type is int16s
            MN::ReactiveCurrentPhaseC::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RmsVoltagePhaseC::Id: { // type is int16u
            MN::RmsVoltagePhaseC::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RmsVoltageMinPhaseC::Id: { // type is int16u
            MN::RmsVoltageMinPhaseC::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RmsVoltageMaxPhaseC::Id: { // type is int16u
            MN::RmsVoltageMaxPhaseC::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RmsCurrentPhaseC::Id: { // type is int16u
            MN::RmsCurrentPhaseC::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RmsCurrentMinPhaseC::Id: { // type is int16u
            MN::RmsCurrentMinPhaseC::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RmsCurrentMaxPhaseC::Id: { // type is int16u
            MN::RmsCurrentMaxPhaseC::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ActivePowerPhaseC::Id: { // type is int16s
            MN::ActivePowerPhaseC::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ActivePowerMinPhaseC::Id: { // type is int16s
            MN::ActivePowerMinPhaseC::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ActivePowerMaxPhaseC::Id: { // type is int16s
            MN::ActivePowerMaxPhaseC::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ReactivePowerPhaseC::Id: { // type is int16s
            MN::ReactivePowerPhaseC::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ApparentPowerPhaseC::Id: { // type is int16u
            MN::ApparentPowerPhaseC::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::PowerFactorPhaseC::Id: { // type is int8s
            MN::PowerFactorPhaseC::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AverageRmsVoltageMeasurementPeriodPhaseC::Id: { // type is int16u
            MN::AverageRmsVoltageMeasurementPeriodPhaseC::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AverageRmsOverVoltageCounterPhaseC::Id: { // type is int16u
            MN::AverageRmsOverVoltageCounterPhaseC::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AverageRmsUnderVoltageCounterPhaseC::Id: { // type is int16u
            MN::AverageRmsUnderVoltageCounterPhaseC::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RmsExtremeOverVoltagePeriodPhaseC::Id: { // type is int16u
            MN::RmsExtremeOverVoltagePeriodPhaseC::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RmsExtremeUnderVoltagePeriodPhaseC::Id: { // type is int16u
            MN::RmsExtremeUnderVoltagePeriodPhaseC::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RmsVoltageSagPeriodPhaseC::Id: { // type is int16u
            MN::RmsVoltageSagPeriodPhaseC::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RmsVoltageSwellPeriodPhaseC::Id: { // type is int16u
            MN::RmsVoltageSwellPeriodPhaseC::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        }
    } catch (const std::out_of_range& e) {
        sl_log_info(LOG_TAG,
            "The request attribute Path for endpoint [%i] is not found in the attribute state "
            "container: %s\n",
            atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalMeasurementAttributeAccess::Write(const ConcreteDataAttributePath& aPath, AttributeValueDecoder& aDecoder)
{
    using namespace chip::app::Clusters::ElectricalMeasurement;

    if (aPath.mClusterId != Clusters::ElectricalMeasurement::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }
    nlohmann::json jsn;

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().write_attribute(aPath, aDecoder);
    }

    switch (aPath.mAttributeId) {
    // measurement type is not supported by UCL
    // dc voltage is not supported by UCL
    // dc voltage min is not supported by UCL
    // dc voltage max is not supported by UCL
    // dc current is not supported by UCL
    // dc current min is not supported by UCL
    // dc current max is not supported by UCL
    // dc power is not supported by UCL
    // dc power min is not supported by UCL
    // dc power max is not supported by UCL
    // dc voltage multiplier is not supported by UCL
    // dc voltage divisor is not supported by UCL
    // dc current multiplier is not supported by UCL
    // dc current divisor is not supported by UCL
    // dc power multiplier is not supported by UCL
    // dc power divisor is not supported by UCL
    // ac frequency is not supported by UCL
    // ac frequency min is not supported by UCL
    // ac frequency max is not supported by UCL
    // neutral current is not supported by UCL
    // total active power is not supported by UCL
    // total reactive power is not supported by UCL
    // total apparent power is not supported by UCL
    // measured 1st harmonic current is not supported by UCL
    // measured 3rd harmonic current is not supported by UCL
    // measured 5th harmonic current is not supported by UCL
    // measured 7th harmonic current is not supported by UCL
    // measured 9th harmonic current is not supported by UCL
    // measured 11th harmonic current is not supported by UCL
    // measured phase 1st harmonic current is not supported by UCL
    // measured phase 3rd harmonic current is not supported by UCL
    // measured phase 5th harmonic current is not supported by UCL
    // measured phase 7th harmonic current is not supported by UCL
    // measured phase 9th harmonic current is not supported by UCL
    // measured phase 11th harmonic current is not supported by UCL
    // ac frequency multiplier is not supported by UCL
    // ac frequency divisor is not supported by UCL
    // power multiplier is not supported by UCL
    // power divisor is not supported by UCL
    // harmonic current multiplier is not supported by UCL
    // phase harmonic current multiplier is not supported by UCL
    // instantaneous voltage is not supported by UCL
    // instantaneous line current is not supported by UCL
    // instantaneous active current is not supported by UCL
    // instantaneous reactive current is not supported by UCL
    // instantaneous power is not supported by UCL
    // rms voltage is not supported by UCL
    // rms voltage min is not supported by UCL
    // rms voltage max is not supported by UCL
    // rms current is not supported by UCL
    // rms current min is not supported by UCL
    // rms current max is not supported by UCL
    // active power is not supported by UCL
    // active power min is not supported by UCL
    // active power max is not supported by UCL
    // reactive power is not supported by UCL
    // apparent power is not supported by UCL
    // power factor is not supported by UCL
    case Attributes::AverageRmsVoltageMeasurementPeriod::Id: {

        Attributes::AverageRmsVoltageMeasurementPeriod::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["AverageRMSVoltageMeasurementPeriod"] = to_json(value);
        break;
    }
    case Attributes::AverageRmsUnderVoltageCounter::Id: {

        Attributes::AverageRmsUnderVoltageCounter::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["AverageRMSUnderVoltageCounter"] = to_json(value);
        break;
    }
    case Attributes::RmsExtremeOverVoltagePeriod::Id: {

        Attributes::RmsExtremeOverVoltagePeriod::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["RMSExtremeOverVoltagePeriod"] = to_json(value);
        break;
    }
    case Attributes::RmsExtremeUnderVoltagePeriod::Id: {

        Attributes::RmsExtremeUnderVoltagePeriod::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["RMSExtremeUnderVoltagePeriod"] = to_json(value);
        break;
    }
    case Attributes::RmsVoltageSagPeriod::Id: {

        Attributes::RmsVoltageSagPeriod::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["RMSVoltageSagPeriod"] = to_json(value);
        break;
    }
    case Attributes::RmsVoltageSwellPeriod::Id: {

        Attributes::RmsVoltageSwellPeriod::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["RMSVoltageSwellPeriod"] = to_json(value);
        break;
    }
    // ac voltage multiplier is not supported by UCL
    // ac voltage divisor is not supported by UCL
    // ac current multiplier is not supported by UCL
    // ac current divisor is not supported by UCL
    // ac power multiplier is not supported by UCL
    // ac power divisor is not supported by UCL
    case Attributes::OverloadAlarmsMask::Id: {

        Attributes::OverloadAlarmsMask::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["DCOverloadAlarmsMask"] = to_json(value);
        break;
    }
    // voltage overload is not supported by UCL
    // current overload is not supported by UCL
    case Attributes::AcOverloadAlarmsMask::Id: {

        Attributes::AcOverloadAlarmsMask::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ACAlarmsMask"] = to_json(value);
        break;
    }
        // ac voltage overload is not supported by UCL
        // ac current overload is not supported by UCL
        // ac active power overload is not supported by UCL
        // ac reactive power overload is not supported by UCL
        // average rms over voltage is not supported by UCL
        // average rms under voltage is not supported by UCL
        // rms extreme over voltage is not supported by UCL
        // rms extreme under voltage is not supported by UCL
        // rms voltage sag is not supported by UCL
        // rms voltage swell is not supported by UCL
        // line current phase b is not supported by UCL
        // active current phase b is not supported by UCL
        // reactive current phase b is not supported by UCL
        // rms voltage phase b is not supported by UCL
        // rms voltage min phase b is not supported by UCL
        // rms voltage max phase b is not supported by UCL
        // rms current phase b is not supported by UCL
        // rms current min phase b is not supported by UCL
        // rms current max phase b is not supported by UCL
        // active power phase b is not supported by UCL
        // active power min phase b is not supported by UCL
        // active power max phase b is not supported by UCL
        // reactive power phase b is not supported by UCL
        // apparent power phase b is not supported by UCL
        // power factor phase b is not supported by UCL
        // average rms voltage measurement period phase b is not supported by UCL
        // average rms over voltage counter phase b is not supported by UCL
        // average rms under voltage counter phase b is not supported by UCL
        // rms extreme over voltage period phase b is not supported by UCL
        // rms extreme under voltage period phase b is not supported by UCL
        // rms voltage sag period phase b is not supported by UCL
        // rms voltage swell period phase b is not supported by UCL
        // line current phase c is not supported by UCL
        // active current phase c is not supported by UCL
        // reactive current phase c is not supported by UCL
        // rms voltage phase c is not supported by UCL
        // rms voltage min phase c is not supported by UCL
        // rms voltage max phase c is not supported by UCL
        // rms current phase c is not supported by UCL
        // rms current min phase c is not supported by UCL
        // rms current max phase c is not supported by UCL
        // active power phase c is not supported by UCL
        // active power min phase c is not supported by UCL
        // active power max phase c is not supported by UCL
        // reactive power phase c is not supported by UCL
        // apparent power phase c is not supported by UCL
        // power factor phase c is not supported by UCL
        // average rms voltage measurement period phase c is not supported by UCL
        // average rms over voltage counter phase c is not supported by UCL
        // average rms under voltage counter phase c is not supported by UCL
        // rms extreme over voltage period phase c is not supported by UCL
        // rms extreme under voltage period phase c is not supported by UCL
        // rms voltage sag period phase c is not supported by UCL
        // rms voltage swell period phase c is not supported by UCL
        // GeneratedCommandList is not supported by UCL
        // AcceptedCommandList is not supported by UCL
        // EventList is not supported by UCL
        // AttributeList is not supported by UCL
        // FeatureMap is not supported by UCL
        // ClusterRevision is not supported by UCL
    }

    if (!jsn.empty()) {
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) + "/ElectricalMeasurement/Commands/WriteAttributes";
        std::string payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NO_MESSAGE_HANDLER;
}

void ElectricalMeasurementAttributeAccess::reported_updated(const bridged_endpoint* ep, const std::string& cluster,
    const std::string& attribute, const nlohmann::json& unify_value)
{
    namespace MN = chip::app::Clusters::ElectricalMeasurement::Attributes;
    namespace UN = unify::matter_bridge::ElectricalMeasurement::Attributes;

    auto cluster_id = m_dev_translator.get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::ElectricalMeasurement::Id)) {
        return;
    }

    // get attribute id
    auto attribute_id = m_dev_translator.get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value()) {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::ElectricalMeasurement::Id, attribute_id.value());
    switch (attribute_id.value()) {
    // type is bitmap32
    case MN::MeasurementType::Id: {
        using T = MN::MeasurementType::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MeasurementType attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::MeasurementType::Id);
        }
        break;
    }
        // type is int16s
    case MN::DcVoltage::Id: {
        using T = MN::DcVoltage::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "DcVoltage attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id, MN::DcVoltage::Id);
        }
        break;
    }
        // type is int16s
    case MN::DcVoltageMin::Id: {
        using T = MN::DcVoltageMin::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "DcVoltageMin attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id, MN::DcVoltageMin::Id);
        }
        break;
    }
        // type is int16s
    case MN::DcVoltageMax::Id: {
        using T = MN::DcVoltageMax::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "DcVoltageMax attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id, MN::DcVoltageMax::Id);
        }
        break;
    }
        // type is int16s
    case MN::DcCurrent::Id: {
        using T = MN::DcCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "DcCurrent attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id, MN::DcCurrent::Id);
        }
        break;
    }
        // type is int16s
    case MN::DcCurrentMin::Id: {
        using T = MN::DcCurrentMin::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "DcCurrentMin attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id, MN::DcCurrentMin::Id);
        }
        break;
    }
        // type is int16s
    case MN::DcCurrentMax::Id: {
        using T = MN::DcCurrentMax::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "DcCurrentMax attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id, MN::DcCurrentMax::Id);
        }
        break;
    }
        // type is int16s
    case MN::DcPower::Id: {
        using T = MN::DcPower::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "DcPower attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id, MN::DcPower::Id);
        }
        break;
    }
        // type is int16s
    case MN::DcPowerMin::Id: {
        using T = MN::DcPowerMin::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "DcPowerMin attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id, MN::DcPowerMin::Id);
        }
        break;
    }
        // type is int16s
    case MN::DcPowerMax::Id: {
        using T = MN::DcPowerMax::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "DcPowerMax attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id, MN::DcPowerMax::Id);
        }
        break;
    }
        // type is int16u
    case MN::DcVoltageMultiplier::Id: {
        using T = MN::DcVoltageMultiplier::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "DcVoltageMultiplier attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::DcVoltageMultiplier::Id);
        }
        break;
    }
        // type is int16u
    case MN::DcVoltageDivisor::Id: {
        using T = MN::DcVoltageDivisor::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "DcVoltageDivisor attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::DcVoltageDivisor::Id);
        }
        break;
    }
        // type is int16u
    case MN::DcCurrentMultiplier::Id: {
        using T = MN::DcCurrentMultiplier::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "DcCurrentMultiplier attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::DcCurrentMultiplier::Id);
        }
        break;
    }
        // type is int16u
    case MN::DcCurrentDivisor::Id: {
        using T = MN::DcCurrentDivisor::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "DcCurrentDivisor attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::DcCurrentDivisor::Id);
        }
        break;
    }
        // type is int16u
    case MN::DcPowerMultiplier::Id: {
        using T = MN::DcPowerMultiplier::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "DcPowerMultiplier attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::DcPowerMultiplier::Id);
        }
        break;
    }
        // type is int16u
    case MN::DcPowerDivisor::Id: {
        using T = MN::DcPowerDivisor::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "DcPowerDivisor attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::DcPowerDivisor::Id);
        }
        break;
    }
        // type is int16u
    case MN::AcFrequency::Id: {
        using T = MN::AcFrequency::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "AcFrequency attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id, MN::AcFrequency::Id);
        }
        break;
    }
        // type is int16u
    case MN::AcFrequencyMin::Id: {
        using T = MN::AcFrequencyMin::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "AcFrequencyMin attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::AcFrequencyMin::Id);
        }
        break;
    }
        // type is int16u
    case MN::AcFrequencyMax::Id: {
        using T = MN::AcFrequencyMax::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "AcFrequencyMax attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::AcFrequencyMax::Id);
        }
        break;
    }
        // type is int16u
    case MN::NeutralCurrent::Id: {
        using T = MN::NeutralCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "NeutralCurrent attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::NeutralCurrent::Id);
        }
        break;
    }
        // type is int32s
    case MN::TotalActivePower::Id: {
        using T = MN::TotalActivePower::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "TotalActivePower attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::TotalActivePower::Id);
        }
        break;
    }
        // type is int32s
    case MN::TotalReactivePower::Id: {
        using T = MN::TotalReactivePower::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "TotalReactivePower attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::TotalReactivePower::Id);
        }
        break;
    }
        // type is int32u
    case MN::TotalApparentPower::Id: {
        using T = MN::TotalApparentPower::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "TotalApparentPower attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::TotalApparentPower::Id);
        }
        break;
    }
        // type is int16s
    case MN::Measured1stHarmonicCurrent::Id: {
        using T = MN::Measured1stHarmonicCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Measured1stHarmonicCurrent attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::Measured1stHarmonicCurrent::Id);
        }
        break;
    }
        // type is int16s
    case MN::Measured3rdHarmonicCurrent::Id: {
        using T = MN::Measured3rdHarmonicCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Measured3rdHarmonicCurrent attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::Measured3rdHarmonicCurrent::Id);
        }
        break;
    }
        // type is int16s
    case MN::Measured5thHarmonicCurrent::Id: {
        using T = MN::Measured5thHarmonicCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Measured5thHarmonicCurrent attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::Measured5thHarmonicCurrent::Id);
        }
        break;
    }
        // type is int16s
    case MN::Measured7thHarmonicCurrent::Id: {
        using T = MN::Measured7thHarmonicCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Measured7thHarmonicCurrent attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::Measured7thHarmonicCurrent::Id);
        }
        break;
    }
        // type is int16s
    case MN::Measured9thHarmonicCurrent::Id: {
        using T = MN::Measured9thHarmonicCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Measured9thHarmonicCurrent attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::Measured9thHarmonicCurrent::Id);
        }
        break;
    }
        // type is int16s
    case MN::Measured11thHarmonicCurrent::Id: {
        using T = MN::Measured11thHarmonicCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Measured11thHarmonicCurrent attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::Measured11thHarmonicCurrent::Id);
        }
        break;
    }
        // type is int16s
    case MN::MeasuredPhase1stHarmonicCurrent::Id: {
        using T = MN::MeasuredPhase1stHarmonicCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MeasuredPhase1stHarmonicCurrent attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::MeasuredPhase1stHarmonicCurrent::Id);
        }
        break;
    }
        // type is int16s
    case MN::MeasuredPhase3rdHarmonicCurrent::Id: {
        using T = MN::MeasuredPhase3rdHarmonicCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MeasuredPhase3rdHarmonicCurrent attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::MeasuredPhase3rdHarmonicCurrent::Id);
        }
        break;
    }
        // type is int16s
    case MN::MeasuredPhase5thHarmonicCurrent::Id: {
        using T = MN::MeasuredPhase5thHarmonicCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MeasuredPhase5thHarmonicCurrent attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::MeasuredPhase5thHarmonicCurrent::Id);
        }
        break;
    }
        // type is int16s
    case MN::MeasuredPhase7thHarmonicCurrent::Id: {
        using T = MN::MeasuredPhase7thHarmonicCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MeasuredPhase7thHarmonicCurrent attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::MeasuredPhase7thHarmonicCurrent::Id);
        }
        break;
    }
        // type is int16s
    case MN::MeasuredPhase9thHarmonicCurrent::Id: {
        using T = MN::MeasuredPhase9thHarmonicCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MeasuredPhase9thHarmonicCurrent attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::MeasuredPhase9thHarmonicCurrent::Id);
        }
        break;
    }
        // type is int16s
    case MN::MeasuredPhase11thHarmonicCurrent::Id: {
        using T = MN::MeasuredPhase11thHarmonicCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MeasuredPhase11thHarmonicCurrent attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::MeasuredPhase11thHarmonicCurrent::Id);
        }
        break;
    }
        // type is int16u
    case MN::AcFrequencyMultiplier::Id: {
        using T = MN::AcFrequencyMultiplier::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "AcFrequencyMultiplier attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::AcFrequencyMultiplier::Id);
        }
        break;
    }
        // type is int16u
    case MN::AcFrequencyDivisor::Id: {
        using T = MN::AcFrequencyDivisor::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "AcFrequencyDivisor attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::AcFrequencyDivisor::Id);
        }
        break;
    }
        // type is int32u
    case MN::PowerMultiplier::Id: {
        using T = MN::PowerMultiplier::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "PowerMultiplier attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::PowerMultiplier::Id);
        }
        break;
    }
        // type is int32u
    case MN::PowerDivisor::Id: {
        using T = MN::PowerDivisor::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "PowerDivisor attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id, MN::PowerDivisor::Id);
        }
        break;
    }
        // type is int8s
    case MN::HarmonicCurrentMultiplier::Id: {
        using T = MN::HarmonicCurrentMultiplier::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "HarmonicCurrentMultiplier attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::HarmonicCurrentMultiplier::Id);
        }
        break;
    }
        // type is int8s
    case MN::PhaseHarmonicCurrentMultiplier::Id: {
        using T = MN::PhaseHarmonicCurrentMultiplier::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "PhaseHarmonicCurrentMultiplier attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::PhaseHarmonicCurrentMultiplier::Id);
        }
        break;
    }
        // type is int16u
    case MN::InstantaneousLineCurrent::Id: {
        using T = MN::InstantaneousLineCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "InstantaneousLineCurrent attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::InstantaneousLineCurrent::Id);
        }
        break;
    }
        // type is int16s
    case MN::InstantaneousActiveCurrent::Id: {
        using T = MN::InstantaneousActiveCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "InstantaneousActiveCurrent attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::InstantaneousActiveCurrent::Id);
        }
        break;
    }
        // type is int16s
    case MN::InstantaneousReactiveCurrent::Id: {
        using T = MN::InstantaneousReactiveCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "InstantaneousReactiveCurrent attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::InstantaneousReactiveCurrent::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsVoltage::Id: {
        using T = MN::RmsVoltage::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RmsVoltage attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id, MN::RmsVoltage::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsVoltageMin::Id: {
        using T = MN::RmsVoltageMin::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RmsVoltageMin attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::RmsVoltageMin::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsVoltageMax::Id: {
        using T = MN::RmsVoltageMax::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RmsVoltageMax attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::RmsVoltageMax::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsCurrent::Id: {
        using T = MN::RmsCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RmsCurrent attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id, MN::RmsCurrent::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsCurrentMin::Id: {
        using T = MN::RmsCurrentMin::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RmsCurrentMin attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::RmsCurrentMin::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsCurrentMax::Id: {
        using T = MN::RmsCurrentMax::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RmsCurrentMax attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::RmsCurrentMax::Id);
        }
        break;
    }
        // type is int16s
    case MN::ActivePower::Id: {
        using T = MN::ActivePower::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ActivePower attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id, MN::ActivePower::Id);
        }
        break;
    }
        // type is int16s
    case MN::ActivePowerMin::Id: {
        using T = MN::ActivePowerMin::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ActivePowerMin attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::ActivePowerMin::Id);
        }
        break;
    }
        // type is int16s
    case MN::ActivePowerMax::Id: {
        using T = MN::ActivePowerMax::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ActivePowerMax attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::ActivePowerMax::Id);
        }
        break;
    }
        // type is int16s
    case MN::ReactivePower::Id: {
        using T = MN::ReactivePower::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ReactivePower attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::ReactivePower::Id);
        }
        break;
    }
        // type is int16u
    case MN::ApparentPower::Id: {
        using T = MN::ApparentPower::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ApparentPower attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::ApparentPower::Id);
        }
        break;
    }
        // type is int8s
    case MN::PowerFactor::Id: {
        using T = MN::PowerFactor::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "PowerFactor attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id, MN::PowerFactor::Id);
        }
        break;
    }
        // type is int16u
    case MN::AverageRmsVoltageMeasurementPeriod::Id: {
        using T = MN::AverageRmsVoltageMeasurementPeriod::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "AverageRmsVoltageMeasurementPeriod attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::AverageRmsVoltageMeasurementPeriod::Id);
        }
        break;
    }
        // type is int16u
    case MN::AverageRmsUnderVoltageCounter::Id: {
        using T = MN::AverageRmsUnderVoltageCounter::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "AverageRmsUnderVoltageCounter attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::AverageRmsUnderVoltageCounter::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsExtremeOverVoltagePeriod::Id: {
        using T = MN::RmsExtremeOverVoltagePeriod::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RmsExtremeOverVoltagePeriod attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::RmsExtremeOverVoltagePeriod::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsExtremeUnderVoltagePeriod::Id: {
        using T = MN::RmsExtremeUnderVoltagePeriod::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RmsExtremeUnderVoltagePeriod attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::RmsExtremeUnderVoltagePeriod::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsVoltageSagPeriod::Id: {
        using T = MN::RmsVoltageSagPeriod::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RmsVoltageSagPeriod attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::RmsVoltageSagPeriod::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsVoltageSwellPeriod::Id: {
        using T = MN::RmsVoltageSwellPeriod::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RmsVoltageSwellPeriod attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::RmsVoltageSwellPeriod::Id);
        }
        break;
    }
        // type is int16u
    case MN::AcVoltageMultiplier::Id: {
        using T = MN::AcVoltageMultiplier::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "AcVoltageMultiplier attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::AcVoltageMultiplier::Id);
        }
        break;
    }
        // type is int16u
    case MN::AcVoltageDivisor::Id: {
        using T = MN::AcVoltageDivisor::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "AcVoltageDivisor attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::AcVoltageDivisor::Id);
        }
        break;
    }
        // type is int16u
    case MN::AcCurrentMultiplier::Id: {
        using T = MN::AcCurrentMultiplier::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "AcCurrentMultiplier attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::AcCurrentMultiplier::Id);
        }
        break;
    }
        // type is int16u
    case MN::AcCurrentDivisor::Id: {
        using T = MN::AcCurrentDivisor::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "AcCurrentDivisor attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::AcCurrentDivisor::Id);
        }
        break;
    }
        // type is int16u
    case MN::AcPowerMultiplier::Id: {
        using T = MN::AcPowerMultiplier::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "AcPowerMultiplier attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::AcPowerMultiplier::Id);
        }
        break;
    }
        // type is int16u
    case MN::AcPowerDivisor::Id: {
        using T = MN::AcPowerDivisor::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "AcPowerDivisor attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::AcPowerDivisor::Id);
        }
        break;
    }
        // type is bitmap8
    case MN::OverloadAlarmsMask::Id: {
        using T = MN::OverloadAlarmsMask::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "OverloadAlarmsMask attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::OverloadAlarmsMask::Id);
        }
        break;
    }
        // type is int16s
    case MN::VoltageOverload::Id: {
        using T = MN::VoltageOverload::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "VoltageOverload attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::VoltageOverload::Id);
        }
        break;
    }
        // type is int16s
    case MN::CurrentOverload::Id: {
        using T = MN::CurrentOverload::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "CurrentOverload attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::CurrentOverload::Id);
        }
        break;
    }
        // type is bitmap16
    case MN::AcOverloadAlarmsMask::Id: {
        using T = MN::AcOverloadAlarmsMask::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "AcOverloadAlarmsMask attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::AcOverloadAlarmsMask::Id);
        }
        break;
    }
        // type is int16s
    case MN::AcVoltageOverload::Id: {
        using T = MN::AcVoltageOverload::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "AcVoltageOverload attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::AcVoltageOverload::Id);
        }
        break;
    }
        // type is int16s
    case MN::AcCurrentOverload::Id: {
        using T = MN::AcCurrentOverload::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "AcCurrentOverload attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::AcCurrentOverload::Id);
        }
        break;
    }
        // type is int16s
    case MN::AcActivePowerOverload::Id: {
        using T = MN::AcActivePowerOverload::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "AcActivePowerOverload attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::AcActivePowerOverload::Id);
        }
        break;
    }
        // type is int16s
    case MN::AcReactivePowerOverload::Id: {
        using T = MN::AcReactivePowerOverload::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "AcReactivePowerOverload attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::AcReactivePowerOverload::Id);
        }
        break;
    }
        // type is int16s
    case MN::AverageRmsOverVoltage::Id: {
        using T = MN::AverageRmsOverVoltage::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "AverageRmsOverVoltage attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::AverageRmsOverVoltage::Id);
        }
        break;
    }
        // type is int16s
    case MN::AverageRmsUnderVoltage::Id: {
        using T = MN::AverageRmsUnderVoltage::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "AverageRmsUnderVoltage attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::AverageRmsUnderVoltage::Id);
        }
        break;
    }
        // type is int16s
    case MN::RmsExtremeOverVoltage::Id: {
        using T = MN::RmsExtremeOverVoltage::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RmsExtremeOverVoltage attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::RmsExtremeOverVoltage::Id);
        }
        break;
    }
        // type is int16s
    case MN::RmsExtremeUnderVoltage::Id: {
        using T = MN::RmsExtremeUnderVoltage::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RmsExtremeUnderVoltage attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::RmsExtremeUnderVoltage::Id);
        }
        break;
    }
        // type is int16s
    case MN::RmsVoltageSag::Id: {
        using T = MN::RmsVoltageSag::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RmsVoltageSag attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::RmsVoltageSag::Id);
        }
        break;
    }
        // type is int16s
    case MN::RmsVoltageSwell::Id: {
        using T = MN::RmsVoltageSwell::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RmsVoltageSwell attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::RmsVoltageSwell::Id);
        }
        break;
    }
        // type is int16u
    case MN::LineCurrentPhaseB::Id: {
        using T = MN::LineCurrentPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "LineCurrentPhaseB attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::LineCurrentPhaseB::Id);
        }
        break;
    }
        // type is int16s
    case MN::ActiveCurrentPhaseB::Id: {
        using T = MN::ActiveCurrentPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ActiveCurrentPhaseB attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::ActiveCurrentPhaseB::Id);
        }
        break;
    }
        // type is int16s
    case MN::ReactiveCurrentPhaseB::Id: {
        using T = MN::ReactiveCurrentPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ReactiveCurrentPhaseB attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::ReactiveCurrentPhaseB::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsVoltagePhaseB::Id: {
        using T = MN::RmsVoltagePhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RmsVoltagePhaseB attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::RmsVoltagePhaseB::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsVoltageMinPhaseB::Id: {
        using T = MN::RmsVoltageMinPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RmsVoltageMinPhaseB attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::RmsVoltageMinPhaseB::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsVoltageMaxPhaseB::Id: {
        using T = MN::RmsVoltageMaxPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RmsVoltageMaxPhaseB attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::RmsVoltageMaxPhaseB::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsCurrentPhaseB::Id: {
        using T = MN::RmsCurrentPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RmsCurrentPhaseB attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::RmsCurrentPhaseB::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsCurrentMinPhaseB::Id: {
        using T = MN::RmsCurrentMinPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RmsCurrentMinPhaseB attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::RmsCurrentMinPhaseB::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsCurrentMaxPhaseB::Id: {
        using T = MN::RmsCurrentMaxPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RmsCurrentMaxPhaseB attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::RmsCurrentMaxPhaseB::Id);
        }
        break;
    }
        // type is int16s
    case MN::ActivePowerPhaseB::Id: {
        using T = MN::ActivePowerPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ActivePowerPhaseB attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::ActivePowerPhaseB::Id);
        }
        break;
    }
        // type is int16s
    case MN::ActivePowerMinPhaseB::Id: {
        using T = MN::ActivePowerMinPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ActivePowerMinPhaseB attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::ActivePowerMinPhaseB::Id);
        }
        break;
    }
        // type is int16s
    case MN::ActivePowerMaxPhaseB::Id: {
        using T = MN::ActivePowerMaxPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ActivePowerMaxPhaseB attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::ActivePowerMaxPhaseB::Id);
        }
        break;
    }
        // type is int16s
    case MN::ReactivePowerPhaseB::Id: {
        using T = MN::ReactivePowerPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ReactivePowerPhaseB attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::ReactivePowerPhaseB::Id);
        }
        break;
    }
        // type is int16u
    case MN::ApparentPowerPhaseB::Id: {
        using T = MN::ApparentPowerPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ApparentPowerPhaseB attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::ApparentPowerPhaseB::Id);
        }
        break;
    }
        // type is int8s
    case MN::PowerFactorPhaseB::Id: {
        using T = MN::PowerFactorPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "PowerFactorPhaseB attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::PowerFactorPhaseB::Id);
        }
        break;
    }
        // type is int16u
    case MN::AverageRmsVoltageMeasurementPeriodPhaseB::Id: {
        using T = MN::AverageRmsVoltageMeasurementPeriodPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "AverageRmsVoltageMeasurementPeriodPhaseB attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::AverageRmsVoltageMeasurementPeriodPhaseB::Id);
        }
        break;
    }
        // type is int16u
    case MN::AverageRmsOverVoltageCounterPhaseB::Id: {
        using T = MN::AverageRmsOverVoltageCounterPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "AverageRmsOverVoltageCounterPhaseB attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::AverageRmsOverVoltageCounterPhaseB::Id);
        }
        break;
    }
        // type is int16u
    case MN::AverageRmsUnderVoltageCounterPhaseB::Id: {
        using T = MN::AverageRmsUnderVoltageCounterPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "AverageRmsUnderVoltageCounterPhaseB attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::AverageRmsUnderVoltageCounterPhaseB::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsExtremeOverVoltagePeriodPhaseB::Id: {
        using T = MN::RmsExtremeOverVoltagePeriodPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RmsExtremeOverVoltagePeriodPhaseB attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::RmsExtremeOverVoltagePeriodPhaseB::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsExtremeUnderVoltagePeriodPhaseB::Id: {
        using T = MN::RmsExtremeUnderVoltagePeriodPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RmsExtremeUnderVoltagePeriodPhaseB attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::RmsExtremeUnderVoltagePeriodPhaseB::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsVoltageSagPeriodPhaseB::Id: {
        using T = MN::RmsVoltageSagPeriodPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RmsVoltageSagPeriodPhaseB attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::RmsVoltageSagPeriodPhaseB::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsVoltageSwellPeriodPhaseB::Id: {
        using T = MN::RmsVoltageSwellPeriodPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RmsVoltageSwellPeriodPhaseB attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::RmsVoltageSwellPeriodPhaseB::Id);
        }
        break;
    }
        // type is int16u
    case MN::LineCurrentPhaseC::Id: {
        using T = MN::LineCurrentPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "LineCurrentPhaseC attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::LineCurrentPhaseC::Id);
        }
        break;
    }
        // type is int16s
    case MN::ActiveCurrentPhaseC::Id: {
        using T = MN::ActiveCurrentPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ActiveCurrentPhaseC attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::ActiveCurrentPhaseC::Id);
        }
        break;
    }
        // type is int16s
    case MN::ReactiveCurrentPhaseC::Id: {
        using T = MN::ReactiveCurrentPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ReactiveCurrentPhaseC attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::ReactiveCurrentPhaseC::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsVoltagePhaseC::Id: {
        using T = MN::RmsVoltagePhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RmsVoltagePhaseC attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::RmsVoltagePhaseC::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsVoltageMinPhaseC::Id: {
        using T = MN::RmsVoltageMinPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RmsVoltageMinPhaseC attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::RmsVoltageMinPhaseC::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsVoltageMaxPhaseC::Id: {
        using T = MN::RmsVoltageMaxPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RmsVoltageMaxPhaseC attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::RmsVoltageMaxPhaseC::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsCurrentPhaseC::Id: {
        using T = MN::RmsCurrentPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RmsCurrentPhaseC attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::RmsCurrentPhaseC::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsCurrentMinPhaseC::Id: {
        using T = MN::RmsCurrentMinPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RmsCurrentMinPhaseC attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::RmsCurrentMinPhaseC::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsCurrentMaxPhaseC::Id: {
        using T = MN::RmsCurrentMaxPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RmsCurrentMaxPhaseC attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::RmsCurrentMaxPhaseC::Id);
        }
        break;
    }
        // type is int16s
    case MN::ActivePowerPhaseC::Id: {
        using T = MN::ActivePowerPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ActivePowerPhaseC attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::ActivePowerPhaseC::Id);
        }
        break;
    }
        // type is int16s
    case MN::ActivePowerMinPhaseC::Id: {
        using T = MN::ActivePowerMinPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ActivePowerMinPhaseC attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::ActivePowerMinPhaseC::Id);
        }
        break;
    }
        // type is int16s
    case MN::ActivePowerMaxPhaseC::Id: {
        using T = MN::ActivePowerMaxPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ActivePowerMaxPhaseC attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::ActivePowerMaxPhaseC::Id);
        }
        break;
    }
        // type is int16s
    case MN::ReactivePowerPhaseC::Id: {
        using T = MN::ReactivePowerPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ReactivePowerPhaseC attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::ReactivePowerPhaseC::Id);
        }
        break;
    }
        // type is int16u
    case MN::ApparentPowerPhaseC::Id: {
        using T = MN::ApparentPowerPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ApparentPowerPhaseC attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::ApparentPowerPhaseC::Id);
        }
        break;
    }
        // type is int8s
    case MN::PowerFactorPhaseC::Id: {
        using T = MN::PowerFactorPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "PowerFactorPhaseC attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::PowerFactorPhaseC::Id);
        }
        break;
    }
        // type is int16u
    case MN::AverageRmsVoltageMeasurementPeriodPhaseC::Id: {
        using T = MN::AverageRmsVoltageMeasurementPeriodPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "AverageRmsVoltageMeasurementPeriodPhaseC attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::AverageRmsVoltageMeasurementPeriodPhaseC::Id);
        }
        break;
    }
        // type is int16u
    case MN::AverageRmsOverVoltageCounterPhaseC::Id: {
        using T = MN::AverageRmsOverVoltageCounterPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "AverageRmsOverVoltageCounterPhaseC attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::AverageRmsOverVoltageCounterPhaseC::Id);
        }
        break;
    }
        // type is int16u
    case MN::AverageRmsUnderVoltageCounterPhaseC::Id: {
        using T = MN::AverageRmsUnderVoltageCounterPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "AverageRmsUnderVoltageCounterPhaseC attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::AverageRmsUnderVoltageCounterPhaseC::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsExtremeOverVoltagePeriodPhaseC::Id: {
        using T = MN::RmsExtremeOverVoltagePeriodPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RmsExtremeOverVoltagePeriodPhaseC attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::RmsExtremeOverVoltagePeriodPhaseC::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsExtremeUnderVoltagePeriodPhaseC::Id: {
        using T = MN::RmsExtremeUnderVoltagePeriodPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RmsExtremeUnderVoltagePeriodPhaseC attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::RmsExtremeUnderVoltagePeriodPhaseC::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsVoltageSagPeriodPhaseC::Id: {
        using T = MN::RmsVoltageSagPeriodPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RmsVoltageSagPeriodPhaseC attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::RmsVoltageSagPeriodPhaseC::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsVoltageSwellPeriodPhaseC::Id: {
        using T = MN::RmsVoltageSwellPeriodPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RmsVoltageSwellPeriodPhaseC attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                MN::RmsVoltageSwellPeriodPhaseC::Id);
        }
        break;
    }
    }
}
