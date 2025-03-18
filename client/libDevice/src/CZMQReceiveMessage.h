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
* \file CZMQReceiveMessage.h
*
* \brief CZMQReceiveMessage is wrapper ove ZMQPULLClient to recieve message
*******************************************************************************
*/

#ifdef ENABLE_ZMQ
#ifndef ZMQ_RECEIVE_MESSAGE_H
#define ZMQ_RECEIVE_MESSAGE_H

#include "CIgniteThread.h"
#include "CZMQClient.h"
#include "CIgniteClient.h"
#include "zmq.h"
#include "jsoncpp/json.h"
#include "IDeviceCommandHandler.h"
#include "IOnOffNotificationReceiver.h"


namespace ic_device
{

/**
 * class CZMQReceiveMessage provide methods to read data over ZeroMQ IPC
 * This class is a wrapper ove ZMQPULLClient to recieve message
 */
class CZMQReceiveMessage : public ic_utils::CIgniteThread, 
    public CZMQPullClient, public ic_core::IOnOffNotificationReceiver
{
public:
    /**
     * Singleton method to get the class instance
     * @param void
     * @return CZMQReceiveMessage class instance
     */
    static CZMQReceiveMessage *GetInstance();

    /**
     * Virtual Destructor
     */
    virtual ~CZMQReceiveMessage();

    /**
     * Overridding CIgniteThread::Run() method
     * @see CIgniteThread::Run()
     */
    virtual void Run() override;

    /**
     * This function listens over ZMQ channel
     * @param[in] Pointer to IDeviceCommandHandler
     * @return void
     */
    void StartListening(IDeviceCommandHandler *pCommandHandler);

    /**
     * This function cleans up and stops thread listening over ZMQ channel.
     * @param void
     * @return void
     */
    void StopListening(void);

    /**
     * Overridding IOnOffNotificationReceiver::notifyShutdown method
     * @see IOnOffNotificationReceiver::NotifyShutdown()
     */
    void NotifyShutdown();

private:
    /**
     * Parameterized constructor
     * @param[in] rstrEngineUri ZMQ URI string to read messages from.
     */
    CZMQReceiveMessage(const std::string &rstrEngineUri);

    /**
     * This function parses command payload
     * @param[in] strPayLoad Command payload string
     * @return void
     */
    void ProcessPayload(const std::string &rstrPayLoad);

    /**
     * Pointer to IDeviceCommandHandler class
     */
    IDeviceCommandHandler *m_pCommandHandler;

    /**
     * Member variable to indicate shutdown is initiated
     */
    bool m_bIsShutdownInitiated;

    /**
     * Pointer to CZMQPushClient class
     */
    CZMQPushClient *m_pZMQPushClient = NULL;
};
} //namespace
#endif // #ifndef ZMQ_RECEIVE_MESSAGE_H
#endif // #ifdef ENABLE_ZMQ
