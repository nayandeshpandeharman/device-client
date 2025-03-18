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

#include <errno.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include "CIgniteConfig.h"
#include "dam/CCacheTransport.h"
#include "CIgniteLog.h"
#include "CIgniteDateTime.h"
#include "CIgniteMutex.h"
#include "ClientBLLibVersion.h"
#include "config/CUploadMode.h"
#include "IOnOff.h"
#include "CIgniteClient.h"
#include "CDBTransportWrapper.h"
#include "upload/CMQTTUploader.h"
#include "dam/CDBTransportWrapper.h"

//! Macro for CCacheTransport string
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CCacheTransport"

//! Macro for value 0
#define SUCCESS 0

//! Macro for value 100
#define FAILURE 100

using ic_event::CIgniteEvent;
using std::string;

namespace ic_bl
{

//! Constant key for 'DAM.EventReceiverThreadPriority' string
static const std::string EVENT_RECEIVER_PRIORITY = 
                                              "DAM.EventReceiverThreadPriority";

//! Constant key for integer 5
static const int DEF_EVENT_RECEIVER_PRIORITY = 5; // Background priority

//! Constant queue size increased to 2Mb for high frequency event data
static const int MAX_QUEUE_SIZE = 2000000;

//! Constant key for 'DAM.IgnitewhiteListedEvents' string
static const std::string KEY_IGNITE_WHITE_LIST_EVENTS = 
                                                  "DAM.IgnitewhiteListedEvents";

//! Constant key for 'MQTT.domainEventMap' string
static const std::string KEY_DOMAIN_EVENT_MAP = "MQTT.domainEventMap";

//! Constant key for 'IgnStatus' string
static const std::string KEY_IGNITIONSTATUS = "IgnStatus";

//! Constant key for 'state' string
static const std::string KEY_IGN_STATE = "state";

//! Constant key for 'off' string
static const std::string KEY_IGN_OFF = "off";

//! Constant key for 'FileLogger.inflowEventLogging.criticalEventLoggingCount'
static const std::string KEY_REMOTE_CRITICAL_EVENT_LOGGING_COUNT = 
                      "FileLogger.inflowEventLogging.criticalEventLoggingCount";

//! Constant key for 'FileLogger.inflowEventLogging.defaultLoggingCount' string
static const std::string KEY_REMOTE_DEFAULT_EVENT_LOGGING_COUNT = 
                            "FileLogger.inflowEventLogging.defaultLoggingCount";

//! Constant key for 'FileLogger.inflowEventLogging' string
static const std::string KEY_INFLOW_EVENT_LOGGING = 
                                                "FileLogger.inflowEventLogging";

//! Constant key for 'criticalEventLoggingCount' string
static const std::string KEY_CRITICAL_EVENT_LOGGING_COUNT = 
                                                    "criticalEventLoggingCount";

//! Constant key for 'defaultLoggingCount' string
static const std::string KEY_DEFAULT_LOGGING_COUNT = "defaultLoggingCount";

//! Constant key for 'MQTT.DirectAlerts' string
static const std::string KEY_DIRECT_ALERTS = "MQTT.DirectAlerts";

//! Constant key for 'FileLogger.logCounterReset' string
static const std::string KEY_LOG_COUNTER_RESET = "FileLogger.logCounterReset";

//! Constant default value of inflow event log count
static const int DEF_INFLOW_EVENT_LOG_CNT = -1;

//! Constant default value of max inflow event log count 
static const int DEF_MAX_INFLOW_EVENT_LOG_CNT = 100;

//! Initial value of logging Non Ignite event count
unsigned long int g_ulNieCnt=0;

//! Initial value of logging Non Ignite event iteration count 
unsigned long int g_ulNieCntIter=0;

//! Initial value of logging Ignite event count
unsigned long int g_ulIgCnt=0;

//! Initial value of logging Ignite event iteration count
unsigned long int g_ulIgCntIter=0;

//! Initial value of Total Overflow Event count
unsigned long long g_ulTotOECnt = 0;

//! Initial value of count of events in into queue 
unsigned long long g_ulInCnt = 0;

//! Initial value of In Event iteration count
unsigned long long g_ulInCntIter = 0;

//! Default value of Event out from queue
unsigned long long g_ulOutCnt = 0;

//! Default value of Out Event iterationCount
unsigned long long g_ulOutCntIter = 0;

#if defined(TEST_HIGH_FREQ)
std::string g_strStartTime = "";
unsigned long long g_ulLastPulled = 0;
unsigned long long g_ulCurrPulled = 0;
unsigned long long g_ulDiscardedEvent= 0;
#endif

CCacheTransport::CCacheTransport():m_bIsEventWhitelistingEnabled(false)
{
    m_bHasStarted = false;

    m_bIsShutdownInitiated = false;

    std::string cacheLogPath = ic_core::CIgniteConfig::GetInstance()->
                                               GetString("DAM.Log.cacheStream");
    if (!cacheLogPath.empty())
    {
        HCPLOG_I << " - Setting stream log file to " << cacheLogPath;
        m_pStreamLog = new std::ofstream(cacheLogPath.c_str(), std::ios::out);
    }
    else
    {
        m_pStreamLog = NULL;
    }

    HCPLOG_C << "Init event processing chain";

    //let us construct the objects for the CHAIN-RESPONSIBILITY pattern
    if(ic_core::CUploadMode::GetInstance()->IsStreamModeSupported()) 
    {
        m_pMsgController = new CMessageController(
                                 CDBTransportWrapper::GetDBTransportInstance());
        m_pSessionStatusHandler = new CSessionStatusHandler(m_pMsgController);
    }
    else 
    {
        m_pMsgController = NULL;
        m_pSessionStatusHandler = new CSessionStatusHandler(
                                 CDBTransportWrapper::GetDBTransportInstance());
    }
    m_pDelayEventHandler = new CActivityDelay(m_pSessionStatusHandler);
    m_pEventTSValidationHandler =
                     new CEventTimestampValidationHandler(m_pDelayEventHandler);

    //create a copy of ignite whitelisted events from configuration
    m_jsonConfigWhitelistedEvents = ic_core::CIgniteConfig::GetInstance()->
                                     GetJsonValue(KEY_IGNITE_WHITE_LIST_EVENTS);
    if (m_jsonConfigWhitelistedEvents != ic_utils::Json::Value::nullRef 
                                    && m_jsonConfigWhitelistedEvents.isArray()
                                    && !(m_jsonConfigWhitelistedEvents.empty()))
    {
        HCPLOG_D << "HighFrequency Feature Enabled..";
        m_bIsEventWhitelistingEnabled = true;
        std::set<std::string> rsetWhitelistedEventsList;
        PopulateWhitelistedEventList(m_jsonConfigWhitelistedEvents,
                                     rsetWhitelistedEventsList);

        //read domainmap to add the corresponding events to whitelistedEvent
        m_jsonConfigDomainEventMap = ic_core::CIgniteConfig::GetInstance()->
                                             GetJsonValue(KEY_DOMAIN_EVENT_MAP);
        std::set<std::string> rSetDomainEventsList;
        if (PopulateEventsFromDomainMap(m_jsonConfigDomainEventMap,
                                        rSetDomainEventsList) > 0) 
        {
            rsetWhitelistedEventsList.insert(rSetDomainEventsList.begin(),
                                             rSetDomainEventsList.end());
        }
        //update m_setIgniteEventsList with populated whitelisteventslist
        m_setIgniteEventsList = rsetWhitelistedEventsList;

        //get additional(if any) whitelist events
        if (m_pMsgController)
        {
            std::set<std::string> strEventList =
                    m_pMsgController->GetSupplimentaryEventsListToWhitelist();
            m_setIgniteEventsList.insert(strEventList.begin(),
                                         strEventList.end());
        }

        HCPLOG_C << "m_setIgniteEventsList size:" << 
                                                m_setIgniteEventsList.size();
    }
    else
    {
        m_jsonConfigWhitelistedEvents = ic_utils::Json::Value::nullRef;
    }

    InitConfigBasedLogging();
    ic_core::CIgniteConfig::GetInstance()->
                  SubscribeForConfigUpdateNotification("CCacheTransport", this);
}

CCacheTransport::~CCacheTransport()
{
    if(m_pMsgController)
    { 
        delete m_pMsgController;
    }
    if (m_pSessionStatusHandler) 
    {
        delete m_pSessionStatusHandler;
    }
    if (m_pDelayEventHandler) 
    {
        delete m_pDelayEventHandler;
    }
    if (m_pEventTSValidationHandler) 
    {
        delete m_pEventTSValidationHandler;
    }
    if (m_pStreamLog != NULL)
    {
        if (m_pStreamLog->is_open())
        {
            m_pStreamLog->close();
        }
        delete m_pStreamLog;
    }
    ic_core::CIgniteConfig::GetInstance()->
                      UnSubscribeForConfigUpdateNotification("CCacheTransport");
    m_setIgniteEventsList.clear();
}

bool CCacheTransport::Send(const std::string& rstrSerialized)
{
    if (m_bIsShutdownInitiated)
    {
        //print complete event log when shutdown initiated
        HCPLOG_W << "Ignoring due to shutdown:"<< rstrSerialized;
        return false;
    }
    if ( (m_pStreamLog != NULL) && m_pStreamLog->is_open() )
    {
        *m_pStreamLog << rstrSerialized << std::endl;
    }

    bool ret = true;
    if (m_eventQueue.Size() < MAX_QUEUE_SIZE)
    {
        m_eventQueue.Put(rstrSerialized, rstrSerialized.size());
        if (0 == g_ulInCnt) 
        {
            g_ulInCntIter++;
        }
        g_ulInCnt++;
        #if defined(TEST_HIGH_FREQ)
        if (g_ulInCnt==1) {
            g_strStartTime = ic_utils::CIgniteDateTime::
                                                GetCurrentFormattedDateTime();
        }
        HCPLOG_I << "[S: " << g_strStartTime << "] Q-Size(INFLOW): " 
                 << m_eventQueue.Size() << "; g_ulInCnt=" << g_ulInCnt 
                 << "; g_ulOutCnt=" << g_ulOutCnt 
                 << "; Discarded: " << g_ulDiscardedEvent;
        #endif
    }
    else
    {
        ret = false;
    }

    if (!ret)
    {
        HCPLOG_E << "Q overflow-discarding " << rstrSerialized;
        g_ulTotOECnt++;
        if ((1 == g_ulTotOECnt) || (0 == g_ulTotOECnt%10)) 
        {
            HCPLOG_C << "OECnt:" << g_ulTotOECnt
                     << "IN:" << g_ulInCntIter << "~" << g_ulInCnt 
                     << ";OUT:" << g_ulOutCntIter << "~" << g_ulOutCnt 
                     << ";NI:" << g_ulNieCntIter << "~" << g_ulNieCnt 
                     << ";I:" << g_ulIgCntIter << "~" << g_ulIgCnt;
        }
        return ret;
    }

    if (!m_bHasStarted)
    {
        m_bHasStarted = true;
        Start();
    }
    else
    {
        m_handleQueueMutex.Lock();
        m_queueHandler.ConditionBroadcast();
        m_handleQueueMutex.Unlock();
    }

    return ret;
}

void CCacheTransport::FlushCache()
{
    std::string strEvent;
    while(m_eventQueue.Take(&strEvent))
    {
        ic_core::CEventWrapper* pEvent = new ic_core::CEventWrapper();
        pEvent->JsonToEvent(strEvent);
        m_pEventTSValidationHandler->HandleEvent(pEvent);
    }
    if(m_pMsgController) 
    {
        m_pMsgController->FlushCache();
    }
}

int CCacheTransport::GetUploadDeferTime()
{
    return m_pDelayEventHandler->ComputeDeferUpload();
}

CCacheTransport* CCacheTransport::GetInstance()
{
    static CCacheTransport cacheTransport;
    
    return &cacheTransport;
}

void CCacheTransport::Run()
{
    int nThreadPriority  = ic_core::CIgniteConfig::GetInstance()->
                                                 GetInt(EVENT_RECEIVER_PRIORITY,
                                                 DEF_EVENT_RECEIVER_PRIORITY);
    ic_utils::CIgniteThread::SetCurrentThreadPriority(nThreadPriority);

    ic_core::CIgniteClient::GetOnOffMonitor()->
                                           RegisterForShutdownNotification(this,
                                           ic_core::IOnOff::eR_CACHE_TRANSPORT);
    while(!m_bIsShutdownInitiated)
    {
        std::string strEvent;
        while (m_eventQueue.Take(&strEvent))
        {
            if (0 == g_ulOutCnt) 
            {
                g_ulOutCntIter++;
            }
            g_ulOutCnt++;
            // handle the event
            ic_core::CEventWrapper* pEvent = new ic_core::CEventWrapper();
            pEvent->JsonToEvent(strEvent);

            //reset log counter for inflow events when IgnStatus is off
            if (IsLogCounterResetNeeded(pEvent))
            {
                ResetInflowEventLogCounter();
                ic_bl::CMQTTUploader::GetInstance()->
                                                 ResetEventUploaderLogCounter();
            }

            if(!m_bIsEventWhitelistingEnabled)
            {
                //print complete log for ignite event
                LogCriticalEvent(pEvent->GetEventId(),strEvent);
                m_pEventTSValidationHandler->HandleEvent(pEvent);
            }
            else
            {
                ProcessEvent(pEvent, strEvent);
            }

            PrintLogCounter();
        }

        if (!m_bIsShutdownInitiated)
        {
            m_handleQueueMutex.Lock();
            m_queueHandler.ConditionWait(m_handleQueueMutex);
            m_handleQueueMutex.Unlock();
        }
    }
    ic_core::CIgniteClient::GetOnOffMonitor()->
                          ReadyForShutdown(ic_core::IOnOff::eR_CACHE_TRANSPORT);
    ic_core::CIgniteClient::GetOnOffMonitor()->
         UnregisterForShutdownNotification(ic_core::IOnOff::eR_CACHE_TRANSPORT);
    Detach();
}

bool CCacheTransport::IsLogCounterResetNeeded(ic_core::CEventWrapper *pEvent)
{
    if((pEvent->GetEventId() == KEY_IGNITIONSTATUS) 
       && (pEvent->GetData()[KEY_IGN_STATE].asString() == KEY_IGN_OFF) 
       && (m_nLogCounterReset == eIGN_CYCLE))
    {
        return true;
    }
    return false;
}

void CCacheTransport::ProcessEvent(ic_core::CEventWrapper *pEvent, 
                                  const std::string &rstrEventStr)
{
    std::string eventId = pEvent->GetEventId();
    m_Mutex.Lock();
    if (m_setIgniteEventsList.end() != m_setIgniteEventsList.find(eventId))
    {
        // Ignite Event
        m_Mutex.Unlock();
        if (0 == g_ulIgCnt) 
        {
            g_ulIgCntIter++;
        }
        g_ulIgCnt++;

        #if defined(TEST_HIGH_FREQ)
        g_ulCurrPulled = ic_utils::CIgniteDateTime::GetCurrentTimeMs();
        HCPLOG_I << "Q-Size(AFTER-PULLED): " << m_eventQueue.Size() 
                 << "-[I-" << eventId << "]- g_ulInCnt=" << g_ulInCnt 
                 << "; g_ulOutCnt=" << g_ulOutCnt 
                 << "; Dscrd: "<< g_ulDiscardedEvent 
                 << " [Diff: " << g_ulCurrPulled-g_ulLastPulled << "]" 
                 << "g_ulNieCnt :" << g_ulNieCnt 
                 << "  g_ulNieCntIter :" << g_ulNieCntIter 
                 << " g_ulIgCnt :" << g_ulIgCnt 
                 << " g_ulIgCntIter : " << g_ulIgCntIter;

        g_ulLastPulled = g_ulCurrPulled;
        #endif

        //print complete log for whitelisted event (ignite event)
        LogCriticalEvent(eventId, rstrEventStr);
        m_pEventTSValidationHandler->HandleEvent(pEvent);
    }
    else
    {
        // Non Ignite Event
        m_Mutex.Unlock();
        if (0 == g_ulNieCnt) 
        {
            g_ulNieCntIter++;
        }
        g_ulNieCnt++;
        double timestamp = pEvent->GetTimestamp();
        
        #if defined(TEST_HIGH_FREQ)
        g_ulCurrPulled = ic_utils::CIgniteDateTime::GetCurrentTimeMs();
        HCPLOG_I << "Q-Size(AFTER-PULLED): " << m_eventQueue.Size() 
                 << "-[NI-" << eventId << "]- g_ulInCnt=" << g_ulInCnt 
                 << "; g_ulOutCnt=" << g_ulOutCnt 
                 << "; Dscrd: " << g_ulDiscardedEvent 
                 << " [Diff: " << g_ulCurrPulled-g_ulLastPulled << "]";

        g_ulLastPulled = g_ulCurrPulled;
        if ((1 == g_ulNieCnt) || (0 == g_ulNieCnt%1000)) 
        {
            //logging first and all 100th events - for later tracking purposes
            HCPLOG_I << "non-ignite event cnt: " << g_ulNieCntIter << ":" 
                     << g_ulNieCnt << "-" << rstrEventStr;
        }
        HCPLOG_D <<"N-IgnEvnt cnt:" << g_ulNieCntIter << ":" << g_ulNieCnt 
                 << "-" << rstrEventStr;
        //print event id and timestamp for non ignite event
        HCPLOG_I << eventId << " TS: " << timestamp;
        #endif

        if(m_pEventTSValidationHandler->IsEventTimestampValid(timestamp))
        {
            CDBTransportWrapper::GetDBTransportInstance()->
                                                   HandleNonIgniteEvent(pEvent);
        }
        else
        {
            HCPLOG_I << "N-IgnEvnt Invalid TimeStamp";

            //delete N-Ign event with invalid timestamp
            if (pEvent)
            {
                delete pEvent;
            }
        }
    } 
}

void CCacheTransport::PrintLogCounter()
{
    if ((1 == g_ulOutCnt) || (0 == g_ulOutCnt%1000)) 
    {
        HCPLOG_D << "IN:" << g_ulInCntIter << "~" << g_ulInCnt 
                 << ";OUT:" << g_ulOutCntIter << "~" << g_ulOutCnt << ";NI:" 
                 << g_ulNieCntIter << "~" << g_ulNieCnt 
                 << ";I:" << g_ulIgCntIter << "~" << g_ulIgCnt;
    }
}

bool CCacheTransport::IsSessionInProgress()
{
    return m_pSessionStatusHandler->GetSessionStatus();
}

void CCacheTransport::NotifyShutdown()
{
    m_bIsShutdownInitiated = true;

    m_handleQueueMutex.Lock();
    m_queueHandler.ConditionBroadcast();
    m_handleQueueMutex.Unlock();
}

bool CCacheTransport::LogCriticalEvent(const std::string& eID,
                                       const std::string& ePayload)
{
    bool bCriticalLogging = true;

    //if alert, log w/o restriction
    if(m_setAlertsList.find(eID) != m_setAlertsList.end())
    {
        HCPLOG_C << ePayload;
        return bCriticalLogging;
    }

    //check if event is part of inflow event list
    std::map<std::string, int>::iterator lcIter = m_mapLogConfig.find(eID);

    if(lcIter != m_mapLogConfig.end())
    {
        bCriticalLogging = LogEventBasedOnConfiguredCount(lcIter->second, 
                                                          eID, ePayload);
    }
    else
    {
        bCriticalLogging = LogEventBasedOnDefaultCount(eID, ePayload);
    }

    return bCriticalLogging;
}

bool CCacheTransport::LogEventBasedOnConfiguredCount(const int &rnMaxCount, 
                                                 const std::string &rstrEvntID,
                                                 const std::string &rstrPayload)
{
    bool bCriticalLogging = true;

    //infinite logging in case of -1
    if (-1 == rnMaxCount)
    {
        HCPLOG_C << rstrPayload;
    }
    else if (0 == rnMaxCount)
    {
        //not logging the event to critical log level when rnMaxCount is 0
        HCPLOG_D << rstrPayload;
        bCriticalLogging = false;
    }
    else 
    {
        //now check how many times the event is logged
        std::map<std::string, int>::iterator elIter = 
                                                m_mapEvntLog.find(rstrEvntID);
        if (elIter != m_mapEvntLog.end())
        {
            int nCurrLoggedCnt = elIter->second;
            if (nCurrLoggedCnt < rnMaxCount)
            {
                HCPLOG_C << rstrPayload;
                elIter->second = ++nCurrLoggedCnt;
            }
            else
            {
                //log cnt exceeded; log as D
                HCPLOG_D << rstrPayload;
                bCriticalLogging = false;
            }
        }
        else
        {
            //not logged yet; log now and put an entry
            HCPLOG_C << rstrPayload;
            m_mapEvntLog.insert(std::pair<std::string, int>(rstrEvntID, 1));
        }
    }
    return bCriticalLogging;
}

bool CCacheTransport::LogEventBasedOnDefaultCount(const std::string &rstrEvntID,
                                                 const std::string &rstrPayload)
{
    bool bCriticalLogging = true;
    //if cnt is -1, log as CRITICAL
    if (-1 == m_nDefInflowEventLogCnt)
    {
        HCPLOG_C << rstrPayload;
    }
    else if (0 == m_nDefInflowEventLogCnt) 
    {
        /* if cnt is 0, do not log as CRITICAL (however we log in DEBUG level
         * for debug purposes)
         */
        HCPLOG_D << rstrPayload;
        bCriticalLogging = false;
    }
    else
    {
        /* event is NOT part of inflow event list; check for default log cnt
         * how many times logged?
         */
        std::map<std::string, int>::iterator elIter = 
                                                m_mapEvntLog.find(rstrEvntID);
        if (elIter != m_mapEvntLog.end())
        {
            int nCurrLoggedCnt = elIter->second;
            if (nCurrLoggedCnt < m_nDefInflowEventLogCnt)
            {
                HCPLOG_C << rstrPayload;
                elIter->second = ++nCurrLoggedCnt;
            }
            else
            {
                //log cnt exceeded; log as D
                HCPLOG_D << rstrPayload;
                bCriticalLogging = false;
            }
        }
        else
        {
            //not logged yet; log now and put an entry
            HCPLOG_C << rstrPayload;
            m_mapEvntLog.insert(std::pair<std::string, int>(rstrEvntID, 1));
        }
    }

    return bCriticalLogging;
}

void CCacheTransport::NotifyConfigUpdate()
{
    HCPLOG_D << "NotifyConfigUpdate";
    ic_utils::Json::Value newEventList = ic_core::CIgniteConfig::GetInstance()->
                          GetJsonValue(KEY_REMOTE_CRITICAL_EVENT_LOGGING_COUNT);
    if(newEventList.isArray() && !newEventList.empty()
       && newEventList !=  m_jsonEvntList)
    {
        if(newEventList !=  m_jsonEvntList)
        {
            m_jsonEvntList = newEventList;
            m_mapLogConfig.clear();
            UpdateLogConfigMap( m_jsonEvntList);
        }
    }
    else
    {
        HCPLOG_E << "NoUpdate for criticalEventLogging";
    }

    m_nDefInflowEventLogCnt = ic_core::CIgniteConfig::GetInstance()->
       GetInt(KEY_REMOTE_DEFAULT_EVENT_LOGGING_COUNT, DEF_INFLOW_EVENT_LOG_CNT);
    m_nLogCounterReset = ic_core::CIgniteConfig::GetInstance()->
                                      GetInt(KEY_LOG_COUNTER_RESET, eIGN_CYCLE);

    //updating whitelisted event list
    UpdateIgniteWhitelistEventList();
}

void CCacheTransport::UpdateIgniteWhitelistEventList()
{
    if (!m_bIsEventWhitelistingEnabled) 
    {
        return;
    }
       
    ic_utils::Json::Value jsonNewIgniteEvents = 
                                    ic_core::CIgniteConfig::GetInstance()->
                                    GetJsonValue(KEY_IGNITE_WHITE_LIST_EVENTS);
    ic_utils::Json::Value jsonNewDomainEventMap = 
                                    ic_core::CIgniteConfig::GetInstance()->
                                    GetJsonValue(KEY_DOMAIN_EVENT_MAP);

    if (IsIgniteWhiteListedEventsValid(jsonNewIgniteEvents))
    {
        bool bIsChanged = false;
        if (m_jsonConfigWhitelistedEvents != jsonNewIgniteEvents) 
        {
            bIsChanged = true;
        }

        if (IsDomainEventMapValid(jsonNewDomainEventMap))
        {
            bIsChanged = true;
        }

        if (bIsChanged) 
        {
            std::set<std::string> setWhitelistedEventsList;
            PopulateWhitelistedEventList(jsonNewIgniteEvents, 
                                         setWhitelistedEventsList);

            //read domainmap to add the corresponding events to whitelistedevent
            std::set<std::string> setDomainEventsList;
            if (0 < PopulateEventsFromDomainMap(jsonNewDomainEventMap, 
                                            setDomainEventsList)) 
            {
                setWhitelistedEventsList.insert(setDomainEventsList.begin(), 
                                                setDomainEventsList.end());
            }

            //update the config member variable
            m_jsonConfigWhitelistedEvents = jsonNewIgniteEvents;

            //update the domainmap member variable
            m_jsonConfigDomainEventMap = jsonNewDomainEventMap;

            //update ignite events list with new config
            m_Mutex.Lock();
            m_setIgniteEventsList = setWhitelistedEventsList;
            m_Mutex.Unlock();
        }
        else 
        {
            HCPLOG_D << "NoUpdate for WhitelistedEvents";
        }
    }
    else 
    {
        //ignore whole process
    }
}

bool CCacheTransport::IsIgniteWhiteListedEventsValid(const ic_utils::Json::Value 
                                                    &rjsonWhitelistedEvents)
{
    if (rjsonWhitelistedEvents != ic_utils::Json::Value::nullRef
        && !(rjsonWhitelistedEvents.empty())
        && rjsonWhitelistedEvents.isArray())
    {
        return true;
    }

    return false;
}

bool CCacheTransport::IsDomainEventMapValid(const ic_utils::Json::Value 
                                           &rjsonDomainEventMap)
{
    if (rjsonDomainEventMap != ic_utils::Json::Value::nullRef
        && !(rjsonDomainEventMap.empty())
        && rjsonDomainEventMap.isObject()
        && m_jsonConfigDomainEventMap != rjsonDomainEventMap)
    {
        return true;
    }

    return false;
}

bool CCacheTransport::UpdateLogConfigMap(
                                    const ic_utils::Json::Value &rjsonEventList)
{
    if(rjsonEventList.empty())
    {
        return false;
    }

    for (int nIter = 0; nIter < rjsonEventList.size(); nIter++)
    {
        ic_utils::Json::Value jsonEvent = rjsonEventList[nIter];
        ic_utils::Json::Value::Members jsonMember = jsonEvent.getMemberNames();
        for (int nJter = 0; nJter < jsonMember.size(); nJter++)
        {
            std::string strEvnID = jsonMember[nJter];
            if (jsonEvent[jsonMember[nJter]].isInt())
            {
                int nCount = 
                   GetValidInflowLogCount(jsonEvent[jsonMember[nJter]].asInt());
                m_mapLogConfig.insert(
                                std::pair<std::string, int>(strEvnID, nCount));
            }
            else
            {
                // default cnt from KEY_REMOTE_DEFAULT_EVENT_LOGGING_COUNT
            }
        }
    }
    PrintMap(m_mapLogConfig);
    return true;
}

int CCacheTransport::GetValidInflowLogCount(int nCount)
{
    if (nCount < DEF_INFLOW_EVENT_LOG_CNT)
    {
        nCount = DEF_INFLOW_EVENT_LOG_CNT;
    }
    else if (nCount > DEF_MAX_INFLOW_EVENT_LOG_CNT) //nCount > 100
    {
        nCount = DEF_MAX_INFLOW_EVENT_LOG_CNT;
    }
    else
    {
        //do nothing valid cnt within range -1 to 100
    }
    
    return nCount;
}

bool CCacheTransport::InitConfigBasedLogging()
{
    bool flag = true;
    ic_utils::Json::Value inflowEvntLogCfg = 
                                         ic_core::CIgniteConfig::GetInstance()->
                                         GetJsonValue(KEY_INFLOW_EVENT_LOGGING);
    if (inflowEvntLogCfg.isMember(KEY_CRITICAL_EVENT_LOGGING_COUNT) 
        && inflowEvntLogCfg[KEY_CRITICAL_EVENT_LOGGING_COUNT].isArray())
    {
         m_jsonEvntList = inflowEvntLogCfg[KEY_CRITICAL_EVENT_LOGGING_COUNT];
        UpdateLogConfigMap( m_jsonEvntList);
        HCPLOG_D << PrintMap(m_mapLogConfig);
    }
    else
    {
        flag = false;
    }

    if (inflowEvntLogCfg.isMember(KEY_DEFAULT_LOGGING_COUNT) 
        && inflowEvntLogCfg[KEY_DEFAULT_LOGGING_COUNT].isInt())
    {
        m_nDefInflowEventLogCnt = 
                            inflowEvntLogCfg[KEY_DEFAULT_LOGGING_COUNT].asInt();
    }
    else
    {
        m_nDefInflowEventLogCnt = DEF_INFLOW_EVENT_LOG_CNT;
    }

    ic_utils::Json::Value alertArray = 
         ic_core::CIgniteConfig::GetInstance()->GetJsonValue(KEY_DIRECT_ALERTS);
    if (alertArray.isArray())
    {
        for (int i = 0; i < alertArray.size(); i++)
        {
            m_setAlertsList.insert(alertArray[i].asString());
        }
    }
    else
    {
        flag = false;
    }

    m_nLogCounterReset = ic_core::CIgniteConfig::GetInstance()->GetInt(
                                             KEY_LOG_COUNTER_RESET, eIGN_CYCLE);
    return flag;
}

bool CCacheTransport::PrintMap(const std::map<std::string, int>& rMap)
{
    if(rMap.empty())
    {
        return false;
    }
    else
    {
        for (auto it = rMap.cbegin(); it != rMap.cend(); ++it)
        {
            HCPLOG_D << (*it).first << " : " << (*it).second;
        }
        return true;
    }
}

bool CCacheTransport::ResetInflowEventLogCounter()
{
    if(!m_mapEvntLog.empty())
    {
        m_mapEvntLog.clear();
    }
    return true;
}

int CCacheTransport::PopulateWhitelistedEventList(const ic_utils::Json::Value& 
                                  rjsonWhitelistedConfig, std::set<std::string>&
                                  rsetWhitelistedEventsList)
{
    rsetWhitelistedEventsList.clear();
    for(int i=0; i<rjsonWhitelistedConfig.size(); i++) 
    {
        rsetWhitelistedEventsList.insert(rjsonWhitelistedConfig[i].asString());
    }
    return rsetWhitelistedEventsList.size();
}

int CCacheTransport::PopulateEventsFromDomainMap(const ic_utils::Json::Value&
               rjsonDomainEventMap, std::set<std::string>& rSetDomainEventsList)
{
    rSetDomainEventsList.clear();
    for (ic_utils::Json::ValueIterator it = rjsonDomainEventMap.begin(); 
         it != rjsonDomainEventMap.end(); it++) 
    {
        ic_utils::Json::Value domainSec =
                                       rjsonDomainEventMap[it.key().asString()];
        if (domainSec.isArray()) 
        {
            for (int i = 0; i < domainSec.size(); i++) 
            {
                rSetDomainEventsList.insert(domainSec[i].asString());
            }
        }
        else if (domainSec.isString()) 
        {
            rSetDomainEventsList.insert(domainSec.asString());
        }
        else 
        {
            HCPLOG_E "Invalid Config";
        }
    }
    return rSetDomainEventsList.size();
}
}/* namespace ic_bl */
