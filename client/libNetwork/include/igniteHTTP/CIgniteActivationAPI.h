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
* \file CIgniteActivationAPI.h
*
* \brief This class provides the necessary interfaces to interact              *
*        with Ignite Cloud for Activation API                                  *
********************************************************************************
*/

#ifndef CIGNITE_ACTIVATION_API_H
#define CIGNITE_ACTIVATION_API_H

#include "CIgniteHTTPConnector.h"

namespace ic_network
{

/**
 * CIgniteAtivationAPI class exposing APIs necessary for device activation.
 */
class CIgniteActivationAPI
{
public:
    /**
     * Default no-argument constructor.
     */
    CIgniteActivationAPI();

    /**
     *  Destructor.
     */
    ~CIgniteActivationAPI();

    /**
     * Method to activate the device.
     * @param[in] rReq Activation request parameters
     * @see CIgniteHTTPConnector::CActivationRequest
     * @param[out] rResp Activation response from cloud
     * @see CIgniteHTTPConnector::CActivationResponse
     * @param[in] strUrl Activation url
     * @return void
     */
    void ActivateDevice(const CIgniteHTTPConnector::CActivationRequest &rReq,
                        CIgniteHTTPConnector::CActivationResponse &rResp,
                        const std::string strUrl);

private:

    /**
     * Method to build the activation payload.
     * @param[in] req Activation request parameters
     * @return  Activation request payload String
     */
    std::string BuildActivationJson(
        const CIgniteHTTPConnector::CActivationRequest req);

    /**
     * Method to parse activation API response data and update Device ID and
     * passcode.
     * @param[in] jsonData Json Data to be processed to get values.
     * @param[out] rResp Activation Response class containing the parsed values.
     * @return void
     */
    void UpdateDeviceIdAndPasscode(const ic_utils::Json::Value jsonData,
                            CIgniteHTTPConnector::CActivationResponse &rResp);

    /**
     * Method to parse activation V4 version API response data.
     * @param[in] jsonRoot activation API response in json format
     * @param[out] jsonData "data" section of the API
     * @return True if parsing is fine. False otherwise
     */
    bool ProcessV4APIResponse(const ic_utils::Json::Value jsonRoot,ic_utils::Json::Value &jsonData);
};

} // namespace
#endif // CIGNITE_ACTIVATION_API_H
