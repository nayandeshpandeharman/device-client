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

#include <set>
#include <cmath>
#include "dam/CDBTransport.h"
#include "dam/CTransportHandlerBase.h"
#include "CIgniteLog.h"
#include "CIgniteDateTime.h"
#include "CIgniteEvent.h"
#include "CIgniteStringUtils.h"
#include "CIgniteConfig.h"
#include "crypto/CIgniteDataSecurity.h"
#include "core/CKeyGenerator.h"
#include "core/CAesSeed.h"
#include "CIgniteFileUtils.h"
#include "upload/CUploadController.h"
#include "auth/CTokenManager.h"
#include "dam/CMessageController.h"
#include "core/CEventIntervalValidator.h"
#include "CIgniteClient.h"
#include "config/CUploadMode.h"
#include "db/CDataBaseFacade.h"

//! Macro for key CDBTransport string
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CDBTransport"

using namespace std;

//! Macro for key vendor string
#define KEY_VENDOR "vendor"

namespace ic_bl 
{
namespace 
{

//! Constant key for 'DAM.DatabaseThreadPriority' string
static const std::string DB_PRIORITY = "DAM.DatabaseThreadPriority";

//! Constant key for '5' int value
static const int DEF_DB_PRIORITY = 5; // Background priority

//! Constant key for 'AcpHeartBeat' string
static const std::string EVENT_HEART_BEAT = "AcpHeartBeat";

//! Constant key for 'PTHREAD_MUTEX_INITIALIZER' pthread_mutex_t
static pthread_mutex_t sEventCacheMutex = PTHREAD_MUTEX_INITIALIZER;

//! Constant key for 'DAM.Database.eventStoreRecordSize' string
static const std::string KEY_DB_EVENTSTORE_SIZE = "DAM.Database.eventStoreRecordSize";

//! Constant key for '200' int value
static const uint16_t DEF_EVENTSTORE_SIZE = 200;

//! Constant key for '500' int value
static const uint16_t MAX_EVENTSTORE_SIZE = 500;

//! Constant key for 'DAM.Database.maxInsertEventInOneTxn' string
static const std::string KEY_MAX_INSERT_IN_ONE_TRANSACTION = "DAM.Database.maxInsertEventInOneTxn";

//! Constant key for '50' int value
static const uint16_t DEF_MAX_INSERT_IN_ONE_TRANSACTION = 50;

//! Constant key for 'attachmentFailureReason' string
static const std::string KEY_ATTACHMENT_FAILURE_REASON = "attachmentFailureReason";

//! Constant key for 'batchModeUnsupported' string
static const std::string BATCH_MODE_UNSUPPORTED        = "batchModeUnsupported";

//! Global variable to hold queue event count
unsigned long int g_ulnQueECnt=0;

//! Global variable to hold queue event iteration
unsigned long int g_ulnQueECntIter=0;

//! Global variable to hold processed queue event count
unsigned long int g_ulnPECnt=0;

//! Global variable to hold processed queue event iteration
unsigned long int g_ulnPECntIter=0;

#ifdef EVENT_LOG_INTERVAL_CNT
    //! Global variable to hold event logging interval count
    unsigned long int g_ulnEvntLogIntrvlCnt = EVENT_LOG_INTERVAL_CNT;
#else
    //! Global variable to hold event logging interval count if EVENT_LOG_INTERVAL_CNT not defined
    unsigned long int g_ulnEvntLogIntrvlCnt = 500;
#endif


/**
 * Global method to trigger encrypt event data based on given input parameter
 * @param[in] rstrEventData event payload to encrypt
 * @return returns the encrypted data; if encryption fails, returns the original
 * input i.e. rstrEventData
 */

static std::string encrypt_event_data(const std::string& rstrEventData)
{
    ic_core::CIgniteDataSecurity securityObject(ic_core::CKeyGenerator::GetActivationKey(),
                       ic_core::CAesSeed::GetInstance()->GetIvRandom());
    return securityObject.Encrypt(rstrEventData);
}

/**
 * Global method to trigger decrypt event data based on given input parameter
 * @param[in] rstrEvent event payload to decrypt
 * @return returns the decrypted data; if decryption fails, return the original
 * input i.e. rstrEvent
 */
static std::string decrypt_event_data(const std::string& rstrEvent)
{
    ic_core::CIgniteDataSecurity securityObject(ic_core::CKeyGenerator::GetActivationKey(),
                       ic_core::CAesSeed::GetInstance()->GetIvRandom());
    return securityObject.Decrypt(rstrEvent);
}

}

void CDBTransport::InsertEvent(const std::string& rstrSerialized)
{
    HCPLOG_METHOD();
    ic_core::CEventWrapper event;
    event.JsonToEvent(rstrSerialized);
    ic_utils::Json::Value jsonEvData = event.GetData();
    std::string strEventId = event.GetEventId();
    ic_core::CUploadMode *pMode = ic_core::CUploadMode::GetInstance();
    if( strEventId == "IgniteClientLaunched")
    {
        SendIgniteStartMessage();
    }

    long long llTimeStamp = event.GetTimestamp();
    long long llTimeZone = event.GetTimezone() * 60 * 1000; // time zone in ms
    long lInsertStatus = -1;
    ic_core::CContentValues data;
    data.Put(ic_core::CDataBaseConst::COL_EVENT_ID, strEventId);
    data.Put(ic_core::CDataBaseConst::COL_TIMESTAMP, llTimeStamp);
    data.Put(ic_core::CDataBaseConst::COL_TIMEZONE, llTimeZone);
    data.Put(ic_core::CDataBaseConst::COL_SIZE, (long long)rstrSerialized.size());
    data.Put(ic_core::CDataBaseConst::COL_HAS_ATTACH, event.GetAttachments().empty() ? 0 : 1);
    data.Put(ic_core::CDataBaseConst::COL_EVENTS, encrypt_event_data(rstrSerialized));

    bool bSupportedEvent(false);

    if(pMode->IsStreamModeSupported()){
        if (CMessageController::IsAlert(strEventId))
        {
            lInsertStatus = ic_core::CDataBaseFacade::GetInstance()->Insert(ic_core::CDataBaseConst::TABLE_ALERT_STORE, &data);
            if(-1 == lInsertStatus)
            {
                HCPLOG_E << "Insert failed " <<  strEventId << " , " << llTimeStamp;
            }
            CUploadController::GetInstance()->TriggerAlertsUpload(START_ALERT_UPLOAD);
            return;
        }
    } //if(pMode->IsStreamModeSupported())

    if (pMode->IsEventSupportedForStream(strEventId)) 
    {
        ProcessEventForStreamMode(event, pMode->IsBatchModeSupported(), data);
        bSupportedEvent = true;

    }//if(pMode->IsEventSupportedForStream(strEventId))

    if (pMode->IsEventSupportedForBatch(strEventId)) {
        HCPLOG_E << " Event supported for batch " << strEventId;
        data.Put(ic_core::CDataBaseConst::COL_BATCH_SUPPORT, 1);
        bSupportedEvent = true;
    }//if(pMode->IsEventSupportedForBatch(strEventId))

    if (jsonEvData.isMember("topic")) 
    {
        GetTopicedEventData(pMode, event, data);
        bSupportedEvent = true;
    }

    if(bSupportedEvent) {
        lInsertStatus = ic_core::CDataBaseFacade::GetInstance()->Insert(ic_core::CDataBaseConst::TABLE_EVENT_STORE, &data);
        if (-1 == lInsertStatus) {
            HCPLOG_E << "Insert failed " << strEventId << " , " << llTimeStamp;
        }
    } //if(bSupportedEvent)
}

void CDBTransport::SendIgniteStartMessage()
{
    ic_core::IClientMessageDispatcher *pDispatcher = 
                           ic_core::CIgniteClient::GetClientMessageDispatcher();
    if (pDispatcher)
    {
        pDispatcher->DeliverIgniteStartMessage();
    }
}

void CDBTransport::ProcessEventForStreamMode(ic_core::CEventWrapper &rEvent,
                                            const bool &rbBatchModeSupported,
                                            ic_core::CContentValues &rData)
{
    std::string strEventId = rEvent.GetEventId();

    /* If event has attachment, it should be uploaded via Batch Upload, 
     * provided batch mode is supported
     */
    if (!rEvent.GetAttachments().empty() && rbBatchModeSupported) 
    {
        //event has attachment and batch mode is supported
        HCPLOG_C << " Event supported for batch " << strEventId 
                 << " as it has attachement";
        rData.Put(ic_core::CDataBaseConst::COL_BATCH_SUPPORT, 1);
    }
    else 
    {
        HCPLOG_I << " Event supported for stream " << strEventId;
        rData.Put(ic_core::CDataBaseConst::COL_STREAM_SUPPORT, 1);

        /* If event has attachment and batch mode is not supported as per config
         * , then event is to sent over MQTT with attachment discarded.
         * In this case, attachments are deleted from "fileAttachmentPath"
         */ 
        if (!rEvent.GetAttachments().empty()) 
        {
            /* Event has attachment but batch mode is not supported, 
             * updating event
             */
            rEvent.AddField(KEY_ATTACHMENT_FAILURE_REASON, 
                            BATCH_MODE_UNSUPPORTED);

            std::string strSerializedEvnt;
            rEvent.EventToJson(strSerializedEvnt);

            rData.Put(ic_core::CDataBaseConst::COL_SIZE, 
                     (long long)strSerializedEvnt.size());
            rData.Put(ic_core::CDataBaseConst::COL_EVENTS, 
                      encrypt_event_data(strSerializedEvnt));

            //Deleting attachment from IC's attachment space
            std::string strAttachmentPath = ic_core::CIgniteConfig::GetInstance()
                                                   ->GetString(KEY_ATTACH_PATH);

            if (!strAttachmentPath.empty())
            {
                for (auto itr : rEvent.GetAttachments()) {
                    std::string strFilePath = strAttachmentPath + "/" + itr;
                    HCPLOG_I << "Deleting attachment " << itr;
                    ic_utils::CIgniteFileUtils::Remove(strFilePath);
                }
            }
        }
    }
}

void CDBTransport::GetTopicedEventData(const ic_core::CUploadMode *pMode, 
                                      ic_core::CEventWrapper &rEvent,
                                      ic_core::CContentValues &rData)
{
    ic_utils::Json::Value jsonEvntData = rEvent.GetData();
    rData.Put(ic_core::CDataBaseConst::COL_TOPIC, jsonEvntData["topic"].asString());

    /* topiced events will go through stream mode by default
     * Upload topiced event on stream, only if it doesn't have any attachment(s)
     * ,since stream upload doesn't support file upload
     */ 
    if (pMode->IsStreamModeSupported() && rEvent.GetAttachments().empty()) 
    {
        rData.Put(ic_core::CDataBaseConst::COL_STREAM_SUPPORT, 1);
    } 
    else if(pMode->IsBatchModeSupported())
    {
        rData.Put(ic_core::CDataBaseConst::COL_BATCH_SUPPORT, 1);
    }
    else
    {
        //Do nothing
    }
}

CDBTransport::CDBTransport(CTransportHandlerBase* handler) : CTransportHandlerBase(handler)
{
    m_nDbEventStoreRecordAvgSize = ic_core::CIgniteConfig::GetInstance()->GetInt(KEY_DB_EVENTSTORE_SIZE, DEF_EVENTSTORE_SIZE);
    if (DEF_EVENTSTORE_SIZE > m_nDbEventStoreRecordAvgSize || MAX_EVENTSTORE_SIZE < m_nDbEventStoreRecordAvgSize)
    {
        m_nDbEventStoreRecordAvgSize = DEF_EVENTSTORE_SIZE;
    } 
    m_nMaxEventInsertInOneTxn = ic_core::CIgniteConfig::GetInstance()->GetInt(KEY_MAX_INSERT_IN_ONE_TRANSACTION, DEF_MAX_INSERT_IN_ONE_TRANSACTION);
    std::string strDbLogPath = ic_core::CIgniteConfig::GetInstance()->GetString("DAM.Log.dbStream");
    if (!strDbLogPath.empty())
    {
        HCPLOG_I << " - Setting stream log file to " << strDbLogPath;
        m_pStreamLog = new std::ofstream(strDbLogPath.c_str(), ios::out);
    }
    else
    {
        m_pStreamLog = NULL;
    }

    m_pGranReducHndlr = NULL;
    m_bIsShutdownInitiated = false;

    ic_core::CIgniteConfig* pConfig = ic_core::CIgniteConfig::GetInstance();
    m_ulEventQueueMaxSize = pConfig->GetInt("DAM.CpuProcessesLog.eventQueueMaxSize");
    m_ulEventInsertWindowSize = pConfig->GetInt("DAM.CpuProcessesLog.eventInsertWindowSize");
    m_nDbSizeLimit = pConfig->GetInt("DAM.Database.dbSizeLimit");
    HCPLOG_T << ">> m_ulEventQueueMaxSize = " << m_ulEventQueueMaxSize;
    HCPLOG_T << ">> m_ulEventInsertWindowSize = " << m_ulEventInsertWindowSize;

    if (m_ulEventQueueMaxSize <= 0)
    {
        m_ulEventQueueMaxSize = 1048576; //default value: 1048576 bytes(1MB)
    }
    if (m_ulEventInsertWindowSize <= 0)
    {
        m_ulEventInsertWindowSize = 104858; //default value 104857bytes (10% of 1MB)
    }
    m_ulEventInsertThresholdSize = m_ulEventQueueMaxSize;

    InitIgnoredEvents();

    m_bUploadAfterActivation = ic_core::CIgniteConfig::GetInstance()->GetBool("DAM.UploadAfterActivation");

    if(m_bUploadAfterActivation)
    {
        HCPLOG_W << "EVENTS ARE CONFIGURED TO BE IGNORED TILL ACTIVATION " ;
    }

    ic_utils::Json::Value jsonRoot = ic_core::CIgniteConfig::GetInstance()->GetJsonValue("DAM");
    ic_utils::Json::Value jsonEvents = jsonRoot["ActivationValidatorExceptions"].isArray() ? jsonRoot["ActivationValidatorExceptions"]
                                                                              : ic_utils::Json::Value::nullRef;
    if (jsonEvents != ic_utils::Json::Value::nullRef)
    {
        for (int nIndex = 0; nIndex < jsonEvents.size(); nIndex++)
        {
            m_SetActivationExceptionEvents.insert(jsonEvents[nIndex].asString());
            HCPLOG_W << "activation exception event:"<<jsonEvents[nIndex].asString();
        }
    }
}

CDBTransport::~CDBTransport()
{
    if (m_pStreamLog != NULL)
    {
        if(m_pStreamLog->is_open())
        {
            m_pStreamLog->close();
        }
        delete m_pStreamLog;
    }
    
    if (m_pGranReducHndlr != NULL)
    {
        delete m_pGranReducHndlr;
    }
}

void CDBTransport::Run()
{
    int nThreadPriority = ic_core::CIgniteConfig::GetInstance()->GetInt(DB_PRIORITY, DEF_DB_PRIORITY);
    ic_utils::CIgniteThread::SetCurrentThreadPriority(nThreadPriority);

    ic_core::CIgniteClient::GetOnOffMonitor()->RegisterForShutdownNotification(this, ic_core::IOnOff::eR_DB_TRANSPORT);
    while (!m_bIsShutdownInitiated)
    {
        pthread_mutex_lock(&sEventCacheMutex);
        unsigned int unQueSize = m_queEvent.Size();
        pthread_mutex_unlock(&sEventCacheMutex);
        if (unQueSize == 0)
        {
            HCPLOG_D << "Q empty. Sleeping 1s";
            sleep(1); // back off if there is not enough events in the cache and release the lock
        }
        else
        {
            // Check the DB size and purge if needed
            ic_core::CDataBaseFacade* pDb = ic_core::CDataBaseFacade::GetInstance();
            size_t nDbSize = pDb->GetSize();
            HCPLOG_D << "EventStore::insertEvents: DB Size = " << nDbSize << ", limit = " <<
                     m_nDbSizeLimit;

            if (nDbSize >= m_nDbSizeLimit)
            {
                HCPLOG_E << "DB size(" << nDbSize << ") OVER (" << m_nDbSizeLimit << "). purging it.";
                PurgeDB(nDbSize);
            }

            // Execute multiple insertions as single transaction for optimized performance
            bool bTransactionStarted = pDb->StartTransaction();

            ProcessQueueData(unQueSize);
            
            if (bTransactionStarted)
            {
                pDb->EndTransaction(true);
            }
        }//end of else
    }//end of while (!m_bIsShutdownInitiated)

    ic_core::CIgniteClient::GetOnOffMonitor()->ReadyForShutdown(ic_core::IOnOff::eR_DB_TRANSPORT);
    ic_core::CIgniteClient::GetOnOffMonitor()->UnregisterForShutdownNotification(ic_core::IOnOff::eR_DB_TRANSPORT);
    Detach();
}

void CDBTransport::ProcessQueueData(const unsigned int &runQueSize)
{
    std::string strEvntData;
    uint16_t unMaxLimit = GetMaxEventToInsertInOneTxn(runQueSize);
    uint16_t unInsertCntr = 1;
    while(unInsertCntr <= unMaxLimit)
    {
        //insertion of events is based on maxLimit value returned from getMaxEventToInsertInOneTxn
        //max events to be inserted in one txn are based on the value configured as maxInsertEventInOneTxn
        if (!m_queEvent.Take(&strEvntData)) 
        {
            break;
        }

        if(0 < strEvntData.size())
        {
            ProcessMessage(strEvntData);
        }
        //increment insert count
        ++unInsertCntr;

        if (0 == g_ulnPECnt) 
        {
            g_ulnPECntIter++;
        }

        g_ulnPECnt++;
        if (0 == g_ulnPECnt%100) 
        {
            //as disscussed during log analysis, changed to debug
            HCPLOG_D << "P>>QSize:" << m_queEvent.Size() 
                     << "; QCntIter:" << g_ulnQueECntIter << "; QCnt:" << g_ulnQueECnt 
                     << "; PCntIter:" << g_ulnPECntIter << "; PCnt:" << g_ulnPECnt;
        }

        /* since one event is popped out from queue, let us revise the 
         * size of m_ulEventSizeInQueue
         */
        m_ulEventSizeInQueue = runQueSize;
        m_ulEventSizeInQueue = m_ulEventSizeInQueue - strEvntData.size();

        /* if current threshold size is not max and eventqueue size went down 
         * below 90% of limit (max-window), set the threshold level as maximum
         */
        if ((m_ulEventInsertThresholdSize != m_ulEventQueueMaxSize) &&
            (m_ulEventSizeInQueue < (m_ulEventQueueMaxSize - m_ulEventInsertWindowSize)))
        {
            m_ulEventInsertThresholdSize = m_ulEventQueueMaxSize;
        }
    }//end of while(unInsertCntr <= unMaxLimit)
}

void CDBTransport::ProcessMessage(const string& rstrSerialized)
{
    HCPLOG_METHOD();

    if ( (m_pStreamLog != NULL) && m_pStreamLog->is_open() )
    {
        *m_pStreamLog << rstrSerialized << endl;
    }
    InsertEvent(rstrSerialized);
}

void CDBTransport::PurgeDB(const size_t dbSize)
{
    HCPLOG_W << "DB size OVER. vacuum it.";

    ic_core::CDataBaseFacade* pDb = ic_core::CDataBaseFacade::GetInstance();
    bool bStat = pDb->VacuumDb();
    size_t nNewSize = pDb->GetSize();
    HCPLOG_C << "Vacuum resp: " << bStat << ". Size Reduced by :" << (int)(dbSize - nNewSize);

    if (nNewSize >= m_nDbSizeLimit)
    {
        if (NULL == m_pGranReducHndlr)
        {
            m_pGranReducHndlr = new CGranularityReductionHandler();
        }

        if (m_pGranReducHndlr)
        {
            m_pGranReducHndlr->PerformGranularityReduction();
        }
        else
        {
            HCPLOG_E << "could not create granularity reduction handler!";
        }
    }
    else
    {
        ic_event::CIgniteEvent event("1.0", "DBOverLimit");

        HCPLOG_W << "Size Reduced by :" << (int)(dbSize - nNewSize) << " after vaccum";
        event.AddField("Action", "Vaccum");
        event.AddField("DBThresoldLimit", (long long)m_nDbSizeLimit);
        event.AddField("DBSizeBeforeAction", (long long)dbSize);
        event.AddField("DBSizeAfterAction", (long long)nNewSize);

        std::string strSerializedEvnt;
        event.EventToJson(strSerializedEvnt);
        InsertEvent(strSerializedEvnt);
    }
}

void CDBTransport::HandleEvent(ic_core::CEventWrapper* pEvent)
{
    std::string strEventId = pEvent->GetEventId();
    double dblEventTs = pEvent->GetTimestamp();
    std::string strSerializedEvnt;
    pEvent->EventToJson(strSerializedEvnt);
    delete pEvent;

    // Before going ahead for anything, first check if the interval Validation required
    if(!CEventIntervalValidator::GetInstance()->IsValidInterval(strEventId, dblEventTs))
    {
        HCPLOG_I << "IGNORING EVENT TO Push in Database " << strEventId << " It failed for interval check";
        return;
    }

    if(WaitForActivation(strEventId))
    {
        //if WaitForActivation(strEventId) true, then event will not be inserted into DB and ignored
        HCPLOG_I << " EVENT IGNORED -> DEVICE NOT YET ACTIVATED";
        return;
    }

    HCPLOG_T << ">> m_ulEventSizeInQueue=" << m_ulEventSizeInQueue << ";event size=" <<
             (strSerializedEvnt.size()) << ";m_ulEventInsertThresholdSize=" << m_ulEventInsertThresholdSize;

    //check the size limits
    pthread_mutex_lock(&sEventCacheMutex);
    unsigned int unQueSize = m_queEvent.Size();
    pthread_mutex_unlock(&sEventCacheMutex);

    m_ulEventSizeInQueue = unQueSize;
    if ((m_ulEventSizeInQueue + strSerializedEvnt.size()) < m_ulEventInsertThresholdSize)
    {
        if (m_nIgnoredEventCnt != 0)
        {
            //before inserting incoming event, let us insert ignored events details
            HCPLOG_T << ">>Inserting IgnoredEvents details...";
            ic_event::CIgniteEvent ignoredEvnts("x.x", "EventCacheOverflow",
                                    dblEventTs); //let us use the dblEventTs from incoming stamp
            ignoredEvnts.AddField("count", m_nIgnoredEventCnt);
            ignoredEvnts.AddField("bytes", ic_utils::CIgniteStringUtils::NumberToString(m_ulIgnoredEventSize));
            ignoredEvnts.AddField("startTs", m_dblEventIgnoreStartTs);
            ignoredEvnts.AddField("endTs", dblEventTs); //let us use the dblEventTs as the end timestamp

            std::string strIgnoredEvnt;
            ignoredEvnts.EventToJson(strIgnoredEvnt);

            m_queEvent.Put(strIgnoredEvnt, strIgnoredEvnt.size());
            HCPLOG_W << ">>IgnoredEvent details pushed into the queue " << strIgnoredEvnt;

            //let us reset the counters
            InitIgnoredEvents();
        }

        m_queEvent.Put(strSerializedEvnt, strSerializedEvnt.size());
        HCPLOG_T << strEventId << ">>event is successfully pushed into the queue!";

        PrintEvntQueueLogs();
    }
    else
    {
        HCPLOG_E << ">>Ignoring the event:" << strEventId << "~ EventQueue size(" << (m_ulEventSizeInQueue + strSerializedEvnt.size()) <<
                 ") exceeds the limit: " << m_ulEventInsertThresholdSize << "~ ";

        m_nIgnoredEventCnt++;
        m_ulIgnoredEventSize = m_ulIgnoredEventSize + strSerializedEvnt.size();
        if (m_dblEventIgnoreStartTs == 0.0)
        {
            m_dblEventIgnoreStartTs = dblEventTs;

            //since this is the first event being ignored, let us reset the threshold size down to 90%;
            //  the threshold size is reset back to max size in Run() when the 10% of events are written into DB.
            m_ulEventInsertThresholdSize = m_ulEventQueueMaxSize - m_ulEventInsertWindowSize; //maxlimit - windowlimit
        }
    }
    //since this is the last phase of event handling, don't pass it to next handler.
}

void CDBTransport::PrintEvntQueueLogs()
{
    if (g_ulnQueECnt == 0) 
    {
        g_ulnQueECntIter++;
    }

    g_ulnQueECnt++;
    if (0 == g_ulnQueECnt%g_ulnEvntLogIntrvlCnt) 
    {
        HCPLOG_C << "Q>>QSize:" << m_queEvent.Size() 
                 << "; QCntIter:" << g_ulnQueECntIter << "; QCnt:" << g_ulnQueECnt 
                 << "; PCntIter:" << g_ulnPECntIter << "; PCnt:" << g_ulnPECnt;
    }
}

void CDBTransport::HandleNonIgniteEvent(ic_core::CEventWrapper* pEvent)
{
    std::string strSerializedEvnt;
    pEvent->EventToJson(strSerializedEvnt);
    delete pEvent;
    m_queEvent.Put(strSerializedEvnt, strSerializedEvnt.size());
}

void CDBTransport::FlushCache()
{
    // Execute multiple insertions as single transaction for optimized performance
    ic_core::CDataBaseFacade* pDb = ic_core::CDataBaseFacade::GetInstance();
    bool bTransactionStarted = pDb->StartTransaction();
    std::string ret;
    while(m_queEvent.Take(&ret))
    {
        if(ret.size() > 0)
        {
            ProcessMessage(ret);
        }
        HCPLOG_W << "flushing cache: " << ret;
    }
    if (bTransactionStarted)
    {
        pDb->EndTransaction(true);
    }
}

void CDBTransport::InitIgnoredEvents()
{
    m_nIgnoredEventCnt = 0;
    m_ulIgnoredEventSize = 0;
    m_ulEventSizeInQueue = 0;
    m_dblEventIgnoreStartTs = 0.0;
}

bool CDBTransport::WaitForActivation(const std::string& rstrEventId)
{

    if (CTokenManager::IsActivated())
    {
        //if isActivated true, means device is already activated, then no need to wait for activation and return false.
        return false;
    } 
    else
    {
        //if isActivated false, device is not activated then check for m_bUploadAfterActivation value
        if(m_bUploadAfterActivation)
        {
            //if m_bUploadAfterActivation true, check if event is configured as "DAM.ActivationValidatorExceptions" in config, will be ignored.
            if (m_SetActivationExceptionEvents.find(rstrEventId) != m_SetActivationExceptionEvents.end())
            {
                return false;
            } else
            {
                return true;
            }
        }
        else
        {
            // if m_bUploadAfterActivation false, event is not configured to be ignored hence return false
            return false;
        }
    }
}

void CDBTransport::NotifyShutdown()
{
    m_bIsShutdownInitiated = true;
}

uint16_t CDBTransport::GetMaxEventToInsertInOneTxn(const unsigned int unQueSize)
{
    //calculate no of records in m_queEvent using current unQueSize and configurable m_nDbEventStoreRecordAvgSize value
    unsigned int unCurrRecordsInQueue = (unQueSize/m_nDbEventStoreRecordAvgSize);

    //If unQueSize is less than m_nDbEventStoreRecordAvgSize, the variable unCurrRecordsInQueue will become zero hence assign it to 1 min value
    if(0 == unCurrRecordsInQueue )
    {
        unCurrRecordsInQueue = 1;
    }

    //set the default value for maxLimit, return same incase unCurrRecordsInQueue is greater than configurable m_nMaxEventInsertInOneTxn value
    uint16_t unMaxLimit = m_nMaxEventInsertInOneTxn;

    //check for unCurrRecordsInQueue value, less than or equal to m_nMaxEventInsertInOneTxn
    if (unCurrRecordsInQueue <= m_nMaxEventInsertInOneTxn){
        //update unMaxLimit value with unCurrRecordsInQueue, which is used to insert events in one transaction
        unMaxLimit = unCurrRecordsInQueue;
    }
    return unMaxLimit;
}
} /* namespace ic_bl*/
