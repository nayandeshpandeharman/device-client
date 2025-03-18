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
* \file IExternalHttpSessionHandler.h
*
* \brief This interface exposes APIs using which a custom implementation for 
*        http communication can be made.                                      *
*******************************************************************************
*/

#ifndef IEXT_HTTP_SESSION_HANDLER_H
#define IEXT_HTTP_SESSION_HANDLER_H

#include "IHttpSession.h"
#include "CSSLAttributes.h"

using namespace ic_network;

/**
 * Interface for APIs that are to be implemented for custom http session
 * management.
 */
class IExternalHttpSessionHandler
{
public:
    /**
     * A pure virtual member.
     * Method to create a http session (of type IHttpSession) by using the 
     * given SSL attributes.
     * @param pSSLAttributes SSL Attributes to use while creating the http
     *        session
     * @return Pointer to the object of the class which implements IHTTPSession
     */
    virtual IHttpSession* CreateSession(CSSLAttributes *pSSLAttributes) = 0;
};

#endif // IEXT_HTTP_SESSION_HANDLER_H