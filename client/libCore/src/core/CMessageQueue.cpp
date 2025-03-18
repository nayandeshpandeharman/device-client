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

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <unistd.h>
#include <netdb.h>
#include <cstring>
#include <cerrno>
#include <string.h>
#include "IOnOff.h"
#if defined __ANDROID__
#include <sys/stat.h>
#include <linux/in.h>
#if defined(INET_SOCKET)
#include <sys/endian.h>
#endif // INET_SOCKET
#endif
#include "CIgniteClient.h"
#include "CIgniteEventSender.h"
#include "CIgniteMessage.h"
#include "CIgniteLog.h"
#include "CMessageQueue.h"
#include "CIgniteConfig.h"

//! Macro for 'CMessageQueue' string
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CMessageQueue"

namespace ic_core
{
namespace 
{
//! Constant key for 'msg read error' integer value
static const int MSG_READ_ERROR = -1;

//! Constant key for 'def msg queue priority' integer value
static const int DEF_MSG_QUEUE_PRIORITY = 11;

//! Constant key for 'DAM.MsgQueueThreadPriority' string
static const std::string MSG_QUEUE_PRIORITY = "DAM.MsgQueueThreadPriority";

//! Constant key for 'shutDownInitiated' string
static const std::string EVENTID_SHUT_DOWN_INITIATED  = "shutDownInitiated";

class Observer 
{
public:
    ic_event::CMessageTypes type;
    IMessageReceiver* handler;
};

inline bool is_queue_valid(int qid)
{
    return (qid >= 0);
}

int read_from_socket(int nSd, void* pvoidReadTo, int nSizeToRead)
{
    HCPLOG_T << "nSd :" << nSd << "readTo :" << pvoidReadTo 
             << "sizeToRead :" << nSizeToRead;

    int nReadSize = 0;

    if (nSd >= 0 && pvoidReadTo && nSizeToRead > 0)
    {
        int nValRead = 0;
        char* pchReadPtr = (char*)pvoidReadTo;

        while (nReadSize < nSizeToRead)
        {
            nValRead = read(nSd , pchReadPtr, nSizeToRead - nReadSize);
            if (nValRead > 0)
            {
                nReadSize += nValRead;
                pchReadPtr += nValRead;
            }
            else
            {
                if (!nValRead)
                {
                    HCPLOG_T << "read size zero on socket:" << nSd;
                }
                else
                {
                    HCPLOG_E << "Error reading on socket:" << nSd;
                }
                break;
            }
        }
    }
    else
    {
        nReadSize = -1;
        HCPLOG_E << "Invalid Input parameters";
    }

    HCPLOG_T << "read size from socket :" << nReadSize;

    return (nReadSize == nSizeToRead) ? nReadSize : MSG_READ_ERROR;
}

}

CMessageQueue::CMessageQueue(std::string strName)
{
    HCPLOG_D << "name=" << strName;

    if (strName.empty() || (strName[0] != '/'))
    {
        throw std::string("ic_event::MessageQueue::MessageQueue(std::string) ERROR : Name must be non-empty, and should start with '/' !");
    }

    m_strQueueName = strName;

#if (defined(__CYGWIN__) || defined(INET_SOCKET))
    HCPLOG_C << "Using INET socket type";
    std::string strServerIp = "127.0.0.1";
#if(defined(INET_SERVER_PORT))
    unsigned short uiServerPort = INET_SERVER_PORT;
#else /* INET_SERVER_PORT */
    unsigned short uiServerPort = 5150;
#endif /* INET_SERVER_PORT */

    m_masterSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#else /* __CYGWIN__ || INET_SOCKET */
    //  create a master socket
    HCPLOG_C << "Using UNIX socket type";
    m_masterSocket = socket(AF_UNIX , SOCK_STREAM , 0);
#endif /* __CYGWIN__ || INET_SOCKET */

    if (m_masterSocket == -1)
    {
        HCPLOG_E << "socket [" << strName << "] creation failed, error:" << errno;
        std::exit(-31);
    }

    HCPLOG_C << "master socket [" << strName << "] created ~ " << m_masterSocket;

    // set master socket to allow multiple connections
    int nOptval = 1;
    if (setsockopt(m_masterSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&nOptval, sizeof(nOptval)) == -1)
    {
        HCPLOG_E << "setsockopt failed, error:" << errno;
        std::exit(-32);
    }

    // type of socket created
#if (defined(__CYGWIN__) || defined(INET_SOCKET))
    struct sockaddr_in stAddr;
    memset(&stAddr, 0, sizeof(stAddr));
    stAddr.sin_family = AF_INET;
    stAddr.sin_port = htons(uiServerPort);
    stAddr.sin_addr.s_addr = inet_addr(strServerIp.c_str());
#else
    struct sockaddr_un stAddr;
    memset(&stAddr, 0, sizeof(stAddr));
    stAddr.sun_family = AF_UNIX;
    std::strncpy(stAddr.sun_path, m_strQueueName.c_str(), sizeof(stAddr.sun_path) - 1);
#endif

    // unlink the name first so that the bind won't fail.
    unlink(m_strQueueName.c_str());

    // bind the socket
    int nRes = bind(m_masterSocket, (struct sockaddr*)&stAddr, sizeof(stAddr));
    if (nRes < 0)
    {
#if (!defined(__CYGWIN__) && !defined(INET_SOCKET))
        HCPLOG_E << "bind failed.  Name=" << stAddr.sun_path << ", Result=" << nRes << ", errno=" << errno;
#else
        HCPLOG_E << "bind failed.  Name=" << strServerIp << ", Result=" << nRes << ", errno=" << errno;
#endif
        std::exit(-33);
    }

#if ( defined __ANDROID__ && !defined(INET_SOCKET))
    // make the socket accessible by anyone
    chmod(stAddr.sun_path, 0666);
#endif

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        m_clientSockets[i] = -1;
    }
}

