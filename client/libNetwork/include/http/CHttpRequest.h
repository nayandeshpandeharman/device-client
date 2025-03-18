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
* \file CHttpRequest.h
*
* \brief This class provides the abstracted interfaces for libcurl APIs to    *
*        communicate with the cloud through HTTPS channel.                    *
*******************************************************************************
*/

#ifndef CHTTP_REQUEST_H
#define CHTTP_REQUEST_H

#include <string>
#include <list>
#include "CHttpResponse.h"

using std::string;

namespace ic_network
{

/**
 * ProxySetting structure to track the proxy details.
 */
typedef struct
{
    string m_strHost; //!< The host name
    int m_nPort;      //!< The port number
    string m_strUser; //!< The User name
    string m_strPassword; //!< The Password

    /**
     * Method to reset the member variables
     */
    void Reset()
    {
        m_strHost = "";
        m_nPort = -1;
        m_strUser = "";
        m_strPassword = "";
    }
}ProxySetting;

/**
 * LocalPortRange structure to track the local port range details.
 */
typedef struct
{
    int m_nStart; //!< The Port start number
    int m_nEnd;   //!< The Port end number

    /**
     * Method to reset the member variables
     */
    void Reset()
    {
        m_nStart = -1;
        m_nEnd = -1;
    }
}LocalPortRange;

/**
 * FormBuffer structure to track the to-be-uploaded buffer content details
 */
typedef struct
{
    string m_strFormName; //!< Name given for the buffer content
    string m_strContentType; //!< Type of the content
    void *m_pvoidBufferPtr;  //!< A pointer to the buffer to be uploaded
    long m_lBufferSize;      //!< The Length of the buffer
    bool m_bUploadAsFile;    //!< Buffer has to be uploaded as a file or not

    /**
     * Method to reset the member variables
     */
    void Reset()
    {
        m_strFormName = "";
        m_strContentType = "";
        m_pvoidBufferPtr = NULL;
        m_lBufferSize = -1;
        m_bUploadAsFile = true;
    }
}FormBuffer;

/**
 * FormFile structure to track the to-be-uploaded file(s) details
 */
typedef struct
{
    string m_strFormName; //!< Name given for the buffer content
    string m_strContentType; //!< Type of the content
    string m_strFilePath;    //!< An absolute file path on device
    string m_strAttachmentUploadName; //!< Name for the upload file content

    /**
     * Method to reset the member variables
     */
    void Reset()
    {
        m_strFormName = "";
        m_strContentType = "";
        m_strFilePath = "";
        m_strAttachmentUploadName = "";
    }
}FormFile;

/**
 * CHttpRequest class exposing APIs to make any HTTPS request.
 */
class CHttpRequest
{
public:
    /**
     * Default no-argument constructor.
     */
    CHttpRequest();

    /**
     *  Destructor.
     */
    virtual ~CHttpRequest();

    /**
     * Method to set the given url to the request.
     * @param[in] url The http URL to set
     * @return void
     */
    void SetUrl(const string strUrl);

    /**
     * Method to get the url that was set before.
     * @param void
     * @return  The http url
     */
    string GetUrl();

    /**
     * Method to set the http-request timeout.
     * @param[in] nTimeout Timeout value
     * @return void
     */
    void SetTimeout(const int nTimeout);

    /**
     * Method to get the http-request timeout value that was set before.
     * @param void
     * @return Timeout value
     */
    const int GetTimeout();

    /**
     * Method to set the http-request postfield value
     * @param[in] strPostFields Postfield value
     * @return void
     */
    void SetPostFields(const string strPostFields);

    /**
     * Method to get the http-request postfield value that was set before.
     * @param void
     * @return Postfield value
     */
    string GetPostFields();

    /**
     * Method to set below headers in one request.
     *                  "Accept: application/json"
     *                  "Content-Type: application/json"
     *                  "charsets: utf-8"
     * @param void
     * @return void
     */
    void SetJsonType();

    /**
     * Method to add given header to the request.
     * @param[in] strHeader A header string
     * @return void
     */
    void AddHeader(const string strHeader);

    /**
     * Method to set the headers given in the list. This will overwrite
     *      any previously added headers.
     * @param[in] liststrHeaders A list of header strings
     * @return void
     */
    void SetHeaders(const std::list<string> liststrHeaders);

    /**
     * Method to get all the previously added headers.
     * @param void
     * @return A list of string containing all the header strings
     */
    std::list<string> GetHeaders();

    /**
     * Method to set the proxy details.
     * @param[in] strHost The host name
     * @param[in] unPort The port number
     * @param[in] strUser The User name (default is empty)
     * @param[in] strPassword The Password (default is empty)
     * @return void
     */
    void SetProxy(const string& rstrHost, const unsigned int unPort,
            const string& rstrUser = "", const string& rstrPassword = "");

