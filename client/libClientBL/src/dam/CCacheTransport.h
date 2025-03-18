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

/*!
********************************************************************************
* \file CCacheTransport.h
*
* \brief This class receives event from CEventReceiver and initializes chain
* of responsibility where different handlers are added to the chain in order to
* handle different processing on the incoming event.
********************************************************************************
*/

#ifndef CCACHE_TRANSPORT_H
#define CCACHE_TRANSPORT_H

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include "CIgniteMutex.h"
#include "CConcurrentQueue.h"
#include "CIgniteConfig.h"
#include "CIgniteThread.h"
#include "IOnOffNotificationReceiver.h"
#include "dam/CEventTimestampValidationHandler.h"
#include "dam/CActivityDelay.h"
#include "dam/CSessionStatusHandler.h"
#include "dam/CMessageController.h"

namespace ic_bl 
{
/**
 * Enum used to reset logging counter
 */
typedef enum
{
    eIGN_CYCLE,  ///< Reset on ignition cycle
    ePOWER_CYCLE ///< Reset on power cycle
} InflowEventLogCounterResetEnum;

/**
 * Class CCacheTransport receives event from CEventReceiver and initializes 
 * chain of responsibility where different handlers are added to the chain in
 * order to handle different processing on the incoming event.
 */
class CCacheTransport : public ic_utils::CIgniteThread, 
                        public ic_core::IOnOffNotificationReceiver,
                        public ic_core::IConfigUpdateNotification 
{
public:

    /**
     * Method to get Instance of CCacheTransport
     * @param void
     * @return Pointer to Singleton Object of CCacheTransport
     */
    static CCacheTransport* GetInstance();

    /**
     * Method to receive events from CEventReceiver and store it in event queue
     * @param [in] rstrSerialized received event from CEventReceiver
     * @return true if event stored in queue, otherwise false if queue is 
     *         overflow
     */
    bool Send(const std::string& rstrSerialized);

    /**
     * This method flushes all events from m_eventQueue
     * @param void
     * @return void
     */
    void FlushCache();

    /**
     * Method to get computes defer time for uploadmanager based on its specific
     * timeouts from config
     * @param void
     * @return computed defer time get from CActivityDelay->ComputeDeferUpload()
     */
    int GetUploadDeferTime();

    /**
     * Method to check the session progress status from CSessionStatusHandler
     * @param void
     * @return true if status is in progress, otherwise false
     */
    bool IsSessionInProgress();

    /**
     * Overriding Method of IOnOffNotificationReceiver class
     * @see IOnOffNotificationReceiver::NotifyShutdown()
     */
    void NotifyShutdown() override;

    /**
     * Overriding Method of IConfigUpdateNotification class
     * @see IConfigUpdateNotification::NotifyConfigUpdate()
     */
    void NotifyConfigUpdate();

    #ifdef IC_UNIT_TEST
        friend class CCacheTransportTest;
    #endif

private:
    /**
     * Default no-argument constructor.
     */
    CCacheTransport();

    /**
     * Destructor
     */
    virtual ~CCacheTransport();

    /**
     * Method is used to log the payload of inflow events as CRITICAL as per
     * the count configured in config file
     * @param[in] rstrEID event-id use to check count in config
     * @param[in] rstrEPayload event payload in string use to log
     * @return true if event is logged as CRITICAL otherwise false
     * (logged in DEBUG level for debug purposes)
     */
    bool LogCriticalEvent(const std::string &rstrEID,
                                              const std::string &rstrEPayload);

    /**
     * Method to update map of eventId and its log count as per received 
     * configuration
     * @param[in] const ic_utils::Json::Value& evntList: configuration value for
     *            critical events and its logging count
     * @return true if map of eventId and log count is updated, otherwise false
     */
    bool UpdateLogConfigMap(const ic_utils::Json::Value& rjsonEvntList);

    /**
     * Method to initialize values from inflowEventLogging and DirectAlerts
     * configuration
     * @param void 
     * @return true if configuration initilized, otherwise false for
     *         invalid or not found
     */
    bool InitConfigBasedLogging();

    /**
     * Method to print Configmap contents (map of eventId and its log count)
     * @param[int] const std::map<std::string, int>& map: map of eventId and
     *             its log count
     * @return true if prints map contents, otherwise false for empty map
     */
    bool PrintMap(const std::map<std::string, int>& rMap);

    /**
     * Method to clear list of events count logged as CRITICAL
     * @param void
     * @return true if list is cleared, otherwise false
     */ 
    bool ResetInflowEventLogCounter();

    /**
     * Method to update list of ignite events from IgnitewhiteListedEvents 
     * configuration and list of events from rjsonDomainEventMap configuration
     * @param void
     * @return void
     */ 
    void UpdateIgniteWhitelistEventList();

    /**
     * Method to populate list of events from rjsonDomainEventMap configuration
     * @param [in] const ic_utils::Json::Value& rjsonDomainEventMap:
     *             configuration value of domain name and its supported events
     * @param [out] std::set<std::string>& rSetDomainEventsList: list of events
     * @return size of events list
     */ 
    int PopulateEventsFromDomainMap(const ic_utils::Json::Value& 
              rjsonDomainEventMap, std::set<std::string>& rSetDomainEventsList);

    /**
     * Method to populate list of ignite events from IgnitewhiteListedEvents
     * configuration
     * @param [in] const ic_utils::Json::Value& rjsonWhitelistedConfig:
     *             config value of ignite whitested events 
     * @param [out] std::set<std::string>& rsetWhitelistedEventsList:
     *              list of ignite events
     * @return size of events list
     */ 
    int PopulateWhitelistedEventList(const ic_utils::Json::Value&
    rJsonWhitelistedConfig, std::set<std::string>& rSetWhitelistedEventsList);

    /**
     * Overriding Method of ic_utils::CIgniteThread class
     * @see CIgniteThread::Run()
     */
    void Run();

    /**
     * Method to check if reset log counter is needed or not
     * @param[in] pEvent event data
     * @return true to reset log counter, false otherwise
     */
    bool IsLogCounterResetNeeded(ic_core::CEventWrapper *pEvent);

    /**
     * Method to process event based on ignite whitelisting
     * @param[in] pEvent event data
     * @param[in] rstrEventStr event data string
     * @return void
     */
    void ProcessEvent(ic_core::CEventWrapper *pEvent, 
                      const std::string &rstrEventStr);
    
    /**
     * Method to print log counter value
     * @param void
     * @return void
     */
    void PrintLogCounter();

    /**
     * Method to get valid inflow log count value
     * @param[in] nCount log count
     * @return return valid log count value
     */
    int GetValidInflowLogCount(int nCount);
	
	/**
     * Method to log event based on configured max count
     * @param[in] rnMaxCount max allowed log count
     * @param[in] rstrEvntID event id
     * @param[in] rstrPayload event payload
     * @return true if event is logged as CRITICAL otherwise false
     * (logged in DEBUG level for debug purposes)
     */
    bool LogEventBasedOnConfiguredCount(const int &rnMaxCount, 
                                        const std::string &rstrEvntID,
                                        const std::string &rstrPayload);
    
    /**
     * Method to log event based on default logging count
     * @param[in] rstrEvntID event id
     * @param[in] rstrPayload event payload
     * @return true if event is logged as CRITICAL otherwise false
     * (logged in DEBUG level for debug purposes)
     */
    bool LogEventBasedOnDefaultCount(const std::string &rstrEvntID,
                                     const std::string &rstrPayload);

    /**
     * Method to check if given whitelisted events config is valid or not
     * @param[in] rjsonWhitelistedEvents whitelisted events
     * @return true if whitelist events is valid, false otherwise
     */
    bool IsIgniteWhiteListedEventsValid(const ic_utils::Json::Value 
                                        &rjsonWhitelistedEvents);

    /**
     * Method to check if given domain events map config is valid or not
     * @param[in] rjsonDomainEventMap domain events map
     * @return true if domain events map config is valid, false otherwise
     */
    bool IsDomainEventMapValid(const ic_utils::Json::Value 
                               &rjsonDomainEventMap);

    //! Member variable holding CEventTimestampValidationHandler class instance
    CEventTimestampValidationHandler* m_pEventTSValidationHandler;

    //! Member variable holding CActivityDelay class instance
    CActivityDelay* m_pDelayEventHandler;

    //! Member variable holding CSessionStatusHandler class instance
    CSessionStatusHandler* m_pSessionStatusHandler;

    //! Member variable holding CMessageController class instance
    CMessageController* m_pMsgController;

    //! Details
    /**
     * Member variable ofstream instance to write received events 
     * in cacheStream file
     * 
     */
    std::ofstream* m_pStreamLog;

    //! Member variable to maintain thread started status
    bool m_bHasStarted;

    //! Member variable to store received events in queue
    ic_utils::CConcurrentQueue<std::string> m_eventQueue;

    //! Member variable holding mutex
    ic_utils::CIgniteMutex m_handleQueueMutex;

    //! Member variable CIgniteThread condition wait variable
    ic_utils::CThreadCondition m_queueHandler;
    
    //! Member variable flag to track the device shutdown status
    bool m_bIsShutdownInitiated;

    //! Details
    /**
     *  Member variable son to store a copy of whitelisted event list based on 
     *  DAM.IgnitewhiteListedEvents configuration update 
     */
    ic_utils::Json::Value m_jsonConfigWhitelistedEvents;

    //! Details
    /**
     * Member variable json to store a copy of domain event map to use for 
     * dynamic config update purposes  
     */ 
    ic_utils::Json::Value m_jsonConfigDomainEventMap;

    //! Member variable container to store whitelisted events list
    std::set<std::string> m_setIgniteEventsList;

    //! Member variable used to enable disable EventWhitelisting feature
    bool m_bIsEventWhitelistingEnabled;

    //! Details
    /**
     * Member variable container to store list of inflow event logging count 
     * as per the configuration file 
     */ 
    std::map<std::string, int> m_mapLogConfig;

    //! Details
    /**
     * Member variable container to store list of all inflow event logging count
     * as critical
     */ 
    std::map<std::string, int> m_mapEvntLog;

    //! Member variable to store default logging count, default it is set to -1
    int m_nDefInflowEventLogCnt;

    //! Member variable to reset event logging count
    int m_nLogCounterReset;

    //! Details
    /**
     * Member variable json to store a copy of event list from 
     * FileLogger.inflowEventLogging.criticalEventLoggingCount
     */
    ic_utils::Json::Value  m_jsonEvntList;

    //! Member variable container to store list of all direct alerts
    std::set<std::string> m_setAlertsList;

    //! Mutex instance
    ic_utils::CIgniteMutex m_Mutex;

};

} /* namespace ic_bl*/

#endif /* CCACHE_TRANSPORT_H */
