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
* \file CActivationBackoff.h
*
* \brief This class implements activation backoff related functionalities.
********************************************************************************
*/

#ifndef CACTIVATION_BACKOFF_H
#define CACTIVATION_BACKOFF_H

#include "dam/CEventWrapper.h"
#include "analytics/CEventProcessor.h"
#include "HttpErrorCodes.h"

using ic_network::HttpErrorCode;

namespace ic_bl
{
/**
 * Class CActivationBackoff implements activation backoff functionalities.
 */
class CActivationBackoff : public ic_core::CEventProcessor
{
private:
    /**
     * Enum of activation backoff state
     */
    enum BackoffState
    {
        eINITIAL, ///< initial backoff state
        eHIGH_FREQ, ///< high frequency backoff state
        eNORMAL_FREQ, ///< normal frequency backoff state
        eLOW_FREQ, ///< low frequency backoff state
        eOVER_IGN_THRESHOLD_HIGH_FREQ, ///< over ign threshold high freq state
        eOVER_IGN_THRESHOLD_LOW_FREQ ///< over ign threshold loq freq state
    };

    /**
     * Default no-argument constructor.
     */
    CActivationBackoff();

    /**
     * Destructor
     */
    ~CActivationBackoff();
    
    /**
     * Method to initialize activation backoff
     * @param void
     * @return void
     */
    void Init();

    /**
     * Method to increment ignition count
     * @param void
     * @return void
     */
    void IncrementIgnitionCount();

    /**
     * Method to reset backoff state
     * @param void
     * @return void
     */
    void ResetState();

    //! Member variable to stores feature enablement status
    bool m_bIsEnable;

    //! Member variable to stores ignition count value
    long m_lIgnitionCount;

    //! Member variable to stores increment ignition count value
    bool m_bIncrementIgnitionCount;

    //! Member variable to stores HTTP error code
    HttpErrorCode m_eLastErrorState;

    /**
     * Class CBackOffStates implements backoff state functionalities
     */
    class CBackOffStates
    {
    public:
        /**
         * Parameterized Constructor
         * @param[in] pActBackoff pointer instance of CActivationBackoff class
         * @return No return
         */
        CBackOffStates(CActivationBackoff *pActBackoff);

        /**
         * Destructor
         */
        virtual ~CBackOffStates(){};

        /**
         * Method to update ignition state change status based on input 
         * parameter
         * @param[in] bIsOn ign status
         * @return void
         */
        virtual void IgnitionStateChange(bool bIsOn);

        /**
         * Method to update device state change status based on input parameter
         * @param[in] bIsConnected connected status
         * @return void
         */
        virtual void DeviceStateChange(bool bIsConnected);

        /**
         * Method to get time until next API call
         * @param void
         * @return next retry time
         */
        virtual unsigned long long int GetNextRetryTime();

        /**
         * Method to check if API call is allowed
         * @param void
         * @return true if API call is allowed, false otherwise
         */
        virtual bool CheckIfProceed();
        
        /**
         * Method to increment unsuccessful activation attempt count and 
         * calculate backoff time.
         * @param void
         * @return void
         */
        virtual void ActivationFailed();

        /**
         * Method to Check if current state expired
         * @param void
         * @return backoff state object
         */
        virtual CBackOffStates *TryStateChange() = 0;

        //! Member variable to stores instance of CActivationBackoff class
        CActivationBackoff *m_pActivBackoff;
        
        //! Member variable to stores current backoff state
        BackoffState m_eCurrBackoffState;

        //! Member variable to stores backoff state changed status
        bool m_bIsStateChanged;

        //! Member variable to stores start time when state was changed
        unsigned long long int m_llStartTime;

        //! Member variable to stores random time until next API call
        unsigned long long int m_llNextTime;
        
        //! Member variable to stores ignition state changed status
        bool m_bIsIgnStateChanged;

        //! Member variable to stores device state changed status
        bool m_bIsDeviceStateChanged;

        //! Member variable to stores fre of max time diff until next API call
        long int m_lnFreq;

        //! Member variable to stores max no of attempts for current state.
        int m_nMaxAttempts;
        
        //! Member variable to stores attempts counter
        int m_nAttempts;

        //! Member variable to stores min time-slot for random no generator
        int m_nRetrySlotMin;

        //! Member variable to stores min time-slot for random no generator
        int m_nRetrySlotMax;
    };

    //! Member variable to stores pointer instance of CBackOffStates class
    CBackOffStates *m_pBackOffState;

    //! Mutex variable
    ic_utils::CIgniteMutex m_StateMutex;

    /**
     * Class CInitialState implements backoff initial state functionalities
     * If ignition count less than 50, 1st 2 minutes we make calls every 5 secs
     */
    class CInitialState : public CBackOffStates
    {
    public:
        /**
         * Parameterized Constructor
         * @param[in] pActBackoff pointer instance of CActivationBackoff class
         * @return No return
         */
        explicit CInitialState(CActivationBackoff *pActBackoff);

        /**
         * Destructor
         */
        ~CInitialState(){};

