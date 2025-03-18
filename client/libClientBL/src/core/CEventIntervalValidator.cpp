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

#include "CIgniteLog.h"
#include "CIgniteStringUtils.h"
#include "CEventIntervalValidator.h"
#include "CIgniteConfig.h"

//! Macro for 'CEventIntervalValidator' string
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CEventIntervalValidator"

namespace ic_bl
{
//! Constant key for 'DAM.Database.validateInterval' string
static const std::string KEY_EVENT_INTERVAL_VALIDATOR = 
                                                "DAM.Database.validateInterval";
//! Constant key for 'DAM.Database.IntervalList' string
static const std::string KEY_EVENT_INTERVAL_LIST = "DAM.Database.IntervalList";

CEventIntervalValidator::CEventIntervalValidator()
{
    m_mapEventInterval.clear();
    m_bValidateInterval = ic_core::CIgniteConfig::GetInstance()->GetBool
                                            (KEY_EVENT_INTERVAL_VALIDATOR,
                                            false);
    PopulateEventIntervalFromConfig();
}

CEventIntervalValidator* CEventIntervalValidator::GetInstance()
{
    static CEventIntervalValidator Instance;
    return &Instance;
}

void CEventIntervalValidator::PopulateEventIntervalFromConfig()
{
    ic_core::CIgniteConfig* pIgniteConfig = ic_core::CIgniteConfig::
                                                                GetInstance();
    ic_utils::Json::Value jsonIntervalList = pIgniteConfig->GetJsonValue(
                                                    KEY_EVENT_INTERVAL_LIST);

    /* Iterate over each list and populate map with it's interval
     * Value -1 indicates, it does not need any interval check and
     * can be uploaded as it came.
     */
    ic_utils::Json::ValueIterator jsonIterStart = jsonIntervalList.begin();
    ic_utils::Json::ValueIterator jsonIterEnd   = jsonIntervalList.end();

    ic_utils::CScopeLock scopeLock(m_IntervalMutex);
    for(; jsonIterStart != jsonIterEnd; jsonIterStart++)
    {
        std::string strEventId = jsonIterStart.key().asString();
        int nIntvl = (*jsonIterStart).asInt64();
        HCPLOG_I << strEventId << " with interval = " << nIntvl <<
                                                "  Configured  to upload";
        m_mapEventInterval[strEventId] = std::make_pair(nIntvl, 0);
    }
}

/* The method returns true
 *           if
 *              interval validation is set to false from configuration.
 *              event is not present in the configuration list.
 *              event is present in the configuration list and the value for
 *                     the event in configuration list is not equal to -1.
 *              event is present in configuration and timestamp difference
 *                    between successive same eventId events is greater than
 *                    or equal to configured interval.
 *         else return false.
 */
bool CEventIntervalValidator::IsValidInterval(std::string strEventId,
                                            long long llTimestamp)
{
    HCPLOG_D << strEventId;
    if(!m_bValidateInterval)
    {
        //Not set to check interval for events
        return true;
    }

    ic_utils::CScopeLock scopeLock(m_IntervalMutex);

    // If event not found, return true to upload as it occured
    std::map<std::string, std::pair<int, long long int> >::iterator iterEntry = 
                                            m_mapEventInterval.find(strEventId);
    
    if(iterEntry == m_mapEventInterval.end())
    {
        HCPLOG_D << "Not found in event interval map";
        //Entry not found to check for the interval, It can be uploaded as it is
        return true;
    }

    /* Here means need to check for interval
     * Get the stored timestamp. If it is 0, then set the timestamp and return
     */
    std::pair<int, long long int>& pairEntry = iterEntry->second;

    //event to be ignored
    if(pairEntry.first == -1)
    {
        HCPLOG_D << "Ignore event:"<<strEventId;
        return false;
    }
    else if(pairEntry.second == 0)
    {
        HCPLOG_D << "pairEntry value is zero";
        pairEntry.second = llTimestamp;
        return true;
    }
    else
    {
        /* do nothing */
    }

    HCPLOG_D << "validating inverval for "<<strEventId;

    // If it not zero, compare the 2 timestamp and diff should be >= interval;
    bool bRet = ValidateInterval(llTimestamp, pairEntry.second, pairEntry.first);

    /* If validation is true, means this event can be put as part of database 
     * to upload. Update the timestamp from the map entry to once again check
     * for the interval
     */
    if(bRet)
    {
        pairEntry.second = llTimestamp;
    }

    return bRet;
}

bool CEventIntervalValidator::ValidateInterval(long long llCurTimestamp, 
                                              long long llPrevTimestamp,
                                              int nInterval)
{
    // time_t object is being created from the epoch time in milisecs
    time_t currTime = llCurTimestamp;
    time_t prevTime = llPrevTimestamp;

    /* diff time always returns in miliisecs if the time_t object 
     * is just pointing to epoch milisecs.
     */
    double  dblMilliSecDiff = difftime(currTime, prevTime);

    // If diff between 2 timestamp is less then the interval, return false
    if(dblMilliSecDiff < nInterval)
    {
        return false;
    }

    return true;
}

CEventIntervalValidator::~CEventIntervalValidator()
{
    //Do nothing
}

}
