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
*******************************************************************************
* \file CRemoteService.h
*
* \brief Handles Remote Operations
*
*******************************************************************************
*/

#ifndef CREMOTE_SERVICE_H
#define CREMOTE_SERVICE_H

#include <string>
#include "jsoncpp/json.h"
#include "CIgniteMutex.h"
#include "analytics/CEventProcessor.h"

#define ACK_RCVD 9999

using namespace std;

namespace ic_auto
{
/**
 * CRemoteService implements remote Service functionality
 */
class CRemoteService : public ic_core::CEventProcessor
{
public:

    /**
     * Method to get Object of CRemoteService
     * @param void
     * @return Singleton Object of RemoteInhibitService
     */
    static CRemoteService *GetInstance();

    /**
     * Method to cleanup resources allocated by class
     * @param void
     * @return void
     */
    static void ReleaseInstance();

    /**
     * Implements Method of CEventProcessor
     * @see CEventProcessor::ProcessEvent()
     */
    void ProcessEvent(ic_core::CEventWrapper& event) override
    {
        /*Do nothing*/
    };

    /**
     * Implements Method of CEventProcessor
     * @see CEventProcessor::ApplyConfig()
     */
    void ApplyConfig(ic_utils::Json::Value& rjsonConfig) override;

    /**
     * Method to handle remote configuration
     * @param[in] rjsonConfig Reference Json object of configuration
     * received
     * @return true for Valid Command,False for Invalid Command
     */
    int HandleRemoteCommand(const ic_utils::Json::Value& rjsonConfig);

    #ifdef IC_UNIT_TEST
        friend class RemoteServiceTest;
    #endif

private:

    /**
     * Default Constructor
     */
    CRemoteService();

    /**
     * Destructor
     */
    ~CRemoteService();

    /**
     * Method to send Response to Remote Command
     * @param[in] rstrReqId Request Id of setting request
     * @param[in] rstrMsgId Message Id of setting request
     * @param[in] rstrBizId Biz Transaction Id of setting request
     * @return Integer zero for success and non-zero for failure
     * to send responseEvent
     */
    int SendResponse(string& rstrReqId, string& rstrMsgId, string& rstrBizId);

    /**
     * Mutex variable to Synchronize incoming Requests
     */
    ic_utils::CIgniteMutex m_RemoteMutex;

    /**
     * Variable to holds Topic String without deviceID
     */
    string m_strTopicWithoutId;

    /**
     * Variable to holds Service Topic Name
     */
    string m_strSerTopic;

    /**
     * Variable to holds Device Id
     */
    string m_strDeviceId;

    /**
     * Variable to hold QoS set for the MQTT Service Topic in configfile
     */
    int m_nQos;
};

} // namespace ic_auto

#endif //CREMOTE_SERVICE_H
