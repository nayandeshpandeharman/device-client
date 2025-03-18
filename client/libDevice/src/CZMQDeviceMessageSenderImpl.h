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
* \file ZMQDeviceMessageDispatcher.h
*
* \brief This class provides methods to deliver messages over ZeroMQ IPC
*
*******************************************************************************
*/

#ifdef ENABLE_ZMQ
#ifndef ZMQ_DEVICE_MESSAGE_SENDER_IMPL_H
#define ZMQ_DEVICE_MESSAGE_SENDER_IMPL_H

#include <string>
#include "CZMQClient.h"
#include "IDeviceMessageSender.h"
#include "CIgniteMutex.h"
#include "CIgniteEvent.h"

namespace ic_device
{

/**
 * class CZMQDeviceMessageDispatcher provide methods to deliver messages over 
 * ZeroMQ IPC
 */
class CZMQDeviceMessageSenderImpl : public IDeviceMessageSender
{
public:
    /**
     * Default constructor
     */
    CZMQDeviceMessageSenderImpl();

    /**
     * Destructor
     */
    virtual ~CZMQDeviceMessageSenderImpl();

    /**
     * This function delivers message over ZMQ IPC using either push client or
     * pub client. Decision is taken based on defined MESSAGE_IDs.
     * @see IDeviceMessageDispatcher::DispatchMessage()
     * @param[in] eMsgId Message ID of the rstrMsg
     * @param[in] rstrMsg String messag to be delivered
     * @return True if message delivery is successful, false otherwise.
     */
    bool DispatchMessage(MESSAGE_ID eMsgId, const std::string &rstrMsg) override;

private:
    /**
     * This function publishes message over ZMQ IPC using pub client
     * @param[in] rstrMsg String messag to be published
     * @return True if publish is successful, false otherwise.
     */
    bool PublishMessage(const std::string &rstrMsg);

    /**
     * This function delivers message over ZMQ IPC using push client
     * @param[in] rstrMsg String messag to be published
     * @return True if message delivery is successful, false otherwise.
     */
    bool DeliverMessage(const std::string &rstrMsg);

    /**
     * Member variable pointing to CZMQPushClient class
     */
    CZMQPushClient *m_pZMQPushClient;

    /**
     * Member variable pointing to CZMQPubClient class
     */
    CZMQPubClient *m_pZMQPubClient;

    /**
     * Member variable containing connection status of m_pZMQPushClient
     */
    bool m_bConnected;

    /**
     * Member mutex variable
     */
    static ic_utils::CIgniteMutex m_Mutex;

    #ifdef IC_UNIT_TEST
        friend class CZMQDeviceMessageSenderImplTest;
    #endif
};

}
#endif // #ifndef ZMQ_DEVICE_MESSAGE_SENDER_IMPL_H
#endif // #ifdef ENABLE_ZMQ
