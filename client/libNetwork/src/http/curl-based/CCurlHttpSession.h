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
* \file CurlHttpSession.h
*
* \brief This class provides methods used for curl based http session.         * 
*        It extends IHttpSession class.                                        *
********************************************************************************
*/

#if (defined(CURL_ENABLED) || !defined(SOCKETSSL_ENABLED))
#ifndef CCURL_HTTP_SESSION_H
#define CCURL_HTTP_SESSION_H

#include <pthread.h>
#include <curl/curl.h>
#include <string>
#include <list>
#include "CIgniteFileUtils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string>

#include "HttpErrorCodes.h"
#include "IHttpSession.h"
#include <jsoncpp/json.h>
#include "CHttpSessionManager.h"

namespace ic_network 
{

/**
 * Class to provide APIs for curl based HTTP transactions
 */
class CCurlHttpSession : public IHttpSession 
{
public:
    /**
     * Parameterized constructor
     * @param[in] pSslAttributes Pointer to the SSLAttributes class, which 
     *            holds the required SSL settings for the session.
     */
    explicit CCurlHttpSession(CSSLAttributes *pSslAttributes);

    /**
     * Destructor
     */
    ~CCurlHttpSession();

    /**
     * Overrides the method IHttpSession::PerformRequest
     * @see ic_network::IHttpSession::PerformRequest()
     */
    HttpErrorCode PerformRequest() override;

    /**
     * Overrides the method IHttpSession::PerformGETRequest
     * @see ic_network::IHttpSession::PerformGETRequest()
     */
    HttpErrorCode PerformGETRequest() override;

    /**
     * Overrides the method IHttpSession::SetUrl
     * @see ic_network::IHttpSession::SetUrl()
     */
    void SetUrl(const std::string strUrl) override;

    /**
     * Overrides the method IHttpSession::SetTimeout
     * @see ic_network::IHttpSession::SetTimeout()
     */
    void SetTimeout(const int nTimeout) override;

    /**
     * Overrides the method IHttpSession::SetPostFields
     * @see ic_network::IHttpSession::SetPostFields()
     */
    void SetPostFields(const std::string &rstrPostFields) override;

    /**
     * Overrides the method IHttpSession::SetHeaders
     * @see ic_network::IHttpSession::SetHeaders()
     */
    void SetHeaders(const std::list<std::string> strlistHeaders) override;

    /**
     * Overrides the method IHttpSession::SetProxy
     * @see ic_network::IHttpSession::SetProxy()
     */
    void SetProxy(const std::string &rstrHost, const unsigned int unPort,
                  const std::string &rstrUser = "",
                  const std::string &rstrPassword = "") override;

    /**
     * Overrides the method IHttpSession::SetLocalPortRange
     * @see ic_network::IHttpSession::SetLocalPortRange()
     */
    void SetLocalPortRange(const unsigned int unStart,
                           const unsigned int unEnd) override;

    /**
     * Overrides the method IHttpSession::AddHeader
     * @see ic_network::IHttpSession::AddHeader()
     */
    void AddHeader(const std::string &rstrHeader) override;

    /**
     * Overrides the method IHttpSession::AddFormFromBuffer
     * @see ic_network::IHttpSession::AddFormFromBuffer()
     */
    void AddFormFromBuffer(const std::string strFormName,
                           const std::string strContentType,
                           const void *pvoidBufferPtr,
                           const long lBufferSize,
                           bool bUploadAsFile = true) override;

    /**
     * Overrides the method IHttpSession::AddFormFromFile
     * @see ic_network::IHttpSession::AddFormFromFile()
     */
    void AddFormFromFile(const std::string strFormName,
                         const std::string strContentType,
                         const std::string strFilePath,
                         const std::string strUploadfilePath = "") override;

    /**
     * Overrides the method IHttpSession::ClearHeaders
     * @see ic_network::IHttpSession::ClearHeaders()
     */
    void ClearHeaders() override;

    /**
     * Overrides the method IHttpSession::Reset
     * @see ic_network::IHttpSession::Reset()
     */
    void Reset() override;

