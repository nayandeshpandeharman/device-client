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

#include "CIgniteAuthTokenAPI.h"
#include "core/CKeyGenerator.h"
#include "CHttpRequest.h"
#include "CHttpResponse.h"
#include "crypto/CIgniteDataSecurity.h"
#include "crypto/CBase64.h"
#include "CIgniteDateTime.h"
#include "CIgniteStringUtils.h"
#include "CIgniteFileUtils.h"
#include <unistd.h>
#include <time.h>
#include "jsoncpp/json.h"

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CIgniteAuthTokenAPI"

namespace ic_network
{

enum LoginError
{
    eLE_SUCCESS,
    eLE_INVALID_CREDENTIAL,
    eLE_INVALID_SCOPE,
    eLE_UNKNOWN
};

/**
 * Method to Convert from the format Thu, 15 Mar 2018 06:48:13 GMT
 * to ":"Y-M-DTHH:MM:SS". Here T is the separator.
 * @param[in] rstrTime Date(string) in the format Thu, 15 Mar 2018 06:48:13 GMT
 * @return date in the converted format
 */
static std::string get_time_format_as_login(std::string strTime) 
{
    tm issueTime;
    std::string strNewFormat = "";
    if (NULL == strptime(strTime.c_str(), "%a, %d %b %Y %H:%M:%S", &issueTime)) 
    {
        HCPLOG_E << "Unable to parse header token time";
        return strNewFormat;
    }

    char cstrDate2[30];
    strftime(cstrDate2, sizeof(cstrDate2), "%Y-%m-%dT%T", &issueTime);
    strNewFormat = cstrDate2;
    return strNewFormat;
}

static std::string get_time_from_payload(std::string strPayloadData, 
                                      std::string strT_time)
{
    // First decode the payload data to json format
    std::string strIat;
    std::string jsonPayload = ic_utils::CIgniteFileUtils::Base64Decode(strPayloadData);
    if(jsonPayload.empty()) {
        HCPLOG_E << "Decoding access token FAILED";
        return strIat;
    }

    ic_utils::Json::Value jsonRoot;
    ic_utils::Json::Reader jsonReader;
    if (!jsonReader.parse(jsonPayload, jsonRoot))
    {
        HCPLOG_E << "Access token parsing FAILED";
        return strIat;
    }

    tm myTm;
#ifdef __ANDROID__
    int t = jsonRoot[strT_time].asInt();
    time_t tx = t;
    myTm = *localtime(&tx);
#else
    std::string strEpocTime =
        ic_utils::CIgniteStringUtils::NumberToString(jsonRoot[strT_time].asInt64());
    if (NULL == strptime(strEpocTime.c_str(), "%s", &myTm))
    {
        HCPLOG_E << "Unable to parse iat time. iat =  " << strEpocTime;
        return strIat;
    }
#endif
    char cstrDate2[30];
    strftime(cstrDate2, sizeof(cstrDate2), "%Y-%m-%dT%T", &myTm);
    strIat = cstrDate2;
    return strIat;
}


/**
 * Method to get the date from the http response header.
 * @param[in] rstrHeaderResp http response header
 * @return date in the string format (ex: Thu, 15 Mar 2018 06:48:13 GMT)
 */
std::string get_date_from_header(std::string& rstrHeaderResp)
{
    std::string strVal = "";
    std::string strEtagStr = "Date: ";
    size_t separator = rstrHeaderResp.find(strEtagStr);
    if(separator == std::string::npos)
        separator = rstrHeaderResp.find("date: ");
    if(separator != std::string::npos) {
        std::string strNewstr =
            rstrHeaderResp.substr(separator + strEtagStr.length(), -1);
        size_t separator1 = strNewstr.find_first_of("\n");
        strVal = strNewstr.substr(0, separator1 - 4); // Stripping of GMT
    }
    return strVal;
}

std::string get_payload_data_from_token(std::string strToken)
{
    std::string strPayloadData;
    size_t separator1 = strToken.find_first_of(".");
    if(separator1 == std::string::npos) {
        std::cout << ". not found in token" << std::endl;
        return strPayloadData;
    }

    std::string payloadToken = strToken.substr(separator1 + 1, -1);
    size_t separator2 = payloadToken.find_first_of(".");
    if(separator2 == std::string::npos) {
        std::cout << ". not found in payload token" << std::endl;
        return strPayloadData;
    }

    strPayloadData = payloadToken.substr(0, separator2);
    return strPayloadData;
}

/* This method is future candidate for removal,
 * as servers sends issuedOn and ttl with v2 api
 */
unsigned long long calc_ttl(const std::string& rstrIssueTime, 
                           const std::string& rstrExpiryTime)
{
    HCPLOG_METHOD();
    unsigned long long lTTLVal = 0;

    std::string strIssueT(rstrIssueTime);
    std::replace(strIssueT.begin(), strIssueT.end(), 'T', ' ');
    std::replace(strIssueT.begin(), strIssueT.end(), '-', '/');

    std::string strExpT(rstrExpiryTime);
    std::replace(strExpT.begin(), strExpT.end(), 'T', ' ');
    std::replace(strExpT.begin(), strExpT.end(), '-', '/');

    HCPLOG_C << "issueTime=" << strIssueT << "~expiryTime=" << strExpT;

    lTTLVal = ic_utils::CIgniteDateTime::ConvertToNumber(strExpT) -
              ic_utils::CIgniteDateTime::ConvertToNumber(strIssueT);

    return lTTLVal;
}

CIgniteAuthTokenAPI::CIgniteAuthTokenAPI()
{

}

CIgniteAuthTokenAPI::~CIgniteAuthTokenAPI()
{
    
}

void CIgniteAuthTokenAPI::FetchAuthToken(
    const CIgniteHTTPConnector::CAuthRequest &rReq,
    CIgniteHTTPConnector::CAuthResponse &rResp,std::string strUrl)
{
    std::string ekey = BuildAuthCode(rReq.m_strLoginStr,
                                         rReq.m_strPasscodeStr);
    HCPLOG_I << "ENCODED KEY = " << ekey;

    std::string postfield = "";
    if (rReq.m_bUseDevType)
    {
        if (!rReq.m_strProductType.empty())
        {
            postfield =
                "grant_type=client_credentials&scope=" + rReq.m_strProductType;
        }
        else 
        {
            HCPLOG_E << "Could not find configuration for productType!!!";
        }
    }
    else
    {
        postfield = "grant_type=client_credentials&scope=GenDevice";
    }

    LoginError le;
    CHttpRequest hRqst;
    CHttpResponse hResp;

    hRqst.SetUrl(strUrl);
    hRqst.AddHeader("Authorization: Basic " + ekey);
    hRqst.AddHeader("Cache-Control: no-cache");
    hRqst.AddHeader("Content-Type: application/x-www-form-urlencoded");
    hRqst.SetPostFields(postfield);

    rResp.m_eHttpSessionErrCode = hRqst.Execute(hResp);

    if (rResp.m_eHttpSessionErrCode == HttpErrorCode::eERR_OK)
    {
        /* Session request is successful but login attempt might have
         * been rejected. need to check the curl response code. 
         * Only for 401 we need to try the re-activation
         */
        rResp.m_lHttpRespCode = hResp.GetHttpCode();
        rResp.m_strHttpRespHeader = hResp.GetHttpResponseHeader();
        rResp.m_strHttpRespData = hResp.GetRespData();

        switch (rResp.m_lHttpRespCode)
        {
            case 200:
            {
                ParseAPIResponse(rResp);
                break;
            }
            case 401:
                rResp.m_strHttpRespData = "Invalid Credential";
                break;
            case 400:
                rResp.m_strHttpRespData = "Invalid Scope";
                break;
            default:
                HCPLOG_E << "login error:" << rResp.m_lHttpRespCode;
                break;
        }
    }
    else
    {
        rResp.m_strHttpRespData = "Network Error";
    }

    HCPLOG_D << "mHttpSessionErrCode:" << rResp.m_eHttpSessionErrCode << 
             " mHttpRespCode:" << rResp.m_lHttpRespCode << " mTokenTime:" << 
             rResp.m_strTokenTime << " mExpTime:" << rResp.m_strExpTime << 
             " mTokenIssuedOn:" << rResp.m_ullTokenIssuedOn << " mTokenTTL:" << 
             rResp.m_ullTokenTTL;
}

void CIgniteAuthTokenAPI::ParseAPIResponse(
                          CIgniteHTTPConnector::CAuthResponse& rResp)
{
    ic_utils::Json::Value jsonRoot;
    ic_utils::Json::Reader jsonReader;
    std::string strResponse = rResp.m_strHttpRespData;
    if (!jsonReader.parse(strResponse, jsonRoot))
    {
        HCPLOG_E << "Parsing Failed!";
        rResp.m_eHttpSessionErrCode = HttpErrorCode::eERR_RESPONSE_DATA;
        rResp.m_strHttpRespData = "Parsing Failed";
    }
    else if ((jsonRoot.isMember("access_token") && 
             !jsonRoot["access_token"].asString().empty()))
    {
        // store token in member variable
        rResp.m_strToken = jsonRoot["access_token"].asString();
        HCPLOG_C << "Token acquired: "<<rResp.m_strToken.substr(0, 5) << ".."<< 
        rResp.m_strToken.substr(rResp.m_strToken.size() - 5, 
                                rResp.m_strToken.size() - 1);

        std::string strEncodedPayLoad = 
                                get_payload_data_from_token(rResp.m_strToken);
        UpdateTokenTime(strEncodedPayLoad,rResp);

        // get monotonic time for reference, store in member variable
        rResp.m_ullTokenIssuedOn = ic_utils::CIgniteDateTime::GetMonotonicTimeMs();
    }
    else
    {
        HCPLOG_E << "token parameters Invalid!";
        rResp.m_eHttpSessionErrCode = HttpErrorCode::eERR_RESPONSE_FORMAT;
        rResp.m_strHttpRespData = "Token Parameters Invalid";
    }
}

void CIgniteAuthTokenAPI::UpdateTokenTime(const std::string &rstrEncodedPayLoad,
                                    CIgniteHTTPConnector::CAuthResponse &rResp)
{
    if (!rstrEncodedPayLoad.empty())
    {
        rResp.m_strTokenTime = get_time_from_payload(rstrEncodedPayLoad, "iat");
    }

    // Get the Token time from header if didn't get from token
    if (rResp.m_strTokenTime.empty())
    {
        std::string strDate = get_date_from_header(rResp.m_strHttpRespHeader);
        if (!strDate.empty())
        {
            rResp.m_strTokenTime = get_time_format_as_login(strDate);
        }
    }

    if (!rResp.m_strTokenTime.empty())
    {
        rResp.m_strExpTime = get_time_from_payload(rstrEncodedPayLoad, "exp");
        rResp.m_ullTokenTTL = calc_ttl(rResp.m_strTokenTime, rResp.m_strExpTime);
    }
    else
    {
        HCPLOG_E << "Recvd empty tokenTime";
    }
}

std::string CIgniteAuthTokenAPI::BuildAuthCode(
            const std::string strLoginStr, const std::string strPasscodeStr)
{
    string strPassKey = ic_core::CKeyGenerator::GetPasscodeKey(strLoginStr);
    ic_core::CIgniteDataSecurity securityObject(strPassKey, strPassKey);
    string strPasscode = securityObject.Decrypt(strPasscodeStr);

    std::string strAuthBasedKey = strLoginStr + ":" + strPasscode;
    std::string strEncodedKey = 
       ic_core::CBase64::Encode((char*)strAuthBasedKey.c_str(), strAuthBasedKey.length());
    return strEncodedKey;
}
#ifdef IC_UNIT_TEST
std::string CIgniteAuthTokenAPI::GetDateFromHeader(std::string &rstrHeaderResp)
{
    return get_date_from_header(rstrHeaderResp);
}

std::string CIgniteAuthTokenAPI::GetTimeFormatAsLogin(std::string &rstrTime)
{
    return get_time_format_as_login(rstrTime);
}
#endif

} //namespace
