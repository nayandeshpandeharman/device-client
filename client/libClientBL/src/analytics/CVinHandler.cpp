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
#include "CVinHandler.h"
#include "db/CLocalConfig.h"
#include "CIgniteConfig.h"
#include "CIgniteClient.h"

//! Macro for 'not available' string
#define DUMMY_VIN "NA"

//! Macro for 'CVinHandler' string
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CVinHandler"

//! Constant key for 'EPIDDBChecksum' string
static const std::string PID_DB_CHECKSUM = "EPIDDBChecksum";

namespace ic_bl
{
CVinHandler* CVinHandler::GetInstance()
{
    static CVinHandler instance;
    return &instance;
}

void CVinHandler::ReleaseInstance()
{
    HCPLOG_METHOD();
    //do required cleanup here
}

CVinHandler::CVinHandler()
{
    ic_utils::Json::Value jsonMqRoot = 
                    ic_core::CIgniteConfig::GetInstance()->GetJsonValue("MQTT");
    m_strTopicprefix = jsonMqRoot["topicprefix"].asString();
}

CVinHandler::~CVinHandler()
{

}

void CVinHandler::ProcessEvent(ic_core::CEventWrapper &rEvent)
{
    std::string strEventId = rEvent.GetEventId();
    if ("VIN" == strEventId) 
    {
        HandleVinEvent(rEvent);
    }
    else if ("ActivationEvent" == strEventId)
    {
        HandleActivationEvent(rEvent);
    }
    else
    {
        HCPLOG_I << strEventId << " not supported.";
    }
}

void CVinHandler::HandleVinEvent(ic_core::CEventWrapper &rEvent)
{
    std::string strTopic = rEvent.GetString("topic");
    if (!strTopic.empty())
    {
        HCPLOG_I << "Ignore topicless event";
        return;
    }

    std::string strNewVin = rEvent.GetString("value");
    ic_utils::CIgniteLog::SetStatus("VIN", strNewVin);
    if (strNewVin.empty())
    {
        strNewVin = DUMMY_VIN;
    }

    std::string strDeviceId = ic_core::CLocalConfig::GetInstance()->
                                                            Get("lastDeviceId");
    if (!strDeviceId.empty()) 
    {
        // If device is already activated , then send same event with topic
        HCPLOG_C << "Device is already activated";
        SendTopicedVinEvent(strNewVin, strDeviceId);
    }
    else
    {
        HCPLOG_D << "Device is not activated yet";
    }

    ic_core::CLocalConfig::GetInstance()->Set("VIN", strNewVin);
    ic_core::CIgniteClient::GetProductImpl()->
                          SetAttribute(ic_core::IProduct::eVIN, strNewVin,
                          ic_core::IProduct::ProductAttributeStatus::eNO_ERROR);
}

void CVinHandler::HandleActivationEvent(ic_core::CEventWrapper &rEvent)
{
    std::string strVin = ic_core::CLocalConfig::GetInstance()->Get("VIN");
    if (!strVin.empty()) 
    {
        /* If device activates and found vin number to be sent then
         * form VIN event with topic and send
         */ 
        std::string strDeviceId = rEvent.GetString("id");
        SendTopicedVinEvent(strVin, strDeviceId);
    }
    else
    {
        HCPLOG_W << "VIN is not yet received";
    }
}

void CVinHandler::SendTopicedVinEvent(const std::string &rstrVIN,
                                     const std::string &rstrDeviceId)
{
    std::string strTopic = m_strTopicprefix  + rstrDeviceId + "/2c/dvp";
    ic_event::CIgniteEvent vinEvent("1.0","VIN");
    vinEvent.AddField("value", rstrVIN);
    bool bDummy(false);
    if(DUMMY_VIN == rstrVIN)
    {
        bDummy = true;
    }
    vinEvent.AddField("dummy", bDummy);
    vinEvent.AddField("topic", strTopic);
    vinEvent.Send();
}
}/* namespace ic_bl */
