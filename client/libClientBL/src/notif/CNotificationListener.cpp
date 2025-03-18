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
#include "CIgniteEvent.h"
#include "notif/CNotificationListener.h"
#include "jsoncpp/json.h"
#include "CIgniteClient.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CNotificationListener"

namespace ic_bl 
{

namespace 
{

//! Global variable for mutex
pthread_mutex_t g_Mutex = PTHREAD_MUTEX_INITIALIZER;

//! Global condition for indicating the availability of notification
pthread_cond_t g_condNotificationAvailable = PTHREAD_COND_INITIALIZER;

} /* end unnamed namespace */

CNotificationListener::CNotificationListener()
{
}

CNotificationListener::~CNotificationListener()
{
}

CNotificationListener* CNotificationListener::GetNotificationListener()
{
    static CNotificationListener instance;
    return &instance;
}

void CNotificationListener::PushNotification(std::string strData)
{
    pthread_mutex_lock( &g_Mutex );
    m_queNotifications.push(strData);
    pthread_cond_signal( &g_condNotificationAvailable );
    pthread_mutex_unlock( &g_Mutex );
}

std::string CNotificationListener::PopNotification()
{
    pthread_mutex_lock( &g_Mutex );
    if ( m_queNotifications.empty() )   //Wait for m_queNotifications !!
    {
        pthread_cond_wait( &g_condNotificationAvailable, &g_Mutex );
    }
    std::string strTemp("");
    if (!m_queNotifications.empty())
    {
        strTemp = m_queNotifications.front();
        m_queNotifications.pop();
    }
    pthread_mutex_unlock( &g_Mutex );
    return strTemp;
}

//Request Queue would be implemented for handling the requests.
void CNotificationListener::SendNotification(const char* cstrRequestJson)
{
    HCPLOG_T << "Parsing JSON";
    ic_event::CIgniteEvent* event = 
                    new ic_event::CIgniteEvent("0.1", "NotificationFailure");
    ic_utils::Json::Value jsonReq;
    ic_utils::Json::Reader jsonReader;

    if (!cstrRequestJson)
    {
        HCPLOG_E << "Request JSON is NULL, Return!";
        event->AddField("Error", "Received Empty Notification");
        event->Send();
    }
    else if(!jsonReader.parse(cstrRequestJson, jsonReq))
    {
        HCPLOG_E << "Received Invalid JSON for Request";
        event->AddField("Error", "Received Invalid JSON for Request");
        event->Send();
    }
    else
    {
        std::map<std::string, IRequestHandler*>::const_iterator iterMap = 
                        m_mapNotifHandlers.find(jsonReq["type"].asString());
        if (iterMap != m_mapNotifHandlers.end())
        {
            iterMap->second->Handle(cstrRequestJson);
        }
        else
        {
            HCPLOG_E << "Request Type not found :" << jsonReq["type"].asString();
            event->AddField("Error", 
                        "Request Type not found :" + jsonReq["type"].asString());
            event->Send();
        }
    }
    if (event)
    {
        delete event;
    }
}

void CNotificationListener::RegisterRequest(IRequestHandler *pReqHndlr, 
                                            std::string strReqType)
{
    std::pair <std::map<std::string, IRequestHandler*>::iterator, bool> pairRet;
    pairRet = m_mapNotifHandlers.insert(
        std::pair<std::string, IRequestHandler *>(strReqType, pReqHndlr));
    if (pairRet.second == false)
    {
        HCPLOG_W << "Handler already existed for" + strReqType;
    }
}

void CNotificationListener::Run()
{
    ic_core::CIgniteClient::GetOnOffMonitor()->RegisterForShutdownNotification(
                                this,ic_core::IOnOff::eR_NOTIFICATION_LISTENER);
    while(!m_bIsShutDownInitiated)
    {
        std::string strNotification = PopNotification();
        if (!strNotification.empty()) {
            SendNotification(strNotification.c_str());
        }
    }
    ic_core::CIgniteClient::GetOnOffMonitor()->ReadyForShutdown(
        ic_core::IOnOff::eR_NOTIFICATION_LISTENER);
    ic_core::CIgniteClient::GetOnOffMonitor()->UnregisterForShutdownNotification(
        ic_core::IOnOff::eR_NOTIFICATION_LISTENER);
    Detach();
}

void CNotificationListener::NotifyShutdown() {
    HCPLOG_METHOD();
    m_bIsShutDownInitiated = true;
    pthread_mutex_lock( &g_Mutex );
    pthread_cond_signal( &g_condNotificationAvailable );
    pthread_mutex_unlock( &g_Mutex );
}

} //namespace ic_bl
