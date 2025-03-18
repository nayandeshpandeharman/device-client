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

/*!
********************************************************************************
* \file CIgniteHTTPConnector.h
*
* \brief This class provides the necessary interfaces to be used  by external  *
*        applications to interact with Ignite Cloud                            *
********************************************************************************
*/

#ifndef CIGNITE_HTTP_CONNECTOR
#define CIGNITE_HTTP_CONNECTOR

#include <string>
#include <jsoncpp/json.h>
#include "HttpErrorCodes.h"
#include "CIgniteLog.h"
#include <list>

namespace ic_network
{

class CIgniteHTTPConnectorTest;

/**
 * CIgniteHTTPConnector class exposing APIs necessary for interaction with
 * Ignite Cloud.
 */
class CIgniteHTTPConnector final 
{

#ifdef IC_UNIT_TEST
   friend class CIgniteHTTPConnectorTest;
#endif   

public:
    /**
     * Method to get an Instance of Ignite HTTP Connector.
     * @param void
     * @return Pointer to an object of CIgniteHTTPConnector class
     */
    static CIgniteHTTPConnector *GetInstance();

    /**
     * CActivationRequest class to contain the parameters of activation request.
     */
    class CActivationRequest
    {
    public:

        //! member variable to hold serial number of the device
        std::string m_strSerialNumber = "";

        //! member variable to hold IMEI number of the device
        std::string m_strImei = "";

        //! member variable to hold Qualifier ID
        std::string m_strQualifierID = "";

        //! member variable to hold hardware version
        std::string m_strHwVersion = "";

        //! member variable to hold software version
        std::string m_strSwVersion = "";

        //! member variable to hold VIN number of the device
        std::string m_strVin = "";

        //! member variable to hold device type ("dongle","dashcam" etc)
        std::string m_strDeviceType = "";

        //! member variable to hold SSID of the device
        std::string m_strSsid = "";

        //! member variable to hold ICCID of the device
        std::string m_strIccid = "";

        //! member variable to hold BSSID of the device
        std::string m_strBssid = "";

        //! member variable to hold MSISDN of the device
        std::string m_strMsisdn = "";

        //! member variable to hold IMSI of the device
        std::string m_strImsi = "";

        //! member variable to hold Product Type
        std::string m_strProductType = "";

        /**
         * member variable indicating whether device type needs to used ,
         * for activation.
         */
        bool m_bUseDeviceType = false;

   };

   /**
    * CActivationResponse class to contain the parameters of activation response.
    */
   class CActivationResponse
   {
   public:

       //! member variable to hold http session return value of activation API.
       ic_network::HttpErrorCode m_eHttpSessionErrCode = 
                   ic_network::HttpErrorCode::eERR_UNKNOWN;

       //! member variable to http response value of activation API.
       long m_lHttpRespCode = 0;

       //! member variable to hold DeviceID received from activation API.
       std::string m_strDeviceID = "";

       //! member variable to hold PassCode received from activation API.
       std::string m_strPassCode = "";

       //! member variable to hold Response string received from activation API.
       std::string m_strRespString = "";
   };

   /**
    * CAuthRequest class to contain the parameters for auth request,
    * to fetch token.
    */
   class CAuthRequest
   {
   public:
       //! member variable to hold login string required for auth API.
       std::string m_strLoginStr = "";

       //! member variable to hold PassCode required for auth API.
       std::string m_strPasscodeStr = "";

       //! member variable indicating whether device type to be used or not.
       bool m_bUseDevType = false;

       //! member variable to hold Product type required for auth API.
       std::string m_strProductType = "";
   };

   /**
    * CAuthResponse class to contain the parameters received as response to auth
    * API call.
    */
   class CAuthResponse
   {
   public:

       //! member variable to hold http session return value of auth API.
       ic_network::HttpErrorCode m_eHttpSessionErrCode = 
                                   ic_network::HttpErrorCode::eERR_UNKNOWN;

       //! member variable to hold http response code.
       long m_lHttpRespCode = 0;

