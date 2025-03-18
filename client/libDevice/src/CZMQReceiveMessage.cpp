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
#include "CZMQReceiveMessage.h"
#include "CIgniteLog.h"
#include "CZMQClient.h"
#include "CIgniteClient.h"
#include "CIgniteConfig.h"
#include "db/CLocalConfig.h"

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CZMQReceiveMessage"

namespace ic_device
{

#define ENGINE_NOTIFY_URL "ipc:///tmp/ipcd_notif.ipc"

CZMQReceiveMessage::CZMQReceiveMessage(const std::string &rstrEngineUri)
    : CZMQPullClient(rstrEngineUri), m_bIsShutdownInitiated(false)
{
    m_pZMQPushClient = new CZMQPushClient(ENGINE_NOTIFY_URL);
}

CZMQReceiveMessage::~CZMQReceiveMessage()
{
    if (m_pZMQPushClient != NULL)
    {
        delete m_pZMQPushClient;
    }
}

CZMQReceiveMessage *CZMQReceiveMessage::GetInstance()
{
    static CZMQReceiveMessage ZMQMessageObject(ENGINE_NOTIFY_URL);
    return &ZMQMessageObject;
}

void CZMQReceiveMessage::StartListening(IDeviceCommandHandler *pCommandHandler)
{
    HCPLOG_METHOD();

    m_pCommandHandler = pCommandHandler;

    if (NULL != m_pvoidSocket)
    {
        Start();
    }
    else
    {
        HCPLOG_E << "could not start listen to ZMQ Channel!";
    }
}

void CZMQReceiveMessage::StopListening(void)
{
    HCPLOG_METHOD();
    m_pCommandHandler = NULL;
    ZMQClose();
    Stop();
}

void CZMQReceiveMessage::Run()
{
    HCPLOG_METHOD();

    ic_core::CIgniteClient::GetOnOffMonitor()-> RegisterForShutdownNotification
        (this, ic_core::IOnOff::eR_ZMQ_RECEIVE_MESSAGE);

    std::string strResult("");

    while (!m_bIsShutdownInitiated)
    {
        strResult = CZMQPullClient::RecvMessage();
        if (m_bIsShutdownInitiated)
        {
            break;
        }
        if (!strResult.empty())
        {
            HCPLOG_C << "Message Rcvd=:" << strResult;
            ProcessPayload(strResult);
        }
    }
    Detach();
    ic_core::CIgniteClient::GetOnOffMonitor()->ReadyForShutdown
        (ic_core::IOnOff::eR_ZMQ_RECEIVE_MESSAGE);
    ic_core::CIgniteClient::GetOnOffMonitor()->UnregisterForShutdownNotification
        (ic_core::IOnOff::eR_ZMQ_RECEIVE_MESSAGE);
}

void CZMQReceiveMessage::ProcessPayload(const std::string &rstrPayLoadStr)
{
    HCPLOG_METHOD();
    m_pCommandHandler->ProcessDeviceCommand(rstrPayLoadStr);
}

void CZMQReceiveMessage::NotifyShutdown()
{
    m_bIsShutdownInitiated = true;
    HCPLOG_I << "sending internal msg to break zmq msg read loop";
    if (m_pZMQPushClient)
    {
        if (m_pZMQPushClient->Connect())
        {
            std::string strMsg = "msg-to-break-zmq-read-loop";
            if (m_pZMQPushClient->SendMessage(strMsg))
            {
                HCPLOG_I << "sent the msg...";
            }
            else
            {
                HCPLOG_E << "could not send the msg.";
            }
        }
        else
        {
            HCPLOG_E << "ZMQPushClient is not connected";
        }
    }
    else
    {
        HCPLOG_E << "could not instantiate ZMQPushClient";
    }
}
}
#endif // #ifdef ENABLE_ZMQ
