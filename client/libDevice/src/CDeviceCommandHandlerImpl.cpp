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

#include "CIgniteLog.h"
#include "CIgniteClient.h"
#include "CIgniteEvent.h"
#include "CDeviceCommandHandlerImpl.h"

#ifdef ENABLE_ZMQ
#include "CZMQReceiveMessage.h"
#endif

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CDeviceCommandHandlerImpl"

namespace ic_device
{

CDeviceCommandHandlerImpl::CDeviceCommandHandlerImpl()
{
}

CDeviceCommandHandlerImpl::~CDeviceCommandHandlerImpl()
{
}

#if IC_UNIT_TEST == 1
bool CDeviceCommandHandlerImpl::SimulateShutdownNotif()
{
    //sample DeviceShutdownNotif payload with exitType 2
    std::string strShutdownNotif = "{\"EventID\": \"DeviceShutdownNotif\", \
                                    \"Timestamp\": 1635853382039,\"Timezone\": 330,\"Version\": \"1.0\", \
                                    \"Data\": {\"state\":1,\"exitOnComplete\":true, \"exitType\":2, \
                                    \"timeout\":60}}";

    return ProcessDeviceCommand(strShutdownNotif);
}
#endif

bool CDeviceCommandHandlerImpl::ProcessDeviceCommand
    (const std::string &rstrCmdPayLoad)                                                 
{
    HCPLOG_METHOD();

    ic_utils::Json::Reader jsonReader;
    ic_utils::Json::Value jsonPayload = ic_utils::Json::Value::nullRef;
    bool bStatus = false;

    if (!jsonReader.parse(rstrCmdPayLoad, jsonPayload))
    {
        HCPLOG_E << "DeviceCmd parse error..." << rstrCmdPayLoad;
        return bStatus;
    }
    std::string strCmdID = jsonPayload[KEY_EVENTID].asString();

    if (EVENT_SHUTDOWN_NOTIF == strCmdID)
    {
        HandleShutdownNotif(jsonPayload);
        bStatus = true;
    }
    else if (EVENT_ACTIVATION_STATUS_QUERY == strCmdID)
    {
        HandleActivationStatusQuery();
        bStatus = true;
    }
    else if (EVENT_DB_SIZE_QUERY == strCmdID)
    {
        HandleDBSizeQuery();
        bStatus = true;
    }
    else if (EVENT_MQTT_STATUS_QUERY == strCmdID)
    {
        HandleMQTTConnStatusQuery();
        bStatus = true;
    }
    else if (EVENT_RO_RESPONSE == strCmdID)
    {
        HandleROResponse(rstrCmdPayLoad);
        bStatus = true;
    }
    else
    {
        HCPLOG_I << "Cmd not supported..." << strCmdID;
    }

    return bStatus;
}

void CDeviceCommandHandlerImpl::HandleShutdownNotif
    (const ic_utils::Json::Value &jsonPayload)
{
    HCPLOG_METHOD();

    if (!(jsonPayload.isMember("Data") && jsonPayload["Data"].isObject()))
    {
        HCPLOG_E << "Invalid cmd! " << jsonPayload.toStyledString();
        return;
    }

    ic_utils::Json::Value jsonData = jsonPayload["Data"];
    if (!(jsonData.isMember("state") && jsonData["state"].isInt()))
    {
        HCPLOG_E << "state attribute not found! " 
                 << jsonPayload.toStyledString();
        return;
    }

    int nState = jsonData["state"].asInt();
    if (PREPARE_FOR_SHUTDOWN == nState)
    {
        HandlePrepareForShutdown(jsonData);
    }
    else if (SHUTDOWN_INITIATED == SHUTDOWN_INITIATED)
    {
        ic_core::CIgniteClient::ShutdownInitiated();
    }
    else
    {
        HCPLOG_E << "Unsupported state! " << nState;
    }
}
void CDeviceCommandHandlerImpl::HandlePrepareForShutdown
    (const ic_utils::Json::Value &rjsonData)
{
    if (rjsonData.isMember("timeout") && rjsonData["timeout"].isInt())
        {
            bool bExitOnComplete = false; // default is false
            ic_core::IC_EXIT_TYPE enumExitType = 
                ic_core::IC_EXIT_TYPE::eQUICK_EXIT; // default is QUICK_EXIT

            if (rjsonData.isMember("exitOnComplete") && 
                rjsonData["exitOnComplete"].isBool())
            {
                bExitOnComplete = rjsonData["exitOnComplete"].asBool();
                if (rjsonData.isMember("exitType") && 
                    rjsonData["exitType"].isInt())
                {
                    int nExitType = rjsonData["exitType"].asUInt();
                    if ((ic_core::IC_EXIT_TYPE::eQUICK_EXIT == nExitType) ||
                       (ic_core::IC_EXIT_TYPE::eNORMAL_EXIT == nExitType))
                    {
                        enumExitType = (ic_core::IC_EXIT_TYPE)nExitType;
                    }
                }
            }
            ic_core::CIgniteClient::PrepareForShutdown(
                                                    rjsonData["timeout"].asInt(), 
                                                    bExitOnComplete, 
                                                    enumExitType);
        }
        else
        {
            HCPLOG_E << "timeout attribute not-found/invalid!";
        }
}
void CDeviceCommandHandlerImpl::HandleActivationStatusQuery()
{
    // Querying and sending Activation Status to Device
    bool bRet = ic_core::CIgniteClient::GetClientMessageDispatcher()-> \
        DeliverActivationDetails(ic_core::CIgniteClient::QueryParameter(
        ic_core::IC_QUERY_ID::eACTIVATION_STATUS));
    if (true == bRet)
    {
        HCPLOG_C << "Activation Status sent to Device";
    }
    else
    {
        HCPLOG_E << "Error is sending Activation Status to Device";
    }
}

void CDeviceCommandHandlerImpl::HandleDBSizeQuery()
{
    // Querying and sending DB Size to Device
    bool bRet = ic_core::CIgniteClient::GetClientMessageDispatcher()-> \
        DeliverDBSizeToDevice(ic_core::CIgniteClient::QueryParameter(
        ic_core::IC_QUERY_ID::eDB_SIZE));
    if (true == bRet)
    {
        HCPLOG_C << "DB Size sent to Device";
    }
    else
    {
        HCPLOG_E << "Error is sending DB Size to Device";
    }
}

void CDeviceCommandHandlerImpl::HandleMQTTConnStatusQuery()
{
    // Querying and sending MQTT Connection Status to Device
    bool bRet = ic_core::CIgniteClient::GetClientMessageDispatcher()->\
        DeliverMQTTConnectionStatusToDevice(
        ic_core::CIgniteClient::QueryParameter(
        ic_core::IC_QUERY_ID::eMQTT_CONN_STATUS));
    if (true == bRet)
    {
        HCPLOG_C << "MQTT Connection Status sent to Device";
    }
    else
    {
        HCPLOG_E << "Error is sending MQTT Connection Status to Device";
    }
}

void CDeviceCommandHandlerImpl::HandleROResponse(
                                        const std::string &rstrRORespPayLoad)
{
    /* Incoming ro response event string is converted to actual event and
     * sent to cloud in the event format
     */
    ic_event::CIgniteEvent roResponseEvent;
    roResponseEvent.JsonToEvent(rstrRORespPayLoad);

    roResponseEvent.Send();
}

void CDeviceCommandHandlerImpl::StartListening(void)
{
#ifdef ENABLE_ZMQ
    CZMQReceiveMessage::GetInstance()->StartListening(this);
#endif
}

void CDeviceCommandHandlerImpl::StopListening(void)
{
    HCPLOG_METHOD();
#ifdef ENABLE_ZMQ
    CZMQReceiveMessage::GetInstance()->StopListening();
#endif
}

} // namespace ic_device
