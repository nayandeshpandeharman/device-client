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

#include "config/CConfigRequestHandler.h"
#include "CIgniteConfig.h"
#include "CIgniteLog.h"

namespace ic_bl
{
CConfigRequestHandler::CConfigRequestHandler(ic_core::CMessageQueue *pPublisher)
{
    HCPLOG_METHOD();
    pPublisher->Subscribe(ic_event::CMessageTypes::eGET_CONFIG, this);
}

CConfigRequestHandler::~CConfigRequestHandler()
{

}

bool CConfigRequestHandler::Handle(const ic_event::CIgniteMessage &rMsg)
{
    rMsg.Reply(ic_core::CIgniteConfig::GetInstance()->GetAsString(
                                                     rMsg.GetMessageAsString()));
    return true;
}
} /* namespace ic_bl */
