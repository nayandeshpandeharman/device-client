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

#include "CClientConnectorImpl.h"
#include "CIgniteClient.h"
#include "db/CLocalConfig.h"
#include "auth/CActivationState.h"

namespace ic_bl
{
//! Constant key for 'LastKnownLocation' string
static const std::string KEY_LAST_KNOWN_LOCATION = 
                                            "UploaderService.LastKnownLocation";

CClientConnectorImpl* CClientConnectorImpl::GetInstance()
{
    static CClientConnectorImpl instance;
    return &instance;
}

CClientConnectorImpl::CClientConnectorImpl()
{

}

CClientConnectorImpl::~CClientConnectorImpl()
{

}

void CClientConnectorImpl::GetActivationState(bool &rbState, int &rnReason)
{
    CActivationState::GetInstance()->GetActivationState(rbState, rnReason);
}

std::string CClientConnectorImpl::GetLocation()
{
    return ic_core::CLocalConfig::GetInstance()->Get(KEY_LAST_KNOWN_LOCATION);
}

std::string CClientConnectorImpl::GetAttribute(DeviceAttribute eAttr)
{
    std::string strAttrValue = "";

    switch(eAttr)
    {
    case IClientConnector::eVIN:
        strAttrValue = ic_core::CIgniteClient::GetProductImpl()->GetAttribute(
                                                       ic_core::IProduct::eVIN);
        break;

    case IClientConnector::eSERIAL_NUMBER:
        strAttrValue = ic_core::CIgniteClient::GetProductImpl()->GetAttribute(
                                              ic_core::IProduct::eSerialNumber);
        break;

    case IClientConnector::eIMEI:
        strAttrValue = ic_core::CIgniteClient::GetProductImpl()->GetAttribute(
                                                      ic_core::IProduct::eIMEI);
        break;

    case IClientConnector::eDEVICE_ID:
        strAttrValue = ic_core::CLocalConfig::GetInstance()->Get("login");
        break;

    default:
        break;
    }

    return strAttrValue;
}

std::string CClientConnectorImpl::GetConfig(std::string strConfigKey)
{
    return ic_core::CLocalConfig::GetInstance()->Get(strConfigKey);
}

bool CClientConnectorImpl::PersistConfig(std::string strConfigKey,
                                         std::string strConfigValue)
{
    return ic_core::CLocalConfig::GetInstance()->Set(strConfigKey,
                                                     strConfigValue);
}
}/* namespace ic_bl */
