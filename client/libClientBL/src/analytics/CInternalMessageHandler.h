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
* \file CInternalMessageHandler.h
*
* \brief This class handle client internal messages and notifications.
********************************************************************************
*/

#ifndef CINTERNAL_MESSAGE_HANDLER_H
#define CINTERNAL_MESSAGE_HANDLER_H

#include <set>
#include "IExtendedClientConnector.h"
#include "analytics/CEventProcessor.h"
#include "analytics/CBaseMessageHandler.h"
#include "CIgniteConfig.h"

namespace ic_bl
{
/**
 * Class CInternalMessageHandler handle client internal messages and 
 * notifications.
 */
class CInternalMessageHandler : public ic_core::CBaseMessageHandler, 
                               public ic_core::IConfigUpdateNotification
{
public:
    /**
     * Default no-argument constructor.
     */
    CInternalMessageHandler();

    /**
     * Destructor
     */
    ~CInternalMessageHandler();

    /**
     * Overriding Method of CBaseMessageHandler class
     * @see CBaseMessageHandler::NotifyMessage()
     */
    void NotifyMessage(MsgPayload stMsgPayload) override;

    /**
     * Overriding Method of CBaseMessageHandler class
     * @see CBaseMessageHandler::SetClientConnector()
     */
    void SetClientConnector(IExtendedClientConnector *pClientCnctr);

    /**
     * Overriding Method of CBaseMessageHandler class
     * @see CBaseMessageHandler::UpdateEventProcessorMapForEventHandler()
     */
    void UpdateEventProcessorMapForEventHandler(const std::string &rstrDomain)
                                                override;

    /**
     * Overriding Method of CBaseMessageHandler class
     * @see CBaseMessageHandler::UpdateEventProcessorMapForNotificationHandler()
     */
    void UpdateEventProcessorMapForNotificationHandler(const std::string 
                                                       &rstrDomain) override;

    /**
     * Overriding Method of CBaseMessageHandler class
     * @see CBaseMessageHandler::Init()
     */
    void Init() override;

    /**
     * Overriding Method of CBaseMessageHandler class
     * @see CBaseMessageHandler::DeInit()
     */
    void DeInit() override;

    /**
     * Overriding Method of IConfigUpdateNotification class
     * @see IConfigUpdateNotification::NotifyConfigUpdate()
     */
    void NotifyConfigUpdate() override;

#ifdef IC_UNIT_TEST
    friend class CInternalMessageHandlerTest;
#endif

    /**
     * Overriding Method of CBaseMessageHandler::IMessageHandler class
     * @see CBaseMessageHandler::IMessageHandler::GetSupplimentaryEventsList()
     */
    std::set<std::string> GetSupplimentaryEventsList() override;
};
} /* namespace ic_bl */
#endif /* CINTERNAL_MESSAGE_HANDLER_H */