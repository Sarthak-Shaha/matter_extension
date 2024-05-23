/*******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#define CHIP_USE_ENUM_CLASS_FOR_IM_ENUM

#include "command_translator.hpp"
#include "chip_types_to_json.hpp"
#include "cluster_emulator.hpp"
#include "matter_device_translator.hpp"
#include "sl_log.h"
#include <iostream>
#include <sstream>
#include <string>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace unify::matter_bridge;

#define LOG_TAG "matter_cluster_command_server"

// Identify : 3
void IdentifyClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext& ctxt)
{
    using namespace chip::app::Clusters::Identify;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node) {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    if (m_node_state_monitor.emulator().is_command_emulated(ctxt.mRequestPath)) {
        emulated_cmd_payload cdata;
        m_node_state_monitor.emulator().invoke_command(ctxt, cdata);
        // if cmd is not completely handled in invoke_command, then
        // cmd_emulation_completed should be set to false in emulated cmd handling
        if (!cdata.cmd_emulation_completed) {
            cmd = cdata.cmd;
            payload = cdata.payload;
        } else {
            return;
        }
    }

    switch (ctxt.mRequestPath.mCommandId) {
    case Commands::Identify::Id: {
        Commands::Identify::DecodableType data;
        cmd = "Identify"; // "Identify";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["IdentifyTime"] = to_json(data.identifyTime);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::TriggerEffect::Id: {
        Commands::TriggerEffect::DecodableType data;
        cmd = "TriggerEffect"; // "TriggerEffect";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["EffectIdentifier"] = to_json(data.effectIdentifier);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["EffectVariant"] = to_json(data.effectVariant);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    }

    if (!cmd.empty()) {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    } else {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

// Groups : 4
void GroupsClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext& ctxt)
{
    using namespace chip::app::Clusters::Groups;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node) {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    if (m_node_state_monitor.emulator().is_command_emulated(ctxt.mRequestPath)) {
        emulated_cmd_payload cdata;
        m_node_state_monitor.emulator().invoke_command(ctxt, cdata);
        // if cmd is not completely handled in invoke_command, then
        // cmd_emulation_completed should be set to false in emulated cmd handling
        if (!cdata.cmd_emulation_completed) {
            cmd = cdata.cmd;
            payload = cdata.payload;
        } else {
            return;
        }
    }

    switch (ctxt.mRequestPath.mCommandId) {
    case Commands::AddGroup::Id: {
        Commands::AddGroup::DecodableType data;
        cmd = "AddGroup"; // "AddGroup";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["GroupId"] = to_json(data.groupID);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["GroupName"] = to_json(data.groupName);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::ViewGroup::Id: {
        Commands::ViewGroup::DecodableType data;
        cmd = "ViewGroup"; // "ViewGroup";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["GroupId"] = to_json(data.groupID);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::GetGroupMembership::Id: {
        Commands::GetGroupMembership::DecodableType data;
        cmd = "GetGroupMembership"; // "GetGroupMembership";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["GroupList"] = to_json(data.groupList);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::RemoveGroup::Id: {
        Commands::RemoveGroup::DecodableType data;
        cmd = "RemoveGroup"; // "RemoveGroup";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["GroupId"] = to_json(data.groupID);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::RemoveAllGroups::Id: {
        Commands::RemoveAllGroups::DecodableType data;
        cmd = "RemoveAllGroups"; // "RemoveAllGroups";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
        }
    } break;
    case Commands::AddGroupIfIdentifying::Id: {
        Commands::AddGroupIfIdentifying::DecodableType data;
        cmd = "AddGroupIfIdentifying"; // "AddGroupIfIdentifying";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["GroupId"] = to_json(data.groupID);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["GroupName"] = to_json(data.groupName);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    }

    if (!cmd.empty()) {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    } else {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

// On/Off : 6
void OnOffClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext& ctxt)
{
    using namespace chip::app::Clusters::OnOff;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node) {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    if (m_node_state_monitor.emulator().is_command_emulated(ctxt.mRequestPath)) {
        emulated_cmd_payload cdata;
        m_node_state_monitor.emulator().invoke_command(ctxt, cdata);
        // if cmd is not completely handled in invoke_command, then
        // cmd_emulation_completed should be set to false in emulated cmd handling
        if (!cdata.cmd_emulation_completed) {
            cmd = cdata.cmd;
            payload = cdata.payload;
        } else {
            return;
        }
    }

    switch (ctxt.mRequestPath.mCommandId) {
    case Commands::Off::Id: {
        Commands::Off::DecodableType data;
        cmd = "Off"; // "Off";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
        }
    } break;
    case Commands::On::Id: {
        Commands::On::DecodableType data;
        cmd = "On"; // "On";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
        }
    } break;
    case Commands::Toggle::Id: {
        Commands::Toggle::DecodableType data;
        cmd = "Toggle"; // "Toggle";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
        }
    } break;
    case Commands::OffWithEffect::Id: {
        Commands::OffWithEffect::DecodableType data;
        cmd = "OffWithEffect"; // "OffWithEffect";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["EffectIdentifier"] = to_json(data.effectIdentifier);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["EffectVariant"] = to_json(data.effectVariant);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::OnWithRecallGlobalScene::Id: {
        Commands::OnWithRecallGlobalScene::DecodableType data;
        cmd = "OnWithRecallGlobalScene"; // "OnWithRecallGlobalScene";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
        }
    } break;
    case Commands::OnWithTimedOff::Id: {
        Commands::OnWithTimedOff::DecodableType data;
        cmd = "OnWithTimedOff"; // "OnWithTimedOff";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["OnOffControl"] = to_json(data.onOffControl);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OnTime"] = to_json(data.onTime);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OffWaitTime"] = to_json(data.offWaitTime);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    }

    if (!cmd.empty()) {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    } else {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

// Level Control : 8
void LevelControlClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext& ctxt)
{
    using namespace chip::app::Clusters::LevelControl;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node) {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    if (m_node_state_monitor.emulator().is_command_emulated(ctxt.mRequestPath)) {
        emulated_cmd_payload cdata;
        m_node_state_monitor.emulator().invoke_command(ctxt, cdata);
        // if cmd is not completely handled in invoke_command, then
        // cmd_emulation_completed should be set to false in emulated cmd handling
        if (!cdata.cmd_emulation_completed) {
            cmd = cdata.cmd;
            payload = cdata.payload;
        } else {
            return;
        }
    }

    switch (ctxt.mRequestPath.mCommandId) {
    case Commands::MoveToLevel::Id: {
        Commands::MoveToLevel::DecodableType data;
        cmd = "MoveToLevel"; // "MoveToLevel";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["Level"] = to_json(data.level);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["TransitionTime"] = to_json(data.transitionTime);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::Move::Id: {
        Commands::Move::DecodableType data;
        cmd = "Move"; // "Move";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["MoveMode"] = to_json(data.moveMode);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["Rate"] = to_json(data.rate);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::Step::Id: {
        Commands::Step::DecodableType data;
        cmd = "Step"; // "Step";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["StepMode"] = to_json(data.stepMode);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["StepSize"] = to_json(data.stepSize);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["TransitionTime"] = to_json(data.transitionTime);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::Stop::Id: {
        Commands::Stop::DecodableType data;
        cmd = "Stop"; // "Stop";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::MoveToLevelWithOnOff::Id: {
        Commands::MoveToLevelWithOnOff::DecodableType data;
        cmd = "MoveToLevelWithOnOff"; // "MoveToLevelWithOnOff";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["Level"] = to_json(data.level);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["TransitionTime"] = to_json(data.transitionTime);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::MoveWithOnOff::Id: {
        Commands::MoveWithOnOff::DecodableType data;
        cmd = "MoveWithOnOff"; // "MoveWithOnOff";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["MoveMode"] = to_json(data.moveMode);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["Rate"] = to_json(data.rate);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::StepWithOnOff::Id: {
        Commands::StepWithOnOff::DecodableType data;
        cmd = "StepWithOnOff"; // "StepWithOnOff";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["StepMode"] = to_json(data.stepMode);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["StepSize"] = to_json(data.stepSize);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["TransitionTime"] = to_json(data.transitionTime);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::StopWithOnOff::Id: {
        Commands::StopWithOnOff::DecodableType data;
        cmd = "StopWithOnOff"; // "StopWithOnOff";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::MoveToClosestFrequency::Id: {
        Commands::MoveToClosestFrequency::DecodableType data;
        cmd = "MoveToClosestFrequency"; // "MoveToClosestFrequency";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["Frequency"] = to_json(data.frequency);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    }

    if (!cmd.empty()) {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    } else {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

// Door Lock : 257
void DoorLockClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext& ctxt)
{
    using namespace chip::app::Clusters::DoorLock;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node) {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    std::string cmd_response;
    nlohmann::json payload = {};

    if (m_node_state_monitor.emulator().is_command_emulated(ctxt.mRequestPath)) {
        emulated_cmd_payload cdata;
        m_node_state_monitor.emulator().invoke_command(ctxt, cdata);
        // if cmd is not completely handled in invoke_command, then
        // cmd_emulation_completed should be set to false in emulated cmd handling
        if (!cdata.cmd_emulation_completed) {
            cmd = cdata.cmd;
            payload = cdata.payload;
        } else {
            return;
        }
    }

    switch (ctxt.mRequestPath.mCommandId) {
    case Commands::LockDoor::Id: {
        Commands::LockDoor::DecodableType data;
        cmd = "LockDoor"; // "LockDoor";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            if (data.PINCode.HasValue()) {
                try {
                    payload["PINOrRFIDCode"] = to_json(data.PINCode.Value());
                } catch (std::exception& ex) {
                    sl_log_warning(LOG_TAG, "Failed to add the command arguments value to json format: %s", ex.what());
                }
            }
        }
        cmd_response = "LockDoorResponse"; // "LockDoor";
    } break;
    case Commands::UnlockDoor::Id: {
        Commands::UnlockDoor::DecodableType data;
        cmd = "UnlockDoor"; // "UnlockDoor";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            if (data.PINCode.HasValue()) {
                try {
                    payload["PINOrRFIDCode"] = to_json(data.PINCode.Value());
                } catch (std::exception& ex) {
                    sl_log_warning(LOG_TAG, "Failed to add the command arguments value to json format: %s", ex.what());
                }
            }
        }
        cmd_response = "UnlockDoorResponse"; // "UnlockDoor";
    } break;
    case Commands::UnlockWithTimeout::Id: {
        Commands::UnlockWithTimeout::DecodableType data;
        cmd = "UnlockWithTimeout"; // "UnlockWithTimeout";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["TimeoutInSeconds"] = to_json(data.timeout);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            if (data.PINCode.HasValue()) {
                try {
                    payload["PINOrRFIDCode"] = to_json(data.PINCode.Value());
                } catch (std::exception& ex) {
                    sl_log_warning(LOG_TAG, "Failed to add the command arguments value to json format: %s", ex.what());
                }
            }
        }
        cmd_response = "UnlockWithTimeoutResponse"; // "UnlockWithTimeout";
    } break;
    case Commands::SetWeekDaySchedule::Id: {
        Commands::SetWeekDaySchedule::DecodableType data;
        cmd = "SetWeekdaySchedule"; // "SetWeekDaySchedule";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["ScheduleID"] = to_json(data.weekDayIndex);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["UserID"] = to_json(data.userIndex);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["DaysMask"] = to_json(data.daysMask);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["StartHour"] = to_json(data.startHour);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["StartMinute"] = to_json(data.startMinute);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["EndHour"] = to_json(data.endHour);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["EndMinute"] = to_json(data.endMinute);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
        cmd_response = "SetWeekdayScheduleResponse"; // "SetWeekDaySchedule";
    } break;
    case Commands::GetWeekDaySchedule::Id: {
        Commands::GetWeekDaySchedule::DecodableType data;
        cmd = "GetWeekdaySchedule"; // "GetWeekDaySchedule";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["ScheduleID"] = to_json(data.weekDayIndex);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["UserID"] = to_json(data.userIndex);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
        cmd_response = "GetWeekdayScheduleResponse"; // "GetWeekDaySchedule";
    } break;
    case Commands::ClearWeekDaySchedule::Id: {
        Commands::ClearWeekDaySchedule::DecodableType data;
        cmd = "ClearWeekdaySchedule"; // "ClearWeekDaySchedule";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["ScheduleID"] = to_json(data.weekDayIndex);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["UserID"] = to_json(data.userIndex);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
        cmd_response = "ClearWeekdayScheduleResponse"; // "ClearWeekDaySchedule";
    } break;
    case Commands::SetYearDaySchedule::Id: {
        Commands::SetYearDaySchedule::DecodableType data;
        cmd = "SetYearDaySchedule"; // "SetYearDaySchedule";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["ScheduleID"] = to_json(data.yearDayIndex);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["UserID"] = to_json(data.userIndex);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["LocalStartTime"] = to_json(data.localStartTime);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["LocalEndTime"] = to_json(data.localEndTime);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
        cmd_response = "SetYearDayScheduleResponse"; // "SetYearDaySchedule";
    } break;
    case Commands::GetYearDaySchedule::Id: {
        Commands::GetYearDaySchedule::DecodableType data;
        cmd = "GetYearDaySchedule"; // "GetYearDaySchedule";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["ScheduleID"] = to_json(data.yearDayIndex);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["UserID"] = to_json(data.userIndex);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
        cmd_response = "GetYearDayScheduleResponse"; // "GetYearDaySchedule";
    } break;
    case Commands::ClearYearDaySchedule::Id: {
        Commands::ClearYearDaySchedule::DecodableType data;
        cmd = "ClearYearDaySchedule"; // "ClearYearDaySchedule";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["ScheduleID"] = to_json(data.yearDayIndex);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["UserID"] = to_json(data.userIndex);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
        cmd_response = "ClearYearDayScheduleResponse"; // "ClearYearDaySchedule";
    } break;
    case Commands::SetHolidaySchedule::Id: {
        Commands::SetHolidaySchedule::DecodableType data;
        cmd = "SetHolidaySchedule"; // "SetHolidaySchedule";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["HolidayScheduleID"] = to_json(data.holidayIndex);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["LocalStartTime"] = to_json(data.localStartTime);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["LocalEndTime"] = to_json(data.localEndTime);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OperatingModeDuringHoliday"] = to_json(data.operatingMode);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
        cmd_response = "SetHolidayScheduleResponse"; // "SetHolidaySchedule";
    } break;
    case Commands::GetHolidaySchedule::Id: {
        Commands::GetHolidaySchedule::DecodableType data;
        cmd = "GetHolidaySchedule"; // "GetHolidaySchedule";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["HolidayScheduleID"] = to_json(data.holidayIndex);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
        cmd_response = "GetHolidayScheduleResponse"; // "GetHolidaySchedule";
    } break;
    case Commands::ClearHolidaySchedule::Id: {
        Commands::ClearHolidaySchedule::DecodableType data;
        cmd = "ClearHolidaySchedule"; // "ClearHolidaySchedule";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["HolidayScheduleID"] = to_json(data.holidayIndex);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
        cmd_response = "ClearHolidayScheduleResponse"; // "ClearHolidaySchedule";
    } break;
    case Commands::SetUser::Id: {
        Commands::SetUser::DecodableType data;
        cmd = "SetUser"; // "SetUser";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["OperationType"] = to_json(data.operationType);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["UserIndex"] = to_json(data.userIndex);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["UserName"] = to_json(data.userName);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["UserUniqueID"] = to_json(data.userUniqueID);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["UserStatus"] = to_json(data.userStatus);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["UserType"] = to_json(data.userType);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["CredentialRule"] = to_json(data.credentialRule);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::GetUser::Id: {
        Commands::GetUser::DecodableType data;
        cmd = "GetUser"; // "GetUser";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["UserIndex"] = to_json(data.userIndex);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::ClearUser::Id: {
        Commands::ClearUser::DecodableType data;
        cmd = "ClearUser"; // "ClearUser";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["UserIndex"] = to_json(data.userIndex);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::SetCredential::Id: {
        Commands::SetCredential::DecodableType data;
        cmd = "SetCredential"; // "SetCredential";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["OperationType"] = to_json(data.operationType);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["Credential"] = to_json(data.credential);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["CredentialData"] = to_json(data.credentialData);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["UserIndex"] = to_json(data.userIndex);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["UserStatus"] = to_json(data.userStatus);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["UserType"] = to_json(data.userType);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::GetCredentialStatus::Id: {
        Commands::GetCredentialStatus::DecodableType data;
        cmd = "GetCredentialStatus"; // "GetCredentialStatus";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["Credential"] = to_json(data.credential);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::ClearCredential::Id: {
        Commands::ClearCredential::DecodableType data;
        cmd = "ClearCredential"; // "ClearCredential";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["Credential"] = to_json(data.credential);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::UnboltDoor::Id: {
        Commands::UnboltDoor::DecodableType data;
        cmd = "UnboltDoor"; // "UnboltDoor";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            if (data.PINCode.HasValue()) {
                try {
                    payload["PINCode"] = to_json(data.PINCode.Value());
                } catch (std::exception& ex) {
                    sl_log_warning(LOG_TAG, "Failed to add the command arguments value to json format: %s", ex.what());
                }
            }
        }
    } break;
    case Commands::SetAliroReaderConfig::Id: {
        Commands::SetAliroReaderConfig::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            Invoke_SetAliroReaderConfig(ctxt, data);
        }
    } break;
    case Commands::ClearAliroReaderConfig::Id: {
        Commands::ClearAliroReaderConfig::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            Invoke_ClearAliroReaderConfig(ctxt, data);
        }
    } break;
    }

    if (!cmd_response.empty() && !cmd.empty()) {
        send_unify_mqtt_cmd(ctxt, cmd, payload, cmd_response);
        sl_log_debug(LOG_TAG, " %s command send to unify dotdot data model, waiting for response", cmd);
    } else if (!cmd.empty()) {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    } else {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

void DoorLockClusterCommandHandler::command_response(const bridged_endpoint* ep, const std::string& cluster,
    const std::string& cmd_response, const nlohmann::json& unify_value,
    CommandHandler::Handle& cmd_handle)
{
    using namespace chip::app::Clusters::DoorLock;

    auto cluster_id = m_dev_translator.get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::DoorLock::Id)) {
        return;
    }

    auto command_id = m_dev_translator.get_command_id(cluster, cmd_response);

    if (!command_id.has_value()) {
        return;
    }

    std::string status = "0";
    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    const ConcreteCommandPath cpath = ConcreteCommandPath(node_matter_endpoint, Clusters::DoorLock::Id, command_id.value());
    switch (command_id.value()) {
    case Commands::LockDoor::Id: {
        status = unify_value["Status"];
        break;
    }
    case Commands::UnlockDoor::Id: {
        status = unify_value["Status"];
        break;
    }
    case Commands::UnlockWithTimeout::Id: {
        status = unify_value["Status"];
        break;
    }
    case Commands::SetWeekDaySchedule::Id: {
        status = unify_value["Status"];
        break;
    }
    case Commands::GetWeekDaySchedule::Id: {
        status = unify_value["Status"];
        break;
    }
    case Commands::ClearWeekDaySchedule::Id: {
        status = unify_value["Status"];
        break;
    }
    case Commands::SetYearDaySchedule::Id: {
        status = unify_value["Status"];
        break;
    }
    case Commands::GetYearDaySchedule::Id: {
        status = unify_value["Status"];
        break;
    }
    case Commands::ClearYearDaySchedule::Id: {
        status = unify_value["Status"];
        break;
    }
    case Commands::SetHolidaySchedule::Id: {
        status = unify_value["Status"];
        break;
    }
    case Commands::GetHolidaySchedule::Id: {
        status = unify_value["Status"];
        break;
    }
    case Commands::ClearHolidaySchedule::Id: {
        status = unify_value["Status"];
        break;
    }
    case Commands::SetUser::Id: {
        status = unify_value["Status"];
        break;
    }
    case Commands::GetUser::Id: {
        status = unify_value["Status"];
        break;
    }
    case Commands::ClearUser::Id: {
        status = unify_value["Status"];
        break;
    }
    case Commands::SetCredential::Id: {
        status = unify_value["Status"];
        break;
    }
    case Commands::GetCredentialStatus::Id: {
        status = unify_value["Status"];
        break;
    }
    case Commands::ClearCredential::Id: {
        status = unify_value["Status"];
        break;
    }
    case Commands::UnboltDoor::Id: {
        status = unify_value["Status"];
        break;
    }
    }

    // we retrieve the same handler used by invoke command to send command to UMB
    auto cmd_rsp_handler = cmd_handle.Get();

    // When the platform shutted down,
    // interaction model engine will invalidate all commandHandle to avoid dangling references.
    // We may receive the callback after it and should make it noop.
    if (cmd_rsp_handler == nullptr) {
        sl_log_warning("command_translator", "failed to get handler\n");
        return;
    }

    if (std::stoi(status) == 1) {
        cmd_rsp_handler->AddStatus(cpath, Protocols::InteractionModel::Status::Success);
    } else {
        cmd_rsp_handler->AddStatus(cpath, Protocols::InteractionModel::Status::Failure);
    }

    cmd_handle.Release();
    sl_log_debug("command_translator", "%s Command response handling Completed \n", cmd_response);
}
// Barrier Control : 259
void BarrierControlClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext& ctxt)
{
    using namespace chip::app::Clusters::BarrierControl;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node) {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    if (m_node_state_monitor.emulator().is_command_emulated(ctxt.mRequestPath)) {
        emulated_cmd_payload cdata;
        m_node_state_monitor.emulator().invoke_command(ctxt, cdata);
        // if cmd is not completely handled in invoke_command, then
        // cmd_emulation_completed should be set to false in emulated cmd handling
        if (!cdata.cmd_emulation_completed) {
            cmd = cdata.cmd;
            payload = cdata.payload;
        } else {
            return;
        }
    }

    switch (ctxt.mRequestPath.mCommandId) {
    case Commands::BarrierControlGoToPercent::Id: {
        Commands::BarrierControlGoToPercent::DecodableType data;
        cmd = "GoToPercent"; // "BarrierControlGoToPercent";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["PercentOpen"] = to_json(data.percentOpen);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::BarrierControlStop::Id: {
        Commands::BarrierControlStop::DecodableType data;
        cmd = "Stop"; // "BarrierControlStop";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
        }
    } break;
    }

    if (!cmd.empty()) {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    } else {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

// Thermostat : 513
void ThermostatClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext& ctxt)
{
    using namespace chip::app::Clusters::Thermostat;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node) {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    if (m_node_state_monitor.emulator().is_command_emulated(ctxt.mRequestPath)) {
        emulated_cmd_payload cdata;
        m_node_state_monitor.emulator().invoke_command(ctxt, cdata);
        // if cmd is not completely handled in invoke_command, then
        // cmd_emulation_completed should be set to false in emulated cmd handling
        if (!cdata.cmd_emulation_completed) {
            cmd = cdata.cmd;
            payload = cdata.payload;
        } else {
            return;
        }
    }

    switch (ctxt.mRequestPath.mCommandId) {
    case Commands::SetpointRaiseLower::Id: {
        Commands::SetpointRaiseLower::DecodableType data;
        cmd = "SetpointRaiseOrLower"; // "SetpointRaiseLower";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["Mode"] = to_json(data.mode);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["Amount"] = to_json(data.amount);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::SetWeeklySchedule::Id: {
        Commands::SetWeeklySchedule::DecodableType data;
        cmd = "SetWeeklySchedule"; // "SetWeeklySchedule";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["NumberOfTransitions"] = to_json(data.numberOfTransitionsForSequence);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["DayOfWeek"] = to_json(data.dayOfWeekForSequence);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["Mode"] = to_json(data.modeForSequence);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["Transitions"] = to_json(data.transitions);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::GetWeeklySchedule::Id: {
        Commands::GetWeeklySchedule::DecodableType data;
        cmd = "GetWeeklySchedule"; // "GetWeeklySchedule";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["DaysToReturn"] = to_json(data.daysToReturn);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["ModeToReturn"] = to_json(data.modeToReturn);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::ClearWeeklySchedule::Id: {
        Commands::ClearWeeklySchedule::DecodableType data;
        cmd = "ClearWeeklySchedule"; // "ClearWeeklySchedule";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
        }
    } break;
    case Commands::SetActiveScheduleRequest::Id: {
        Commands::SetActiveScheduleRequest::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            Invoke_SetActiveScheduleRequest(ctxt, data);
        }
    } break;
    case Commands::SetActivePresetRequest::Id: {
        Commands::SetActivePresetRequest::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            Invoke_SetActivePresetRequest(ctxt, data);
        }
    } break;
    case Commands::StartPresetsSchedulesEditRequest::Id: {
        Commands::StartPresetsSchedulesEditRequest::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            Invoke_StartPresetsSchedulesEditRequest(ctxt, data);
        }
    } break;
    case Commands::CancelPresetsSchedulesEditRequest::Id: {
        Commands::CancelPresetsSchedulesEditRequest::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            Invoke_CancelPresetsSchedulesEditRequest(ctxt, data);
        }
    } break;
    case Commands::CommitPresetsSchedulesRequest::Id: {
        Commands::CommitPresetsSchedulesRequest::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            Invoke_CommitPresetsSchedulesRequest(ctxt, data);
        }
    } break;
    case Commands::CancelSetActivePresetRequest::Id: {
        Commands::CancelSetActivePresetRequest::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            Invoke_CancelSetActivePresetRequest(ctxt, data);
        }
    } break;
    case Commands::SetTemperatureSetpointHoldPolicy::Id: {
        Commands::SetTemperatureSetpointHoldPolicy::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            Invoke_SetTemperatureSetpointHoldPolicy(ctxt, data);
        }
    } break;
    }

    if (!cmd.empty()) {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    } else {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

// Fan Control : 514
void FanControlClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext& ctxt)
{
    using namespace chip::app::Clusters::FanControl;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node) {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    if (m_node_state_monitor.emulator().is_command_emulated(ctxt.mRequestPath)) {
        emulated_cmd_payload cdata;
        m_node_state_monitor.emulator().invoke_command(ctxt, cdata);
        // if cmd is not completely handled in invoke_command, then
        // cmd_emulation_completed should be set to false in emulated cmd handling
        if (!cdata.cmd_emulation_completed) {
            cmd = cdata.cmd;
            payload = cdata.payload;
        } else {
            return;
        }
    }

    switch (ctxt.mRequestPath.mCommandId) {
    case Commands::Step::Id: {
        Commands::Step::DecodableType data;
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            Invoke_Step(ctxt, data);
        }
    } break;
    }

    if (!cmd.empty()) {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    } else {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

// Thermostat User Interface Configuration : 516
void ThermostatUserInterfaceConfigurationClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext& ctxt)
{
    using namespace chip::app::Clusters::ThermostatUserInterfaceConfiguration;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node) {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    if (m_node_state_monitor.emulator().is_command_emulated(ctxt.mRequestPath)) {
        emulated_cmd_payload cdata;
        m_node_state_monitor.emulator().invoke_command(ctxt, cdata);
        // if cmd is not completely handled in invoke_command, then
        // cmd_emulation_completed should be set to false in emulated cmd handling
        if (!cdata.cmd_emulation_completed) {
            cmd = cdata.cmd;
            payload = cdata.payload;
        } else {
            return;
        }
    }

    switch (ctxt.mRequestPath.mCommandId) {
    }

    if (!cmd.empty()) {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    } else {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

// Color Control : 768
void ColorControlClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext& ctxt)
{
    using namespace chip::app::Clusters::ColorControl;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node) {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    if (m_node_state_monitor.emulator().is_command_emulated(ctxt.mRequestPath)) {
        emulated_cmd_payload cdata;
        m_node_state_monitor.emulator().invoke_command(ctxt, cdata);
        // if cmd is not completely handled in invoke_command, then
        // cmd_emulation_completed should be set to false in emulated cmd handling
        if (!cdata.cmd_emulation_completed) {
            cmd = cdata.cmd;
            payload = cdata.payload;
        } else {
            return;
        }
    }

    switch (ctxt.mRequestPath.mCommandId) {
    case Commands::MoveToHue::Id: {
        Commands::MoveToHue::DecodableType data;
        cmd = "MoveToHue"; // "MoveToHue";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["Hue"] = to_json(data.hue);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["Direction"] = to_json(data.direction);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["TransitionTime"] = to_json(data.transitionTime);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::MoveHue::Id: {
        Commands::MoveHue::DecodableType data;
        cmd = "MoveHue"; // "MoveHue";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["MoveMode"] = to_json(data.moveMode);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["Rate"] = to_json(data.rate);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::StepHue::Id: {
        Commands::StepHue::DecodableType data;
        cmd = "StepHue"; // "StepHue";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["StepMode"] = to_json(data.stepMode);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["StepSize"] = to_json(data.stepSize);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["TransitionTime"] = to_json(data.transitionTime);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::MoveToSaturation::Id: {
        Commands::MoveToSaturation::DecodableType data;
        cmd = "MoveToSaturation"; // "MoveToSaturation";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["Saturation"] = to_json(data.saturation);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["TransitionTime"] = to_json(data.transitionTime);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::MoveSaturation::Id: {
        Commands::MoveSaturation::DecodableType data;
        cmd = "MoveSaturation"; // "MoveSaturation";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["MoveMode"] = to_json(data.moveMode);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["Rate"] = to_json(data.rate);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::StepSaturation::Id: {
        Commands::StepSaturation::DecodableType data;
        cmd = "StepSaturation"; // "StepSaturation";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["StepMode"] = to_json(data.stepMode);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["StepSize"] = to_json(data.stepSize);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["TransitionTime"] = to_json(data.transitionTime);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::MoveToHueAndSaturation::Id: {
        Commands::MoveToHueAndSaturation::DecodableType data;
        cmd = "MoveToHueAndSaturation"; // "MoveToHueAndSaturation";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["Hue"] = to_json(data.hue);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["Saturation"] = to_json(data.saturation);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["TransitionTime"] = to_json(data.transitionTime);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::MoveToColor::Id: {
        Commands::MoveToColor::DecodableType data;
        cmd = "MoveToColor"; // "MoveToColor";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["ColorX"] = to_json(data.colorX);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["ColorY"] = to_json(data.colorY);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["TransitionTime"] = to_json(data.transitionTime);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::MoveColor::Id: {
        Commands::MoveColor::DecodableType data;
        cmd = "MoveColor"; // "MoveColor";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["RateX"] = to_json(data.rateX);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["RateY"] = to_json(data.rateY);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::StepColor::Id: {
        Commands::StepColor::DecodableType data;
        cmd = "StepColor"; // "StepColor";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["StepX"] = to_json(data.stepX);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["StepY"] = to_json(data.stepY);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["TransitionTime"] = to_json(data.transitionTime);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::MoveToColorTemperature::Id: {
        Commands::MoveToColorTemperature::DecodableType data;
        cmd = "MoveToColorTemperature"; // "MoveToColorTemperature";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["ColorTemperatureMireds"] = to_json(data.colorTemperatureMireds);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["TransitionTime"] = to_json(data.transitionTime);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::EnhancedMoveToHue::Id: {
        Commands::EnhancedMoveToHue::DecodableType data;
        cmd = "EnhancedMoveToHue"; // "EnhancedMoveToHue";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["EnhancedHue"] = to_json(data.enhancedHue);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["Direction"] = to_json(data.direction);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["TransitionTime"] = to_json(data.transitionTime);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::EnhancedMoveHue::Id: {
        Commands::EnhancedMoveHue::DecodableType data;
        cmd = "EnhancedMoveHue"; // "EnhancedMoveHue";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["MoveMode"] = to_json(data.moveMode);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["Rate"] = to_json(data.rate);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::EnhancedStepHue::Id: {
        Commands::EnhancedStepHue::DecodableType data;
        cmd = "EnhancedStepHue"; // "EnhancedStepHue";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["StepMode"] = to_json(data.stepMode);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["StepSize"] = to_json(data.stepSize);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["TransitionTime"] = to_json(data.transitionTime);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::EnhancedMoveToHueAndSaturation::Id: {
        Commands::EnhancedMoveToHueAndSaturation::DecodableType data;
        cmd = "EnhancedMoveToHueAndSaturation"; // "EnhancedMoveToHueAndSaturation";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["EnhancedHue"] = to_json(data.enhancedHue);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["Saturation"] = to_json(data.saturation);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["TransitionTime"] = to_json(data.transitionTime);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::ColorLoopSet::Id: {
        Commands::ColorLoopSet::DecodableType data;
        cmd = "ColorLoopSet"; // "ColorLoopSet";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["UpdateFlags"] = to_json(data.updateFlags);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["Action"] = to_json(data.action);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["Direction"] = to_json(data.direction);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["Time"] = to_json(data.time);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["StartHue"] = to_json(data.startHue);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::StopMoveStep::Id: {
        Commands::StopMoveStep::DecodableType data;
        cmd = "StopMoveStep"; // "StopMoveStep";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::MoveColorTemperature::Id: {
        Commands::MoveColorTemperature::DecodableType data;
        cmd = "MoveColorTemperature"; // "MoveColorTemperature";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["MoveMode"] = to_json(data.moveMode);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["Rate"] = to_json(data.rate);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["ColorTemperatureMinimumMireds"] = to_json(data.colorTemperatureMinimumMireds);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["ColorTemperatureMaximumMireds"] = to_json(data.colorTemperatureMaximumMireds);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    case Commands::StepColorTemperature::Id: {
        Commands::StepColorTemperature::DecodableType data;
        cmd = "StepColorTemperature"; // "StepColorTemperature";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["StepMode"] = to_json(data.stepMode);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["StepSize"] = to_json(data.stepSize);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["TransitionTime"] = to_json(data.transitionTime);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["ColorTemperatureMinimumMireds"] = to_json(data.colorTemperatureMinimumMireds);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["ColorTemperatureMaximumMireds"] = to_json(data.colorTemperatureMaximumMireds);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsMask"] = to_json(data.optionsMask);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["OptionsOverride"] = to_json(data.optionsOverride);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    }

    if (!cmd.empty()) {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    } else {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

// Illuminance Measurement : 1024
void IlluminanceMeasurementClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext& ctxt)
{
    using namespace chip::app::Clusters::IlluminanceMeasurement;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node) {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    if (m_node_state_monitor.emulator().is_command_emulated(ctxt.mRequestPath)) {
        emulated_cmd_payload cdata;
        m_node_state_monitor.emulator().invoke_command(ctxt, cdata);
        // if cmd is not completely handled in invoke_command, then
        // cmd_emulation_completed should be set to false in emulated cmd handling
        if (!cdata.cmd_emulation_completed) {
            cmd = cdata.cmd;
            payload = cdata.payload;
        } else {
            return;
        }
    }

    switch (ctxt.mRequestPath.mCommandId) {
    }

    if (!cmd.empty()) {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    } else {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

// Temperature Measurement : 1026
void TemperatureMeasurementClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext& ctxt)
{
    using namespace chip::app::Clusters::TemperatureMeasurement;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node) {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    if (m_node_state_monitor.emulator().is_command_emulated(ctxt.mRequestPath)) {
        emulated_cmd_payload cdata;
        m_node_state_monitor.emulator().invoke_command(ctxt, cdata);
        // if cmd is not completely handled in invoke_command, then
        // cmd_emulation_completed should be set to false in emulated cmd handling
        if (!cdata.cmd_emulation_completed) {
            cmd = cdata.cmd;
            payload = cdata.payload;
        } else {
            return;
        }
    }

    switch (ctxt.mRequestPath.mCommandId) {
    }

    if (!cmd.empty()) {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    } else {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

// Pressure Measurement : 1027
void PressureMeasurementClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext& ctxt)
{
    using namespace chip::app::Clusters::PressureMeasurement;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node) {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    if (m_node_state_monitor.emulator().is_command_emulated(ctxt.mRequestPath)) {
        emulated_cmd_payload cdata;
        m_node_state_monitor.emulator().invoke_command(ctxt, cdata);
        // if cmd is not completely handled in invoke_command, then
        // cmd_emulation_completed should be set to false in emulated cmd handling
        if (!cdata.cmd_emulation_completed) {
            cmd = cdata.cmd;
            payload = cdata.payload;
        } else {
            return;
        }
    }

    switch (ctxt.mRequestPath.mCommandId) {
    }

    if (!cmd.empty()) {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    } else {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

// Flow Measurement : 1028
void FlowMeasurementClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext& ctxt)
{
    using namespace chip::app::Clusters::FlowMeasurement;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node) {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    if (m_node_state_monitor.emulator().is_command_emulated(ctxt.mRequestPath)) {
        emulated_cmd_payload cdata;
        m_node_state_monitor.emulator().invoke_command(ctxt, cdata);
        // if cmd is not completely handled in invoke_command, then
        // cmd_emulation_completed should be set to false in emulated cmd handling
        if (!cdata.cmd_emulation_completed) {
            cmd = cdata.cmd;
            payload = cdata.payload;
        } else {
            return;
        }
    }

    switch (ctxt.mRequestPath.mCommandId) {
    }

    if (!cmd.empty()) {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    } else {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

// Relative Humidity Measurement : 1029
void RelativeHumidityMeasurementClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext& ctxt)
{
    using namespace chip::app::Clusters::RelativeHumidityMeasurement;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node) {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    if (m_node_state_monitor.emulator().is_command_emulated(ctxt.mRequestPath)) {
        emulated_cmd_payload cdata;
        m_node_state_monitor.emulator().invoke_command(ctxt, cdata);
        // if cmd is not completely handled in invoke_command, then
        // cmd_emulation_completed should be set to false in emulated cmd handling
        if (!cdata.cmd_emulation_completed) {
            cmd = cdata.cmd;
            payload = cdata.payload;
        } else {
            return;
        }
    }

    switch (ctxt.mRequestPath.mCommandId) {
    }

    if (!cmd.empty()) {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    } else {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

// Occupancy Sensing : 1030
void OccupancySensingClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext& ctxt)
{
    using namespace chip::app::Clusters::OccupancySensing;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node) {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    if (m_node_state_monitor.emulator().is_command_emulated(ctxt.mRequestPath)) {
        emulated_cmd_payload cdata;
        m_node_state_monitor.emulator().invoke_command(ctxt, cdata);
        // if cmd is not completely handled in invoke_command, then
        // cmd_emulation_completed should be set to false in emulated cmd handling
        if (!cdata.cmd_emulation_completed) {
            cmd = cdata.cmd;
            payload = cdata.payload;
        } else {
            return;
        }
    }

    switch (ctxt.mRequestPath.mCommandId) {
    }

    if (!cmd.empty()) {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    } else {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}

// Electrical Measurement : 2820
void ElectricalMeasurementClusterCommandHandler::InvokeCommand(CommandHandlerInterface::HandlerContext& ctxt)
{
    using namespace chip::app::Clusters::ElectricalMeasurement;

    auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
    if (!unify_node) {
        sl_log_info(LOG_TAG, "The endpoint [%i] is not a part of unify matter bridge node", ctxt.mRequestPath.mEndpointId);
        return;
    }

    std::string cmd;
    nlohmann::json payload = {};

    if (m_node_state_monitor.emulator().is_command_emulated(ctxt.mRequestPath)) {
        emulated_cmd_payload cdata;
        m_node_state_monitor.emulator().invoke_command(ctxt, cdata);
        // if cmd is not completely handled in invoke_command, then
        // cmd_emulation_completed should be set to false in emulated cmd handling
        if (!cdata.cmd_emulation_completed) {
            cmd = cdata.cmd;
            payload = cdata.payload;
        } else {
            return;
        }
    }

    switch (ctxt.mRequestPath.mCommandId) {
    case Commands::GetProfileInfoCommand::Id: {
        Commands::GetProfileInfoCommand::DecodableType data;
        cmd = "GetProfileInfoResponse"; // "GetProfileInfoCommand";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
        }
    } break;
    case Commands::GetMeasurementProfileCommand::Id: {
        Commands::GetMeasurementProfileCommand::DecodableType data;
        cmd = "GetMeasurementProfileResponse"; // "GetMeasurementProfileCommand";
        if (DataModel::Decode(ctxt.GetReader(), data) == CHIP_NO_ERROR) {
            try {
                payload["StartTime"] = to_json(data.attributeId);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["Status"] = to_json(data.startTime);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
            try {
                payload["ProfileIntervalPeriod"] = to_json(data.numberOfIntervals);
            } catch (std::exception& ex) {
                sl_log_warning(LOG_TAG, "Failed to add the command argument value to json format: %s", ex.what());
            }
        }
    } break;
    }

    if (!cmd.empty()) {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Success);
        send_unify_mqtt_cmd(ctxt, cmd, payload);
        sl_log_debug(LOG_TAG, "Mapped [%] command to unify dotdot data model", cmd.c_str());
    } else {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
    ctxt.SetCommandHandled();
}
