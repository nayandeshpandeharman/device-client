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
* \file CEventProcessor.h
*
* \brief This class provides the necessary interfaces to receive event/notifi
* from device platform/ cloud for further processing. 
*******************************************************************************
*/

#ifndef CEVENT_PROCESSOR_H
#define CEVENT_PROCESSOR_H

#include "string"
#include "jsoncpp/json.h"
#include "dam/CEventWrapper.h"

namespace ic_core 
{
//! Extern constant key for 'remoteConfig' string
extern const std::string DOMAIN_REMOTE_CONTROL;

//! Extern constant key for 'RemoteOperation' string
extern const std::string DOMAIN_RO_SERVICE;

//! Extern constant key for 'vinHandler' string
extern const std::string DOMAIN_VIN_HANDLER;

//! Extern constant key for 'disassociation' string
extern const std::string DOMAIN_DISASSOCIATION_REQUEST;

//! Extern constant key for 'activationBackOff' string
extern const std::string DOMAIN_ACTIVATION_BACKOFF;

//! Extern constant key for 'DeviceActivationHandler' string
extern const std::string DOMAIN_ACTIVATION_HANDLER;

//! Extern constant key for 'data' string
extern const std::string KEY_DATA;

//! Extern constant key for 'version' string
extern const std::string KEY_VERSION;

//! Extern constant key for 'command' string
extern const std::string KEY_CMD;

//! Extern constant key for 'timestamp' string
extern const std::string KEY_TS;

/**
 * CEventProcessor class expose interfaces for receiving various event and
 * notification from device and cloud platform for further processing
 */
class CEventProcessor 
{
public:
    /**
     * Virtual destructor
     */
    virtual ~CEventProcessor(){}

    /**
     * Method to process events received from device platform
     * @param[in] rEvent Event data
     * @return void
     */
    virtual void ProcessEvent(CEventWrapper &rEvent) = 0;

    /**
     * Method to accept the incoming config from cloud
     * @param[in] rjsonConfigValue Config value
     * @return void
     */
    virtual void ApplyConfig(ic_utils::Json::Value &rjsonConfigValue) = 0;

    /**
     * Method to reset data
     * @param void
     * @return void
     */
    virtual void Reset();

protected:
    /**
     * Method to check if the config is latest or not based on input parameter
     * @param[in] rstrConfKey Config value
     * @param[in] ullReceivedTS Received timestamp
     * @return true if config is latest, false otherwise
     */
    bool IsLatestConfig(const std::string &rstrConfKey, 
                        unsigned long long ullReceivedTS);

    /**
     * Method to update device config timestamp
     * @param[in] rstrConfKey Config key
     * @param[in] ullConfigTS Config timestamp
     * @return void
     */
    void UpdateDeviceConfigTS(const std::string &rstrConfKey, 
                              unsigned long long ullConfigTS);

    /**
     * Method to get service data from configuration based on input parameter
     * @param[in] rstrServiceName Service Name
     * @return Service data based on service name
     */
    ic_utils::Json::Value GetServiceJsonFromConfig(const std::string &rstrServiceName);
};
} /* namespace ic_core */
#endif // CEVENT_PROCESSOR_H
