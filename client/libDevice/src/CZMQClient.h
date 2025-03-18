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
* \file ZMQClient.h
*
* \brief This file has classes to communicate over ZMQ IPC
* config
*******************************************************************************
*/
#ifdef ENABLE_ZMQ
#ifndef ZMQ_CLIENT_H
#define ZMQ_CLIENT_H
#include <string>

/**
 * class CZMQClient provide interface methods to communicate using ZeroMQ IPC
 */
class CZMQClient
{
protected:
    /**
     * Member variable to store ZMQ URI string
     */
    std::string m_strEngineUri;

    /**
     * Member pointer variable to store ZMQ context
     */
    void *m_pvoidContext;

    /**
     * Member variable to store ZMQ socket
     */
    void *m_pvoidSocket;

    /**
     * Parameterized constructor
     * @param[in] rstrEngineUri ZMQ URI string
     * @param[in] nType type of ZMQ socket
     */
    CZMQClient(const std::string &rstrEngineUri, const int &nType);

public:
    /**
     * Virtual destructor
     */
    virtual ~CZMQClient() = 0;

    /**
     * This function closes ZMQ socket and terminates ZMQ socket context
     * @param void
     * @return void
     */
    void ZMQClose();
};

/**
 * class CZMQPushClient provide methods to push data over ZeroMQ IPC
 */
class CZMQPushClient : public CZMQClient
{
public:
    /**
     * Parameterized constructor
     * @param[in] rstrEngineUri ZMQ URI string
     */
    CZMQPushClient(const std::string &rstrEngineUri);

    /**
     * This function connects the client with server
     * @param void
     * @return True if connection successful, false otherwise
     */
    bool Connect();

    /**
     * This function sends message to the server
     * @param[in] rstrMsg to be sent
     * @return True if message sent successfully, false otherwise
     */
    bool SendMessage(const std::string &rstrMsg);
};

/**
 * class CZMQPullClient provide methods to pull data from ZeroMQ IPC
 */
class CZMQPullClient : public CZMQClient
{
public:
    /**
     * Parameterized constructor
     * @param[in] rstrEngineUri ZMQ URI string
     */
    CZMQPullClient(const std::string &rstrEngineUri);

    /**
     * This function receives message from server. This is blocking call
     * @param void
     * @return Received message
     */
    std::string RecvMessage();
};

/**
 * class CZMQPubClient provide methods to publish data over ZeroMQ IPC
 */
class CZMQPubClient : public CZMQClient
{
public:
    /**
     * Parameterized constructor
     * @param[in] rstrEngineUri ZMQ URI string
     */
    CZMQPubClient(const std::string &rstrEngineUri);

    /**
     * This function publishes data over ZMQ.
     * @param[in] rstrMsg Message string to be published
     * @return True if publish is successful, false otherwise
     */
    bool Publish(const std::string &rstrMsg);
};
#endif // #ifndef ZMQ_CLIENT_H
#endif // #ifdef ENABLE_ZMQ