    /**
     * Overrides the method IHttpSession::GetLastError
     * @see ic_network::IHttpSession::GetLastError()
     */
    HttpErrorCode GetLastError() override;

    /**
     * Overrides the method IHttpSession::GetHttpCode
     * @see ic_network::IHttpSession::GetHttpCode()
     */
    long GetHttpCode() override;

    /**
     * Overrides the method IHttpSession::GetHttpResponseHeader
     * @see ic_network::IHttpSession::GetHttpResponseHeader()
     */
    std::string GetHttpResponseHeader() override {return m_strCurlRespHeaders;}

    /**
     * Overrides the method IHttpSession::GetData
     * @see ic_network::IHttpSession::GetData()
     */
    const std::string GetData() override;

    /**
     * Overrides the method IHttpSession::ApplyTLSSettings
     * @see ic_network::IHttpSession::ApplyTLSSettings()
     */
    void ApplyTLSSettings() override;

private:
    /**
     * Method to set default curl options.
     * @param void
     * @return void
     */
    void SetDefaultCurlOptions();

    /**
     * Method to process the curl response.
     * @param void
     * @return HttpErrorCode error code
     */
    HttpErrorCode ProcessCurlResponse();

    /**
     * Method to set default curl options.
     * @param curlError Curl Error received as part of curl_easy_perform
     * @return HttpErrorCode error code
     */
    HttpErrorCode ProcessCurlError(const CURLcode curlError);

    /**
     * member variable to hold the curl handle
     */
    CURL *m_pCurlHandle;

    /**
     * member variable to hold curl data
     */
    std::string m_strCurlData;

    /**
     * member variable to hold the curl response headers
     */
    std::string m_strCurlRespHeaders;

    /**
     * member variable to hold the curl http code
     */
    long m_lCurlHttpCode;

    /**
     * member variable to hold the curl return code
     */
    CURLcode m_CurlError;

    /**
     * member variable to hold the curl return code
     */
    struct curl_slist* m_pstCurlHttpHeaders;

    /**
     * member variable to hold multipart formpost content start
     */
    struct curl_httppost* m_pstCurlFormaddPost;

    /**
     * member variable to hold multipart formpost content last
     */
    struct curl_httppost* m_pstCurlFormaddLast;

    /**
     * member variable to hold last http respone code
     */
    HttpErrorCode m_eLastError;

    /**
     * member variable pointing to SSL Attributes
     */
    CSSLAttributes* m_pSSLAttributes = NULL;

    /**
     * Method to set CURLOPT_SSL_VERIFYPEER and CURLOPT_SSL_VERIFYHOST.
     * values will be read via m_pSSLAttributes
     * @param void
     * @return void
     */
    void SetVerifyPeerAndHost(void);

    /**
     * Method to set CURLOPT_CAPATH.
     * values will be read via m_pSSLAttributes
     * @param void
     * @return void
     */
    void SetCAPath(void);

    /**
     * Method to set CURLOPT_CAINFO.
     * values will be read via m_pSSLAttributes
     * @param void
     * @return void
     */
    void SetCAInfo(void);

    /**
     * Method to set CURLOPT_SSLCERT.
     * values will be read via m_pSSLAttributes
     * @param void
     * @return void
     */
    void SetCertInfo(void);

    /**
     * Method to set CURLOPT_SSLCERTTYPE.
     * values will be read via m_pSSLAttributes
     * @param void
     * @return void
     */
    void SetCertType(void);

    /**
     * Method to set CURLOPT_SSLKEY.
     * values will be read via m_pSSLAttributes
     * @param void
     * @return void
     */
    void SetKeyPath(void);

    /**
     * Method to set CURLOPT_SSLKEYTYPE.
     * values will be read via m_pSSLAttributes
     * @param void
     * @return void
     */
    void SetKeyType(void);

    /**
     * Method to set TLS Engine ID (CURLOPT_SSLENGINE).
     * values will be read via m_pSSLAttributes
     * @param void
     * @return void
     */
    void SetTLSEngineID(void);
};

} /* namespace ic_network */

#endif /* CCURL_HTTP_SESSION_H */

#endif

