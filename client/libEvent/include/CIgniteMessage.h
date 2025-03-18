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
* \file: CIgniteMessage.h
*
* \brief: This class deals with packing and unpacking of messages to Client
          in proper message format.
*******************************************************************************
*/

#ifndef CIGNITE_MESSAGE_H
#define CIGNITE_MESSAGE_H

#include "CMessageTypes.h"

#if defined SINGLE_APK
	#define HCP_MSGQUEUE_NAME "/data/data/com.harman.acp/cache/hcp_socket_main"
#else
	#ifndef HCP_MSGQUEUE_NAME
		#define HCP_MSGQUEUE_NAME "/tmp/hcp_socket_main"
	#endif
#endif

#include <string>
#include <cstdio>

namespace ic_event 
{

//! Defining int as sockid 
typedef int sockid;

/**
 * Message class is used to pack and unpack messages to Client in proper format 
 */
class CIgniteMessage 
{
public:
    //! Maximum length of a message is set as 2097152
    static const unsigned int MAX_MESSAGE_LENGTH = 2097152; // 2mb 158880

    //! Maximum length of a serialized message
    static const unsigned int MAX_SERIALIZED_SIZE;

    //! Message header size
    static const unsigned int MSG_HEADER_SIZE;

    /**
     * Static method to open UNIX/INET socket connection based on compile time configuration 
     * @param void
     * @return SocketID if connection is successful else -1
     */
    static const sockid OpenConnection();

    /**
     * Static method to open socket connection with UNIX socket server
     * @param[in] strName Socket name (pathname)
     * @return SocketID if connection is successful else -1
     */
    static const sockid OpenConnection(const std::string strName);

    /**
     * Static method to open socket connection with INET socket server
     * @param[in] strIpAddr IP Address of socket server
     * @param[in] nPort Port number of the server socket
     * @return SocketID if connection is successful else -1
     */
    static const sockid OpenConnection(const std::string strIpAddr, const int nPort);

    /**
     * Static method to close socket connection to ignite client
     * @param[in] nConnId Connection ID of the socket 
     * @return void
     */
    static const void CloseConnection(const sockid nConnId);

    /**
     * Parameterized constructor with two parameters
     * @param[in] nType Message type
     * @param[in] nTo SocketID to which the message is to be sent
     */
    CIgniteMessage(const unsigned int unType, const sockid nTo);

    /**
     * Parameterized constructor with one parameter
     * @param[in] pSerializedMessage Serialized message to be sent
     */
    CIgniteMessage(const void* const pvoidSerializedMessage);

    /**
     * Parameterized constructor with one parameter
     * @param[in] rMsg Message to be sent
     */
    CIgniteMessage(const CIgniteMessage& rMsg);

    /**
    * Destructor
    */
    virtual ~CIgniteMessage();

    /**
     * Method to send the messages
     * @param[in] rstrReply Message to be sent
     * @return 0 if sending message is successful else -1
     */
    int Send(std::string& rstrReply = m_strNoReplay);

    /**
     * Method to receive reply for a particular message
     * @param[in] rstrReplyMsg Message to which reply is expected
     * @return 0 if reply is received successfully else -1 
     */
    int Reply(const std::string& rstrReplyMsg) const;

    /**
     * Method to receive reply for a particular message and its length
     * @param[in] puchReplyMsg Message to which reply is expected
     * @param[in] unLen Length of message
     * @return 0 if reply is received successfully else -1  
     */
    int Reply(const unsigned char* puchReplyMsg, const unsigned int unLen) const;

    /**
     * Method to set a message based on the type of message to be sent  
     * @param[in] rstrMsg Message to be set
     * @return 0 if message is set successfuy else -1 
     */
    int SetMessage(const std::string& rstrMsg);

    /**
     * Method to set a message based on the type of message to be sent  and the length
     * @param[in] puchMsg Message to be set
     * @param[in] unLen Length of the message
     * @return 0 if message is set successfuy else -1 
     */
    int SetMessage(const unsigned char* puchMsg, const unsigned int unLen);

    /**
     * Methos to set recipient to receive message
     * @param[in] nTo SocketID to which the message is to be sent
     * @return 0 when successful 
     */
    int SetRecipient(const sockid nTo);

    /**
     * Method to get Connection ID of the socket
     * @param void
     * @return Connection ID of the socket 
     */
    const unsigned int GetType() const;

    /**
     * Method to get a required reply for the message
     * @param void
     * @return True if reply is received else false 
     */
    const bool GetReplyRequired() const;

    /**
     * @brief Method to get message in string format
     * @param void
     * @return if message then return message else empty string
     */
    const std::string GetMessageAsString() const;

    /**
     * Method to get message 
     * @param[in] nLen Length of message
     * @return Message
     */
    const unsigned char* GetMessage(unsigned int& runLen) const;

    /**
     * Method to get the message size
     * @param[in] pSerializedMsg Serialized message
     * @return Size of message 
     */
    static const int GetMessageSize(const void* const pvoidSerializedMsg);

private:
    /**
     * Method to check if connection is still alive before sending message
     * @param void
     * @return True if connection exists else false
     */
    bool IsConnected();

    /**
     * Method to serialize the message of given length
     * @param[in] rnLen Length of serialized message
     * @return void
     */
    void* Serialize(int& rnLen) const;

    //! Member variable to store NO_REPLY value
    static std::string m_strNoReplay;

    //! Member variable to store type of messge to be sent
    unsigned int m_unType;

    //! Member variable to store SocketID to which the message is to be sent
    sockid m_nTo;

    //! Member variable to store the serialized message sequence number
    unsigned int m_unSeqnum;

    //! Member variable to store the reply request value
    bool m_bReplyReq;

    //! Member variable to store the reply destination
    std::string m_strRplyto;

    //! Member variable to store the message
    unsigned char* m_puchMsg;

    //! Member variable to store the message length
    unsigned int m_unLen;

    /**
     * Static method to connect to UNIX/INET server based on compile time configuration
     * @param[in] rnSockfd File descriptor of socket created in function openConnection
     * @return void
     */
    static void ConnectToServer(sockid& rnSockfd);

    /**
     * Static method to connect to UNIX socket server with name socket_name
     * @param[in] rnSockfd File descriptor of socket created in function openConnection
     * @param[in] strSocketName Unix socket pathname
     * @return void
     */
    static void ConnectToServer(sockid& rnSockfd , const std::string strSocketName);

    /**
     * Static method to connect to INET socket sever with ip address and port
     * @param[in] rnSockfd File descriptor of socket created in function openConnection
     * @param[in] strIpAddr IP Address of socket server
     * @param[in] nPort Port number of the server socket
     * @return void
     */
    static void ConnectToServer(sockid& rnSockfd , const std::string strIpAddr, const int nPort);
};

} /* namespace ic_event */

#endif /* CIGNITE_MESSAGE_H */