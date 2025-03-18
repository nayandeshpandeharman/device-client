/*******************************************************************************
 * Copyright (c) 2023-24 Harman International
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 ******************************************************************************/

/*!
*******************************************************************************
* \file CDeviceCommandHandlerImpl.h
*
* \brief CDeviceCommandHandlerImpl handle commands recieved from Device
*******************************************************************************
*/

#ifndef DEVICE_COMMANDHANDLER_H
#define DEVICE_COMMANDHANDLER_H

#include "IDeviceCommandHandler.h"

namespace ic_device
{
/**
 * class CDeviceCommandHandlerImpl handle commands recieved from device
 */
class CDeviceCommandHandlerImpl : public IDeviceCommandHandler
{
public:
    /**
     * Default constructor
     */
    CDeviceCommandHandlerImpl();

    /**
     * Virtual destructor
     */
    virtual ~CDeviceCommandHandlerImpl();

    /**
     * This function starts thread to listen for commands
     * @param void
     * @return void
     */
    virtual void StartListening(void);

    /**
     * This function cleans-up and stops thread listening over ZMQ channel
     * @param void
     * @return void
     */
    virtual void StopListening(void);

    /**
     * Overriding Method of ic_device::IDeviceCommandHandler class
     * @see ic_device::IDeviceCommandHandler::
     *                  HandleShutdownNotif(const ic_utils::Json::Value &jsonPayload)
     */
    virtual void HandleShutdownNotif(const ic_utils::Json::Value &jsonPayload) override;

    /**
     * Overriding Method of ic_device::IDeviceCommandHandler class
     * @see ic_device::IDeviceCommandHandler::HandleActivationStatusQuery()
     */
    virtual void HandleActivationStatusQuery() override;

    /**
     * Overriding Method of ic_device::IDeviceCommandHandler class
     * @see ic_device::IDeviceCommandHandler::HandleDBSizeQuery()
     */
    virtual void HandleDBSizeQuery() override;

    /**
     * Overriding Method of ic_device::IDeviceCommandHandler class
     * @see ic_device::IDeviceCommandHandler::HandleMQTTConnStatusQuery()
     */
    virtual void HandleMQTTConnStatusQuery() override;

    /**
     * Overriding Method of ic_device::IDeviceCommandHandler class
     * @see ic_device::IDeviceCommandHandler::
     *                          HandleROResponse(const std::string &rstrCmdPayLoad)
     */
    void HandleROResponse(const std::string &rstrCmdPayLoad) override;

    /**
     * Overriding Method of ic_device::IDeviceCommandHandler class
     * @see ic_device::IDeviceCommandHandler::
     *                          ProcessDeviceCommand(const std::string &rstrCmdPayLoad)
     */
    virtual bool ProcessDeviceCommand(const std::string &rstrCmdPayLoad) override;

#if IC_UNIT_TEST == 1
    /**
     * Method to simulate DeviceShutdownNotif
     * @param void
     * @return true if DeviceShutdownNotif payload is sent to Ignite Client
     *         else it returns false
     */
    bool SimulateShutdownNotif();
#endif

private:
    /**
     * This function is to handle the prepare-for-shutdown scenario.
     * @param[in] rjsonData Json payload of handle-shutdown scenario.
     * @return void
     */
    void HandlePrepareForShutdown(const ic_utils::Json::Value &rjsonData);
};
}// namespace
#endif // DEVICE_COMMANDHANDLER_H
