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

#include "analytics/CBaseMessageHandler.h"
#include "CIgniteLog.h"
#include "CIgniteConfig.h"
#include "db/CLocalConfig.h"

//! Macro for CBaseMessageHandler string
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CBaseMessageHandler"

namespace ic_core 
{
CBaseMessageHandler::CBaseMessageHandler()
{
    m_jsonEventDomainMap = ic_utils::Json::nullValue;
    m_jsonDomainEventMap = ic_utils::Json::nullValue;
    m_jsonConfigDomainEventMap = ic_utils::Json::nullValue;
}

CBaseMessageHandler::~CBaseMessageHandler()
{

}

void CBaseMessageHandler::Init()
{
    HCPLOG_METHOD();

    CreateReverseOfDomainEventMap();
    UpdateEventProcessorMap();

    /* The map m_jsonEventDomainMap is populated initially based on the 
     *  'domainEventMap' section of the configuration file. Once populated, this
     *  map can contain many mappings that are not only handled by 
     *  InternalMessageHandler but also by AnalyticsMessageHandler, so update 
     *  this m_jsonEventDomainMap map after the EventProcessors are populated. 
     *  This m_jsonEventDomainMap map will be also used to determine if any 
     *  processor is there for given event hence this map should not have any 
     *  mapping where there is no associated event processor. 
     */
    RefreshEventDomainMap();

    /* Based on mqtt service topics, update the processor list to handle 
     *  notifications 
     */
    ic_utils::Json::Value topicArray = 
        CIgniteConfig::GetInstance()->GetJsonValue(MQTT_SERVICE_TOPIC);

    if (topicArray.isNull()) 
    {
        HCPLOG_I << "No service topics!";
    }

    if (!topicArray.isArray()) 
    {
        HCPLOG_E << "Invalid services topic";
    }

    for (int nItr = 0 ; nItr < topicArray.size() ; nItr++)
    {
        ic_utils::Json::Value jsonServiceObj = topicArray[nItr];
        std::string strServiceName = jsonServiceObj[SERVICE_NAME].asString();
        UpdateEventProcessorMapForNotificationHandler(strServiceName);
    }

    /* Based on domain-event map, update the processor list to handle 
     *  notification 
     */ 
    for (ic_utils::Json::ValueIterator jsonIter = m_jsonDomainEventMap.begin(); 
         jsonIter != m_jsonDomainEventMap.end(); jsonIter++) 
    {
        std::string strDomain = jsonIter.key().asString();
        UpdateEventProcessorMapForNotificationHandler(strDomain);
    }

    // Print event processor list - for debugging purpose
    for (std::map<std::string, CEventProcessor*>::iterator iterEvent = 
        m_mapEventProcessors.begin(); iterEvent != m_mapEventProcessors.end(); 
        iterEvent++)
    {
        HCPLOG_I << "Active Event Processor - Domain ~" << iterEvent->first;
    }

    // Print notif processor list - for debugging purpose
    for (std::map<std::string, CEventProcessor*>::iterator iterNoti = 
        m_mapNotifProcessors.begin(); iterNoti != m_mapNotifProcessors.end(); 
        iterNoti++)
    {
        HCPLOG_I << "Active Notif Processor - Domain ~" << iterNoti->first;
    }
}

void CBaseMessageHandler::DeInit()
{
    // Do any deinitialization here
}

void CBaseMessageHandler::NotifyMessage(MsgPayload stMsgPayload)
{
    HCPLOG_METHOD();

    if (stMsgPayload.eType == IMessageHandler::eMSG_TYPE_EVENT)
    {
        ProcessEventTypeMessage(stMsgPayload);
    }
    else if (stMsgPayload.eType == IMessageHandler::eMSG_TYPE_NOTIFICATION)
    {
        ProcessNotificationTypeMessage(stMsgPayload);
    }
    else
    {
        HCPLOG_E << "Invalid type " << stMsgPayload.eType;
    }
}

void CBaseMessageHandler::ProcessEventTypeMessage(const MsgPayload 
                                                  &rstMsgPayload)
{
    CEventWrapper event;
    event.JsonToEvent(rstMsgPayload.strPayloadJson);
    std::string strEventID = event.GetEventId();

    HCPLOG_I << "Processing Event ~ " << rstMsgPayload.strPayloadJson;

    // Send the event to respective processor
    if (m_jsonEventDomainMap[strEventID].isArray())
    {
        for (int nItr = 0; 
             nItr < m_jsonEventDomainMap[strEventID].size(); 
             nItr++)
        {
            DispatchEvent(strEventID, 
                          m_jsonEventDomainMap[strEventID][nItr].asString(),
                          rstMsgPayload.strPayloadJson);
        }
    }
    else
    {
        DispatchEvent(strEventID, m_jsonEventDomainMap[strEventID].asString(),
                      rstMsgPayload.strPayloadJson);
    }
}

bool CBaseMessageHandler::ProcessNotificationTypeMessage(const MsgPayload 
                                                         &rstMsgPayload)
{
    bool bIsNotifSent = false;
    std::string strMsg = rstMsgPayload.strPayloadJson;
    ic_utils::Json::Value jsonPayload;
    ic_utils::Json::Reader jsonReader;
    if (jsonReader.parse(strMsg, jsonPayload))
    {
        if ((jsonPayload.isMember("domain") && jsonPayload["domain"].isString())
          && (jsonPayload.isMember("notif") && jsonPayload["notif"].isObject()))
        {
            std::string strDomain = jsonPayload["domain"].asString();
            ic_utils::Json::Value jsonNotif = jsonPayload["notif"];

            HCPLOG_I << "Processing Notif ~~ [domain]~" << strDomain 
                     << "; [payload]~" << jsonNotif.toStyledString();

            std::map<std::string, CEventProcessor *>::iterator iter = 
                                           m_mapNotifProcessors.find(strDomain);

            if (m_mapNotifProcessors.end() != iter) 
            {
                HCPLOG_I << "Sending Notif [domain~" << 
                          strDomain << "] to the handler...";
                iter->second->ApplyConfig(jsonNotif);
                bIsNotifSent = true;
            }
            else
            {
                HCPLOG_I << "invalid domain: " << strDomain;
            }
        }
        else
        {
            HCPLOG_E << "Domain not found";
        }
    }
    else
    {
        HCPLOG_E << "Invalid payload";
    }
    return bIsNotifSent;
}

void CBaseMessageHandler::DispatchEvent(const std::string &rstrEventID, 
                                        const std::string &rstrDomain, 
                                        const std::string &rstrSerEvent)
{
    CEventWrapper event;
    event.JsonToEvent(rstrSerEvent);

    std::map<std::string, CEventProcessor*>::iterator iter = 
                                          m_mapEventProcessors.find(rstrDomain);
    if (m_mapEventProcessors.end() != iter && NULL != iter->second) 
    {
        HCPLOG_I << "Sending event " << rstrEventID << 
                   "[Domain:" << rstrDomain << "] to handler...";
        iter->second->ProcessEvent(event);
    } 
    else 
    {
        HCPLOG_D << "No processor found for " << rstrDomain;
    }
}

void CBaseMessageHandler::SetClientConnector(IClientConnector *pCCnctr)
{
    HCPLOG_METHOD();
    // Let the overriding handlers extend the definition of this method
}

void CBaseMessageHandler::AddDomainToMap(const std::string &rstrEventID,
                                         const std::string &rstrDomain)
{
    HCPLOG_METHOD() << "EventID ~ " << rstrEventID 
                    << "; Domain ~ " << rstrDomain;
    if (!m_jsonDomainEventMap.isMember(rstrDomain))
    {
        m_jsonDomainEventMap[rstrDomain] = rstrEventID;
    }
    else
    {
        if (m_jsonDomainEventMap[rstrDomain].isArray())
        {
            m_jsonDomainEventMap[rstrDomain].append(rstrEventID);
        }
        else
        {
            ic_utils::Json::Value jsonDomainArray(ic_utils::Json::arrayValue);
            jsonDomainArray.append(m_jsonDomainEventMap[rstrDomain].asString());
            jsonDomainArray.append(rstrEventID);
            m_jsonDomainEventMap[rstrDomain] = jsonDomainArray;
        }
    }
}

void CBaseMessageHandler::CreateReverseOfDomainEventMap()
{
    for (ic_utils::Json::ValueIterator iter = m_jsonDomainEventMap.begin();
        iter != m_jsonDomainEventMap.end(); iter++)
    {
        std::string sreDomain = iter.key().asString();
        ic_utils::Json::Value jsonVal = iter.operator*();
        std::string strEventId;
        if (!jsonVal.empty())
        {
            if (jsonVal.isString())
            {
                strEventId = (iter.operator*()).asString();
                UpdateEventDomainMap(strEventId, sreDomain);

            }
            else if (jsonVal.isArray())
            {
                for(int i = 0; i < jsonVal.size(); i++)
                {
                    strEventId = jsonVal[i].asString();
                    UpdateEventDomainMap(strEventId, sreDomain);
                }
            }
        }
    }
}

void CBaseMessageHandler::UpdateEventDomainMap(const std::string &rstrEventID,
                                               const std::string &rstrDomain)
{
    HCPLOG_METHOD() << "EventID~" << rstrEventID << "; Domain~" << rstrDomain;

    if (!(m_jsonEventDomainMap.isMember(rstrEventID)))
    {
        m_jsonEventDomainMap[rstrEventID] = rstrDomain;
    }
    else
    {
        if (m_jsonEventDomainMap[rstrEventID].isArray())
        {
            m_jsonEventDomainMap[rstrEventID].append(rstrDomain);
        }
        else
        {
            ic_utils::Json::Value jsonEventArray(ic_utils::Json::arrayValue);
            jsonEventArray = ic_utils::Json::nullValue;
            jsonEventArray.append(m_jsonEventDomainMap[rstrEventID].asString());
            jsonEventArray.append(rstrDomain);
            m_jsonEventDomainMap[rstrEventID] = jsonEventArray;
        }
    }
}

void CBaseMessageHandler::UpdateEventProcessorMap()
{
    HCPLOG_METHOD();
    if (!m_jsonDomainEventMap.empty())
    {
        for (ic_utils::Json::ValueIterator iter = m_jsonDomainEventMap.begin();
            iter != m_jsonDomainEventMap.end(); iter++) 
        {
            std::string strKey = iter.key().asString();
            HCPLOG_D << "Key is " << strKey;
            UpdateEventProcessorMapForEventHandler(strKey);
            UpdateEventProcessorMapForNotificationHandler(strKey);
        }
    }
}

void CBaseMessageHandler::UpdateEventProcessorMapForEventHandler(
                                                  const std::string &rstrDomain)
{
    HCPLOG_D << "Domain ~ " << rstrDomain;
    // Let the overriding handlers extend the definition of this method
}

void CBaseMessageHandler::UpdateEventProcessorMapForNotificationHandler(
                                                  const std::string &rstrDomain)
{
    HCPLOG_D << "Domain ~ "<< rstrDomain;
    // Let the overriding handlers extend the definition of this method
}

bool CBaseMessageHandler::IsHandlerSubscribedForEvent(const std::string 
                                                      &rstrEventID)
{
    if (m_jsonEventDomainMap.isMember(rstrEventID))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool CBaseMessageHandler::IsHandlerSubscribedForNotification(const std::string 
                                                             &rstrDomain)
{
    std::map<std::string, CEventProcessor *>::iterator iter = 
                                          m_mapNotifProcessors.find(rstrDomain);
    if (m_mapNotifProcessors.end() != iter) 
    {
        return true;
    }
    else 
    {
        return false;
    }
}

std::set<std::string> CBaseMessageHandler::GetSupplimentaryEventsList()
{
    std::set<std::string> setEventList;

    //if any supplimentary events to be added to the list, add them here

    return setEventList;
}

void CBaseMessageHandler::RefreshEventDomainMap()
{
    ic_utils::Json::Value jsonTmpEventDomainMap = m_jsonEventDomainMap;
    std::vector<std::string> vEventNames = m_jsonEventDomainMap.getMemberNames();

    for (int nItr = 0; nItr < vEventNames.size(); nItr++)
    {
        std::string strEvName = vEventNames.at(nItr);
        HCPLOG_T << "EventID: " << strEvName;
        if (m_jsonEventDomainMap[strEvName].isArray())
        {
            RefreshEventDomainListBasedOnEventProcessorsMap(strEvName, 
                                                         jsonTmpEventDomainMap);
        }
        else
        {
            std::string strDomain = m_jsonEventDomainMap[strEvName].asString();
            HCPLOG_T << "->Domain ~ " << strDomain;

            std::map<std::string, CEventProcessor *>::iterator iter = 
                                           m_mapEventProcessors.find(strDomain);
            if (m_mapEventProcessors.end() == iter) 
            {
                jsonTmpEventDomainMap.removeMember(strEvName);
                HCPLOG_T << "--->Removed " << strDomain 
                         << " as no processor found!";
            }
        }
    }
    m_jsonEventDomainMap = jsonTmpEventDomainMap;

    PrintEventDomainMapLogs();
}

void CBaseMessageHandler::RefreshEventDomainListBasedOnEventProcessorsMap(
                                                   const std::string &rstrEName,
                                          ic_utils::Json::Value &rjsonEventDomainMap)
{
    ic_utils::Json::Value jsonTmpEveArray;
    int nTmpEArrayInd = 0;
    for (int j = 0; j < m_jsonEventDomainMap[rstrEName].size() ; j++)
    {
        std::string strDomain = 
                            m_jsonEventDomainMap[rstrEName][j].asString();
        HCPLOG_T << "->Domain ~ " << strDomain;
        std::map<std::string, CEventProcessor *>::iterator iter = 
                                    m_mapEventProcessors.find(strDomain);
        if (m_mapEventProcessors.end() != iter)
        {
            jsonTmpEveArray[nTmpEArrayInd++] = strDomain;
        }
        else
        {
            HCPLOG_T << "--->Removed " << strDomain 
                        << " as no processor found!";
        }
        rjsonEventDomainMap[rstrEName] = jsonTmpEveArray;
    }

    if (rjsonEventDomainMap[rstrEName].size() == 0) 
    {
        HCPLOG_T << "***All domains are removed hence removing \'" 
                    << rstrEName << "\' from map";
        rjsonEventDomainMap.removeMember(rstrEName);
    }
}

void CBaseMessageHandler::PrintEventDomainMapLogs()
{
    // BEGIN: debug code starts
    HCPLOG_T << ">>>>Updated EventDomainMap size ~ "
             << m_jsonEventDomainMap.size();

    std::vector<std::string> vEventNames = m_jsonEventDomainMap.getMemberNames();

    for (int i = 0; i < vEventNames.size(); i++) 
    {
        std::string strEvName = vEventNames.at(i);
        HCPLOG_T << "EventID: " << strEvName;
        if (m_jsonEventDomainMap[strEvName].isArray())
        {
            ic_utils::Json::Value jsonTempMap = m_jsonEventDomainMap[strEvName];
            for (int j = 0; j < jsonTempMap.size() ; j++)
            {
                std::string strDomain = jsonTempMap[j].asString();
                HCPLOG_T << "->Domain ~ " << strDomain;
            }
        }
        else 
        {
            std::string strDomain = m_jsonEventDomainMap[strEvName].asString();
            HCPLOG_T << "->Domain ~ " << strDomain;
        }
    }
    // END: debug code ends
}

void CBaseMessageHandler::UpdateCurrentEventDomainMap(
                                       ic_utils::Json::Value &rjsonNewDomainEventMap)
{
    // Create local copy of m_jsonEventDomainMap
    ic_utils::Json::Value jsonUpdatedEventDomainMap = m_jsonEventDomainMap;
    
    /* From the domain->event map (of original configuration), find out which 
     *  domain section is changed 
     */
    for (ic_utils::Json::ValueIterator iter = m_jsonConfigDomainEventMap.begin(); 
         iter != m_jsonConfigDomainEventMap.end(); iter++)
    {
        std::string strDomain = iter.key().asString();
        /* If current domain is not available in the new map, ignore.
         *  Adding/removing domain dynamically is not supported yet. 
         */
        if (!rjsonNewDomainEventMap.isMember(strDomain)) 
        {
            HCPLOG_I << "Domain " << strDomain << " not found in new mapping!";
            continue;
        }
        // Is the domain entries are changed?
        ic_utils::Json::Value jsonCurrDomainSec = 
                                          m_jsonConfigDomainEventMap[strDomain];
        ic_utils::Json::Value jsonNewDomainSec = rjsonNewDomainEventMap[strDomain];
        
        if (jsonCurrDomainSec == jsonNewDomainSec) 
        {
            continue;
        } 
        else 
        {
            HCPLOG_C << "change- " << strDomain << " Curr:" << jsonCurrDomainSec
                     << " New:" << jsonNewDomainSec;
        }

        // Check if jsonCurrDomainSec is a string or array
        if (jsonCurrDomainSec.isArray()) 
        {
            UpdateEventDomainMapForArrayType(jsonUpdatedEventDomainMap,
                                             strDomain, jsonCurrDomainSec,
                                             jsonNewDomainSec);
        }
        else 
        {
            UpdateEventDomainMapForStringType(jsonUpdatedEventDomainMap, 
                                              strDomain, jsonCurrDomainSec, 
                                              jsonNewDomainSec);
        }        
    }
    // Update the config variable
    m_jsonConfigDomainEventMap = rjsonNewDomainEventMap;

    // Update Event domain map with new config
    m_jsonEventDomainMap = jsonUpdatedEventDomainMap;
}

void CBaseMessageHandler::UpdateEventDomainMapForArrayType(
                                   ic_utils::Json::Value &rjsonUpdatedEventDomainMap,
                                   const std::string &rstrDomain,
                                   const ic_utils::Json::Value &rjsonCurrDomainSec,
                                   const ic_utils::Json::Value &rjsonNewDomainSec)
{
    if (rjsonNewDomainSec.isArray()) 
    {
        /* 
         * Current domain section is an array & new domain section also an 
         * array remove current eventnames from eventDomain map
         */
        for (int i = 0; i < rjsonCurrDomainSec.size(); i++) 
        {
            std::string strCurrEventName = rjsonCurrDomainSec[i].asString();
            RemoveEventDomainReferenceFromMap(rjsonUpdatedEventDomainMap, 
                                                rstrDomain, strCurrEventName);
        }
        // Now, add new events to the event->domain map for the new events
        for (int i = 0; i < rjsonNewDomainSec.size() ; i++) 
        {
            std::string strNewEID = rjsonNewDomainSec[i].asString();
            AddEventDomainReferenceToMap(rjsonUpdatedEventDomainMap,
                                            rstrDomain, strNewEID);
        }
    }
    else 
    {
        /* 
         * Current domain section is an array & new domain section is a 
         * string remove current eventnames from eventDomain map 
         */
        for (int i = 0; i < rjsonCurrDomainSec.size() ; i++) 
        {
            std::string strCurrEventName = rjsonCurrDomainSec[i].asString();
            RemoveEventDomainReferenceFromMap(rjsonUpdatedEventDomainMap, 
                                                rstrDomain, strCurrEventName);
        }
        // Now add the new eventName to the eventDomain map
        std::string strNewEID = rjsonNewDomainSec.asString();
        AddEventDomainReferenceToMap(rjsonUpdatedEventDomainMap, rstrDomain, 
                                     strNewEID);
    }
}

void CBaseMessageHandler::UpdateEventDomainMapForStringType(
                                   ic_utils::Json::Value &rjsonUpdatedEventDomainMap,
                                   const std::string &rstrDomain,
                                   const ic_utils::Json::Value &rjsonCurrDomainSec,
                                   const ic_utils::Json::Value &rjsonNewDomainSec)
{
    if (rjsonNewDomainSec.isArray()) 
    {
        /* 
         * Current domain section is a string & new domain section is an 
         * array 
         */
        std::string strCurrEventName = rjsonCurrDomainSec.asString();
        // Remove current eventname from eventDomain map
        RemoveEventDomainReferenceFromMap(rjsonUpdatedEventDomainMap, 
                                            rstrDomain, strCurrEventName);
        // Now, add new events to the event->domain map for the new events
        for (int i = 0; i < rjsonNewDomainSec.size() ; i++) 
        {
            std::string strNewEID = rjsonNewDomainSec[i].asString();
            AddEventDomainReferenceToMap(rjsonUpdatedEventDomainMap,
                                            rstrDomain, strNewEID);
        }
    }
    else 
    {
        /* 
         * Current domain section is a string & new domain section also a 
         * string 
         */
        std::string strCurrEventName = rjsonCurrDomainSec.asString();
        // Remove current eventname from eventDomain map
        RemoveEventDomainReferenceFromMap(rjsonUpdatedEventDomainMap, 
                                            rstrDomain, strCurrEventName);

        // Add the new event name to eventDomain map
        std::string strNewEID = rjsonNewDomainSec.asString();
        AddEventDomainReferenceToMap(rjsonUpdatedEventDomainMap,
                                        rstrDomain, strNewEID);
    }
}

void CBaseMessageHandler::RemoveEventDomainReferenceFromMap(
                                          ic_utils::Json::Value &rjsonEventDomainMap,
                                           const std::string &rstrDomain,
                                           const std::string &rstrEventName)
{
    if (rjsonEventDomainMap.isMember(rstrEventName) &&
        rjsonEventDomainMap[rstrEventName].isString() &&
        (rjsonEventDomainMap[rstrEventName].asString() == rstrDomain)) 
    {
        // Remove domain from current string
        rjsonEventDomainMap.removeMember(rstrEventName);
    }
    else 
    {
        // Remove domain from current array
        if (rjsonEventDomainMap.isMember(rstrEventName) &&
            rjsonEventDomainMap[rstrEventName].isArray()) 
        {
            ic_utils::Json::Value jsonEventList = rjsonEventDomainMap[rstrEventName];

            ic_utils::Json::Value jsonNewList;
            for (int i=0; i<jsonEventList.size(); i++) 
            {
                std::string strOldDomain = jsonEventList[i].asString();
                if (strOldDomain != rstrDomain) 
                {
                    jsonNewList.append(strOldDomain);
                } 
                else 
                {
                    // Ignore (remove)for same domain
                }
            }
            rjsonEventDomainMap[rstrEventName] = jsonNewList;
        }
    }
}

void CBaseMessageHandler::AddEventDomainReferenceToMap(
                                          ic_utils::Json::Value &rjsonEventDomainMap,
                                          const std::string &rstrDomain,
                                          const std::string &rstrEventName)
{
    if (!(rjsonEventDomainMap.isMember(rstrEventName))) 
    {
        rjsonEventDomainMap[rstrEventName] = rstrDomain;
    }
    else
    {
        if(rjsonEventDomainMap[rstrEventName].isArray()) 
        {
            rjsonEventDomainMap[rstrEventName].append(rstrDomain);
        }
        else 
        {
            ic_utils::Json::Value jsonEventArray(ic_utils::Json::arrayValue);
            jsonEventArray = ic_utils::Json::nullValue;
            jsonEventArray.append(rjsonEventDomainMap[rstrEventName].asString());
            jsonEventArray.append(rstrDomain);
            rjsonEventDomainMap[rstrEventName] = jsonEventArray;
        }
    }
}
} /* namespace ic_core */
