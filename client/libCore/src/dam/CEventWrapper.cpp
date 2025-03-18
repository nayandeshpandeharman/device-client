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

#include "dam/CEventWrapper.h"

namespace ic_core 
{
void CEventWrapper::SetEventId(std::string strId)
{
    m_jsonEventFields[ic_event::EVENT_ID_TAG] = strId;
}

int CEventWrapper::GetInt(const std::string &rstrKey, int nDefaultValue)
{
    if (m_jsonValue.isMember(rstrKey) && m_jsonValue[rstrKey].isInt())
    {
        return m_jsonValue[rstrKey].asInt();
    }
    
    return nDefaultValue;
}

bool CEventWrapper::GetBool(const std::string &rstrKey, bool bDefaultValue)
{
    if (m_jsonValue.isMember(rstrKey) && m_jsonValue[rstrKey].isBool())
    {
        return m_jsonValue[rstrKey].asBool();
    }

    return bDefaultValue;
}

std::string CEventWrapper::GetString(const std::string &rstrKey,
                                    std::string strDefaultValue)
{
    /* Removing m_value[rstrKey].isString() check from validation as it is taken 
       care while assiging (asString() call) */
    if (m_jsonValue.isMember(rstrKey))
    {
        return m_jsonValue[rstrKey].asString();
    }
    
    return strDefaultValue;
}

long long CEventWrapper::GetLong(const std::string &rstrKey, 
                                long long llDefaultValue)
{
    if (m_jsonValue.isMember(rstrKey))
    {
#if defined(JSON_HAS_INT64)
        return m_jsonValue[rstrKey].asInt64();
#else
        return m_jsonValue[rstrKey].asLong();
#endif        
    }
    
    return llDefaultValue;
}

void CEventWrapper::AddPiiField(const std::string &rstrName,
                               const std::string &rstrVal) 
{
    m_jsonPiiFields[rstrName] = rstrVal;
}

void CEventWrapper::AddPiiField(const std::string &rstrName, 
                               const ic_utils::Json::Value &rjsonVal)
{
    m_jsonPiiFields[rstrName] = rjsonVal;
}

std::vector<std::string> CEventWrapper::GetAttachments()
{
    return m_vectAttachment;
}

void CEventWrapper::RemoveAttachments()
{
    m_vectAttachment.clear();
}

ic_utils::Json::Value CEventWrapper::GetData() 
{
    return m_jsonValue;
}

int CEventWrapper::GetTimezone()
{
    return m_jsonEventFields[ic_event::TIMEZONE_TAG].asInt();
}

bool CEventWrapper::Hasfield(const std::string &rstrKey)
{
    return m_jsonValue.isMember(rstrKey);
}
} /* namespace ic_core */