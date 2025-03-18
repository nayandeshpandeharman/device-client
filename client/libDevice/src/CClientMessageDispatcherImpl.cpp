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

#include <iostream>
#include "CIgniteLog.h"
#include <unistd.h>

#include "CClientMessageDispatcherImpl.h"
#include "db/CLocalConfig.h"

#ifdef ENABLE_ZMQ
#include "CZMQDeviceMessageSenderImpl.h"
#else
#include "CDefaultDeviceMessageSenderImpl.h"
#endif

namespace ic_device
{
namespace
{
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CClientMessageDispatcherImpl"

static int UNIQUE_MSGID = 876345;
}

CClientMessageDispatcherImpl::CClientMessageDispatcherImpl()
{
#ifdef ENABLE_ZMQ
    m_pDeviceMsgSender = new CZMQDeviceMessageSenderImpl();
#else
    m_pDeviceMsgSender = new CDefaultDeviceMessageSenderImpl();
#endif
    if (!m_pDeviceMsgSender)
    {
        HCPLOG_F << "could not create MessageDispatcher!";
        m_pDeviceMsgSender = NULL;
    }
}

CClientMessageDispatcherImpl::~CClientMessageDispatcherImpl()
{
    if (m_pDeviceMsgSender)
    {
        delete m_pDeviceMsgSender;
        m_pDeviceMsgSender = NULL;
    }
}

int CClientMessageDispatcherImpl::SendROSuccessResponse(std::string &rstrReqId,
                                             std::string &rstrMsgId,
                                             std::string &rstrBizId,
                                             const std::string &rstrTopic)
{
    ic_event::CIgniteEvent roResponseEvent("1.1", "RemoteOperationResponse");
    roResponseEvent.AddField("response", "SUCCESS");
    roResponseEvent.AddField("roRequestId", rstrReqId);
    roResponseEvent.AddField("topic", rstrTopic);
    std::stringstream ss;
    ss << ++UNIQUE_MSGID;
    roResponseEvent.AddMessageId(ss.str());
    roResponseEvent.AddBizTransaction(rstrBizId);
    roResponseEvent.AddCorrelationId(rstrMsgId);

    return roResponseEvent.Send();
}

bool CClientMessageDispatcherImpl::DeliverIgniteStartMessage()
{
    HCPLOG_W << "Not supported yet!";
    return false;
}

bool CClientMessageDispatcherImpl::DeliverRemoteOperationMessage
    (const ic_utils::Json::Value &rJsonMessage, const std::string &rStrTopic)
{
    bool bStatus = false;

    if (rJsonMessage != ic_utils::Json::Value::nullRef)
    {
        ic_utils::Json::Value jsonConfigVal(rJsonMessage);
        ic_utils::Json::Value &jsonData = jsonConfigVal["Data"];
        std::string strEventId = jsonConfigVal["EventID"].asString();

        std::string strState = jsonData["state"].asString();
        std::string strReqId = jsonData["roRequestId"].asString();
        std::string strMessageId = jsonConfigVal["MessageId"].asString();
        std::string strBizTrazId = jsonConfigVal["BizTransactionId"].asString();

        jsonData["topic"] = rStrTopic;

        ic_utils::Json::FastWriter jsonFastwriter;
        std::string strMsg = jsonFastwriter.write(jsonConfigVal);

        bStatus = m_pDeviceMsgSender->DispatchMessage(
                                   MESSAGE_ID::eRemoteOperationMessage, strMsg);
    }

    return bStatus;
}

bool CClientMessageDispatcherImpl::DeliverDeviceActivationStatusMessage
    (const bool &rbState, const int &rnNotActivationReason)
{
    HCPLOG_W << "Not supported yet!";
    return false;
}

bool CClientMessageDispatcherImpl::DeliverVINRequestToDevice()
{
    HCPLOG_W << "Not supported yet!";
    return false;
}

bool CClientMessageDispatcherImpl::DeliverICStatusToDevice
    (const ic_utils::Json::Value &rJsonMessage)
{
    bool bStatus = false;

    if (rJsonMessage != ic_utils::Json::Value::nullRef)
    {
        ic_event::CIgniteEvent eventICStatus("1.0", "ICRunningStatus");
        ic_utils::Json::Value::Members jsonMembers = rJsonMessage.getMemberNames();
        for (int i = 0; i < jsonMembers.size(); ++i)
        {
            eventICStatus.AddField(jsonMembers[i], rJsonMessage[jsonMembers[i]]);
        }
        std::string strMsg = "";
        eventICStatus.EventToJson(strMsg);

        bStatus = m_pDeviceMsgSender->DispatchMessage(MESSAGE_ID::eICStatusToDevice, strMsg);
    }

    return bStatus;
}

bool CClientMessageDispatcherImpl::DeliverShutdownNotifAckToDevice
    (const ic_utils::Json::Value &rJsonMessage)
{
    bool bStatus = false;

    if (rJsonMessage != ic_utils::Json::Value::nullRef)
    {
        ic_event::CIgniteEvent eventAck("1.0","DeviceShutdownNotifAck");
        ic_utils::Json::Value::Members jsonMembers = rJsonMessage.getMemberNames();
        for (int i = 0; i < jsonMembers.size(); ++i)
        {
            eventAck.AddField(jsonMembers[i], rJsonMessage[jsonMembers[i]]);
        }

        std::string strMsg = "";
        eventAck.EventToJson(strMsg);

        bStatus = m_pDeviceMsgSender->DispatchMessage(MESSAGE_ID::eShutdownNotifAckToDevice, strMsg);
    }

    return bStatus;
}

bool CClientMessageDispatcherImpl::DeliverActivationDetails
    (const ic_utils::Json::Value &rJsonMessage)
{
    bool bStatus = false;

    if (rJsonMessage != ic_utils::Json::Value::nullRef)
    {
        ic_event::CIgniteEvent eventActivationStatus("1.0", "ActivationStatus");

        ic_utils::Json::Value::Members jsonMembers = rJsonMessage.getMemberNames();
        for (int i = 0; i < jsonMembers.size(); ++i)
        {
            eventActivationStatus.AddField(jsonMembers[i], rJsonMessage[jsonMembers[i]]);
        }

        std::string strMsg = "";
        eventActivationStatus.EventToJson(strMsg);

        bStatus = m_pDeviceMsgSender->DispatchMessage(MESSAGE_ID::eActivationDetails, strMsg);
    }

    return bStatus;
}

bool CClientMessageDispatcherImpl::DeliverDBSizeToDevice
    (const ic_utils::Json::Value &rJsonMessage)
{
    bool bStatus = false;

    if (rJsonMessage != ic_utils::Json::Value::nullRef)
    {
        ic_event::CIgniteEvent eventDBSize("1.0", "DBSize");

        ic_utils::Json::Value::Members jsonMembers = rJsonMessage.getMemberNames();
        for (int i = 0; i < jsonMembers.size(); ++i) {
            eventDBSize.AddField(jsonMembers[i], rJsonMessage[jsonMembers[i]]);
        }

        std::string strMsg = "";
        eventDBSize.EventToJson(strMsg);

        bStatus = m_pDeviceMsgSender->DispatchMessage(MESSAGE_ID::eDBSizeToDevice, strMsg);
    }

    return bStatus;
}

bool CClientMessageDispatcherImpl::DeliverMQTTConnectionStatusToDevice
    (const ic_utils::Json::Value &rJsonMessage)
{
    bool bStatus = false;

    if (rJsonMessage != ic_utils::Json::Value::nullRef)
    {
        ic_event::CIgniteEvent eventMqttConnStatus("1.0", "MQTTConnectionStatus");

        ic_utils::Json::Value::Members jsonMembers = rJsonMessage.getMemberNames();
        for (int i = 0; i < jsonMembers.size(); ++i) {
            eventMqttConnStatus.AddField(jsonMembers[i], rJsonMessage[jsonMembers[i]]);
        }

        std::string strMsg = "";
        eventMqttConnStatus.EventToJson(strMsg);

        bStatus = m_pDeviceMsgSender->DispatchMessage(MESSAGE_ID::eMQTTConnectionStatusToDevice, strMsg);
    }

    return bStatus;
}

} //namespace ic_device
