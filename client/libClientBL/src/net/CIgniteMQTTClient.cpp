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

#include "CIgniteMQTTClient.h"
#include "CIgniteLog.h"
#include "CIgniteConfig.h"
#include "CIgniteDateTime.h"
#include "notif/CNotificationListener.h"
#include "net/CExponentialBackoff.h"
#include "HttpErrorCodes.h"
#include "auth/CTokenManager.h"
#include "db/CLocalConfig.h"
#include "upload/CUploadController.h"
#include <unistd.h>
#include <CIgniteStringUtils.h>
#include <CIgniteEvent.h>
#include <auth/CActivationBackoff.h>
#include <CIgniteHTTPConnector.h>
#include "CIgniteClient.h"
#include "CHttpResponse.h"
#include "upload/CMidHandler.h"

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CIgniteMQTTClient"

using namespace ic_network;

#define MAX_INFLIGHT_MESSAGES "maxInflightMessages"

namespace ic_bl
{

int CIgniteMQTTClient::m_nNoOfTopicsToSubscribe = 0;
int CIgniteMQTTClient::m_nNoOfTopicSubAckRcvd = 0;

bool CIgniteMQTTClient::GetCredentials(std::string &rstrUsername,
                                      std::string &rstrPassword,
                                      std::string &rstrClientid)
{
    ic_utils::Json::Value jsonMqRoot = ic_core::CIgniteConfig::GetInstance()->
                                                           GetJsonValue("MQTT");

    if (jsonMqRoot.isMember("username") && jsonMqRoot.isMember("pwd_val"))
    {
        rstrUsername = jsonMqRoot["username"].asString();
        rstrPassword = jsonMqRoot["pwd_val"].asString();
    }
    else 
    {
        rstrPassword = GetTokenAsPassword();

        /* if m_bIsShutDownInitiated or m_bStopClientReq is set, no need to
         * proceed further
         */
        if (m_bIsShutDownInitiated || m_bStopClientReq)
        {
            return false;
        }

        HCPLOG_D << "tok:[" << rstrPassword << "]";
    }
    
    m_strUnameprefix = jsonMqRoot["unameprefix"].asString();
    bool bUseDevType = ic_core::CIgniteConfig::GetInstance()->
                                                       GetBool("useDeviceType");

    if (bUseDevType) 
    {
        std::string strProductType = ic_core::CIgniteConfig::GetInstance()->
                                                       GetString("ProductType");

        if (strProductType.empty() 
            || m_strUnameprefix.find(strProductType) == std::string::npos) 
        {
            HCPLOG_E << "ProductType is not configured in uname prefix !!!";
        }
        else 
        {
            HCPLOG_D << "Valid ProductType in uname prefix:"<< m_strUnameprefix;
        }
    }
    //got the token now, get device ID
    m_strDeviceID = ic_core::CLocalConfig::GetInstance()->Get("login");
    rstrClientid = m_strDeviceID;
    rstrUsername = m_strUnameprefix + m_strDeviceID;

    return true;
}

std::string CIgniteMQTTClient::GetTokenAsPassword()
{
    //get username and authtoken
    std::string strPassword = "";
    HttpErrorCode eErrorCode = HttpErrorCode::eERR_UNKNOWN;
    int nBackoff = 0;
    int nDefaultBackOff = 2;
    CExponentialBackoff expBackoffObj(0, nDefaultBackOff);
    
    do 
    {
        strPassword = CTokenManager::GetInstance()->GetToken(eErrorCode);

        /* use backoff to Wait here
         * If not activated and activation backoff is enable use default backoff
         * time.
         */
        if (!CTokenManager::IsActivated() 
            && CActivationBackoff::GetInstance()->IsEnable()) 
        {
            nBackoff = (HttpErrorCode::eERR_OK == eErrorCode) ? 0 
                                                              : nDefaultBackOff;
        } 
        else 
        {
            nBackoff = expBackoffObj.GetNextRetryTime(eErrorCode);
        }

        if (nBackoff) 
        {
            HCPLOG_W << "MQTT getToken error,[" 
                     << CHttpResponse::GetHttpErrorCodeString(eErrorCode) 
                     << "] retry after sec:" << nBackoff;

            if (!m_bStopClientReq && !m_bIsShutDownInitiated) 
            {
                Wait(nBackoff);
            }
        }
    } 
    while ((HttpErrorCode::eERR_OK != eErrorCode) 
            && (!m_bIsShutDownInitiated) && (!m_bStopClientReq));

    return strPassword;
}

bool CIgniteMQTTClient::GetMaxInflightMsgCnt(unsigned int &unMaxMsgCnt)
{
    bool bStatus = false; //default

    //read max-inflight-msg-count if configured
    ic_utils::Json::Value jsonMqRoot = 
                    ic_core::CIgniteConfig::GetInstance()->GetJsonValue("MQTT");
    if (jsonMqRoot != ic_utils::Json::nullValue &&
        jsonMqRoot.isMember(MAX_INFLIGHT_MESSAGES) &&
        jsonMqRoot[MAX_INFLIGHT_MESSAGES].isUInt())
    {
        unMaxMsgCnt = jsonMqRoot[MAX_INFLIGHT_MESSAGES].asUInt();
        bStatus = true;
    }

    return bStatus;
}

CIgniteMQTTClient::CIgniteMQTTClient() : m_pConnNotif(NULL), 
                    m_nPUBACKLogMaxCnt(-1), 
                    m_eLastBroadcasted(ic_core::eSTATE_NOT_CONNECTED), 
                    m_nPUBACKCurrLogCnt(0)
{
    HCPLOG_METHOD();

    // initialize library
    ic_utils::Json::Value jsonNetMqRoot = 
                ic_core::CIgniteConfig::GetInstance()->GetJsonValue("NET.MQTT");

    // get and set tls options
    if (jsonNetMqRoot != ic_utils::Json::nullValue 
        && jsonNetMqRoot.isMember("SSL") && jsonNetMqRoot["SSL"].isObject())
    {
        ic_utils::Json::Value jsonSsl = jsonNetMqRoot["SSL"];

        std::string strCAFile = GetCAFile(jsonSsl);
        HCPLOG_I << "cafile : " << strCAFile;

        std::string strCAPath = GetCAPath(jsonSsl);
        HCPLOG_I << "capath : " << strCAPath;

        std::string strCertfile = GetCertFile(jsonSsl);
        HCPLOG_I << "clientcert path: " << strCertfile;

        std::string strKeypath = GetKeyPath(jsonSsl);
        bool bInsecure = GetInsecureStatus(jsonSsl);
        bool bUseTLSEngine = GetUseTLSEngineStatus(jsonSsl);
        std::string strTLSEngineID = GetTLSEngineID(jsonSsl);

        m_stConfig.m_pstSSL = new SSLAttributes(strCAFile, strCAPath, strCertfile
                                                ,strKeypath, bInsecure
                                                ,bUseTLSEngine, strTLSEngineID);
    }
    
    ic_utils::Json::Value jsonMqRoot = 
                    ic_core::CIgniteConfig::GetInstance()->GetJsonValue("MQTT");
    if (jsonMqRoot != ic_utils::Json::nullValue)
    {
        m_strTopicprefix = jsonMqRoot["topicprefix"].asString();

        m_stConfig.m_nKeepAlive = jsonMqRoot.isMember("keepalive") 
                                  ? jsonMqRoot["keepalive"].asInt() : 15;
        m_stConfig.m_strHost = jsonMqRoot["host"].asString();
        m_stConfig.m_nPort = jsonMqRoot["port"].asInt();
    }
}

std::string CIgniteMQTTClient::GetCAFile(const ic_utils::Json::Value &rjsonSsl)
{
    return rjsonSsl.isMember("cafile") ? rjsonSsl["cafile"].asString() : "";
}

std::string CIgniteMQTTClient::GetCAPath(const ic_utils::Json::Value &rjsonSsl)
{
    return rjsonSsl.isMember("capath") ? rjsonSsl["capath"].asString() : "";
}

std::string CIgniteMQTTClient::GetCertFile(const ic_utils::Json::Value &rjsonSsl)
{
    return rjsonSsl.isMember("certfile") ? rjsonSsl["certfile"].asString() : "";
}

std::string CIgniteMQTTClient::GetKeyPath(const ic_utils::Json::Value &rjsonSsl)
{
    return rjsonSsl.isMember("keypath") ? rjsonSsl["keypath"].asString() : "";
}

bool CIgniteMQTTClient::GetInsecureStatus(const ic_utils::Json::Value &rjsonSsl)
{
    return rjsonSsl.isMember("insecure") ? rjsonSsl["insecure"].asBool() : true;
}

bool CIgniteMQTTClient::GetUseTLSEngineStatus(const ic_utils::Json::Value 
                                             &rjsonSsl)
{
    return rjsonSsl.isMember("useTLSEngine") ? rjsonSsl["useTLSEngine"].asBool()
                                             : false;
}

std::string CIgniteMQTTClient::GetTLSEngineID(const ic_utils::Json::Value 
                                             &rjsonSsl)
{
    return rjsonSsl.isMember("TLSEngineID") ? rjsonSsl["TLSEngineID"].asString()
                                            : "";
}

CIgniteMQTTClient::~CIgniteMQTTClient()
{
    if(m_stConfig.m_pstSSL)
    {
        delete m_stConfig.m_pstSSL;
        m_stConfig.m_pstSSL = NULL;
    }
}

void CIgniteMQTTClient::ReloadTopics()
{
    m_strConftopic = m_strTopicprefix + m_strDeviceID + "/2d/config";
    ReloadPublishTopic();
    m_mapFeaturesTopicMap.clear();
    PopulateServicesTopic(m_strDeviceID, m_strTopicprefix);
    HCPLOG_D << "Initializing MidHandler...";
    CMidHandler::GetInstance()->InitMid();
}

CIgniteMQTTClient* CIgniteMQTTClient::GetInstance()
{
    HCPLOG_METHOD();
    static CIgniteMQTTClient mqClient;
    return &mqClient;
}

void CIgniteMQTTClient::ReloadPublishTopic()
{
    ic_utils::CScopeLock scopeLock(m_PubTopicMutex);
    ic_core::CIgniteConfig *config = ic_core::CIgniteConfig::GetInstance();
    std::string strAlertSufix("/2c/alerts");
    strAlertSufix = 
        config->GetString("MQTT.pub_topics.alerts.sufix_topic",strAlertSufix);
    m_strAlerttopic = m_strTopicprefix + m_strDeviceID + strAlertSufix;

    std::string strEventSufix("/2c/events");
    strEventSufix = 
        config->GetString("MQTT.pub_topics.events.sufix_topic",strEventSufix);
    m_strEventtopic = m_strTopicprefix + m_strDeviceID + strEventSufix;
}

int CIgniteMQTTClient::PublishAlerts(int *pnMid, const void *pvoidAlertPayload,
                                const int nSize, const std::string &rstrVendor)
{
    ic_utils::CScopeLock scopeLock(m_PubTopicMutex);
    HCPLOG_METHOD();

    /* Ensures alerts are sent only once..no duplicates
     * As alerts are less as compared to events so wont impact performance
     */
    int nQos = 
    ic_core::CIgniteConfig::GetInstance()->GetInt("MQTT.pub_topics.alerts.qos", 2);

    std::string strTopic(m_strAlerttopic);
    //if application name/vendor provided then upload data on its topic
    if (!rstrVendor.empty())
    {
        strTopic =
            m_strTopicprefix + m_strDeviceID + "/2c/" + rstrVendor + "alerts";
    }
    HCPLOG_I << "topic:" << strTopic;

    return PublishMessage(pnMid, pvoidAlertPayload, nSize, strTopic, nQos);
}

int CIgniteMQTTClient::PublishEvents(int *pnMid, const void *pvoidEventPayload,
                                const int nSize, const std::string &rstrVendor)
{
    ic_utils::CScopeLock scopeLock(m_PubTopicMutex);
    HCPLOG_METHOD();
    // default qos =1 , as duplicates are handled at backend
    int nQos = 
    ic_core::CIgniteConfig::GetInstance()->GetInt("MQTT.pub_topics.events.qos", 1);

    std::string strTopic(m_strEventtopic);
    //if application name/vendor provided then upload data on its topic
    if (!rstrVendor.empty())
    {
        strTopic = 
            m_strTopicprefix + m_strDeviceID + "/2c/" + rstrVendor + "events";
    }
    HCPLOG_I << "topic:" << strTopic;

    return PublishMessage(pnMid, pvoidEventPayload, nSize, strTopic, nQos);
}

int CIgniteMQTTClient::PublishEventsOnTopic(int *pnMid,
                                const void *pvoidEventPayload, const int nSize, 
                                const std::string &rstrtopic)
{
    HCPLOG_METHOD();
    int nQos = 1;
    return PublishMessage(pnMid, pvoidEventPayload, nSize, rstrtopic, nQos);
}

void CIgniteMQTTClient::PopulateServicesTopic(std::string strDeviceId,
                                              std::string strTopicPrefix)
{
    HCPLOG_METHOD();
    ic_utils::Json::Value jsonMqRoot = 
                    ic_core::CIgniteConfig::GetInstance()->GetJsonValue("MQTT");
    ic_utils::Json::Value jsonTopicArray = jsonMqRoot["servicesTopic"];

    HCPLOG_I << "SERVICES SIZE = " << jsonTopicArray.size();
    if(jsonTopicArray.isArray()) {
        HCPLOG_I << "SERVICES IS ARRAY " ;
    }

    for (int nItr = 0; nItr < jsonTopicArray.size(); nItr++)
    {
        ic_utils::Json::Value jsonServiceObj = jsonTopicArray[nItr];
        std::string strServiceName = jsonServiceObj["serviceName"].asString();
        if (strServiceName != "RemoteOperation")
        {
            HCPLOG_E << "INVALID TOPIC ENTRY = " << strServiceName;
            continue;
        }

        std::string strSufix =jsonServiceObj["sufix_topic"].asString();
        std::string strPub = jsonServiceObj["pub"].asString();
        std::string strSub = jsonServiceObj["sub"].asString();
        std::string strFullTopic = strTopicPrefix + strDeviceId + "/" + strSufix;
        // std::string strFullTopic = strTopicPrefix + strSufix + "/" + strDeviceId;
        std::string strPubTopic = strTopicPrefix + strDeviceId + "/" + strPub;
        std::string strSubTopic = strTopicPrefix + strDeviceId + "/" + strSub;
        int nQos = jsonServiceObj["QOS"].asInt();
        TopicInfo topic(strPubTopic, strSubTopic, nQos);
        m_mapFeaturesTopicMap[strServiceName] = topic;
        HCPLOG_I << "MAP TOPIC KEY = " << strServiceName << 
                    " And topic = " << strFullTopic;
    }
}

ic_utils::Json::Value CIgniteMQTTClient::GetCurrentConnectionState()
{
    ic_utils::Json::Value jsonStatus;
    jsonStatus["status"] = m_eLastBroadcasted;
    return jsonStatus;
}

void CIgniteMQTTClient::on_connect(int nRc)
{
    CMQTTClient::on_connect(nRc);
    if (nRc == 0)
    {
        if (m_pConnNotif)
        {
            m_pConnNotif->ConnNotificationUpdate(
                acp::IConnNotification::eUL_CONN_NOTIF_DIRECT_STREAM_CONNECTED);
        }
        else
        {
            /* do Nothing */
        }

        m_nNoOfTopicsToSubscribe = 0;
        m_nNoOfTopicSubAckRcvd = 0;

        /* Sending connection status to device as connected but 
         * subscription pending
         */
        BroadcastConnStatus(eSTEP_CONNECTED_BUT_SUBS_PENDING);

        int nConfMid = 0;
        // subscribe to config topic after successful connection
        m_nNoOfTopicsToSubscribe++;
        int nErr = Subscribe(&nConfMid, m_strConftopic);
        HCPLOG_C << "suscribe : Topic: " << m_strConftopic
                 << " ~ mid:" << nConfMid << " err:[" << nErr << "] : "
                 << mosqpp::strerror(nErr);

        //increment the topics counter based on number of more topics to subscribe
        m_nNoOfTopicsToSubscribe += m_mapFeaturesTopicMap.size();

        // Iterate over services topic to subscribe
        std::map<std::string, TopicInfo>::iterator iterItr = 
                                                m_mapFeaturesTopicMap.begin();
        for (; iterItr != m_mapFeaturesTopicMap.end(); iterItr++)
        {
            int nMid = 0;
            std::string strSer = iterItr->first;
            std::string strSerTop = (iterItr->second).GetSubTopic();
            std::string strPubTopic = (iterItr->second).GetPubTopic();

            int nErr = Subscribe(&nMid, strSerTop);
            HCPLOG_C << "Service:" << strSer << " SUB Topic: " << strSerTop
                     << " ~ mid:" << nMid << " ~ err[" << nErr << "] : "
                     << mosqpp::strerror(nErr);
            HCPLOG_I << "Service: " << strSer << "PUB Topic: " << strPubTopic;

        }

        CUploadController::GetInstance()->TriggerAlertsUpload(START_ALERT_UPLOAD);

        ic_core::CLocalConfig *pConfig = 
                (ic_core::CLocalConfig *)ic_core::CLocalConfig::GetInstance();
        std::string strDbfrmVersion = pConfig->Get("displayVersion");
        ic_core::IProduct *pProd = ic_core::CIgniteClient::GetProductImpl();
        std::string strFrmVersion = 
                            pProd->GetAttribute(ic_core::IProduct::eSWVersion);

        HCPLOG_C << "database swVersion:[" << strDbfrmVersion 
                 << "] cache swVersion:[" << strFrmVersion << "]";

        SendFirmwareVersionEvent(strDbfrmVersion, strFrmVersion);
    }
    else
    {
        /* Based on 
         * http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/errata01/os/mqtt-v3.1.1-errata01-os-complete.html#_Toc442180846
         * on_connect will return nRc values 0, 1,2,3,4,5 But now we are 
         * handling only 4 5 and 2. But if nRc = 1 connection is refused due to 
         * protocol violation, and nRc=3 Connection Refused, Server unavailable 
         * both scenarios has never came into scenario. 
         */
        if (4 == nRc || 5 == nRc || 2 == nRc)
        {
            /* This is bad username or password / not authorised / 
             * Identifier rejected error .Disconnect to come out of loop_forever 
             * and re-initialize with new authtoken
             */ 
            HCPLOG_C << "!!!disconnecting to get new authtoken!!!";
            CTokenManager::GetInstance()->InvalidateToken();
            disconnect();
        }
        else
        {
            HCPLOG_E << "Got unhandled error code rc:" << nRc;
        }
    }
}

void CIgniteMQTTClient::SendFirmwareVersionEvent(const std::string 
                           &rstrDbfrmVersion, const std::string &rstrFrmVersion)
{
    if (rstrDbfrmVersion != rstrFrmVersion)
    {
        HCPLOG_D << "send swVersion event";
        ic_event::CIgniteEvent frmVersionEvent("1.0", "FirmwareVersion");
        frmVersionEvent.AddField("value", rstrFrmVersion);
        ic_core::CLocalConfig::GetInstance()->Set("displayVersion"
                                                  , rstrFrmVersion);
        frmVersionEvent.Send();
    }
    else
    {
        /* do Nothing */
    }
}

void CIgniteMQTTClient::on_disconnect(int nRc)
{
    CMQTTClient::on_disconnect(nRc);

    m_nNoOfTopicsToSubscribe = 0;
    m_nNoOfTopicSubAckRcvd = 0;

    //Sending connection status to device as disconnected
    BroadcastConnStatus(eSTEP_DISCONNECTED);

    if (m_pConnNotif)
    {
        m_pConnNotif->ConnNotificationUpdate(
            acp::IConnNotification::eUL_CONN_NOTIF_DIRECT_STREAM_DISCONNECTED);
    }
    else
    {
        /* do Nothing */
    }
}

bool CIgniteMQTTClient::OnMessageReceived(const string &rstrTopic,
                                          const string &rstrPayload)
{
    HCPLOG_C << rstrTopic << "~payload=" << rstrPayload;
    bool bRetVal = false;
    ic_utils::Json::Value jsonPayload;
    ic_utils::Json::Reader jsonReader;
    if (jsonReader.parse(rstrPayload, jsonPayload))
    {
        // create notification to be handled by MQTTNotificationHandler
        ic_utils::Json::Value jsonMqNotification;
        jsonMqNotification["type"] = "MQTT_CONFIG_REQUEST";
        jsonMqNotification["message"] = rstrPayload;
        jsonMqNotification["topic"] = rstrTopic;

        ic_utils::Json::FastWriter writer;
        CNotificationListener::GetNotificationListener()->PushNotification(
            writer.write(jsonMqNotification));

        ic_event::CIgniteEvent confEv("1.0", "MQTTConfig");
        confEv.AddField("payload", rstrPayload);
        confEv.Send();
        bRetVal = true;
    }
    else
    {
        HCPLOG_E << "Payload parsing failed";
        bRetVal = false;
    }

    return bRetVal;
}

void CIgniteMQTTClient::on_publish(int nMid)
{
    if (ic_core::CDataBaseConst::TABLE_ALERT_STORE ==
        CMidHandler::GetInstance()->GetTableOfPublishedMid(nMid))
    {
        HCPLOG_C << "mid[" << nMid << "]";
    }
    else if ((-1 == m_nPUBACKLogMaxCnt) ||
             (m_nPUBACKCurrLogCnt <= m_nPUBACKLogMaxCnt))
    {
        m_nPUBACKCurrLogCnt++;

        HCPLOG_C << "mid[" << nMid << "]";
    }
    else
    {
        HCPLOG_D << "mid[" << nMid << "]";
    }
    CMidHandler::GetInstance()->ProcessPublishedMid(nMid);
}

void CIgniteMQTTClient::on_log(int nLevel, const char* cstrStr)
{
    if ((-1 == m_nPUBACKLogMaxCnt) ||
        (m_nPUBACKCurrLogCnt <= m_nPUBACKLogMaxCnt))
    {
        HCPLOG_C << "level:[" << nLevel << "]:" << cstrStr;
    }
    else
    {
        HCPLOG_D << "level:[" << nLevel << "]:" << cstrStr;
    }
}

void CIgniteMQTTClient::on_subscribe(int nMid, int nQosCount, 
                                     const int *pnGrantedQos)
{
    CMQTTClient::on_subscribe(nMid, nQosCount, pnGrantedQos);

    m_nNoOfTopicSubAckRcvd++;
    HCPLOG_C << "SUB:" << m_nNoOfTopicsToSubscribe << "~ACK:"
             << m_nNoOfTopicSubAckRcvd << "-mid:" << nMid << " qos_count:" 
             << nQosCount << " granted_qos:" << *pnGrantedQos;

    /* Sending connection status to device as connected and 
     * subscription Ack Received
     */
    BroadcastConnStatus(eSTEP_CONNECTED_AND_SUBS_ACK_RCVD);
}

int CIgniteMQTTClient::StopClient()
{
    HCPLOG_C << "Stopping client";

    //Sending connection status to device as graceful Disconnect Request
    BroadcastConnStatus(eSTEP_CONNECTION_TEARING_DOWN);
    CMQTTClient::StopClient();
    Notify();
    return 0;
}

void CIgniteMQTTClient::InitRun() {
    HCPLOG_METHOD();
    ic_core::CIgniteClient::GetOnOffMonitor()->RegisterForShutdownNotification(
                                        this,ic_core::IOnOff::eR_MQTT_CLIENT);
}

void CIgniteMQTTClient::ExitRun() {
    HCPLOG_METHOD();
    ic_core::CIgniteClient::GetOnOffMonitor()->ReadyForShutdown(
                                              ic_core::IOnOff::eR_MQTT_CLIENT);
    ic_core::CIgniteClient::GetOnOffMonitor()->UnregisterForShutdownNotification(
                                              ic_core::IOnOff::eR_MQTT_CLIENT);
}

void CIgniteMQTTClient::RegisterForConnNotif(acp::IConnNotification *pCNotif)
{
    m_pConnNotif = pCNotif;

    //Notify the current connection status
    if (m_bConnected) {
        m_pConnNotif->ConnNotificationUpdate(
                acp::IConnNotification::eUL_CONN_NOTIF_DIRECT_STREAM_CONNECTED);
    }
    else {
        m_pConnNotif->ConnNotificationUpdate(
            acp::IConnNotification::eUL_CONN_NOTIF_DIRECT_STREAM_DISCONNECTED);
    }
}

int CIgniteMQTTClient::DisconnectConn()
{
    HCPLOG_METHOD();
    return disconnect();
}

void CIgniteMQTTClient::Wait(unsigned int timeOut) {
    std::unique_lock<std::mutex> lk(m_WaitMutex);
    std::chrono::milliseconds ms;
    m_WaitCondition.wait_for(lk,std::chrono::milliseconds(timeOut*1000));
}

void CIgniteMQTTClient::Notify() {
    m_WaitCondition.notify_all();
}

void CIgniteMQTTClient::NotifyShutdown()
{
    HCPLOG_METHOD();
    CMQTTClient::NotifyShutdown();
    Notify();
}

void CIgniteMQTTClient::ResetPUBACKLogCounter()
{
    m_nPUBACKCurrLogCnt = 0;
}

void CIgniteMQTTClient::SetPUBACKLogStatus(int maxcnt)
{
    m_nPUBACKLogMaxCnt = maxcnt;
}

bool CIgniteMQTTClient::BroadcastConnStatus(
                        MQTT_CONNECTION_STEPS eConnState, bool bForced)
{
    bool bStatus = false;

    switch (eConnState)
    {
        case eSTEP_DISCONNECTED:
        {
            bStatus = BroadcastDisconnectStatus(bForced);
        }
        break;

        case eSTEP_CONNECTED_BUT_SUBS_PENDING:
        {
            bStatus = BroadcastConnSubsPendingStatus(bForced);
        }
        break;

        case eSTEP_CONNECTED_AND_SUBS_ACK_RCVD:
        {
            bStatus = BroadcastConnSubsAckRcvdStatus(bForced);
        }
        break;

        case eSTEP_CONNECTION_TEARING_DOWN:
        {
            bStatus = BroadcastConnTearingDownStatus(bForced);
        }
        break;

        default:
        break;
    }

    return bStatus;
}

bool CIgniteMQTTClient::BroadcastDisconnectStatus(bool bForced)
{
    bool bStatus = false;

    //if broadcasted already, not to do it again if the state is repeated
    if ((ic_core::MQTT_CONNECTION_STATE::eSTATE_NOT_CONNECTED
        != m_eLastBroadcasted) || bForced)
    {
        m_eLastBroadcasted = 
                        ic_core::MQTT_CONNECTION_STATE::eSTATE_NOT_CONNECTED;

        ic_utils::Json::Value jsonStatus;
        jsonStatus["status"] = m_eLastBroadcasted;

        //Sending status to dispatcher
        bStatus = ic_core::CIgniteClient::GetClientMessageDispatcher()->
                                DeliverMQTTConnectionStatusToDevice(jsonStatus);
    }
    else
    {
        HCPLOG_D << "State " << m_eLastBroadcasted 
                 << " broadcasted already; not sending again.";
    }

    return bStatus;
}

bool CIgniteMQTTClient::BroadcastConnSubsPendingStatus(bool bForced)
{
    bool bStatus = false;

    //if broadcasted already, not to do it again if the state is repeated
    if ((ic_core::MQTT_CONNECTION_STATE::eSTATE_CONNECTED_BUT_TOPICS_SUBS_PENDING
        != m_eLastBroadcasted) || bForced)
    {
        m_eLastBroadcasted = 
        ic_core::MQTT_CONNECTION_STATE::eSTATE_CONNECTED_BUT_TOPICS_SUBS_PENDING;

        ic_utils::Json::Value jsonStatus;
        jsonStatus["status"] = m_eLastBroadcasted;

        //Sending status to dispatcher
        bStatus = ic_core::CIgniteClient::GetClientMessageDispatcher()->
                                DeliverMQTTConnectionStatusToDevice(jsonStatus);
    }
    else
    {
        HCPLOG_D << "State " << m_eLastBroadcasted 
                 << " broadcasted already; not sending again.";
    }

    return bStatus;
}

bool CIgniteMQTTClient::BroadcastConnSubsAckRcvdStatus(bool bForced)
{
    bool bStatus = false;

    if ((0 != m_nNoOfTopicsToSubscribe) && (0 != m_nNoOfTopicSubAckRcvd)
            && (m_nNoOfTopicsToSubscribe == m_nNoOfTopicSubAckRcvd))
    {
        //if broadcasted already, not to do it again if the state is repeated
        if ((ic_core::MQTT_CONNECTION_STATE::eSTATE_CONNECTION_COMPLETE
            != m_eLastBroadcasted) || bForced)
        {
            m_eLastBroadcasted = 
                     ic_core::MQTT_CONNECTION_STATE::eSTATE_CONNECTION_COMPLETE;

            ic_utils::Json::Value jsonStatus;
            jsonStatus["status"] = m_eLastBroadcasted;

            //Sending status to dispatcher
            bStatus = ic_core::CIgniteClient::GetClientMessageDispatcher()->
                                DeliverMQTTConnectionStatusToDevice(jsonStatus);
        }
        else
        {
            HCPLOG_D << "State " << m_eLastBroadcasted 
                     << " broadcasted already; not sending again.";
        }
    }
    else
    {
        HCPLOG_C << "State " 
                 << ic_core::MQTT_CONNECTION_STATE::eSTATE_CONNECTION_COMPLETE
                 << " not broadcasted! SUB:" << m_nNoOfTopicsToSubscribe 
                 << " ~ ACK:" << m_nNoOfTopicSubAckRcvd;
    }

    return bStatus;
}

bool CIgniteMQTTClient::BroadcastConnTearingDownStatus(bool bForced)
{
    bool bStatus = false;

    if ((ic_core::MQTT_CONNECTION_STATE::eSTATE_CONNECTION_TEARING_DOWN
        != m_eLastBroadcasted) || bForced)
    {
        m_eLastBroadcasted = 
                 ic_core::MQTT_CONNECTION_STATE::eSTATE_CONNECTION_TEARING_DOWN;

        ic_utils::Json::Value jsonStatus;
        jsonStatus["status"] = m_eLastBroadcasted;

        //Sending status to dispatcher
        bStatus = ic_core::CIgniteClient::GetClientMessageDispatcher()->
                                DeliverMQTTConnectionStatusToDevice(jsonStatus);
    }
    else
    {
        HCPLOG_D << "State " << m_eLastBroadcasted 
                 << " broadcasted already; not sending again.";
    }

    return bStatus;
}

std::string CIgniteMQTTClient::GetComponentStatus()
{
    ic_utils::Json::Value mqttJson;

    mqttJson["ConnectionStatus"] = m_eLastBroadcasted;

    ic_utils::Json::FastWriter fastWriter;
    std::string strDiagString(fastWriter.write(mqttJson));

    // FastWriter introduces newline at the end , that needs to be truncated
    strDiagString.erase(std::remove(
        strDiagString.begin(), strDiagString.end(), '\n'), strDiagString.end());

    return strDiagString;
}

} //namespace ic_bl
