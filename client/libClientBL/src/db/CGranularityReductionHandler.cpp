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

#include <regex>
#include "CGranularityReductionHandler.h"
#include "CIgniteLog.h"
#include "CIgniteConfig.h"
#include "CIgniteStringUtils.h"
#include "CIgniteDateTime.h"
#include "CIgniteEvent.h"
#include "upload/CUploadController.h"
#include "db/CDataBaseFacade.h"
#include "config/CUploadMode.h"
#include "jsoncpp/json.h"

//! Macro for CGranularityReductionHandler string
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CGranularityReductionHandler"

namespace ic_bl 
{
namespace 
{
//! Constant key for 'freeStorageGainPercent' string
const std::string KEY_FREE_STORAGE_GAIN_PERCENT = "freeStorageGainPercent";

//! Constant key for 'reductionEventsType' string
const std::string KEY_REDUCTION_EVENTS_TYPE = "reductionEventsType";

//! Constant key for 'default free storage gain percent' numeric value
const uint8_t DEF_FREE_STORAGE_GAIN_PERCENT = 15;

//! Constant key for 'exemptedEvents' string
const std::string KEY_EXEMPTED_EVENTS = "exemptedEvents";

//! Constant key for 'policyOrder' string
const std::string KEY_POLICIES = "policyOrder";

//! Constant key for 'defaultPolicy' string
const std::string KEY_DEFAULT_POLICY = "defaultPolicy";

//! Constant key for 'RemoveFIFOEvents' string
const std::string KEY_DEF_POLICY_NAME = "RemoveFIFOEvents";

//! Constant key for 'default free storage gain percent' numeric value
const uint8_t DEF_POLICY_FIFO_PERCENT = 20;

//! Constant key for 'name' string
const std::string KEY_POLICY_NAME = "name";

//! Constant key for 'percent' string
const std::string KEY_DEF_POLICY_FIFO_PERCENT = "percent";

//! Constant key for 'exemptionOverride' string
const std::string KEY_EXEMPTION_OVERRIDE = "exemptionOverride";

//! Constant key for 'add' string
const std::string KEY_EXEMPTION_ADD = "add";

//! Constant key for 'remove' string
const std::string KEY_EXEMPTION_REMOVE = "remove";

//! Constant key for 'clear' string
const std::string KEY_EXEMPTION_CLEAR = "clear";

//! Constant key for 'IgniteClientLaunched' string
const std::string CLIENT_LAUNCHED_EVENT = "IgniteClientLaunched";

//! Constant key for 'dbSizeLimit' config string
const std::string KEY_DATABASE_LIMIT = "DAM.Database.dbSizeLimit";

//! Constant key for 'preTriggerEvent_IdSuffix' string
const std::string KEY_PRE_TRIGGER_EVENT_ID_SUFFIX = "preTriggerEvent_IdSuffix";

//! Constant key for 'postTriggerEvent_IdSuffix' string
const std::string KEY_POST_TRIGGER_EVENT_ID_SUFFIX = "postTriggerEvent_IdSuffix";

//! Constant key for 'BEF' string
const std::string DEF_PRE_TRIGGER_EVENT_ID_SUFFIX = "BEF";

//! Constant key for 'AFT' string
const std::string DEF_POST_TRIGGER_EVENT_ID_SUFFIX = "AFT";

//! Constant key for 'RemoveAlternateSimilarEvent' string
const std::string POLICY_REMOVE_ALTERNATE_SIMILAR_EVENT = 
                                                  "RemoveAlternateSimilarEvent";

//! Constant key for 'RemoveRepeatedTriggerEvents_LeaveFirstAndLast' string
const std::string POLICY_REMOVE_REPEATED_TRIGGER_EVENTS_LEAVE_FIRST_AND_LAST =
                                "RemoveRepeatedTriggerEvents_LeaveFirstAndLast";

//! Constant key for 'RemoveTriggerEventPostFiles' string
const std::string POLICY_REMOVE_TRIGGER_EVENT_POST_FILES = 
                                                  "RemoveTriggerEventPostFiles";

//! Constant key for 'dbEventQueryLimit' numeric value
const uint16_t DB_EVENT_QUERY_LIMIT = 1000;

//! Constant key for 'attachmentAvailableFlag' numeric value
const uint8_t ATTACHMENT_AVAILABLE_FLAG=1;
}

int CGranularityReductionHandler::GetEventRowCount()
{
    std::vector<std::string> vecProjection;
    vecProjection.push_back("COUNT(*)");
    ic_core::CCursor *pCurObj = ic_core::CDataBaseFacade::GetInstance()->Query(
                    ic_core::CDataBaseConst::TABLE_EVENT_STORE, vecProjection);

    int nRowCount = 0;
    if (pCurObj)
    {
        if (pCurObj->MoveToFirst())
        {
            nRowCount = pCurObj->GetInt(0);
        }
        delete pCurObj;
    }

    return nRowCount;
}

std::set<std::string> 
                CGranularityReductionHandler::PopulateUniqueSteadyStateEventIds(
                                     long long llStartTime, long long llEndTime,
                                     std::string strExempEventList)
{
    HCPLOG_METHOD() << "startTime=" << llStartTime << "; endTime=" << llEndTime
                    << "; exempEventList=" << strExempEventList;

    std::set<std::string> setUniqEvntIds;

    std::vector<std::string> vecProjection;
    vecProjection.push_back("DISTINCT " 
                             + ic_core::CDataBaseConst::COL_EVENT_ID);
    
    /* Selection of record time is changed to >= so that pulled records does not
     * skip records of startTimestamp for which GR not applied
     */
    std::string strSelection = "(" + ic_core::CDataBaseConst::COL_TIMESTAMP 
            + " >=" + ic_utils::CIgniteStringUtils::NumberToString(llStartTime);
    strSelection += " AND " + ic_core::CDataBaseConst::COL_TIMESTAMP + " <= " 
                      + ic_utils::CIgniteStringUtils::NumberToString(llEndTime);
    strSelection += ")";

    strSelection += " AND " + ic_core::CDataBaseConst::COL_EVENT_ID;
    strSelection += " NOT IN (" + strExempEventList;
    strSelection += ") AND " + ic_core::CDataBaseConst::COL_EVENT_ID;
    strSelection += "!=\"" + CLIENT_LAUNCHED_EVENT;
    strSelection += "\"";
    strSelection += " AND " + ic_core::CDataBaseConst::COL_HAS_ATTACH + "!=1";

    HCPLOG_T << "sql=" << strSelection;

    ic_core::CCursor *pCurObj = ic_core::CDataBaseFacade::GetInstance()->Query(
                                     ic_core::CDataBaseConst::TABLE_EVENT_STORE,
                                     vecProjection, strSelection);

    if (pCurObj)
    {
        HCPLOG_T << "retrieving items...";
        for (bool bWorking = pCurObj->MoveToFirst(); bWorking;
             bWorking = pCurObj->MoveToNext())
        {
            std::string strEId = pCurObj->GetString(pCurObj->GetColumnIndex(
                          "DISTINCT " + ic_core::CDataBaseConst::COL_EVENT_ID));
            setUniqEvntIds.insert(strEId);
            HCPLOG_T << "EVENTID=" << strEId;
        }

        delete pCurObj;
    }

    return setUniqEvntIds;
}

std::set<CGranularityReductionHandler::EventList>
                     CGranularityReductionHandler::PopulateSessionBasedEventList(
                                  std::string strEventId, long long llStartTime,
                                  long long llEndTime, long long llStartId,
                                  int nLimit)
{
    HCPLOG_METHOD() << "eventId=" << strEventId << "; startTime=" << llStartTime
                    << "; endTime=" << llEndTime << "; startId=" << llStartId;

    std::set<EventList> setEvntList;

    std::vector<std::string> vecProjection;
    vecProjection.push_back(ic_core::CDataBaseConst::COL_ID);
    vecProjection.push_back(ic_core::CDataBaseConst::COL_EVENT_ID);
    vecProjection.push_back(ic_core::CDataBaseConst::COL_TIMESTAMP);
    vecProjection.push_back(ic_core::CDataBaseConst::COL_HAS_ATTACH);
    vecProjection.push_back(ic_core::CDataBaseConst::COL_EVENTS);

    /* Selection of record time is changed to >= so that pulled records does not
     * skip records of startTimestamp for which GR not applied
     */
    std::string strSelection = "(" + ic_core::CDataBaseConst::COL_TIMESTAMP 
           + " >= " + ic_utils::CIgniteStringUtils::NumberToString(llStartTime);
    strSelection += " AND " + ic_core::CDataBaseConst::COL_TIMESTAMP + " <= " 
                      + ic_utils::CIgniteStringUtils::NumberToString(llEndTime);
    strSelection += ")";

    strSelection += " AND " + ic_core::CDataBaseConst::COL_ID;
    strSelection += " > " + ic_utils::CIgniteStringUtils::NumberToString(
                            llStartId);

    strSelection += " AND (" + ic_core::CDataBaseConst::COL_EVENT_ID;
    strSelection += "=\"" + strEventId + "\" OR ";
    strSelection += ic_core::CDataBaseConst::COL_EVENT_ID + "=";
    strSelection += "\"" + CLIENT_LAUNCHED_EVENT + "\")";

    HCPLOG_T << "sql=" << strSelection;

    std::vector<std::string> vecOrderBy;
    vecOrderBy.push_back(ic_core::CDataBaseConst::COL_ID + " ASC");

    ic_core::CCursor *pCurObj = ic_core::CDataBaseFacade::GetInstance()->Query(
                     ic_core::CDataBaseConst::TABLE_EVENT_STORE, vecProjection,
                     strSelection, vecOrderBy, nLimit);

    if (pCurObj)
    {
        HCPLOG_T << "retrieving items...";
        for (bool bWorking = pCurObj->MoveToFirst(); bWorking;
             bWorking = pCurObj->MoveToNext())
        {
            EventList stEvnt;
            stEvnt.llEventId = pCurObj->GetLong(pCurObj->GetColumnIndex(
                                              ic_core::CDataBaseConst::COL_ID));
            stEvnt.strEventId = pCurObj->GetString(pCurObj->GetColumnIndex(
                                        ic_core::CDataBaseConst::COL_EVENT_ID));
            stEvnt.llTimestamp = pCurObj->GetLong(pCurObj->GetColumnIndex(
                                       ic_core::CDataBaseConst::COL_TIMESTAMP));
            stEvnt.nHasAttach= pCurObj->GetInt(pCurObj->GetColumnIndex(
                                      ic_core::CDataBaseConst::COL_HAS_ATTACH));
            stEvnt.strEvents = pCurObj->GetString(pCurObj->GetColumnIndex(
                                          ic_core::CDataBaseConst::COL_EVENTS));
            setEvntList.insert(stEvnt);
        }

        delete pCurObj;
        pCurObj = NULL;
    }
    return setEvntList;
}

std::set<std::string> 
                  CGranularityReductionHandler::PopulateUniquePreTriggerEventIds(
                                     long long llStartTime, long long llEndTime,
                                     std::string strPreTrgrEvntIdSuffix,
                                     std::string strExempEventList)
{
    HCPLOG_METHOD() << "startTime=" << llStartTime << "; endTime=" << llEndTime
                    << "; exempEventList=" << strExempEventList;

    std::set<std::string> setUniqEvntIds;

    std::vector<std::string> vecProjection;
    vecProjection.push_back("DISTINCT " 
                             + ic_core::CDataBaseConst::COL_EVENT_ID);

    std::string strSelection = "(" + ic_core::CDataBaseConst::COL_TIMESTAMP 
            + " > " + ic_utils::CIgniteStringUtils::NumberToString(llStartTime);
    strSelection += " AND " + ic_core::CDataBaseConst::COL_TIMESTAMP + " <= " 
                      + ic_utils::CIgniteStringUtils::NumberToString(llEndTime);
    strSelection += ")";

    strSelection += " AND " + ic_core::CDataBaseConst::COL_EVENT_ID;
    strSelection += " NOT IN (" + strExempEventList + ")";
    strSelection += " AND " + ic_core::CDataBaseConst::COL_EVENT_ID;
    strSelection += "!=\"" + CLIENT_LAUNCHED_EVENT + "\"";
    strSelection += " AND " + ic_core::CDataBaseConst::COL_HAS_ATTACH + "=1";
    strSelection += " AND substr(" + ic_core::CDataBaseConst::COL_EVENT_ID 
                                   + ",-3) == \"";
    strSelection += strPreTrgrEvntIdSuffix + "\"";

    HCPLOG_T << "sql=" << strSelection;

    ic_core::CCursor *pCurObj = ic_core::CDataBaseFacade::GetInstance()->Query(
                                     ic_core::CDataBaseConst::TABLE_EVENT_STORE,
                                     vecProjection, strSelection);

    if (pCurObj)
    {
        HCPLOG_T << "retrieving items...";
        for (bool bWorking = pCurObj->MoveToFirst(); bWorking;
             bWorking = pCurObj->MoveToNext())
        {
            std::string strEId = pCurObj->GetString(pCurObj->GetColumnIndex(
                          "DISTINCT " + ic_core::CDataBaseConst::COL_EVENT_ID));
            setUniqEvntIds.insert(strEId);
            HCPLOG_T << "EVENTID=" << strEId;
        }

        delete pCurObj;
    }

    return setUniqEvntIds;
}
std::set<CGranularityReductionHandler::EventList> 
              CGranularityReductionHandler::PopulateSessionBasedTriggerEventList(
                                     std::string strTrigEventId,
                                     std::string strPostTrgrEvntIdSuffix,
                                     long long llStartTime, long long llEndTime,
                                     long long llStartId, int nLimit)
{
    HCPLOG_METHOD() << "trigEventId=" << strTrigEventId 
                    << "; startTime=" << llStartTime 
                    << "; endTime=" << llEndTime << "; startId=" << llStartId;

    std::set<EventList> setEvntList;

    std::vector<std::string> vecProjection;
    vecProjection.push_back(ic_core::CDataBaseConst::COL_ID);
    vecProjection.push_back(ic_core::CDataBaseConst::COL_EVENT_ID);
    vecProjection.push_back(ic_core::CDataBaseConst::COL_TIMESTAMP);
    vecProjection.push_back(ic_core::CDataBaseConst::COL_HAS_ATTACH);
    vecProjection.push_back(ic_core::CDataBaseConst::COL_EVENTS);

    std::string strSelection = "(" + ic_core::CDataBaseConst::COL_TIMESTAMP 
            + " > " + ic_utils::CIgniteStringUtils::NumberToString(llStartTime);
    strSelection += " AND " + ic_core::CDataBaseConst::COL_TIMESTAMP + " <= " 
                      + ic_utils::CIgniteStringUtils::NumberToString(llEndTime);
    strSelection += ")";

    strSelection += " AND (" + ic_core::CDataBaseConst::COL_ID;
    strSelection += " > " 
                + ic_utils::CIgniteStringUtils::NumberToString(llStartId) + ")";
    strSelection += " AND (((" + ic_core::CDataBaseConst::COL_EVENT_ID + "=\"" 
                               + strTrigEventId + "\"";
    strSelection += " OR " + ic_core::CDataBaseConst::COL_EVENT_ID + "=\"";
    strSelection += strTrigEventId.substr(0,
                        strTrigEventId.size() - strPostTrgrEvntIdSuffix.size());
    strSelection += strPostTrgrEvntIdSuffix + "\") ";

    strSelection += " AND " + ic_core::CDataBaseConst::COL_HAS_ATTACH + "=1)";
    strSelection += " OR " + ic_core::CDataBaseConst::COL_EVENT_ID + "=\"" 
                                                + CLIENT_LAUNCHED_EVENT + "\")";

    HCPLOG_T << "sql=" << strSelection;

    std::vector<std::string> vecOrderBy;
    vecOrderBy.push_back(ic_core::CDataBaseConst::COL_ID + " ASC");

    ic_core::CCursor *pCurObj = ic_core::CDataBaseFacade::GetInstance()->Query(
                     ic_core::CDataBaseConst::TABLE_EVENT_STORE, vecProjection,
                     strSelection, vecOrderBy, nLimit);

    if (pCurObj)
    {
        HCPLOG_T << "retrieving items...";
        for (bool bWorking = pCurObj->MoveToFirst(); bWorking;
             bWorking = pCurObj->MoveToNext())
        {
            EventList stEvnt;
            stEvnt.llEventId = pCurObj->GetLong(pCurObj->GetColumnIndex(
                                              ic_core::CDataBaseConst::COL_ID));
            stEvnt.strEventId = pCurObj->GetString(pCurObj->GetColumnIndex(
                                        ic_core::CDataBaseConst::COL_EVENT_ID));
            stEvnt.llTimestamp = pCurObj->GetLong(pCurObj->GetColumnIndex(
                                     ic_core::CDataBaseConst::COL_TIMESTAMP));
            stEvnt.nHasAttach= pCurObj->GetInt(pCurObj->GetColumnIndex(
                                      ic_core::CDataBaseConst::COL_HAS_ATTACH));
            stEvnt.strEvents = pCurObj->GetString(pCurObj->GetColumnIndex(
                                          ic_core::CDataBaseConst::COL_EVENTS));
            setEvntList.insert(stEvnt);
        }

        delete pCurObj;
    }
    return setEvntList;
}

std::set<CGranularityReductionHandler::EventList>
                      CGranularityReductionHandler::PopulatePostTriggerEventList(
                                     long long llStartTime, long long llEndTime,
                                     std::string strPostTrgrEvntIdSuffix,
                                     long long llStartId, int nLimit)
{
    HCPLOG_METHOD() << "startTime=" << llStartTime << "; endTime=" << llEndTime
                    << "; startId=" << llStartId;

    std::set<EventList> setEvntList;

    std::vector<std::string> vecProjection;
    vecProjection.push_back(ic_core::CDataBaseConst::COL_ID);
    vecProjection.push_back(ic_core::CDataBaseConst::COL_EVENTS);

    std::string strSelection = "(" + ic_core::CDataBaseConst::COL_TIMESTAMP 
            + " > " + ic_utils::CIgniteStringUtils::NumberToString(llStartTime);
    strSelection += " AND " + ic_core::CDataBaseConst::COL_TIMESTAMP + " <= " 
                      + ic_utils::CIgniteStringUtils::NumberToString(llEndTime);
    strSelection += ")";

    strSelection += " AND (" + ic_core::CDataBaseConst::COL_ID;
    strSelection += " > " 
                + ic_utils::CIgniteStringUtils::NumberToString(llStartId) + ")";
    strSelection += " AND substr(" + ic_core::CDataBaseConst::COL_EVENT_ID 
                                   + ",-";
    strSelection += ic_utils::CIgniteStringUtils::NumberToString(
                                       strPostTrgrEvntIdSuffix.size()) + ")=\"";
    strSelection += strPostTrgrEvntIdSuffix + "\"";
    strSelection += " AND " + ic_core::CDataBaseConst::COL_HAS_ATTACH + "=1";

    HCPLOG_T << "sql=" << strSelection;

    std::vector<std::string> vecOrderBy;
    vecOrderBy.push_back(ic_core::CDataBaseConst::COL_ID + " ASC");

    ic_core::CCursor *pCurObj = ic_core::CDataBaseFacade::GetInstance()->Query(
                     ic_core::CDataBaseConst::TABLE_EVENT_STORE, vecProjection,
                     strSelection, vecOrderBy, nLimit);

    if (pCurObj)
    {
        HCPLOG_T << "retrieving items...";
        for (bool bWorking = pCurObj->MoveToFirst(); bWorking;
             bWorking = pCurObj->MoveToNext())
        {
            EventList stEvnt;
            stEvnt.llEventId = pCurObj->GetLong(pCurObj->GetColumnIndex(
                                              ic_core::CDataBaseConst::COL_ID));
            stEvnt.strEvents = pCurObj->GetString(pCurObj->GetColumnIndex(
                                          ic_core::CDataBaseConst::COL_EVENTS));
            setEvntList.insert(stEvnt);
        }

        delete pCurObj;
    }
    return setEvntList;
}

CGranularityReductionHandler::CGranularityReductionHandler()
{
    m_bIsPolicyReadFromConfig = false;
    m_stLastGrRecord.Reset();
    m_strPreTrgrEventIdSuffix = "";
    m_strPostTrgrEventIdSuffix = "";
    m_nFreeStorageGainPercent = DEF_FREE_STORAGE_GAIN_PERCENT;
    m_strMandatoryExemptedEvents = "";
    m_eGReductionEventType = CUploadUtils::UploadDataMode::eBATCH;
}


CGranularityReductionHandler::~CGranularityReductionHandler()
{

}

void CGranularityReductionHandler::LoadMandatoryExemptedEvents()
{
    HCPLOG_METHOD();

    m_strMandatoryExemptedEvents = "";

    ic_core::CUploadMode *pMode = ic_core::CUploadMode::GetInstance();

    if (pMode->IsStreamModeSupported())
    {
        std::set<std::string> setStreamEventList = 
                                                pMode->GetStreamModeEventList();
        for (std::set<std::string>::iterator iter = setStreamEventList.begin();
             iter != setStreamEventList.end(); ++iter)
        {
            m_strMandatoryExemptedEvents += "\"" + *iter + "\",";
        }

        //remove the final "," char
        if (!m_strMandatoryExemptedEvents.empty()) 
        {
            m_strMandatoryExemptedEvents = m_strMandatoryExemptedEvents.substr(
                                       0,m_strMandatoryExemptedEvents.size()-1);
        }

        HCPLOG_T << "Mandatory exempted event list: " 
                 << m_strMandatoryExemptedEvents;
    }
}


bool CGranularityReductionHandler::ReadEventTypeToDeleteDuringGR(
                                        const ic_utils::Json::Value &rjsonGR)
{
    bool bStatus = false;

    if (rjsonGR.isMember(KEY_REDUCTION_EVENTS_TYPE) 
        && rjsonGR[KEY_REDUCTION_EVENTS_TYPE].isUInt())
    {
        unsigned int unType = rjsonGR[KEY_REDUCTION_EVENTS_TYPE].asUInt();

        //if the type is higher than max supported type, use default
        if ((unType > 
                ((unsigned int) CUploadUtils::UploadDataMode::eBATCH_AND_STREAM)))
        {
            m_eGReductionEventType = CUploadUtils::UploadDataMode::eBATCH;
        }
        else 
        {
            m_eGReductionEventType = (CUploadUtils::UploadDataMode)unType;
        }
        HCPLOG_C << "EventType to reduce: " << m_eGReductionEventType;

        bStatus = true;
    }

    return bStatus;
}

int CGranularityReductionHandler::ReadGRFreeStorageGainPercent(
                                        const ic_utils::Json::Value &rjsonGR)
{
    if (rjsonGR.isMember(KEY_FREE_STORAGE_GAIN_PERCENT) &&
        rjsonGR[KEY_FREE_STORAGE_GAIN_PERCENT].isInt())
    {
        m_nFreeStorageGainPercent = 
                                 rjsonGR[KEY_FREE_STORAGE_GAIN_PERCENT].asInt();
    }
    else
    {
        m_nFreeStorageGainPercent = DEF_FREE_STORAGE_GAIN_PERCENT;
        HCPLOG_T << KEY_FREE_STORAGE_GAIN_PERCENT
                 << " not found in config; use default " 
                 << m_nFreeStorageGainPercent;
    }

    return m_nFreeStorageGainPercent;
}

bool CGranularityReductionHandler::ReadGRExemptedEvents(
                                        const ic_utils::Json::Value &rjsonGR)
{
    bool bStatus = false;

    if (rjsonGR.isMember(KEY_EXEMPTED_EVENTS) 
        && rjsonGR[KEY_EXEMPTED_EVENTS].isArray())
    {
        ic_utils::Json::Value jsonExEvnts = rjsonGR[KEY_EXEMPTED_EVENTS];
        for (int nItr=0; nItr < jsonExEvnts.size(); nItr++)
        {
            m_setExemptedEvents.insert(jsonExEvnts[nItr].asString());
        }
        bStatus = true;
    }

    return bStatus;
}

std::string CGranularityReductionHandler::ReadGRPreTriggerEventIdSuffix(
                                        const ic_utils::Json::Value &rjsonGR)
{
    if (rjsonGR.isMember(KEY_PRE_TRIGGER_EVENT_ID_SUFFIX)
        && rjsonGR[KEY_PRE_TRIGGER_EVENT_ID_SUFFIX].isString())
    {
        m_strPreTrgrEventIdSuffix = 
                            rjsonGR[KEY_PRE_TRIGGER_EVENT_ID_SUFFIX].asString();
    }
    else
    {
        m_strPreTrgrEventIdSuffix = DEF_PRE_TRIGGER_EVENT_ID_SUFFIX;
        HCPLOG_T << KEY_PRE_TRIGGER_EVENT_ID_SUFFIX
                 << " not found in config; use default " 
                 << m_strPreTrgrEventIdSuffix;
    }

    return m_strPreTrgrEventIdSuffix;
}

std::string CGranularityReductionHandler::ReadGRPostTriggerEventIdSuffix(
                                        const ic_utils::Json::Value &rjsonGR)
{
    if (rjsonGR.isMember(KEY_POST_TRIGGER_EVENT_ID_SUFFIX) &&
            rjsonGR[KEY_POST_TRIGGER_EVENT_ID_SUFFIX].isString())
    {
        m_strPostTrgrEventIdSuffix = 
                           rjsonGR[KEY_POST_TRIGGER_EVENT_ID_SUFFIX].asString();
    }
    else
    {
        m_strPostTrgrEventIdSuffix = DEF_POST_TRIGGER_EVENT_ID_SUFFIX;
        HCPLOG_T << KEY_POST_TRIGGER_EVENT_ID_SUFFIX
                 << " not found in config; use default " 
                 << m_strPostTrgrEventIdSuffix;
    }

    return m_strPostTrgrEventIdSuffix;
}

bool CGranularityReductionHandler::ReadDefaultGRPolicy(
                                        const ic_utils::Json::Value &rjsonGR)
{
    bool bStatus = false;

    if (rjsonGR.isMember(KEY_DEFAULT_POLICY) 
        && rjsonGR[KEY_DEFAULT_POLICY].isObject())
    {
        ic_utils::Json::Value jsonDPolicy = rjsonGR[KEY_DEFAULT_POLICY];

        if (jsonDPolicy.isMember(KEY_POLICY_NAME) 
            && jsonDPolicy[KEY_POLICY_NAME].isString())
        {
            m_stDefPolicy.strName = jsonDPolicy[KEY_POLICY_NAME].asString();
        }

        if (jsonDPolicy.isMember(KEY_DEF_POLICY_FIFO_PERCENT) &&
                jsonDPolicy[KEY_DEF_POLICY_FIFO_PERCENT].isInt())
        {
            m_stDefPolicy.nFifoEventsRemovePercent = 
                               jsonDPolicy[KEY_DEF_POLICY_FIFO_PERCENT].asInt();

            //boundary check
            if ((m_stDefPolicy.nFifoEventsRemovePercent <= 0) 
                || (m_stDefPolicy.nFifoEventsRemovePercent > 100)) 
            {
                m_stDefPolicy.nFifoEventsRemovePercent = DEF_POLICY_FIFO_PERCENT;
            }
        }
        HCPLOG_T << "Default policy: name=" << m_stDefPolicy.strName
                 << "; percent=" << m_stDefPolicy.nFifoEventsRemovePercent;

        bStatus = true;
    }

    return bStatus;
}

bool CGranularityReductionHandler::ReadGRPolicies(
                                        const ic_utils::Json::Value &rjsonGR)
{
    if (!(rjsonGR.isMember(KEY_POLICIES) && rjsonGR[KEY_POLICIES].isArray()))
    {
        return false;
    }

    ic_utils::Json::Value jsonPolicies = rjsonGR[KEY_POLICIES];
    for (int nItr=0; nItr < jsonPolicies.size(); nItr++)
    {
        GRPolicyConfigDetails stPolicy;
        ic_utils::Json::Value jsonPolicy = jsonPolicies[nItr];

        if (!(jsonPolicy.isMember(KEY_POLICY_NAME) 
            && jsonPolicy[KEY_POLICY_NAME].isString()))
        {
            continue;
        }

        stPolicy.strName = jsonPolicy[KEY_POLICY_NAME].asString();
        stPolicy.bClearDefExemptedEvents = false;

        if (jsonPolicy.isMember(KEY_EXEMPTION_OVERRIDE) 
            && jsonPolicy[KEY_EXEMPTION_OVERRIDE].isObject())
        {
            ic_utils::Json::Value jsonExemp = jsonPolicy[KEY_EXEMPTION_OVERRIDE];
            AddExemptedOverrideEvents(jsonExemp, stPolicy);
        }

        //populate exemption list for the policy
        std::string strEventsExempted;
        PopulateExemptionList(stPolicy, strEventsExempted);

        //add the final policy details into the list
        GRPolicy stPlcy;
        stPlcy.strName = stPolicy.strName;
        stPlcy.strExemptedEvents = strEventsExempted;
        stPlcy.nGranLevel = nItr + 1;
        m_vectPolicies.push_back(stPlcy);
    }

    return true;
}

void CGranularityReductionHandler::AddExemptedOverrideEvents(
                                    const ic_utils::Json::Value &rjsonExemp,
                                    GRPolicyConfigDetails &rstPolicy)
{
    if (rjsonExemp.isMember(KEY_EXEMPTION_ADD) &&
            rjsonExemp[KEY_EXEMPTION_ADD].isArray())
    {
        ic_utils::Json::Value jsonEvnts = rjsonExemp[KEY_EXEMPTION_ADD];
        for (int nItr = 0; nItr < jsonEvnts.size(); nItr++)
        {
            rstPolicy.setAddlExemptedEvnts.insert(jsonEvnts[nItr].asString());
        }
    }

    if (rjsonExemp.isMember(KEY_EXEMPTION_REMOVE) &&
            rjsonExemp[KEY_EXEMPTION_REMOVE].isArray())
    {
        ic_utils::Json::Value rEvnts = rjsonExemp[KEY_EXEMPTION_REMOVE];
        for (int nItr = 0; nItr < rEvnts.size(); nItr++)
        {
            rstPolicy.setRemoveFromExemptedEvnts.insert(rEvnts[nItr].asString());
        }
    }

    if (rjsonExemp.isMember(KEY_EXEMPTION_CLEAR) &&
            rjsonExemp[KEY_EXEMPTION_CLEAR].isBool())
    {
        rstPolicy.bClearDefExemptedEvents = 
                                       rjsonExemp[KEY_EXEMPTION_CLEAR].asBool();
    }
}

void CGranularityReductionHandler::LoadPolicies()
{
    HCPLOG_METHOD();

    m_strAttachPath = 
              ic_core::CIgniteConfig::GetInstance()->GetString(KEY_ATTACH_PATH);
    m_unDbSizeLimit = 
              ic_core::CIgniteConfig::GetInstance()->GetInt(KEY_DATABASE_LIMIT);

    /* Even if could not read policies from config file or no default policy is 
     * mentioned, load the default values
     */
    m_stDefPolicy.strName = KEY_DEF_POLICY_NAME;
    m_stDefPolicy.nFifoEventsRemovePercent = DEF_POLICY_FIFO_PERCENT;

    //default type of events to delete during GR is BATCH events
    m_eGReductionEventType = CUploadUtils::UploadDataMode::eBATCH;

    ic_utils::Json::Value jsonGR = ic_core::CIgniteConfig::GetInstance()->
                              GetJsonValue("DAM.Database.granularityReduction");
    if (!jsonGR.empty())
    {
        //if configured, read type of events to delete during GR
        bool bReadStatus = ReadEventTypeToDeleteDuringGR(jsonGR);
        HCPLOG_D << "Config read status of GR deletion events type~" 
                 << bReadStatus;

        //read freeStorageGainPercent
        int nGainPercent = ReadGRFreeStorageGainPercent(jsonGR);
        HCPLOG_D << "GR free storage gain percent~" << nGainPercent;

        //read exempted events
        bReadStatus = ReadGRExemptedEvents(jsonGR);
        HCPLOG_D << "Config read status of GR exempted events~" << bReadStatus;

        //read pre trigger event id suffix
        std::string strSfx = ReadGRPreTriggerEventIdSuffix(jsonGR);
        HCPLOG_D << "GR pre trigger event id suffix~" << strSfx;

        //read post trigger event id suffix
        strSfx = ReadGRPostTriggerEventIdSuffix(jsonGR);
        HCPLOG_D << "GR post trigger event id suffix~" << strSfx;

        //read default GR policy
        bReadStatus = ReadDefaultGRPolicy(jsonGR);
        HCPLOG_D << "Config read status of GR default policy~" << bReadStatus;

        HCPLOG_D << "Reading GR policies...";
        bReadStatus = ReadGRPolicies(jsonGR);
        HCPLOG_D << "Config read status of GR policies~" << bReadStatus;
    }
    else
    {
        HCPLOG_T << "Granularity Reduction section not found!";
    }
}

void CGranularityReductionHandler::PopulateExemptionList(
                                                 GRPolicyConfigDetails stPolicy,
                                                 std::string &rstrEventsExempted)
{
    HCPLOG_METHOD() << "Policy: " << stPolicy.strName;

    rstrEventsExempted ="";
    std::set<std::string> setTmpList;

    //adding default exemption events
    if (!stPolicy.bClearDefExemptedEvents)
    {
        setTmpList.insert(m_setExemptedEvents.begin(),
                          m_setExemptedEvents.end());
    }

    //add additional events
    if (!stPolicy.setAddlExemptedEvnts.empty())
    {
        setTmpList.insert(stPolicy.setAddlExemptedEvnts.begin(),
                          stPolicy.setAddlExemptedEvnts.end());
    }

    //remove events (if configured)
    if (!stPolicy.setRemoveFromExemptedEvnts.empty())
    {
        for (std::set<std::string>::iterator setItr = 
             stPolicy.setRemoveFromExemptedEvnts.begin();
             setItr != stPolicy.setRemoveFromExemptedEvnts.end(); setItr++)
        {
            setTmpList.erase(*setItr);
        }
    }

    //construct final list...";
    for (std::set<std::string>::iterator setItr = setTmpList.begin();
         setItr != setTmpList.end(); setItr++)
    {
        rstrEventsExempted += "\"" + (*setItr)+ "\",";
    }

    //remove the final "," char
    if (!rstrEventsExempted.empty()) 
    {
        rstrEventsExempted = rstrEventsExempted.substr(
                                                 0,rstrEventsExempted.size()-1);

        if (!m_strMandatoryExemptedEvents.empty()) 
        {
            rstrEventsExempted += "," + m_strMandatoryExemptedEvents;
        }
    }
    else
    {
        rstrEventsExempted = m_strMandatoryExemptedEvents;
    }

    HCPLOG_T << "Policy: " << stPolicy.strName 
             << "; Exempted event list: " << rstrEventsExempted;
}

void CGranularityReductionHandler::PerformGranularityReduction()
{
    HCPLOG_METHOD();

    //reading policies from config upon demand; once read, no need to read again.
    if (!m_bIsPolicyReadFromConfig) 
    {
        m_bIsPolicyReadFromConfig = true;

        LoadMandatoryExemptedEvents();
        LoadPolicies();
    }

    //suspend the uploaders until completing granularity reduction
    ic_bl::CUploadController::GetInstance()->SuspendUploadController();

    //give a time for uploaders to get suspended
    HCPLOG_T << "waiting for uploaders to stop...";
    sleep(5);

    ic_core::CDataBaseFacade *pDB = ic_core::CDataBaseFacade::GetInstance();
    size_t unDBSizeBefGR = pDB->GetSize();

    HCPLOG_T << "Total policies found:" << m_vectPolicies.size();

    long long llStartTime = m_stLastGrRecord.llTimeStamp;
    long long llEndTime = ic_utils::CIgniteDateTime::GetCurrentTimeMs();

    bool bIsRemoveAlternateSimilarEventPerformed = false;
    bool bIsGRSuccess = false;
    long long llStartId = m_stLastGrRecord.llLastGRPerRecId;

    for (std::vector<GRPolicy>::iterator vecItr = m_vectPolicies.begin();
         vecItr != m_vectPolicies.end(); vecItr++)
    {
        GRPolicy stPolicy = *vecItr;
        HCPLOG_T << "Processing policy..." << stPolicy.strName;

        if (POLICY_REMOVE_ALTERNATE_SIMILAR_EVENT == stPolicy.strName)
        {
            /* if performed already i.e. doing for 2nd time, start from the 
             * beginning
             */
            if (bIsRemoveAlternateSimilarEventPerformed)
            {
                RemoveAlternateSimilarEventPolicyHandler(0, llEndTime,
                                                         stPolicy, 0);
                //reset the flag
                bIsRemoveAlternateSimilarEventPerformed = false;
            }
            else 
            {
                RemoveAlternateSimilarEventPolicyHandler(llStartTime, llEndTime,
                                                            stPolicy, llStartId);
                //set the flag
                bIsRemoveAlternateSimilarEventPerformed = true;
            }

            /* Change done to fix WI [480217]:GranularityReduction improvement. 
             * It avoids gap in GR permormed events between Start & endtime, 
             * by updating startTime & startId by timestamp of lastEvent for 
             * which GR was performed,rather than taking endTime (which is 
             * curretnTimestamp when GR started)
             */
            GetLastGranularityPerformedRecord(llStartTime, llEndTime,
                                              m_stLastGrRecord);

            HCPLOG_D << "LastGRTimestamp: " << m_stLastGrRecord.llTimeStamp 
                        << "LastGRID: " << m_stLastGrRecord.llLastGRPerRecId;
        }
        else if (POLICY_REMOVE_REPEATED_TRIGGER_EVENTS_LEAVE_FIRST_AND_LAST == 
                 stPolicy.strName)
        {
            RemoveRepeatedTriggerEventsLeaveFirstAndLastPolicyHandler(0,
                                                            llEndTime, stPolicy);
        }
        else if (POLICY_REMOVE_TRIGGER_EVENT_POST_FILES == stPolicy.strName)
        {
            RemoveTriggerEventPostFilesPolicyHandler(0, llEndTime, stPolicy);
        }
        else
        {
            HCPLOG_E << "Unsupported policy " << stPolicy.strName;
            continue;
        }

        //vacuum now after granularity performed
        HCPLOG_C <<  "Vacuuming the DB..";
        bool bStat = pDB->VacuumDb();
        size_t unDBSizeAftGR = pDB->GetSize();
        size_t unDBSizeGained = unDBSizeBefGR - unDBSizeAftGR ;
        size_t unExpectedDbSizeToGain = ((float)m_nFreeStorageGainPercent/100) * 
                                        m_unDbSizeLimit;

        HCPLOG_T << "Vacuum resp: " << bStat << ". Post GR: name="
                 << stPolicy.strName << "; DBSizeLimit=" << m_unDbSizeLimit
                 << "; FSGPercent=" << m_nFreeStorageGainPercent 
                 << "; DBSizeBefGR=" << unDBSizeBefGR << "; DBSizeAfterGR=" 
                 << unDBSizeAftGR << "; DBSizeGained=" << unDBSizeGained 
                 << "; expectedGain=" << unExpectedDbSizeToGain;

        ic_event::CIgniteEvent event("1.0", "GranularityReduction");
        event.AddField("Policy", stPolicy.strName);
        event.AddField("DBSizeBeforeGR", (long long)unDBSizeBefGR);
        event.AddField("DBSizeAfterGR", (long long)unDBSizeAftGR);
        event.AddField("DBSizeGained", (long long)unDBSizeGained);
        event.Send();

        //if FSG is met stop, else continue;
        if (unDBSizeGained < unExpectedDbSizeToGain)
        {
            continue;
        }
        else 
        {
            bIsGRSuccess = true;
            break;
        }
    }

    //if FSG is met stop, else apply defaultPolicy.
    if (!bIsGRSuccess)
    {
        DefaultPolicyHandler();
    }

    //resume uploaders
    ic_bl::CUploadController::GetInstance()->ResumeUploadController();
}

void CGranularityReductionHandler::PopulateEventsList(
                                    const std::string &rstrEId,
                                    const long long &rllStartTime,
                                    const long long &rllEndTime,
                                    const long long &rllLastGRedId,
                                    std::set<long long> &rsetDeleteEvntIds,
                                    std::set<std::string> &rsetDeleteFileList)
{

    unsigned int unEvntIndex=0;
    long long llStartId = rllLastGRedId;

    while (true)
    {
        HCPLOG_T << "Populating events for EVENT \'" << rstrEId
                 << "\'; startid=" << llStartId << "; startTime="
                 << rllStartTime << "; endTime=" << rllEndTime;

        std::set<EventList> setEvntList =
                            PopulateSessionBasedEventList(rstrEId, rllStartTime,
                            rllEndTime, llStartId, DB_EVENT_QUERY_LIMIT);

        HCPLOG_T << "Total events found for \'" << rstrEId << "\' : " << setEvntList.size();

        if (setEvntList.empty())
        {
            break;
        }

        //check each event occurrence
        for (std::set<EventList>::iterator setItr = setEvntList.begin();
             setItr != setEvntList.end(); setItr++)
        {
            EventList stRecord = *setItr;

            llStartId = stRecord.llEventId;
            std::string eventName = stRecord.strEventId;

            //once new Client session is started, reset the index
            if (CLIENT_LAUNCHED_EVENT == eventName)
            {
                //reset counters
                unEvntIndex=0;
                continue;
            }

            unEvntIndex++;

            //consider only the alternate events
            if (!(unEvntIndex % 2 == 0)) // even number
            {
                continue;
            }

            rsetDeleteEvntIds.insert(stRecord.llEventId);

            AddAttachmentsFromEventRecord(stRecord, rsetDeleteFileList);
        }//end of evntList for loop
    }//while loop
}

void CGranularityReductionHandler::AddAttachmentsFromEventRecord(
                                    const EventList &rstRecord,
                                    std::set<std::string> &rsetDeleteFileList)
{
    //if any attachments are available
    if (ATTACHMENT_AVAILABLE_FLAG != rstRecord.nHasAttach)
    {
        return;
    }

    std::string strEventData = CUploadUtils::DecryptEventData(rstRecord.strEvents);

    ic_core::CEventWrapper event;
    event.JsonToEvent(strEventData);
    std::vector<std::string> vectAttachments = event.GetAttachments();
    for (std::vector<std::string>::iterator vectItr = vectAttachments.begin();
         vectItr != vectAttachments.end(); vectItr++)
    {
        //note down the files to be deleted.
        rsetDeleteFileList.insert(m_strAttachPath + FILE_SEPARATOR + *vectItr);
    }
}

void CGranularityReductionHandler::RemoveAlternateSimilarEventPolicyHandler(
                                     long long llStartTime, long long llEndTime,
                                     GRPolicy stPolicy, long long llLastGRedId)
{
    HCPLOG_METHOD() << "startTime=" << llStartTime << "; endTime=" << llEndTime
                    << "level=" << stPolicy.nGranLevel
                    << "lastGRedId=" << llLastGRedId;

    //retrieve unique eventids from database
    std::set<std::string> setUniqEvnts = PopulateUniqueSteadyStateEventIds(
                            llStartTime, llEndTime, stPolicy.strExemptedEvents);
    HCPLOG_T << "Total unique events found = " << setUniqEvnts.size();

    //for each unique event, retrieve all the event occurrences
    for (std::set<string>::iterator setItr = setUniqEvnts.begin();
         setItr != setUniqEvnts.end(); setItr++)
    {
        std::string strEventId = *setItr;

        std::set<long long> setDeleteEvntIds;
        std::set<std::string> setDeleteFileList;

        PopulateEventsList(strEventId, llStartTime, llEndTime,
                           llLastGRedId, setDeleteEvntIds, setDeleteFileList);

        //if events found, delete them
        HCPLOG_T <<"Total ids to delete: " << setDeleteEvntIds.size();
        if (!setDeleteEvntIds.empty()) 
        {
            CUploadUtils::DeleteEventsFromDB(
                                     ic_core::CDataBaseConst::TABLE_EVENT_STORE,
                                     setDeleteEvntIds, m_eGReductionEventType);
        }
        else 
        {
            HCPLOG_T << "no ids to delete!";
        }

        //if attachments found, delete them
        HCPLOG_T <<"Total files to delete: " << setDeleteFileList.size();
        if (!setDeleteFileList.empty()) 
        {
            //delete the attaachment files
            CUploadUtils::DeleteAttachmentFiles(setDeleteFileList);
        }
        else 
        {
            HCPLOG_T << "no attachment files to delete!";
        }

    }//end of uniqEvnts
    
    //update the corresponding granularity level in the DB
    UpdateGRLevel(llStartTime, llEndTime, stPolicy);
}

void CGranularityReductionHandler::PopulateTriggerEventsList(
                            const std::string &rstrPreTrigEventId,
                            const long long &rllStartTime,
                            const long long &rllEndTime,
                            std::vector<TrigEventDetails> &rvecTrigEvntsToDelete,
                            std::vector<TrigEventDetails> &rvecTrigEvnts)
{
    long long llStartId=0;

    while (true)
    {
        HCPLOG_T << "populating trigger events for EVENT " << rstrPreTrigEventId
                 << "; startid=" << llStartId << "; startTime=" << rllStartTime
                 << "; endTime=" << rllEndTime;

        std::set<EventList> setEvntList =
                PopulateSessionBasedTriggerEventList(rstrPreTrigEventId,
                        m_strPostTrgrEventIdSuffix, rllStartTime, rllEndTime,
                        llStartId, DB_EVENT_QUERY_LIMIT);
        HCPLOG_T << "Total events found for " << rstrPreTrigEventId 
                 << " : " << setEvntList.size();

        if (setEvntList.empty())
        {
            break;
        }

        ProcessEventListAndAddEvents(setEvntList, llStartId,
                                     rvecTrigEvntsToDelete, rvecTrigEvnts);
    }//while
}

void CGranularityReductionHandler::ProcessEventListAndAddEvents(
                            const std::set<EventList> &rsetEvntList,
                            long long &rllStartId,
                            std::vector<TrigEventDetails> &rvecTrigEvntsToDelete,
                            std::vector<TrigEventDetails> &rvecTrigEvnts)
{
    for (std::set<EventList>::iterator setItr = rsetEvntList.begin();
                                       setItr != rsetEvntList.end(); setItr++)
    {
        EventList stRecord = *setItr;

        rllStartId = stRecord.llEventId;
        std::string strEventName = stRecord.strEventId;

        /* once new session is started, add the events from rvecTrigEvnts to main
         * 'rvecTrigEvntsToDelete' list.
         * if at all this for loop ends (due to no further data found in DB, the
         *  events added into the list 'tmpTrigEvnts' so far will be considered 
         * outside of this while loop.
         */
        if (CLIENT_LAUNCHED_EVENT == strEventName)
        {
            //if only two events found, ignore; if more found, leave 1st and last
            if (rvecTrigEvnts.size() > 2)
            {
                //remove the last element
                rvecTrigEvnts.erase(rvecTrigEvnts.begin() + 
                                    rvecTrigEvnts.size() -1); 
                
                //remove the first element
                rvecTrigEvnts.erase(rvecTrigEvnts.begin());

                //add the remaining into the list
                rvecTrigEvntsToDelete.insert(rvecTrigEvntsToDelete.end(),
                                     rvecTrigEvnts.begin(),rvecTrigEvnts.end());
            }

            //reset list
            rvecTrigEvnts.clear();
            continue;
        }

        /* check if the event is a pretrigger event;
         * if not, move to next event.
         */
        std::string strRegExp = "(.*)(" + m_strPreTrgrEventIdSuffix + ")";
        if (!(std::regex_match (strEventName, std::regex(strRegExp))))
        {
            continue;
        }

        HCPLOG_T << "Pre Trigger event found. id=" << stRecord.llEventId;
        TrigEventDetails stTEvnt;
        stTEvnt.Init();
        stTEvnt.llPreTrigId = stRecord.llEventId;
        stTEvnt.strPreTrigEvents = stRecord.strEvents;

        setItr++;
        if (setItr != rsetEvntList.end())
        {
            EventList stNextRecord = *setItr;
            std::string strNextEventName = stNextRecord.strEventId;

            std::string strRegExp = "(.*)(" + m_strPostTrgrEventIdSuffix + ")";
            if (std::regex_match (strNextEventName, std::regex(strRegExp)))
            {
                HCPLOG_T << "Post trigger event found. id=" 
                         << stNextRecord.llEventId;
                stTEvnt.llPostTrigId = stNextRecord.llEventId;
                stTEvnt.strPostTrigEvents = stNextRecord.strEvents;
            }
            else
            {
                /* move the iterator back; noted events will be added
                 * in CLIENT-LAUNCHED check section
                 */
                setItr--;
            }
            rvecTrigEvnts.push_back(stTEvnt);
            HCPLOG_T << "Adding to list:" << stTEvnt.llPreTrigId 
                     << ":" << stTEvnt.llPostTrigId;
        }
        else
        {
            HCPLOG_T << "end of list found";
            rvecTrigEvnts.push_back(stTEvnt);
            HCPLOG_T << "Adding to list:" << stTEvnt.llPreTrigId 
                     << ":" << stTEvnt.llPostTrigId;

            /* move the iterator back; noted events will be added in 
             * CLIENT-LAUNCHED check section
             */
            setItr--;
        }
    }//for loop
}

void CGranularityReductionHandler::AddAttachmentsToDeleteList(
                                       const std::string &rstrEventData,
                                       std::set<std::string> &rsetFilesToDelete)
{
    if (!rstrEventData.empty())
    {
        ic_core::CEventWrapper ev;
        ev.JsonToEvent(rstrEventData);

        std::vector<std::string> vecAttachments = ev.GetAttachments();
        for (std::vector<std::string>::iterator vectItr = vecAttachments.begin();
             vectItr != vecAttachments.end(); vectItr++)
        {
            rsetFilesToDelete.insert(m_strAttachPath + FILE_SEPARATOR + *vectItr);
        }
    }
}

void CGranularityReductionHandler::PopulateEventsAndFilesToDelete(
                    const std::vector<TrigEventDetails> &rvecTrigEvntsToDelete,
                    std::set<long long> &rsetIdsToDelete,
                    std::set<std::string> &rsetFilesToDelete)
{
    for (unsigned unItr=0; unItr < rvecTrigEvntsToDelete.size(); unItr++)
    {
        TrigEventDetails stTEDetails = rvecTrigEvntsToDelete[unItr];

        //add id to the deletion list
        HCPLOG_T << "Adding preTrigId " << stTEDetails.llPreTrigId 
                 << " to deletion list";
        rsetIdsToDelete.insert(stTEDetails.llPreTrigId);

        //find corresponding attachments
        std::string strEventData = CUploadUtils::DecryptEventData(
                                   stTEDetails.strPreTrigEvents);
        AddAttachmentsToDeleteList(strEventData, rsetFilesToDelete);

        //add after trigger id to the deletion list
        if (-1 != stTEDetails.llPostTrigId)
        {
            HCPLOG_T << "Adding postTrigId " << stTEDetails.llPostTrigId 
                     << " to deletion list";
            rsetIdsToDelete.insert(stTEDetails.llPostTrigId);

            //find corresponding attachments
            std::string strEventData = CUploadUtils::DecryptEventData(
                                       stTEDetails.strPostTrigEvents);
            AddAttachmentsToDeleteList(strEventData, rsetFilesToDelete);
        }
    }//for loop
}

void CGranularityReductionHandler::
                      RemoveRepeatedTriggerEventsLeaveFirstAndLastPolicyHandler(
                                     long long llStartTime, long long llEndTime,
                                     GRPolicy stPolicy)
{
    HCPLOG_METHOD() << "startTime=" << llStartTime << "; endTime=" 
                    << llEndTime << "level=" << stPolicy.nGranLevel;

    std::set<std::string> setUniqPreTrgEvnts = PopulateUniquePreTriggerEventIds(
                              llStartTime, llEndTime, m_strPreTrgrEventIdSuffix,
                              stPolicy.strExemptedEvents);
    HCPLOG_T << "Total unique trigger events found = " << setUniqPreTrgEvnts.size();

    for (std::set<string>::iterator setItr = setUniqPreTrgEvnts.begin();
         setItr != setUniqPreTrgEvnts.end(); setItr++)
    {
        std::string strPreTrigEventId = *setItr;
        HCPLOG_T << "Checking eventId..." << strPreTrigEventId;

        std::vector<TrigEventDetails> vecTrigEvntsToDelete;
        std::vector<TrigEventDetails> vecTmpTrigEvnts;

        PopulateTriggerEventsList(strPreTrigEventId, llStartTime, llEndTime,
                                  vecTrigEvntsToDelete, vecTmpTrigEvnts);

        /* if any trig events are still in the list(i.e. in the above while loop,
         * CLIENT-LAUNCHED event is not hit at the end.), add them excluding the
         * 1st & last occurrence
         */
        if (vecTmpTrigEvnts.size() > 2)
        {
            //remove the last element
            vecTmpTrigEvnts.erase(vecTmpTrigEvnts.begin() + 
                                  vecTmpTrigEvnts.size()-1); 

            //remove the first element
            vecTmpTrigEvnts.erase(vecTmpTrigEvnts.begin());

            //add the remaining into the list
            vecTrigEvntsToDelete.insert(vecTrigEvntsToDelete.end(),
                                 vecTmpTrigEvnts.begin(),vecTmpTrigEvnts.end());
        }

        HCPLOG_T << "Total trigger events found=" << vecTrigEvntsToDelete.size();

        std::set<long long> setIdsToDelete;
        std::set<std::string> setFilesToDelete;

        PopulateEventsAndFilesToDelete(vecTrigEvntsToDelete, setIdsToDelete,
                                       setFilesToDelete);


        HCPLOG_T <<"Total ids to delete: " << setIdsToDelete.size();
        if (0 != setIdsToDelete.size())
        {
            CUploadUtils::DeleteEventsFromDB(
                                     ic_core::CDataBaseConst::TABLE_EVENT_STORE,
                                     setIdsToDelete, m_eGReductionEventType);
        }

        HCPLOG_T <<"Total files to delete: " << setFilesToDelete.size();
        if (0 != setFilesToDelete.size())
        {
            CUploadUtils::DeleteAttachmentFiles(setFilesToDelete);
        }
    }//for each unique event

    //update the corresponding granularity level in the DB
    UpdateGRLevel(llStartTime, llEndTime, stPolicy);
}

void CGranularityReductionHandler::RemoveTriggerEventPostFilesPolicyHandler(
                  long long llStartTime, long long llEndTime, GRPolicy stPolicy)
{
    HCPLOG_METHOD() << "startTime=" << llStartTime 
                    << "; endTime=" << llEndTime 
                    << "level=" << stPolicy.nGranLevel;

    long long llStartId=0;
    std::vector<EventList> vecPostTrigEvntsToDelete;

    while (true)
    {
        std::set<EventList> setEvntList = PopulatePostTriggerEventList(
                             llStartTime, llEndTime, m_strPostTrgrEventIdSuffix,
                             llStartId, DB_EVENT_QUERY_LIMIT);
        HCPLOG_T << "Total post trigger events found " << setEvntList.size();

        if (setEvntList.empty())
        {
            break;
        }

        for (std::set<EventList>::iterator setItr = setEvntList.begin();
             setItr != setEvntList.end(); setItr++)
        {
            EventList stRecord = *setItr;

            llStartId = stRecord.llEventId;

            vecPostTrigEvntsToDelete.push_back(stRecord);
        }
    }

    HCPLOG_T << "Total post trigger events found="
             << vecPostTrigEvntsToDelete.size();

    std::set<long long> setIdsToDelete;
    std::set<std::string> setFilesToDelete;

    for (unsigned unItr=0; unItr < vecPostTrigEvntsToDelete.size(); unItr++)
    {
        EventList stTEvnt = vecPostTrigEvntsToDelete[unItr];

        HCPLOG_T << "Adding Post-Trig-Id " << stTEvnt.llEventId 
                 << " deletion list";

        //add id to the deletion list
        setIdsToDelete.insert(stTEvnt.llEventId);

        //find corresponding attachments
        std::string eventData = CUploadUtils::DecryptEventData(stTEvnt.strEvents);
        if (!eventData.empty())
        {
            ic_core::CEventWrapper ev;
            ev.JsonToEvent(eventData);

            std::vector<std::string> vecAttachments = ev.GetAttachments();
            for (std::vector<std::string>::iterator vectItr = 
                 vecAttachments.begin(); vectItr != vecAttachments.end();
                 vectItr++)
            {
                setFilesToDelete.insert(m_strAttachPath + FILE_SEPARATOR + 
                                        *vectItr);
            }
        }
    }

    HCPLOG_T <<"Total ids to delete: " << setIdsToDelete.size();
    if (0 != setIdsToDelete.size())
    {
        CUploadUtils::DeleteEventsFromDB(ic_core::CDataBaseConst::TABLE_EVENT_STORE,
                                        setIdsToDelete, m_eGReductionEventType);
    }

    HCPLOG_T <<"Total files to delete: " << setFilesToDelete.size();
    if (0 != setFilesToDelete.size())
    {
        CUploadUtils::DeleteAttachmentFiles(setFilesToDelete);
    }

    //update the corresponding granularity level in the DB
    UpdateGRLevel(llStartTime, llEndTime, stPolicy);
}

void CGranularityReductionHandler::DefaultPolicyHandler()
{
    HCPLOG_METHOD();

    int nRowCount = GetEventRowCount();
    HCPLOG_C << "Total rows: " << nRowCount;

    // number of rows to delete
    nRowCount = ((nRowCount * m_stDefPolicy.nFifoEventsRemovePercent) / 100) - 1;
    HCPLOG_C << "Rows to delete(" << m_stDefPolicy.nFifoEventsRemovePercent 
             << "%): " << nRowCount;
    if (nRowCount <= 0)
    {
        HCPLOG_W << "No rows to delete, aborting DB purge...";
        return;
    }
    HCPLOG_D << "Rows to be deleted << " << nRowCount;

    ic_core::CDataBaseFacade* pDB = ic_core::CDataBaseFacade::GetInstance();
    size_t unDBSize = pDB->GetSize();

    std::vector<std::string> vecProjection;
    vecProjection.push_back(ic_core::CDataBaseConst::COL_TIMESTAMP);
    std::vector<std::string> vecOrderBy;
    vecOrderBy.push_back(ic_core::CDataBaseConst::COL_TIMESTAMP + " ASC");
    ic_core::CCursor *pCurObj = pDB->Query(
                      ic_core::CDataBaseConst::TABLE_EVENT_STORE, vecProjection,
                      ic_core::CDataBaseConst::COL_TIMESTAMP + " IS NOT NULL",
                      vecOrderBy, nRowCount);

    long long llStartTime = 0;
    long long llEndTime = 0;
    if (pCurObj)
    {
        if (pCurObj->MoveToFirst())
        {
            llStartTime = pCurObj->GetLong(pCurObj->GetColumnIndex(
                                       ic_core::CDataBaseConst::COL_TIMESTAMP));
            pCurObj->MoveToLast();
            llEndTime = pCurObj->GetLong(pCurObj->GetColumnIndex(
                                       ic_core::CDataBaseConst::COL_TIMESTAMP));
        }
        delete pCurObj;
    }

    std::string strSelection = "(" + ic_core::CDataBaseConst::COL_TIMESTAMP +
              ">=" + ic_utils::CIgniteStringUtils::NumberToString(llStartTime) +
              ") AND (" + ic_core::CDataBaseConst::COL_TIMESTAMP +
              "<=" + ic_utils::CIgniteStringUtils::NumberToString(llEndTime) +
              ")";

    // Query and delete attachments belonging to the events to be purged
    vecProjection.clear();
    vecProjection.push_back(ic_core::CDataBaseConst::COL_EVENTS);
    pCurObj = pDB->Query(ic_core::CDataBaseConst::TABLE_EVENT_STORE,
                         vecProjection, strSelection + " AND (" +
                         ic_core::CDataBaseConst::COL_HAS_ATTACH + "=1)");

    std::set<std::string> setFileList;
    if (pCurObj)
    {
        for (bool bWorking = pCurObj->MoveToFirst(); bWorking;
             bWorking = pCurObj->MoveToNext())
        {
            std::string eventData = pCurObj->GetString(pCurObj->GetColumnIndex(
                                          ic_core::CDataBaseConst::COL_EVENTS));
            eventData = CUploadUtils::DecryptEventData(eventData);
            if (eventData.empty())
            {
                continue;
            }
            ic_core::CEventWrapper ev;
            ev.JsonToEvent(eventData);

            std::vector<std::string> vecAttachments = ev.GetAttachments();
            if (!vecAttachments.empty())
            {
                setFileList.insert(vecAttachments.begin(), vecAttachments.end());
            }
        }
        delete pCurObj;
    }

    for (std::set<std::string>::iterator setItr = setFileList.begin();
         setItr != setFileList.end(); setItr++)
    {
        std::string strFilePath = *setItr;
        ic_utils::CIgniteFileUtils::Remove(m_strAttachPath + "/" + strFilePath);
    }

    // Purge the events
    pDB->Remove(ic_core::CDataBaseConst::TABLE_EVENT_STORE, strSelection);

    HCPLOG_C <<  "Vacuuming the DB..";
    bool bStat = pDB->VacuumDb();

    size_t unNewSize = pDB->GetSize();
    HCPLOG_C <<  "Vacuum resp: " << bStat << ". Size Reduced by :" <<
                                (int)(unDBSize - unNewSize) << " after purging";
    HCPLOG_C << "Timestamps to be deleted from :" << llStartTime 
             << "to" << llEndTime;

    ic_event::CIgniteEvent event("1.0", "DBOverLimit");
    event.AddField("Action", "Purge");
    event.AddField("DBThresoldLimit", (long long)m_unDbSizeLimit);
    event.AddField("DBSizeBeforeAction", (long long)unDBSize);
    event.AddField("DBSizeAfterAction", (long long)unNewSize);
    event.AddField("EventsDeleted", nRowCount);
    event.AddField("EventsDeletedFrom", llStartTime);
    event.AddField("EventsDeletedTo", llEndTime);
    event.Send();
}

void CGranularityReductionHandler::UpdateGRLevel(long long llStartTime,
                                         long long llEndTime, GRPolicy stPolicy)
{
    HCPLOG_METHOD() << "startTime=" << llStartTime << "; endTime=" << llEndTime;

    ic_core::CDataBaseFacade *pDB = ic_core::CDataBaseFacade::GetInstance();

    std::string strSelection = "(" + ic_core::CDataBaseConst::COL_TIMESTAMP + 
              " > " + ic_utils::CIgniteStringUtils::NumberToString(llStartTime);
    strSelection += " AND " + ic_core::CDataBaseConst::COL_TIMESTAMP + " <= " +
                        ic_utils::CIgniteStringUtils::NumberToString(llEndTime);
    strSelection += ")";

    strSelection += " AND " + ic_core::CDataBaseConst::COL_EVENT_ID;
    strSelection += " NOT IN (" + stPolicy.strExemptedEvents;
    strSelection += ") AND " + ic_core::CDataBaseConst::COL_EVENT_ID;
    strSelection += "!=\"" + CLIENT_LAUNCHED_EVENT;
    strSelection += "\"";

    HCPLOG_T << "sql=" << strSelection;

    bool bTransactionStarted = pDB->StartTransaction();

    //mark remaining events as uploaded
    ic_core::CContentValues data;
    data.Put(ic_core::CDataBaseConst::COL_GRANULARITY,stPolicy.nGranLevel);

    pDB->Update(ic_core::CDataBaseConst::TABLE_EVENT_STORE, &data, strSelection);

    if (bTransactionStarted)
    {
        pDB->EndTransaction(true);
    }
}

bool CGranularityReductionHandler::GetLastGranularityPerformedRecord(
                                  long long llStartTime, long long llEndTime,
                                  LastGRPerformedRecord &rstLastGrRecordDetails)
{
    uint8_t uchLimit = 1;
    bool bRetValue = false;
    HCPLOG_METHOD() << "startTime=" << llStartTime << "; endTime=" << llEndTime;

    std::vector<std::string> vecProjection;
    vecProjection.push_back(ic_core::CDataBaseConst::COL_TIMESTAMP);
    vecProjection.push_back(ic_core::CDataBaseConst::COL_ID);
    std::string strSelection = "(" + ic_core::CDataBaseConst::COL_TIMESTAMP + 
              " > " + ic_utils::CIgniteStringUtils::NumberToString(llStartTime);
    strSelection += " AND " + ic_core::CDataBaseConst::COL_TIMESTAMP + " <= " + 
                        ic_utils::CIgniteStringUtils::NumberToString(llEndTime);
    strSelection += ")";
    strSelection += " AND " + ic_core::CDataBaseConst::COL_GRANULARITY + "!=0";
    HCPLOG_T << "sql=" << strSelection;

    std::vector<std::string> vecOrderBy;
    vecOrderBy.push_back(ic_core::CDataBaseConst::COL_TIMESTAMP + " DESC");

    ic_core::CCursor *pCurObj = ic_core::CDataBaseFacade::GetInstance()->Query(
                      ic_core::CDataBaseConst::TABLE_EVENT_STORE, vecProjection,
                      strSelection, vecOrderBy,uchLimit);

    //Size check of cursor added to avoid crash (WI 517806)
    if (pCurObj)
    {
        HCPLOG_T << "retrieving items...1";
        if(pCurObj->Size() > 0)
        {
            rstLastGrRecordDetails.llTimeStamp = pCurObj->GetLong(
               pCurObj->GetColumnIndex(ic_core::CDataBaseConst::COL_TIMESTAMP));
            rstLastGrRecordDetails.llLastGRPerRecId = pCurObj->GetLong(pCurObj->
                               GetColumnIndex(ic_core::CDataBaseConst::COL_ID));
            bRetValue = true;
        }

        delete pCurObj;
        pCurObj = NULL;
    }
    return bRetValue;
}

#ifdef IC_UNIT_TEST
bool CGranularityReductionHandler::IsPolicyReadFromConfig()
{
    return m_bIsPolicyReadFromConfig;
}

CGranularityReductionHandler::LastGRPerformedRecord CGranularityReductionHandler::GetLastGRRecord()
{
    return m_stLastGrRecord;
}

std::string CGranularityReductionHandler::GetPreTrgrEventIdSuffix ()
{
    return m_strPreTrgrEventIdSuffix;
}

std::string CGranularityReductionHandler::GetPostTrgrEventIdSuffix()
{
    return m_strPostTrgrEventIdSuffix;
}

int CGranularityReductionHandler::GetFreeStorageGainPercent()
{
    return m_nFreeStorageGainPercent;
}

std::string CGranularityReductionHandler::GetMandatoryExemptedEvents()
{
    return m_strMandatoryExemptedEvents;
}

CUploadUtils::UploadDataMode CGranularityReductionHandler::GetGReductionEventType()
{
    return m_eGReductionEventType;
}

int CGranularityReductionHandler::GetDefaultFreeStorageGainPercent()
{
    return DEF_FREE_STORAGE_GAIN_PERCENT;
}

std::string CGranularityReductionHandler::GetDefaultPostTriggerEventIDSuffix()
{
    return DEF_POST_TRIGGER_EVENT_ID_SUFFIX;
}

std::string CGranularityReductionHandler::GetDefaultPreTriggerEventIDSuffix()
{
    return DEF_PRE_TRIGGER_EVENT_ID_SUFFIX;
}

#endif

}//namespace
