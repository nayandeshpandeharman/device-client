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

#include <vector>
#include "db/CInvalidTimestampEventStore.h"
#include "CIgniteConfig.h"
#include "CIgniteLog.h"
#include "CIgniteStringUtils.h"
#include "core/CKeyGenerator.h"
#include "core/CAesSeed.h"
#include "crypto/CIgniteDataSecurity.h"
#include "db/CDataBaseFacade.h"
#include "dam/CEventWrapper.h"

//! Macro for 'CInvalidTimestampEventStore' string
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CInvalidTimestampEventStore"

namespace ic_bl
{
/**
 * Method to encrypt event data
 * @param[in] rstrEventData event data
 * @return encrypted event data
 */
static std::string encrypt_event_data(const std::string &rstrEventData)
{
    ic_core::CIgniteDataSecurity securityObject(
                               ic_core::CKeyGenerator::GetActivationKey(),
                               ic_core::CAesSeed::GetInstance()->GetIvRandom());
    return securityObject.Encrypt(rstrEventData);
}

/**
 * Method to decrypt event data
 * @param[in] rstrEvent event string
 * @return decrypted event data
 */
static std::string decrypt_event(const std::string &rstrEvent)
{
    ic_core::CIgniteDataSecurity securityObject(
                               ic_core::CKeyGenerator::GetActivationKey(),
                               ic_core::CAesSeed::GetInstance()->GetIvRandom());
    return securityObject.Decrypt(rstrEvent);
}

CInvalidTimestampEventStore::CInvalidTimestampEventStore()
{
    m_unDbSizeLimit = ic_core::CIgniteConfig::GetInstance()->
                                             GetInt("DAM.Database.dbSizeLimit");

    bool bResult = ic_core::CDataBaseFacade::GetInstance()->Remove(
                            ic_core::CDataBaseConst::TABLE_INVALID_EVENT_STORE);
}

CInvalidTimestampEventStore* CInvalidTimestampEventStore::GetInstance()
{
    static CInvalidTimestampEventStore instance;
    return &instance;
}

int CInvalidTimestampEventStore::GetEventRowCount()
{
    std::vector<std::string> vecProjection;
    vecProjection.push_back("COUNT(*)");
    ic_core::CCursor *pCursObj = ic_core::CDataBaseFacade::GetInstance()->Query(
             ic_core::CDataBaseConst::TABLE_INVALID_EVENT_STORE, vecProjection);

    int nRowCount = 0;
    if (pCursObj)
    {
        if (pCursObj->MoveToFirst())
        {
            nRowCount = pCursObj->GetInt(0);
        }
        delete pCursObj;
    }

    HCPLOG_C << nRowCount;
    return nRowCount;
}

void CInvalidTimestampEventStore::PurgeInvalidEvents(int nEvSize)
{
    ic_event::CIgniteEvent *pEvent = new ic_event::CIgniteEvent(
                                                          "1.0", "DBOverLimit");
    ic_core::CDataBaseFacade *pDBFacade = ic_core::CDataBaseFacade::GetInstance();
    size_t unDBSize = pDBFacade->GetSize();

    HCPLOG_I << "Remove old events from db";
    std::vector<std::string> vecProjection;
    vecProjection.push_back(ic_core::CDataBaseConst::COL_ID);
    vecProjection.push_back(ic_core::CDataBaseConst::COL_TIMESTAMP);

    std::vector<std::string> strOrderBy;
    strOrderBy.push_back(ic_core::CDataBaseConst::COL_TIMESTAMP + " ASC");

    ic_core::CCursor *pCursObj = ic_core::CDataBaseFacade::GetInstance()->Query(
                         ic_core::CDataBaseConst::TABLE_INVALID_EVENT_STORE,
                         vecProjection, ic_core::CDataBaseConst::COL_TIMESTAMP +
                         " IS NOT NULL", strOrderBy, nEvSize);

    std::list<int> listRowIDs;
    std::stringstream ssRows;
    int nRowCount(0);
    if (pCursObj) 
    {
        for (bool working = pCursObj->MoveToFirst();
             working; working = pCursObj->MoveToNext())
        {
            int nId = pCursObj->GetInt(pCursObj->GetColumnIndex(
                                              ic_core::CDataBaseConst::COL_ID));
            listRowIDs.push_back(nId);
            ssRows << " " << nId;
            ++nRowCount;
        }

        delete pCursObj;
    }
    if (!listRowIDs.empty()) 
    {
        HCPLOG_C << "Delete rows:" << ssRows.str();
        DeleteEvents(listRowIDs);
    }

    size_t unNewSize = pDBFacade->GetSize();
    pEvent->AddField("Action", "Purge");
    pEvent->AddField("DBThresoldLimit", (long long)m_unDbSizeLimit);
    pEvent->AddField("DBSizeBeforeAction", (long long)unDBSize);
    pEvent->AddField("DBSizeAfterAction", (long long)unNewSize);
    pEvent->AddField("EventsDeleted", nRowCount);
    pEvent->AddField("IsInvalidTimstampEvent", true);

    //since this event should be uploaded first store it in internal cache
    m_listDBOverLimitEventList.push_back(pEvent);

}

bool CInvalidTimestampEventStore::InsertEvents(
                               std::list <ic_core::CEventWrapper*> &rlistEvents)
{
    if(rlistEvents.empty())
    {
        HCPLOG_D << "empty list.";
        return false;
    }

    size_t unDBSize = ic_core::CDataBaseFacade::GetInstance()->GetSize();
    HCPLOG_D << "CInvalidTimestampEventStore::insertEvents: DB Size = " 
             << unDBSize << ", limit = " << m_unDbSizeLimit;

    if (unDBSize >= m_unDbSizeLimit)
    {
        PurgeInvalidEvents(rlistEvents.size());
    }

    ic_core::CDataBaseFacade *pDBFacade = ic_core::CDataBaseFacade::GetInstance();
    bool bTransactionStarted = pDBFacade->StartTransaction();

    while (!rlistEvents.empty()) 
    {
        ic_core::CEventWrapper *pInvalidEvent = rlistEvents.front();
        std::string strSerialized;
        pInvalidEvent->EventToJson(strSerialized);
        HCPLOG_T << "move from queue to database >>" << strSerialized;
        InsertIntoDb(strSerialized);
        rlistEvents.pop_front();

        delete pInvalidEvent;
    }
    if (bTransactionStarted) 
    {
        pDBFacade->EndTransaction(true);
    }
    return true;
}

long CInvalidTimestampEventStore::InsertIntoDb(const std::string &rstrSerialized)
{
    ic_core::CEventWrapper event;
    event.JsonToEvent(rstrSerialized);
    long long llTimestamp = event.GetTimestamp();

    ic_core::CContentValues data;
    data.Put(ic_core::CDataBaseConst::COL_TIMESTAMP, llTimestamp);

    data.Put(ic_core::CDataBaseConst::COL_EVENTS,
             encrypt_event_data(rstrSerialized));
    return ic_core::CDataBaseFacade::GetInstance()->Insert(
                     ic_core::CDataBaseConst::TABLE_INVALID_EVENT_STORE, &data);
}

void CInvalidTimestampEventStore::RetriveEvents(int nNumRowsRequested,
                                     std::list<int> &rlistRowIDs,
                                     std::list<std::string> &rlistEventDataList)
{
    HCPLOG_METHOD();
    if(!m_listDBOverLimitEventList.empty())
    {
        while(!m_listDBOverLimitEventList.empty())
        {
            ic_event::CIgniteEvent *pEvent = m_listDBOverLimitEventList.front();
            pEvent->Send();
            delete pEvent;
            m_listDBOverLimitEventList.pop_front();
        }
    }

    std::vector<std::string> vecProjection;
    vecProjection.push_back(ic_core::CDataBaseConst::COL_ID);
    vecProjection.push_back(ic_core::CDataBaseConst::COL_TIMESTAMP);
    vecProjection.push_back(ic_core::CDataBaseConst::COL_EVENTS);

    std::vector<std::string> vecOrderBy;
    vecOrderBy.push_back(ic_core::CDataBaseConst::COL_TIMESTAMP + " ASC");

    ic_core::CCursor* pCursObj = ic_core::CDataBaseFacade::GetInstance()->Query(
              ic_core::CDataBaseConst::TABLE_INVALID_EVENT_STORE, vecProjection,
              ic_core::CDataBaseConst::COL_TIMESTAMP + " IS NOT NULL",
              vecOrderBy, nNumRowsRequested);

    if (pCursObj)
    {
        HCPLOG_C << "retrieving events";
        for (bool bWorking = pCursObj->MoveToFirst(); bWorking;
             bWorking = pCursObj->MoveToNext())
        {
            int nId = pCursObj->GetInt(pCursObj->GetColumnIndex(
                                              ic_core::CDataBaseConst::COL_ID));
            std::string strEvData = pCursObj->GetString(pCursObj->GetColumnIndex(
                                          ic_core::CDataBaseConst::COL_EVENTS));

            strEvData = decrypt_event(strEvData);
            HCPLOG_C << strEvData;
            rlistEventDataList.push_back(strEvData);
            rlistRowIDs.push_back(nId);

        }
        delete pCursObj;
    }
}

bool CInvalidTimestampEventStore::DeleteEvents(std::list<int> &rlistRowIds)
{
    HCPLOG_METHOD();
    if (rlistRowIds.empty())
    {
        HCPLOG_D << "Nothing to delete";
        return false;
    }

    std::string strSelection = ic_core::CDataBaseConst::COL_ID + " in (";
    std::list<int>::iterator iter;
    for (iter = rlistRowIds.begin(); iter != rlistRowIds.end(); ++iter)
    {
        strSelection.append(ic_utils::CIgniteStringUtils::NumberToString(*iter));
        strSelection.append(",");
    }
    strSelection.replace(strSelection.size() - 1, 1, ")");

    return ic_core::CDataBaseFacade::GetInstance()->Remove(
              ic_core::CDataBaseConst::TABLE_INVALID_EVENT_STORE, strSelection);
}
}/* namespace ic_bl */
