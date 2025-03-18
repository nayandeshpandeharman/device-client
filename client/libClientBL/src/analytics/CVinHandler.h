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
* \file CVinHandler.h
*
* \brief This class implements VIN related functionalities.
********************************************************************************
*/

#ifndef CVIN_HANDLER_H
#define CVIN_HANDLER_H

#include "jsoncpp/json.h"
#include "analytics/CEventProcessor.h"

namespace ic_bl
{
/**
 * Class CVinHandler implements VIN related functionalities.
 */
class CVinHandler : public ic_core::CEventProcessor
{
public:
    /**
     * Destructor
     */
    virtual ~CVinHandler();

    /**
     * Method to get Instance of CVinHandler
     * @param void
     * @return Pointer to Singleton Object of CVinHandler
     */
    static CVinHandler *GetInstance();

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
    void ProcessEvent(ic_core::CEventWrapper &rEvent) override;
    
    /**
     * Implements Method of CEventProcessor
     * @see CEventProcessor::ApplyConfig()
     */
    void ApplyConfig(ic_utils::Json::Value &rjsonConfigValue) override {};

private:
    /**
     * Default no-argument constructor.
     */
    CVinHandler();

    /**
     * Method to send topiced VIN event
     * @param[in] rstrVIN VIN VIN string
     * @param[in] rstrDeviceId device id
     * @return void
     */
    void SendTopicedVinEvent(const std::string &rstrVIN, 
                             const std::string &rstrDeviceId);

    /**
     * Method to handle VIN event
     * @param[in] rEvent VIN event data
     * @return void
     */
    void HandleVinEvent(ic_core::CEventWrapper &rEvent);

    /**
     * Method to handle Activation event
     * @param[in] rEvent Activation event data
     * @return void
     */
    void HandleActivationEvent(ic_core::CEventWrapper &rEvent);

    //! Member variable to stores topic prefix
    std::string m_strTopicprefix;
};
} /* namespace ic_bl */
#endif //CVIN_HANDLER_H