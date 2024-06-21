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
#include "app/data-model/NullObject.h"
#include "chip_type_traits.hpp"
#include "zcl_global_types.hpp"

// Default translation
template <typename T>
std::optional<T> inline from_json(const nlohmann::json& value)
{
    if constexpr (is_nullable<T>::value) {
        auto v = from_json<typename T::UnderlyingType>(value);
        if (v.has_value()) {
            return T(v.value());
        } else {
            return std::nullopt;
        }
    } else {
        return std::optional<T>(value);
    }
}

template <>
std::optional<chip::Span<const char>> inline from_json(const nlohmann::json& value)
{
    std::string s = value.get<std::string>();
    chip::Span<const char> span(s.c_str(), s.length());
    return span;
}

/***************************** Bitmap Converters **************/

template <>
inline std::optional<Identify::EffectIdentifierEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, Identify::EffectIdentifierEnum> table = {
        { "Blink", Identify::EffectIdentifierEnum::kBlink },
        { "Breathe", Identify::EffectIdentifierEnum::kBreathe },
        { "Okay", Identify::EffectIdentifierEnum::kOkay },
        { "ChannelChange", Identify::EffectIdentifierEnum::kChannelChange },
        { "FinishEffect", Identify::EffectIdentifierEnum::kFinishEffect },
        { "StopEffect", Identify::EffectIdentifierEnum::kStopEffect },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<Identify::EffectVariantEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, Identify::EffectVariantEnum> table = {
        { "Default", Identify::EffectVariantEnum::kDefault },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<Identify::IdentifyTypeEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, Identify::IdentifyTypeEnum> table = {
        { "None", Identify::IdentifyTypeEnum::kNone },
        { "LightOutput", Identify::IdentifyTypeEnum::kLightOutput },
        { "VisibleIndicator", Identify::IdentifyTypeEnum::kVisibleIndicator },
        { "AudibleBeep", Identify::IdentifyTypeEnum::kAudibleBeep },
        { "Display", Identify::IdentifyTypeEnum::kDisplay },
        { "Actuator", Identify::IdentifyTypeEnum::kActuator },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
/***************************** Bitmap Converters **************/
template <>
inline std::optional<chip::BitMask<Groups::Feature>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<Groups::Feature> r;
    r.SetField(Groups::Feature::kGroupNames, obj.value("GroupNames", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<Groups::NameSupportBitmap>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<Groups::NameSupportBitmap> r;
    r.SetField(Groups::NameSupportBitmap::kGroupNames, obj.value("GroupNames", false));
    return r;
}

/***************************** Bitmap Converters **************/
template <>
inline std::optional<chip::BitMask<OnOff::Feature>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<OnOff::Feature> r;
    r.SetField(OnOff::Feature::kLighting, obj.value("Lighting", false));
    r.SetField(OnOff::Feature::kDeadFrontBehavior, obj.value("DeadFrontBehavior", false));
    r.SetField(OnOff::Feature::kOffOnly, obj.value("OffOnly", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<OnOff::OnOffControlBitmap>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<OnOff::OnOffControlBitmap> r;
    r.SetField(OnOff::OnOffControlBitmap::kAcceptOnlyWhenOn, obj.value("AcceptOnlyWhenOn", false));
    return r;
}

template <>
inline std::optional<OnOff::DelayedAllOffEffectVariantEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, OnOff::DelayedAllOffEffectVariantEnum> table = {
        { "DelayedOffFastFade", OnOff::DelayedAllOffEffectVariantEnum::kDelayedOffFastFade },
        { "NoFade", OnOff::DelayedAllOffEffectVariantEnum::kNoFade },
        { "DelayedOffSlowFade", OnOff::DelayedAllOffEffectVariantEnum::kDelayedOffSlowFade },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<OnOff::DyingLightEffectVariantEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, OnOff::DyingLightEffectVariantEnum> table = {
        { "DyingLightFadeOff", OnOff::DyingLightEffectVariantEnum::kDyingLightFadeOff },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<OnOff::EffectIdentifierEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, OnOff::EffectIdentifierEnum> table = {
        { "DelayedAllOff", OnOff::EffectIdentifierEnum::kDelayedAllOff },
        { "DyingLight", OnOff::EffectIdentifierEnum::kDyingLight },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<OnOff::StartUpOnOffEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, OnOff::StartUpOnOffEnum> table = {
        { "SetOnOffTo0", OnOff::StartUpOnOffEnum::kOff },
        { "SetOnOffTo1", OnOff::StartUpOnOffEnum::kOn },
        { "TogglePreviousOnOff", OnOff::StartUpOnOffEnum::kToggle },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
/***************************** Bitmap Converters **************/
template <>
inline std::optional<chip::BitMask<LevelControl::Feature>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<LevelControl::Feature> r;
    r.SetField(LevelControl::Feature::kOnOff, obj.value("OnOff", false));
    r.SetField(LevelControl::Feature::kLighting, obj.value("Lighting", false));
    r.SetField(LevelControl::Feature::kFrequency, obj.value("Frequency", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<LevelControl::OptionsBitmap>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<LevelControl::OptionsBitmap> r;
    r.SetField(LevelControl::OptionsBitmap::kExecuteIfOff, obj.value("ExecuteIfOff", false));
    r.SetField(LevelControl::OptionsBitmap::kCoupleColorTempToLevel, obj.value("CoupleColorTempToLevel", false));
    return r;
}

template <>
inline std::optional<LevelControl::MoveModeEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, LevelControl::MoveModeEnum> table = {
        { "Up", LevelControl::MoveModeEnum::kUp },
        { "Down", LevelControl::MoveModeEnum::kDown },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<LevelControl::StepModeEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, LevelControl::StepModeEnum> table = {
        { "Up", LevelControl::StepModeEnum::kUp },
        { "Down", LevelControl::StepModeEnum::kDown },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
/***************************** Bitmap Converters **************/
template <>
inline std::optional<chip::BitMask<DoorLock::DaysMaskMap>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<DoorLock::DaysMaskMap> r;
    r.SetField(DoorLock::DaysMaskMap::kSunday, obj.value("Sun", false));
    r.SetField(DoorLock::DaysMaskMap::kMonday, obj.value("Mon", false));
    r.SetField(DoorLock::DaysMaskMap::kTuesday, obj.value("Tue", false));
    r.SetField(DoorLock::DaysMaskMap::kWednesday, obj.value("Wed", false));
    r.SetField(DoorLock::DaysMaskMap::kThursday, obj.value("Thu", false));
    r.SetField(DoorLock::DaysMaskMap::kFriday, obj.value("Fri", false));
    r.SetField(DoorLock::DaysMaskMap::kSaturday, obj.value("Sat", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<DoorLock::DlCredentialRuleMask>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<DoorLock::DlCredentialRuleMask> r;
    r.SetField(DoorLock::DlCredentialRuleMask::kSingle, obj.value("Single", false));
    r.SetField(DoorLock::DlCredentialRuleMask::kDual, obj.value("Dual", false));
    r.SetField(DoorLock::DlCredentialRuleMask::kTri, obj.value("Tri", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<DoorLock::DlCredentialRulesSupport>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<DoorLock::DlCredentialRulesSupport> r;
    r.SetField(DoorLock::DlCredentialRulesSupport::kSingle, obj.value("Single", false));
    r.SetField(DoorLock::DlCredentialRulesSupport::kDual, obj.value("Dual", false));
    r.SetField(DoorLock::DlCredentialRulesSupport::kTri, obj.value("Tri", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<DoorLock::DlDefaultConfigurationRegister>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<DoorLock::DlDefaultConfigurationRegister> r;
    r.SetField(DoorLock::DlDefaultConfigurationRegister::kEnableLocalProgrammingEnabled,
        obj.value("DefaultEnableLocalProgrammingAttributeIsEnabled", false));
    r.SetField(DoorLock::DlDefaultConfigurationRegister::kKeypadInterfaceDefaultAccessEnabled,
        obj.value("DefaultKeypadInterfaceIsEnabled", false));
    r.SetField(DoorLock::DlDefaultConfigurationRegister::kRemoteInterfaceDefaultAccessIsEnabled,
        obj.value("DefaultRFInterfaceIsEnabled", false));
    r.SetField(DoorLock::DlDefaultConfigurationRegister::kSoundEnabled, obj.value("DefaultSoundVolumeIsEnabled", false));
    r.SetField(DoorLock::DlDefaultConfigurationRegister::kAutoRelockTimeSet, obj.value("DefaultAutoRelockTimeIsEnabled", false));
    r.SetField(DoorLock::DlDefaultConfigurationRegister::kLEDSettingsSet, obj.value("DefaultLEDSettingsIsEnabled", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<DoorLock::DlKeypadOperationEventMask>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<DoorLock::DlKeypadOperationEventMask> r;
    r.SetField(DoorLock::DlKeypadOperationEventMask::kUnknown, obj.value("KeypadOpUnknownOrMS", false));
    r.SetField(DoorLock::DlKeypadOperationEventMask::kLock, obj.value("KeypadOpLock", false));
    r.SetField(DoorLock::DlKeypadOperationEventMask::kUnlock, obj.value("KeypadOpUnlock", false));
    r.SetField(DoorLock::DlKeypadOperationEventMask::kLockInvalidPIN, obj.value("KeypadOpLockErrorInvalidPIN", false));
    r.SetField(DoorLock::DlKeypadOperationEventMask::kLockInvalidSchedule, obj.value("KeypadOpLockErrorInvalidSchedule", false));
    r.SetField(DoorLock::DlKeypadOperationEventMask::kUnlockInvalidCode, obj.value("KeypadOpUnlockInvalidPIN", false));
    r.SetField(DoorLock::DlKeypadOperationEventMask::kUnlockInvalidSchedule, obj.value("KeypadOpUnlockInvalidSchedule", false));
    r.SetField(DoorLock::DlKeypadOperationEventMask::kNonAccessUserOpEvent, obj.value("KeypadOpNonAccessUser", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<DoorLock::DlKeypadProgrammingEventMask>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<DoorLock::DlKeypadProgrammingEventMask> r;
    r.SetField(DoorLock::DlKeypadProgrammingEventMask::kUnknown, obj.value("KeypadProgUnknownOrMS", false));
    r.SetField(DoorLock::DlKeypadProgrammingEventMask::kProgrammingPINChanged, obj.value("KeypadProgMasterCodeChanged", false));
    r.SetField(DoorLock::DlKeypadProgrammingEventMask::kPINAdded, obj.value("KeypadProgPINAdded", false));
    r.SetField(DoorLock::DlKeypadProgrammingEventMask::kPINCleared, obj.value("KeypadProgPINDeleted", false));
    r.SetField(DoorLock::DlKeypadProgrammingEventMask::kPINChanged, obj.value("KeypadProgPINChanged", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<DoorLock::DlLocalProgrammingFeatures>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<DoorLock::DlLocalProgrammingFeatures> r;
    r.SetField(DoorLock::DlLocalProgrammingFeatures::kAddUsersCredentialsSchedulesLocally,
        obj.value("AddUsersCredentialsSchedulesLocally", false));
    r.SetField(DoorLock::DlLocalProgrammingFeatures::kModifyUsersCredentialsSchedulesLocally,
        obj.value("ModifyUsersCredentialsSchedulesLocally", false));
    r.SetField(DoorLock::DlLocalProgrammingFeatures::kClearUsersCredentialsSchedulesLocally,
        obj.value("ClearUsersCredentialsSchedulesLocally", false));
    r.SetField(DoorLock::DlLocalProgrammingFeatures::kAdjustLockSettingsLocally, obj.value("AdjustLockSettingsLocally", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<DoorLock::DlManualOperationEventMask>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<DoorLock::DlManualOperationEventMask> r;
    r.SetField(DoorLock::DlManualOperationEventMask::kUnknown, obj.value("ManualOpUnknownOrMS", false));
    r.SetField(DoorLock::DlManualOperationEventMask::kThumbturnLock, obj.value("ManualOpThumbturnLock", false));
    r.SetField(DoorLock::DlManualOperationEventMask::kThumbturnUnlock, obj.value("ManualOpThumbturnUnlock", false));
    r.SetField(DoorLock::DlManualOperationEventMask::kOneTouchLock, obj.value("ManualOpOneTouchLock", false));
    r.SetField(DoorLock::DlManualOperationEventMask::kKeyLock, obj.value("ManualOpKeyLock", false));
    r.SetField(DoorLock::DlManualOperationEventMask::kKeyUnlock, obj.value("ManualOpKeyUnlock", false));
    r.SetField(DoorLock::DlManualOperationEventMask::kAutoLock, obj.value("ManualOpAutoLock", false));
    r.SetField(DoorLock::DlManualOperationEventMask::kScheduleLock, obj.value("ManualOpScheduleLock", false));
    r.SetField(DoorLock::DlManualOperationEventMask::kScheduleUnlock, obj.value("ManualOpScheduleUnlock", false));
    r.SetField(DoorLock::DlManualOperationEventMask::kManualLock, obj.value("ManualOpLock", false));
    r.SetField(DoorLock::DlManualOperationEventMask::kManualUnlock, obj.value("ManualOpUnlock", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<DoorLock::DlRFIDOperationEventMask>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<DoorLock::DlRFIDOperationEventMask> r;
    r.SetField(DoorLock::DlRFIDOperationEventMask::kUnknown, obj.value("RFIDOpUnknownOrMS", false));
    r.SetField(DoorLock::DlRFIDOperationEventMask::kLock, obj.value("RFIDOpLock", false));
    r.SetField(DoorLock::DlRFIDOperationEventMask::kUnlock, obj.value("RFIDOpUnlock", false));
    r.SetField(DoorLock::DlRFIDOperationEventMask::kLockInvalidRFID, obj.value("RFIDOpLockErrorInvalidRFID", false));
    r.SetField(DoorLock::DlRFIDOperationEventMask::kLockInvalidSchedule, obj.value("RFIDOpLockErrorInvalidSchedule", false));
    r.SetField(DoorLock::DlRFIDOperationEventMask::kUnlockInvalidRFID, obj.value("RFIDOpUnlockErrorInvalidRFID", false));
    r.SetField(DoorLock::DlRFIDOperationEventMask::kUnlockInvalidSchedule, obj.value("RFIDOpUnlockErrorInvalidSchedule", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<DoorLock::DlRFIDProgrammingEventMask>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<DoorLock::DlRFIDProgrammingEventMask> r;
    r.SetField(DoorLock::DlRFIDProgrammingEventMask::kUnknown, obj.value("RFIDProgUnknownOrMS", false));
    r.SetField(DoorLock::DlRFIDProgrammingEventMask::kRFIDCodeAdded, obj.value("RFIDProgRFIDAdded", false));
    r.SetField(DoorLock::DlRFIDProgrammingEventMask::kRFIDCodeCleared, obj.value("RFIDProgRFIDDeleted", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<DoorLock::DlRemoteOperationEventMask>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<DoorLock::DlRemoteOperationEventMask> r;
    r.SetField(DoorLock::DlRemoteOperationEventMask::kUnknown, obj.value("RFOpUnknownOrMS", false));
    r.SetField(DoorLock::DlRemoteOperationEventMask::kLock, obj.value("RFOpLock", false));
    r.SetField(DoorLock::DlRemoteOperationEventMask::kUnlock, obj.value("RFOpUnlock", false));
    r.SetField(DoorLock::DlRemoteOperationEventMask::kLockInvalidCode, obj.value("RFOpLockErrorInvalidCode", false));
    r.SetField(DoorLock::DlRemoteOperationEventMask::kLockInvalidSchedule, obj.value("RFOpLockErrorInvalidSchedule", false));
    r.SetField(DoorLock::DlRemoteOperationEventMask::kUnlockInvalidCode, obj.value("RFOpUnlockInvalidCode", false));
    r.SetField(DoorLock::DlRemoteOperationEventMask::kUnlockInvalidSchedule, obj.value("RFOpUnlockInvalidSchedule", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<DoorLock::DlRemoteProgrammingEventMask>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<DoorLock::DlRemoteProgrammingEventMask> r;
    r.SetField(DoorLock::DlRemoteProgrammingEventMask::kUnknown, obj.value("RFProgUnknownOrMS", false));
    r.SetField(DoorLock::DlRemoteProgrammingEventMask::kProgrammingPINChanged, obj.value("", false));
    r.SetField(DoorLock::DlRemoteProgrammingEventMask::kPINAdded, obj.value("RFProgPINAdded", false));
    r.SetField(DoorLock::DlRemoteProgrammingEventMask::kPINCleared, obj.value("RFProgPINDeleted", false));
    r.SetField(DoorLock::DlRemoteProgrammingEventMask::kPINChanged, obj.value("RFProgPINChanged", false));
    r.SetField(DoorLock::DlRemoteProgrammingEventMask::kRFIDCodeAdded, obj.value("RFProgRFIDAdded", false));
    r.SetField(DoorLock::DlRemoteProgrammingEventMask::kRFIDCodeCleared, obj.value("RFProgRFIDDeleted", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<DoorLock::DlSupportedOperatingModes>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<DoorLock::DlSupportedOperatingModes> r;
    r.SetField(DoorLock::DlSupportedOperatingModes::kNormal, obj.value("NormalModeSupported", false));
    r.SetField(DoorLock::DlSupportedOperatingModes::kVacation, obj.value("VacationModeSupported", false));
    r.SetField(DoorLock::DlSupportedOperatingModes::kPrivacy, obj.value("PrivacyModeSupported", false));
    r.SetField(DoorLock::DlSupportedOperatingModes::kNoRemoteLockUnlock, obj.value("NoRFLockOrUnlockModeSupported", false));
    r.SetField(DoorLock::DlSupportedOperatingModes::kPassage, obj.value("PassageModeSupported", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<DoorLock::DoorLockDayOfWeek>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<DoorLock::DoorLockDayOfWeek> r;
    r.SetField(DoorLock::DoorLockDayOfWeek::kSunday, obj.value("Sunday", false));
    r.SetField(DoorLock::DoorLockDayOfWeek::kMonday, obj.value("Monday", false));
    r.SetField(DoorLock::DoorLockDayOfWeek::kTuesday, obj.value("Tuesday", false));
    r.SetField(DoorLock::DoorLockDayOfWeek::kWednesday, obj.value("Wednesday", false));
    r.SetField(DoorLock::DoorLockDayOfWeek::kThursday, obj.value("Thursday", false));
    r.SetField(DoorLock::DoorLockDayOfWeek::kFriday, obj.value("Friday", false));
    r.SetField(DoorLock::DoorLockDayOfWeek::kSaturday, obj.value("Saturday", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<DoorLock::Feature>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<DoorLock::Feature> r;
    r.SetField(DoorLock::Feature::kPinCredential, obj.value("PIN Credential", false));
    r.SetField(DoorLock::Feature::kRfidCredential, obj.value("RFID Credential", false));
    r.SetField(DoorLock::Feature::kFingerCredentials, obj.value("Finger Credentials", false));
    r.SetField(DoorLock::Feature::kLogging, obj.value("Logging", false));
    r.SetField(DoorLock::Feature::kWeekDayAccessSchedules, obj.value("Week Day Access Schedules", false));
    r.SetField(DoorLock::Feature::kDoorPositionSensor, obj.value("Door Position Sensor", false));
    r.SetField(DoorLock::Feature::kFaceCredentials, obj.value("Face Credentials", false));
    r.SetField(DoorLock::Feature::kCredentialsOverTheAirAccess, obj.value("Credentials Over-the-Air Access", false));
    r.SetField(DoorLock::Feature::kUser, obj.value("User", false));
    r.SetField(DoorLock::Feature::kNotification, obj.value("Notification", false));
    r.SetField(DoorLock::Feature::kYearDayAccessSchedules, obj.value("Year Day Access Schedules", false));
    r.SetField(DoorLock::Feature::kHolidaySchedules, obj.value("Holiday Schedules", false));
    r.SetField(DoorLock::Feature::kUnbolt, obj.value("Unbolt", false));
    r.SetField(DoorLock::Feature::kAliroProvisioning, obj.value("AliroProvisioning", false));
    r.SetField(DoorLock::Feature::kAliroBLEUWB, obj.value("AliroBLEUWB", false));
    return r;
}

template <>
inline std::optional<DoorLock::AlarmCodeEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::AlarmCodeEnum> table = {
        { "LockJammed", DoorLock::AlarmCodeEnum::kLockJammed },
        { "LockFactoryReset", DoorLock::AlarmCodeEnum::kLockFactoryReset },
        { "LockRadioPowerCycled", DoorLock::AlarmCodeEnum::kLockRadioPowerCycled },
        { "WrongCodeEntryLimit", DoorLock::AlarmCodeEnum::kWrongCodeEntryLimit },
        { "FrontEsceutcheonRemoved", DoorLock::AlarmCodeEnum::kFrontEsceutcheonRemoved },
        { "DoorForcedOpen", DoorLock::AlarmCodeEnum::kDoorForcedOpen },
        { "DoorAjar", DoorLock::AlarmCodeEnum::kDoorAjar },
        { "ForcedUser", DoorLock::AlarmCodeEnum::kForcedUser },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::CredentialRuleEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::CredentialRuleEnum> table = {
        { "Single", DoorLock::CredentialRuleEnum::kSingle },
        { "Dual", DoorLock::CredentialRuleEnum::kDual },
        { "Tri", DoorLock::CredentialRuleEnum::kTri },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::CredentialTypeEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::CredentialTypeEnum> table = {
        { "ProgrammingPIN", DoorLock::CredentialTypeEnum::kProgrammingPIN },
        { "PIN", DoorLock::CredentialTypeEnum::kPin },
        { "RFID", DoorLock::CredentialTypeEnum::kRfid },
        { "Fingerprint", DoorLock::CredentialTypeEnum::kFingerprint },
        { "FingerVein", DoorLock::CredentialTypeEnum::kFingerVein },
        { "Face", DoorLock::CredentialTypeEnum::kFace },
        { "", DoorLock::CredentialTypeEnum::kAliroCredentialIssuerKey },
        { "", DoorLock::CredentialTypeEnum::kAliroEvictableEndpointKey },
        { "", DoorLock::CredentialTypeEnum::kAliroNonEvictableEndpointKey },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DataOperationTypeEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::DataOperationTypeEnum> table = {
        { "Add", DoorLock::DataOperationTypeEnum::kAdd },
        { "Clear", DoorLock::DataOperationTypeEnum::kClear },
        { "Modify", DoorLock::DataOperationTypeEnum::kModify },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DlLockState> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::DlLockState> table = {
        { "NotFullyLocked", DoorLock::DlLockState::kNotFullyLocked },
        { "Locked", DoorLock::DlLockState::kLocked },
        { "Unlocked", DoorLock::DlLockState::kUnlocked },
        { "Unlatched", DoorLock::DlLockState::kUnlatched },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DlLockType> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::DlLockType> table = {
        { "DeadBolt", DoorLock::DlLockType::kDeadBolt },
        { "Magnetic", DoorLock::DlLockType::kMagnetic },
        { "Other", DoorLock::DlLockType::kOther },
        { "Mortise", DoorLock::DlLockType::kMortise },
        { "Rim", DoorLock::DlLockType::kRim },
        { "LatchBolt", DoorLock::DlLockType::kLatchBolt },
        { "CylindricalLock", DoorLock::DlLockType::kCylindricalLock },
        { "TubularLock", DoorLock::DlLockType::kTubularLock },
        { "InterconnectedLock", DoorLock::DlLockType::kInterconnectedLock },
        { "DeadLatch", DoorLock::DlLockType::kDeadLatch },
        { "DoorFurniture", DoorLock::DlLockType::kDoorFurniture },
        { "Eurocylinder", DoorLock::DlLockType::kEurocylinder },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DlStatus> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::DlStatus> table = {
        { "Success", DoorLock::DlStatus::kSuccess },
        { "Failure", DoorLock::DlStatus::kFailure },
        { "Duplicate", DoorLock::DlStatus::kDuplicate },
        { "Occupied", DoorLock::DlStatus::kOccupied },
        { "InvalidField", DoorLock::DlStatus::kInvalidField },
        { "ResourceExhausted", DoorLock::DlStatus::kResourceExhausted },
        { "NotFound", DoorLock::DlStatus::kNotFound },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DoorLockOperationEventCode> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::DoorLockOperationEventCode> table = {
        { "UnknownOrMS", DoorLock::DoorLockOperationEventCode::kUnknownOrMfgSpecific },
        { "Lock", DoorLock::DoorLockOperationEventCode::kLock },
        { "Unlock", DoorLock::DoorLockOperationEventCode::kUnlock },
        { "LockFailureInvalidPINOrID", DoorLock::DoorLockOperationEventCode::kLockInvalidPinOrId },
        { "LockFailureInvalidSchedule", DoorLock::DoorLockOperationEventCode::kLockInvalidSchedule },
        { "UnlockFailureInvalidPINOrID", DoorLock::DoorLockOperationEventCode::kUnlockInvalidPinOrId },
        { "UnlockFailureInvalidSchedule", DoorLock::DoorLockOperationEventCode::kUnlockInvalidSchedule },
        { "OneTouchLock", DoorLock::DoorLockOperationEventCode::kOneTouchLock },
        { "KeyLock", DoorLock::DoorLockOperationEventCode::kKeyLock },
        { "KeyUnlock", DoorLock::DoorLockOperationEventCode::kKeyUnlock },
        { "AutoLock", DoorLock::DoorLockOperationEventCode::kAutoLock },
        { "ScheduleLock", DoorLock::DoorLockOperationEventCode::kScheduleLock },
        { "ScheduleUnlock", DoorLock::DoorLockOperationEventCode::kScheduleUnlock },
        { "ManualLock", DoorLock::DoorLockOperationEventCode::kManualLock },
        { "ManualUnlock", DoorLock::DoorLockOperationEventCode::kManualUnlock },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DoorLockProgrammingEventCode> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::DoorLockProgrammingEventCode> table = {
        { "UnknownOrMS", DoorLock::DoorLockProgrammingEventCode::kUnknownOrMfgSpecific },
        { "MasterCodeChanged", DoorLock::DoorLockProgrammingEventCode::kMasterCodeChanged },
        { "PINCodeAdded", DoorLock::DoorLockProgrammingEventCode::kPinAdded },
        { "PINCodeDeleted", DoorLock::DoorLockProgrammingEventCode::kPinDeleted },
        { "PINCodeChanged", DoorLock::DoorLockProgrammingEventCode::kPinChanged },
        { "RFIDCodeAdded", DoorLock::DoorLockProgrammingEventCode::kIdAdded },
        { "RFIDCodeDeleted", DoorLock::DoorLockProgrammingEventCode::kIdDeleted },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DoorLockSetPinOrIdStatus> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::DoorLockSetPinOrIdStatus> table = {
        { "Success", DoorLock::DoorLockSetPinOrIdStatus::kSuccess },
        { "GeneralFailure", DoorLock::DoorLockSetPinOrIdStatus::kGeneralFailure },
        { "MemoryFull", DoorLock::DoorLockSetPinOrIdStatus::kMemoryFull },
        { "DuplicateCode", DoorLock::DoorLockSetPinOrIdStatus::kDuplicateCodeError },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DoorLockUserStatus> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::DoorLockUserStatus> table = {
        { "Available", DoorLock::DoorLockUserStatus::kAvailable },
        { "OccupiedEnabled", DoorLock::DoorLockUserStatus::kOccupiedEnabled },
        { "OccupiedDisabled", DoorLock::DoorLockUserStatus::kOccupiedDisabled },
        { "NotSupported", DoorLock::DoorLockUserStatus::kNotSupported },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DoorLockUserType> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::DoorLockUserType> table = {
        { "UnrestrictedUser", DoorLock::DoorLockUserType::kUnrestricted },
        { "YearDayScheduleUser", DoorLock::DoorLockUserType::kYearDayScheduleUser },
        { "WeekDayScheduleUser", DoorLock::DoorLockUserType::kWeekDayScheduleUser },
        { "MasterUser", DoorLock::DoorLockUserType::kMasterUser },
        { "NonAccessUser", DoorLock::DoorLockUserType::kNonAccessUser },
        { "ForcedUser", DoorLock::DoorLockUserType::kNotSupported },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::DoorStateEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::DoorStateEnum> table = {
        { "Open", DoorLock::DoorStateEnum::kDoorOpen },
        { "Closed", DoorLock::DoorStateEnum::kDoorClosed },
        { "ErrorJammed", DoorLock::DoorStateEnum::kDoorJammed },
        { "ErrorForcedOpen", DoorLock::DoorStateEnum::kDoorForcedOpen },
        { "ErrorUnspecified", DoorLock::DoorStateEnum::kDoorUnspecifiedError },
        { "DoorAjar", DoorLock::DoorStateEnum::kDoorAjar },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::LockDataTypeEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::LockDataTypeEnum> table = {
        { "Unspecified", DoorLock::LockDataTypeEnum::kUnspecified },
        { "ProgrammingCode", DoorLock::LockDataTypeEnum::kProgrammingCode },
        { "UserIndex", DoorLock::LockDataTypeEnum::kUserIndex },
        { "WeekDaySchedule", DoorLock::LockDataTypeEnum::kWeekDaySchedule },
        { "YearDaySchedule", DoorLock::LockDataTypeEnum::kYearDaySchedule },
        { "HolidaySchedule", DoorLock::LockDataTypeEnum::kHolidaySchedule },
        { "PIN", DoorLock::LockDataTypeEnum::kPin },
        { "RFID", DoorLock::LockDataTypeEnum::kRfid },
        { "Fingerprint", DoorLock::LockDataTypeEnum::kFingerprint },
        { "FingerVein", DoorLock::LockDataTypeEnum::kFingerVein },
        { "Face", DoorLock::LockDataTypeEnum::kFace },
        { "AliroCredentialIssuerKey", DoorLock::LockDataTypeEnum::kAliroCredentialIssuerKey },
        { "AliroEvictableEndpointKey", DoorLock::LockDataTypeEnum::kAliroEvictableEndpointKey },
        { "AliroNonEvictableEndpointKey", DoorLock::LockDataTypeEnum::kAliroNonEvictableEndpointKey },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::LockOperationTypeEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::LockOperationTypeEnum> table = {
        { "Lock", DoorLock::LockOperationTypeEnum::kLock },
        { "Unlock", DoorLock::LockOperationTypeEnum::kUnlock },
        { "NonAccessUserEvent", DoorLock::LockOperationTypeEnum::kNonAccessUserEvent },
        { "ForcedUserEvent", DoorLock::LockOperationTypeEnum::kForcedUserEvent },
        { "Unlatch", DoorLock::LockOperationTypeEnum::kUnlatch },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::OperatingModeEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::OperatingModeEnum> table = {
        { "Normal", DoorLock::OperatingModeEnum::kNormal },
        { "Vacation", DoorLock::OperatingModeEnum::kVacation },
        { "Privacy", DoorLock::OperatingModeEnum::kPrivacy },
        { "NoRemoteLockUnlock", DoorLock::OperatingModeEnum::kNoRemoteLockUnlock },
        { "Passage", DoorLock::OperatingModeEnum::kPassage },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::OperationErrorEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::OperationErrorEnum> table = {
        { "Unspecified", DoorLock::OperationErrorEnum::kUnspecified },
        { "InvalidCredential", DoorLock::OperationErrorEnum::kInvalidCredential },
        { "DisabledUserDenied", DoorLock::OperationErrorEnum::kDisabledUserDenied },
        { "Restricted", DoorLock::OperationErrorEnum::kRestricted },
        { "InsufficientBattery", DoorLock::OperationErrorEnum::kInsufficientBattery },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::OperationSourceEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::OperationSourceEnum> table = {
        { "Unspecified", DoorLock::OperationSourceEnum::kUnspecified },
        { "Manual", DoorLock::OperationSourceEnum::kManual },
        { "ProprietaryRemote", DoorLock::OperationSourceEnum::kProprietaryRemote },
        { "Keypad", DoorLock::OperationSourceEnum::kKeypad },
        { "Auto", DoorLock::OperationSourceEnum::kAuto },
        { "Button", DoorLock::OperationSourceEnum::kButton },
        { "Schedule", DoorLock::OperationSourceEnum::kSchedule },
        { "Remote", DoorLock::OperationSourceEnum::kRemote },
        { "RFID", DoorLock::OperationSourceEnum::kRfid },
        { "Biometric", DoorLock::OperationSourceEnum::kBiometric },
        { "Aliro", DoorLock::OperationSourceEnum::kAliro },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::UserStatusEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::UserStatusEnum> table = {
        { "Available", DoorLock::UserStatusEnum::kAvailable },
        { "OccupiedEnabled", DoorLock::UserStatusEnum::kOccupiedEnabled },
        { "OccupiedDisabled", DoorLock::UserStatusEnum::kOccupiedDisabled },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<DoorLock::UserTypeEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, DoorLock::UserTypeEnum> table = {
        { "UnrestrictedUser", DoorLock::UserTypeEnum::kUnrestrictedUser },
        { "YearDayScheduleUser", DoorLock::UserTypeEnum::kYearDayScheduleUser },
        { "WeekDayScheduleUser", DoorLock::UserTypeEnum::kWeekDayScheduleUser },
        { "ProgrammingUser", DoorLock::UserTypeEnum::kProgrammingUser },
        { "NonAccessUser", DoorLock::UserTypeEnum::kNonAccessUser },
        { "ForcedUser", DoorLock::UserTypeEnum::kForcedUser },
        { "DisposableUser", DoorLock::UserTypeEnum::kDisposableUser },
        { "ExpiringUser", DoorLock::UserTypeEnum::kExpiringUser },
        { "ScheduleRestrictedUser", DoorLock::UserTypeEnum::kScheduleRestrictedUser },
        { "RemoteOnlyUser", DoorLock::UserTypeEnum::kRemoteOnlyUser },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
/***************************** Bitmap Converters **************/
template <>
inline std::optional<chip::BitMask<BarrierControl::BarrierControlCapabilities>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<BarrierControl::BarrierControlCapabilities> r;
    r.SetField(BarrierControl::BarrierControlCapabilities::kPartialBarrier, obj.value("PartialBarrier", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<BarrierControl::BarrierControlSafetyStatus>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<BarrierControl::BarrierControlSafetyStatus> r;
    r.SetField(BarrierControl::BarrierControlSafetyStatus::kRemoteLockout, obj.value("RemoteLockout", false));
    r.SetField(BarrierControl::BarrierControlSafetyStatus::kTemperDetected, obj.value("TamperDetected", false));
    r.SetField(BarrierControl::BarrierControlSafetyStatus::kFailedCommunication, obj.value("FailedCommunication", false));
    r.SetField(BarrierControl::BarrierControlSafetyStatus::kPositionFailure, obj.value("PositionFailure", false));
    return r;
}

/***************************** Bitmap Converters **************/
template <>
inline std::optional<chip::BitMask<Thermostat::ACErrorCodeBitmap>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<Thermostat::ACErrorCodeBitmap> r;
    r.SetField(Thermostat::ACErrorCodeBitmap::kCompressorFail, obj.value("CompressorFail", false));
    r.SetField(Thermostat::ACErrorCodeBitmap::kRoomSensorFail, obj.value("RoomSensorFail", false));
    r.SetField(Thermostat::ACErrorCodeBitmap::kOutdoorSensorFail, obj.value("OutdoorSensorFail", false));
    r.SetField(Thermostat::ACErrorCodeBitmap::kCoilSensorFail, obj.value("CoilSensorFail", false));
    r.SetField(Thermostat::ACErrorCodeBitmap::kFanFail, obj.value("FanFail", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<Thermostat::Feature>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<Thermostat::Feature> r;
    r.SetField(Thermostat::Feature::kHeating, obj.value("Heating", false));
    r.SetField(Thermostat::Feature::kCooling, obj.value("Cooling", false));
    r.SetField(Thermostat::Feature::kOccupancy, obj.value("Occupancy", false));
    r.SetField(Thermostat::Feature::kScheduleConfiguration, obj.value("ScheduleConfiguration", false));
    r.SetField(Thermostat::Feature::kSetback, obj.value("Setback", false));
    r.SetField(Thermostat::Feature::kAutoMode, obj.value("AutoMode", false));
    r.SetField(Thermostat::Feature::kLocalTemperatureNotExposed, obj.value("LocalTemperatureNotExposed", false));
    r.SetField(Thermostat::Feature::kMatterScheduleConfiguration, obj.value("MatterScheduleConfiguration", false));
    r.SetField(Thermostat::Feature::kPresets, obj.value("Presets", false));
    r.SetField(Thermostat::Feature::kSetpoints, obj.value("Setpoints", false));
    r.SetField(Thermostat::Feature::kQueuedPresetsSupported, obj.value("QueuedPresetsSupported", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<Thermostat::HVACSystemTypeBitmap>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<Thermostat::HVACSystemTypeBitmap> r;
    r.SetField(Thermostat::HVACSystemTypeBitmap::kCoolingStage, obj.value("CoolingStage", false));
    r.SetField(Thermostat::HVACSystemTypeBitmap::kHeatingStage, obj.value("HeatingStage", false));
    r.SetField(Thermostat::HVACSystemTypeBitmap::kHeatingIsHeatPump, obj.value("HeatingIsHeatPump", false));
    r.SetField(Thermostat::HVACSystemTypeBitmap::kHeatingUsesFuel, obj.value("HeatingUsesFuel", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<Thermostat::PresetTypeFeaturesBitmap>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<Thermostat::PresetTypeFeaturesBitmap> r;
    r.SetField(Thermostat::PresetTypeFeaturesBitmap::kAutomatic, obj.value("Automatic", false));
    r.SetField(Thermostat::PresetTypeFeaturesBitmap::kSupportsNames, obj.value("SupportsNames", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<Thermostat::ProgrammingOperationModeBitmap>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<Thermostat::ProgrammingOperationModeBitmap> r;
    r.SetField(Thermostat::ProgrammingOperationModeBitmap::kScheduleActive, obj.value("ScheduleActive", false));
    r.SetField(Thermostat::ProgrammingOperationModeBitmap::kAutoRecovery, obj.value("AutoRecovery", false));
    r.SetField(Thermostat::ProgrammingOperationModeBitmap::kEconomy, obj.value("Economy", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<Thermostat::RelayStateBitmap>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<Thermostat::RelayStateBitmap> r;
    r.SetField(Thermostat::RelayStateBitmap::kHeat, obj.value("Heat", false));
    r.SetField(Thermostat::RelayStateBitmap::kCool, obj.value("Cool", false));
    r.SetField(Thermostat::RelayStateBitmap::kFan, obj.value("Fan", false));
    r.SetField(Thermostat::RelayStateBitmap::kHeatStage2, obj.value("HeatStage2", false));
    r.SetField(Thermostat::RelayStateBitmap::kCoolStage2, obj.value("CoolStage2", false));
    r.SetField(Thermostat::RelayStateBitmap::kFanStage2, obj.value("FanStage2", false));
    r.SetField(Thermostat::RelayStateBitmap::kFanStage3, obj.value("FanStage3", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<Thermostat::RemoteSensingBitmap>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<Thermostat::RemoteSensingBitmap> r;
    r.SetField(Thermostat::RemoteSensingBitmap::kLocalTemperature, obj.value("LocalTemperature", false));
    r.SetField(Thermostat::RemoteSensingBitmap::kOutdoorTemperature, obj.value("OutdoorTemperature", false));
    r.SetField(Thermostat::RemoteSensingBitmap::kOccupancy, obj.value("Occupancy", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<Thermostat::ScheduleDayOfWeekBitmap>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<Thermostat::ScheduleDayOfWeekBitmap> r;
    r.SetField(Thermostat::ScheduleDayOfWeekBitmap::kSunday, obj.value("Sunday", false));
    r.SetField(Thermostat::ScheduleDayOfWeekBitmap::kMonday, obj.value("Monday", false));
    r.SetField(Thermostat::ScheduleDayOfWeekBitmap::kTuesday, obj.value("Tuesday", false));
    r.SetField(Thermostat::ScheduleDayOfWeekBitmap::kWednesday, obj.value("Wednesday", false));
    r.SetField(Thermostat::ScheduleDayOfWeekBitmap::kThursday, obj.value("Thursday", false));
    r.SetField(Thermostat::ScheduleDayOfWeekBitmap::kFriday, obj.value("Friday", false));
    r.SetField(Thermostat::ScheduleDayOfWeekBitmap::kSaturday, obj.value("Saturday", false));
    r.SetField(Thermostat::ScheduleDayOfWeekBitmap::kAway, obj.value("Away", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<Thermostat::ScheduleModeBitmap>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<Thermostat::ScheduleModeBitmap> r;
    r.SetField(Thermostat::ScheduleModeBitmap::kHeatSetpointPresent, obj.value("HeatSetpointPresent", false));
    r.SetField(Thermostat::ScheduleModeBitmap::kCoolSetpointPresent, obj.value("CoolSetpointPresent", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<Thermostat::ScheduleTypeFeaturesBitmap>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<Thermostat::ScheduleTypeFeaturesBitmap> r;
    r.SetField(Thermostat::ScheduleTypeFeaturesBitmap::kSupportsPresets, obj.value("SupportsPresets", false));
    r.SetField(Thermostat::ScheduleTypeFeaturesBitmap::kSupportsSetpoints, obj.value("SupportsSetpoints", false));
    r.SetField(Thermostat::ScheduleTypeFeaturesBitmap::kSupportsNames, obj.value("SupportsNames", false));
    r.SetField(Thermostat::ScheduleTypeFeaturesBitmap::kSupportsOff, obj.value("SupportsOff", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<Thermostat::TemperatureSetpointHoldPolicyBitmap>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<Thermostat::TemperatureSetpointHoldPolicyBitmap> r;
    r.SetField(Thermostat::TemperatureSetpointHoldPolicyBitmap::kHoldDurationElapsed, obj.value("HoldDurationElapsed", false));
    r.SetField(Thermostat::TemperatureSetpointHoldPolicyBitmap::kHoldDurationElapsedOrPresetChanged,
        obj.value("HoldDurationElapsedOrPresetChanged", false));
    return r;
}

template <>
inline std::optional<Thermostat::ACCapacityFormatEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, Thermostat::ACCapacityFormatEnum> table = {
        { "BTUh", Thermostat::ACCapacityFormatEnum::kBTUh },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<Thermostat::ACCompressorTypeEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, Thermostat::ACCompressorTypeEnum> table = {
        { "Unknown", Thermostat::ACCompressorTypeEnum::kUnknown },
        { "T1", Thermostat::ACCompressorTypeEnum::kT1 },
        { "T2", Thermostat::ACCompressorTypeEnum::kT2 },
        { "T3", Thermostat::ACCompressorTypeEnum::kT3 },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<Thermostat::ACLouverPositionEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, Thermostat::ACLouverPositionEnum> table = {
        { "Closed", Thermostat::ACLouverPositionEnum::kClosed },
        { "Open", Thermostat::ACLouverPositionEnum::kOpen },
        { "Quarter", Thermostat::ACLouverPositionEnum::kQuarter },
        { "Half", Thermostat::ACLouverPositionEnum::kHalf },
        { "ThreeQuarters", Thermostat::ACLouverPositionEnum::kThreeQuarters },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<Thermostat::ACRefrigerantTypeEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, Thermostat::ACRefrigerantTypeEnum> table = {
        { "Unknown", Thermostat::ACRefrigerantTypeEnum::kUnknown },
        { "R22", Thermostat::ACRefrigerantTypeEnum::kR22 },
        { "R410a", Thermostat::ACRefrigerantTypeEnum::kR410a },
        { "R407c", Thermostat::ACRefrigerantTypeEnum::kR407c },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<Thermostat::ACTypeEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, Thermostat::ACTypeEnum> table = {
        { "Unknown", Thermostat::ACTypeEnum::kUnknown },
        { "CoolingFixed", Thermostat::ACTypeEnum::kCoolingFixed },
        { "HeatPumpFixed", Thermostat::ACTypeEnum::kHeatPumpFixed },
        { "CoolingInverter", Thermostat::ACTypeEnum::kCoolingInverter },
        { "HeatPumpInverter", Thermostat::ACTypeEnum::kHeatPumpInverter },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<Thermostat::ControlSequenceOfOperationEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, Thermostat::ControlSequenceOfOperationEnum> table = {
        { "CoolingOnly", Thermostat::ControlSequenceOfOperationEnum::kCoolingOnly },
        { "CoolingWithReheat", Thermostat::ControlSequenceOfOperationEnum::kCoolingWithReheat },
        { "HeatingOnly", Thermostat::ControlSequenceOfOperationEnum::kHeatingOnly },
        { "HeatingWithReheat", Thermostat::ControlSequenceOfOperationEnum::kHeatingWithReheat },
        { "CoolingAndHeating4Pipes", Thermostat::ControlSequenceOfOperationEnum::kCoolingAndHeating },
        { "CoolingAndHeating4PipesWithReheat", Thermostat::ControlSequenceOfOperationEnum::kCoolingAndHeatingWithReheat },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<Thermostat::PresetScenarioEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, Thermostat::PresetScenarioEnum> table = {
        { "Unspecified", Thermostat::PresetScenarioEnum::kUnspecified },
        { "Occupied", Thermostat::PresetScenarioEnum::kOccupied },
        { "Unoccupied", Thermostat::PresetScenarioEnum::kUnoccupied },
        { "Sleep", Thermostat::PresetScenarioEnum::kSleep },
        { "Wake", Thermostat::PresetScenarioEnum::kWake },
        { "Vacation", Thermostat::PresetScenarioEnum::kVacation },
        { "UserDefined", Thermostat::PresetScenarioEnum::kUserDefined },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<Thermostat::SetpointChangeSourceEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, Thermostat::SetpointChangeSourceEnum> table = {
        { "Manual", Thermostat::SetpointChangeSourceEnum::kManual },
        { "Schedule", Thermostat::SetpointChangeSourceEnum::kSchedule },
        { "External", Thermostat::SetpointChangeSourceEnum::kExternal },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<Thermostat::SetpointRaiseLowerModeEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, Thermostat::SetpointRaiseLowerModeEnum> table = {
        { "Heat", Thermostat::SetpointRaiseLowerModeEnum::kHeat },
        { "Cool", Thermostat::SetpointRaiseLowerModeEnum::kCool },
        { "Both", Thermostat::SetpointRaiseLowerModeEnum::kBoth },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<Thermostat::StartOfWeekEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, Thermostat::StartOfWeekEnum> table = {
        { "Sunday", Thermostat::StartOfWeekEnum::kSunday },
        { "Monday", Thermostat::StartOfWeekEnum::kMonday },
        { "Tuesday", Thermostat::StartOfWeekEnum::kTuesday },
        { "Wednesday", Thermostat::StartOfWeekEnum::kWednesday },
        { "Thursday", Thermostat::StartOfWeekEnum::kThursday },
        { "Friday", Thermostat::StartOfWeekEnum::kFriday },
        { "Saturday", Thermostat::StartOfWeekEnum::kSaturday },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<Thermostat::SystemModeEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, Thermostat::SystemModeEnum> table = {
        { "Off", Thermostat::SystemModeEnum::kOff },
        { "Auto", Thermostat::SystemModeEnum::kAuto },
        { "Cool", Thermostat::SystemModeEnum::kCool },
        { "Heat", Thermostat::SystemModeEnum::kHeat },
        { "EmergencyHeating", Thermostat::SystemModeEnum::kEmergencyHeat },
        { "Precooling", Thermostat::SystemModeEnum::kPrecooling },
        { "FanOnly", Thermostat::SystemModeEnum::kFanOnly },
        { "Dry", Thermostat::SystemModeEnum::kDry },
        { "Sleep", Thermostat::SystemModeEnum::kSleep },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<Thermostat::TemperatureSetpointHoldEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, Thermostat::TemperatureSetpointHoldEnum> table = {
        { "SetpointHoldOff", Thermostat::TemperatureSetpointHoldEnum::kSetpointHoldOff },
        { "SetpointHoldOn", Thermostat::TemperatureSetpointHoldEnum::kSetpointHoldOn },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<Thermostat::ThermostatRunningModeEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, Thermostat::ThermostatRunningModeEnum> table = {
        { "Off", Thermostat::ThermostatRunningModeEnum::kOff },
        { "Cool", Thermostat::ThermostatRunningModeEnum::kCool },
        { "Heat", Thermostat::ThermostatRunningModeEnum::kHeat },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
/***************************** Bitmap Converters **************/
template <>
inline std::optional<chip::BitMask<FanControl::Feature>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<FanControl::Feature> r;
    r.SetField(FanControl::Feature::kMultiSpeed, obj.value("MultiSpeed", false));
    r.SetField(FanControl::Feature::kAuto, obj.value("Auto", false));
    r.SetField(FanControl::Feature::kRocking, obj.value("Rocking", false));
    r.SetField(FanControl::Feature::kWind, obj.value("Wind", false));
    r.SetField(FanControl::Feature::kStep, obj.value("Step", false));
    r.SetField(FanControl::Feature::kAirflowDirection, obj.value("Airflow Direction", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<FanControl::RockBitmap>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<FanControl::RockBitmap> r;
    r.SetField(FanControl::RockBitmap::kRockLeftRight, obj.value("RockLeftRight", false));
    r.SetField(FanControl::RockBitmap::kRockUpDown, obj.value("RockUpDown", false));
    r.SetField(FanControl::RockBitmap::kRockRound, obj.value("RockRound", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<FanControl::WindBitmap>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<FanControl::WindBitmap> r;
    r.SetField(FanControl::WindBitmap::kSleepWind, obj.value("Sleep Wind", false));
    r.SetField(FanControl::WindBitmap::kNaturalWind, obj.value("Natural Wind", false));
    return r;
}

template <>
inline std::optional<FanControl::AirflowDirectionEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, FanControl::AirflowDirectionEnum> table = {
        { "Forward", FanControl::AirflowDirectionEnum::kForward },
        { "Reverse", FanControl::AirflowDirectionEnum::kReverse },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<FanControl::FanModeEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, FanControl::FanModeEnum> table = {
        { "Off", FanControl::FanModeEnum::kOff },
        { "Low", FanControl::FanModeEnum::kLow },
        { "Medium", FanControl::FanModeEnum::kMedium },
        { "High", FanControl::FanModeEnum::kHigh },
        { "On", FanControl::FanModeEnum::kOn },
        { "Auto", FanControl::FanModeEnum::kAuto },
        { "Smart", FanControl::FanModeEnum::kSmart },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<FanControl::FanModeSequenceEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, FanControl::FanModeSequenceEnum> table = {
        { "Off/Low/Med/High", FanControl::FanModeSequenceEnum::kOffLowMedHigh },
        { "Off/Low/High", FanControl::FanModeSequenceEnum::kOffLowHigh },
        { "Off/Low/Med/High/Auto", FanControl::FanModeSequenceEnum::kOffLowMedHighAuto },
        { "Off/Low/High/Auto", FanControl::FanModeSequenceEnum::kOffLowHighAuto },
        { "Off/High/Auto", FanControl::FanModeSequenceEnum::kOffHighAuto },
        { "Off/High", FanControl::FanModeSequenceEnum::kOffHigh },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<FanControl::StepDirectionEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, FanControl::StepDirectionEnum> table = {
        { "Increase", FanControl::StepDirectionEnum::kIncrease },
        { "Decrease", FanControl::StepDirectionEnum::kDecrease },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
/***************************** Bitmap Converters **************/

template <>
inline std::optional<ThermostatUserInterfaceConfiguration::KeypadLockoutEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, ThermostatUserInterfaceConfiguration::KeypadLockoutEnum> table = {
        { "NoLockout", ThermostatUserInterfaceConfiguration::KeypadLockoutEnum::kNoLockout },
        { "Lockout1", ThermostatUserInterfaceConfiguration::KeypadLockoutEnum::kLockout1 },
        { "Lockout2", ThermostatUserInterfaceConfiguration::KeypadLockoutEnum::kLockout2 },
        { "Lockout3", ThermostatUserInterfaceConfiguration::KeypadLockoutEnum::kLockout3 },
        { "Lockout4", ThermostatUserInterfaceConfiguration::KeypadLockoutEnum::kLockout4 },
        { "Lockout5", ThermostatUserInterfaceConfiguration::KeypadLockoutEnum::kLockout5 },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<ThermostatUserInterfaceConfiguration::ScheduleProgrammingVisibilityEnum>
from_json(const nlohmann::json& value)
{
    const std::map<std::string, ThermostatUserInterfaceConfiguration::ScheduleProgrammingVisibilityEnum> table = {
        { "ScheduleProgrammingPermitted",
            ThermostatUserInterfaceConfiguration::ScheduleProgrammingVisibilityEnum::kScheduleProgrammingPermitted },
        { "ScheduleProgrammingDenied",
            ThermostatUserInterfaceConfiguration::ScheduleProgrammingVisibilityEnum::kScheduleProgrammingDenied },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<ThermostatUserInterfaceConfiguration::TemperatureDisplayModeEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, ThermostatUserInterfaceConfiguration::TemperatureDisplayModeEnum> table = {
        { "Celsius", ThermostatUserInterfaceConfiguration::TemperatureDisplayModeEnum::kCelsius },
        { "Fahrenheit", ThermostatUserInterfaceConfiguration::TemperatureDisplayModeEnum::kFahrenheit },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
/***************************** Bitmap Converters **************/
template <>
inline std::optional<chip::BitMask<ColorControl::ColorCapabilities>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<ColorControl::ColorCapabilities> r;
    r.SetField(ColorControl::ColorCapabilities::kHueSaturationSupported, obj.value("HueSaturationSupported", false));
    r.SetField(ColorControl::ColorCapabilities::kEnhancedHueSupported, obj.value("EnhancedHueSupported", false));
    r.SetField(ColorControl::ColorCapabilities::kColorLoopSupported, obj.value("ColorLoopSupported", false));
    r.SetField(ColorControl::ColorCapabilities::kXYAttributesSupported, obj.value("XYSupported", false));
    r.SetField(ColorControl::ColorCapabilities::kColorTemperatureSupported, obj.value("ColorTemperatureSupported", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<ColorControl::ColorLoopUpdateFlags>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<ColorControl::ColorLoopUpdateFlags> r;
    r.SetField(ColorControl::ColorLoopUpdateFlags::kUpdateAction, obj.value("UpdateAction", false));
    r.SetField(ColorControl::ColorLoopUpdateFlags::kUpdateDirection, obj.value("UpdateDirection", false));
    r.SetField(ColorControl::ColorLoopUpdateFlags::kUpdateTime, obj.value("UpdateTime", false));
    r.SetField(ColorControl::ColorLoopUpdateFlags::kUpdateStartHue, obj.value("UpdateStartHue", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<ColorControl::Feature>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<ColorControl::Feature> r;
    r.SetField(ColorControl::Feature::kHueAndSaturation, obj.value("HueAndSaturation", false));
    r.SetField(ColorControl::Feature::kEnhancedHue, obj.value("EnhancedHue", false));
    r.SetField(ColorControl::Feature::kColorLoop, obj.value("ColorLoop", false));
    r.SetField(ColorControl::Feature::kXy, obj.value("XY", false));
    r.SetField(ColorControl::Feature::kColorTemperature, obj.value("ColorTemperature", false));
    return r;
}

template <>
inline std::optional<ColorControl::ColorLoopAction> from_json(const nlohmann::json& value)
{
    const std::map<std::string, ColorControl::ColorLoopAction> table = {
        { "DeactivateColorLoop", ColorControl::ColorLoopAction::kDeactivate },
        { "ActivateColorLoopFromColorLoopStartEnhancedHue", ColorControl::ColorLoopAction::kActivateFromColorLoopStartEnhancedHue },
        { "ActivateColorLoopFromEnhancedCurrentHue", ColorControl::ColorLoopAction::kActivateFromEnhancedCurrentHue },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<ColorControl::ColorLoopDirection> from_json(const nlohmann::json& value)
{
    const std::map<std::string, ColorControl::ColorLoopDirection> table = {
        { "DecrementEnhancedCurrentHue", ColorControl::ColorLoopDirection::kDecrementHue },
        { "IncrementEnhancedCurrentHue", ColorControl::ColorLoopDirection::kIncrementHue },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<ColorControl::ColorMode> from_json(const nlohmann::json& value)
{
    const std::map<std::string, ColorControl::ColorMode> table = {
        { "CurrentHueAndCurrentSaturation", ColorControl::ColorMode::kCurrentHueAndCurrentSaturation },
        { "CurrentXAndCurrentY", ColorControl::ColorMode::kCurrentXAndCurrentY },
        { "ColorTemperatureMireds", ColorControl::ColorMode::kColorTemperature },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<ColorControl::HueDirection> from_json(const nlohmann::json& value)
{
    const std::map<std::string, ColorControl::HueDirection> table = {
        { "ShortestDistance", ColorControl::HueDirection::kShortestDistance },
        { "LongestDistance", ColorControl::HueDirection::kLongestDistance },
        { "Up", ColorControl::HueDirection::kUp },
        { "Down", ColorControl::HueDirection::kDown },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<ColorControl::HueMoveMode> from_json(const nlohmann::json& value)
{
    const std::map<std::string, ColorControl::HueMoveMode> table = {
        { "Stop", ColorControl::HueMoveMode::kStop },
        { "Up", ColorControl::HueMoveMode::kUp },
        { "Down", ColorControl::HueMoveMode::kDown },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<ColorControl::HueStepMode> from_json(const nlohmann::json& value)
{
    const std::map<std::string, ColorControl::HueStepMode> table = {
        { "Up", ColorControl::HueStepMode::kUp },
        { "Down", ColorControl::HueStepMode::kDown },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<ColorControl::SaturationMoveMode> from_json(const nlohmann::json& value)
{
    const std::map<std::string, ColorControl::SaturationMoveMode> table = {
        { "Stop", ColorControl::SaturationMoveMode::kStop },
        { "Up", ColorControl::SaturationMoveMode::kUp },
        { "Down", ColorControl::SaturationMoveMode::kDown },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
template <>
inline std::optional<ColorControl::SaturationStepMode> from_json(const nlohmann::json& value)
{
    const std::map<std::string, ColorControl::SaturationStepMode> table = {
        { "Up", ColorControl::SaturationStepMode::kUp },
        { "Down", ColorControl::SaturationStepMode::kDown },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
/***************************** Bitmap Converters **************/

template <>
inline std::optional<IlluminanceMeasurement::LightSensorTypeEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, IlluminanceMeasurement::LightSensorTypeEnum> table = {
        { "Photodiode", IlluminanceMeasurement::LightSensorTypeEnum::kPhotodiode },
        { "CMOS", IlluminanceMeasurement::LightSensorTypeEnum::kCmos },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
/***************************** Bitmap Converters **************/

/***************************** Bitmap Converters **************/
template <>
inline std::optional<chip::BitMask<PressureMeasurement::Feature>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<PressureMeasurement::Feature> r;
    r.SetField(PressureMeasurement::Feature::kExtended, obj.value("Extended", false));
    return r;
}

/***************************** Bitmap Converters **************/

/***************************** Bitmap Converters **************/

/***************************** Bitmap Converters **************/
template <>
inline std::optional<chip::BitMask<OccupancySensing::OccupancyBitmap>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<OccupancySensing::OccupancyBitmap> r;
    r.SetField(OccupancySensing::OccupancyBitmap::kOccupied, obj.value("SensedOccupancy", false));
    return r;
}
template <>
inline std::optional<chip::BitMask<OccupancySensing::OccupancySensorTypeBitmap>> from_json(const nlohmann::json& obj)
{
    chip::BitMask<OccupancySensing::OccupancySensorTypeBitmap> r;
    r.SetField(OccupancySensing::OccupancySensorTypeBitmap::kPir, obj.value("PIR", false));
    r.SetField(OccupancySensing::OccupancySensorTypeBitmap::kUltrasonic, obj.value("Ultrasonic", false));
    r.SetField(OccupancySensing::OccupancySensorTypeBitmap::kPhysicalContact, obj.value("PhysicalContact", false));
    return r;
}

template <>
inline std::optional<OccupancySensing::OccupancySensorTypeEnum> from_json(const nlohmann::json& value)
{
    const std::map<std::string, OccupancySensing::OccupancySensorTypeEnum> table = {
        { "PIR", OccupancySensing::OccupancySensorTypeEnum::kPir },
        { "Ultrasonic", OccupancySensing::OccupancySensorTypeEnum::kUltrasonic },
        { "PIRAndUltrasonic", OccupancySensing::OccupancySensorTypeEnum::kPIRAndUltrasonic },
        { "PhysicalContact", OccupancySensing::OccupancySensorTypeEnum::kPhysicalContact },
    };

    auto i = table.find(value);
    if (i != table.end()) {
        return i->second;
    } else {
        return std::nullopt;
    }
}
/***************************** Bitmap Converters **************/
