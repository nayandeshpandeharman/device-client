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
* \file CTransportHandlerBase.h
*
* \brief This class is base handler for all the responbility handlers.
********************************************************************************
*/

#ifndef CTRANSPORT_HANDLER_BASE_H
#define CTRANSPORT_HANDLER_BASE_H

#include <string>
#include "dam/CEventWrapper.h"
namespace ic_bl 
{

/**
 * Class CTransportHandlerBase is base handler for the responsiblity handlers 
 * like CActivityDelay, CEventTimestampValidationHandler etc.
 * It handles the event transport through multiple responsiblity handlers.
 */
class CTransportHandlerBase
{
public:
    /**
     * Parameterized constructor
     * @param[in] pNextHandler Pointer to the next transport handler
     */
    CTransportHandlerBase(CTransportHandlerBase* pNextHandler);

    /**
     * Method to handle event for its responsibilty based on handler's object
     * @param[in] pEvent Event data
     * @return void
     */
    virtual void HandleEvent(ic_core::CEventWrapper* pEvent);

    /**
     * Destructor
     */
    virtual ~CTransportHandlerBase() {};

protected:
    //! Member variable holding object of next responsiblity handler
    CTransportHandlerBase* m_pNextHandler;
};
} /* namespace ic_bl*/
#endif /* CTRANSPORT_HANDLER_BASE_H */
