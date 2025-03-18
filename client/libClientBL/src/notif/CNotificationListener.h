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
* \file CNotificationListener.h
*
* \brief This class provides the interfaces for handling MQTT notifications
* received from cloud 
********************************************************************************
*/

#ifndef CNOTIFICATION_LISTENER_H
#define CNOTIFICATION_LISTENER_H

#include "CIgniteThread.h"

#include <string>
#include <queue>
#include <map>
#include <condition_variable>
#include <mutex>
#include "IOnOffNotificationReceiver.h"

namespace ic_bl 
{

/**
 * Interface for Handling the MQTT notification/message
 */
class IRequestHandler  {
public:
    /**
     * Pure Virtual Method for handling the MQTT notification/message
     * @param[in] cstrRequestJson Request Json that needs to be handled
     * @return void
     */
    virtual void Handle(const char *cstrRequestJson) = 0;

    /**
     * Destructor
     */
    virtual ~IRequestHandler(){};
};

/**
 * Class implements methods which would be used for handling the MQTT
 * notification/commands coming from the mqtt host
 */
class CNotificationListener : public ic_utils::CIgniteThread, 
                              public ic_core::IOnOffNotificationReceiver  
{
public:
    /**
     * Pure Virtual Method for handling the MQTT notification/message
     * @param[in] cstrRequestJson Request Json that needs to be handled
     * @return void
     */
    static CNotificationListener* GetNotificationListener();

    /**
     * Destructor
     */
    virtual ~CNotificationListener();

    /**
     * Overriding the method CIgniteThread::Run
     * @see ic_utils::CIgniteThread::Run()
     */
    void Run();

    /**
     * Method for registering the Request handler for a particular request type
     * @param[in] pReqHndlr Pointer to a request handler
     * @param[in] strReqType Request type
     * @return void
     */
    void RegisterRequest(IRequestHandler *pReqHndlr, std::string strReqType);

    /**
     * Method to push the request/message to the notifications queue
     * @param[in] strData Request to be pushed
     * @return void
     */
    void PushNotification(std::string strData);

    /**
     * Overriding IOnOffNotificationReceiver::NotifyShutdown
     * @see ic_core::IOnOffNotificationReceiver::NotifyShutdown()
     */
    void NotifyShutdown() override;

    #ifdef IC_UNIT_TEST
        friend class FileNotificationHandlerTest;
    #endif

private:
    /**
     * Default constructor
     */
    CNotificationListener();

    /**
     * Method to pop the notification/message from the queue
     * @param void
     * @return Popped message/notification
     */
    std::string PopNotification();

    /**
     * Method to send the notification/message to corresponding request handlers
     * @param[in] cstrRequestJson Request string
     * @return void
     */
    void SendNotification(const char *cstrRequestJson);

    /**
     * Queue for storing the notifications received
     */
    std::queue<std::string> m_queNotifications;

    /**
     * Map for storing Request type and Corresponding request handler
     */
    std::map<std::string, IRequestHandler*> m_mapNotifHandlers;

    /**
     * Flag to maintain shut down status
     */
    bool m_bIsShutDownInitiated = false;
};

} //namespace ic_bl

#endif /* CNOTIFICATION_LISTENER_H */
