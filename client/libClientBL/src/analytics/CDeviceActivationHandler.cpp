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

#include "IClientMessageDispatcher.h"
#include "CIgniteClient.h"
#include "CDeviceActivationHandler.h"
#include "CIgniteLog.h"
#include "CClientConn.h"

//! Macro for CDeviceActivationHandler string
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CDeviceActivationHandler"

//! Constant key for 'ActivationStateRequest' string
static const std::string EVENTID_ACTIVATION_REQUEST = "ActivationStateRequest";

namespace ic_bl
{
CDeviceActivationHandler::CDeviceActivationHandler()
{

}

CDeviceActivationHandler::~CDeviceActivationHandler() 
{

}

CDeviceActivationHandler* CDeviceActivationHandler::GetInstance() 
{
    HCPLOG_METHOD();
    static CDeviceActivationHandler instance;
    return &instance;
}

void CDeviceActivationHandler::ReleaseInstance()
{
    HCPLOG_METHOD();
    //do required cleanup here
}

void CDeviceActivationHandler::ProcessEvent(ic_core::CEventWrapper &rEvent) 
{
    std::string strEvntId = rEvent.GetEventId();
    HCPLOG_D << "Event is:" << strEvntId;
    if (EVENTID_ACTIVATION_REQUEST == strEvntId)
    {
        UpdateActivationStatusChanged();
    }
}

void CDeviceActivationHandler::UpdateActivationStatusChanged()
{
    /* Set default value of "state" = mActivationState(false) and 
     * reason = mNotActivationReason(no_error=-1) default value
     */
    bool bState = false;
    int nReason = -1;
    ic_core::CClientConn::GetInstance()->GetConnector()->
                                            GetActivationState(bState, nReason);
    HCPLOG_D << "ActivationState:" << bState 
             << ",notActivationReason:" << nReason;

    ic_core::IClientMessageDispatcher *pDispatcher =
                           ic_core::CIgniteClient::GetClientMessageDispatcher();
    if (pDispatcher)
    {
        pDispatcher->DeliverDeviceActivationStatusMessage(bState, nReason);
    }
}
}/* namespace ic_bl */
