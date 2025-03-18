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

#include <set>
#include "config/CUploadMode.h"
#include "CIgniteConfig.h"
#include "CIgniteLog.h"

//! Macro for 'CUploadMode' string
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CUploadMode"

namespace ic_core 
{

CUploadMode::CUploadMode()
{
    Init();
}

void CUploadMode::Init()
{
    ic_utils::CScopeLock lock(m_DataLock);
    m_ConfiguredModes.reset();
    m_DefaultMode.reset();
    m_mapEventModeMap.clear();
    m_ConfiguredModes.set(eSTREAM);
    m_DefaultMode.set(eSTREAM);

    ic_utils::Json::Value jsonUploadConfig = 
                        CIgniteConfig::GetInstance()->GetJsonValue("uploadMode");
    if (jsonUploadConfig != ic_utils::Json::Value::nullRef) 
    {
        HCPLOG_I << "read uploadMode config";
        std::bitset<2> tempConfiguredModes;
        ic_utils::Json::Value jsonSupported = jsonUploadConfig["supported"];
        ic_utils::Json::Value jsonEvents = jsonUploadConfig["events"];

        if (jsonSupported == ic_utils::Json::Value::nullRef) 
        {
            /*
             * Stream will be supported by default if "supported" array is empty
             * In that case also we need to read stream event list
             */
            HCPLOG_I << "by default stream mode supported if list is emtpy";
            jsonSupported.append("stream");
        }

        HCPLOG_I << "read supported modes";
        ReadSupportedModes(jsonSupported, jsonEvents, tempConfiguredModes);

        if (jsonEvents != ic_utils::Json::Value::nullRef && 
            jsonUploadConfig["events"].isMember("default"))
        {
            m_DefaultMode = GetModeBitSet(jsonUploadConfig["events"]["default"]
                                               .asString(),tempConfiguredModes);

            /*
             * Default mode must contain configured modes
             */
            m_DefaultMode &= tempConfiguredModes;
        } 
        else
        {
            m_DefaultMode = tempConfiguredModes;
        }

        HCPLOG_I << "defaultMode :" << m_DefaultMode;
        m_ConfiguredModes = tempConfiguredModes;

        m_bAnonymousUploadMode = IsAnonymousUploadModeSupported(jsonUploadConfig);

        HCPLOG_I << "Anonymous upload :" << m_bAnonymousUploadMode;

        if (jsonUploadConfig.isMember("storeAndForward") && 
            jsonUploadConfig["storeAndForward"].isBool()) 
        {
            m_bStoreAndForwardEnabled = 
                                   jsonUploadConfig["storeAndForward"].asBool();
        }
        else 
        {
            m_bStoreAndForwardEnabled = false;
        }
        HCPLOG_I << "Store and Forward :" << m_bStoreAndForwardEnabled;
    }
    HCPLOG_I << "configuredModes :" <<m_ConfiguredModes;
}

void CUploadMode::ReadSupportedModes(const ic_utils::Json::Value &rjsonSupported,
                                     const ic_utils::Json::Value &rjsonEvents,
                                     std::bitset<2> &rTempConfiguredModes)
{
    for (int nIval = 0; nIval < rjsonSupported.size(); nIval++)
    {
        std::string strMode = rjsonSupported[nIval].asString();
        HCPLOG_I << "mode: "<< strMode;
        rTempConfiguredModes |= GetModeBitSet(strMode);
        if (rjsonEvents != ic_utils::Json::Value::nullRef)
        {
            FillEventModeMap(rjsonEvents, strMode);
        }
    }
}

bool CUploadMode::IsAnonymousUploadModeSupported(const ic_utils::Json::Value &rjsonUploadConfig)
{
    if (rjsonUploadConfig.isMember("anonymousUpload") && 
        rjsonUploadConfig["anonymousUpload"].isBool()) 
    {
        return rjsonUploadConfig["anonymousUpload"].asBool();
    }
    else 
    {
        return false;
    }
}

void CUploadMode::ReloadInstance()
{
    HCPLOG_D << "reloading instance";
    Init();
}

CUploadMode::~CUploadMode()
{

}

CUploadMode* CUploadMode::GetInstance()
{
    static CUploadMode instance;
    return &instance;
}

std::bitset<2> CUploadMode::GetModeBitSet(const std::string &rstrMode, 
                                      std::bitset<2> defaultBitset)
{
    std::bitset<2> result;
    if (rstrMode == "stream" || rstrMode == "all")
    {
        result.set(eSTREAM);
    }
    else if (rstrMode == "batch" || rstrMode == "all")
    {
        result.set(eBATCH);
    }
    else if (rstrMode != "none")
    {
        result = defaultBitset;
        HCPLOG_E << "Invalid mode :"<< rstrMode 
                 <<" Use default :" << defaultBitset;
    }
    return result;
}

void CUploadMode::FillEventModeMap(const ic_utils::Json::Value &rjsonEventList,
                               const std::string &rstrMode)
{
    if (rjsonEventList.isMember(rstrMode)) 
    {
        HCPLOG_I << "reading event list for mode: "<< rstrMode;
        ic_utils::Json::Value jsonEvents = rjsonEventList[rstrMode];
        for (int i = 0; i < jsonEvents.size(); i++) 
        {
            std::string strEvent = jsonEvents[i].asString();
            HCPLOG_I << "event :"<< strEvent;
            std::bitset<2> uploadMode;
            if (m_mapEventModeMap.find(strEvent) != m_mapEventModeMap.end())
            {
                uploadMode = m_mapEventModeMap[strEvent];
            }
            uploadMode |= GetModeBitSet(rstrMode);
            m_mapEventModeMap[strEvent] = uploadMode;
            HCPLOG_I << strEvent << " " << uploadMode;
        }
    }
}

bool CUploadMode::IsEventSupportedForBatch(const std::string &rstrEventId) const
{
    ic_utils::CScopeLock lock(m_DataLock);
    std::map<std::string, std::bitset<2> >::const_iterator iter = 
                                            m_mapEventModeMap.find(rstrEventId);
    const std::bitset<2>& uploadMode = (iter != m_mapEventModeMap.end()) ? 
                                                   iter->second : m_DefaultMode;
    return uploadMode.test(eBATCH);
}

bool CUploadMode::IsEventSupportedForStream(const std::string &rstrEventId) const
{
    ic_utils::CScopeLock lock(m_DataLock);
    std::map<std::string, std::bitset<2> >::const_iterator iter = 
                                            m_mapEventModeMap.find(rstrEventId);
    const std::bitset<2>& uploadMode = (iter != m_mapEventModeMap.end()) ?
                                                   iter->second : m_DefaultMode;
    return uploadMode.test(eSTREAM);
}

bool CUploadMode::IsBatchModeSupported() const
{
    ic_utils::CScopeLock lock(m_DataLock);
    return m_ConfiguredModes.test(eBATCH);
}

bool CUploadMode::IsStreamModeSupported() const
{
    ic_utils::CScopeLock lock(m_DataLock);
    return m_ConfiguredModes.test(eSTREAM);
}

bool CUploadMode::IsAnonymousUploadSupported() const
{
    return m_bAnonymousUploadMode;
}

bool CUploadMode::IsStoreAndForwardSupported() const
{
    return m_bStoreAndForwardEnabled;
}

bool CUploadMode::IsBatchModeSupportedAsDefault() const
{
    ic_utils::CScopeLock lock(m_DataLock);
    return m_DefaultMode.test(eBATCH);
}

bool CUploadMode::IsStreamModeSupportedAsDefault() const
{
    ic_utils::CScopeLock lock(m_DataLock);
    return m_DefaultMode.test(eSTREAM);
}

std::set<std::string> CUploadMode::GetBatchModeEventList() const
{
    ic_utils::CScopeLock lock(m_DataLock);
    std::set<std::string> setEventList;

    for (std::map<std::string, std::bitset<2> >::const_iterator itr = 
         m_mapEventModeMap.begin();
        itr != m_mapEventModeMap.end(); ++itr)
    {
        const std::bitset<2> modes = itr->second;
        if(modes.test(eBATCH))
        {
            HCPLOG_T << itr->first;
            setEventList.insert(itr->first);
        }
    }
    return setEventList;
}

std::set<std::string> CUploadMode::GetStreamModeEventList() const
{
    ic_utils::CScopeLock lock(m_DataLock);
    std::set<std::string> setEventList;

    for (std::map<std::string, std::bitset<2> >::const_iterator itr = 
        m_mapEventModeMap.begin();
        itr != m_mapEventModeMap.end(); ++itr)
    {
        const std::bitset<2> modes = itr->second;
        if(modes.test(eSTREAM)) 
        {
            HCPLOG_T << itr->first;
            setEventList.insert(itr->first);
        }
    }
    return setEventList;
}
}
