/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
 *    All rights reserved.
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

#include "AppTask.h"
#include "AppConfig.h"
#include "AppEvent.h"
#include "LEDWidget.h"

#include <app/clusters/on-off-server/on-off-server.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>

#include <assert.h>

#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include <lib/support/CodeUtils.h>

#include <platform/CHIPDeviceLayer.h>

#include <platform/silabs/platformAbstraction/SilabsPlatform.h>

#ifdef SL_CATALOG_SIMPLE_LED_LED1_PRESENT
#define ONOFF_LED 1
#else
#define ONOFF_LED 0
#endif

#define APP_FUNCTION_BUTTON 0
#define APP_ONOFF_BUTTON 1

using namespace chip;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Silabs;

namespace {

LEDWidget sOnOffLED;

} // namespace

using namespace chip::TLV;
using namespace ::chip::DeviceLayer;

AppTask AppTask::sAppTask;

CHIP_ERROR AppTask::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::DeviceLayer::Silabs::GetPlatform().SetButtonsCb(AppTask::ButtonEventHandler);

#ifdef DISPLAY_ENABLED
    GetLCD().Init((uint8_t *) "onoffPlug-App");
#endif

    err = BaseApplication::Init();
    if (err != CHIP_NO_ERROR)
    {
        SILABS_LOG("BaseApplication::Init() failed");
        appError(err);
    }

    err = PlugMgr().Init();
    if (err != CHIP_NO_ERROR)
    {
        SILABS_LOG("PlugMgr().Init() failed");
        appError(err);
    }

    PlugMgr().SetCallbacks(ActionInitiated, ActionCompleted);

    sOnOffLED.Init(ONOFF_LED);
    sOnOffLED.Set(PlugMgr().IsPlugOn());

// Update the LCD with the Stored value. Show QR Code if not provisioned
#ifdef DISPLAY_ENABLED
    GetLCD().WriteDemoUI(PlugMgr().IsPlugOn());
#ifdef QR_CODE_ENABLED
#ifdef SL_WIFI
    if (!ConnectivityMgr().IsWiFiStationProvisioned())
#else
    if (!ConnectivityMgr().IsThreadProvisioned())
#endif /* !SL_WIFI */
    {
        GetLCD().ShowQRCode(true);
    }
#endif // QR_CODE_ENABLED
#endif

    return err;
}

CHIP_ERROR AppTask::StartAppTask()
{
    return BaseApplication::StartAppTask(AppTaskMain);
}

void AppTask::AppTaskMain(void * pvParameter)
{
    AppEvent event;
    QueueHandle_t sAppEventQueue = *(static_cast<QueueHandle_t *>(pvParameter));

    CHIP_ERROR err = sAppTask.Init();
    if (err != CHIP_NO_ERROR)
    {
        SILABS_LOG("AppTask.Init() failed");
        appError(err);
    }

#if !(defined(CHIP_CONFIG_ENABLE_ICD_SERVER) && CHIP_CONFIG_ENABLE_ICD_SERVER)
    sAppTask.StartStatusLEDTimer();
#endif

    SILABS_LOG("App Task started");

    while (true)
    {
        BaseType_t eventReceived = xQueueReceive(sAppEventQueue, &event, portMAX_DELAY);
        while (eventReceived == pdTRUE)
        {
            sAppTask.DispatchEvent(&event);
            eventReceived = xQueueReceive(sAppEventQueue, &event, 0);
        }
    }
}

void AppTask::OnOffActionEventHandler(AppEvent * aEvent)
{
    bool initiated = false;
    OnOffPlugManager::Action_t action;
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (aEvent->Type == AppEvent::kEventType_Button)
    {
        action = (PlugMgr().IsPlugOn()) ? OnOffPlugManager::OFF_ACTION : OnOffPlugManager::ON_ACTION;
    }
    else
    {
        err = APP_ERROR_UNHANDLED_EVENT;
    }

    if (err == CHIP_NO_ERROR)
    {
        initiated = PlugMgr().InitiateAction(aEvent->Type, action);

        if (!initiated)
        {
            SILABS_LOG("Action is already in progress or active.");
        }
    }
}

void AppTask::ButtonEventHandler(uint8_t button, uint8_t btnAction)
{
    AppEvent button_event           = {};
    button_event.Type               = AppEvent::kEventType_Button;
    button_event.ButtonEvent.Action = btnAction;

    if (button == APP_ONOFF_BUTTON && btnAction == static_cast<uint8_t>(SilabsPlatform::ButtonAction::ButtonPressed))
    {
        button_event.Handler = OnOffActionEventHandler;
        sAppTask.PostEvent(&button_event);
    }
    else if (button == APP_FUNCTION_BUTTON)
    {
        button_event.Handler = BaseApplication::ButtonHandler;
        sAppTask.PostEvent(&button_event);
    }
}

void AppTask::ActionInitiated(OnOffPlugManager::Action_t aAction, int32_t aActor)
{
    // Action initiated, update the light led
    bool lightOn = aAction == OnOffPlugManager::ON_ACTION;
    SILABS_LOG("Turning light %s", (lightOn) ? "On" : "Off")

    sOnOffLED.Set(lightOn);

#ifdef DISPLAY_ENABLED
    sAppTask.GetLCD().WriteDemoUI(lightOn);
#endif

    if (aActor == AppEvent::kEventType_Button)
    {
        sAppTask.mSyncClusterToButtonAction = true;
    }
}

void AppTask::ActionCompleted(OnOffPlugManager::Action_t aAction)
{
    // action has been completed on the outlet
    if (aAction == OnOffPlugManager::ON_ACTION)
    {
        SILABS_LOG("Outlet ON")
    }
    else if (aAction == OnOffPlugManager::OFF_ACTION)
    {
        SILABS_LOG("Outlet OFF")
    }

    if (sAppTask.mSyncClusterToButtonAction)
    {
        chip::DeviceLayer::PlatformMgr().ScheduleWork(UpdateClusterState, reinterpret_cast<intptr_t>(nullptr));
        sAppTask.mSyncClusterToButtonAction = false;
    }
}

void AppTask::UpdateClusterState(intptr_t context)
{
    uint8_t newValue = PlugMgr().IsPlugOn();

    // write the new on/off value
    Protocols::InteractionModel::Status status = OnOffServer::Instance().setOnOffValue(1, newValue, false);

    if (status != Protocols::InteractionModel::Status::Success)
    {
        SILABS_LOG("ERR: updating on/off %x", to_underlying(status));
    }
}
