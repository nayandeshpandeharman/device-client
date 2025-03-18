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
* \file CDisassociationRequestHandler.h
*
* \brief This class implements dongle disassociation mechanism
********************************************************************************
*/

#ifndef CDISASSOCIATION_REQUEST_HANDLER_H
#define CDISASSOCIATION_REQUEST_HANDLER_H

#include "analytics/CEventProcessor.h"
#include "jsoncpp/json.h"

namespace ic_bl
{
/**
 * Class CDisassociationRequestHandler subscribes and handles disassociation 
 * request
 */
class CDisassociationRequestHandler : public ic_core::CEventProcessor
{
public:
    /**
     * Destructor
     */
    ~CDisassociationRequestHandler();

    /**
     * Method to get Instance of CDisassociationRequestHandler
     * @param void
     * @return Pointer to Singleton Object of CDisassociationRequestHandler
     */
    static CDisassociationRequestHandler *GetInstance();

    /**
     * Method to cleanup resources allocated by class
     * @param void
     * @return void
     */
    static void ReleaseInstance();

    /**
     * Implements Method of CEventProcessor
     * @see CEventProcessor::ProcessEvent()
     */
    void ProcessEvent(ic_core::CEventWrapper &rEvent) override {};
    
    /**
     * Implements Method of CEventProcessor
     * @see CEventProcessor::ApplyConfig()
     */
    void ApplyConfig(ic_utils::Json::Value &rjsonConfigValue) override;

    /**
     * Method to handle disassociation request
     * @param void
     * @return void
     */
    void Handle();

private:
    /**
     * Default no-argument constructor.
     */
    CDisassociationRequestHandler();
};
} /* namespace ic_bl */
#endif //CDISASSOCIATION_REQUEST_HANDLER_H