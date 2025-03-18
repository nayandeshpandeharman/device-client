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

#include "CIgniteHTTPConnector.h"
#include "CIgniteActivationAPI.h"
#include "CIgniteAuthTokenAPI.h"
#include "CIgniteConnHealthCheckAPI.h"

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CIgniteHTTPConnector"

namespace ic_network
{

CIgniteHTTPConnector::CIgniteHTTPConnector()
{

}

CIgniteHTTPConnector::~CIgniteHTTPConnector()
{

}

CIgniteHTTPConnector* CIgniteHTTPConnector::GetInstance()
{
    static CIgniteHTTPConnector mInstance;
    return &mInstance;
}

bool CIgniteHTTPConnector::SetActivationURL(std::string strUrl)
{
    m_strActivationUrl = strUrl;
    return true;
}

bool CIgniteHTTPConnector::SetAuthURL(std::string strUrl)
{
    m_strAuthUrl = strUrl;
    return true;
}

bool CIgniteHTTPConnector::SetHealthCheckURL(std::string strUrl)
{
    m_strHealthCheckUrl = strUrl;
    return true;
}

void CIgniteHTTPConnector::Activate(const CActivationRequest &rReq,
                                    CActivationResponse &rResp,
                                    std::string strUrl)
{
    CIgniteActivationAPI actAPI;

    if (strUrl.empty())
    {
        if (!m_strActivationUrl.empty())
        {
            actAPI.ActivateDevice(rReq, rResp, m_strActivationUrl);
        }
        else
        {
            HCPLOG_E << "Activation URL not set";
            rResp.m_eHttpSessionErrCode = 
                                    ic_network::HttpErrorCode::eERR_INV_INPUT;
            rResp.m_strRespString = "Activation URL not set";
        }
        
    } 
    else
    {
        actAPI.ActivateDevice(rReq, rResp, strUrl);
    }
    
}

void CIgniteHTTPConnector::GetAuthToken(const CAuthRequest &rReq,
                                         CAuthResponse &rResp,
                                         std::string strUrl)
{
    CIgniteAuthTokenAPI authAPI;
    if (strUrl.empty())
    {
        if(!m_strAuthUrl.empty())
        {
            authAPI.FetchAuthToken(rReq, rResp, m_strAuthUrl);
        }
        else
        {
            HCPLOG_E << "Auth URL not set";
            rResp.m_eHttpSessionErrCode = 
                                    ic_network::HttpErrorCode::eERR_INV_INPUT;
            rResp.m_strHttpRespData = "Auth URL not set";
        }
        
    } 
    else
    {
        authAPI.FetchAuthToken(rReq, rResp, strUrl);
    }
}


void CIgniteHTTPConnector::GetConnectionHealthCheckStatus(
    CIgniteHTTPConnector::CIgniteConnHealthCheckResponse &rResp,
    std::string strUrl)
{
    HCPLOG_METHOD();
    CIgniteConnHealthCheckAPI healthCheckApi;
    std::string strHealthCheckApiUrl =
        (strUrl.empty() ? (m_strHealthCheckUrl.empty() ? "" : 
                                                m_strHealthCheckUrl) : strUrl);
    if (!strHealthCheckApiUrl.empty())
    {
        healthCheckApi.CheckConnectionHealthStatus(rResp, strHealthCheckApiUrl);
    }
    else {
        HCPLOG_E << "health check URL not set";
        rResp.m_eHttpSessionErrCode = ic_network::HttpErrorCode::eERR_INV_INPUT;
    }
}


} //namespace
