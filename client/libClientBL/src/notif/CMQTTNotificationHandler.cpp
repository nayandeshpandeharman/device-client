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

#include <regex>
#include "CMQTTNotificationHandler.h"
#include "CIgniteLog.h"
#include "jsoncpp/json.h"
#include "dam/CMessageController.h"
#include "CIgniteConfig.h"
#include "analytics/CEventProcessor.h"

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CMQTTNotificationHandler"

//! Constant key for '/ro' string
static const std::string TOPIC_RO                = "/ro";

//! Constant key for '/location' string
static const std::string TOPIC_LOCATION          = "/location";

namespace ic_bl 
{

//! Constant key for 'MQTT_CONFIG_REQUEST' string
const std::string CMQTTNotificationHandler::REQ_TYPE = "MQTT_CONFIG_REQUEST";

//! Constant key for 'serviceName' string
static const std::string SERVICE_NAME = "serviceName";

//! Constant key for 'MQTT.servicesTopic' string
static const std::string MQTT_SERVICE_TOPIC = "MQTT.servicesTopic";

CMQTTNotificationHandler::CMQTTNotificationHandler(
                                        CNotificationListener *pNotifListner)
{
    HCPLOG_METHOD();
    ic_utils::Json::Value jsonTopicArray = 
        ic_core::CIgniteConfig::GetInstance()->GetJsonValue(MQTT_SERVICE_TOPIC);
    if(jsonTopicArray.isNull()) 
    {
        HCPLOG_W << "Services topics are not found";
    }
    if(!jsonTopicArray.isArray()) 
    {
        HCPLOG_E << "Services topic format is not array";
    }
    for( int nItr = 0 ; nItr < jsonTopicArray.size() ; nItr++ ) 
    {
        ic_utils::Json::Value jsonServiceObj = jsonTopicArray[nItr];
        std::string strServiceName = jsonServiceObj[SERVICE_NAME].asString();
        UpdateTopicDomainMap(jsonServiceObj, strServiceName);
    }

    pNotifListner->RegisterRequest(this, REQ_TYPE);
}

CMQTTNotificationHandler::~CMQTTNotificationHandler()
{
}

// Function to get the domain from the topic
std::string CMQTTNotificationHandler::GetDomainFromTopic(
                                                const std::string &rstrTopic)
{
    HCPLOG_I << "TOPIC = " << rstrTopic;
    if (rstrTopic.find(TOPIC_RO) != std::string::npos)
    {
        return m_jsonTopicDomainMap[TOPIC_RO].asString();
    }
    else 
    {
        int nTopicStringPos = 0;
        std::string strSubTopic;
        nTopicStringPos = rstrTopic.find("2d", 0, 2);
        strSubTopic = rstrTopic.substr(nTopicStringPos + 2);
        if (m_jsonTopicDomainMap.isMember(strSubTopic))
        {
            HCPLOG_I << "Domain is " << 
                        m_jsonTopicDomainMap[strSubTopic].asString();
            return m_jsonTopicDomainMap[strSubTopic].asString();
        }
        else
        {
            // return empty
            HCPLOG_I << "Returning domain empty";
            return "";
        }
    }
}

void CMQTTNotificationHandler::Handle(const char* cstrRequestJson)
{
    HCPLOG_METHOD() << "RequestJson=" << cstrRequestJson;
    //parse the request
    ic_utils::Json::Reader jsonParser;
    ic_utils::Json::Value jsonReq;
    if (!jsonParser.parse(cstrRequestJson, jsonReq))
    {
        HCPLOG_E << "parse error - request :" << cstrRequestJson;
        return;
    }

    // validate the message
    ic_utils::Json::Value jsonMqReq;
    if (!jsonReq.isMember("message") || 
        (!jsonParser.parse(jsonReq["message"].asString(), jsonMqReq)) || 
        (!jsonMqReq.isObject() && !jsonMqReq.isArray()))
    {
        HCPLOG_E << "invalid message :" << jsonReq["message"].asString();
        return;
    }

    // call respective handlers to update localconfig
    std::string strDomain = "";
    if(jsonMqReq.isMember("domain")) 
    {
        strDomain = jsonMqReq["domain"].asString();
    }

    //std::string cmdVersion = jsonMqReq["version"].asString();

    if (strDomain.empty())
    {
        if(jsonReq.isMember("topic")) 
        {
            std::string strTopic = jsonReq["topic"].asString();
            strDomain = GetDomainFromTopic(strTopic);
        } 
        else 
        {
            HCPLOG_E << "NO TOPIC in request" << cstrRequestJson;
        }
    }

    HCPLOG_T << "domain : " << strDomain;

    /* The domain may be there and may not be there. For latest Ignite Spec
     * for Remoteperation the domain is not there.
     */
    if (!strDomain.empty())
    {
        //pass the notification to mqtt
        CMessageController::HandleNotification(strDomain, jsonMqReq);
    } 
    else 
    {
        HCPLOG_E << "Domain not found: The request will not be processed " 
                 << strDomain;
    }
}

void CMQTTNotificationHandler::UpdateTopicDomainMap(
                                        ic_utils::Json::Value& rjsonServiceObj, 
                                        std::string& rstrServiceName) 
{
    if (rjsonServiceObj.isMember("sub"))
    {
        std::string strSub = rjsonServiceObj["sub"].asString();
        if (strSub.find(TOPIC_RO) != std::string::npos)
        {
            m_jsonTopicDomainMap[TOPIC_RO] = rstrServiceName;
        }
        else if (!strSub.empty())
        {
            std::string  strSubTopic="";
            /* considering in config the sub key will always have value of 
             * topic starting from "2d"
             */
            strSubTopic = strSub.substr(2);

            if(!strSubTopic.empty())
            {
                m_jsonTopicDomainMap[strSubTopic] = rstrServiceName;
            }
        }
        else 
        {
            //do nothing
        }
    }
}

}

