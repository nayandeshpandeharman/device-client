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

#include "CIgniteLog.h"
#include "CActivationState.h"
#include "CIgniteEvent.h"

//! Macro for 'CActivationState' string
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CActivationState"

//! Constant key for 'ActivationStateRequest' string
static const std::string EVENTID_ACTIVATION_REQUEST = "ActivationStateRequest";

//! Constant key for 'TokenChanged' string
static const std::string EVENTID_TOKEN_CHANGED = "TokenChanged";

namespace ic_bl
{
CActivationState::CActivationState(): m_bActivationState(false),
                                    m_eNotActivationReason(eNO_ERROR)
{

}

CActivationState *CActivationState::GetInstance()
{
    HCPLOG_METHOD();
    static CActivationState instance;
    return &instance;
}

CActivationState::~CActivationState()
{

}

void CActivationState::SetActivationState(bool bState,
                                    ActivationFailureError eNotActivationReason)
{

    HCPLOG_C << "SetActivationState:" << bState 
             << " ,notActivationReason: " << eNotActivationReason;
    if (m_bActivationState != bState 
        || m_eNotActivationReason != eNotActivationReason)
    {
        UpdateActivationStateChanged();
        m_eNotActivationReason = eNotActivationReason;
        m_bActivationState = bState;
    }
}

void CActivationState::GetActivationState(bool &rbState, int &rnReason)
{
    rbState = m_bActivationState;
    rnReason = m_eNotActivationReason;
    HCPLOG_D << "Get activation state:" << rbState << " ,Reason:" << rnReason;
}

void CActivationState::UpdateActivationStateChanged()
{
    HCPLOG_T << "Send ActivationStateRequest Event";
    ic_event::CIgniteEvent event("1.0", EVENTID_ACTIVATION_REQUEST);
    event.Send();
}

void CActivationState::SetToken(std::string strToken)
{
    if (m_strToken != strToken)
    {
        m_strToken = strToken;
        UpdateTokenChanged();
    }
}

void CActivationState::UpdateTokenChanged()
{
    HCPLOG_T << "Send token updated Event";
    ic_event::CIgniteEvent event("1.0", EVENTID_TOKEN_CHANGED);
    event.Send();
}

std::string CActivationState::GetTokenValue()
{
    HCPLOG_METHOD();
    return m_strToken;
}
}/* namespace ic_bl */
