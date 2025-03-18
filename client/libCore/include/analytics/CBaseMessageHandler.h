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
*******************************************************************************
* \file CBaseMessageHandler.h
*
* \brief This class/module implements Ignite-Client's internal base 
* implementation of MessageHandler interface. Since some of the API
* implementations will be same irrespective of Internal/Analytics handlers of 
* Ignite-Client, they can just extend this based implementation to add handler
* specific implementation for other APIs.
*******************************************************************************
*/

#ifndef CBASE_MESSAGE_HANDLER_H
#define CBASE_MESSAGE_HANDLER_H

#include <vector>
#include "jsoncpp/json.h"
#include "CIgniteMutex.h"
#include "analytics/CEventProcessor.h"
#include "IMessageHandler.h"
#include "IClientConnector.h"

//! Constant key for 'serviceName' string
static const std::string SERVICE_NAME = "serviceName";

//! Constant key for 'MQTT.servicesTopic' string
static const std::string MQTT_SERVICE_TOPIC = "MQTT.servicesTopic";

//! Constant key for 'MQTT.domainEventMap' string
static const std::string MQTT_DOMAIN_EVENT_MAP = "MQTT.domainEventMap";

namespace ic_core 
{
/**
 * Class CBaseMessageHandler implements internal base implementation of 
 * MessageHandler interface
 */
class CBaseMessageHandler : public IMessageHandler 
{
public:
    /**
     * Default no-argument constructor.
     */
    CBaseMessageHandler();

    /**
     * Destructor
     */
    virtual ~CBaseMessageHandler();

    /**
     * Overriding Method of IMessageHandler class
     * @see IMessageHandler::Init()
     */
    void Init();

    /**
     * Overriding Method of IMessageHandler class
     * @see IMessageHandler::DeInit()
     */
    virtual void DeInit();

    /**
     * Overriding Method of IMessageHandler class
     * @see IMessageHandler::NotifyMessage()
     */
    virtual void NotifyMessage(MsgPayload stMsgPayload);

    /**
     * Overriding Method of IMessageHandler class
     * @see IMessageHandler::SetClientConnector()
     */
    virtual void SetClientConnector(IClientConnector *pCCnctr);

    /**
     * Overriding Method of IMessageHandler class
     * @see IMessageHandler::IsHandlerSubscribedForEvent()
     */
    virtual bool IsHandlerSubscribedForEvent(const std::string &rstrEventID);

    /**
     * Overriding Method of IMessageHandler class
     * @see IMessageHandler::IsHandlerSubscribedForNotification()
     */
    virtual bool IsHandlerSubscribedForNotification(const std::string 
                                                    &rstrDomain);

    /**
     * Overriding Method of IMessageHandler class
     * @see IMessageHandler::GetSupplimentaryEventsList()
     */
    virtual std::set<std::string> GetSupplimentaryEventsList();

    /**
     * Method to update event processor map
     * @param void
     * @return void
     */
    void UpdateEventProcessorMap();

    /**
     * Method to update event processor map for event handler by adding the 
     * associated handler according to the given domain
     * @param[in] rstrDomain String containing domain name
     * @return void
     */
    virtual void UpdateEventProcessorMapForEventHandler(const std::string 
                                                        &rstrDomain);

    /**
     * Method to update event processor map for notification handler by adding 
     * the associated handler according to the given domain
     * @param[in] rstrDomain String containing domain name
     * @return void
     */
    virtual void UpdateEventProcessorMapForNotificationHandler(const std::string
                                                               &rstrDomain);

    /**
     * Method to add the domain to map based on input parameter
     * @param[in] rstrEventID String containing eventID
     * @param[in] rstrDomain String containing domain name
     * @return void
     */
    void AddDomainToMap(const std::string &rstrEventID,
                        const std::string &rstrDomain);

    /**
     * Method to create reverse map of domain event map
     * @param void
     * @return void
     */
    void CreateReverseOfDomainEventMap();

    /**
     * Method to update map of eventID as key and domain as value
     * @param[in] rstrEventID String containing eventID
     * @param[in] rstrDomain String containing domain name
     * @return void
     */
    void UpdateEventDomainMap(const std::string &rstrEventID,
                              const std::string &rstrDomain);

    /**
     * Method to refresh map of eventID as key and domain as value
     * @param void
     * @return void
     */
    void RefreshEventDomainMap();

    #ifdef IC_UNIT_TEST
        friend class CBaseMessageHandlerTest;
    #endif

private:
    /**
     * Method to process received event type message
     * @param[in] reMsgPayload Event message
     * @return void
     */
    void ProcessEventTypeMessage(const MsgPayload &rstMsgPayload);

