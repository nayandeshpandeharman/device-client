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

#include <unistd.h>
#include "CIgniteLog.h"
#include "CInitialEvents.h"
#include "CIgniteConfig.h"
#include "CIgniteClient.h"

using namespace std;

//! Macro for CInitialEvents class
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CInitialEvents"

namespace ic_bl
{
void CInitialEvents::StartInitialEvents()
{
    ic_utils::Json::Value jsonRoot = ic_core::CIgniteConfig::GetInstance()->
            GetJsonValue("DAM");
    if (jsonRoot.isNull())
    {
        HCPLOG_W << "URLs not available for DAM config!";
    }

    if (jsonRoot.isMember("InitialEvents")&&jsonRoot["InitialEvents"].isArray()
            && !jsonRoot["InitialEvents"].empty())
    {
        for(unsigned int i = 0; i < jsonRoot["InitialEvents"].size(); i++)
        {
            ic_utils::Json::Value jsonInitialEvents = 
                    jsonRoot["InitialEvents"][i];
            ProcessInitialEventsConfig(jsonInitialEvents);
        }
    }
    else
    {
        HCPLOG_D << "No InitialEvents found!";
    }
}

void CInitialEvents::ProcessInitialEventsConfig(const ic_utils::Json::Value 
                                               &rjsonInitialEvents)
{
    std::vector<std::string>* pvectReqIDs = new std::vector<std::string>();

    if (rjsonInitialEvents.isMember("reqIDs") 
        && rjsonInitialEvents["reqIDs"].isArray()
        && rjsonInitialEvents.isMember("reqPeriod") 
        && rjsonInitialEvents.isMember("reqPriority")
        && rjsonInitialEvents["reqPeriod"].isNumeric() 
        && rjsonInitialEvents["reqPeriod"].asDouble() > 0)
    {
        ic_utils::Json::Value jsonIds = rjsonInitialEvents["reqIDs"];
        for(unsigned int i = 0; i < jsonIds.size(); i++)
        {
            if (jsonIds[i].isString())
            {
                std::string strEventName = jsonIds[i].asString();
                pvectReqIDs->push_back(strEventName);
            }
        }
        int nPeriod  = rjsonInitialEvents["reqPeriod"].asInt();
        int nPriority = rjsonInitialEvents["reqPriority"].asInt();

        CInitialEvents* pIe = new CInitialEvents(pvectReqIDs,nPeriod,nPriority);
        pIe->Start();
    }
    else
    {
        HCPLOG_E << "Initial Events array contains bad contents!";
    }
}

CInitialEvents::CInitialEvents(std::vector<std::string>* pvectReqIDs, unsigned
                                int unPeriodInSecs, unsigned int unPriority) : 
                                m_unPeriodSec(unPeriodInSecs), 
                                m_pvectRequestIDs(pvectReqIDs), 
                                m_unPriority(unPriority),
                                m_bIsShutdownInitiated(false), 
                                m_strThreadName("InitialEvents")
{
}

CInitialEvents::~CInitialEvents()
{
}

void CInitialEvents::Run()
{
    HCPLOG_METHOD();

    //Set thread priority for this event group:
    SetCurrentThreadPriority(m_unPriority);

    m_strThreadName += std::to_string(m_unPeriodSec);
    ic_core::CIgniteClient::GetOnOffMonitor()->RegisterForShutdownNotification
                (this, ic_core::IOnOff::eR_OTHER, m_strThreadName);

    while(!m_bIsShutdownInitiated)
    {
        if (m_pvectRequestIDs->size() > 0)
        {
            /* Since initial events are read during startup, let us sleep first
             * to give sometime for the domains to comes up.
             */
            Wait(m_unPeriodSec);
            unsigned int i = 0;
            while (i < m_pvectRequestIDs->size())
            {
                HCPLOG_T << "Generating Initial Event of type " << 
                            (*m_pvectRequestIDs)[i];
                
                /* If invoke request successful, remove the corresponding 
                 * initial event from the list.
                 */
                if (true == ic_core::CIgniteClient::GetProductImpl()->
                            GenerateEvent((*m_pvectRequestIDs)[i]))
                {
                    m_pvectRequestIDs->erase(m_pvectRequestIDs->begin() + i);
                }
                else
                {
                    //Read the next event
                    i++;
                }
            }
        }
        else
        {
            HCPLOG_T << "All the initial events are retrieved.";
            break;
        }
    }

    ic_core::CIgniteClient::GetOnOffMonitor()->ReadyForShutdown
            (ic_core::IOnOff::eR_OTHER, m_strThreadName);
    ic_core::CIgniteClient::GetOnOffMonitor()->
            UnregisterForShutdownNotification(ic_core::IOnOff::eR_OTHER, 
                                              m_strThreadName);
    Detach();
}

void CInitialEvents::Wait(unsigned int unTimeOut) 
{
    std::unique_lock<std::mutex> lk(m_WaitMutex);
    std::chrono::milliseconds ms;
    m_WaitCondition.wait_for(lk,std::chrono::milliseconds(unTimeOut*1000));
}

void CInitialEvents::Notify() 
{
    m_WaitCondition.notify_all();
}

void CInitialEvents::NotifyShutdown() 
{
    HCPLOG_METHOD();
    m_bIsShutdownInitiated = true;
    Notify();
}
}/* namespace ic_bl */
