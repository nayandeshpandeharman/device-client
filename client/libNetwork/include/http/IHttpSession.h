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
* \file IHttpSession.h
*
* \brief This interface exposes various APIs using which HTTP request session *
*          classes can be defined.                                            *
*******************************************************************************
*/

#ifndef IHTTP_SESSION_H
#define IHTTP_SESSION_H

#include <string>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include "HttpErrorCodes.h"

namespace ic_network 
{

/**
 * Interface for various APIs which will be used for HTTP request
 */
class IHttpSession 
{
public:
    /**
     * Destructor 
     */
    virtual ~IHttpSession(){};

    /**
     * A pure virtual member.
     * Method to execute a HTTP request.
     * @param void
     * @return The http error code corresponding to http call made
     */
    virtual HttpErrorCode PerformRequest() = 0;

    /**
     * A pure virtual member.
     * Method to execute a HTTP GET request.
     * @param void
     * @return The http error code corresponding to http call made
     */
    virtual HttpErrorCode PerformGETRequest() = 0;

    /**
     * A pure virtual member.
     * Method to set the url.
     * @param[in] strUrl The url to be set
     * @return void
     */
    virtual void SetUrl(const std::string strUrl) = 0;

    /**
     * A pure virtual member.
     * Method to set the timeout value.
     * @param[in] nTimeout The value to be set for timeout
     * @return void
     */
    virtual void SetTimeout(const int nTimeout) = 0;

    /**
     * A pure virtual member.
     * Method to set the POST request fields.
     * @param[in] rstrPostFields  The string containing the post fields
     * @return void
     */
    virtual void SetPostFields(const std::string& rstrPostFields) = 0;

    /**
     * A pure virtual member.
     * Method to set the http request headers.
     * @param[in] strlistHeaders A list of header strings
     * @return void
     */
    virtual void SetHeaders(const std::list<std::string> strlistHeaders) = 0;

    /**
     * A pure virtual member.
     * Method to set the proxy details.
     * @param[in] rstrHost  the host name
     * @param[in] unPort  the port number
     * @param[in] rstrUser  user name (default is empty)
     * @param[in] rstrPassword Password (default is empty)
     * @return void
     */
    virtual void SetProxy(const std::string& rstrHost,const unsigned int unPort,
                          const std::string& rstrUser = "", 
                          const std::string& rstrPassword = "") = 0;

    /**
     * A pure virtual member.
     * Method to set the additional local ports to try.
     * @param[in] unStart  The Port start number
     * @param[in] unEnd  The Port end number
     * @return void
     */
    virtual void SetLocalPortRange(const unsigned int unStart,
                                   const unsigned int unEnd) = 0;

    /**
     * A pure virtual member.
     * Method to add given header to the request.
     * @param[in] rstrHeader A header string to be added to the request
     * @return void
     */
    virtual void AddHeader(const std::string& rstrHeader) = 0;

    /**
     * A pure virtual member.
     * Method to add a buffer content to be uploaded
     * @param[in] strFormName The Name given for the buffer content
     * @param[in] strContentType The Type of the content
     * @param[in] pvoidBufferPtr A pointer to the buffer to be uploaded
     * @param[in] lBufferSize The Length of the buffer.
     * @param[in] bUploadAsFile A Flag indicating if the buffer has to be 
     *            uploaded as a file or not
     * @return void
     */
    virtual void AddFormFromBuffer(const std::string strFormName, 
                                   const std::string strContentType,
                                   const void* pvoidBufferPtr, 
                                   const long lBufferSize,
                                   bool bUploadAsFile = true) = 0;

    /**
     * A pure virtual member.
     * Method to attach a file to be uploaded
     * @param[in] strFormName The name given for the buffer content
     * @param[in] strContentType  The type of the content
     * @param[in] strFilePath An absolute file path on device
     * @param[in] strUploadfilePath Name to be set for the upload file content.
     * @return void
     */
    virtual void AddFormFromFile(const std::string strFormName, 
                                 const std::string strContentType,
                                 const std::string strFilePath, 
                                 const std::string strUploadfilePath="") = 0;

    /**
     * A pure virtual member.
     * Method to remove all the previously added headers from the request.
     * @param void
     * @return void
     */
    virtual void ClearHeaders() = 0;

    /**
     * A pure virtual member.
     * Method to clear all the settings of the session.
     * @param void
     * @return void
     */
    virtual void Reset() = 0;

    /**
     * A pure virtual member.
     * Method to get last http error.
     * @param void
     * @return Last http error code
     */
    virtual HttpErrorCode GetLastError() = 0;

    /**
     * A pure virtual member.
     * Method to get the http response code
     * @param void
     * @return The http code
     */
    virtual long GetHttpCode() = 0;

    /**
     * A pure virtual member.
     * Method to get the http response header
     * @param void
     * @return http response string
     */
    virtual std::string GetHttpResponseHeader() = 0;

    /**
     * A pure virtual member.
     * Method to get the http response data
     * @param void
     * @return http response data
     */
    virtual const std::string GetData() = 0;

    /**
     * A pure virtual member.
     * Method to apply TLS settings
     * @param void
     * @return void
     */
    virtual void ApplyTLSSettings() = 0;
};

} /* namespace HCP */

#endif /* IHTTP_SESSION_H */




