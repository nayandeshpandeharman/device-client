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
* \file CConfigRequestHandler.h
*
* \brief This class handles request for configured message type
********************************************************************************
*/

#ifndef CCONFIG_REQUEST_HANDLER_H
#define CCONFIG_REQUEST_HANDLER_H

#include "CIgniteMessage.h"
#include "CMessageQueue.h"

namespace ic_bl
{
/**
 * Class CConfigRequestHandler handle request for 'eGET_CONFIG' message type
 */
class CConfigRequestHandler : public ic_core::IMessageReceiver
{
public:
    /**
     * Parameterized Constructor
     * @param[in] pPublisher pointer instance of CMessageQueue
     * @return No return
     */
    CConfigRequestHandler(ic_core::CMessageQueue *pPublisher);

    /**
     * Destructor
     */
    virtual ~CConfigRequestHandler();

    /**
     * Implements Method of IMessageReceiver
     * @see IMessageReceiver::Handle()
     */
    bool Handle(const ic_event::CIgniteMessage &rMsg) override;
};
} /* namespace ic_bl */
#endif /* CCONFIG_REQUEST_HANDLER_H */
