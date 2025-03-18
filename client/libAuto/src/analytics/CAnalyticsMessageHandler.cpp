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

#include "CAnalyticsMessageHandler.h"
#include "CClientConn.h"
#include "CIgniteLog.h"
#include "db/CLocalConfig.h"
#include "CRemoteService.h"

using namespace std;

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CAnalyticsMessageHandler"

namespace ic_auto
{

namespace
{
static const string DOMAIN_ANALYTICS_MSG_HANDLER = "CAnalyticsMessageHandler";
}

CAnalyticsMessageHandler::CAnalyticsMessageHandler() : ic_core::CBaseMessageHandler()
{
    ic_core::CIgniteConfig::GetInstance()->SubscribeForConfigUpdateNotification
        (DOMAIN_ANALYTICS_MSG_HANDLER, this);
}

CAnalyticsMessageHandler::~CAnalyticsMessageHandler()
{
    HCPLOG_METHOD();
    ic_core::CIgniteConfig::GetInstance()->UnSubscribeForConfigUpdateNotification
                                                 (DOMAIN_ANALYTICS_MSG_HANDLER);
    DeInit();
    HCPLOG_METHOD() << "Object destroyed...";
}

void CAnalyticsMessageHandler::Init()
{
    HCPLOG_METHOD();

    //Reading Domain Event Map from Config file
    m_jsonDomainEventMap = 
     ic_core::CIgniteConfig::GetInstance()->GetJsonValue(MQTT_DOMAIN_EVENT_MAP);

    /* 
     * Saving original m_jsonDomainEventMap to identify change in 
     * configuration later
     */ 
    m_jsonConfigDomainEventMap = m_jsonDomainEventMap;

    //execute base init operations
    ic_core::CBaseMessageHandler::Init();
}

void CAnalyticsMessageHandler::DeInit()
{
    for (map<string, ic_core::CEventProcessor*>::iterator iterEventIt =
                m_mapEventProcessors.begin(); 
                iterEventIt != m_mapEventProcessors.end(); iterEventIt++)
    {
        //currently no known event-processor is associated
        HCPLOG_W << "Unknown event-processor detected. " << iterEventIt->first;
    }

    for (map<string, ic_core::CEventProcessor*>::iterator iterNotifIt =
                m_mapNotifProcessors.begin(); 
                iterNotifIt != m_mapNotifProcessors.end(); iterNotifIt++)
    {
        HCPLOG_I << "Deleting the instance..." << iterNotifIt->first;
        if (iterNotifIt->first == ic_core::DOMAIN_RO_SERVICE)
        {
            CRemoteService::ReleaseInstance();
        }
        else
        {
            HCPLOG_W << "Unknown notif-processor detected. " << 
                        iterNotifIt->first;
        }
    }

    //execute base init operations
    ic_core::CBaseMessageHandler::DeInit();
}

void CAnalyticsMessageHandler::NotifyMessage(MsgPayload msgPayload)
{
    if (msgPayload.eType == ic_core::IMessageHandler::eMSG_TYPE_EVENT)
    {
        ic_core::CEventWrapper event;
        event.JsonToEvent(msgPayload.strPayloadJson);
        std::string strEventID = event.GetEventId();

        //proceed only if it is required event
        if (!m_jsonEventDomainMap.isMember(strEventID))
        {
            return;
        }

        if (strEventID.compare("Location") == 0)
        {
            //store in localConfig
            ic_core::CLocalConfig::GetInstance()->Set("lastLocation",
                                                    msgPayload.strPayloadJson);
        }
        // store Odometer event
        if (strEventID.compare("Odometer") == 0)
        {
            //store in localConfig
            ic_core::CLocalConfig::GetInstance()->Set("lastOdometer",
                                                    msgPayload.strPayloadJson);
        }
    }

    //execute base notifyMessage
    ic_core::CBaseMessageHandler::NotifyMessage(msgPayload);
}

void CAnalyticsMessageHandler::SetClientConnector
                                        (ic_bl::IExtendedClientConnector *cCnctr)
{
    ic_core::CClientConn::GetInstance()->SetClientConnector(cCnctr);
}

void CAnalyticsMessageHandler::UpdateEventProcessorMapForEventHandler
                                                (const string &rstrDomain)
{
    //currently no event-processor is to be associated for event handling
    HCPLOG_W << "Non-supported domain detected. " << rstrDomain;
}

void CAnalyticsMessageHandler::UpdateEventProcessorMapForNotificationHandler
                                                (const string &rstrDomain)
{
    HCPLOG_D <<"Updating notifProcessor map for " << rstrDomain;

    if (rstrDomain == ic_core::DOMAIN_RO_SERVICE)
    {
        m_mapNotifProcessors[ic_core::DOMAIN_RO_SERVICE] = 
                                        CRemoteService::GetInstance();
    }
    else
    {
        HCPLOG_W << "Non-supported domain detected. " << rstrDomain;
    }
}

void CAnalyticsMessageHandler::NotifyConfigUpdate()
{
    ic_utils::Json::Value jsonNewDomainEventMap = 
        ic_core::CIgniteConfig::GetInstance()->GetJsonValue(MQTT_DOMAIN_EVENT_MAP);
    if(m_jsonConfigDomainEventMap != jsonNewDomainEventMap)
    {
        ic_core::CBaseMessageHandler::UpdateCurrentEventDomainMap(jsonNewDomainEventMap);
    }
    else
    {
        //no change in domainEventMap
        HCPLOG_D << "NoUpdate for domainEventMap";
    }
}

} /* namespace ic_auto */