CMessageQueue::~CMessageQueue()
{
    if (is_queue_valid(m_masterSocket))
    {
        // Delete the queue:
        unlink(m_strQueueName.c_str());
    }
}

int CMessageQueue::Subscribe(unsigned int unType, IMessageReceiver* pHandler)
{
    HCPLOG_D << "Subscribing " << unType;
    // Check list for matching items first:
    std::list<SubscriberListItem>::iterator iter;
    for (iter = m_listSubscriberList.begin(); 
         iter != m_listSubscriberList.end(); iter++)
    {
        if ((iter->m_unType == unType) && (iter->m_pHandler == pHandler) )
        {
            HCPLOG_C << "found match of type=" << unType 
                     << ", handler=" << pHandler;
            return -1;
        }
    }

    HCPLOG_C << "adding type~" << unType;
    SubscriberListItem item;
    item.m_unType = unType;
    item.m_pHandler = pHandler;
    m_listSubscriberList.push_back(item);

    return 0;
}

int CMessageQueue::Unsubscribe(unsigned int unType, IMessageReceiver* pHandler)
{
    HCPLOG_T << "Unsubscribing " << unType;
    // Check list and remove matching items.
    std::list<SubscriberListItem>::iterator iter;
    for (iter = m_listSubscriberList.begin(); 
         iter != m_listSubscriberList.end(); iter++)
    {
        if ( (iter->m_unType == unType) && (iter->m_pHandler == pHandler) )
        {
            HCPLOG_T << "unsubscribing type=" << unType << ", handler=" << pHandler;
            iter = m_listSubscriberList.erase(iter);
            return 0;
        }
    }
    HCPLOG_T << "no match found of type=" << unType << ", handler=" << pHandler;
    //HCPLOG_D << "returning error -1";
    return -1;
}

void CMessageQueue::Run()
{
    HCPLOG_I << "Started Consumer" ;
    int nThreadPriority = CIgniteConfig::GetInstance()->GetInt(MSG_QUEUE_PRIORITY,
                                                        DEF_MSG_QUEUE_PRIORITY);
    ic_utils::CIgniteThread::SetCurrentThreadPriority(nThreadPriority);

#if defined (__ANDROID__)
    int nfds = sysconf(_SC_OPEN_MAX);
#else
    int nfds = getdtablesize();       // Maximum file descriptor
#endif

    HCPLOG_C << "MaxFileDescrip.=" << nfds;

    // maximum of MAX_CLIENT pending connections for the master socket
    if (listen(m_masterSocket, MAX_CLIENTS) < 0)
    {
        HCPLOG_E << "failed to listen";
        std::exit(-34);
    }

    Connect();
}

void CMessageQueue::AddSockets(int &rnMaxSd, fd_set &rfds)
{
    int nSd;

    // add child sockets to set
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        // socket descriptor
        nSd = m_clientSockets[i];

        //if valid socket descriptor then add to read list
        if(nSd >= 0)
        {
            FD_SET(nSd , &rfds);
        }

        //highest file descriptor number, need it for the select function
        if(nSd > rnMaxSd)
        {
            rnMaxSd = nSd;
        }
    }
}

void CMessageQueue::AddNewIncomingConnection()
{
    int nNewSocket;

    if ((nNewSocket = accept(m_masterSocket, NULL, NULL)) < 0)
    {
        HCPLOG_E << "accept: error";
        std::exit(-35);
    }

    //inform user of socket number - used in send and receive commands
    HCPLOG_D << "New connection , socket fd is :" << nNewSocket;

    //add new socket to array of sockets
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        //if position is empty
        if (m_clientSockets[i] == -1)
        {
            m_clientSockets[i] = nNewSocket;
            HCPLOG_D << "Adding to list of sockets as :" << i;
            break;
        }
    }
}

