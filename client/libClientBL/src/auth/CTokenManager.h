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
* \file CTokenManager.h
*
* \brief This class handles device activation and token management
********************************************************************************
*/

#ifndef CTOKEN_MANAGER_H
#define CTOKEN_MANAGER_H

#include <string>
#include "CIgniteMutex.h"
#include "HttpErrorCodes.h"
#include "CIgniteHTTPConnector.h"

using ic_network::HttpErrorCode;

namespace ic_bl
{
/**
 * Enum of login error
 */
enum LoginError
{
    eLE_SUCCESS, ///< login success
    eLE_INVALID_CREDENTIAL, ///< invalid login credential
    eLE_INVALID_SCOPE, ///< invalid login scope
    eLE_UNKNOWN ///< unknown login
};

/**
 * Class CTokenManager handles device activation and token management
 */
class CTokenManager 
{
public:
    /**
     * Method to get Instance of CTokenManager
     * @param void
     * @return Pointer to an instance of CTokenManager
     */
    static CTokenManager* GetInstance();

    /**
     * Method to release the class instance
     * @param void
     * @return void
     */
    static void ReleaseInstance();

    /**
     * Method to get token
     * @param[in] reSessionErr The enum error code
     * @return token string
     */
    const std::string GetToken(HttpErrorCode &reSessionErr);

    /**
     * Method to clear token
     * @param void
     * @return void
     */
    void InvalidateToken();

    /**
     * Method to get token issue time
     * @param void
     * @return token issued time
     */
    time_t GetTokenIssueTime();

    /**
     * Method to check for device activation
     * @param void
     * @return true if device is activated, false otherwise
     */
    static bool IsActivated();

private:
    /**
     * Default no-argument constructor.
     */
    CTokenManager();

    /**
     * Destructor
     */
    virtual ~CTokenManager();
    
    /**
     * Method to generates the auth token
     * @param[in] reSessionErr The enum error code
     * @return login error
     */
    LoginError GetAuthToken(HttpErrorCode &reSessionErr);

    /**
     * Method to invalidate passcode
     * @param void
     * @return void
     */
    void InvalidatePassCode();

    /**
     * Method to trigger the device activation
     * @param void
     * @return HTTP error code
     */
    HttpErrorCode Activate();

    /**
     * Method to set token issue time
     * @param[in] lTime time
     * @return void
     */
    void SetTokenIssueTime(time_t lTime);

    /**
     * Overload method to set token issue time
     * @param[in] sTime time string
     * @return void
     */
    void SetTokenIssueTime(std::string sTime);

    /**
     * Method to check the activation is successfully or not
     * @param[in] rSessionErr The enum error code
     * @return true if activation is successful , false otherwise
     */
    bool CheckForActivation(HttpErrorCode &rSessionErr);

    /**
     * Method to update activation state
     * @param[in] reSessionErr The enum error code
     * @return void
     */
    void UpdateActivationState(HttpErrorCode &reSessionErr);

    /**
     * Method to process response payload received for activation API 
     * @param[in] rActivationResponse activation response received after calling
     *    activation API
     * @param[in] rstrQualifierID qualifier ID
     * @return eERR_OK: if processing activation response is success
     *   eERR_RESPONSE_DATA: if http response code is not equal to 200
     */
    HttpErrorCode ProcessActivationResponse(
                        const ic_network::CIgniteHTTPConnector::
                                CActivationResponse &rActivationResponse,
                        const std::string &rstrQualifierID);

    //! Mutex variable
    static ic_utils::CIgniteMutex m_tknMngrMutex;

    //! Member variable to stores activation url
    std::string m_strActivateUrl;
    
    //! Member variable to stores auth url
    std::string m_strAuthUrl;
    
    //! Member variable to stores token
    std::string m_strToken;
    
    //! Member variable to stores token issued on value
    unsigned long long m_llTokenIssuedOn;
    
    //! Member variable to stores activation time
    unsigned long long m_llActivationTime;
    
    //! Member variable to stores token time to live
    unsigned long long m_llTokenTTL;
    
    //! Member variable to stores token TTL margin percentage
    int m_nMarginPercent;
    
    //! Member variable to stores activation url
    time_t m_lTokenIssueTime;
};
} /* namespace ic_bl */
#endif /* CTOKEN_MANAGER_H */
