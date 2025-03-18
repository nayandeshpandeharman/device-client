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

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CurlHttpSession"

#if (defined(CURL_ENABLED) || !defined(SOCKETSSL_ENABLED))
#include "CCurlHttpSession.h"

#include "CIgniteLog.h"
using namespace std;

namespace ic_network
{

namespace 
{

/**
 * CURLOPT_WRITEFUNCTION - callback for writing received data
 */
extern "C" size_t write_data_callback(void *pvoidPtr, size_t size, 
                                      size_t nmemb, void *pvoidUserdata)
{
    // HCPLOG_METHOD() << userdata << " - " << (char*)pvoidPtr;
    std::string *pstrWriteString = 
                            reinterpret_cast<std::string *>(pvoidUserdata);
    size_t nbytes = size * nmemb;
    pstrWriteString->append((char *)pvoidPtr, nbytes);
    return nbytes;
}

/**
 * CURLOPT_HEADERFUNCTION - callback that receives header data
 */
extern "C" size_t header_callback(void *pvoidData, size_t size, size_t nmemb,
                                  void *pvoidUserdata)
{
    std::string *pstrHeaderString = 
                                reinterpret_cast<std::string *>(pvoidUserdata);
    size_t nbytes = size * nmemb;
    pstrHeaderString->append((char *)pvoidData, nbytes);
    return nbytes;
}

} /* namespace */

CCurlHttpSession::CCurlHttpSession(CSSLAttributes *pSslAttributes)
{
    m_pCurlHandle = curl_easy_init();
    HCPLOG_T << "curl_easy_init returned curlHandle=" << m_pCurlHandle;
    HCPLOG_METHOD() << "this=" << this << ", m_pCurlHandle=" << m_pCurlHandle;
    m_pstCurlHttpHeaders = NULL;
    m_pstCurlFormaddPost = NULL;
    m_pstCurlFormaddLast = NULL;
    m_pSSLAttributes = pSslAttributes;
    SetDefaultCurlOptions();
}

void CCurlHttpSession::SetDefaultCurlOptions()
{

    //curl_easy_setopt(m_pCurlHandle, CURLOPT_VERBOSE, true);
    curl_easy_setopt(m_pCurlHandle, CURLOPT_WRITEFUNCTION, write_data_callback);
    curl_easy_setopt(m_pCurlHandle, CURLOPT_WRITEDATA, &m_strCurlData);

    // set the header callback function 
    curl_easy_setopt(m_pCurlHandle, CURLOPT_HEADERFUNCTION, header_callback);
    // callback object for headers
    curl_easy_setopt(m_pCurlHandle, CURLOPT_HEADERDATA, &m_strCurlRespHeaders);

    curl_easy_setopt(m_pCurlHandle, CURLOPT_NOPROGRESS, 1);
    //changing default value for VERIFYPEER from 0 to 1 to fix ASOC issue
    curl_easy_setopt(m_pCurlHandle, CURLOPT_SSL_VERIFYPEER, 1L);
    //changing default value for VERIFYHOST from 0 to 1 to fix ASOC issue
    curl_easy_setopt(m_pCurlHandle, CURLOPT_SSL_VERIFYHOST, 1L);
    curl_easy_setopt(m_pCurlHandle, CURLOPT_DNS_USE_GLOBAL_CACHE, 0L);
    curl_easy_setopt(m_pCurlHandle, CURLOPT_DNS_CACHE_TIMEOUT, 0L);
    curl_easy_setopt(m_pCurlHandle, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(m_pCurlHandle, CURLOPT_TIMEOUT, 60L);
    curl_easy_setopt(m_pCurlHandle, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(m_pCurlHandle, CURLOPT_FOLLOWLOCATION, 1L);
    /*curl_easy_setopt(m_pCurlHandle, CURLOPT_SSL_CIPHER_LIST,
     *   "TLSv1.2"); 
     * for HTTP"NULL-SHA256,AES128-SHA256,AES256-SHA256,AES128-GCM-SHA256,
     * AES256-GCM-SHA384,DH-RSA-AES128-SHA256,DH-RSA-AES256-SHA256,
     * DH-RSA-AES128-GCM-SHA256,DH-RSA-AES256-GCM-SHA384,DH-DSS-AES128-SHA256,
     * DH-DSS-AES256-SHA256,DH-DSS-AES128-GCM-SHA256,DH-DSS-AES256-GCM-SHA384,
     * DHE-RSA-AES128-SHA256,DHE-RSA-AES256-SHA256,DHE-RSA-AES128-GCM-SHA256,
     * DHE-RSA-AES256-GCM-SHA384,DHE-DSS-AES128-SHA256,DHE-DSS-AES256-SHA256,
     * DHE-DSS-AES128-GCM-SHA256,DHE-DSS-AES256-GCM-SHA384,
     * ECDHE-RSA-AES128-SHA256,ECDHE-RSA-AES256-SHA384,
     * ECDHE-RSA-AES128-GCM-SHA256,ECDHE-RSA-AES256-GCM-SHA384,
     * ECDHE-ECDSA-AES128-SHA256,ECDHE-ECDSA-AES256-SHA384,
     * ECDHE-ECDSA-AES128-GCM-SHA256,ECDHE-ECDSA-AES256-GCM-SHA384,
     * ADH-AES128-SHA256,ADH-AES256-SHA256,ADH-AES128-GCM-SHA256,
     * ADH-AES256-GCM-SHA384,AES128-CCM,AES256-CCM,DHE-RSA-AES128-CCM,
     * DHE-RSA-AES256-CCM,AES128-CCM8,AES256-CCM8,DHE-RSA-AES128-CCM8,
     * DHE-RSA-AES256-CCM8,ECDHE-ECDSA-AES128-CCM,ECDHE-ECDSA-AES256-CCM,
     * ECDHE-ECDSA-AES128-CCM8,ECDHE-ECDSA-AES256-CCM8");
     */ 
    curl_easy_setopt(m_pCurlHandle,CURLOPT_SSL_CIPHER_LIST,"ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256");
}

void CCurlHttpSession::ApplyTLSSettings()
{
    // TLS settings related configurations
    if (m_pSSLAttributes != NULL)
    {
        // Verify Peer and Host
        SetVerifyPeerAndHost();

        // CA Path
        SetCAPath();

        // CA info
        SetCAInfo();

        // Cert info
        SetCertInfo();

        // Cert Type
        // default = "PEM" will be set
        SetCertType();

        // Key path
        SetKeyPath();

        // Key type
        SetKeyType();

        // TLS engine id
        SetTLSEngineID();

        curl_easy_setopt(m_pCurlHandle, CURLOPT_SSLVERSION,
                         CURL_SSLVERSION_TLSv1_2);
        // curl_easy_setopt(m_pCurlHandle, CURLOPT_CERTINFO, 1L);
#if 0
        std::FILE *filep = fopen("curl.txt", "wb");
        curl_easy_setopt(m_pCurlHandle, CURLOPT_STDERR, filep);
        //TBD - remove after debugging is completed
        curl_easy_setopt(m_pCurlHandle, CURLOPT_VERBOSE, 1);
#endif
    }
    else
    {
        HCPLOG_W << "No TLS settings!";
    }
}

void CCurlHttpSession::SetVerifyPeerAndHost()
{
    bool bVerifyPeer = m_pSSLAttributes->GetVerifyPeer();
    bool bVerifyHost = m_pSSLAttributes->GetVerifyHost();

    curl_easy_setopt(m_pCurlHandle, CURLOPT_SSL_VERIFYPEER,
                     bVerifyPeer ? 1L : 0L);
    curl_easy_setopt(m_pCurlHandle, CURLOPT_SSL_VERIFYHOST,
                     bVerifyHost ? 2L : 0L);
}

void CCurlHttpSession::SetCAPath()
{
    std::string strCapath = m_pSSLAttributes->GetCApath();
    if (!strCapath.empty())
    {
        curl_easy_setopt(m_pCurlHandle, CURLOPT_CAPATH,
                         strCapath.empty() ? NULL : strCapath.c_str());
    }
}

void CCurlHttpSession::SetCAInfo()
{
    std::string strCafile = m_pSSLAttributes->GetCAfile();
    if (!strCafile.empty())
    {
        curl_easy_setopt(m_pCurlHandle, CURLOPT_CAINFO,
                         strCafile.empty() ? NULL : strCafile.c_str());
    }
}

void CCurlHttpSession::SetCertInfo()
{
    std::string strCertfile = m_pSSLAttributes->GetCertFile();
    if (!strCertfile.empty())
    {
        curl_easy_setopt(m_pCurlHandle, CURLOPT_SSLCERT,
                         strCertfile.empty() ? NULL : strCertfile.c_str());
    }
}

void CCurlHttpSession::SetCertType()
{
    std::string strCertType = m_pSSLAttributes->GetCertType();
    curl_easy_setopt(m_pCurlHandle, CURLOPT_SSLCERTTYPE,
                     strCertType.empty() ? "PEM" : strCertType.c_str());
}

void CCurlHttpSession::SetKeyPath()
{
    std::string strKeypath = m_pSSLAttributes->GetKeyFile();
    if (!strKeypath.empty())
    {
        curl_easy_setopt(m_pCurlHandle, CURLOPT_SSLKEY,
                         strKeypath.empty() ? NULL : strKeypath.c_str());
    }
}

void CCurlHttpSession::SetKeyType()
{
    std::string strKeyType = m_pSSLAttributes->GetKeyType();
    /* Following section is applicable if keytype is ENG
     * https://curl.se/libcurl/c/CURLOPT_SSLKEYTYPE.html
     */
    if (!strKeyType.empty())
    {
        curl_easy_setopt(m_pCurlHandle, CURLOPT_SSLKEYTYPE,
                         strKeyType.empty() ? "PEM" : strKeyType.c_str());
    }
}

void CCurlHttpSession::SetTLSEngineID()
{
    bool bUseTLSEngine = m_pSSLAttributes->UseTLSengine();
    if (bUseTLSEngine)
    {
        std::string strEngineID = m_pSSLAttributes->GetTLSengineID();
        curl_easy_setopt(m_pCurlHandle, CURLOPT_SSLENGINE,
                         strEngineID.empty() ? NULL : strEngineID.c_str());
    }
}

void CCurlHttpSession::SetUrl(const std::string strUrl)
{
    HCPLOG_METHOD() << "this=" << this << " url=" << strUrl << 
                       " m_pCurlHandle=" << m_pCurlHandle;
    if (!strUrl.empty())
    {
        curl_easy_setopt(m_pCurlHandle, CURLOPT_URL, strUrl.c_str());
    }
}

void CCurlHttpSession::SetTimeout(const int nTimeout)
{
    HCPLOG_D << "timeout=" << nTimeout;
    curl_easy_setopt(m_pCurlHandle, CURLOPT_TIMEOUT, nTimeout);
}

void CCurlHttpSession::SetPostFields(const std::string &rstrPostFields)
{
    HCPLOG_D << "postFields=" << rstrPostFields;
    if (!rstrPostFields.empty())
    {
        curl_easy_setopt(m_pCurlHandle, CURLOPT_COPYPOSTFIELDS,
                         rstrPostFields.c_str());
    }
}

void CCurlHttpSession::Reset()
{
    HCPLOG_METHOD();

    m_strCurlData.clear();

    ClearHeaders();
    if (m_pstCurlFormaddPost != NULL)
    {
        curl_formfree(m_pstCurlFormaddPost);
        m_pstCurlFormaddPost = NULL;
        m_pstCurlFormaddLast = NULL;
    }
    curl_easy_setopt(m_pCurlHandle, CURLOPT_HTTPPOST, NULL);
    curl_easy_setopt(m_pCurlHandle, CURLOPT_POST, 0L);
    curl_easy_reset(m_pCurlHandle);
    SetDefaultCurlOptions();

}

void CCurlHttpSession::ClearHeaders()
{
    HCPLOG_METHOD();
    if (m_pstCurlHttpHeaders != NULL)
    {
        curl_slist_free_all(m_pstCurlHttpHeaders);
        m_pstCurlHttpHeaders = NULL;
    }
    curl_easy_setopt(m_pCurlHandle, CURLOPT_HTTPHEADER, m_pstCurlHttpHeaders);

}

void CCurlHttpSession::SetHeaders(const std::list<std::string> strlistHeaders)
{
    HCPLOG_METHOD();

    if (m_pstCurlHttpHeaders != NULL)
    {
        curl_slist_free_all(m_pstCurlHttpHeaders);
    }

    for (std::list<std::string>::const_iterator it = strlistHeaders.begin(); 
         it != strlistHeaders.end(); it++)
    {
        m_pstCurlHttpHeaders = curl_slist_append(m_pstCurlHttpHeaders, 
                                                 (*it).c_str());
        HCPLOG_D << "Adding Header: " << (*it).c_str();
    }

    curl_easy_setopt(m_pCurlHandle, CURLOPT_HTTPHEADER, m_pstCurlHttpHeaders);
}

void CCurlHttpSession::AddHeader(const std::string &rstrHeader)
{
    HCPLOG_D << rstrHeader;
    m_pstCurlHttpHeaders = curl_slist_append(m_pstCurlHttpHeaders, 
                                             rstrHeader.c_str());
    curl_easy_setopt(m_pCurlHandle, CURLOPT_HTTPHEADER, m_pstCurlHttpHeaders);
}

void CCurlHttpSession::SetProxy(const std::string &rstrHost,
                               const unsigned int unPort,
                               const std::string &rstrUser,
                               const std::string &rstrPassword)
{
    curl_easy_setopt(m_pCurlHandle, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
    curl_easy_setopt(m_pCurlHandle, CURLOPT_PROXY, rstrHost.c_str());
    curl_easy_setopt(m_pCurlHandle, CURLOPT_PROXYPORT, unPort);
    if (!rstrUser.empty() && !rstrPassword.empty())
    {
        std::string strUserpwd = rstrUser + ":" + rstrPassword;
        curl_easy_setopt(m_pCurlHandle, CURLOPT_PROXYUSERPWD, 
                         strUserpwd.c_str());
    }
}

void CCurlHttpSession::SetLocalPortRange(const unsigned int unStart,
                                    const unsigned int unEnd)
{
    if (unStart > 0 && unStart <= 65535 &&
        unEnd > 0 && unEnd <= 65535)
    {
        int range = unEnd - unStart + 1;
        curl_easy_setopt(m_pCurlHandle, CURLOPT_LOCALPORT, unStart);
        curl_easy_setopt(m_pCurlHandle, CURLOPT_LOCALPORTRANGE, range);
    }
}

HttpErrorCode CCurlHttpSession::PerformGETRequest()
{
    /* since curl internally handles, just forwarding the request 
     * to performRequest()
     */
    return PerformRequest();
}

HttpErrorCode CCurlHttpSession::PerformRequest()
{
    HCPLOG_METHOD();
    m_strCurlData.clear();
    HttpErrorCode eErr = eERR_UNKNOWN;
    char *url = NULL;
    curl_easy_getinfo(m_pCurlHandle, CURLINFO_EFFECTIVE_URL, &url);
    if(url) {
        HCPLOG_C << m_pCurlHandle << " exec..." << url;
    }
    else {
        HCPLOG_E << "could not retrieve url!";
    }
    m_CurlError = curl_easy_perform(m_pCurlHandle);
    HCPLOG_D << m_pCurlHandle << " done.";
    switch (m_CurlError)
    {
    case CURLE_OK:
        eErr = ProcessCurlResponse();
        break;

    default:
        eErr = ProcessCurlError(m_CurlError);
        break;
    }

    HCPLOG_C << m_pCurlHandle << ".res=" << m_CurlError << ", err=" << eErr <<
              ", httpCode=" << m_lCurlHttpCode << ", respData=" << 
              m_strCurlData;


    if (m_pstCurlFormaddPost != NULL)
    {
        curl_formfree(m_pstCurlFormaddPost);
        m_pstCurlFormaddPost = NULL;
        m_pstCurlFormaddLast = NULL;
    }

    m_eLastError = eErr;
    return eErr;
}

HttpErrorCode CCurlHttpSession::ProcessCurlResponse()
{
    HttpErrorCode eErrCode = eERR_UNKNOWN;
    curl_easy_getinfo(m_pCurlHandle, CURLINFO_RESPONSE_CODE,
                      &m_lCurlHttpCode);
    switch (m_lCurlHttpCode)
    {
    // These error codes will be set by the server
    case 500: // internal server error
    case 503: // Service unavailable
    case 504: // Gateway Timeout
    case 408: // request timeout
    case 404: // resource not found
        eErrCode = eERR_SERVER;
        break;
    default:
        eErrCode = eERR_OK;
        break;
    }

    return eErrCode;
}

HttpErrorCode CCurlHttpSession::ProcessCurlError(const CURLcode curlError)
{
    HttpErrorCode eErrCode = eERR_UNKNOWN;
    switch (m_CurlError)
    {
    case CURLE_COULDNT_RESOLVE_HOST:
    case CURLE_COULDNT_RESOLVE_PROXY:
    case CURLE_COULDNT_CONNECT:
        // Intentional fall-through for all network related errors
        eErrCode = eERR_NETWORK;
        break;
    case CURLE_OPERATION_TIMEDOUT:
        eErrCode = eERR_TIMEOUT;
        break;
    case CURLE_SSL_CONNECT_ERROR:
    case CURLE_TOO_MANY_REDIRECTS:
    case CURLE_GOT_NOTHING:
        eErrCode = eERR_SERVER;
        break;
    default:
        eErrCode = eERR_OTHER;
        break;
    }

    return eErrCode;
}

HttpErrorCode CCurlHttpSession::GetLastError()
{
    return m_eLastError;
}

long CCurlHttpSession::GetHttpCode()
{
    return m_lCurlHttpCode;
}
const std::string CCurlHttpSession::GetData()
{
    return m_strCurlData;
}

CCurlHttpSession::~CCurlHttpSession()
{
    HCPLOG_METHOD();

    if (m_pstCurlHttpHeaders != NULL)
    {
        curl_slist_free_all(m_pstCurlHttpHeaders);
        m_pstCurlHttpHeaders = NULL;
    }
    if (m_pstCurlFormaddPost != NULL)
    {
        curl_formfree(m_pstCurlFormaddPost);
        m_pstCurlFormaddPost = NULL;
        m_pstCurlFormaddLast = NULL;
    }
    curl_easy_cleanup(m_pCurlHandle);
    HCPLOG_T << "curl_easy_cleanup on curlHandle=" << m_pCurlHandle;
    m_pCurlHandle = NULL;
}

void CCurlHttpSession::AddFormFromBuffer(const string strFormName,
                                        const string strContentType,
                                        const void *pvoidBufferPtr,
                                        const long lBufferSize,
                                        bool bUploadAsFile)
{
    HCPLOG_D << "formName=" << strFormName << ", bufferSize=" << lBufferSize;

    if (bUploadAsFile)
    {
        HCPLOG_D << "Uploading as file...";
        curl_formadd(&m_pstCurlFormaddPost, &m_pstCurlFormaddLast,
                     CURLFORM_COPYNAME, strFormName.c_str(),
                     CURLFORM_BUFFER, strFormName.c_str(),
                     CURLFORM_BUFFERPTR, (const char *)pvoidBufferPtr,
                     CURLFORM_BUFFERLENGTH, (long)lBufferSize,
                     CURLFORM_CONTENTTYPE, strContentType.c_str(),
                     CURLFORM_END);
    }
    else {
        HCPLOG_D << "Uploading as a buffer...";
        curl_formadd(&m_pstCurlFormaddPost, &m_pstCurlFormaddLast,
                     CURLFORM_COPYNAME, strFormName.c_str(),
                     CURLFORM_BUFFERPTR, (const char *)pvoidBufferPtr,
                     CURLFORM_BUFFERLENGTH, (long)lBufferSize,
                     CURLFORM_CONTENTTYPE, strContentType.c_str(),
                     CURLFORM_END);
    }
    curl_easy_setopt(m_pCurlHandle, CURLOPT_HTTPPOST, m_pstCurlFormaddPost);
}

void CCurlHttpSession::AddFormFromFile(const std::string strFormName,
                                      const std::string strContentType,
                                      const std::string strFilePath,
                                      const std::string strUploadfilePath)
{
    HCPLOG_D << "formName=" << strFormName;
    HCPLOG_D << "filePath=" << strFilePath;
    HCPLOG_D << "uploadfilePath=" << strUploadfilePath;

    if (strUploadfilePath.empty())
    {
        curl_formadd(&m_pstCurlFormaddPost, &m_pstCurlFormaddLast,
                     CURLFORM_COPYNAME, strFormName.c_str(),
                     CURLFORM_FILE, strFilePath.c_str(),
                     CURLFORM_CONTENTTYPE, strContentType.c_str(),
                     CURLFORM_END);
    }
    else {
        curl_formadd(&m_pstCurlFormaddPost, &m_pstCurlFormaddLast,
                     CURLFORM_COPYNAME, strFormName.c_str(),
                     CURLFORM_FILE, strFilePath.c_str(), 
                     CURLFORM_FILENAME, strUploadfilePath.c_str(),
                     CURLFORM_CONTENTTYPE, strContentType.c_str(),
                     CURLFORM_END);
    }
    curl_easy_setopt(m_pCurlHandle, CURLOPT_HTTPPOST, m_pstCurlFormaddPost);
}


}
#endif
