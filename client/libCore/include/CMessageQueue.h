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
* \file CMessageQueue.h
*
* \brief This class/module handle message queue implementation
*******************************************************************************
*/

#ifndef CMESSAGE_QUEUE_H
#define CMESSAGE_QUEUE_H

#include <list>
#include "CIgniteMessage.h"
#include "CIgniteThread.h"
#include "IOnOffNotificationReceiver.h"

namespace ic_core 
{
/**
 * Interface class expose APIs for handling message.
 */
class IMessageReceiver
{
public:
    /**
     * Destructor
     */
    virtual ~IMessageReceiver() {};

    /**
     * Method to handle received message based on input parameter
     * @param[in] rMsg instance of message class
     * @return true if msg is successfully handled, false otherwise
     */
    virtual bool Handle(const ic_event::CIgniteMessage &rMsg) = 0;
};

/**
 * This class used to handle message queue functionality
 */
class CMessageQueue : public ic_utils::CIgniteThread , public IOnOffNotificationReceiver 
{
public:
    /**
     * Parameterized constructor
     * @param[in] strName string contain queue name
     */
    CMessageQueue(std::string strName);

    /**
     * Destructor
     */
    virtual ~CMessageQueue();

    /**
     * Overriding Method of Thread class
     * @see CIgniteThread::Run()
     */
    void Run() override;

    /**
     * Method to subscribe message handler based on input parameter
     * @param[in] unType Subscriber type
     * @param[in] pHandler Instance of handler
     * @return 0 on successfully subscribe of handler, non zero otherwise
     */
    int Subscribe(unsigned int unType, IMessageReceiver* pHandler);

    /**
     * Method to unsubscribe message handler based on input parameter
     * @param[in] unType Subscriber type
     * @param[in] pHandler Instance of handler
     * @return 0 on successfully unsubscribe of handler, non zero otherwise
     */
    int Unsubscribe(unsigned int unType, IMessageReceiver* pHandler);

    /**
     * Method to get queue name string
     * @param void
     * @return string contain queue name
     */
    std::string GetQueueName()
    {
        return m_strQueueName;
    }

    /**
     * Overriding Method of IOnOffNotificationReceiver class
     * @see IOnOffNotificationReceiver::NotifyShutdown()
     */
    void NotifyShutdown() override;

private:
    /**
     * Method to initiate connections with the message-queue sockets
     * to process the incoming messages.
     * @param none
     * @return void
     */
    void Connect();

    /**
     * Method to add child sockets to the list
     * @param[in] nMaxSd current max socket descriptor id
     * @param[in] rfds file descriptor set of the new socket
     * @return void
     */
    void AddSockets(int &nMaxSd, fd_set &rfds);

    /**
     * Method to add incoming child socket connection to the client sockets list
     * @param none
     * @return void
     */
    void AddNewIncomingConnection();

    /**
     * Method to perform IO operation on the given socket connection.
     * @param[in] rfds the file descriptor set of the socket on which to perform IO operation
     * @param[in] pvoidFullMsg void pointer to the memory allocated to process the message
     * @return void
     */
    void PerformIOOperation(fd_set &rfds, void** pvoidFullMsg);

    /**
     * Method to construct an Ignite message from the given raw message.
     * @param[in] pvoidFullMsg void pointer pointing to the raw message
     * @param[in] rnMsgLen message length
     * @param[in] rnSocketId socket-id
     * @return true if the message is successfully read and sent; false otherwise
     */
    bool ReadMsgAndSend(void *pvoidFullMsg, int &rnMsgLen, int &rnSocketId);

    //! Member variable to stores shutdown initiated status
    bool m_bIsShutDownInitiated = false;

    //! Member variable to stores queue name string
    std::string m_strQueueName;

    //! Member variable to stores master socket descriptor
    ic_event::sockid m_masterSocket;

    //! Constant variable to stores maximum clients 
    static const int MAX_CLIENTS = 30;

    //! Member variable to stores array of socket descriptor 
    ic_event::sockid m_clientSockets[MAX_CLIENTS];

    /**
     * This class handle subscriber list item
     */
    class SubscriberListItem 
    {
    public:
        //! Member variable to stores subscriber type
        unsigned int m_unType;

        //! Instance of IMessageReceiver class
        IMessageReceiver* m_pHandler;
    };

    //! Member variable to stores the list of subscriber
    std::list<SubscriberListItem> m_listSubscriberList;
};
} /* namespace ic_core */

#endif /* CMESSAGE_QUEUE_H */
