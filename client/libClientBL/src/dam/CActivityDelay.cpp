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

#include "dam/CActivityDelay.h"
#include "CIgniteLog.h"
#include "CIgniteDateTime.h"
#include "CIgniteConfig.h"

namespace ic_bl 
{

//! Macro for calculating time difference
#define TIME_DIFFERNCE_IN_SECONDS(x,y)  (x-y)/1000

using std::string;

CActivityDelay::CActivityDelay(CTransportHandlerBase* pNextHandler) :
                               CTransportHandlerBase(pNextHandler)
{
    //read the config values
    ic_utils::Json:: Value jsonUploadEventConfigValue =
        ic_core::CIgniteConfig::GetInstance()->GetJsonValue
                                                    ("DAM.UploadEventConfig");
    if(jsonUploadEventConfigValue.size() == 0)
    {
        HCPLOG_I << "UploadEventConfig tag does not exist";
        m_mapEventConfigData.clear();
        return ;
    }

    for(unsigned i = 0; i < jsonUploadEventConfigValue.size(); i++)
    {
        ic_utils::Json:: Value jsonConfigVal = jsonUploadEventConfigValue[i];
        string strEventId = jsonConfigVal["eventID"].asString();
        int nTimeout = jsonConfigVal["timeoutSec"].asInt();
        m_mapEventConfigData.insert(std::pair <string, int>
                                                        (strEventId, nTimeout));
    }
}

CActivityDelay::~CActivityDelay()
{

}

void CActivityDelay::HandleEvent(ic_core::CEventWrapper* pEvent)
{
    HCPLOG_METHOD();
    long long  llCurrentTime = ic_utils::CIgniteDateTime::GetMonotonicTimeMs();
    std::string strEventID = pEvent->GetEventId();

    if(m_mapEventConfigData.find(strEventID) != m_mapEventConfigData.end())
    {
        std::pair<std::map<std::string, long long>::iterator, bool> ret;
        ret = m_mapDefervent.insert(std::pair <std::string,
                                        long long>(strEventID, llCurrentTime));
        if (ret.second == false)
        {
            //event exists just update the timestamp
            ret.first->second = llCurrentTime;
        }
    }

    #ifdef IC_UNIT_TEST
        //This check is required because in case of UT, m_pNextHandler is nullptr
        if(nullptr == m_pNextHandler)
        {
            return;
        }
    #endif

    //let us pass it to next phase
    m_pNextHandler->HandleEvent(pEvent);
}

/**
 * Computes defer time for uploadmanager
 * based on its specific timeouts from config
 */
int CActivityDelay::ComputeDeferUpload()
{
    HCPLOG_METHOD();
    long long llCurrentTime = ic_utils::CIgniteDateTime::GetMonotonicTimeMs();

    int nDefertime = 0;
    if(!m_mapDefervent.empty())
    {
        for (std::map<std::string, long long>::iterator defer_iter = 
             m_mapDefervent.begin(); defer_iter != m_mapDefervent.end();
             ++defer_iter)
        {
            if((TIME_DIFFERNCE_IN_SECONDS(llCurrentTime,
              defer_iter->second )) >=  m_mapEventConfigData[defer_iter->first])
            {
                continue;
            }
            else
            {
                //calculating the max defer time among the stored UI events
                if((m_mapEventConfigData[defer_iter->first] - 
                            TIME_DIFFERNCE_IN_SECONDS(llCurrentTime,
                            defer_iter->second)) > nDefertime)
                {
                    nDefertime = m_mapEventConfigData[defer_iter->first] - 
                                 TIME_DIFFERNCE_IN_SECONDS(llCurrentTime,
                                 defer_iter->second);
                }
            }
        }
    }
    return nDefertime;
}

} /* namespace ic_bl*/