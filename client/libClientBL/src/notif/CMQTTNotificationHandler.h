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

/*!
********************************************************************************
* \file CMQTTNotificationHandler.h
*
* \brief This class implements methods which would be used for handling the MQTT
* notification/commands coming from the mqtt host
********************************************************************************
*/

#ifndef CMQTT_NOTIFICATION_HANDLER_H
#define CMQTT_NOTIFICATION_HANDLER_H

#include "notif/CNotificationListener.h"
#include "string"
#include <map>
#include "jsoncpp/json.h"

namespace ic_bl 
{

/**
 * Class implements methods which would be used for handling the MQTT 
 * notification/commands coming from the mqtt host
 */
class CMQTTNotificationHandler : public ic_bl::IRequestHandler
{
public:
    /**
     * Parameterized Constructor
     * @param[in] pNotifListner Pointer to notification listener 
     * @return No return
     */
    CMQTTNotificationHandler(CNotificationListener *pNotifListner);

    /**
     * Destructor
     */
    virtual ~CMQTTNotificationHandler();

    /**
     * Overriding the IRequestHandler::Handle
     * @see IRequestHandler::Handle()
     */
    void Handle(const char* cstrRequestJson);

    #ifdef IC_UNIT_TEST
        friend class CMQTTNotificationHandlerTest;
    #endif

private:

    /**
     * Member variable for storing the type of req which will be handled
     * by this handler
     */
    static const std::string REQ_TYPE;

    /**
     * m_jsonTopicDomainMap - This json object has map of topic and its 
     * respective domain 
     */
    ic_utils::Json::Value m_jsonTopicDomainMap;

    /**
     * Method to get the domain corresponding to the topic
     * @param[in] rstrTopic Topic name
     * @return Domain name corresponding to topic
     */
    std::string GetDomainFromTopic(const std::string& rstrTopic);

    /**
     * Method to populates topic to domain map
     * @param[in] rjsonServiceObj Service Json which contains the details
     * @param[in] rstrServiceName Service name
     * @return void
     */
    void UpdateTopicDomainMap(ic_utils::Json::Value& rjsonServiceObj,
                              std::string& rstrServiceName);
};

}

#endif // CMQTT_NOTIFICATION_HANDLER_H
