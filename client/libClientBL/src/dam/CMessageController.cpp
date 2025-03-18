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

#include <set>
#include <algorithm>
#include "CMessageController.h"
#include "core/CClientConnectorImpl.h"
#include "CIgniteConfig.h"
#include "CIgniteLog.h"
#include "dam/CEventWrapper.h"
#include "CIgniteConfig.h"
#include "CIgniteLog.h"
#include "upload/CUploadController.h"
#include "db/CLocalConfig.h"

//! Macro for CMessageController string
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CMessageController"

namespace ic_bl 
{
namespace {

//! Global variable to set of alert Ids
std::set<std::string> g_setAlerts;

//! Constant key for 1MB int
static const int MAX_QUEUE_SIZE = 1000000; // 1MB
}

CMessageController::CMessageController(CTransportHandlerBase* pNextHandler)
    : CTransportHandlerBase(pNextHandler), m_bIsShutdownInitiated(false)
{
    Init();
    Start();
}

CMessageController::~CMessageController()
{
    //delete the active handler objects
    for (std::vector<CMessageController::MessageHandler>::iterator iter = m_vecHndlrList.begin() ; iter != m_vecHndlrList.end(); ++iter)
    {
        delete iter->pHandlerRef;
        iter->pHandlerRef = NULL;
    }

    m_vecHndlrList.clear();

    m_pImh = NULL;

#ifdef INCLUDE_IGNITE_AUTO
    m_pAmh = NULL;
#endif
}

void CMessageController::Init()
{
    HCPLOG_METHOD();

    m_pImh = NULL;
    if (ic_core::CIgniteConfig::GetInstance()->GetBool("MessageHandlers.primaryHandlerEnabled", true))
    {
        HCPLOG_T << "Adding internal message handler...";
        m_pImh = new CInternalMessageHandler();
        AddHandler(m_pImh);
        m_pImh->SetClientConnector(CClientConnectorImpl::GetInstance());
    }

#ifdef INCLUDE_IGNITE_AUTO
    HCPLOG_T << "Adding Ignite Analytics message handler...";
    m_pAmh = new ic_auto::CAnalyticsMessageHandler();
    AddHandler(m_pAmh);
    m_pAmh->SetClientConnector(CClientConnectorImpl::GetInstance());
#endif
}

bool CMessageController::IsAlert(const std::string& rstrEventID)
{
    if (g_setAlerts.size() == 0)
    {
        ic_utils::Json::Value jsonAlertArray = ic_core::CIgniteConfig::GetInstance()->GetJsonValue("MQTT.DirectAlerts");
        if (jsonAlertArray.isArray())
        {
            for (int i = 0; i < jsonAlertArray.size() ; i++)
            {
                g_setAlerts.insert(jsonAlertArray[i].asString());
            }
        }
    }
    return (g_setAlerts.find(rstrEventID) != g_setAlerts.end());
}

void CMessageController::HandleEvent(ic_core::CEventWrapper* pEvent)
{
    std::string strEventId = pEvent->GetEventId();
    if (IsAlert(strEventId))
    {
        //notify to upload alerts
        //direct alert is been sent , uploadAlert as it is
        HCPLOG_W << "Direct Alert for : " << strEventId;
        CUploadController::GetInstance()->TriggerAlertsUpload(START_ALERT_UPLOAD);
    }

    if (m_queMqttEvents.Size() < MAX_QUEUE_SIZE)
    {
        std::string strSerialized;
        pEvent->EventToJson(strSerialized);
        m_queMqttEvents.Put(strSerialized, strSerialized.size());
        Notify(); //Process mqtt events
    }
    else
    {
        HCPLOG_E << "MQTT alerts queue full, no alerts will be raised for event :" << strEventId;
    }

    m_pNextHandler->HandleEvent(pEvent);
}

void CMessageController::HandleNotification(const std::string& rstrdomain, const ic_utils::Json::Value& rjsonNotif)
{
    //check if any handler is interested for this notification
    for (std::vector<CMessageController::MessageHandler>::iterator iter = m_vecHndlrList.begin() ; iter != m_vecHndlrList.end(); ++iter)
    {
        if (iter->pHandlerRef->IsHandlerSubscribedForNotification(rstrdomain))
        {
            ic_utils::Json::Value jsonTemp;
            jsonTemp["domain"] = rstrdomain;
            jsonTemp["notif"] = rjsonNotif;

            ic_core::IMessageHandler::MsgPayload payload;
            payload.eType = ic_core::IMessageHandler::eMSG_TYPE_NOTIFICATION;
            ic_utils::Json::FastWriter writer;
            payload.strPayloadJson = writer.write(jsonTemp);

            HCPLOG_T << "Domain:" << rstrdomain << "; Notifying handler " << iter->pHandlerRef;
            iter->pHandlerRef->NotifyMessage(payload);
        }
    }
}

void CMessageController::ProcessEvent(const std::string& rstrEventId, const std::string& rstrSerEvent)
{
    //check if any handler is interested for this event
    for (std::vector<CMessageController::MessageHandler>::iterator iter = m_vecHndlrList.begin() ; iter != m_vecHndlrList.end(); ++iter)
    {
        if (iter->pHandlerRef->IsHandlerSubscribedForEvent(rstrEventId))
        {
            ic_core::IMessageHandler::MsgPayload payload;
            payload.eType = ic_core::IMessageHandler::eMSG_TYPE_EVENT;
            payload.strPayloadJson = rstrSerEvent;

            iter->pHandlerRef->NotifyMessage(payload);
        }
    }
}

void CMessageController::FlushCache()
{
    //Execute multiple insertions as single transaction for optimized performance
    ic_core::CDataBaseFacade* pDb = ic_core::CDataBaseFacade::GetInstance();
    bool bTransactionStarted = pDb->StartTransaction();
    std::string strRet;
    while(m_queMqttEvents.Take(&strRet) && strRet.size() > 0)
    {
        ic_core::CEventWrapper event;
        event.JsonToEvent(strRet);
        ProcessEvent(event.GetEventId(), strRet);
        HCPLOG_W << "flushing cache: " << strRet;
    }
    if (bTransactionStarted)
    {
        pDb->EndTransaction(true);
    }
}

void CMessageController::Run()
{
    ic_core::CIgniteClient::GetOnOffMonitor()->RegisterForShutdownNotification(this, ic_core::IOnOff::eR_MESSAGE_CONTROLLER);

    //initialize handlers to create eventToDomain map, so performance of Event Handling will not be impacted
    for (std::vector<CMessageController::MessageHandler>::iterator iter = m_vecHndlrList.begin() ; iter != m_vecHndlrList.end(); ++iter)
    {
        iter->pHandlerRef->Init();
    }


    while (!m_bIsShutdownInitiated)
    {
        if (m_queMqttEvents.Size() == 0)
        {
            //Wait for mqtt events
            Wait();

            //if Wait() is broken due to shutdown
            if (m_bIsShutdownInitiated) {
                break;
            }
        }
        std::string strEventJson;
        if (m_queMqttEvents.Take(&strEventJson) && strEventJson.size() > 0)
        {
            ic_core::CEventWrapper event;
            event.JsonToEvent(strEventJson);
            ProcessEvent(event.GetEventId(), strEventJson);
        }
    }

    ic_core::CIgniteClient::GetOnOffMonitor()->ReadyForShutdown(ic_core::IOnOff::eR_MESSAGE_CONTROLLER);
    ic_core::CIgniteClient::GetOnOffMonitor()->UnregisterForShutdownNotification(ic_core::IOnOff::eR_MESSAGE_CONTROLLER);
    Detach();
}

void CMessageController::Wait()
{
    m_mqttEvWaitMutex.TryLock();
    m_mqttEvWaitCondition.ConditionWait(m_mqttEvWaitMutex);
}

void CMessageController::Notify()
{
    m_mqttEvWaitCondition.ConditionBroadcast();
    m_mqttEvWaitMutex.Unlock();
}

std::vector<CMessageController::MessageHandler> CMessageController::m_vecHndlrList;
void CMessageController::AddHandler(ic_core::IMessageHandler *pHandler)
{
    HCPLOG_METHOD();

    MessageHandler aHndlr;
    aHndlr.pHandlerRef = pHandler;
    m_vecHndlrList.push_back(aHndlr);
}

void CMessageController::NotifyShutdown()
{
    HCPLOG_METHOD();
    m_bIsShutdownInitiated = true;
    Notify();
}

std::set<std::string> CMessageController::GetSupplimentaryEventsListToWhitelist()
{
    //get the list from CInternalMessageHandler
    std::set<std::string> setMcEventList =
            m_pImh->GetSupplimentaryEventsList();
#ifdef INCLUDE_IGNITE_NALYTICS
    //get the list from AnalyticsMessageHandler
    std::set<std::string> setAmhEventList =
            m_pAmh->GetSupplimentaryEventsList();

    //combine the list
    setMcEventList.insert(setAmhEventList.begin(), setAmhEventList.end());
#endif
    return setMcEventList;
}
} /* namespace ic_bl*/
