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

#include <signal.h>
#include "CIgniteConfig.h"
#include "CIgniteDateTime.h"
#include "CIgniteFileUtils.h"
#include "CIgniteLog.h"
#include "CIgniteClient.h"

#include "dam/CInsightEngine.h"
#include "dam/CEventReceiver.h"
#include "dam/CPeriodicEvents.h"
#include "dam/CInitialEvents.h"
#include "db/CDataBaseFacade.h"

//! Macro for CInsightEngine string
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CInsightEngine"

using ic_event::CIgniteEvent;
using std::string;
using std::endl;
using std::vector;
namespace ic_bl
{

CInsightEngine::CInsightEngine(ic_core::CMessageQueue* pPublisher,
        bool bDataAggregationStatus)
{
    m_bIsDataAggregationActive = bDataAggregationStatus;

    ic_core::CIgniteConfig* pConfig = ic_core::CIgniteConfig::GetInstance();
    
    // Check and create attachment folder if needed
    std::string strAttachPath = pConfig->GetString(
                                    "DAM.Upload.CurlSender.fileAttachmentPath");

    if (!strAttachPath.empty())
    {
        if (!ic_utils::CIgniteFileUtils::Exists(strAttachPath))
        {
            ic_utils::CIgniteFileUtils::MakeDirectory(strAttachPath);
        }
    }

    HCPLOG_D << "Starting Event Receiver...";
    m_pEvtReceiver = new CEventReceiver(pPublisher,
                                                (!m_bIsDataAggregationActive));

    HCPLOG_C << "Event receiver started!";

    m_bIsInitialEventsThreadLaunched = false;
    if (m_bIsDataAggregationActive)
    {
        CPeriodicEvents::StartPeriodicEvents();

        CInitialEvents::StartInitialEvents();
        m_bIsInitialEventsThreadLaunched = true;
    }
}

CInsightEngine::~CInsightEngine()
{
    if (m_pEvtReceiver)
    {
        delete(m_pEvtReceiver);
        m_pEvtReceiver = nullptr;
    }
    CPeriodicEvents::StopPeriodicEvents();
}

void CInsightEngine::SuspendDataAggregation()
{
    m_bIsDataAggregationActive = false;
    SuspendInsightEngine();
}

void CInsightEngine::ResumeDataAggregation()
{
    m_bIsDataAggregationActive = true;
    ResumeInsightEngine();
}

void CInsightEngine::SuspendInsightEngine()
{
    if (m_pEvtReceiver)
    {
        HCPLOG_W << " - suspending event receiver...";
        m_pEvtReceiver->SuspendReceiver();
    }
    CPeriodicEvents::StopPeriodicEvents();
    HCPLOG_T << "CInsightEngine is suspended.";
}

void CInsightEngine::ResumeInsightEngine()
{
    if (m_bIsDataAggregationActive)
    {
        if (m_pEvtReceiver)
        {
            HCPLOG_W << " - resuming event receiver...";
            m_pEvtReceiver->ResumeReceiver();
        }
        CPeriodicEvents::StartPeriodicEvents();

        if (!m_bIsInitialEventsThreadLaunched)
        {
            CInitialEvents::StartInitialEvents();
            m_bIsInitialEventsThreadLaunched = true;
        }
        HCPLOG_T << "CInsightEngine is resumed.";
    }
    else
    {
        HCPLOG_T << "event receiver is not active!";
    }
}

}/* namespace ic_bl */
