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
* \file CMessageDispatcher.h
*
* \brief CMessageDispatcher provide methods to deliver IC messages to device
* platform according to the defined channel/interface.
*******************************************************************************
*/

#ifndef CCLIENT_MESSAGE_DISPATCHER_IMPL_H
#define CCLIENT_MESSAGE_DISPATCHER_IMPL_H

#include <string>

#include "IDeviceMessageSender.h"
#include "IClientMessageDispatcher.h"
#include "CIgniteMutex.h"
#include "CIgniteEvent.h"

namespace ic_device
{
/**
 * class CClientMessageDispatcherImpl provides methods to deliver IC messages to 
 * device platform according to the defined channel/interface.
 */
class CClientMessageDispatcherImpl : public ic_core::IClientMessageDispatcher
{
private:
    /**
     * Member variable pointing to device platform specific message dispatcher
     * e.g. ZMQ interface
     */
    IDeviceMessageSender *m_pDeviceMsgSender;

public:
    /**
     * Default constructor
     */
    CClientMessageDispatcherImpl();

    /**
     * Virtual destructor
     */
    virtual ~CClientMessageDispatcherImpl();

    /**
     * This function sends a SUCCESS response for RemoteOperationResponse event.
     * @param[in] rstrReqId Request Id of the event.
     * @param[in] rstrMsgId Message Id of the event.
     * @param[in] rstrBizId BizTransaction Id of the event.
     * @param[in] rstrTopic Topic foe the event.
     * @return 0 if Remote Operation Response is sent successfully
     *         else return less than 0
     */
    int SendROSuccessResponse(std::string &rstrReqId, std::string &rstrMsgId,
                             std::string &rstrBizId, 
                             const std::string &rstrTopic);

    /**
     * Overriding Method of ic_core::IClientMessageDispatcher class
     * @see ic_core::IClientMessageDispatcher::DeliverIgniteStartMessage()
     */
    bool DeliverIgniteStartMessage() override;

    /**
     * Overriding Method of ic_core::IClientMessageDispatcher class
     * @see IClientMessageDispatcher::DeliverRemoteOperationMessage
        (const ic_utils::Json::Value &rJsonMessage, 
         const std::string &rStrTopic)
     */
    bool DeliverRemoteOperationMessage
        (const ic_utils::Json::Value &rJsonMessage, 
         const std::string &rStrTopic) override;

    /**
     * Overriding Method of ic_core::IClientMessageDispatcher class
     * @see IClientMessageDispatcher::DeliverDeviceActivationStatusMessage
        (const bool &rbState, const int &rnNotActivationReason)
     */
    bool DeliverDeviceActivationStatusMessage
        (const bool &rbState, const int &rnNotActivationReason) override;

    /**
     * Overriding Method of ic_core::IClientMessageDispatcher class
     * @see IClientMessageDispatcher::DeliverVINRequestToDevice(void)
     */
    bool DeliverVINRequestToDevice() override;

    /**
     * Overriding Method of ic_core::IClientMessageDispatcher class
     * @see IClientMessageDispatcher::DeliverICStatusToDevice
        (const ic_utils::Json::Value &rJsonData)
     */
    bool DeliverICStatusToDevice
        (const ic_utils::Json::Value &rJsonMessage) override;

    /**
     * Overriding Method of ic_core::IClientMessageDispatcher class
     * @see IClientMessageDispatcher::DeliverShutdownNotifAckToDevice
        (const ic_utils::Json::Value &rJsonMessage)
     */
    bool DeliverShutdownNotifAckToDevice
        (const ic_utils::Json::Value &rJsonMessage) override;

    /**
     * Overriding Method of ic_core::IClientMessageDispatcher class
     * @see IClientMessageDispatcher::DeliverActivationDetails
        (const ic_utils::Json::Value &rJsonData)
     */
    bool DeliverActivationDetails
        (const ic_utils::Json::Value &rJsonMessage) override;

    /**
     * Overriding Method of ic_core::IClientMessageDispatcher class
     * @see IClientMessageDispatcher::DeliverDBSizeToDevice
     *  (const ic_utils::Json::Value &rJsonData)
     */
    bool DeliverDBSizeToDevice(const ic_utils::Json::Value &rJsonMessage) override;

    /**
     * Overriding Method of ic_core::IClientMessageDispatcher class
     * @see IClientMessageDispatcher::DeliverMQTTConnectionStatusToDevice
        (const ic_utils::Json::Value &rJsonData)
     */
    bool DeliverMQTTConnectionStatusToDevice
        (const ic_utils::Json::Value &rJsonMessage) override;

    #ifdef IC_UNIT_TEST
        friend class CClientMessageDispatcherImplTest;

    /**
     * This functions gets IDeviceMessageSender object
     * @param[in] void
     * @return IDeviceMessageSender object
     *
     */
    IDeviceMessageSender * GetIDeviceMessageSender()
    {
        return m_pDeviceMsgSender;
    }
    #endif
};
}
#endif // #ifndef CMESSAGE_DISPATCHER_H
