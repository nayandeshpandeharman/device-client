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
*******************************************************************************
* \file CHttpResponse.h
*
* \brief This class provides the necessary interfaces to process the cloud    *
*          response received through HTTPS channel.                           *
*******************************************************************************
*/

#ifndef CHTTP_RESPONSE_H
#define CHTTP_RESPONSE_H

#include <string>
#include "HttpErrorCodes.h"

using std::string;

namespace ic_network
{

/**
 * CHttpResponse class exposing APIs to handle the response of a HTTPS request.
 */
class CHttpResponse
{
public:
    /**
     * Default no-argument constructor
     */
    CHttpResponse();

    /**
     * Destructor
     */
    ~CHttpResponse();

    /**
     * Method to set the last http communication error code
     * @param[in] HttpErrorCode The enum error code
     * @return void
     */
    void SetLastError(HttpErrorCode);

    /**
     * Method to get the last http communication error code
     * @param void
     * @return HttpErrorCode The enum error code
     */
    HttpErrorCode GetLastError();

    /**
     * Method to set the http response code
     * @param[in] long The http code
     * @return void
     */
    void SetHttpCode(long);

    /**
     * Method to get the http response code
     * @param void
     * @return http code
     */
    long GetHttpCode();

    /**
     * Method to set the http response header
     * @param[in] string The http response string
     * @return void
     */
    void SetHttpResponseHeader(string);

    /**
     * Method to get the http response header
     * @param void
     * @return http response string
     */
    string GetHttpResponseHeader();

    /**
     * Method to set the http response data
     * @param[in] string The http response data
     * @return void
     */
    void SetRespData(string);

    /**
     * Method to get the http response data
     * @param void
     * @return http response data
     */
    string GetRespData();

    /**
     * Method to set the authentication error flag
     * @param[in] bool The flag to be set
     * @return void
     */
    void SetAuthError(bool);

    /**
     * Method to check if authentication error occurred or not
     * @param void
     * @return True if auth error occurred; false otherwise
     */
    bool IsAuthError();

    /**
     * Method to reset all internal response related data
     * @param void
     * @return void
     */
    void Reset();

    /**
     * Method to get the HttpErrorCode in associated string format
     * @param[in] eErr The http error code
     * @return string value associated to given HttpErrorCode
     */
    static const std::string GetHttpErrorCodeString(HttpErrorCode eErr);

private:
    /**
     * Member variable to hold the http error code
     */
    HttpErrorCode m_eLastError;

    /**
     * Member variable to hold the http response code
     */
    long m_lHttpCode;

    /**
     * Member variable to hold the http response header
     */
    string m_strHttpResponseHeader;

    /**
     * Member variable to hold the http response data
     */
    string m_strHttpRespdata;

    /**
     * Member variable to track the status of auth error
     */
    bool m_bAuthError;
};

}
#endif /* CHTTP_RESPONSE_H */
