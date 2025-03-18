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

#include <map>
#include <cstring>
#include "CIgniteConfig.h"
#include "CIgniteEvent.h"
#include "CIgniteLog.h"
#include "CIgniteClient.h"
#include "CIgniteStringUtils.h"
#include "dam/CPeriodicEvents.h"
#include "upload/CUploadController.h"
#include "config/CUploadMode.h"

using std::string;

namespace ic_bl
{
std::vector<CPeriodicEvents*> CPeriodicEvents::m_pvectThreads;

void CPeriodicEvents::StartPeriodicEvents()
{
    /* In order to support dynamic start/stop of PeriodicEvents use-case, 
     * making sure that the previously started threads are stopped before 
     * starting them again.
     */
    StopPeriodicEvents();

    ic_utils::Json::Value jsonRoot = ic_core::CIgniteConfig::GetInstance()->
                                                        GetJsonValue("DAM");
    if (jsonRoot.isNull())
    {
        HCPLOG_W << "URLs not available for DAM config!";
    }

    // Set up Periodic Events
    int nNoOfPeriodicEvnt = 0;

    if (jsonRoot.isMember("PeriodicEvents") 
        && jsonRoot["PeriodicEvents"].isArray()
        && !jsonRoot["PeriodicEvents"].empty() )
    {
        for (unsigned int i = 0; i < jsonRoot["PeriodicEvents"].size(); i++)
        {
            ic_utils::Json::Value jsonPeriodicEvent = 
                                                jsonRoot["PeriodicEvents"][i];

            ProcessPeriodicEvents(jsonPeriodicEvent, nNoOfPeriodicEvnt);
        }
    }
    else
    {
        HCPLOG_D << "No PeriodicEvents found!";
    }

    HCPLOG_D << "Total number of periodic events groups started is " 
             << nNoOfPeriodicEvnt;
}

void CPeriodicEvents::ProcessPeriodicEvents(const ic_utils::Json::Value 
                                           &rjsonPeriodicEvent,
                                           int &rnNoOfPeriodicEvnt)
{
    std::vector<std::string> *pvectReqIDs = new std::vector<std::string>();

    if (rjsonPeriodicEvent.isMember("reqIDs") 
        && rjsonPeriodicEvent["reqIDs"].isArray()
        && rjsonPeriodicEvent.isMember("reqPeriod") 
        && rjsonPeriodicEvent.isMember("reqPriority")
        && rjsonPeriodicEvent["reqPeriod"].isNumeric() 
        && (0 < rjsonPeriodicEvent["reqPeriod"].asDouble()))
    {
        ic_utils::Json::Value jsonIds = rjsonPeriodicEvent["reqIDs"];
        for (unsigned int unIter = 0; unIter < jsonIds.size(); unIter++)
        {
            if (jsonIds[unIter].isString())
            {
                std::string strEventName = jsonIds[unIter].asString();
                pvectReqIDs->push_back(strEventName);
            }
        }
        int nPeriod = rjsonPeriodicEvent["reqPeriod"].asInt();
        int nPriority = rjsonPeriodicEvent["reqPriority"].asInt();

        CPeriodicEvents *pPEObj = new CPeriodicEvents(
                                               pvectReqIDs, nPeriod, nPriority);
        pPEObj->Start();
        rnNoOfPeriodicEvnt++;
        m_pvectThreads.push_back(pPEObj);
    }
    else
    {
        HCPLOG_E << "Periodic Events array contains bad contents!";
    }
}

void CPeriodicEvents::StopPeriodicEvents()
{
    // Makes sure all the threads are
    for (std::vector<CPeriodicEvents*>::iterator iter = m_pvectThreads.begin(); 
            iter != m_pvectThreads.end() && m_pvectThreads.size() > 0; iter ++)
    {
        HCPLOG_T << "Stopping Periodic threads !!";
        (*iter)->Stop();
    }
    for (std::vector<CPeriodicEvents*>::iterator iter = m_pvectThreads.begin(); 
            iter != m_pvectThreads.end() && m_pvectThreads.size() > 0; iter ++)
    {
        HCPLOG_T << "Waiting for periodic threads cleanup !!";
        (*iter)->Join();
        delete((*iter));
    }
    m_pvectThreads.clear();
    HCPLOG_T << "All periodic threads are stopped!";
}

CPeriodicEvents::CPeriodicEvents(std::vector<std::string>* pVectReqIDs, 
                                unsigned int unPeriodInSecs, unsigned int 
                                unPriority) : m_unPeriodSec(unPeriodInSecs), 
                                m_pvectRequestIDs(pVectReqIDs),
                                m_unPriority(unPriority), m_bStopThread(false), 
                                m_bIsShutdownInitiated(false), 
                                m_strThreadName("PeriodicEvents")
{
}

CPeriodicEvents::~CPeriodicEvents()
{
    if (m_pvectRequestIDs)
    {
        m_pvectRequestIDs->clear();
        delete(m_pvectRequestIDs);
        m_pvectRequestIDs = NULL;
    }
}

int CPeriodicEvents::Stop()
{
    HCPLOG_METHOD();
    m_bStopThread = true;
    return 0;
}

void CPeriodicEvents::Run()
{
    HCPLOG_METHOD();

    // Set thread priority for this event group:
    SetCurrentThreadPriority(m_unPriority);

    m_strThreadName += std::to_string(m_unPeriodSec);
    ic_core::CIgniteClient::GetOnOffMonitor()->RegisterForShutdownNotification
                            (this, ic_core::IOnOff::eR_OTHER, m_strThreadName);

    while ((!m_bStopThread) && (!m_bIsShutdownInitiated))
    {
        for (unsigned int i = 0; i < m_pvectRequestIDs->size(); i++ )
        {
            std::string strReqIDs = (*m_pvectRequestIDs)[i];
            HCPLOG_T << "Generating Periodic Event of type " << strReqIDs;
            ic_core::CUploadMode *pMode = ic_core::CUploadMode::GetInstance();
            if (pMode->IsStreamModeSupported() && strReqIDs == "Alerts") 
            {
                CUploadController::GetInstance()->TriggerAlertsUpload
                                                    (START_ALERT_UPLOAD);
            }
            else 
            {
                ic_core::CIgniteClient::GetProductImpl()->GenerateEvent
                                                            (strReqIDs);
            }
        }

        HCPLOG_T << "Sleeping for " << m_unPeriodSec << " seconds.";
        if (!m_bStopThread) 
        {
            Wait(m_unPeriodSec);
        }
    }
    ic_core::CIgniteClient::GetOnOffMonitor()->ReadyForShutdown
            (ic_core::IOnOff::eR_OTHER, m_strThreadName);
    ic_core::CIgniteClient::GetOnOffMonitor()->
            UnregisterForShutdownNotification(ic_core::IOnOff::eR_OTHER,
                                              m_strThreadName);
    Detach();
}

void CPeriodicEvents::Wait(unsigned int unTimeOut) 
{
    std::unique_lock<std::mutex> lk(m_WaitMutex);
    std::chrono::milliseconds ms;
    m_WaitCondition.wait_for(lk,std::chrono::milliseconds(unTimeOut*1000));
}

void CPeriodicEvents::Notify() 
{
    m_WaitCondition.notify_all();
}

void CPeriodicEvents::NotifyShutdown() 
{
    HCPLOG_METHOD();
    m_bIsShutdownInitiated = true;
    Notify();
}
}/* namespace ic_bl */