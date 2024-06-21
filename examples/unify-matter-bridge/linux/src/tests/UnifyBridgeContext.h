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

/**
 * @file
 *  A collection of mocks and pseudo application context that help testing each
 *  bridge functionality locally.
 *
 */

#ifndef UNIFY_BRIDGE_CONTEXT_H_
#define UNIFY_BRIDGE_CONTEXT_H_

#include "matter_device_translator.hpp"
#include <app/tests/AppTestContext.h>

// Mocks
#include "MockClusterEmulator.hpp"
#include "MockGroupTranslator.hpp"
#include "MockNodeStateMonitor.hpp"
#include "MockUnifyMqtt.hpp"

// Third party library
#include <nlunit-test.h>
#include "sl_log.h"

namespace unify::matter_bridge {
namespace Test {

/**
 * @brief
 *  A base class for mocking CHIPTool client. In a test context it is already paird
 *  to Unify Bridge and you can instantly start interaction with command/attribute
 *  handlers.
 *
 */
class UnifyBridgeContext : public chip::Test::AppContext
{
    typedef chip::Test::AppContext Super;

public:
    UnifyBridgeContext() :
        mMqttHandler(), mNodeStateMonitor(mDeviceTranslator, mEmulator, mEmberInterface), mGroupTranslator(m_matter_data_storage)
    {}

    /// Initialize the underlying layers.
    CHIP_ERROR UMB_Initialize();

    // Shutdown all layers, finalize operations
    void UMB_Finalize();

    UnifyEmberInterface mEmberInterface;
    device_translator mDeviceTranslator = device_translator(false);
    matter_data_storage m_matter_data_storage;
    Test::MockUnifyMqtt mMqttHandler;
    Test::MockNodeStateMonitor mNodeStateMonitor;
    Test::MockGroupTranslator mGroupTranslator;
    Test::MockClusterEmulator mEmulator;
};

} // namespace Test
} // namespace unify::matter_bridge

#endif // UNIFY_BRIDGE_CONTEXT_H_