    /**
     * Method to get the previously added proxy details.
     * @param void
     * @return A struct containing the proxy details.
     */
    ProxySetting GetProxy();

    /**
     * Method to set the additional local ports to try.
     * @param[in] nStart  The Port start number
     * @param[in] nEnd  The Port end number
     * @return void
     */
    void SetLocalPortRange(const unsigned int nStart, const unsigned int nEnd);

    /**
     * Method to get the additional local ports set before.
     * @param void
     * @return A struct containing the local port details
     */
    LocalPortRange GetLocalPortRange();

    /**
     * Method to add a buffer content to be uploaded
     * @param[in] strFormName The Name given for the buffer content
     * @param[in] strContentType The Type of the content
     * @param[in] pBufferPtr A pointer to the buffer to be uploaded
     * @param[in] lBufferSize The Length of the buffer.
     * @param[in] bUploadAsFile A Flag indicating if the buffer has to be 
     *            uploaded as a file or not
     * @return void
     */
    void AddFormFromBuffer(const string strFormName, const string strContentType,
            const void* pvoidBufferPtr, const long lBufferSize, 
            bool bUploadAsFile=true);

    /**
     * Method to get the buffer content details that were added before
     *                  using addFormFromBuffer() API.
     * @param void
     * @return A list containing the buffer content details.
     */
    std::list<FormBuffer> GetFormBuffers();

    /**
     * Method to attach a file to be uploaded
     * @param[in] strFormName The name given for the buffer content
     * @param[in] strContentType  The type of the content
     * @param[in] strFilePath An absolute file path on device
     * @param[in] strUploadfilePath Name to be set for the upload file content.
     * @return void
     */
    void AddFormFromFile(const string strFormName, const string strContentType,
            const string strFilePath, const std::string strUploadfilePath="");

    /**
     * Method to get the file attachment details that were added before
     *                  using addFormFromFile() before.
     * @param void
     * @return A list containing the file attachment details.
     */
    std::list<FormFile> GetFormFiles();

    /**
     * Method to remove all the previously added headers
     *                  from the request.
     * @param void
     * @return void
     */
    void ClearHeaders();

    /**
     * Method to remove all the previously added buffer contents
     *                  from the request.
     * @param void
     * @return void
     */
    void ClearFormBuffers();

    /**
     * Method to clear all the previously added buffer contents
     *                  from the request.
     * @param void
     * @return void
     */
    void ClearFormFiles();

    /**
     * Method to clear all the settings added to the request.
     * @param void
     * @return void
     */
    void ResetAll();

    /**
     * Method to execute the requeset as a HTTP POST Request.
     * @param[out] CHttpResponse A struct containing the HTTP response details.
     * @return An error code indicating the request execution status
     */
    virtual HttpErrorCode Execute(CHttpResponse&);

    /**
     * Method to execute the request as a HTTP GET Request.
     * @param[out] CHttpResponse The struct containing the HTTP response details
     * @return The error code indicating the request execution status
     */
    virtual HttpErrorCode ExecuteGET(CHttpResponse&);

private:
    /**
     * Method to check if proxy settings being applied for http session is
     * valid or not.
     * @param[in] rstProxy ProxySettings to be set for HTTP session
     * @return true if proxy settings are valid, false otherwise
     */
    bool IsProxySettingsValid(const ProxySetting &rstProxy);

    /**
     * Method to check if local port range being applied for http session is
     * valid or not.
     * @param[in] rstPortRange Local Port Range to be set for HTTP session
     * @return true if Local port range is valid, false otherwise
     */
    bool IsPortRangeValid(const LocalPortRange &rstPortRange);

    /**
     * member variable to hold the url of the request
     */
    string m_strUrl;

    /**
     * member variable to hold the timeout value of the request
     */
    int m_nTimeout;

    /**
     * member variable to hold the postfields content
     */
    string m_strPostFields;

    /**
     * member variable to hold the header details
     */
    std::list<string> m_strHeaders;

    /**
     * member variable to hold the proxy details
     */
    ProxySetting m_stProxy;

    /**
     * member variable to hold the local port range details
     */
    LocalPortRange m_stPortRange;

    /**
     * member variable to hold the to-be-uploaded upload buffer content details
     */
    std::list<FormBuffer> m_stFormBuffs;

    /**
     * member variable holding the to-be-uploaded file(s) details
     */
    std::list<FormFile> m_stFormFiles;
};

}
#endif /* CHTTP_REQUEST_H */
