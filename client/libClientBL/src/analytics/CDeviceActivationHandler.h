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
* \file CDeviceActivationHandler.h
*
* \brief This class maintains activation state of the device 
********************************************************************************
*/

#ifndef CDEVICE_ACTIVATION_HANDLER_H
#define CDEVICE_ACTIVATION_HANDLER_H

#include <string>
#include "analytics/CEventProcessor.h"
#include "jsoncpp/json.h"

namespace ic_bl
{
/**
 * Class CDeviceActivationHandler maintains activation state of the device 
 */
class CDeviceActivationHandler : public ic_core::CEventProcessor 
{
public:
    /**
     * Destructor
     */
    ~CDeviceActivationHandler();

    /**
     * Implements Method of CEventProcessor
     * @see CEventProcessor::ApplyConfig()
     */
    void ApplyConfig(ic_utils::Json::Value &rjsonConfigValue) override {};

    /**
     * Implements Method of CEventProcessor
     * @see CEventProcessor::ProcessEvent()
     */
    void ProcessEvent(ic_core::CEventWrapper &rEvent) override;

    /**
     * Method to get Instance of CDeviceActivationHandler
     * @param void
     * @return Pointer to Singleton Object of CDeviceActivationHandler
     */
    static CDeviceActivationHandler *GetInstance();

    /**
     * Method to cleanup resources allocated by class
     * @param void
     * @return void
     */
    static void ReleaseInstance();

private:
    /**
     * Default no-argument constructor.
     */
    CDeviceActivationHandler();

    /**
     * Method to update activation status
     * @param void
     * @return void
     */
    void UpdateActivationStatusChanged();
};
}/* namespace ic_bl */
#endif //CDEVICE_ACTIVATION_HANDLER_H