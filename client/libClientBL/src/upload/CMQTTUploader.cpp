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
#include <sys/time.h>
#include "unistd.h"
#include <vector>
#include <iterator>
#include <limits.h>
#include "CMQTTUploader.h"
#include "CIgniteLog.h"
#include "CIgniteConfig.h"
#include "CIgniteStringUtils.h"
#include "db/CDataBaseFacade.h"
#include "crypto/CIgniteDataSecurity.h"
#include "core/CKeyGenerator.h"
#include "dam/CEventWrapper.h"
#include "CIgniteGZip.h"
#include "core/CAesSeed.h"
#include "config/CUploadMode.h"
#include "CUploadUtils.h"

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CMQTTUploader"

using namespace acp;

namespace ic_bl
{
#define KEY_VENDOR "vendor"
namespace 
{
#if defined(TEST_HIGH_FREQ)
unsigned long long ueCnt = 0;//Upload Event Count
unsigned long long tUeCnt = ueCnt; //take the backup of ueCnt before increment
unsigned long long ueCntIter = 1;//Upload Event iterationCount
unsigned long long uaCnt = 0;//Upload Alert Count
unsigned long long tUaCnt = uaCnt; //take the backup of uaCnt before increment
unsigned long long uaCntIter = 1;//Upload Alert iterationCount
unsigned long long uteCnt = 0;//Upload Topic Event Count
unsigned long long tUteCnt = uteCnt; //take the backup of uteCnt before increment
unsigned long long uteCntIter = 1;//Upload Topic Event iterationCount
unsigned long long ueaCnt = 0;//Upload External event count
unsigned long long tUeaCnt = ueaCnt; //take the backup of ueaCnt before increment
unsigned long long ueaCntIter = 1;//Upload External event iteration count
#endif

// Number of events to be bundled in single payload for upload
static const int EVENT_COUNT = 20;

// Number of retry count untill which the alerts upload will be attempted
static const int RETRY_COUNT = 10;

// Wait time untill which the uplaod will be waiting when suspended
static const int SERVICE_RESUME_RETRY_WAIT_TIME = 10;

/* Wait time untill which the alert upload will be waiting when there is 
 * no connection
 */
static const int UPLOAD_RETRY_WAIT_TIME = 5;

// Default upload event summary count
static const int DEF_UPLOAD_EVENT_SUMMARY_CNT = 20;

// Min events count for uploading in a bundle
static const int MIN_UPLOAD_EVENT_COUNT = 20;

// Maximum events count for uploading in a bundle
static const int MAX_UPLOAD_EVENT_COUNT = 175;

/**
 * Method to update the MID column in rows specified in the database ,
 * with mid value passed
 * @param[in] rstrTable Table name in the database
 * @param[in] rvecRowIDs Row IDs for which mid value needs to be updated
 * @param[in] nMid Mid value to be updated against rows
 * @return True if the updation is successful , false otherwise
 */
bool update_mid(const std::string& rstrTable, 
                const std::vector<long long>& rvecRowIDs, const int nMid )
{
    bool bRetVal = true;
    if (rvecRowIDs.empty())
    {
        HCPLOG_D << "Nothing to update";
    }

    std::string strSelection = ic_core::CDataBaseConst::COL_ID + " in (";
    for (int nI = 0; nI < rvecRowIDs.size(); nI++)
    {
        strSelection.append(
            ic_utils::CIgniteStringUtils::NumberToString(rvecRowIDs[nI]));
        strSelection.append(",");
    }
    strSelection.replace(strSelection.size() - 1, 1, ")");

    ic_core::CContentValues data;
    data.Put(ic_core::CDataBaseConst::COL_MID, nMid);

    bool bResult = ic_core::CDataBaseFacade::GetInstance()->Update(rstrTable, 
                                                            &data, strSelection);
    if (!bResult)
    {
        HCPLOG_E << "Failed to update table " << rstrTable << "~" << nMid;
        bRetVal = false;
    }

    return bRetVal;
}

/**
 * Method to check if the compression of MQTT payload is enabled
 * with mid value passed
 * @param void
 * @return True if the compression is enabled, false otherwise
 */
bool is_compression_enabled()
{
    //get mqtt compression value from config
    bool bRetVal =  true;
    ic_utils::Json::Value jsonMqRoot = 
                    ic_core::CIgniteConfig::GetInstance()->GetJsonValue("MQTT");
    if (jsonMqRoot.isMember("compression") && 
        jsonMqRoot["compression"].asBool() == false)
    {
        bRetVal = false;
        HCPLOG_D << "MQTT compression disabled";
    }
    return bRetVal;
}

} // local namespace

CMQTTUploader::CMQTTUploader()
    : m_bCompression(is_compression_enabled()), m_bAlertsAvailable(true),
      m_bIsPeriodicityChanged(false)
{
    HCPLOG_METHOD();
    m_pMqClient = CIgniteMQTTClient::GetInstance();
    m_bUploadRequested = false;
    m_bShutdownRequested = false;
    m_EventTimer = NULL;
    m_AlertTimer = NULL;
    m_bUploadSuspended = false;

    ic_utils::Json::Value jsonEventArray = 
        ic_core::CIgniteConfig::GetInstance()->GetJsonValue(
                                                    "MQTT.ForceUploadEvents");
    if (jsonEventArray.isArray())
    {
        for (int nI = 0; nI < jsonEventArray.size(); nI++)
        {
            m_setForceUploadEvents.insert(jsonEventArray[nI].asString());
            HCPLOG_I << "Force upload event " << jsonEventArray[nI].asString();
        }
    }

    m_unCurrEvntUploadCnt = 0;
    m_unCurrEvntUploadSummaryCnt = 0;
    m_unCurrUploadedEvntCnt = 0;

    InitEventsUploadCnt();

    InitConfigBasedLogging();

    ic_core::CIgniteConfig::GetInstance()->SubscribeForConfigUpdateNotification(
                                            "MQTTUploader", this);
}

void CMQTTUploader::InitEventsUploadCnt()
{
    int nConfiguredMaxEventUploadCnt = 
        ic_core::CIgniteConfig::GetInstance()->GetInt(
            "MQTT.pub_topics.events.uploadEventCount",MIN_UPLOAD_EVENT_COUNT);

    HCPLOG_I << "Max upload-event cnt based on config file: " 
             << nConfiguredMaxEventUploadCnt;

    /* if Max event upload count < MIN_UPLOAD_EVENT_COUNT set it to 
     * MIN_UPLOAD_EVENT_COUNT else if Max event upload count > 
     * MAX_UPLOAD_EVENT_COUNT set it to MAX_UPLOAD_EVENT_COUNT else assigned
     * the given value 
     */
    if(nConfiguredMaxEventUploadCnt <= MIN_UPLOAD_EVENT_COUNT)
    {
        m_unMaxEventUploadCnt = MIN_UPLOAD_EVENT_COUNT;
    }
    else if(nConfiguredMaxEventUploadCnt >= MAX_UPLOAD_EVENT_COUNT)
    {
        m_unMaxEventUploadCnt = MAX_UPLOAD_EVENT_COUNT;
    }
    else
    {
        m_unMaxEventUploadCnt = (unsigned int)nConfiguredMaxEventUploadCnt;
    }

    HCPLOG_C << "Max upload-event cnt:  " << m_unMaxEventUploadCnt;
}

CMQTTUploader::~CMQTTUploader()
{
    HCPLOG_METHOD();

    if (NULL != m_EventTimer)
    {
        delete m_EventTimer;
        m_EventTimer = NULL;
    }
    if (NULL != m_AlertTimer)
    {
        delete m_AlertTimer;
        m_AlertTimer = NULL;
    }
    ic_core::CIgniteConfig::GetInstance()->UnSubscribeForConfigUpdateNotification("MQTTUploader");
}

CMQTTUploader *CMQTTUploader::GetInstance(void)
{
    HCPLOG_METHOD();
    static CMQTTUploader mqUploader;
    return &mqUploader;
}

int CMQTTUploader::AlertUpload(const std::string &rstrAlert)
{
    HCPLOG_METHOD();
    int nErr = -1;
    if (rstrAlert.empty())
    {
        //Alert already inserted into database
        m_bAlertsAvailable = true;
        Notify(eTT_ALERT);
        return 0;
    }
    ic_utils::Json::Value jsonRoot;
    ic_utils::Json::Reader jsonReader;
    if (jsonReader.parse(rstrAlert, jsonRoot))
    {
        ic_core::CEventWrapper event;
        event.JsonToEvent(rstrAlert);
        event.Send();
        return 0;
    }
    return nErr;
}

void CMQTTUploader::StartAlertsUpload()
{
    HCPLOG_METHOD();
    //get periodicity
    int nPeriodicity = ic_core::CIgniteConfig::GetInstance()->GetInt(
                                    "MQTT.pub_topics.alerts.periodicity", 0);

    //clear previous timer object.
    if(m_AlertTimer){
        delete m_AlertTimer;
        m_AlertTimer = NULL;
    }

    RequestUpload(eTT_ALERT, nPeriodicity);
    return;
}

void CMQTTUploader::RequestAlertsUpload()
{
    while (!m_bShutdownRequested)
    {
        if(!m_bAlertsAvailable) 
        {
            Wait(eTT_ALERT); // wait for alerts notification
        }

        if (m_bUploadSuspended && !m_bShutdownRequested)
        {
            HCPLOG_I << "Upload suspended... sleeping for " 
                     << SERVICE_RESUME_RETRY_WAIT_TIME << "secs";
            WaitWhileSuspended(SERVICE_RESUME_RETRY_WAIT_TIME);
            continue;
        }

        //check if offline data is already cleared
        if (!m_pMqClient->IsConnected()) {
            WaitWhileSuspended(UPLOAD_RETRY_WAIT_TIME);
            continue;
        }

        m_bAlertsAvailable = false;
        ProcessAlertUploads();
        
        if (m_bShutdownRequested) {
            //Shutdown requested end gracefully
            return;
        }

    }//while (true)
}

CMQTTUploader::UploadError CMQTTUploader::ProcessPublishedAlerts(
                                    const int &rnMid,
                                    const std::vector <long long> &rvectRowIDs)
{
    if(rnMid <= 0)
    {
        HCPLOG_W << "Mid received is Invalid, disconnecting MQTT :"<<rnMid;
        m_pMqClient->disconnect();
        return eUE_DATA_NOT_AVAILABLE;
    }
    HCPLOG_D << "update events:" << rvectRowIDs.size() << " mid:"<< rnMid;

    bool bUpdateMid_result = update_mid(ic_core::CDataBaseConst::
                                    TABLE_ALERT_STORE, rvectRowIDs, rnMid);

    if (!bUpdateMid_result)
    {
        HCPLOG_W << "Error in updateMid";
        return eUE_DATA_NOT_AVAILABLE;
    }

    CMidHandler::GetInstance()->SetMidTable(rnMid,ic_core::CDataBaseConst::
                                                        TABLE_ALERT_STORE);
    #if defined(TEST_HIGH_FREQ)
    uaCnt += rvectRowIDs.size();
    if(tUaCnt > uaCnt)
    {
        uaCntIter++;
    }
    tUaCnt = uaCnt;
    #endif

    return eUE_UPLOAD_SUCCESS;
}

void CMQTTUploader::ProcessAlertUploads()
{
    int nCount = 0;
    UploadError eErrGenericUpload(eUE_DATA_NOT_AVAILABLE);

    while (nCount < RETRY_COUNT && !m_bShutdownRequested)
    {
        if (nCount > 0)
        {
            // wait only after first attempt
            sleep(1);//wait till alerts are available from database
        }

        eErrGenericUpload = UploadAlerts();
        if (eErrGenericUpload == eUE_UPLOAD_SUCCESS) 
        {
            HCPLOG_I << "Alerts Uploaded";
        }
        nCount++;
    }
}

CMQTTUploader::UploadError CMQTTUploader::UploadAlerts()
{
    HCPLOG_METHOD();
    UploadError eErr(eUE_NO_CONNECTION);
    if (m_pMqClient->IsConnected())
    {
        std::string strAlerts;
        std::vector <long long> vectRowIDs;
        eErr = eUE_DATA_NOT_AVAILABLE;
        while(!m_bShutdownRequested)
        {
            std::string strLogStr = "";
            CUploadUtils::GetStreamingEventsFromDB(strLogStr,
                                    ic_core::CDataBaseConst::TABLE_ALERT_STORE,
                                    EVENT_COUNT, &vectRowIDs, strAlerts);

            if (vectRowIDs.size() == 0)
            {
                HCPLOG_I << "No Pending alerts to Upload";
                break;
            }
            else
            {
                HCPLOG_C << strLogStr;

                eErr = eUE_UPLOAD_SUCCESS;
                int nMid;
                HCPLOG_T << "Alerts selected for Upload:" << vectRowIDs.size() 
                         << "; " << strAlerts;
                int nResult = m_pMqClient->PublishAlerts(&nMid, 
                                        strAlerts.c_str(), strAlerts.length());
                if(0 != nResult)
                {
                    HCPLOG_E << "Error in uploading alerts: " << nResult;
                    eErr = eUE_PUBLISH_ERROR;
                    break;
                }

                eErr = ProcessPublishedAlerts(nMid,vectRowIDs);

                if(eUE_UPLOAD_SUCCESS != eErr)
                {
                    break;
                }

                vectRowIDs.clear();
                strAlerts.clear();
                HCPLOG_I << " Uploading alerts succeed , check next batch of alerts..";
            }
        }
    }
    return eErr;
}

bool CMQTTUploader::ReloadPeriodicity()
{
    HCPLOG_METHOD();
    m_bIsPeriodicityChanged = true;
    //get periodicity
    m_unEventPeriodicity = ic_core::CIgniteConfig::GetInstance()->GetInt(
                                       "MQTT.pub_topics.events.periodicity", 5);
    HCPLOG_I << "starting  Events timer with duration: " 
             << m_unEventPeriodicity;
    return true;
}

int CMQTTUploader::StartEventsUpload()
{
    HCPLOG_METHOD();

    ReloadPeriodicity();
    m_bIsPeriodicityChanged = false;

    while (1)
    {
        if (!m_bUploadRequested)
        {
            if (m_bShutdownRequested)
            {
                HCPLOG_C << "Shutdown requested";
#if defined (__ANDROID__)
                abort();
#else
                //std::exit(0);
                return 0;
#endif
                return 0;
            }

            WaitForNextUploadRequest();
            m_bUploadRequested = true;
        }

        while (m_bUploadSuspended && !m_bShutdownRequested)
        {
            HCPLOG_I << "Upload suspended... sleeping for " 
                     << m_unEventPeriodicity << "secs";
            WaitWhileSuspended(m_unEventPeriodicity);
        }

        if (m_bUploadRequested)
        {
            TriggerEventUploads();
        }
    }
}

void CMQTTUploader::WaitForNextUploadRequest()
{
    int nPeriodicCount = m_unEventPeriodicity*2;
    while (0 < nPeriodicCount && !m_bShutdownRequested)
    {
        if (m_bIsPeriodicityChanged)
        {
            m_bIsPeriodicityChanged = false;
            break;
        }
        usleep(500000);
        nPeriodicCount--;
    }
}

void CMQTTUploader::TriggerEventUploads()
{
    m_bUploadRequested = false;

    if (!m_pMqClient->IsConnected()) 
    {
        HCPLOG_I << "Upload is blocked! Connected:" 
                    << m_pMqClient->IsConnected();
        ResetEventUploaderLogCounter();
        return;
    }
    else 
    {
        //do nothing
    }

    if (!UploadTopicedEvents())
    {
        HCPLOG_I << "Topic based Events Uploaded Successfully;";
    }

    if (!UploadEvents())
    {
        HCPLOG_I << "Events Uploaded Successfully;";
    }

    #if defined(TEST_HIGH_FREQ)
    HCPLOG_W << "UploadEvtCnt: E-" << ueCnt << "E-ITR:" << ueCntIter 
                << " TE-" << uteCnt << "TE-ITR:" << uteCntIter 
                << " EAE-" << ueaCnt << "EAE-ITR:" << ueaCntIter  
                << " A-" << uaCnt << "A-ITR:" << uaCntIter ;
    #endif
}

int CMQTTUploader::UploadTopicedEvents()
{
    HCPLOG_METHOD();
    int nErr = -1;
    if (m_pMqClient->IsConnected())
    {
        std::vector <long long> vectRowIDs;
        // Map of colId, pair(topic, eventdata)
        std::map<long long, std::pair<std::string, std::string>> mapTopicedEvent;
        do {

            CUploadUtils::GetTopicedStreamingEventsFromDB(
                ic_core::CDataBaseConst::TABLE_EVENT_STORE,
                m_unMaxEventUploadCnt, &vectRowIDs,
                mapTopicedEvent);
            if (mapTopicedEvent.size() > 0)
            {
                nErr = UploadAllTopicedEvent(mapTopicedEvent);
                if (nErr != 0)
                {
                    HCPLOG_I << "error in uploading topicad events : " << nErr;
                    break;
                }
#if defined(TEST_HIGH_FREQ)
                else {
                    uteCnt += mapTopicedEvent.size();
                    if(tUteCnt > uteCnt)
                    {
                        uteCntIter++;
                    }
                    tUteCnt = uteCnt;
                }
                #endif
            }
            else
            {
                HCPLOG_D <<" No More topiced events to upload";
                break;
            }
        } while ((mapTopicedEvent.size() == m_unMaxEventUploadCnt) 
                  && !m_bShutdownRequested);
    }
    return nErr;
}

int CMQTTUploader::UploadAllTopicedEvent(
    std::map<long long, std::pair<std::string, std::string>> &rmapTopicedEvent)
{
    int nErr = -1;
    std::map<long long, std::pair<std::string, std::string>>::iterator iterStart 
                                                    = rmapTopicedEvent.begin();
    for (; iterStart != rmapTopicedEvent.end(); iterStart++)
    {
        std::string strTopic = (iterStart->second).first;
        std::string strEventData = (iterStart->second).second;
        long long llRowId = iterStart->first;
        int nMid;
        HCPLOG_C << " Uploading event: " << strEventData << " and topic = " 
                 << strTopic;

        if (!PrepareDataAndPublish(nMid, strEventData, nErr, strTopic))
        {
            continue;
        }

        //wait till we get callback to delete events from db
        //deleting them for now if err is 0
        if (!nErr)
        {
            std::vector<long long> rowIDs;
            rowIDs.push_back(llRowId);
            if (nMid <= 0)
            {
                HCPLOG_W << "Mid received is Invalid, disconnecting MQTT :" 
                         << nMid;
                m_pMqClient->disconnect();
                nErr = -1;
                break;
            }
            HCPLOG_I << "updating events:" << rowIDs.size() << " mid:" << nMid;
            bool bUpdateMidResult = update_mid(
                                    ic_core::CDataBaseConst::TABLE_EVENT_STORE, 
                                    rowIDs, nMid);
            if (!bUpdateMidResult)
            {
                HCPLOG_W << "Error in update_mid";
                nErr = -1;
                break;
            }
            CMidHandler::GetInstance()->SetMidTable(
                              nMid, ic_core::CDataBaseConst::TABLE_EVENT_STORE);
        }
        else
        {
            HCPLOG_E << "Error in uploading allTopicEvents: " << nErr;
            break;
        }
    }
    return nErr;
}

bool CMQTTUploader::PrepareDataAndPublish(int &rnMid,
                                          const std::string &rstrEventData,
                                          int &rnErr,
                                          const std::string &rstrTopic)
{
    if (m_bCompression)
    {
        ic_utils::CZippedMsg *pData = ic_utils::CIgniteGZip::GzipMsg(
                                         (unsigned char*) rstrEventData.c_str(),
                                         rstrEventData.size());
        if (NULL == pData)
        {
            HCPLOG_E << "Compression Error,retry in next iteration";
            return false;
        }

        rnErr = m_pMqClient->PublishEventsOnTopic(&rnMid, pData->m_puchBuf, 
                                              (int)pData->m_unSize, rstrTopic);
        delete pData;
        pData = NULL;
    }
    else
    {
        //No compression
        rnErr = m_pMqClient->PublishEventsOnTopic(&rnMid, rstrEventData.c_str(),
                                            rstrEventData.length(), rstrTopic);
    }
    return true;
}

bool CMQTTUploader::PublishEvents(int &rnErr, int &rnMid,
                                 std::string &rstrEvents)
{
    if (m_bCompression)
    {
        ic_utils::CZippedMsg* pData = ic_utils::CIgniteGZip::
                                   GzipMsg((unsigned char*) rstrEvents.c_str(),
                                   rstrEvents.size());
        if (NULL == pData)
        {
            HCPLOG_E << "Compression Error!";
            return false;
        }
        rnErr = m_pMqClient->PublishEvents(&rnMid,pData->m_puchBuf,
                                            (int)pData->m_unSize);
        delete pData;
    }
    else
    {
        //No compression
        rnErr = m_pMqClient->PublishEvents(&rnMid,rstrEvents.c_str(),
                                         rstrEvents.length());
    }

    return true;
}

bool CMQTTUploader::VerifyPostPublish(const int &rnMid, int &rnErr,
                                    const std::vector <long long> rvectRowIDs)
{
    if (!rnErr)
    {
        if (rnMid <= 0)
        {
            HCPLOG_W << "Mid received is Invalid, disconnecting MQTT :" <<
                                                                        rnMid;
            m_pMqClient->disconnect();
            rnErr = -1;
            return false;
        }

        HCPLOG_I << "updating events:" << rvectRowIDs.size();
        bool bUpdateMid_result = update_mid(ic_core::CDataBaseConst::
                                                    TABLE_EVENT_STORE,
                                                    rvectRowIDs, rnMid);
        if (!bUpdateMid_result)
        {
            HCPLOG_W << "Error in updateMid";
            rnErr = -1;
            return false;
        }

        CMidHandler::GetInstance()->SetMidTable(rnMid,ic_core::CDataBaseConst::
                                                            TABLE_EVENT_STORE);

        #if defined(TEST_HIGH_FREQ)
        ueCnt += rvectRowIDs.size();
        if(tUeCnt > ueCnt)
        {
            ueCntIter++;
        }
        tUeCnt = ueCnt;
        #endif

        return true;
    }
    else{
        HCPLOG_E << "Error in uploading events: " << rnErr;
        return false;
    }
}

int CMQTTUploader::UploadEvents(void)
{
    HCPLOG_METHOD();
    int nErr = -1;
    if (m_pMqClient->IsConnected())
    {
        //get events from db and upload to mqtt server
        std::string strEvents;
        std::vector<long long> vecRowIDs;
        do
        {
            std::string strLogStr="";
            CUploadUtils::GetStreamingEventsFromDB(strLogStr, 
                                    ic_core::CDataBaseConst::TABLE_EVENT_STORE, 
                                    m_unMaxEventUploadCnt, &vecRowIDs, 
                                    strEvents);

            if (vecRowIDs.size() > 0)
            {
                LogUploadEvntPayload(strLogStr, vecRowIDs.size());

                int nMid;
                HCPLOG_I << "events selected for Upload:" << vecRowIDs.size();
                bool bPubResult = PublishEvents(nErr, nMid, strEvents);

                //wait till we get callback to delete events from db
                //deleting them for now if nErr is 0
                if (!bPubResult)
                {
                    //retry in next iteration
                    continue;
                }

                if (!VerifyPostPublish(nMid, nErr, vecRowIDs))
                {
                    break;
                }
            }
        } while ((vecRowIDs.size() == m_unMaxEventUploadCnt) && 
                 !m_bShutdownRequested);
    }
    return nErr;
}

int CMQTTUploader::StartMQTTUpload()
{
    int nRetValue = -1;
    HCPLOG_METHOD();
    m_bPauseUploader = false;
    HCPLOG_I << " Staring Alerts Upload..";
    StartAlertsUpload();
    HCPLOG_I << " Staring Events Upload..";
    nRetValue = StartEventsUpload();
    return nRetValue;
}

void CMQTTUploader::Run()
{
    HCPLOG_METHOD();
    SetCurrentThreadName("CMQTTUploader");

    //Register to get the Shutdown Notification
    ic_core::CIgniteClient::GetOnOffMonitor()->RegisterForShutdownNotification(
                                        this,ic_core::IOnOff::eR_MQTT_UPLOADER);
    
    HCPLOG_C <<"Started";

    int nRetValue = -1;

    if (m_pMqClient)
    {
        m_pMqClient->StartClient();
        nRetValue = StartMQTTUpload();
        HCPLOG_D << "CMQTTUploader thread ended with " << nRetValue;
    }
    else
    {
        HCPLOG_E << "MQTTClient not created";
    }

    ic_core::CIgniteClient::GetOnOffMonitor()->ReadyForShutdown(
                                            ic_core::IOnOff::eR_MQTT_UPLOADER);
    ic_core::CIgniteClient::GetOnOffMonitor()->UnregisterForShutdownNotification(
                                            ic_core::IOnOff::eR_MQTT_UPLOADER);

    Detach();
}

int CMQTTUploader::RequestUpload(TimerType timerType, time_t delay)
{
    int nRetVal = -1;
    if (timerType == eTT_EVENT )
    {
        if (m_EventTimer)
        {
            delete m_EventTimer;
            m_EventTimer = NULL;
        }

        if (delay > 0)
        {
            m_EventTimer = new CIgniteTimer(this, delay, timerType) ;
            m_EventTimer->Start();
            return 0;
        }
        m_bUploadRequested = true;
        Notify(eTT_EVENT);
        nRetVal = 0;
    }
    else if(timerType == eTT_ALERT )
    {
        if (NULL == m_AlertTimer )
        {
            m_AlertTimer = new CIgniteTimer(this, delay, timerType);
            m_AlertTimer->Start();
            return 0;
        }
        RequestAlertsUpload();
        nRetVal = 0;
    }
    else
    {
        //Invlaid timer
    }
    return nRetVal;
}

int CMQTTUploader::ForceUpload(bool bExitWhenDone)
{
    int nUploadVal = -1;
    if (bExitWhenDone)
    {
        m_bShutdownRequested = true;
        /* Postpone upload a bit to let all the pending events make it to 
         * the DB before shutdown
         */
        Notify(eTT_ALERT); //notify alerts
        nUploadVal = RequestUpload(eTT_EVENT, 5);
    }
    else
    {
        Notify(eTT_ALERT); //notify alerts
        nUploadVal = RequestUpload(eTT_EVENT);
    }
    return nUploadVal;
}

void CMQTTUploader::Wait(TimerType timerType)
{
    if (timerType == eTT_EVENT)
    {
        m_EventWaitMutex.TryLock();
        m_EventWaitCondition.ConditionWait(m_EventWaitMutex);
    }
    else if(timerType == eTT_ALERT )
    {
        m_AlertWaitMutex.TryLock();
        m_AlertWaitCondition.ConditionWait(m_AlertWaitMutex);
    }
    else
    {
        //Invlaid timer
    }

}

void CMQTTUploader::Notify(TimerType timerType)
{
    if (timerType == eTT_EVENT)
    {
        m_EventWaitCondition.ConditionBroadcast();
        m_EventWaitMutex.Unlock();
    }
    else if(timerType == eTT_ALERT )
    {
        m_AlertWaitCondition.ConditionBroadcast();
        m_AlertWaitMutex.Unlock();
    }
    else
    {
        //Invlaid timer
    }

}

void CMQTTUploader::WaitWhileSuspended(unsigned int timeOut)
{

    std::unique_lock<std::mutex> lk(m_WaitMutex);
    m_WaitCondition.wait_for(lk,std::chrono::milliseconds(timeOut*1000));
}

void CMQTTUploader::NotifySuspended()
{

    m_WaitCondition.notify_all();
}

CMQTTUploader::CIgniteTimer::CIgniteTimer(CMQTTUploader *pUploader, time_t delay, 
                                          TimerType timerType)
{
    m_Delay = (delay < 0) ? 1 : delay;
    m_pUploader = pUploader;
    m_Timetype = timerType;
}

void CMQTTUploader::CIgniteTimer::Run()
{
    if (NULL != m_pUploader)
    {
        sleep(m_Delay);
        m_pUploader->RequestUpload(m_Timetype);
    }
}

int CMQTTUploader::CIgniteTimer::Start()
{
    int nRet = ic_utils::CIgniteThread::Start();
    Detach();
    return nRet;
}

bool CMQTTUploader::IsForceUploadEvent(const std::string &rstrEventID)
{
    return (m_setForceUploadEvents.find(rstrEventID) != 
            m_setForceUploadEvents.end());
}

bool CMQTTUploader::SuspendUpload()
{
    HCPLOG_METHOD() << " - suspending stream upload...";
    m_bUploadSuspended = true;
    return true;
}

void CMQTTUploader::ResumeUpload()
{
    HCPLOG_METHOD() << " - resuming stream upload...";
    m_bUploadSuspended = false;
}

void CMQTTUploader::NotifyShutdown()
{
    HCPLOG_D << "Shutdown Request Recieved for CMQTTUploader";
    m_bShutdownRequested = true;
    /* Postpone upload a bit to let all the pending events make it to the DB 
     * before shutdown
     */
    Notify(eTT_ALERT); //notify alerts    
    NotifySuspended();
}

bool CMQTTUploader::LogUploadEvntPayload(std::string &rstrLogEvntStr, 
                                         int nEvntCnt)
{
    bool bFlag = true;
    //infinite logging in case of -1
    if (-1 == m_nEvntUploadLogIterCount) 
    {
        HCPLOG_C << rstrLogEvntStr;
        return bFlag;
    }

    m_unCurrEvntUploadCnt++;
    if (m_unCurrEvntUploadCnt <= m_nEvntUploadLogIterCount) 
    {
        HCPLOG_C << rstrLogEvntStr;
    }
    else 
    {
        m_unCurrEvntUploadSummaryCnt++;
        m_unCurrUploadedEvntCnt += nEvntCnt;

        if (m_unCurrEvntUploadSummaryCnt >= m_nEvntUploadSummaryIterCount) 
        {
            HCPLOG_C << "# of uploads: " << m_unCurrEvntUploadSummaryCnt << 
                        "; Events uploaded " << m_unCurrUploadedEvntCnt;
            m_unCurrEvntUploadSummaryCnt = 0;
            m_unCurrUploadedEvntCnt = 0;
        }
        else{
            bFlag = false;
        }
    }
    return bFlag;
}

void CMQTTUploader::NotifyConfigUpdate()
{
    HCPLOG_D << "notifyConfigUpdate";
    ic_utils::Json::Value jsonUploadEvntLogCfg = 
                            ic_core::CIgniteConfig::GetInstance()->GetJsonValue(
                                "FileLogger.uploadEventLogging");
    if (jsonUploadEvntLogCfg.isMember("count") && 
        jsonUploadEvntLogCfg["count"].isInt())
    {
        int nEventUploadLogIterCount = jsonUploadEvntLogCfg["count"].asInt();

        if(nEventUploadLogIterCount != m_nEvntUploadLogIterCount)
        {
            m_nEvntUploadLogIterCount = nEventUploadLogIterCount;
            m_pMqClient->SetPUBACKLogStatus(m_nEvntUploadLogIterCount);
            HCPLOG_D << "uploadEventLogging.count" << m_nEvntUploadLogIterCount;
        }
    }
    if (jsonUploadEvntLogCfg.isMember("summaryLogIterCount") && 
        jsonUploadEvntLogCfg["summaryLogIterCount"].isInt())
    {
        int evntUploadSummaryIterCount = 
                            jsonUploadEvntLogCfg["summaryLogIterCount"].asInt();
        if(evntUploadSummaryIterCount != m_nEvntUploadSummaryIterCount)
        {
            m_nEvntUploadSummaryIterCount = evntUploadSummaryIterCount;
            HCPLOG_D << "uploadEventLogging.summaryLogIterCount" 
                     << m_nEvntUploadSummaryIterCount;
        }
    }
    else
    {
        m_nEvntUploadSummaryIterCount = DEF_UPLOAD_EVENT_SUMMARY_CNT;
    }

    InitEventsUploadCnt();
}

bool CMQTTUploader::ResetEventUploaderLogCounter()
{
    m_unCurrEvntUploadCnt = 0;
    m_unCurrEvntUploadSummaryCnt = 0;
    m_unCurrUploadedEvntCnt = 0;
    m_pMqClient->ResetPUBACKLogCounter();
    return true;
}

bool CMQTTUploader::InitConfigBasedLogging()
{
    m_nEvntUploadLogIterCount = -1;
    m_nEvntUploadSummaryIterCount = DEF_UPLOAD_EVENT_SUMMARY_CNT;
    ic_utils::Json::Value jsonUploadEvntLogCfg = 
                                  ic_core::CIgniteConfig::GetInstance()->
                                  GetJsonValue("FileLogger.uploadEventLogging");

    if (jsonUploadEvntLogCfg.isMember("count") && 
        jsonUploadEvntLogCfg["count"].isInt())
    {
        if (SetUploadEvntLogCount(jsonUploadEvntLogCfg["count"].asInt()))
        {
            m_pMqClient->SetPUBACKLogStatus(m_nEvntUploadLogIterCount);

            if (jsonUploadEvntLogCfg.isMember("summaryLogIterCount") 
                && jsonUploadEvntLogCfg["summaryLogIterCount"].isInt())
            {
                bool bStatus = SetSummaryLogIterCount(
                                        jsonUploadEvntLogCfg["count"].asInt());
                if (!bStatus)
                {
                    /* If ‘count’>0 & summaryLogIterCount is -1 or 0, it is 
                     * invalid hence default is 20
                     */
                }
            }
            else
            {
                //if ‘count’>0 & summaryLogIterCount is not present: default  20
            }
        }
        else
        {
            //if count present & value is -1 or 0:- no restrictions
        }
    }
    else
    {
        //if count not present & value is -1 or 0:- no restrictions
    }
    
    HCPLOG_C << "LogIterCnt~" << m_nEvntUploadLogIterCount 
             << ";SummaryIterCnt~" << m_nEvntUploadSummaryIterCount;
    return true;
}

bool CMQTTUploader::SetUploadEvntLogCount(const int &rCnt)
{
    if (rCnt > 0)
    {
        //taking count value: - as per the configured number
        m_nEvntUploadLogIterCount = rCnt;
        return true;
    }
    else
    {
         //if count present & value is -1 or 0:- no restrictions
        return false;
    }
}

bool CMQTTUploader::SetSummaryLogIterCount(const int &rCnt)
{
    if (rCnt > 0)
    {
        //if ‘count’>0 & summaryLogIterCount is > 0: - as per the configured no
        m_nEvntUploadSummaryIterCount = rCnt;
        return true;
    }
    else
    {
        /* If ‘count’>0 & summaryLogIterCount is -1 or 0, it is invalid 
         * hence default is 20
         */
        return false;
    }
}

std::string CMQTTUploader::GetComponentStatus()
{
    ic_utils::Json::Value jsonMqttUploaderJson;

    jsonMqttUploaderJson["UploadSuspended"] = m_bUploadSuspended;

    ic_utils::Json::FastWriter jsonFastWriter;
    std::string strDiagString(jsonFastWriter.write(jsonMqttUploaderJson));

    // FastWriter introduces newline at the end , that needs to be truncated
    strDiagString.erase(
                std::remove(strDiagString.begin(), strDiagString.end(), '\n'), 
                strDiagString.end());

    return strDiagString;
}

}// namespace ic_bl