       //! member variable to hold http response header.
       std::string m_strHttpRespHeader = "";

       //! member variable to hold http response data.
       std::string m_strHttpRespData = "";

       //! member variable to hold token received.
       std::string m_strToken = "";

       //! member variable to hold token time.
       std::string m_strTokenTime = "";

       //! member variable to hold token Expiry time.
       std::string m_strExpTime = "";

       //! member variable to hold token Issue time .
       unsigned long long m_ullTokenIssuedOn = 0;

       //! member variable to hold token time to live.
       unsigned long long m_ullTokenTTL = 0; 
   };


    /**
     * CIgniteConnHealthCheckResponse class to contain the response for
     * health check API call.
     */
    class CIgniteConnHealthCheckResponse 
    {
    public:
        //! member variable to hold http code of the response.
        long m_lHttpCode = 0;

        //! member variable to hold the http session response code.
       ic_network::HttpErrorCode m_eHttpSessionErrCode = 
                                   ic_network::HttpErrorCode::eERR_UNKNOWN;
    };

    /**
     * Method to set actiovation url.
     * @param[in] strUrl The activation url
     * @return true -> on Success , false -> on failure
     */
    bool SetActivationURL(const std::string strUrl);

    /**
     *  Method to set auth url.
     * @param[in] strUrl The auth url
     * @return true -> on Success , false -> on failure
     */
    bool SetAuthURL(const std::string strUrl);

    /**
     *  Method to set health check url.
     * @param[in] strUrl The health check url
     * @return true -> on Success , false -> on failure
     */
    bool SetHealthCheckURL(const std::string strUrl);

    /**
     * Method to activate the device.
     * @param[in] rReq The activation request parameters
     * @see CIgniteHTTPConnector::CActivationRequest
     * @param[out] rResp The activation reesponse from cloud
     * @see CIgniteHTTPConnector::CActivationResponse
     * @param[in] strUrl Optional activation url, if notspecified value set
     *            using setActivationURL API will be used.
     * @return void
     */
    void Activate(const CActivationRequest &rReq, CActivationResponse &rResp,
                  std::string strUrl = "");

    /**
     * Method to activate the device.
     * @param[in] rReq The auth request parameters
     * @see CIgniteHTTPConnector::CAuthRequest
     * @param[out] rResp The auth response from cloud
     * @see CIgniteHTTPConnector::CAuthResponse
     * @param[in] strUrl Optional auth url, if not specified, value set using
     *           SetAuthURL API will be used.
     * @return void
     */
    void GetAuthToken(const CAuthRequest &rReq, CAuthResponse &rResp,
                       std::string strUrl = "");

    /**
     * Method to get connection health check status.
     * @param[out] rResp The health check update response from cloud
     * @see CIgniteHTTPConnector::CIgniteConnHealthCheckResponse
     * @param[in] strUrl Optional health check  url, if not specified ,
     *           value set using setHealthCheckURL API will be used.
     * @return void
     */
    void GetConnectionHealthCheckStatus(CIgniteConnHealthCheckResponse &rResp,
                                        std::string strUrl= "");

private:
    /**
     * Default no-argument constructor.
     */
    CIgniteHTTPConnector();

    /**
     *  Destructor.
     */
    ~CIgniteHTTPConnector();

    //! member variable to hold activation url.
    std::string m_strActivationUrl;

    //! member variable to hold auth url.
    std::string m_strAuthUrl;

    //! member variable to hold login url.
    std::string m_strLoginUrl;

    //! member variable to hold device config sync url.
    std::string m_strDeviceConfigSyncUrl;

    //! member variable to hold wifi response url.
    std::string m_strWifiResponseUrl;

    //! member variable to hold health check url.
    std::string m_strHealthCheckUrl;

    //! member variable to hold batch upload url.
    std::string m_strBatchUploadUrl;

    //! member variable to hold config setting all url.
    std::string m_strConfigSettingAllUrl;
};

} //namespace

#endif // CIGNITE_HTTP_CONNECTOR
