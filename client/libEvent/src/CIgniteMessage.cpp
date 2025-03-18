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

#include "CIgniteMessage.h"
#include "CIgniteDateTime.h"
#include "CIgniteLog.h"
#include "CCrc32.h"

#if (defined (__CYGWIN__) || defined(INET_SOCKET))
#include <arpa/inet.h>
#endif
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <climits>
#include <sstream>

#if defined(__CYGWIN__)
#include <cygwin/in.h>
#include <netdb.h>
#endif

#include <netdb.h>
#include <netinet/in.h>

using std::malloc;
using std::free;
using std::memcpy;

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CIgniteMessage"

namespace ic_event 
{

namespace 
{
// Socket handshaking timeouts
#define CONNECT_TIMEOUT 5
#define READ_WRITE_TIMEOUT 10

pthread_mutex_t seqnumMutex = PTHREAD_MUTEX_INITIALIZER;
unsigned int g_unGlobalSeqnum = 0;

const unsigned char SYNC_BYTES_LEN = 8;
const unsigned char SYNC_BYTES[SYNC_BYTES_LEN] = { 0xFF, 0x61, 0x63, 0x70, 0x6d, 0x73, 0x67, 0xFE };
const unsigned int FLAGS_REPLY_REQUIRED_BITPOS = 0x00000001;

const int ACP_SOCKET_ERROR = -1;

/**
 * Data structure representing serialized message fileds
 */
typedef struct SerializedMessage
{
    char chSync[SYNC_BYTES_LEN];    ///< Length of bytes
    unsigned int unType;    ///< Type
    unsigned int unFlags;   ///< Flag
    unsigned int unSeqnum;  ///< Sequence number
    unsigned int unMessageLen;  ///< Message length
    unsigned char uchMessageData[1];    ///< Message data
} SerializedMessage;

/**
 * Enum of different socket operations
 */
typedef enum 
{ 
    eACP_SOCKET_READ,    ///< Socket read
    eACP_SOCKET_WRITE    ///< Socket write
} SocketOperation;

/**
 * Method to perform socket operation like read or write
 * @param nSd Socket ID
 * @param pvoidBufferPtr Buffer pointer holds the serialized message
 * @param nSizeToOperate Serialized message length 
 * @param eOp Enum indicating the operation type that is Read or Write
 * @param unMsgType Message type to be sent
 * @return Positive value if message length is non-zero and operation type is read/write else -1 
 */
int operate_on_socket(int nSd, void* pvoidBufferPtr, int nSizeToOperate, SocketOperation eOp, unsigned int unMsgType)
{
    HCPLOG_T << "socket:" << nSd << "; Oper: " << eOp << "; msgType: " << unMsgType <<
                                            "bufferPtr:" << pvoidBufferPtr << "; nSizeToOperate:" << nSizeToOperate;

    int nOperatedSize = 0;

    if (nSd && pvoidBufferPtr && nSizeToOperate > 0 && (eOp == eACP_SOCKET_READ || eOp == eACP_SOCKET_WRITE))
    {
        // timeout set for read/write operations - begin
        fd_set readWritefds;
        struct timeval stTimeout;
        stTimeout.tv_sec = READ_WRITE_TIMEOUT;
        stTimeout.tv_usec = 0;
        FD_ZERO(&readWritefds);
        FD_SET(nSd , &readWritefds);

        if (eOp == eACP_SOCKET_WRITE)
        {
            // for write operation
            if (select(nSd + 1, NULL, &readWritefds, NULL, &stTimeout ) < 1)
            {
                HCPLOG_T << "Timeout for write error:" << nSd;
                return ACP_SOCKET_ERROR;
            }
        }
        else
        {
            // for read operation
            if (select(nSd + 1, &readWritefds, NULL, NULL, &stTimeout ) < 1)
            {
                HCPLOG_T << "Timeout for read error:" << nSd;
                return ACP_SOCKET_ERROR;
            }
        }

        if (!FD_ISSET(nSd, &readWritefds))
        {
            return ACP_SOCKET_ERROR;
        }
        // timeout set for read/write operations - end

        int nLength = 0;
        char* pchOpPtr = (char*)pvoidBufferPtr;

        while (nOperatedSize < nSizeToOperate)
        {
            switch (eOp)
            {
            case eACP_SOCKET_READ:
                nLength = read(nSd, pchOpPtr, nSizeToOperate - nOperatedSize);
                break;
            case eACP_SOCKET_WRITE:
                nLength = write(nSd, pchOpPtr, nSizeToOperate - nOperatedSize);
                break;
            default:
                // unreachable
                break;
            }
            if (nLength > 0)
            {
                nOperatedSize += nLength;
                pchOpPtr += nLength;
            }
            else
            {
                if (!nLength)
                {
                    HCPLOG_W << "read/write size zero" << nSd;
                }
                else
                {
                    HCPLOG_W << "read/write error on socket:" << nSd << "; err=" << std::strerror(errno);
                }
                break;
            }
        }
    }
    else
    {
        nOperatedSize = -1;
        HCPLOG_E << "Invalid Input parameters";
    }

    return (nOperatedSize == nSizeToOperate) ? nOperatedSize : ACP_SOCKET_ERROR;
}

}

const unsigned int CIgniteMessage::MSG_HEADER_SIZE = sizeof(SerializedMessage);
const unsigned int CIgniteMessage::MAX_SERIALIZED_SIZE = CIgniteMessage::MAX_MESSAGE_LENGTH + sizeof(
            SerializedMessage);
std::string CIgniteMessage::m_strNoReplay = std::string("");


const sockid CIgniteMessage::OpenConnection() 
{
    HCPLOG_METHOD();

    int nSockFd = -1;

#if (defined (__CYGWIN__) || defined(INET_SOCKET))
    nSockFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
    nSockFd = socket(AF_UNIX, SOCK_STREAM, 0);
#endif
    if (nSockFd < 0) 
    {
        HCPLOG_E << "ERROR opening socket";
    } 
    else 
    {
        ConnectToServer(nSockFd);
    }

    return nSockFd;
}

const sockid CIgniteMessage::OpenConnection(const std::string strName) 
{
    HCPLOG_METHOD() << "name=" << strName;

    int nSockFd = -1;
    if (!strName.empty()) 
    {
        nSockFd = socket(AF_UNIX, SOCK_STREAM, 0);

        if (nSockFd < 0) 
        {
            HCPLOG_T << "ERROR opening socket";
        } 
        else 
        {
            ConnectToServer(nSockFd, strName);
        }
    }

    return nSockFd;
}


const sockid CIgniteMessage::OpenConnection(const std::string strIpAddr, const int nPort) {

    int nSockFd = -1;
    if (!strIpAddr.empty()) 
    {
        nSockFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        if (nSockFd < 0) 
        {
            HCPLOG_T << "ERROR opening socket";
        } 
        else 
        {
            ConnectToServer(nSockFd, strIpAddr, nPort);
        }
    }

    return nSockFd;
}


const void CIgniteMessage::CloseConnection(const sockid nConnId)
{
    HCPLOG_METHOD() << "Closing socket connection... connId: " << nConnId;
    close(nConnId);
}


CIgniteMessage::CIgniteMessage(const unsigned int unType, const sockid nTo)
{
    HCPLOG_METHOD() << ", this=" << this << ", type=" << unType << ", to=" << nTo;
    
    signal(SIGPIPE, SIG_IGN);

    if (unType > 0)
    {
        m_unType = unType;
    }
    else
    {
        throw std::string("ic_event::CIgniteMessage::Message(int) ERROR:  Message type must be a positive number to be compatible with System V IPC!");
    }

    m_nTo = nTo;
    m_strRplyto = "";
    m_bReplyReq = false;

    pthread_mutex_lock(&seqnumMutex);
    m_unSeqnum = ++g_unGlobalSeqnum;
    pthread_mutex_unlock(&seqnumMutex);

    m_puchMsg = 0;
    m_unLen = 0;
    HCPLOG_METHOD() << " - RETURNING";
}

CIgniteMessage::CIgniteMessage(const void* const pvoidSerializedMessage)
{
    HCPLOG_METHOD() << ", this=" << this;
    SerializedMessage* pRaw = (SerializedMessage*)pvoidSerializedMessage;
    
    signal(SIGPIPE, SIG_IGN);

    if (pRaw->unType > 0)
    {
        m_unType = pRaw->unType;
    }
    else
    {
        throw std::string("ic_event::CIgniteMessage::Message(void*) ERROR:  Message type must be a positive number to be compatible with System V IPC!");
    }

    m_nTo = -1;
    m_bReplyReq = ((pRaw->unFlags & FLAGS_REPLY_REQUIRED_BITPOS) == FLAGS_REPLY_REQUIRED_BITPOS);
    m_unSeqnum = pRaw->unSeqnum;
    m_unLen = pRaw->unMessageLen;
    if (m_unLen > 0)
    {
        m_puchMsg = new unsigned char[m_unLen];
        memcpy(m_puchMsg, pRaw->uchMessageData, m_unLen);
    }
    else
    {
        m_puchMsg = 0;
    }
    HCPLOG_I << "serialized msg seqnum:" << m_unSeqnum;
    HCPLOG_METHOD() << ", this=" << this << ", m_unLen=" << m_unLen << ", m_puchMsg=" << (void*)m_puchMsg;
}

CIgniteMessage::~CIgniteMessage()
{
    HCPLOG_METHOD() << ", this=" << this << ", m_unLen=" << m_unLen << ", m_puchMsg=" << (void*)m_puchMsg;
    if (m_puchMsg != 0)
    {
        delete [] m_puchMsg;
    }
}

// This method is used to check if connection is still alive before sending message
bool CIgniteMessage::IsConnected()
{
    bool bConnected = false;
    if (m_nTo >= 0)
    {
        int nValopt = 0;
        socklen_t lon = sizeof(int);
        getsockopt(m_nTo, SOL_SOCKET, SO_ERROR, (void*)(&nValopt), &lon);
        if (!nValopt)
        {
            bConnected = true;
        }
        else
        {
            HCPLOG_T << "IsConnected:Error in connection() " << nValopt << " - " << strerror(nValopt);
        }
    }
    return bConnected;
}

int CIgniteMessage::Send(std::string& rstrReply)
{
    if (!IsConnected())
    {
        return -1;
    }

    m_bReplyReq = (&rstrReply != &m_strNoReplay);
    HCPLOG_METHOD() << ", this=" << this << ", to=" << m_nTo << ", reply?=" << m_bReplyReq;

    int nRet = -1;
    int nLen = 0;
    void* pvoidRawmsg = Serialize(nLen);
    void* pvoidMsgWritePtr = pvoidRawmsg;

    if (nLen > 0)
    {
        if (operate_on_socket(m_nTo, pvoidMsgWritePtr, nLen, eACP_SOCKET_WRITE, m_unType) != ACP_SOCKET_ERROR)
        {
            HCPLOG_T << "Sent message successfully.";

            if (m_bReplyReq)
            {
                void* pvoidFullMsg = malloc(CIgniteMessage::MAX_SERIALIZED_SIZE);

                if (pvoidFullMsg)
                {
                    // Read Header Structure
                    void* pvoidMsgHeader = pvoidFullMsg;

                    HCPLOG_T << "Reading Header on socket: " << m_nTo;

                    if (operate_on_socket(m_nTo, pvoidMsgHeader, CIgniteMessage::MSG_HEADER_SIZE,
                                        eACP_SOCKET_READ, m_unType) != ACP_SOCKET_ERROR)
                    {
                        // Get Message Length
                        int nMsgLen = CIgniteMessage::GetMessageSize(pvoidMsgHeader);
                        HCPLOG_T << "actual message Length: " << nMsgLen;

                        // Read actual Message
                        if (nMsgLen >= 0 && (unsigned int)nMsgLen <=  CIgniteMessage::MAX_MESSAGE_LENGTH)
                        {
                            int nReadSize = 0;

                            if (nMsgLen > 0)
                            {
                                // Read data to buffer after header if nMsgLen is not zero
                                char* pchActualMsg = (char*)pvoidFullMsg + CIgniteMessage::MSG_HEADER_SIZE;
                                HCPLOG_T << "Reading actualMsg on socket: " << m_nTo;
                                nReadSize = operate_on_socket(m_nTo, pchActualMsg, nMsgLen, eACP_SOCKET_READ,m_unType);
                            }
                            // else it is message with only header

                            if (nReadSize == nMsgLen)
                            {
                                CIgniteMessage returnMsg(pvoidFullMsg);
                                if (returnMsg.m_unSeqnum == m_unSeqnum)
                                {
                                    nRet = 0;
                                    rstrReply = returnMsg.GetMessageAsString();
                                }
                                else
                                {
                                    HCPLOG_E << "Sequence number does not match! invalid reply.";
                                }
                            }
                            else
                            {
                                HCPLOG_T << "No Valid data was read from socket: " << m_nTo;
                            }
                        }
                    }
                    free(pvoidFullMsg);
                    pvoidFullMsg = NULL;
                }
                else
                {
                    HCPLOG_E << "Memory Allocation failure.";
                }
            }
            else
            {
                nRet = 0;
            }
        }
    }

    if (pvoidRawmsg != NULL)
    {
        free(pvoidRawmsg);
    }

    return nRet;
}

int CIgniteMessage::Reply(const std::string& rstrReplyMsg) const
{
    HCPLOG_METHOD() << ", this=" << this << ", replyMsg=" << rstrReplyMsg;

    int nRet = -1;
    if (m_bReplyReq && m_nTo >= 0)
    {
        CIgniteMessage reply(m_unType, m_nTo);
        reply.m_unSeqnum = m_unSeqnum;
        reply.SetMessage(rstrReplyMsg);
        nRet = reply.Send();
    }
    else
    {
        HCPLOG_E << " - Invalid Reply Address!";
    }

    return nRet;
}

int CIgniteMessage::Reply(const unsigned char* puchReplyMsg, const unsigned int unLen) const
{
    HCPLOG_METHOD() << ", this=" << this;

    int nRet = -1;
    if (m_bReplyReq && m_nTo >= 0)
    {
        CIgniteMessage reply(m_unType, m_nTo);
        reply.m_unSeqnum = m_unSeqnum;
        reply.SetMessage(puchReplyMsg, unLen);
        nRet = reply.Send();
    }
    else
    {
        HCPLOG_E << " - Invalid Reply Address!";
    }
    return nRet;
}

int CIgniteMessage::SetMessage(const std::string& rstrMsg)
{
    HCPLOG_METHOD() << ", this=" << this << ", type = " << m_unType << ", msg = " << rstrMsg;

    unsigned char* puchOrigMsg = m_puchMsg;

    if ( (rstrMsg.length() + 1) <= MAX_MESSAGE_LENGTH  )
    {
        if (rstrMsg.length() > 0)
        {
            m_unLen = rstrMsg.length() + 1;
            m_puchMsg = new unsigned char[m_unLen];
            memcpy(m_puchMsg, rstrMsg.c_str(), m_unLen);
            m_puchMsg[m_unLen - 1] = '\0';
        }
        else
        {
            m_unLen = 0;
            m_puchMsg = 0;
        }
        if (puchOrigMsg != 0)
        {
            delete [] puchOrigMsg;
        }
        return 0;
    }
    else
    {
        HCPLOG_E << "CIgniteMessage::setMessage() length too long!  type=" << m_unType << ", len=" << rstrMsg.length() <<
                 ", msg=" << rstrMsg ;
        return -1;
    }
}

int CIgniteMessage::SetMessage(const unsigned char* puchMsg, const unsigned int unLen)
{
    unsigned char* puchOrigMsg = m_puchMsg;

    if ( unLen <= MAX_MESSAGE_LENGTH )
    {
        m_unLen = unLen;
        if (unLen > 0)
        {
            m_puchMsg = new unsigned char[m_unLen];
            memcpy(m_puchMsg, puchMsg, unLen);
        }
        else
        {
            m_puchMsg = 0;
        }
        if (puchOrigMsg != 0)
        {
            delete [] puchOrigMsg;
        }
        return 0;
    }
    else
    {
        HCPLOG_E << "CIgniteMessage::SetMessage() length too long!  type=" << m_unType << ", len=" << unLen << ", msg="
                 << puchMsg;
        return -1;
    }
}

int CIgniteMessage::SetRecipient(const sockid nTo)
{
    m_nTo = nTo;
    return 0;
}

const unsigned int CIgniteMessage::GetType() const
{
    return m_unType;
}

const bool CIgniteMessage::GetReplyRequired() const
{
    return m_bReplyReq;
}

const std::string CIgniteMessage::GetMessageAsString() const
{
    if (m_puchMsg != 0 )
    {
        return std::string((char*)m_puchMsg);
    }
    else
    {
        return std::string("");
    }
}

const unsigned char* CIgniteMessage::GetMessage(unsigned int& nLen) const
{
    nLen = m_unLen;
    return m_puchMsg;
}

void* CIgniteMessage::Serialize(int& rnLen) const
{
    HCPLOG_I << "MAXsize" << MAX_SERIALIZED_SIZE;

    rnLen = sizeof(SerializedMessage) + m_unLen * sizeof(char);

    SerializedMessage* pSerializedData = (SerializedMessage*) malloc(rnLen);
    memset(pSerializedData,0,rnLen);
    memcpy(pSerializedData->chSync, (const void*)SYNC_BYTES, SYNC_BYTES_LEN);
    pSerializedData->unType = m_unType;
    pSerializedData->unFlags = 0;

    if (m_bReplyReq)
    {
        pSerializedData->unFlags |= FLAGS_REPLY_REQUIRED_BITPOS;
    }

    pSerializedData->unMessageLen = m_unLen;
    pSerializedData->unSeqnum = m_unSeqnum;
    memcpy(pSerializedData->uchMessageData, m_puchMsg, m_unLen);

    HCPLOG_I << "msgsize" << rnLen ;

    return (void*) pSerializedData;
}

const int CIgniteMessage::GetMessageSize(const void* const pvoidSerializedMsg)
{
    int nMsgLen = -1;

    if (pvoidSerializedMsg)
    {
        SerializedMessage* pSerMsg = (SerializedMessage*)pvoidSerializedMsg;

        if (pSerMsg->unType <= 0)
        {
            HCPLOG_I << "invalid type" ;
            throw std::string("ic_event::CIgniteMessage::Message(void*) ERROR:  Message type must be a positive number to be compatible with System V IPC!");
        }

        // check sync bytes to know if it is valid message
        if (memcmp((void*)pSerMsg->chSync, (void*)SYNC_BYTES, SYNC_BYTES_LEN))
        {
            HCPLOG_E << "ERROR: This is not a valid message!";
        }
        else
        {
            nMsgLen = pSerMsg->unMessageLen;
            HCPLOG_I << "msgLen :" << nMsgLen ;
        }
    }
    else
    {
        HCPLOG_E << "ERROR: Null pointer input!";
    }

    return nMsgLen;
}

void CIgniteMessage::ConnectToServer(sockid& rnSockfd) 
{
        HCPLOG_METHOD();

#if (defined (__CYGWIN__) || defined(INET_SOCKET))
        struct sockaddr_in stAddr;
    stAddr.sin_family = AF_INET;
    std::string strServerIp = "127.0.0.1";
    unsigned short uiServerPort = 5150;
    stAddr.sin_port = htons(uiServerPort);
    stAddr.sin_addr.s_addr = inet_addr(strServerIp.c_str());
#else

        struct sockaddr_un stAddr;
        memset(&stAddr, 0, sizeof(stAddr));
        stAddr.sun_family = AF_UNIX;
        strncpy(stAddr.sun_path, HCP_MSGQUEUE_NAME, sizeof(stAddr.sun_path) - 1);
#endif

        int nRes, nValopt = 0;
        long lArg;
        fd_set myset;
        struct timeval stTv;
        socklen_t lon;

        // Set non-blocking
        lArg = fcntl(rnSockfd, F_GETFL, NULL);
        lArg |= O_NONBLOCK;
        fcntl(rnSockfd, F_SETFL, lArg);

        // Trying to connect with timeout
        nRes = connect(rnSockfd, (struct sockaddr *) &stAddr, sizeof(stAddr));
        if (nRes < 0) 
        {
            bool bConnected = false;
            if (errno == EINPROGRESS) 
            {
                stTv.tv_sec = CONNECT_TIMEOUT;
                stTv.tv_usec = 0;
                FD_ZERO(&myset);
                FD_SET(rnSockfd, &myset);
                if (select(rnSockfd + 1, NULL, &myset, NULL, &stTv) > 0) 
                {
                    lon = sizeof(int);
                    getsockopt(rnSockfd, SOL_SOCKET, SO_ERROR, (void *) (&nValopt), &lon);
                    if (nValopt) 
                    {
                        HCPLOG_T << "Error in connection... socket-id=" << rnSockfd << "; " << nValopt
                                 << " - " << strerror(nValopt);
                    } 
                    else 
                    {
                        HCPLOG_T << "Connected to Server Successfully. socket-id=" << rnSockfd;
                        bConnected = true;
                    }
                } 
                else 
                {
                    HCPLOG_E << "Timeout or error() in connecting to socket(socket-id=" << rnSockfd
                             << "); " << nValopt << " - " << strerror(nValopt);
                }
            } 
            else 
            {
                HCPLOG_T << "Error connecting to socket(socket-id=" << rnSockfd << "); Err#" << errno
                         << "; " << strerror(errno);
            }
            if (!bConnected) 
            {
                close(rnSockfd);
                rnSockfd = -1;
            }
        } 
        else 
        {
            HCPLOG_I << "Connected to Server. socket-id=" << rnSockfd;
        }
        if (rnSockfd > 0) 
        {
            // Set to blocking mode again...
            lArg = fcntl(rnSockfd, F_GETFL, NULL);
            lArg &= (~O_NONBLOCK);
            fcntl(rnSockfd, F_SETFL, lArg);
        }
    } //connect ends

