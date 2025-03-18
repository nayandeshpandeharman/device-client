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

#include "CMQTTClient.h"
#include "CIgniteLog.h"
#include <unistd.h>
#include <mosquitto.h>

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CMQTTClient"

namespace ic_network
{

#define MAX_INFLIGHT_MESSAGES "maxInflightMessages"
CMQTTClient::CMQTTClient() : m_bConnected(false), m_bStopClientReq(false),
                           m_bIsShutDownInitiated(false)
{
    HCPLOG_METHOD();
    // initialize library
    mosqpp::lib_init();
}

int CMQTTClient::InitClient()
{
    std::string strUsername, strPassword, strClientID;
    int retValue;
    if(!GetCredentials(strUsername, strPassword, strClientID) || 
                       m_bStopClientReq || m_bIsShutDownInitiated)
    {
        return 1; // return non-zero number to denote error occured
    }

    /* std::string strClientID = "mosq_" + deviceID;
     * removing prefix because of HiveMQ requirements to have
     * same client id as Device Id in topic
     */
    HCPLOG_C << "Reinitializing...strClientID: " << strClientID;
    reinitialise(strClientID.c_str(), true);

    unsigned int unMaxMsgCnt = 0;
    if (GetMaxInflightMsgCnt(unMaxMsgCnt) && (unMaxMsgCnt > 0))
    {
        #if defined LIBMOSQUITTO_VERSION_NUMBER && \
                    LIBMOSQUITTO_VERSION_NUMBER >= 2000015
            int retValue = set_int_option(MOSQ_OPT_SEND_MAXIMUM, unMaxMsgCnt);
            HCPLOG_C << "Set max-inflight-msg-cnt:"<< unMaxMsgCnt <<
                    "-set_int_option()->" << retValue;
        #else
            int retValue = max_inflight_messages_set(unMaxMsgCnt);
            HCPLOG_C << "Set max-inflight-msg-cnt:"<< unMaxMsgCnt <<
                        "-max_inflight_messages_set()->" << retValue;
        #endif
    }
    else
    {
        HCPLOG_C << "No valid max-inflight-msg-cnt is set (" << 
                   unMaxMsgCnt << ")-Using default.";
    }
    threaded_set(true);

    if (m_stConfig.m_pstSSL != NULL)
    {
        HCPLOG_D << "setting SSL properties";
        tls_opts_set(1, "tlsv1.2", "ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256");

        if(!access(m_stConfig.m_pstSSL->m_strCaFile.c_str(),F_OK)){
            if(access(m_stConfig.m_pstSSL->m_strCaFile.c_str(),R_OK)){
                HCPLOG_E << "can not read cafile.";
                return 1; // return non-zero number to denote error occured
            }
        } else{
            HCPLOG_E << "cafile does not exists";
            return 1;
        }
/* This code is commented now.This was added for 2wayTLS EngineSupport test,
 * Need to enable when required.
 */
#if 0 //defined LIBMOSQUITTO_VERSION_NUMBER && \ 
      //          LIBMOSQUITTO_VERSION_NUMBER >= 1006012
        if (m_stConfig.m_pstSSL->m_bUseTLSEngine)
        {
            retValue = set_string_option(MOSQ_OPT_TLS_ENGINE, 
                        (char *)m_stConfig.m_pstSSL->m_strTLSEngineID.c_str());
            HCPLOG_C << "Mosquitto Set TLS Engine status "<<retValue;
            if(retValue != MOSQ_ERR_SUCCESS)  {
                return retValue;
            }
            tls_set(m_stConfig.m_pstSSL->m_strCaFile.empty() ? NULL :
                                    m_stConfig.m_pstSSL->m_strCaFile.c_str(),
                    m_stConfig.m_pstSSL->m_strCaPath.empty() ? NULL : 
                                    m_stConfig.m_pstSSL->m_strCaPath.c_str(),
                    m_stConfig.m_pstSSL->m_strCertFile.empty() ? NULL : 
                                    m_stConfig.m_pstSSL->m_strCertFile.c_str(),
                    NULL); //Key path is not set, as key is available in engine
        }
        else 
        {
            SetTLSValues();
        }
#else
        SetTLSValues();
#endif

        HCPLOG_C << "is insecure: " << m_stConfig.m_pstSSL->m_bInsecure;
        tls_insecure_set(m_stConfig.m_pstSSL->m_bInsecure);
    }
    else
    {
        HCPLOG_E << "SSL properties not found/set!";
    }

    ReloadTopics();

    // set strUsername and strPassword
    username_pw_set(strUsername.c_str(), strPassword.c_str());
    HCPLOG_T << "USR:[" << strUsername << "] PWD:[" << strPassword << "]";

    //connect to broker
    int err = connect(m_stConfig.m_strHost.c_str(), 
                      m_stConfig.m_nPort, m_stConfig.m_nKeepAlive);
    HCPLOG_C << "connect call: err[" << err << 
                "]:[" << mosqpp::strerror(err) << "]";

    return err;
}

void CMQTTClient::SetTLSValues()
{
    tls_set(m_stConfig.m_pstSSL->m_strCaFile.empty() ? NULL : 
                                    m_stConfig.m_pstSSL->m_strCaFile.c_str(),
            m_stConfig.m_pstSSL->m_strCaPath.empty() ? NULL : 
                                    m_stConfig.m_pstSSL->m_strCaPath.c_str(),
            m_stConfig.m_pstSSL->m_strCertFile.empty() ? NULL :
                                    m_stConfig.m_pstSSL->m_strCertFile.c_str(),
            m_stConfig.m_pstSSL->m_strKeyPath.empty() ? NULL : 
                                    m_stConfig.m_pstSSL->m_strKeyPath.c_str());
}

int CMQTTClient::Subscribe(int* pnMid, const std::string& rstrTopic)
{
    HCPLOG_METHOD();
    int nErr = MOSQ_ERR_NO_CONN;
    // subscription will always be 2,to avoid receiving duplicate configs
    int nQos = 2;
    if (m_bConnected)
    {
        nErr = mosquittopp::subscribe(pnMid, rstrTopic.c_str(), nQos);
    }
    return nErr;
}

int CMQTTClient::PublishMessage(int* pMid, const void* pvoidPayload, 
                                const int nSize, const std::string& rstrTopic, 
                                const int& rnQos)
{
    HCPLOG_METHOD();
    int err = MOSQ_ERR_NO_CONN;

    if (m_bConnected)
    {
        err = mosquittopp::publish(pMid, rstrTopic.c_str(), nSize,
                                   pvoidPayload, rnQos, false);
        HCPLOG_D << "Err=" << err << "; mid=" << *pMid << "; size=" << nSize << 
                 "; topic=" << rstrTopic;
    }
    return err;
}

int CMQTTClient::StopClient()
{
    HCPLOG_METHOD();
    m_bStopClientReq = true;
    return disconnect();
}

int CMQTTClient::StartClient()
{
    HCPLOG_METHOD();
    m_bStopClientReq = false;
    if(!m_bIsRunning)
    {
        HCPLOG_D << "Starting MQTTClient thread";
        Start();
    }
    else
    {
        HCPLOG_E << "MQTTClient thread is already running";
    }
    return 0;
}

std::string CMQTTClient::GetErrorString(int error)
{
    return std::string(mosqpp::strerror(error));
}

void CMQTTClient::on_connect(int rc)
{
    HCPLOG_C << "[" << rc << "]: " << mosqpp::connack_string(rc);

    if(0 == rc)
    {
        m_bConnected = true;
    }
    else
    {
        m_bConnected = false;
    }
}

void CMQTTClient::on_message(const struct mosquitto_message *pMessage)
{
    OnMessageReceived(pMessage->topic,
                      static_cast<const char *>(pMessage->payload));
}

void CMQTTClient::on_subscribe(int nMid, int nQosCount, 
                               const int* pnGrantedQos)
{
    HCPLOG_METHOD() << "mid:" << nMid << " qos_count :" << nQosCount << 
                       " granted_qos:" << *pnGrantedQos;
}

void CMQTTClient::on_disconnect(int rc)
{
    HCPLOG_C << "[" << rc << "]: " << mosqpp::strerror(rc);

    if (m_bConnected)
    {
        m_bConnected = false;
    }
}

void CMQTTClient::on_publish(int nMid)
{
    HCPLOG_C << "mid[" << nMid << "]";
}

void CMQTTClient::on_unsubscribe(int nMid)
{
    HCPLOG_METHOD() << "mid[" << nMid << "]";
}

void CMQTTClient::on_log(int nLevel, const char* pchStr)
{
    HCPLOG_C << "level:[" << nLevel << "]:" << pchStr;
}

void CMQTTClient::on_error()
{
    /* this function is not implemented in mosqpp lib, 
     * so far this is not been called
     */
}

void CMQTTClient::Run()
{
    HCPLOG_METHOD();
    /* This change is added so that stop client scenario can be handled 
     * seamlessly.
     */
    m_bStopClientReq = false;

    //calling InitRun() to execute any logics prior to start the thread loop
    InitRun();

    while(!m_bIsShutDownInitiated)
    {
        if(m_bStopClientReq && !m_bIsShutDownInitiated)
        {
            sleep(1);
        }
        else
        {
            Initialize();
            HCPLOG_D << "MQTT conn. loop exit~" << m_bStopClientReq << 
                        "~" << m_bIsShutDownInitiated;
        }
    }

    //calling ExitRun() to execute any logics prior to exit the thread loop
    ExitRun();

    HCPLOG_D << "exiting mqttclient run";
    Detach();
}

void CMQTTClient::Initialize()
{
    do
    {
        HCPLOG_D << "initializing";
        while (!m_bStopClientReq && !m_bIsShutDownInitiated &&
               (InitClient() != 0))
        {
            // backoff can be used in case of connectivity errors
            sleep(1);
        }

        // if (m_bIsShutDownInitiated)
        if (m_bStopClientReq || m_bIsShutDownInitiated)
        {
            break;
        }
        HCPLOG_C << "*** LOOPING FOREVER ***";
        int err = loop_forever(-1, 1);
        HCPLOG_C << "Loop is broken: err~" << mosqpp::strerror(err);
    } while (!m_bStopClientReq && !m_bIsShutDownInitiated);
}

void CMQTTClient::NotifyShutdown() {
    HCPLOG_METHOD();
    m_bIsShutDownInitiated = true;
    int rc = StopClient();
    if( 0 != rc)
    {
        HCPLOG_E << "Error in Stopping MQTTClient:"<< rc;
    }
    mosqpp::lib_cleanup();//Added to cleanup mqttlib
}

CMQTTClient::~CMQTTClient()
{
}

} //namespace
