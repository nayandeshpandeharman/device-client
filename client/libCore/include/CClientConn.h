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
* \file CClientConn.h
*
* \brief Helper class to contain the IExtendedClientConnector interface object
* in order to be used by many analytics handlers using singleton pattern.     
*******************************************************************************
*/

#ifndef CCLIENT_CONN_H
#define CCLIENT_CONN_H

#include "jsoncpp/json.h"
#include "IExtendedClientConnector.h"

namespace ic_core
{
/**
 * Class provides set and get interfaces for ClientConnector object reference
 * using which IExtendedClientConnector interface APIs can be invoked.
 */
class CClientConn
{  
public:
    /**
     * Method to return object reference of CClientConn class
     * @param void
     * @return CClientConn object reference
     */
    static CClientConn* GetInstance();

    /**
     * Destructor
     */
    ~CClientConn();

    /**
     * Method to set the IExtendedClientConnector object reference
     * @param[in] pConn Client connector object reference
     * @return void
     */
    void SetClientConnector(ic_bl::IExtendedClientConnector* pConn);

    /**
     * Method to get the ClientConnector object reference
     * @param void
     * @return IExtendedClientConnector object reference
     */
    ic_bl::IExtendedClientConnector* GetConnector();

private:
    /**
     * Default no-argument constructor.
     */
    CClientConn();

    //! Member variable stores ClientConnector object reference
    ic_bl::IExtendedClientConnector *m_pClientCnctr;
};
} /* namespace ic_core */
#endif //CCLIENT_CONN_H
