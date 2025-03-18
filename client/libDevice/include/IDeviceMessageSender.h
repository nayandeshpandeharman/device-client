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
* \file IDeviceMessageSender.h
*
* \brief IDeviceMessageSender for handling messages that needs to be sent
* to the device.
*******************************************************************************
*/

#ifndef IDEVICE_MESSAGE_SENDER_H
#define IDEVICE_MESSAGE_SENDER_H

#include <iostream>
#include "CIgniteLog.h"

namespace ic_device
{

/**
 * Enum of various device message IDs
 */
enum MESSAGE_ID
{
    eRemoteOperationMessage,        ///< Msg id for RemoteOperationMessage
    eVINRequestToDevice,            ///< Msg id for VINRequestToDevice
    eICStatusToDevice,              ///< Msg id for ICStatusToDevice
    eShutdownNotifAckToDevice,      ///< Msg id for ShutdownNotifAckToDevice
    eActivationDetails,             ///< Msg id for ActivationDetails
    eDBSizeToDevice,                ///< Msg id for DBSizeToDevice
    eMQTTConnectionStatusToDevice,  ///< Msg id for MQTTConnectionStatusToDevice
    eUnkonwnMessageId               //< Msg id not in above list
};

/**
 * IDeviceMessageSender interface for handling messages that needs to be sent
 * to the device.
 */
class IDeviceMessageSender
{
public:
    /**
     * Virtual destructor
     */
    virtual ~IDeviceMessageSender() {};

    /**
     * This pure virtual function enables the implementor to send the given 
     * message payload to the device.
     * @param[in] eMsgId enum of message ID
     * @param[in] rstrMsg message payload in string format
     * @return True if successfully sent, false otherwise.
     */
    virtual bool DispatchMessage(MESSAGE_ID eMsgId, 
        const std::string &rstrMsg) = 0;
};
} //namespace ic_device
#endif // IDEVICE_MESSAGE_SENDER_H
