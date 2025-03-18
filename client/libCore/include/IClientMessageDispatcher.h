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
********************************************************************************
* \file IClientMessageDispatcher.h
*
* \brief This class provides the necessary interfaces to generates and delivers
* various messages to device platform according to the defined channel/interface
********************************************************************************
*/

#ifndef ICLIENT_MESSAGE_DISPATCHER_H
#define ICLIENT_MESSAGE_DISPATCHER_H

#include "jsoncpp/json.h"

namespace ic_core
{
/**
 * This class provides interfaces to generates and delivers various messages to 
 * device platform
 */
class IClientMessageDispatcher
{
public:
    /**
     * Virtual destructor
     */
    virtual ~IClientMessageDispatcher() {};

    /**
     * Method to generates and delivers Ignite start event to device platform
     * @param void
     * @return void
     */
    virtual bool DeliverIgniteStartMessage() = 0;

    /**
     * This function generates and delivers remote operation message to device
     * platform.
     * @param[in] rJsonMessage JSON object containing remote operation message
     * @param[in] rStrTopic String containing remote operation topic
     * @return void
     */
    virtual bool DeliverRemoteOperationMessage
        (const ic_utils::Json::Value& rJsonMessage, const std::string& rStrTopic) = 0;

    /**
     * This function generates and delivers device activation status event to
     * device platform based on input parameter.
     * @param[in] rbState Device activation status
     * @param[in] rnNotActivationReason Reason if activation fails
     * @return void
     */
    virtual bool DeliverDeviceActivationStatusMessage
        (const bool &rbState, const int &rnNotActivationReason) = 0;

    /**
     * This function generates and delivers 'VINRequest' event to device
     * platform
     * @param void
     * @return void
     */
    virtual bool DeliverVINRequestToDevice() = 0;

    /**
     * This function generates and delivers ignite client running status event
     * to device platform
     * @param[in] rJsonData JSON object containing ignite client status
     * @return True if status is delivered successfully, false otherwise
     */
    virtual bool DeliverICStatusToDevice(const ic_utils::Json::Value &rJsonMessage) = 0;

    /**
     * This function generates and delivers 'DeviceShutdownNotifAck' event to
     * device platform based on input parameter
     * @param[in] rJsonMessage JSON object containing shutdown notification
     * @return True if message is not NULL and delivered successfully,
     * false otherwise.
     */
    virtual bool DeliverShutdownNotifAckToDevice
        (const ic_utils::Json::Value &rJsonMessage) = 0;

    /**
     * This function generates and delivers 'ActivationStatus' event to device
     * platform based on input parameter
     * @param[in] rJsonData JSON object containing activation details
     * @return True if data is delivered successfully, false otherwise.
     */
    virtual bool DeliverActivationDetails(const ic_utils::Json::Value& rJsonMessage) = 0;

    /**
     * This function generates and delivers 'DBSize' event to device platform
     * based on input parameter
     * @param[in] rJsonData JSON object containing database size details
     * @return True if data is delivered successfully, false otherwise.
     */
    virtual bool DeliverDBSizeToDevice(const ic_utils::Json::Value &rJsonMessage) = 0;

    /**
     * This function generates and delivers 'MQTTConnectionStatus' event to
     * device platform based on input parameter
     * @param[in] rJsonData JSON object containing MQTT connection status
     * @return True if data is delivered successfully, false otherwise.
     */
    virtual bool DeliverMQTTConnectionStatusToDevice
        (const ic_utils::Json::Value& rJsonMessage) = 0;
};
}  /* namespace acp */
#endif //IMESSAGE_DISPATCHER_H

