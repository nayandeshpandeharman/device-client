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
* \file IMessageHandler.h
*
* \brief This class provides the necessary interfaces for handlers to receive 
* various messages (event/notif.) for further processing.
********************************************************************************
*/

#ifndef IMESSAGE_HANDLER_H
#define IMESSAGE_HANDLER_H

#include <vector>
#include <string>
#include <set>
#include "IClientConnector.h"

namespace ic_core 
{
/**
 * Interface class expose APIs necessary for handlers to receive various 
 * messages
 */
class IMessageHandler 
{
public:
    /**
     * Enum of message type
     */
    typedef enum 
    {
        eMSG_TYPE_EVENT = 1,       ///< Event message
        eMSG_TYPE_NOTIFICATION = 2 ///< Notification message
    } MsgType;

    /**
     * Structure for message type and its payload
     */
    typedef struct 
    {
        MsgType eType;              ///< Enum of message type
        std::string strPayloadJson; ///< String contains message payload
    } MsgPayload;

    /**
     * Destructor
     */
    virtual ~IMessageHandler() 
    { 

    };

    /**
     * Method to initialize handlers to receive various messages
     * @param void
     * @return void
     */
    virtual void Init() = 0;

    /**
     * Method to deInitialize handlers from receiving various messages
     * @param void
     * @return void
     */
    virtual void DeInit() = 0;

    /**
     * Method to handle incoming message notification.
     * @param[in] msgPayload message payload
     * @return void
     */
    virtual void NotifyMessage(MsgPayload eMsgPayload) = 0;

    /**
     * Method to set the instance of IClientConnector
     * @param[in] pClientConntr instance of IClientConnector
     * @return void
     */
    virtual void SetClientConnector(IClientConnector *pClientConntr) = 0;

    /**
     * Method to check if the handler is subscribed for the event or not
     * @param[in] rStrEventID event ID
     * @return true if handler is subscribed for event , false otherwise
     */
    virtual bool IsHandlerSubscribedForEvent(const std::string &rStrEventID)= 0;

    /**
     * Method to check if the handler is subscribed for the notification or not
     * @param[in] rStrDomain domain name
     * @return true if handler is subscribed for notification, false otherwise
     */
    virtual bool IsHandlerSubscribedForNotification
        (const std::string& rStrDomain) = 0;

    /**
     * Method to get the list of additional events for the MessageHandler to 
     * process
     * @param void
     * @return set of additional eventIds
     */
    virtual std::set<std::string> GetSupplimentaryEventsList() = 0;
};
} /* namespace ic_core */

#endif /* IMESSAGE_HANDLER_H */
