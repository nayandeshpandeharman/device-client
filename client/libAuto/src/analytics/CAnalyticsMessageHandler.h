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
* \file CAnalyticsMessageHandler.h
*
* \brief Analytics data processing component which registers the Analytics
*        Handlers
*******************************************************************************
*/

#ifndef CANALYTICS_MESSAGE_HANDLER_H
#define CANALYTICS_MESSAGE_HANDLER_H

#include <vector>
#include "jsoncpp/json.h"
#include "IExtendedClientConnector.h"
#include "CIgniteMutex.h"
#include "CConcurrentQueue.h"
#include "analytics/CEventProcessor.h"
#include "analytics/CBaseMessageHandler.h"
#include "CIgniteConfig.h"

namespace ic_auto
{

/**
 * CAnalyticsMessageHandler class manages all the automotive related data
 * handlers in terms of initiating,mapping the data with the associated
 * handlers and Notifying Handlers.
 */
class CAnalyticsMessageHandler : public ic_core::CBaseMessageHandler,
                                 public ic_core::IConfigUpdateNotification
{

public:

    /**
     * Default Constructor
     */
    CAnalyticsMessageHandler();

    /**
     * Destructor
     */
    ~CAnalyticsMessageHandler();

    /**
     * Overriding Method of CBaseMessageHandler class
     * @see CBaseMessageHandler::Init()
     */
    void Init() override;

    /**
     * Overriding Method of CBaseMessageHandler class
     * @see CBaseMessageHandler::NotifyMessage()
     */
    void NotifyMessage(MsgPayload msgPayload) override;

    /**
     * Overriding Method of CBaseMessageHandler class
     * @see CBaseMessageHandler::SetClientConnector()
     */
    void SetClientConnector(ic_bl::IExtendedClientConnector *cCnctr);

    /**
     * Overriding Method of CBaseMessageHandler class
     * of ImessageHandler
     * @see CBaseMessageHandler::UpdateEventProcessorMapForEventHandler()
     */
    void UpdateEventProcessorMapForEventHandler
                                       (const std::string &rstrDomain) override;

    /**
     * Overriding Method of CBaseMessageHandler class
     * @see CBaseMessageHandler::
     *      UpdateEventProcessorMapForNotificationHandler()
     */
    void UpdateEventProcessorMapForNotificationHandler
                                       (const std::string &rstrDomain) override;

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
};

} /* namespace ic_auto */

#endif /* CANALYTICS_MESSAGE_HANDLER_H */
