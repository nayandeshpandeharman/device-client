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
* \file: CIgniteEventSender.h
*
* \brief: This class creates event senders for different types of
          socket servers and sends the events to queue. 
*******************************************************************************
*/

#ifndef CIGNITE_EVENT_SENDER_H
#define CIGNITE_EVENT_SENDER_H

#include <queue>
#include <string>
#include "CIgniteEvent.h"
#include "CConcurrentQueue.h"

namespace ic_event 
{
/**
 * Enum of different socket types
 */
typedef enum 
{
    eIP_SOCKET, ///< IP socket
    eUNIX_SOCKET, ///< UNIX socket
    eINVALID ///< Invalid socket type
} SocketType;

/**
 * EventSender class creats events for different sockets and sents events to queue
 */
class CIgniteEventSender : public ic_event::IEventSender 
{
public:
    /**
     * EventSender default constructor
     */
    CIgniteEventSender();

    /**
     * Parameterized constructor with one parameter to create event sender for UNIX socket server
     * @param[in] strSocketName socket name (pathname)
     */
    CIgniteEventSender(std::string strSocketName);

    /**
     * Parameterized constructor with two parameters to create event sender for INET socket server
     * @param[in] strIpAddr IP Address of socket server
     * @param[in] nPort Port number of the server socket
     */
    CIgniteEventSender(std::string strIpAddr , int nPort);

    /**
    * Destructor
    */
    ~CIgniteEventSender();

    /**
     * @brief Method to send event to queue
     * @param[in] rstrSerializedEvent Event getting added to queue
     * @return 0 if event is sent successfully else -1
     */
    int Send(const std::string &rstrSerializedEvent);

    /**
     * Method to open the connection based on the type of socket used
     * @param void
     * @return void
     */
    void OpenConnectionBasedOnSocketType();

    /**
     * Method to add events to start-up queue
     * @param[in] rstrSerializedEvent Serialised event string to add to the startup queue
     * @return void
     */
    void AddEventToStartUpQueue(const std::string &rstrSerializedEvent);

    /**
     * @brief Method to get the type of socket
     * @param void
     * @return SocketType Socket type can be eIP_SOCKET, eUNIX_SOCKET or eINVALID
     */
    SocketType GetSocketType();

    /**
     * @brief Method to get IP address
     * @param void
     * @return IP Address of the socket
     */
    std::string GetIPAddress();

    /**
     * @brief Method to get the Port number
     * @param void
     * @return Port value of the socket
     */
    int GetPort();

    /**
     * @brief Method to get the socket path name
     * @param void
     * @return Name of the socket path
     */
    std::string GetSocketPath();

    #ifdef IC_UNIT_TEST
        friend class CIgniteEventSenderTest;

        /**
         * @brief Method to get messageId of the event
         * @param void
         * @return MessageId of the event
         */
        int GetMsqId()
        {
            return m_nMsqid;
        }

        /**
         * @brief Method to get the Startup events queue status
         * @param void
         * @return True if startup events are queued else false
         */
        bool GetQueueStartupEvents()
        {
            return m_bQueueStartupEvents;
        }
    #endif

private:
    //! Variable to store the Message queue ID
    int m_nMsqid;

    //! Variable to store the state of queued startup events
    bool m_bQueueStartupEvents;

    //! Variable to store the socket name with default value as empty string
    std::string m_strSocketName = "";

    //! Variable to store the IP address of the socket with default value as empty string
    std::string m_strIpAddr = "";

    //! Variable to store the socket type with default value as invalid
    SocketType m_eType= eINVALID;

    //! Variable to store the socket port number with default value as 0
    int m_nPort = 0;

    //! Variable to store the startup events
    ic_utils::CConcurrentQueue<std::string> m_queStartupEvents;
};

} /* namespace ic_event */

#endif /* CIGNITE_EVENT_SENDER_H */
