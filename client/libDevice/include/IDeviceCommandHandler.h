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
* \file IDeviceCommandHandler.h
*
* \brief IDeviceMessageDispatcher handle device commands.
*******************************************************************************
*/

#ifndef IDEVICE_COMMAND_HANDLER_H
#define IDEVICE_COMMAND_HANDLER_H

#include <string>
#include "jsoncpp/json.h"

namespace ic_device
{

//! constant key for string EventID
static const std::string KEY_EVENTID = "EventID";
//! constant key for string Data
static const std::string KEY_DATA = "Data";
//! constant key for string status
static const std::string KEY_STATUS = "status";
//! constant key for string ActivationStatusQuery
static const std::string EVENT_ACTIVATION_STATUS_QUERY = "ActivationStatusQuery";
//! constant key for string DBSizeQuery
static const std::string EVENT_DB_SIZE_QUERY = "DBSizeQuery";
//! constant key for string MQTTConnectionStatusQuery
static const std::string EVENT_MQTT_STATUS_QUERY = "MQTTConnectionStatusQuery";
//! constant key for string RemoteOperationResponse
static const std::string EVENT_RO_RESPONSE = "RemoteOperationResponse";
//! constant key for string DeviceShutdownNotif
static const std::string EVENT_SHUTDOWN_NOTIF = "DeviceShutdownNotif";
//! constant int for PREPARE_FOR_SHUTDOWN
static const int PREPARE_FOR_SHUTDOWN = 1;
//! constant int for SHUTDOWN_INITIATED
static const int SHUTDOWN_INITIATED = 2;

/**
 * IDeviceCommandHandler is an interface for handling device commands
 */
class IDeviceCommandHandler
{
public:
    /**
     * This function handles shutdown notification
     * @param[in] strPayload Notification payload string
     * @return void
     */
    virtual void HandleShutdownNotif(const ic_utils::Json::Value &rjsonPayload) = 0;

    /**
     * This function handles activation status query
     * @param void
     * @return void
     */
    virtual void HandleActivationStatusQuery() = 0;

    /**
     * This function handles database size query
     * @param void
     * @return void
     */
    virtual void HandleDBSizeQuery() = 0;

    /**
     * This function handles MQTT connection status query
     * @param void
     * @return void
     */
    virtual void HandleMQTTConnStatusQuery() = 0;

    /**
     * This function handles remote operations(RO) response
     * @param @param[in] rstrCmdPayLoad Response payload string
     * @return void
     */
    virtual void HandleROResponse(const std::string &rstrCmdPayLoad) = 0;

    /**
     * This function parses and handles command payload
     * @param rstrCmdPayLoad Command payload
     * @return true if processing of device command payload is success
     *         else return false
     */
    virtual bool ProcessDeviceCommand(const std::string &rstrCmdPayLoad) = 0;
};
} //namespace ic_device
#endif // IDEVICE_COMMAND_HANDLER_H
