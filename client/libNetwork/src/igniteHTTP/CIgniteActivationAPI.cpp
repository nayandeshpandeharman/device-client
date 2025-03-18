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

#include <string>
#include "CIgniteActivationAPI.h"
#include "core/CKeyGenerator.h"
#include "CIgniteConfig.h"
#include "crypto/CIgniteDataSecurity.h"
#include "crypto/CAes.h"
#include "jsoncpp/json.h"
#include "crypto/CRNG.h"
#include <algorithm>
#include <unistd.h>
#include "CHttpRequest.h"
#include "CHttpResponse.h"

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CIgniteActivationAPI"

using namespace std;

namespace ic_network
{

const std::string PRODUCT_TYPE_HEADUNIT = "Headunit";
const std::string PRODUCT_TYPE_HU = "hu";
const std::string KEY_USE_AES_GCM_ENCRYPTION = "useGCMEncryptForActivation";

CIgniteActivationAPI::CIgniteActivationAPI()
{

}

CIgniteActivationAPI::~CIgniteActivationAPI()
{

}

void CIgniteActivationAPI::ActivateDevice(
                          const CIgniteHTTPConnector::CActivationRequest &rReq,
                          CIgniteHTTPConnector::CActivationResponse &rResp,
                          const std::string strUrl)
{
    std::string strActivationJson="";
    CHttpRequest hRqst;
    CHttpResponse hResp;
    bool bIsValidresponse = false;

    strActivationJson = BuildActivationJson(rReq);

    hRqst.SetUrl(strUrl);
    hRqst.AddHeader("Accept: application/json");
    hRqst.AddHeader("Content-Type: application/json");
    hRqst.AddHeader("charsets: utf-8");
    hRqst.SetPostFields(strActivationJson);

    HttpErrorCode eRet = HttpErrorCode::eERR_UNKNOWN;

    bool bIsActivated = false;
    if (HttpErrorCode::eERR_OK == (eRet = hRqst.Execute(hResp)))
    {
        ic_utils::Json::Value jsonRoot;
        ic_utils::Json::Reader jsonReader;
        std::string strResponse = hResp.GetRespData();

        rResp.m_eHttpSessionErrCode = eRet;
        rResp.m_lHttpRespCode = hResp.GetHttpCode();
        rResp.m_strRespString = strResponse;

        if (!jsonReader.parse(strResponse, jsonRoot))
        {
            HCPLOG_E << "Parsing Failed!";
            rResp.m_eHttpSessionErrCode = HttpErrorCode::eERR_RESPONSE_FORMAT;
            rResp.m_strRespString = strResponse;
        }
        else
        {
            ic_utils::Json::Value jsonData;
            bool bUseDevType = rReq.m_bUseDeviceType;

            if (bUseDevType)
            {
                HCPLOG_D << "using v4 api";
                bIsValidresponse = ProcessV4APIResponse(jsonRoot, jsonData);
            }
            else 
            {
                HCPLOG_D << "using v2 api";
                /*  Response data of Activation api v4 is {
                 *  "passcode": "sample123passcode",
                 *  "deviceId": "Device123ID"
                 *  }
                 */
                jsonData = jsonRoot;
                bIsValidresponse = true;
            }

            if (bIsValidresponse) {
                UpdateDeviceIdAndPasscode(jsonData, rResp);
            }
            else
            {
                rResp.m_eHttpSessionErrCode = HttpErrorCode::eERR_RESPONSE_DATA;
            }
        }
    }
    else
    {
        long lHttpCode = hResp.GetHttpCode();
        rResp.m_eHttpSessionErrCode = eRet;
        rResp.m_lHttpRespCode = lHttpCode;
        rResp.m_strRespString = hResp.GetRespData();
    }

}

void CIgniteActivationAPI::UpdateDeviceIdAndPasscode(const ic_utils::Json::Value jsonData,
                      CIgniteHTTPConnector::CActivationResponse &rResp)
{
    std::string strDeviceId = jsonData.isMember("deviceID") ? 
                              "deviceID" : "deviceId";

    if ((jsonData.isMember(strDeviceId) &&
         !jsonData[strDeviceId].asString().empty()) &&
        (jsonData.isMember("passcode") &&
         !jsonData["passcode"].asString().empty()))
    {
        rResp.m_strDeviceID = jsonData[strDeviceId].asString();
        rResp.m_strPassCode = jsonData["passcode"].asString();
    }
}

bool CIgniteActivationAPI::ProcessV4APIResponse(const ic_utils::Json::Value jsonRoot,
                                         ic_utils::Json::Value &jsonData)
{
    bool bValidResponse = false;

    /* Response data of Activation api v4 is {
     * "code": "dauth-002",
     * "message": "Success",
     * "data": {
     *   "passcode": "sample123passcode",
     *   "deviceId": "Device123ID"
     *   }
     * }
     */
    if (jsonRoot.isMember("message") &&
        jsonRoot["message"].isString())
    {
        std::string strMessage = jsonRoot["message"].asString();
        if ("Success" == strMessage)
        {
            if (jsonRoot.isMember("data"))
            {
                jsonData = jsonRoot["data"];
                bValidResponse = true;
            }
            else
            {
                HCPLOG_W << "Activation failed: data missing";
            }
        }
        else
        {
            HCPLOG_W << "Activation failed:" << strMessage;
        }
    }
    else
    {
        HCPLOG_W << "Activation failed: Missing \'message\'";
    }

    return bValidResponse;
}

std::string CIgniteActivationAPI::BuildActivationJson(
                             const CIgniteHTTPConnector::CActivationRequest req)
{
    HCPLOG_METHOD();

    std::string strQual = req.m_strVin + "-delim-" + req.m_strSerialNumber + 
                        "-delim-" + 
                        ic_core::CRNG::GetString(req.m_strSerialNumber, 10000);

    HCPLOG_T << "Qualifier=" << strQual;

    if (req.m_strProductType.empty())
    {
        HCPLOG_E << "Activation: could not find configuration for productType";
    }

    std::string strActKey = ic_core::CKeyGenerator::GetActivationQualifierKey(
                                        req.m_strVin, req.m_strSerialNumber);
    HCPLOG_T << "Activation Key=" << strActKey;
    
    ic_utils::Json::Value jsonActreq;
    jsonActreq["productType"] = req.m_strProductType;
    jsonActreq["vin"] = req.m_strVin;
    jsonActreq["serialNumber"] = req.m_strSerialNumber;

    /* Check configuration for useGCMEncryptForActivation, 
     * by default using GCM encypt method
     */
    if (ic_core::CIgniteConfig::GetInstance()->GetBool(KEY_USE_AES_GCM_ENCRYPTION, true)) 
    {
        //use AES-GCM encryption method
        ic_core::CIgniteDataSecurity securityObject(strActKey, strActKey);
        jsonActreq["aad"] = "yes";
        // encryption with tag enabled & AAD
        jsonActreq["qualifier"] = securityObject.Encrypt(strQual, true,
                                                         req.m_strSerialNumber);
        HCPLOG_C << "use AES-GCM encrypt for Activation";
    }
    else {
        //use AES-CBC Encryption method
        ic_core::CAes aes(strActKey, strActKey);
        jsonActreq["qualifier"] = aes.Encrypt(strQual);
        HCPLOG_C << "use AES-CBC encrypt for Activation";
    }

    jsonActreq["HW-Version"] = req.m_strHwVersion;
    jsonActreq["SW-Version"] = req.m_strSwVersion;

    if (req.m_strProductType  != PRODUCT_TYPE_HEADUNIT && req.m_strProductType  != PRODUCT_TYPE_HU)
    {
        jsonActreq["imei"] = req.m_strImei;
    }
    jsonActreq["deviceType"] = req.m_strProductType;

    HCPLOG_D << "Payload~" << jsonActreq.toStyledString();

    std::string strOut = jsonActreq.toStyledString();
    strOut.erase(std::remove(strOut.begin(), strOut.end(), '\n'), strOut.end());
    strOut.erase(std::remove(strOut.begin(), strOut.end(), '\r'), strOut.end());
    return strOut;
}

} // namespace
