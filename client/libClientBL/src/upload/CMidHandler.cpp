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

#include "CMidHandler.h"

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CMidHandler"

namespace ic_bl
{

/**
 * Method to delete events from table which are marked against given mid
 * @param[in] strTable Table name from which the events needs to be deleted
 * @param[in] nMid Message Id
 * @return True if deletion is successful and false otherwise
 */
bool delete_events_from_db(const std::string strTable, int nMid)
{
    std::string strSelection = ic_core::CDataBaseConst::COL_MID + " == " +
                                ic_utils::CIgniteStringUtils::NumberToString(nMid);
    ic_core::CDataBaseFacade *pDb = ic_core::CDataBaseFacade::GetInstance();
    if (strTable == ic_core::CDataBaseConst::TABLE_EVENT_STORE &&
        ic_core::CUploadMode::GetInstance()->IsBatchModeSupported())
    {
        // Note: Dont need to apply filter for Topiced events.
        //       From 1st deletion quiry, topiced events will get deleted.

        // delete events which already uploaded or not supported by batch mode
        bool bTransactionStarted = pDb->StartTransaction();
        std::string strDeleteSelection(strSelection);
        strDeleteSelection += " AND " + ic_core::CDataBaseConst::COL_BATCH_SUPPORT +
                                " == 0";
        bool bResult = pDb->Remove(strTable, strDeleteSelection);

        // mark remaining events as uploaded
        ic_core::CContentValues data;
        data.Put(ic_core::CDataBaseConst::COL_STREAM_SUPPORT, 0);
        bResult = bResult && pDb->Update(strTable, &data, strSelection);

        if (bTransactionStarted)
        {
            pDb->EndTransaction(true);
        }
        return bResult;
    }
    else
        return pDb->Remove(strTable, strSelection);
}


CMidHandler::CMidHandler()
{
    Start();
}

CMidHandler::~CMidHandler()
{
}

CMidHandler* CMidHandler::GetInstance(void)
{
    HCPLOG_METHOD();
    static CMidHandler handler;
    return &handler;
}

bool CMidHandler::InitMid()
{
    std::set<int>::iterator iterItr;
    std::map<int,std::string>::iterator iterMidTableItr;
    HCPLOG_D << "Clearing all the pending packets which are published";
    /* publishMidSet will contain the mid's for which we have recieved ACK 
     * before those mid's are set in DB. In case of publish done outisde 
     * MQTTUploader(ex:LocationService), will have mid entries in this 
     * publishMidSet. This needs to be cleared at every re-connection to avoid 
     * clearing of valid packets in next connection.
     */ 
    HCPLOG_I << "m_setPublishedMidSet.size~" << m_setPublishedMidSet.size();
    for(iterItr = m_setPublishedMidSet.begin();iterItr!=m_setPublishedMidSet.end();iterItr++)
    {
        iterMidTableItr = m_mapMidDBTableMap.find(*iterItr);
        if(iterMidTableItr != m_mapMidDBTableMap.end())
        {
            HCPLOG_C << "Clearing " << *iterItr << " from " << iterMidTableItr->second;
            ClearMid(*iterItr,iterMidTableItr->second);
        }
    }
    HCPLOG_D << "Clearing the mid table map and published mid set";
    m_setPublishedMidSet.clear();
    m_mapMidDBTableMap.clear();
    
    HCPLOG_D <<"Reset Mid value for all events/alerts to default";
    ic_core::CContentValues data;
    data.Put(ic_core::CDataBaseConst::COL_MID, 0);
    ic_core::CDataBaseFacade *pDb = ic_core::CDataBaseFacade::GetInstance();
    bool bRetAlertMid = pDb->Update(ic_core::CDataBaseConst::TABLE_ALERT_STORE,
                                    &data,
                                    ic_core::CDataBaseConst::COL_MID + "!=0");
    bool bRetEventMid = pDb->Update(ic_core::CDataBaseConst::TABLE_EVENT_STORE,
                                    &data,
                                    ic_core::CDataBaseConst::COL_MID + "!=0");
    HCPLOG_C << "initMid done. bRetAlertMid: " << bRetAlertMid 
             << ", bRetEventMid: " << bRetEventMid;

    return (bRetEventMid && bRetAlertMid);
}


bool CMidHandler::ClearMid(int nMid,std::string strTable)
{
    HCPLOG_I << nMid << " " << strTable;
    MidTable *pMt = new MidTable(nMid, strTable);
    if (pMt == NULL)
    {
        return false;
    }
    m_queMidTobeDeleted.Put(pMt, sizeof(pMt));
    m_QueueMutex.Lock();
    m_condQueueHandler.ConditionBroadcast();
    m_QueueMutex.Unlock();
    return true;
}

bool CMidHandler::SetMidTable(int nMid, const std::string& rstrTable)
{
    ic_utils::CScopeLock sLock(m_MapMutex);
    bool bMidFlag = false;
    HCPLOG_I << nMid << " - " << rstrTable;
    if (m_setPublishedMidSet.find(nMid) != m_setPublishedMidSet.end())
    {
        m_setPublishedMidSet.erase(nMid);
        bMidFlag = ClearMid(nMid, rstrTable);
    }
    else
    {
        m_mapMidDBTableMap[nMid] = rstrTable;
        bMidFlag = true;
    }
    return bMidFlag;
}

bool CMidHandler::ProcessPublishedMid(int nMid)
{
    ic_utils::CScopeLock sLock(m_MapMutex);
    bool bMidFlag = false;

    HCPLOG_I << nMid;
    std::map<int, std::string>::iterator iterIt = m_mapMidDBTableMap.find(nMid);
    if (iterIt != m_mapMidDBTableMap.end())
    {
        std::string strTable = iterIt->second;
        bMidFlag = ClearMid(nMid, strTable);
        m_mapMidDBTableMap.erase(iterIt);
    }
    else
    {
        bMidFlag = m_setPublishedMidSet.insert(nMid).second;
    }
    return bMidFlag;
}

std::string CMidHandler::GetTableOfPublishedMid(int nMid)
{
    std::string strTableName = "";
    std::map<int, std::string>::iterator iterIt = m_mapMidDBTableMap.find(nMid);
    if (iterIt != m_mapMidDBTableMap.end())
    {
        strTableName = iterIt->second;
    }
    return strTableName;
}

void CMidHandler::Run()
{
    HCPLOG_METHOD();
    //Register to get the Shutdown Notification
    ic_core::CIgniteClient::GetOnOffMonitor()->RegisterForShutdownNotification(
                                        this,ic_core::IOnOff::eR_MID_HANDLER);
    while(true)
    {
        MidTable* pMt;
        while (m_queMidTobeDeleted.Take(&pMt))
        {
            //as discussed logLevel changed to debug
            HCPLOG_D << "Deleting events for mid:" << pMt->m_nMid
                     << " from table:" << pMt->m_strTable;
            delete_events_from_db(pMt->m_strTable, pMt->m_nMid);
            delete pMt;
        }

        if(m_bShutdownInitiated)
        {
            HCPLOG_D << "Shutdown Requested";
            break;
        }

        m_QueueMutex.Lock();
        m_condQueueHandler.ConditionWait(m_QueueMutex);
        m_QueueMutex.Unlock();
    }
    ic_core::CIgniteClient::GetOnOffMonitor()->ReadyForShutdown(
                                               ic_core::IOnOff::eR_MID_HANDLER);
    ic_core::CIgniteClient::GetOnOffMonitor()->UnregisterForShutdownNotification(
                                               ic_core::IOnOff::eR_MID_HANDLER);
    Detach();
}

void CMidHandler::NotifyShutdown()
{
    HCPLOG_D << "Shutdown Request Recieved for CMidHandler";
    m_bShutdownInitiated = true;  
    m_QueueMutex.Lock();
    m_condQueueHandler.ConditionBroadcast();
    m_QueueMutex.Unlock();
}

} //namespace ic_bl
