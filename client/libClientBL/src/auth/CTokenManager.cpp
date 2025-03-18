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

#include <algorithm>
#include <unistd.h>
#include <time.h>
#include "CIgniteLog.h"
#include "CIgniteConfig.h"
#include "CIgniteClient.h"
#include "CIgniteDateTime.h"
#include "CIgniteStringUtils.h"
#include "CIgniteEvent.h"
#include "auth/CTokenManager.h"
#include "core/CKeyGenerator.h"
#include "crypto/CIgniteDataSecurity.h"
#include "db/CLocalConfig.h"
#include "CHttpResponse.h"
#include "CActivationBackoff.h"
#include <analytics/CDisassociationRequestHandler.h>
#include "CActivationState.h"
#include "core/CPersistancyAndStateHandler.h"

//! Macro for CDisassociationRequestHandler string
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CTokenManager"

using ic_core::CKeyGenerator;

namespace ic_bl 
{
namespace 
{
//! Constant key for 'login' string
const std::string LOGIN = "login";

//! Constant key for 'passcode' string
const std::string PASSCODE = "passcode";

//! Constant key for 'activationTS' string
const std::string ACTIVATION_TS = "activationTS";
}

//static member declaration
ic_utils::CIgniteMutex CTokenManager::m_tknMngrMutex;

CTokenManager* CTokenManager::GetInstance(void)
{
    static CTokenManager tokenManagerSingleton;
    return &tokenManagerSingleton;
}

void CTokenManager::ReleaseInstance(void)
{
    //do required cleanup here
}

CTokenManager::CTokenManager()
    : m_strActivateUrl(""), m_strAuthUrl(""),m_strToken(""),
      m_llTokenIssuedOn(0), m_llActivationTime(0), m_llTokenTTL(0),
      m_nMarginPercent(0)
{
    m_lTokenIssueTime = 0;
    ic_core::CIgniteConfig *pConfig = ic_core::CIgniteConfig::GetInstance();

    m_strActivateUrl = pConfig->GetString("HCPAuth.activate_url");
    if (m_strActivateUrl.empty())
    {
        HCPLOG_F << "could not find configuration for activate_url!!!";
    }
    else
    {
        ic_network::CIgniteHTTPConnector::GetInstance()->
                                             SetActivationURL(m_strActivateUrl);
    }

    m_strAuthUrl = pConfig->GetString("HCPAuth.auth_url");
    if (m_strAuthUrl.empty())
    {
        HCPLOG_F << "could not find configuration for auth url!!!";
    }
    else
    {
        ic_network::CIgniteHTTPConnector::GetInstance()->
                                                       SetAuthURL(m_strAuthUrl);
    }

    m_nMarginPercent = pConfig->GetInt("HCPAuth.token_marginPercent");
    HCPLOG_T << "ttl safety margin Percentage: " << m_nMarginPercent;

    if(IsActivated() == 0)
    {
        if (Activate() == HttpErrorCode::eERR_RESPONSE_DATA)
        {
            HCPLOG_E << "Activation Failed:" 
                     << HttpErrorCode::eERR_RESPONSE_DATA;
            CActivationState::GetInstance()->SetActivationState(false,
                                CActivationState::eDEVICE_NOT_ASSOCIATED_ERROR);
        }
    }
}

CTokenManager::~CTokenManager()
{

}

const std::string CTokenManager::GetToken(HttpErrorCode &reSessionErr)
{
    HCPLOG_METHOD();
    reSessionErr = HttpErrorCode::eERR_OK;

    /* Since many modules are racing for authenticated web requests, lock the
     * activation and login in order to avoid multiple activations/logins.
     */
    m_tknMngrMutex.Lock();

    if (m_strToken.empty())
    {
        if (!CheckForActivation(reSessionErr))
        {
            UpdateActivationState(reSessionErr);
            HCPLOG_E << "Activation Failed:" 
                     << CHttpResponse::GetHttpErrorCodeString(reSessionErr);
            m_tknMngrMutex.Unlock();
            CActivationState::GetInstance()->SetToken("");
            return "";
        } 
    }
    else
    {
        unsigned long long ullCurrMonoTimeMs =
                                ic_utils::CIgniteDateTime::GetMonotonicTimeMs();

        HCPLOG_C << "Current Token valid till: " 
                 << m_llTokenIssuedOn + m_llTokenTTL 
                 << ", Current: " << ullCurrMonoTimeMs;

        if (ullCurrMonoTimeMs < (m_llTokenIssuedOn + m_llTokenTTL))
        {
            // token still valid
            m_tknMngrMutex.Unlock();
            CActivationState::GetInstance()->SetToken(m_strToken);
            return m_strToken;
        }

        HCPLOG_C << "Token expired! Get new token...";
    }

    // login and get new token
    LoginError loginErr = GetAuthToken(reSessionErr);

    if (loginErr == eLE_INVALID_CREDENTIAL || loginErr == eLE_INVALID_SCOPE) 
    {
        /* re-activate device in case of invalid credential OR invalid_scope
         * Forcefully clear token value, pass code and device id
         */
        InvalidateToken();
        InvalidatePassCode();

        if ((reSessionErr = Activate()) == HttpErrorCode::eERR_OK) 
        {
            if ((loginErr = GetAuthToken(reSessionErr)) != eLE_SUCCESS)
            {
                HCPLOG_E << "Auth token failure:" << loginErr;
            }
        }
        else 
        {
            HCPLOG_E << "Re-activation failure:" 
                     << CHttpResponse::GetHttpErrorCodeString(reSessionErr);
        }
    }
    m_tknMngrMutex.Unlock();

    return m_strToken;
}

bool CTokenManager::CheckForActivation(HttpErrorCode &reSessionErr)
{
    bool bStatus = true;
    if (!IsActivated() && (reSessionErr = Activate()) != HttpErrorCode::eERR_OK)
    {
        bStatus = false;
    }
    else
    {
        //already activated
    }

    return bStatus;
}

void CTokenManager::UpdateActivationState(HttpErrorCode &reSessionErr)
{
    if (HttpErrorCode::eERR_NETWORK == reSessionErr)
    {
        CActivationState::GetInstance()->SetActivationState(false, 
                                              CActivationState::eNETWORK_ERROR);
    }
}

LoginError CTokenManager::GetAuthToken(HttpErrorCode &reSessionErr)
{
    LoginError eLoginErr = eLE_UNKNOWN;

    ic_network::CIgniteHTTPConnector::CAuthRequest req;
    ic_network::CIgniteHTTPConnector::CAuthResponse resp;
    ic_core::CLocalConfig *pConfig = ic_core::CLocalConfig::GetInstance();

    req.m_strLoginStr = pConfig->Get("login");
    req.m_strPasscodeStr = pConfig->Get("passcode");
    req.m_bUseDevType = ic_core::CIgniteConfig::GetInstance()->
                                                 GetBool("useDeviceType",false);
    req.m_strProductType = ic_core::CIgniteConfig::GetInstance()->
                                                       GetString("ProductType");
    ic_network::CIgniteHTTPConnector::GetInstance()->GetAuthToken(req,resp);

    reSessionErr = resp.m_eHttpSessionErrCode;
    if (reSessionErr == HttpErrorCode::eERR_OK )
    {
        /* Session request is successful but login attempt might have been
         * rejected need to check the curl response code. Only for 401 we need
         * to try the re-activation
         */ 
        switch (resp.m_lHttpRespCode)
        {
            case 200:
            {
                m_strToken = resp.m_strToken;
                CActivationState::GetInstance()->SetToken(m_strToken);
                if (!resp.m_strTokenTime.empty())
                {
                    SetTokenIssueTime(resp.m_strTokenTime);

                    pConfig->Set("expirationTime", resp.m_strExpTime);
                    pConfig->Set("issueTime", resp.m_strTokenTime);
                }
                else
                {
                    HCPLOG_E << "Recvd empty tokenTime";
                }

                m_llTokenIssuedOn = resp.m_ullTokenIssuedOn;

                if (pConfig->Get(ACTIVATION_TS).empty())
                {
                    unsigned long long loginTime = m_lTokenIssueTime;
                    string actTime = ic_utils::CIgniteStringUtils::
                           NumberToString<unsigned long long>((loginTime*1000)-
                           (m_llTokenIssuedOn -m_llActivationTime));
                    HCPLOG_C << "ActivationTime : [" << actTime << "]";
                    pConfig->Set(ACTIVATION_TS, actTime);
                }

                m_llTokenTTL = resp.m_ullTokenTTL;

                if (m_nMarginPercent > 0 && m_nMarginPercent < 100)
                {
                    m_llTokenTTL -= (m_llTokenTTL * m_nMarginPercent) / 100;
                }
                    
                HCPLOG_C << "authtime:[" << m_lTokenIssueTime
                         << "][" << m_llTokenIssuedOn 
                         << "][" << m_llTokenTTL << "]";

                eLoginErr = eLE_SUCCESS;
                CActivationState::GetInstance()->SetActivationState(true,
                                                   CActivationState::eNO_ERROR);   
                break;
            }

            case 401:
                eLoginErr = eLE_INVALID_CREDENTIAL;
                break;
            case 400:
                // In case of invalid_scope fall back to reactivation
                eLoginErr = eLE_INVALID_SCOPE;
                break;
            default:
                HCPLOG_E << "login error:" << resp.m_lHttpRespCode;
                break;
        }
    }
    else
    {
        CActivationState::GetInstance()->SetActivationState(false,
                                              CActivationState::eNETWORK_ERROR);
        HCPLOG_E << "Login failed:" 
                 << CHttpResponse::GetHttpErrorCodeString(reSessionErr);
    }

    return eLoginErr;
}

void CTokenManager::InvalidateToken()
{
    m_strToken.clear();
}

void CTokenManager::InvalidatePassCode()
{
    ic_core::CLocalConfig::GetInstance()->Remove(LOGIN);
    ic_core::CLocalConfig::GetInstance()->Remove(PASSCODE);
}

bool CTokenManager::IsActivated()
{
    ic_core::CLocalConfig *pConfig = 
                  (ic_core::CLocalConfig*) ic_core::CLocalConfig::GetInstance();

    if (pConfig->Get("login").empty() || pConfig->Get("passcode").empty())
    {
        return false;
    }
    else
    {
        return true;
    }
}

void CTokenManager::SetTokenIssueTime(std::string strTime)
{
    tm issueTime{};

    if (NULL == strptime(strTime.c_str(), "%Y-%m-%dT%T", &issueTime))
    {
        HCPLOG_E << "Unable to parse token issue time";
        return;
    }

    SetTokenIssueTime(mktime(&issueTime));
}

void CTokenManager::SetTokenIssueTime(time_t lTime)
{
    m_lTokenIssueTime = lTime;
}

time_t CTokenManager::GetTokenIssueTime()
{
    return m_lTokenIssueTime;
}

HttpErrorCode CTokenManager::ProcessActivationResponse(
                            const ic_network::CIgniteHTTPConnector::
                                    CActivationResponse &rActivationResponse,
                            const std::string &rstrQualifierID)
{
    HttpErrorCode eHttpErrorCode = HttpErrorCode::eERR_OK;

    if (200 == rActivationResponse.m_lHttpRespCode)
    {
        ic_core::CLocalConfig *pLocalConfig =
                  (ic_core::CLocalConfig *)ic_core::CLocalConfig::GetInstance();

        // store monotonic time to get server activation time on login
        m_llActivationTime = ic_utils::CIgniteDateTime::GetMonotonicTimeMs();
        HCPLOG_C << "activationMono:[" << m_llActivationTime << "]";
        pLocalConfig->Remove(ACTIVATION_TS);

        /* If deviceId changes in between connection then 
         * notify other handlers by sending event, so that other handlers can 
         * register themselves and if any handling is required on DeviceId 
         * change then can do that.
         */
        std::string strOldDeviceId = pLocalConfig->Get("login");
        std::string strNewDeviceId = rActivationResponse.m_strDeviceID;
        if (strOldDeviceId != strNewDeviceId)
        {
            ic_event::CIgniteEvent igniteDeviceIdEvent("1.0", "DeviceId");
            igniteDeviceIdEvent.AddField("value", strNewDeviceId);
            igniteDeviceIdEvent.Send();
        }
        ic_utils::CIgniteLog::SetStatus("DeviceId", strNewDeviceId);
        pLocalConfig->Set("login", rActivationResponse.m_strDeviceID);

        ic_core::IProduct *pProduct = ic_core::CIgniteClient::GetProductImpl();
        std::string strLastDeviceId = pLocalConfig->Get("lastDeviceId");
        if (strLastDeviceId != strNewDeviceId)
        {
            pLocalConfig->Set("lastDeviceId", strNewDeviceId);

            std::string strIMEI = pProduct->GetAttribute(
                                                      ic_core::IProduct::eIMEI);
            std::string strSerial = pProduct->GetAttribute(
                                            ic_core::IProduct::eSerialNumber);

            /* This event is added in DirectAlerts configuration list to be
             * sent as Alert.
             */ 
            ic_event::CIgniteEvent igniteEventActivationAlert("1.0", 
                                                              "Activation");
            igniteEventActivationAlert.AddField("id", strNewDeviceId);
            igniteEventActivationAlert.AddField("imei", strIMEI);
            igniteEventActivationAlert.AddField("serialNumber", strSerial);
            igniteEventActivationAlert.Send();

            /* below event will be sent to event handlers because
             * alert are not passed to event handlers.
             */ 
            ic_event::CIgniteEvent igniteEventActivationEvent("1.0",
                                                            "ActivationEvent");
            igniteEventActivationEvent.AddField("id", strNewDeviceId);
            igniteEventActivationEvent.Send();

            if (!strLastDeviceId.empty())
            {
                HCPLOG_C << "Device re-associated";
                HCPLOG_D << "clean db and restart analytics";
                CDisassociationRequestHandler::GetInstance()->Handle();
            }
        }

        // Based on new device id the passcode key will generated once again
        string strPassKey = ic_core::CKeyGenerator::GetPasscodeKey(
                                                rActivationResponse.m_strDeviceID);
        ic_core::CIgniteDataSecurity securityObject(strPassKey, strPassKey);
        string strEncString = securityObject.Encrypt(
                                             rActivationResponse.m_strPassCode);
        pLocalConfig->Set("passcode", strEncString);

        // persist activation details
        ic_utils::Json::Value jsonPayload;
        jsonPayload["status"] = ic_core::IC_ACTIVATION_STATUS::eACTIVATED;
        jsonPayload["vin"] = rstrQualifierID;
        jsonPayload["deviceId"] = strNewDeviceId;
        ic_utils::Json::FastWriter jsonWriter;
        std::string strActDetails = jsonWriter.write(jsonPayload);
        CPersistancyAndStateHandler::GetInstance()
                                    ->PersistActivationStatus(strActDetails);
        ic_core::CIgniteClient::GetClientMessageDispatcher()
                    ->DeliverActivationDetails(
                    CPersistancyAndStateHandler::GetInstance()
                    ->GetICParam(ic_core::IC_QUERY_ID::eACTIVATION_STATUS));
        CActivationState::GetInstance()->SetActivationState(true,
                                                   CActivationState::eNO_ERROR);
        HCPLOG_C << "Activation Successful";
    }
    else
    {
        HCPLOG_E << "Activation failed " << rActivationResponse.m_strRespString;
        eHttpErrorCode = HttpErrorCode::eERR_RESPONSE_DATA;
        CActivationState::GetInstance()->SetActivationState(false, 
                                CActivationState::eDEVICE_NOT_ASSOCIATED_ERROR);
    }

    return eHttpErrorCode;
}

HttpErrorCode CTokenManager::Activate()
{
    HCPLOG_METHOD();
    // Check for activation backoff.
    if (!CActivationBackoff::GetInstance()->Proceed())
    {
        CActivationState::GetInstance()->SetActivationState(false,
                                              CActivationState::eBACKOFF_ERROR);
        return HttpErrorCode::eERR_BACKOFF;
    }

    ic_network::CIgniteHTTPConnector::CActivationRequest actReq;
    ic_network::CIgniteHTTPConnector::CActivationResponse actResp;
    bool bIsActivated = false;

    HttpErrorCode eRet = HttpErrorCode::eERR_UNKNOWN;
    std::string strQualifierID = "";

    // Default returns VIN as per the activation API requirement
    strQualifierID = ic_core::CIgniteClient::GetProductImpl()->
                                                     GetActivationQualifierID();

    if (strQualifierID == "NOT_AVAILABLE" || strQualifierID.empty())
    {
        return HttpErrorCode::eERR_BACKOFF;
    }
    else
    {
        actReq.m_strQualifierID = strQualifierID;
        actReq.m_strVin = strQualifierID; // Default Qualifier is VIN
        actReq.m_strSerialNumber = ic_core::CIgniteClient::GetProductImpl()->
                                 GetAttribute(ic_core::IProduct::eSerialNumber);
        actReq.m_strImei = ic_core::CIgniteClient::GetProductImpl()->
                                         GetAttribute(ic_core::IProduct::eIMEI);
        actReq.m_strHwVersion = ic_core::CIgniteClient::GetProductImpl()->
                                    GetAttribute(ic_core::IProduct::eHWVersion);
        actReq.m_strSwVersion = ic_core::CIgniteClient::GetProductImpl()->
                                    GetAttribute(ic_core::IProduct::eSWVersion);
        actReq.m_strProductType = ic_core::CIgniteConfig::GetInstance()->
                                                       GetString("ProductType");
        actReq.m_bUseDeviceType = ic_core::CIgniteConfig::GetInstance()->
                                                       GetBool("useDeviceType");
        
        ic_network::CIgniteHTTPConnector::GetInstance()->
                                                      Activate(actReq, actResp);
        eRet = actResp.m_eHttpSessionErrCode;
        HCPLOG_I << "http_code " << actResp.m_lHttpRespCode
                 << " http_error_code: " << actResp.m_eHttpSessionErrCode 
                 << " HCPID:" << actResp.m_strDeviceID
                 << " passcode:" << actResp.m_strPassCode
                 << " response:" << actResp.m_strRespString;
    }

    // Checking the status of API invocation
    if (HttpErrorCode::eERR_OK == eRet)
    {
        HttpErrorCode eHttpErrorCode = 
                             ProcessActivationResponse(actResp, strQualifierID);

        // Checking the actual http response
        if(HttpErrorCode::eERR_OK == eHttpErrorCode)
        {
            bIsActivated = true;
        }
    }
    else
    {
        HCPLOG_E << "Activation failed";
        eRet = HttpErrorCode::eERR_RESPONSE_DATA;
        CActivationState::GetInstance()->SetActivationState(false,
                                CActivationState::eDEVICE_NOT_ASSOCIATED_ERROR);
    }

    if (bIsActivated)
    {
        CActivationBackoff::GetInstance()->Reset();
    }
    else
    {
        CActivationBackoff::GetInstance()->CalculateNextRetry(eRet);
    }

    return eRet;
}
} /* namespace ic_bl */
