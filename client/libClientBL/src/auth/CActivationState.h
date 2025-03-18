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
* \file CActivationState.h
*
* \brief This class maintains activation state
********************************************************************************
*/

#ifndef CACTIVATION_STATE_H
#define CACTIVATION_STATE_H

namespace ic_bl 
{
/**
 * Class CActivationState maintains activation state
 */
class CActivationState 
{
public:
    /**
     * Method to get Instance of CActivationState class
     * @param void
     * @return Pointer to Singleton Object of CActivationState class
     */
    static CActivationState *GetInstance();

    /**
     * Destructor
     */
    ~CActivationState();

    /**
     * Enum of activation failure error
     */
    enum ActivationFailureError
    {
        eNO_ERROR = -1, ///< no failure error
        eSTATE_RESET = 0, ///< state reset
        eNETWORK_ERROR, ///< network error
        ePAYLOAD_ERROR, ///< payload error
        eDEVICE_NOT_ASSOCIATED_ERROR, ///< device not associated error
        eBACKOFF_ERROR ///< backoff error
    };

    /**
     * Method to set activation state
     * @param[in] bState activation state
     * @param[in] eNotActivationReason activation failure reason
     * @return void
     */
    void SetActivationState(bool bState,
                            ActivationFailureError eNotActivationReason);

    /**
     * Method to get activation state
     * @param[out] rbState activation state
     * @param[out] rnReason activation failure reason
     * @return void
     */
    void GetActivationState(bool &rbState, int &rnReason);

    /**
     * Method to set token
     * @param[in] strToken token string
     * @return void
     */
    void SetToken(std::string strToken);

    /**
     * Method to get token value
     * @param void
     * @return token string
     */
    std::string GetTokenValue();

private:
    /**
     * Default no-argument constructor.
     */
    CActivationState();

    /**
     * Method to update changed activation state
     * @param void
     * @return void
     */
    void UpdateActivationStateChanged();

    /**
     * Method to update changed token
     * @param void
     * @return void
     */
    void UpdateTokenChanged();

    //! Member variable to stores activation state
    bool m_bActivationState;
    
    //! Member variable to stores activation failure reason
    ActivationFailureError m_eNotActivationReason;

    //! Member variable to stores token
    std::string m_strToken;
};
}/* namespace ic_bl*/
#endif //CACTIVATION_STATE_H