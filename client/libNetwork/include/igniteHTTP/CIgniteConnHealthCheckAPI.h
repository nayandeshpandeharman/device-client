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
* \file CIgniteConnHealthCheckAPI.h
*
* \brief This class provides the necessary interfaces to check                 *
*        http connection health                                                *
********************************************************************************
*/

#ifndef CIGNITE_HEALTH_CHECK_API_H
#define CIGNITE_HEALTH_CHECK_API_H

#include "CIgniteHTTPConnector.h"

namespace ic_network
{

/**
 * CIgniteConnHealthCheckAPI class exposing APIs necessary for http connection
 * Healthcheck.
 */
class CIgniteConnHealthCheckAPI 
{
public:
    /**
     * Default no-argument constructor.
     */
    CIgniteConnHealthCheckAPI();

    /**
     *  Destructor.
     */
    ~CIgniteConnHealthCheckAPI();

    /**
     * Method to to build the activation payload.
     * @param[out] resp Health check response from cloud
     * @see CIgniteHTTPConnector::CIgniteConnHealthCheckResponse
     * @param[in] url health check API url
     * @return void
     */
    void CheckConnectionHealthStatus(
         CIgniteHTTPConnector::CIgniteConnHealthCheckResponse &rResp,
         const std::string &rstrUrl);

};

} //namespace
#endif // CIGNITE_HEALTH_CHECK_API_H