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

#ifdef ENABLE_ZMQ
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "zmq.h"
#include "CZMQClient.h"
#include "CIgniteLog.h"

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CZMQClient"

CZMQClient::CZMQClient(const std::string &strEngineUri, const int &nType)
    : m_strEngineUri(strEngineUri)
{
    m_pvoidContext = zmq_ctx_new();
    m_pvoidSocket = zmq_socket(m_pvoidContext, nType);
    int nOpt = 0;
    int nRetVal = zmq_setsockopt(m_pvoidSocket, ZMQ_LINGER, &nOpt, 
                                sizeof(nOpt));
    HCPLOG_D << "zmq_setsockopt retVal:" << nRetVal;
}

CZMQClient::~CZMQClient()
{
    if (NULL != m_pvoidSocket)
    {
        zmq_close(m_pvoidSocket);
        m_pvoidSocket = NULL;
    }
    if (NULL != m_pvoidContext)
    {
        zmq_term(m_pvoidContext);
        m_pvoidContext = NULL;
    }
}

void CZMQClient::ZMQClose()
{
    HCPLOG_METHOD();
    if (NULL != m_pvoidSocket)
    {
        zmq_close(m_pvoidSocket);
        m_pvoidSocket = NULL;
    }
    if (NULL != m_pvoidContext)
    {
        zmq_term(m_pvoidContext);
        m_pvoidContext = NULL;
    }
}

bool CZMQPushClient::Connect()
{
    return (zmq_connect(m_pvoidSocket, m_strEngineUri.c_str()) != -1);
}

CZMQPushClient::CZMQPushClient(const std::string &strEngineUri)
    : CZMQClient(strEngineUri, ZMQ_PUSH)
{
}

bool CZMQPushClient::SendMessage(const std::string &rstrMsg)
{
    HCPLOG_D << rstrMsg;
    zmq_msg_t zmqMsg;
    size_t size = rstrMsg.size() + 1;

    if (zmq_msg_init_size(&zmqMsg, size) != -1)
    {
        memcpy(zmq_msg_data(&zmqMsg), rstrMsg.c_str(), size);
        return (zmq_msg_send(&zmqMsg, m_pvoidSocket, 0) != -1);
    }
    else
    {
        HCPLOG_E << "Failed to allocate memory for message";
        return false;
    }
}

CZMQPullClient::CZMQPullClient(const std::string &rstrEngineUri)
    : CZMQClient(rstrEngineUri, ZMQ_PULL)
{
    zmq_bind(m_pvoidSocket, m_strEngineUri.c_str());
}

std::string CZMQPullClient::RecvMessage()
{
    HCPLOG_I << "Initiate receive ZMQ";
    int nRC;
    char *pchMsgContent;
    std::string strResult = "";
    zmq_msg_t strMsg;

    nRC = zmq_msg_init(&strMsg);
    if (zmq_msg_init(&strMsg) == -1)
    {
        HCPLOG_E << "Failed to allocate memory for message";
    }
    else
    {
        HCPLOG_D << "Wait to receive message on ZMQ";

        zmq_msg_recv(&strMsg, m_pvoidSocket, 0);
        pchMsgContent = (char *)zmq_msg_data(&strMsg);
        strResult = pchMsgContent;

        HCPLOG_D << strResult;
        zmq_msg_close(&strMsg);
    }
    return strResult;
}

CZMQPubClient::CZMQPubClient(const std::string &rstrEngineUri)
    : CZMQClient(rstrEngineUri, ZMQ_PUB)
{
    zmq_bind(m_pvoidSocket, m_strEngineUri.c_str());
}

bool CZMQPubClient::Publish(const std::string &rstrMsg)
{
    int nRet = 0;
    size_t size = rstrMsg.size() + 1;

    nRet = zmq_send(m_pvoidSocket, (void *)rstrMsg.c_str(), size, 0);
    if (nRet < 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}
#endif //#ifdef ENABLE_ZMQ
