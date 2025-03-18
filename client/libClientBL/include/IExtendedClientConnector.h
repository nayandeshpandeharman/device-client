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
* \file IExtendedClientConnector.h
*
* \brief This extended client connector provides interface used by Internal and 
* Analytics handlers to interact with deviceClient to utilize Client's internal
* resources.
********************************************************************************
*/


#ifndef IEXTENDED_CLIENT_CONNECTOR_H
#define IEXTENDED_CLIENT_CONNECTOR_H

#include <string>
#include "jsoncpp/json.h"
#include "IClientConnector.h"

namespace ic_bl
{
/**
 * Class IExtendedClientConnector provides interface to interact with 
 * deviceClient to utilize Client's internal resources.
 */
class IExtendedClientConnector : virtual public ic_core::IClientConnector
{
public:
    /**
     * Default no-argument constructor.
     */
    IExtendedClientConnector(){}

    /**
     * Destructor
     */
    virtual ~IExtendedClientConnector(){}

    /**
     * Method to get activation state
     * @param[out] rbState activation state
     * @param[out] rnReason activation failure reason
     * @return void
     */
    virtual void GetActivationState(bool &rbState, int &rnReason) = 0;

};
} /* namespace ic_bl */
#endif /* IEXTENDED_CLIENT_CONNECTOR_H */
