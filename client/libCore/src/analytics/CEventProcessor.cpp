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

#include "db/CLocalConfig.h"
#include "CIgniteStringUtils.h"
#include "analytics/CEventProcessor.h"
#include "CIgniteLog.h"
#include "CIgniteConfig.h"

//! Macro for CEventProcessor string
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CEventProcessor"

namespace ic_core 
{
//! Constant key for 'remoteConfig' string
const std::string DOMAIN_REMOTE_CONTROL = "remoteConfig";

//! Constant key for 'RemoteOperation' string
const std::string DOMAIN_RO_SERVICE = "RemoteOperation";

//! Constant key for 'vinHandler' string
const std::string DOMAIN_VIN_HANDLER = "vinHandler";

//! Constant key for 'disassociation' string
const std::string DOMAIN_DISASSOCIATION_REQUEST = "disassociation";

//! Constant key for 'activationBackOff' string
const std::string DOMAIN_ACTIVATION_BACKOFF = "activationBackOff";

//! Constant key for 'DeviceActivationHandler' string
const std::string DOMAIN_ACTIVATION_HANDLER = "DeviceActivationHandler";

//! Constant key for 'data' string
const std::string KEY_DATA = "data";

//! Constant key for 'version' string
const std::string KEY_VERSION = "version";

//! Constant key for 'command' string
const std::string KEY_CMD = "command";

//! Constant key for 'timestamp' string
const std::string KEY_TS = "timestamp";


void CEventProcessor::Reset()
{
    HCPLOG_METHOD() << "not implemented";
}

bool CEventProcessor::IsLatestConfig(const std::string &rstrConfKey, 
                                     unsigned long long ullReceivedTS)
{
    std::string strLastTS = CLocalConfig::GetInstance()->Get(rstrConfKey);
    if (!strLastTS.empty())
    {
        unsigned long long ullLastStoredTS = 
               ic_utils::CIgniteStringUtils::StringToNumber<unsigned long long>(strLastTS);
        if (ullReceivedTS <= ullLastStoredTS)
        {
            // this is old config
            HCPLOG_C << rstrConfKey << "old value rcvd" << ullLastStoredTS 
                     << "~" << ullReceivedTS;
            return false;
        }
    }
    HCPLOG_T << "config for [" << rstrConfKey << "]:[" 
             << ullReceivedTS << "]is latest";
    return true;
}

void CEventProcessor::UpdateDeviceConfigTS(const std::string& rstrConfKey, 
                                           unsigned long long ullConfigTS)
{
    // this is for individual domain config check
    CLocalConfig::GetInstance()->Set(rstrConfKey, 
            ic_utils::CIgniteStringUtils::NumberToString<unsigned long long>(ullConfigTS));

    // this is for full device config sync
    std::string strTS = CLocalConfig::GetInstance()->Get("lastConfigTS");
    if (ic_utils::CIgniteStringUtils::StringToNumber<unsigned long long>(strTS) < ullConfigTS)
    {
        HCPLOG_T << "config for [" << rstrConfKey << "]:[" 
                 << ullConfigTS << "]is latest";
        CLocalConfig::GetInstance()->Set("lastConfigTS", 
            ic_utils::CIgniteStringUtils::NumberToString<unsigned long long>(ullConfigTS));
        return;
    }
    HCPLOG_C << rstrConfKey << "old value rcvd" << strTS << "~" << ullConfigTS;
}

ic_utils::Json::Value CEventProcessor::GetServiceJsonFromConfig(const std::string
                                                           &rstrServiceName)
{
    ic_utils::Json::Value jsonTopicArray = 
                 CIgniteConfig::GetInstance()->GetJsonValue("MQTT.servicesTopic");
    if (jsonTopicArray.isNull()) 
    {
        HCPLOG_D << "Services topics are not found";
        return ic_utils::Json::Value::nullRef;
    }
    if (!jsonTopicArray.isArray()) 
    {
        HCPLOG_E << "Invalid services topic";
        return ic_utils::Json::Value::nullRef;
    }

    for ( int nItr = 0 ; nItr < jsonTopicArray.size() ; nItr++ ) 
    {
        ic_utils::Json::Value jsonServiceObj = jsonTopicArray[nItr];
        std::string strObjServiceName = jsonServiceObj["serviceName"].asString();
        if (strObjServiceName == rstrServiceName) 
        {
            return jsonTopicArray[nItr];
        }
    }
    return ic_utils::Json::Value::nullRef;
}
}
