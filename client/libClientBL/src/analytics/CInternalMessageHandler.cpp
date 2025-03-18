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

#include "CInternalMessageHandler.h"
#include "CClientConn.h"
#include "CIgniteLog.h"
#include "analytics/CDeviceActivationHandler.h"
#include "analytics/CDisassociationRequestHandler.h"
#include "analytics/CVinHandler.h"
#include "auth/CActivationBackoff.h"

//! Macro for CInternalMessageHandler string
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CInternalMessageHandler"

namespace ic_bl
{
namespace 
{
    //! Constant key for 'InternalMessageHandler' string
    static const std::string DOMAIN_INTERNAL_MSG_HANDLER = 
                                                       "InternalMessageHandler";
}

CInternalMessageHandler::CInternalMessageHandler() : 
                                                  ic_core::CBaseMessageHandler()
{
    ic_core::CIgniteConfig::GetInstance()->
        SubscribeForConfigUpdateNotification(DOMAIN_INTERNAL_MSG_HANDLER, this);
}

CInternalMessageHandler::~CInternalMessageHandler()
{
    ic_core::CIgniteConfig::GetInstance()->
            UnSubscribeForConfigUpdateNotification(DOMAIN_INTERNAL_MSG_HANDLER);
    DeInit();
    HCPLOG_METHOD() << "Object destroyed...";
}

void CInternalMessageHandler::Init()
{
    HCPLOG_METHOD();

    //for events
    m_jsonDomainEventMap = ic_core::CIgniteConfig::GetInstance()->
                                            GetJsonValue(MQTT_DOMAIN_EVENT_MAP);
    /* create original copy of m_jsonDomainEventMap which is used to check about
     * configuration update
     */
    m_jsonConfigDomainEventMap = m_jsonDomainEventMap;
    AddDomainToMap("IgnStatus", ic_core::DOMAIN_ACTIVATION_BACKOFF);
    AddDomainToMap("DeviceRemoval", ic_core::DOMAIN_ACTIVATION_BACKOFF);

    //execute base init operations
    ic_core::CBaseMessageHandler::Init();
}

void CInternalMessageHandler::DeInit()
{
    for (std::map<std::string, ic_core::CEventProcessor*>::iterator mapEPItr = 
                             m_mapEventProcessors.begin(); 
                             mapEPItr != m_mapEventProcessors.end(); mapEPItr++)
    {
        HCPLOG_I << "Deleting the instance..." << mapEPItr->first;
        if (ic_core::DOMAIN_ACTIVATION_BACKOFF == mapEPItr->first)
        {
            CActivationBackoff::ReleaseInstance();
        }
        else if (ic_core::DOMAIN_ACTIVATION_HANDLER == mapEPItr->first)
        {
            CDeviceActivationHandler::ReleaseInstance();
        }
        else if (ic_core::DOMAIN_VIN_HANDLER == mapEPItr->first)
        {
            CVinHandler::ReleaseInstance();
        }
        else 
        {
            HCPLOG_D << "No internal event processor for domain ~ " 
                     << mapEPItr->first ;
        }
    }

    for (std::map<std::string, ic_core::CEventProcessor*>::iterator mapNPItr = 
                             m_mapNotifProcessors.begin(); 
                             mapNPItr != m_mapNotifProcessors.end(); mapNPItr++)
    {
        HCPLOG_I << "Deleting the instance..." << mapNPItr->first;
        if (ic_core::DOMAIN_DISASSOCIATION_REQUEST == mapNPItr->first)
        {
            CDisassociationRequestHandler::ReleaseInstance();
        }
        else
        {
            HCPLOG_D << "No internal notif processor for domain ~ " 
                     << mapNPItr->first ;
        }
    }

    //execute base init operations
    ic_core::CBaseMessageHandler::DeInit();
}

void CInternalMessageHandler::NotifyMessage(MsgPayload stMsgPayload)
{
    /* if any CInternalMessageHandler specific tasks, they can go here
     * execute base notifyMessage
     */
    ic_core::CBaseMessageHandler::NotifyMessage(stMsgPayload);
}

void CInternalMessageHandler::SetClientConnector(IExtendedClientConnector 
                                                *pClientCnctr)
{
    HCPLOG_METHOD();
    ic_core::CClientConn::GetInstance()->SetClientConnector(pClientCnctr);
}

void CInternalMessageHandler::UpdateEventProcessorMapForEventHandler(
                                                  const std::string &rstrDomain) 
{
    HCPLOG_D << "Updating eventProcessor map for ~ " << rstrDomain;

    if (ic_core::DOMAIN_ACTIVATION_BACKOFF == rstrDomain)
    {
        m_mapEventProcessors[ic_core::DOMAIN_ACTIVATION_BACKOFF] = 
                                               CActivationBackoff::GetInstance();
    }
    else if (ic_core::DOMAIN_ACTIVATION_HANDLER == rstrDomain)
    {
        m_mapEventProcessors[ic_core::DOMAIN_ACTIVATION_HANDLER] = 
                                        CDeviceActivationHandler::GetInstance();
    }
    else if (ic_core::DOMAIN_VIN_HANDLER == rstrDomain)
    {
        m_mapEventProcessors[ic_core::DOMAIN_VIN_HANDLER] = 
                                                      CVinHandler::GetInstance();
    }
    else 
    {
        HCPLOG_D << "No internal event processor for domain ~ " << rstrDomain;
    }
}

void CInternalMessageHandler::UpdateEventProcessorMapForNotificationHandler(
                                                  const std::string &rstrDomain)
{
    HCPLOG_D << "Updating notifProcessor map for ~ " << rstrDomain;

    if (ic_core::DOMAIN_DISASSOCIATION_REQUEST == rstrDomain){
        m_mapNotifProcessors[ic_core::DOMAIN_DISASSOCIATION_REQUEST] = 
                                   CDisassociationRequestHandler::GetInstance();
    }
    else 
    {
        HCPLOG_D << "No internal notif processor for domain ~ " << rstrDomain;
    }
}

void CInternalMessageHandler::NotifyConfigUpdate()
{
    ic_utils::Json::Value jsonNewDomainEventMap = 
     ic_core::CIgniteConfig::GetInstance()->GetJsonValue(MQTT_DOMAIN_EVENT_MAP);

    if(m_jsonConfigDomainEventMap != jsonNewDomainEventMap)
    {
        ic_core::CBaseMessageHandler::UpdateCurrentEventDomainMap(
                                                         jsonNewDomainEventMap);
    } 
    else 
    {
        //no change in domainEventMap
        HCPLOG_D << "NoUpdate for domainEventMap";
    }
}

std::set<std::string> CInternalMessageHandler::GetSupplimentaryEventsList()
{
    std::set<std::string> strEventList;

    //if any supplimentary events to be added to the list, add them here

    return strEventList;
}
} /* namespace ic_bl */