void CMessageQueue::PerformIOOperation(fd_set &rfds, void** pvoidFullMsg)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        int nSd = m_clientSockets[i];

        //if socket-id is invalid OR descriptor is not set,
        //  continue with the next socket
        if (!((nSd >= 0) && FD_ISSET(nSd, &rfds)))
        {
            continue;
        }

        // continue with reading the header structure
        void* pvoidMsgHeader = *pvoidFullMsg;

        if (read_from_socket(nSd, pvoidMsgHeader, ic_event::CIgniteMessage::MSG_HEADER_SIZE) == MSG_READ_ERROR)
        {
            HCPLOG_E << "No Header data on socket: " << nSd;
            close(nSd);
            m_clientSockets[i] = -1;
        }
        else
        {
            // Read Actual Message
            int nMsgLen = ic_event::CIgniteMessage::GetMessageSize(pvoidMsgHeader);
            HCPLOG_I << "actual message Length: " << nMsgLen;

            if (nMsgLen >= 0 && (unsigned int)nMsgLen <=  ic_event::CIgniteMessage::MAX_MESSAGE_LENGTH)
            {

                if (!ReadMsgAndSend(*pvoidFullMsg, nMsgLen, nSd))
                {
                    // close socket
                    HCPLOG_E << "actual message not read from socket: " << nSd;
                    close(nSd);
                    m_clientSockets[i] = -1;
                }
            }
        }
    }
}

bool CMessageQueue::ReadMsgAndSend(void *pvoidFullMsg, int &rnMsgLen, int &rnSocketId)
{
    bool status = false;

    int nReadSize = 0;

    if (rnMsgLen > 0)
    {
        char* actualMsg = (char*)(pvoidFullMsg) + ic_event::CIgniteMessage::MSG_HEADER_SIZE;
        nReadSize = read_from_socket(rnSocketId, actualMsg, rnMsgLen);
    }
    // else it is message with only header with msgLen 0

    if (nReadSize == rnMsgLen)
    {
        // construct message from serialized data
        ic_event::CIgniteMessage rcvmsg((void*)pvoidFullMsg);
        rcvmsg.SetRecipient(rnSocketId);

        HCPLOG_LINE() << "-Received message: ";
        HCPLOG_LINE() << "  -Type: " << rcvmsg.GetType();
        HCPLOG_LINE() << "  -Reply Required: " << rcvmsg.GetReplyRequired();
        HCPLOG_LINE() << "  -Length: " << rcvmsg.GetMessageAsString().length();
        HCPLOG_LINE() << "  -Data: " << rcvmsg.GetMessageAsString();

        // Get observers, send message to observers.
        bool bFoundHandler = false;
        std::list<SubscriberListItem>::iterator iter;

        for (iter = m_listSubscriberList.begin(); iter != m_listSubscriberList.end(); iter++)
        {
            HCPLOG_T << "Looking for match. " << iter->m_unType;
            if (iter->m_unType == rcvmsg.GetType())
            {
                HCPLOG_T << "Match found - type : " << iter->m_pHandler;
                bFoundHandler = true;
                iter->m_pHandler->Handle(rcvmsg);
            }
        }

        if (!bFoundHandler)
        {
            HCPLOG_T << "Handler not found";
            if(rcvmsg.GetReplyRequired())
            {
                rcvmsg.Reply("");
            }
        }

        status = true;
    }

    return status;
}

void CMessageQueue::Connect()
{
    int nActivity, nMaxSd;
    fd_set fds;  // Read file descriptor set

    //accept the incoming connection
    HCPLOG_D << "Waiting for connections ...";

    void* pvoidFullMsg = malloc(ic_event::CIgniteMessage::MAX_SERIALIZED_SIZE);

    if (pvoidFullMsg)
    {
        CIgniteClient::GetOnOffMonitor()->RegisterForShutdownNotification(this,
                                                      IOnOff::eR_MESSAGE_QUEUE);
        while (!m_bIsShutDownInitiated)
        {
            //clear the socket set
            FD_ZERO(&fds);

            //add master socket to set
            FD_SET(m_masterSocket, &fds);
            nMaxSd = m_masterSocket;

            AddSockets(nMaxSd, fds);

            //wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
            nActivity = select( nMaxSd + 1 , &fds , NULL , NULL , NULL);

            if ((nActivity < 0) && (errno != EINTR))
            {
                HCPLOG_E << "socket select error";
            }

            //If something happened on the master socket , then its an incoming connection
            if (FD_ISSET(m_masterSocket, &fds))
            {
                AddNewIncomingConnection();
            }

            //else its some IO operation on some other socket :)
            PerformIOOperation(fds, &pvoidFullMsg);

            if (m_bIsShutDownInitiated) {
                HCPLOG_I<<"Shut down initiated, breaking from loop";
                break;
            }
        }
        free(pvoidFullMsg);
        pvoidFullMsg = NULL;
        CIgniteClient::GetOnOffMonitor()->ReadyForShutdown(IOnOff::eR_MESSAGE_QUEUE);
        CIgniteClient::GetOnOffMonitor()->UnregisterForShutdownNotification(IOnOff::eR_MESSAGE_QUEUE);
        Detach();
    }
    else
    {
        HCPLOG_E << "Memory allocation Error.";
    }
}

void CMessageQueue::NotifyShutdown() 
{
    m_bIsShutDownInitiated = true;
    ic_event::CIgniteEvent* pEvent =  new ic_event::CIgniteEvent("1.0", EVENTID_SHUT_DOWN_INITIATED);
    pEvent->Send();
    delete pEvent;
}
} /* namespace ic_core */
