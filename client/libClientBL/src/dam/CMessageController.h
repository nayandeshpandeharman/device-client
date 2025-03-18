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
* \file CMessageController.h
*
* \brief This class handles event and sends it to store into database 
* and to message handlers for further processessing
********************************************************************************
*/

#ifndef CMESSAGE_CONTROLLER_H
#define CMESSAGE_CONTROLLER_H

#include <string>
#include <vector>
#include <CIgniteMutex.h>
#include "CIgniteThread.h"
#include "jsoncpp/json.h"
#include "CTransportHandlerBase.h"
#include "CConcurrentQueue.h"
#include "analytics/CEventProcessor.h"

#include "IMessageHandler.h"
#include "IExtendedClientConnector.h"

#include "analytics/CInternalMessageHandler.h"
#ifdef INCLUDE_IGNITE_AUTO
#include "analytics/CAnalyticsMessageHandler.h"
#endif

#include "IOnOffNotificationReceiver.h"

namespace ic_bl 
{

/**
 * Class CMessageController is the last handler in chain of resposibilty, it 
 * sends event to store in the database and to the processors for analytics 
 * through message handlers.
 */
class CMessageController : public ic_utils::CIgniteThread, public CTransportHandlerBase, public ic_core::IOnOffNotificationReceiver
{
public:

    /**
     * Parameterized constructor
     * @param[in] pNextHandler Pointer to the CTransportHandlerBase class
     */
    CMessageController(CTransportHandlerBase* pNextHandler);

    /**
     * Method to handle event to store in database and to processors for analytics
     * @param[in] pEvent Event data
     * @return void
     */
    virtual void HandleEvent(ic_core::CEventWrapper* pEvent);

    /**
     * Destructor
     */
    ~CMessageController();

    
    /**
     * Method to check if received message is alert
     * @param[in] rstrEventID Event Id
     * @return true if it is alert, false otherwise
     */
    static bool IsAlert(const std::string& rstrEventID);

    /**
     * Overriding CIgniteThread::Run
     * @see ic_utils::CIgniteThread::Run()
     */
    virtual void Run();

    /**
     * Method flushes all events from m_queMqttEvents
     * @param void
     * @return void
     */
    void FlushCache();

    /**
     * Method to init message handlers
     * @param void
     * @return void
     */
    void Init();

    /**
     * Method to check if received message is alert
     * @param[in] pHandler Pointer to Message handlers object
     * @return void
     */
    static void AddHandler(ic_core::IMessageHandler *pHandler);

    /**
     * Method to handle notification received over MQTT
     * @param[in] rstrDomain Domain name
     * @param[in] rjsonNotifPayload Payload recieved over MQTT
     * @return void
     */
    static void HandleNotification(const std::string& rstrDomain, const ic_utils::Json::Value& rjsonNotifPayload);

    /**
     * Overriding Method of IOnOffNotificationReceiver class
     * @see IOnOffNotificationReceiver::NotifyShutdown()
     */
    void NotifyShutdown() override;

    /**
    * Method to get the list of events for whitelisting
    * @param void
    * @return set of strings - set of eventIds for whitelisting
    */
    std::set<std::string> GetSupplimentaryEventsListToWhitelist();


private:

    /**
    * Method to process event by its respective handler
    * @param[in] rstrEventID Event Id
    * @param[in] rstrSerEvent Event payload json string
    * @return void
    */
    void ProcessEvent(const std::string& rstrEventID, const std::string& rstrSerEvent);

    /**
    * Method for mutex wait
    * @param void
    * @return void
    */
    void Wait();

    /**
    * Method for mutex notify
    * @param void
    * @return void
    */
    void Notify();

    /**
     * Structure of handler and event and notification list belonging to it.
     */
    typedef struct
    {
        ic_core::IMessageHandler* pHandlerRef;  ///< Message Handler's object
        ic_utils::Json::Value::Members jsonEventList;   ///< Event domain list
        ic_utils::Json::Value::Members jsonNotifDomainList; ///< Notification domain list
    }MessageHandler;

    //! Member variable to vector of Message handlers
    static std::vector<MessageHandler> m_vecHndlrList;

    //! Member variable for mqtt event wait mutex
    ic_utils::CIgniteMutex m_mqttEvWaitMutex;
    
    //! Member variable for mqtt event wait condition
    ic_utils::CThreadCondition m_mqttEvWaitCondition;
    
    //! Member variable to hold queue of string of mqtt event
    ic_utils::CConcurrentQueue<std::string> m_queMqttEvents;

    //! Member variable to track device shutdown status
    bool m_bIsShutdownInitiated;

   //! Member variable to holds object of internal message handler
    CInternalMessageHandler* m_pImh;

#ifdef INCLUDE_IGNITE_AUTO
   //! Member variable to holds object of Analytics message handler
    ic_auto::CAnalyticsMessageHandler* m_pAmh;
#endif
};
} /* namespace ic_bl*/
#endif // CMESSAGE_CONTROLLER_H
