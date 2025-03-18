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
#include <ctype.h>
#include <algorithm>
#include <typeinfo>
#include "db/CContentValues.h"
#include "CIgniteLog.h"
#include "CIgniteStringUtils.h"

//! Macro for 'CContentValues' string
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CContentValues"

namespace ic_core 
{
namespace 
{
//! Constant key for 'true' string
static const std::string STR_TRUE = "true";

//! Constant key for 'false' string
static const std::string STR_FALSE = "false";
}

CContentValues::CContentValues()
{
    m_jsonData.clear();
}

std::vector<std::string> CContentValues::GetKeys()
{
    return m_jsonData.getMemberNames();
}

void CContentValues::Clear()
{
    m_jsonData.clear();
}

int CContentValues::Size()
{
    return m_jsonData.size();
}

void CContentValues::Put(std::string strKey, int nValue)
{
    m_jsonData[strKey] = nValue;
}

void CContentValues::Put(std::string strKey, long long llValue)
{
    m_jsonData[strKey] = llValue;
}

void CContentValues::Put(std::string strKey, std::string strValue)
{
    m_jsonData[strKey] = strValue;
}

void CContentValues::Put(std::string strKey, const char* pchValue)
{
    Put(strKey, std::string(pchValue));
}

void CContentValues::Put(std::string strKey, bool bValue)
{
    m_jsonData[strKey] = bValue;
}

void CContentValues::Put(std::string strKey, float fltValue)
{
    m_jsonData[strKey] = fltValue;
}

void CContentValues::Put(std::string strKey, double dblValue)
{
    m_jsonData[strKey] = dblValue;
}

std::string CContentValues::GetString(std::string strKey)
{
    return m_jsonData[strKey].asString();
}

std::string CContentValues::GetAsString(std::string strKey, std::string strDef)
{
    if (!m_jsonData.isMember(strKey))
    {
        return strDef;
    }
    
    return GetString(strKey);
}

long long CContentValues::GetAsLong(std::string strKey, long long llDef)
{
    if (!m_jsonData.isMember(strKey))
    {
        return llDef;
    }
    
    std::string strValue = GetString(strKey);    
    long long llValue = ic_utils::CIgniteStringUtils::StringToNumber<long long>(strValue);    
    return llValue;
}

int CContentValues::GetAsInt(std::string strKey, int nDef)
{
    return GetAsLong(strKey, nDef);
}

double CContentValues::GetAsDouble(std::string strKey, double dblDef)
{
    if (!m_jsonData.isMember(strKey))
    {
        return dblDef;
    }
    
    std::string strValue = GetString(strKey);
    double dblValue = ic_utils::CIgniteStringUtils::StringToNumber<double>(strValue);      
    return dblValue;
}

float CContentValues::GetAsFloat(std::string strKey, float fltDef)
{
    return GetAsDouble(strKey, fltDef);
}

bool CContentValues::GetAsBool(std::string strKey, bool bDef)
{
    if (!m_jsonData.isMember(strKey))
    {
        return bDef;
    }
    
    std::string strValue = GetString(strKey);
    std::transform(strValue.begin(), strValue.end(), strValue.begin(), tolower);
    if (0 == STR_TRUE.compare(strValue))
    {
        return true;
    }
    else if (0 == STR_FALSE.compare(strValue))
    {
        return false;
    }
    else
    {
        HCPLOG_E << "could not convert " << strValue  
                 << " to bool. Returning the default value " << bDef;
        return bDef; 
    }
}
} /* namespace ic_core */
