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

#include <stdlib.h>
#include <algorithm>
#include "CDisassociationRequestHandler.h"
#include "CIgniteClient.h"
#include "CIgniteLog.h"
#include "db/CLocalConfig.h"
#include "core/CPersistancyAndStateHandler.h"

//! Macro for CDisassociationRequestHandler string
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CDisassociationRequestHandler"

//! Constant key for 'deviceDisassociated' string
static const std::string KEY_DISASSOCIATION = "deviceDisassociated";

namespace ic_bl
{
CDisassociationRequestHandler *CDisassociationRequestHandler::GetInstance()
{
    static CDisassociationRequestHandler requestHandler;
    return &requestHandler;
}

void CDisassociationRequestHandler::ReleaseInstance()
{
    HCPLOG_METHOD();
    //do required cleanup here
}

CDisassociationRequestHandler::CDisassociationRequestHandler() 
{

}

CDisassociationRequestHandler::~CDisassociationRequestHandler()
{
    
}

void CDisassociationRequestHandler::Handle()
{
    CPersistancyAndStateHandler::GetInstance()->ClearActivationStatus();
    
    ic_utils::Json::Value jsonPayload;
    jsonPayload["status"] = ic_core::IC_ACTIVATION_STATUS::eNOT_ACTIVATED;
    jsonPayload["vin"] = ic_core::CIgniteClient::GetProductImpl()->
                                                     GetActivationQualifierID();
    ic_core::CIgniteClient::GetClientMessageDispatcher()->
                                          DeliverActivationDetails(jsonPayload);

    ic_core::CLocalConfig::GetInstance()->Set(KEY_DISASSOCIATION, "true");
    HCPLOG_C << "Device is disassociated";

    HCPLOG_C << "Restart analytics";
    ic_core::CIgniteClient::RestartAnalytics(
                                ic_core::ExitCode::eDISASSOCIATION_EXIT, false);
}

void CDisassociationRequestHandler::ApplyConfig(ic_utils::Json::Value 
                                               &rjsonConfigValue)
{
    HCPLOG_METHOD();
    Handle();
}
}/* namespace ic_bl */