    void CIgniteMessage::ConnectToServer(sockid& rnSockfd, std::string strSocketName) 
    {
        HCPLOG_METHOD();

        struct sockaddr_un stAddr;
        memset(&stAddr, 0, sizeof(stAddr));
        stAddr.sun_family = AF_UNIX;
        strncpy(stAddr.sun_path, strSocketName.c_str(), sizeof(stAddr.sun_path) - 1);

        int nRes, nValopt = 0;
        long lArg;
        fd_set myset;
        struct timeval stTv;
        socklen_t lon;

        // Set non-blocking
        lArg = fcntl(rnSockfd, F_GETFL, NULL);
        lArg |= O_NONBLOCK;
        fcntl(rnSockfd, F_SETFL, lArg);

        // Trying to connect with timeout
        nRes = connect(rnSockfd, (struct sockaddr *) &stAddr, sizeof(stAddr));
        if (nRes < 0) 
        {
            bool bConnected = false;
            if (errno == EINPROGRESS) 
            {
                stTv.tv_sec = CONNECT_TIMEOUT;
                stTv.tv_usec = 0;
                FD_ZERO(&myset);
                FD_SET(rnSockfd, &myset);
                if (select(rnSockfd + 1, NULL, &myset, NULL, &stTv) > 0) 
                {
                    lon = sizeof(int);
                    getsockopt(rnSockfd, SOL_SOCKET, SO_ERROR, (void *) (&nValopt), &lon);
                    if (nValopt) 
                    {
                        HCPLOG_T << "Error in connection... socket-id=" << rnSockfd << "; " << nValopt
                                 << " - " << strerror(nValopt);
                    } 
                    else 
                    {
                        HCPLOG_T << "Connected to Server Successfully. socket-id=" << rnSockfd;
                        bConnected = true;
                    }
                } 
                else 
                {
                    HCPLOG_E << "Timeout or error() in connecting to socket(socket-id=" << rnSockfd
                             << "); " << nValopt << " - " << strerror(nValopt);
                }
            } 
            else 
            {
                HCPLOG_T << "Error connecting to socket(socket-id=" << rnSockfd << "); Err#" << errno
                         << "; " << strerror(errno);
            }
            if (!bConnected) 
            {
                close(rnSockfd);
                rnSockfd = -1;
            }
        } 
        else 
        {
            HCPLOG_I << "Connected to Server. socket-id=" << rnSockfd;
        }
        if (rnSockfd > 0) 
        {
            // Set to blocking mode again...
            lArg = fcntl(rnSockfd, F_GETFL, NULL);
            lArg &= (~O_NONBLOCK);
            fcntl(rnSockfd, F_SETFL, lArg);
        }
    } //connect ends