        /**
         * Overriding Method of CBackOffStates class
         * @see CBackOffStates::GetNextRetryTime()
         */
        unsigned long long int GetNextRetryTime() override;

        /**
         * Overriding Method of CBackOffStates class
         * @see CBackOffStates::TryStateChange()
         */
        CBackOffStates *TryStateChange() override;
    };

    /**
     * Class CHighFreqState implements backoff high freq state functionalities
     * If ignition count less than 50, Retry for 10 times(add randomization) 
     * in next 10 mins.
     */
    class CHighFreqState : public CBackOffStates
    {
    public:
        /**
         * Parameterized Constructor
         * @param[in] pActBackoff pointer instance of CActivationBackoff class
         * @return No return
         */
        explicit CHighFreqState(CActivationBackoff *pActBackoff);

        /**
         * Destructor
         */
        ~CHighFreqState(){};

        /**
         * Overriding Method of CBackOffStates class
         * @see CBackOffStates::TryStateChange()
         */
        CBackOffStates *TryStateChange() override;
    };

    /**
     * Class CNormalFreqState implements backoff normal freq state functionalities
     * If ignition count less than 50, Retry for 5 times(add randomization) 
     * in next 20 mins.
     */
    class CNormalFreqState : public CBackOffStates
    {
    public:
        /**
         * Parameterized Constructor
         * @param[in] pActBackoff pointer instance of CActivationBackoff class
         * @return No return
         */
        explicit CNormalFreqState(CActivationBackoff *pActBackoff);
        
        /**
         * Destructor
         */
        ~CNormalFreqState(){};

        /**
         * Overriding Method of CBackOffStates class
         * @see CBackOffStates::TryStateChange()
         */
        CBackOffStates *TryStateChange() override;
    };

    /**
     * Class CLowFreqState implements backoff low freq state functionalities
     * If ignition count less than 50, Retry for 5 times every 1 hour.
     */
    class CLowFreqState : public CBackOffStates
    {
    public:
        /**
         * Parameterized Constructor
         * @param[in] pActBackoff pointer instance of CActivationBackoff class
         * @return No return
         */
        explicit CLowFreqState(CActivationBackoff *pActBackoff);

        /**
         * Destructor
         */
        ~CLowFreqState(){};

        /**
         * Overriding Method of CBackOffStates class
         * @see CBackOffStates::TryStateChange()
         */
        CBackOffStates *TryStateChange() override;
    };

    /**
     * Class COverIgnThresholdHighFreq implements over ignition threshold high
     * frequency related functionalities
     * If ignition count greater than 50, Retry period is 5 retries per minute
     * for first 2 mins.
     */
    class COverIgnThresholdHighFreq : public CBackOffStates
    {
    public:
        /**
         * Parameterized Constructor
         * @param[in] pActBackoff pointer instance of CActivationBackoff class
         * @return No return
         */
        explicit COverIgnThresholdHighFreq(CActivationBackoff *pActBackoff);

        /**
         * Destructor
         */
        ~COverIgnThresholdHighFreq(){};

        /**
         * Overriding Method of CBackOffStates class
         * @see CBackOffStates::GetNextRetryTime()
         */
        unsigned long long int GetNextRetryTime() override;

        /**
         * Overriding Method of CBackOffStates class
         * @see CBackOffStates::TryStateChange()
         */
        CBackOffStates *TryStateChange() override;
    };

    /**
     * Class COverIgnThresholdLowFreq implements over ignition threshold low
     * frequency related functionalities
     * If ignition count greater than 50, Retry once every 15 min.
     */
    class COverIgnThresholdLowFreq : public CBackOffStates
    {
    public:
        /**
         * Parameterized Constructor
         * @param[in] pActBackoff pointer instance of CActivationBackoff class
         * @return No return
         */
        explicit COverIgnThresholdLowFreq(CActivationBackoff *pActBackoff);

        /**
         * Destructor
         */
        ~COverIgnThresholdLowFreq(){};

        /**
         * Overriding Method of CBackOffStates class
         * @see CBackOffStates::TryStateChange()
         */
        CBackOffStates *TryStateChange() override;
    };

public:
    /**
     * Method to get Instance of CActivationBackoff
     * @param void
     * @return Pointer to Singleton Object of CActivationBackoff
     */
    static CActivationBackoff *GetInstance();

    /**
     * Method to cleanup resources allocated by class
     * @param void
     * @return void
     */
    static void ReleaseInstance();

    /**
     * Method to check feature is enable or not
     * @param void
     * @return true if feature is enabled, false otherwise
     */
    bool IsEnable() const;

    /**
     * Method to proceed activation backoff process
     * @param void
     * @return true if process proceed, false otherwise
     */
    bool Proceed();

    /**
     * Method to calculate next retry value
     * @param[in] reErrCode HTTP error code
     * @return void
     */
    void CalculateNextRetry(const HttpErrorCode &reErrCode);

    /**
     * Method to check current backoff state
     * @param void
     * @return void
     */
    void CheckCurrentState();

    /**
     * Method to reset activation backoff state
     * @param void
     * @return void
     */
    void Reset();

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
};
}/* namespace ic_bl */
#endif //CACTIVATION_BACKOFF_H
