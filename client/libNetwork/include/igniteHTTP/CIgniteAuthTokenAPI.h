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
* \file CIgniteAuthTokenAPI.h
*
* \brief This class provides the necessary interfaces to interact              *
*        with Ignite Cloud for auth token API                                  *
********************************************************************************
*/

#ifndef CIGNITE_AUTH_TOKEN_API_H
#define CIGNITE_AUTH_TOKEN_API_H

#include "CIgniteHTTPConnector.h"

namespace ic_network
{

/**
 * CIgniteAuthTokenAPI class exposing APIs necessary for fetching auth token.
 */
class CIgniteAuthTokenAPI
{
public:
    /**
     * Default no-argument constructor.
     */
    CIgniteAuthTokenAPI();

    /**
     *  Destructor.
     */
    ~CIgniteAuthTokenAPI();

    /**
     * Method to fetch auth token from Ignite cloud.
     * @param[in] rReq auth token request parameters
     * @see CIgniteHTTPConnector::CAuthRequest
     * @param[out] rResp auth token response from cloud
     * @see CIgniteHTTPConnector::CAuthResponse
     * @param[in] strUrl auth token url
     * @return void
     */
    void FetchAuthToken(const CIgniteHTTPConnector::CAuthRequest &rReq,
         CIgniteHTTPConnector::CAuthResponse &rResp,std::string strUrl);

#ifdef IC_UNIT_TEST
    friend class CIgniteAuthTokenAPITest;
    
    /**
     * Method to get the date from the http response header.
     * @param[in] rstrHeaderResp http response header
     * @return date in the string format
     */
    std::string GetDateFromHeader(std::string &rstrHeaderResp);

    /**
     * Method to Convert from the format Thu, 15 Mar 2018 06:48:13 GMT
     * to ":"Y-M-DTHH:MM:SS". Here T is the separator.
     * @param[in] rstrTime Date(string) in the format Thu, 15 Mar 2018 06:48:13 GMT
     * @return date in the converted format
     */
    std::string GetTimeFormatAsLogin(std::string &rstrTime);

    
#endif

private:
    /**
     * Method to build the auth string to be used for fetching the token.
     * @param[in] strLoginStr Device ID
     * @param[in] strPasscodeStr Passcode
     * @return Auth string to be used for fetching the token
     */
    std::string BuildAuthCode(const std::string strLoginStr,
                                  const std::string strPasscodeStr);

    /**
     * Method to parse the response from from auth API
     * @param[out] rResp Response class to hold the values sent from API
     * @return void
     */
    void ParseAPIResponse(CIgniteHTTPConnector::CAuthResponse &rResp);

    /**
     * Method to parse the payload and update Token Time, Expiry time and TTL
     * @param[in] rstrEncodedPayLoad Encoded Payload from the API response
     * @param[out] rResp Response class to hold the values sent from AP
     * @return void
     */
    void UpdateTokenTime(const std::string &rstrEncodedPayLoad,
                         CIgniteHTTPConnector::CAuthResponse &rResp);
};

} // namespace
#endif // CIGNITE_AUTH_TOKEN_API_H