    void CIgniteMessage::ConnectToServer(sockid &rnSockfd, std::string strIpAddr, int nPort) 
    {
        HCPLOG_METHOD();

        struct sockaddr_in stAddr;
        stAddr.sin_family = AF_INET;
        struct hostent *host;
        std::string strServerIp = strIpAddr;
        unsigned short uiServerPort = nPort;
        host = gethostbyname(strServerIp.c_str());
        stAddr.sin_port = htons(uiServerPort);
        stAddr.sin_addr = *((in_addr *) host->h_addr);

        int nRes, nValopt = 0;
        long lArg;
        fd_set myset;
        struct timeval stTv;
        socklen_t lon;

        // Set non-blocking
        lArg = fcntl(rnSockfd, F_GETFL, NULL);
        lArg |= O_NONBLOCK;
        fcntl(rnSockfd, F_SETFL, lArg);

        // Trying to connect with timeout
        nRes = connect(rnSockfd, (struct sockaddr *) &stAddr, sizeof(stAddr));
        if (nRes < 0) 
        {
            bool bConnected = false;
            if (errno == EINPROGRESS) 
            {
                stTv.tv_sec = CONNECT_TIMEOUT;
                stTv.tv_usec = 0;
                FD_ZERO(&myset);
                FD_SET(rnSockfd, &myset);
                if (select(rnSockfd + 1, NULL, &myset, NULL, &stTv) > 0) 
                {
                    lon = sizeof(int);
                    getsockopt(rnSockfd, SOL_SOCKET, SO_ERROR, (void *) (&nValopt), &lon);
                    if (nValopt) 
                    {
                        HCPLOG_T << "Error in connection... socket-id=" << rnSockfd << "; " << nValopt
                                 << " - " << strerror(nValopt);
                    } 
                    else 
                    {
                        HCPLOG_T << "Connected to Server Successfully. socket-id=" << rnSockfd;
                        bConnected = true;
                    }
                } 
                else 
                {
                    HCPLOG_E << "Timeout or error() in connecting to socket(socket-id=" << rnSockfd
                             << "); " << nValopt << " - " << strerror(nValopt);
                }
            } 
            else 
            {
                HCPLOG_T << "Error connecting to socket(socket-id=" << rnSockfd << "); Err#" << errno
                         << "; " << strerror(errno);
            }
            if (!bConnected) 
            {
                close(rnSockfd);
                rnSockfd = -1;
            }
        } 
        else 
        {
            HCPLOG_I << "Connected to Server. socket-id=" << rnSockfd;
        }
        if (rnSockfd > 0) 
        {
            // Set to blocking mode again...
            lArg = fcntl(rnSockfd, F_GETFL, NULL);
            lArg &= (~O_NONBLOCK);
            fcntl(rnSockfd, F_SETFL, lArg);
        }
    } //connect ends


} /* namespace ic_event */