    /**
     * Method to process received notification type message
     * @param[in] reMsgPayload Notification message
     * @return true if notification is processed abd send to handler, 
     *      false otherwise
     */
    bool ProcessNotificationTypeMessage(const MsgPayload &rstMsgPayload);

    /**
     * Method to print logs based on input parameter
     * @param void
     * @return void
     */
    void PrintEventDomainMapLogs();

    /**
     * Method to refresh eventDomain list based on EventProcessorMap
     * @param[in] rstrEName String containing EventID
     * @param[out] rjsonEventDomainMap JSON Containing EventDomainMap value
     * @return void
     */
    void RefreshEventDomainListBasedOnEventProcessorsMap(
                                                   const std::string &rstrEName,
                                         ic_utils::Json::Value &rjsonEventDomainMap);

    /**
     * Method to update currEventDomainMap for array type
     * @param[out] rjsonUpdatedEventDomainMap UpdatedEventDomainMap
     * @param[in] rstrDomain String Containing domain name
     * @param[in] rjsonCurrDomainSec Current Domain section
     * @param[in] rjsonNewDomainSec New Domain section
     * @return void
     */
    void UpdateEventDomainMapForArrayType(
                                   ic_utils::Json::Value &rjsonUpdatedEventDomainMap,
                                   const std::string &rstrDomain,
                                   const ic_utils::Json::Value &rjsonCurrDomainSec,
                                   const ic_utils::Json::Value &rjsonNewDomainSec);

    /**
     * Method to update currEventDomainMap for string type
     * @param[out] rjsonUpdatedEventDomainMap UpdatedEventDomainMap
     * @param[in] rstrDomain String Containing domain name
     * @param[in] rjsonCurrDomainSec Current Domain section
     * @param[in] rjsonNewDomainSec New Domain section
     * @return void
     */
    void UpdateEventDomainMapForStringType(
                                   ic_utils::Json::Value &rjsonUpdatedEventDomainMap,
                                   const std::string &rstrDomain,
                                   const ic_utils::Json::Value &rjsonCurrDomainSec,
                                   const ic_utils::Json::Value &rjsonNewDomainSec);

protected:
    /**
     * Method to update the current map of eventID as key and domain as value
     * based on input parameter
     * @param[in] rjsonNewDomainEventMap JSON object containing map of new 
     * domain and eventID
     * @return void
     */
    void UpdateCurrentEventDomainMap(ic_utils::Json::Value &rjsonNewDomainEventMap);

    /**
     * Method to remove event domain reference from map based on input parameter
     * @param[in] rjsonEventDomainMap JSON object containing event domain map
     * @param[in] rstrDomain String containing domain name
     * @param[in] rstrEventName String containing event name
     * @return void
     */
    void RemoveEventDomainReferenceFromMap(ic_utils::Json::Value &rjsonEventDomainMap,
                                           const std::string &rstrDomain,
                                           const std::string &rstrEventName);

    /**
     * Method to add event domain reference to map based on input parameter
     * @param[in] rjsonEventDomainMap JSON object containing event domain map
     * @param[in] rstrDomain String containing domain name
     * @param[in] rstrEventName String containing event name
     * @return void
     */
    void AddEventDomainReferenceToMap(ic_utils::Json::Value &rjsonEventDomainMap,
                                      const std::string &rstrDomain,
                                      const std::string &rstrEventName);

    /**
     * Method to dispatch event entries from map based on input parameter
     * @param[in] rstrEventID String containing event id
     * @param[in] rstrDomain String containing domain name
     * @param[in] rstrSerEvent String containing serialized event
     * @return void
     */
    void DispatchEvent(const std::string &rstrEventID, 
                       const std::string &rstrDomain, 
                       const std::string &rstrSerEvent);

    //! Member variable to stores events domain and associated processor in map
    std::map <std::string, CEventProcessor*> m_mapEventProcessors;

    //! Member variable to stores notif's domain and associated processor in map
    std::map <std::string, CEventProcessor*> m_mapNotifProcessors;

    //! Member variable to stores JSON object for domain event configuration
    ic_utils::Json::Value m_jsonDomainEventMap;

    //! Member variable to stores JSON object for event domain configuration
    ic_utils::Json::Value m_jsonEventDomainMap;

    //! Member variable to stores copy of domain event map configuration
    ic_utils::Json::Value m_jsonConfigDomainEventMap;
};
} /* namespace ic_core */

#endif /* CBASE_MESSAGE_HANDLER_H */
