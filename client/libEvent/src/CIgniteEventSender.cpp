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

#include "CIgniteEvent.h"
#include "CIgniteEventSender.h"
#include "CIgniteMessage.h"
#include "CIgniteLog.h"

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CIgniteEventSender"

namespace ic_event 
{
/*
 * Initializing startup event queue size as 20kb which
 *   approximately can hold 60 events with an assumption of
 *   300bytes per event.
 */
const unsigned long ulStartupEventQueueMaxSize = 20480;

CIgniteEventSender::CIgniteEventSender()
{
    m_nMsqid = -1;
    m_bQueueStartupEvents = true;
}

CIgniteEventSender::CIgniteEventSender(std::string strName) 
{
    m_nMsqid = -1;
    m_bQueueStartupEvents = true;
    m_eType = eUNIX_SOCKET;
    m_strSocketName = strName;
}

CIgniteEventSender::CIgniteEventSender(std::string strIpAddr , int nSocketPort) 
{
    m_nMsqid = -1;
    m_bQueueStartupEvents = true;
    m_eType = eIP_SOCKET;
    m_strIpAddr = strIpAddr;
    m_nPort = nSocketPort;
}

CIgniteEventSender::~CIgniteEventSender()
{
    ic_event::CIgniteMessage::CloseConnection(m_nMsqid);
}

int CIgniteEventSender::Send(const std::string &rstrSerializedEvent)
{
    HCPLOG_METHOD();

    if (m_nMsqid < 0)
    {
        OpenConnectionBasedOnSocketType();
    }

    int nRet = -1;
    if (m_nMsqid >= 0)
    {
        //clear the startup events queue before sending the incoming event        
        std::string strEvnt;
        while(m_queStartupEvents.Take(&strEvnt))
        {
            ic_event::CIgniteMessage event(ic_event::CMessageTypes::eEVENT, m_nMsqid);
            event.SetMessage(strEvnt);
            nRet = event.Send();
            HCPLOG_T << "Sending queued startup event=" << strEvnt << "; nRet = " << nRet;
            //Only startup events are allowed to be queued; since the startup events are already cleared here,
            // no need to queue anymore events even if the msq connection is broken for some reasons;
            // otherwise it will unnecessarily add overhead to the clients that are using the libAcpEvent library.
            m_bQueueStartupEvents = false;
        }

        ic_event::CIgniteMessage event(ic_event::CMessageTypes::eEVENT, m_nMsqid);
        event.SetMessage(rstrSerializedEvent);
        HCPLOG_T << "Sending Event Message to queue=" << m_nMsqid << ", data=" << rstrSerializedEvent;
        nRet = event.Send();
        if(nRet == -1)
        {
            ic_event::CIgniteMessage::CloseConnection(m_nMsqid);
            m_nMsqid = -1 ;
            HCPLOG_T << "Connection closed";
        }
    }
    else
    {
        if (m_bQueueStartupEvents)
        {
            AddEventToStartUpQueue(rstrSerializedEvent);
        }
        else
        {
            HCPLOG_E << "could not send the event " << rstrSerializedEvent;
        }
    }
    return nRet;
}

SocketType CIgniteEventSender::GetSocketType() 
{
    return m_eType;
}

std::string CIgniteEventSender::GetIPAddress() 
{
    return m_strIpAddr;
}

int CIgniteEventSender::GetPort() 
{
    return m_nPort;
}

std::string CIgniteEventSender::GetSocketPath() 
{
    return m_strSocketName;
}

void CIgniteEventSender::OpenConnectionBasedOnSocketType()
{
    if (m_eType == eUNIX_SOCKET) 
    {
        m_nMsqid = ic_event::CIgniteMessage::OpenConnection(m_strSocketName);
        HCPLOG_C << "UNIX Sock. opened[" << m_strSocketName << "]-id=" << m_nMsqid;
    } 
    else if(m_eType == eIP_SOCKET)
    {
        m_nMsqid = ic_event::CIgniteMessage::OpenConnection(m_strIpAddr,m_nPort);
        HCPLOG_C << "IP Sock. opened[" << m_strSocketName << "]-id=" << m_nMsqid;
    } 
    else 
    {
        m_nMsqid = ic_event::CIgniteMessage::OpenConnection();
        HCPLOG_C << "Sock. opened[" << m_strSocketName << "]-id=" << m_nMsqid;
    }
}

void CIgniteEventSender::AddEventToStartUpQueue(const std::string &rstrSerializedEvent)
{
    //if the startup events queue is full, let us ignore the remaining events.
    if ((m_queStartupEvents.Size() + rstrSerializedEvent.size()) <= ulStartupEventQueueMaxSize)
    {
        if(!m_queStartupEvents.Put (rstrSerializedEvent, rstrSerializedEvent.size())) 
        {
            HCPLOG_T << "Failed to Queue startup event " << rstrSerializedEvent;
        } 
        else 
        {
            HCPLOG_T << "Successdully Queued startup event " << rstrSerializedEvent;
        }
    }
    else
    {
        HCPLOG_E << "Startup event queue full! missing event " << rstrSerializedEvent;
    }
}

} /* namespace ic_event */
