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

#include "CIgniteConfig.h"
#include "db/CLocalConfig.h"
#include "dam/CEventWrapper.h"
#include "CIgniteEvent.h"
#include "CIgniteLog.h"
#include "CRemoteService.h"
#include "CIgniteClient.h"

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CRemoteService"

using namespace std;

namespace ic_auto
{

namespace
{
static int  UNIQUE_MSGID = 349654;
}

CRemoteService::CRemoteService()
{
    m_strDeviceId = "";
    m_strSerTopic = "";
    // Reading topic from config file
    ic_utils::Json::Value jsonMqRoot = 
                    ic_core::CIgniteConfig::GetInstance()->GetJsonValue("MQTT");
    string topicPrefix = jsonMqRoot["topicprefix"].asString();

    ic_utils::Json::Value jsonMqRemoteService = 
                                    GetServiceJsonFromConfig("RemoteOperation");
    if(!jsonMqRemoteService.isNull()) {
        HCPLOG_C << "RemoteOperation services configured";
        m_strTopicWithoutId = topicPrefix;
        m_strSerTopic = jsonMqRemoteService["pub"].asString();
        m_nQos = jsonMqRemoteService["QOS"].asInt();
    } else {
        HCPLOG_D << "RemoteOperation services not configured";
    }
}

CRemoteService::~CRemoteService()
{

}

CRemoteService* CRemoteService::GetInstance()
{
    HCPLOG_METHOD();
    static CRemoteService instance;
    return &instance;
}

void CRemoteService::ReleaseInstance()
{
    HCPLOG_METHOD();

    //do required cleanup here
}

int CRemoteService::SendResponse(string& rstrReqId, string& rstrMsgId,
                                                    string& rstrBizId)
{
    ic_event::CIgniteEvent roResponseEvent("1.1","RemoteOperationResponse");
    roResponseEvent.AddField("response", "SUCCESS_CONTINUE");
    roResponseEvent.AddField("roRequestId", rstrReqId);
    string strFullTopic = m_strTopicWithoutId + m_strDeviceId + "/"
                                                              + m_strSerTopic;
    roResponseEvent.AddField("topic", strFullTopic);
    stringstream strStream;
    strStream << ++UNIQUE_MSGID;
    roResponseEvent.AddMessageId(strStream.str());
    roResponseEvent.AddBizTransaction(rstrBizId);
    roResponseEvent.AddCorrelationId(rstrMsgId);

    return roResponseEvent.Send();
}

void CRemoteService::ApplyConfig(ic_utils::Json::Value &rjsonConfig)
{
    m_RemoteMutex.Lock();
    HandleRemoteCommand(rjsonConfig);
    m_RemoteMutex.Unlock();
}


int CRemoteService::HandleRemoteCommand(
                                       const ic_utils::Json::Value& rjsonConfig)
{
    int nResponseValue;
    HCPLOG_D << "PROCESSING REMOTE COMMAND";
    if(m_strDeviceId.empty()) {
        // Set the device Id
        m_strDeviceId = ic_core::CLocalConfig::GetInstance()->Get("login");
        HCPLOG_I << "Got device ID = " << m_strDeviceId;
    }

    ic_utils::Json::FastWriter writer;
    string strNotification = writer.write(rjsonConfig);
    HCPLOG_I << "RO SERVICE NOTIFICATION = " << strNotification << endl;

    string strEventId = rjsonConfig["EventID"].asString();
    HCPLOG_C << "Recieved:"<< strEventId;

    if (strEventId == "Acknowledgement") {
        HCPLOG_I << "Recieved ack messages, no need to process it";
        return ACK_RCVD;
    }

    ic_utils::Json::Value jsonData = rjsonConfig["Data"];
    string strState = jsonData["state"].asString();
    string strReqId = jsonData["roRequestId"].asString();
    string strMessageId = rjsonConfig["MessageId"].asString();
    string strBizTrazId = rjsonConfig["BizTransactionId"].asString();
    string strResponseTopic = m_strTopicWithoutId + m_strDeviceId + "/" +
                                                                m_strSerTopic;
    // Generate a response of the remote command
    nResponseValue = SendResponse(strReqId, strMessageId, strBizTrazId);

    ic_core::IClientMessageDispatcher *dispatcher =
                        ic_core::CIgniteClient::GetClientMessageDispatcher();
    if(dispatcher != NULL)
    {
       dispatcher->DeliverRemoteOperationMessage(rjsonConfig, strResponseTopic);
    }

   return nResponseValue;
}

} // namespace ic_auto
