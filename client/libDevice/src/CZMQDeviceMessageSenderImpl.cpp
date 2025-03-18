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

#ifdef ENABLE_ZMQ
#include <iostream>
#include <unistd.h>
#include "CZMQDeviceMessageSenderImpl.h"
#include "CIgniteLog.h"
#include "db/CLocalConfig.h"

#define ENGINE_URL "ipc:///tmp/ipcd_remote.ipc"
#define PUB_ENGINE_URL "ipc:///tmp/pub_ic.ipc"

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CZMQDeviceMessageSenderImpl"

namespace ic_device
{
static int UNIQUE_MSGID = 876345;
ic_utils::CIgniteMutex CZMQDeviceMessageSenderImpl::m_Mutex;

CZMQDeviceMessageSenderImpl::CZMQDeviceMessageSenderImpl()
    : m_bConnected(false), m_pZMQPubClient(NULL)
{
    m_pZMQPushClient = new CZMQPushClient(ENGINE_URL);
    m_bConnected = m_pZMQPushClient->Connect();
}

CZMQDeviceMessageSenderImpl::~CZMQDeviceMessageSenderImpl()
{
    if (m_pZMQPushClient)
    {
        delete m_pZMQPushClient;
        m_pZMQPushClient = NULL;
    }

    if (m_pZMQPubClient)
    {
        delete m_pZMQPubClient;
        m_pZMQPubClient = NULL;
    }
}

bool CZMQDeviceMessageSenderImpl::DispatchMessage(MESSAGE_ID eMsgId, 
                                                const std::string &rstrMsg)
{
    bool bStatus = false;

    switch (eMsgId)
    {
        case MESSAGE_ID::eRemoteOperationMessage:
        case MESSAGE_ID::eVINRequestToDevice:
        case MESSAGE_ID::eICStatusToDevice:
        case MESSAGE_ID::eShutdownNotifAckToDevice:
        case MESSAGE_ID::eActivationDetails:
        case MESSAGE_ID::eDBSizeToDevice:
        case MESSAGE_ID::eMQTTConnectionStatusToDevice:
            bStatus = DeliverMessage(rstrMsg);
            break;

        default:
            HCPLOG_E << "Non-supported Message ID - " << eMsgId;
            break;
    }

    return bStatus;
}

bool CZMQDeviceMessageSenderImpl::DeliverMessage(const std::string &rstrMsg)
{
    bool bRetVal = false;

    if (!m_bConnected)
    {
        m_bConnected = m_pZMQPushClient->Connect();
    }
    if (m_bConnected)
    {
        if (m_pZMQPushClient)
        {
            if (m_pZMQPushClient->SendMessage(rstrMsg))
            {
                HCPLOG_C << "Sent:" << rstrMsg;
                bRetVal = true;
            }
            else
            {
                HCPLOG_E << "Failed to send:" << rstrMsg;
                bRetVal = false;
            }
        }
        else
        {
            HCPLOG_E << "Invalid ZMQPushClient object!";
        }
    }
    return bRetVal;
}

bool CZMQDeviceMessageSenderImpl::PublishMessage(const std::string &rstrMsg)
{
    m_Mutex.Lock();
    bool bRetVal = false;

    if (m_pZMQPubClient == NULL)
    {
        m_pZMQPubClient = new CZMQPubClient(PUB_ENGINE_URL);
        usleep(500 * 1000);//wait for 500mS
    }

    if (m_pZMQPubClient != NULL)
    {
        HCPLOG_D << rstrMsg;
        if (m_pZMQPubClient->Publish(rstrMsg))
        {
            HCPLOG_C << "Message successfully published ";
            bRetVal = true;
        }
        else
        {
            HCPLOG_E << "Message failed to publish " << rstrMsg;
        }
    }
    m_Mutex.Unlock();
    return bRetVal;
}
}
#endif //#ifdef ENABLE_ZMQ
