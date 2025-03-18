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

#include "CIgniteDateTime.h"
#include "crypto/CRNG.h"
#include "CIgniteConfig.h"
#include "db/CLocalConfig.h"
#include "CIgniteStringUtils.h"
#include "CIgniteLog.h"
#include "CActivationBackoff.h"
#include "CTokenManager.h"
#include "CActivationState.h"
#include "CIgniteClient.h"

//! Macro for 'ONE_SEC_IN_MS' integer value 
#define ONE_SEC_IN_MS 1000

//! Macro for 'FIVE_SEC_IN_MS)' integer value 
#define FIVE_SEC_IN_MS (5 * ONE_SEC_IN_MS)

//! Macro for 'TWELVE_SEC_IN_MS)' integer value 
#define TWELVE_SEC_IN_MS (12 * ONE_SEC_IN_MS)

//! Macro for 'ONE_MIN_IN_MS' integer value 
#define ONE_MIN_IN_MS (60 * ONE_SEC_IN_MS)

//! Macro for 'FOUR_MIN_IN_MS' integer value 
#define FOUR_MIN_IN_MS (ONE_MIN_IN_MS * 4)

//! Macro for 'TWELVE_MIN_IN_MS' integer value 
#define TWELVE_MIN_IN_MS (ONE_MIN_IN_MS * 12)

//! Macro for 'FIFTEEN_MIN_IN_MS' integer value 
#define FIFTEEN_MIN_IN_MS (ONE_MIN_IN_MS * 15)

//! Macro for 'IGNITION_COUNT' string
#define DB_IGNITION_COUNT "IGNITION_COUNT"

//! Macro for 'CActivationBackoff' string
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CActivationBackoff"

namespace ic_bl
{
namespace 
{
    //! Constant key for 'MAX_INITIAL_ATTEMPTS' integer value
    const int MAX_INITIAL_ATTEMPTS = 24;

    //! Constant key for 'MAX_HIGH_FREQ_ATTEMPTS_OVER_IGN_THRESHOLD' int value
    const int MAX_HIGH_FREQ_ATTEMPTS_OVER_IGN_THRESHOLD = 10;

    //! Constant key for 'MIN_VALID_TIME' integer value
    const int MIN_VALID_TIME = FIVE_SEC_IN_MS;
    
    //! Constant key for 'HIGH_FREQ_ATTEMPTS' integer value
    const int HIGH_FREQ_ATTEMPTS = 10;
    
    //! Constant key for 'NORMAL_FREQ_ATTEMPTS' integer value
    const int NORMAL_FREQ_ATTEMPTS = 15;
    
    //! Constant key for 'IGNITION_THRESHOLD' integer value
    const int IGNITION_THRESHOLD = 50;
}

namespace 
{
    //! Global variable to stores ignition threshold integer value
    int g_nIgnitionThreshold = 0;
}

CActivationBackoff::CActivationBackoff()
{
    m_bIsEnable = ic_core::CIgniteConfig::GetInstance()->
                         GetBool("HCPAuth.ActivationBackOffConf.enable", false);

    if (m_bIsEnable) 
    {
        g_nIgnitionThreshold = ic_core::CIgniteConfig::GetInstance()->
                       GetInt("HCPAuth.ActivationBackOffConf.ignitionThreshold",
                       IGNITION_THRESHOLD);

        m_lIgnitionCount = ic_utils::CIgniteStringUtils::StringToNumber<int>(
                  ic_core::CLocalConfig::GetInstance()->Get(DB_IGNITION_COUNT));

        m_eLastErrorState = HttpErrorCode::eERR_NETWORK;
        m_pBackOffState = NULL;
        Init();
        HCPLOG_D << "Init Activation Backoff";
    }
    else
    {
        HCPLOG_W << "Backoff disabled";
    }
}

void CActivationBackoff::Init()
{
    m_bIncrementIgnitionCount = false;
    ResetState();
}

CActivationBackoff::~CActivationBackoff()
{
    if (m_pBackOffState)
    {
        delete m_pBackOffState;
    }
}

CActivationBackoff *CActivationBackoff::GetInstance()
{
    static CActivationBackoff backoff;
    return &backoff;
}

void CActivationBackoff::ReleaseInstance()
{
    HCPLOG_METHOD();
    //do required cleanup here
}

bool CActivationBackoff::Proceed()
{
    HCPLOG_METHOD();
    if (!IsEnable())
    {
        HCPLOG_W << "Backoff disabled: Proceed.";
        return true;
    }

    //Increment ignition count on non-network errors.
    if (HttpErrorCode::eERR_NETWORK != m_eLastErrorState 
        && HttpErrorCode::eERR_TIMEOUT != m_eLastErrorState)
    {
        IncrementIgnitionCount();
    }

    bool bProceed;
    {
        ic_utils::CScopeLock lock(m_StateMutex);
        bProceed = m_pBackOffState->CheckIfProceed();
    }

    if (bProceed)
    {
        HCPLOG_C << "Retry Time:" 
                 << ic_utils::CIgniteDateTime::GetMonotonicTimeMs();
        CheckCurrentState();
    }

    return bProceed;
}

void CActivationBackoff::CalculateNextRetry(const HttpErrorCode &reErrCode)
{
    HCPLOG_METHOD();
    if (!IsEnable())
    {
        return;
    }

    m_eLastErrorState = reErrCode;

    //Do not initiate backoff for network related errors.
    if (HttpErrorCode::eERR_NETWORK == reErrCode 
        || HttpErrorCode::eERR_TIMEOUT == reErrCode)
    {
        HCPLOG_E << "Network error";
        return;
    }

    ic_utils::CScopeLock lock(m_StateMutex);
    m_pBackOffState->ActivationFailed();
    HCPLOG_W << "Unsuccessful activation attempts in current state: " 
             << m_pBackOffState->m_nAttempts;
}

void CActivationBackoff::Reset()
{
    HCPLOG_METHOD();
    Init();

    m_lIgnitionCount = 0;
    ic_core::CLocalConfig::GetInstance()->Set(DB_IGNITION_COUNT,
           ic_utils::CIgniteStringUtils::NumberToString<int>(m_lIgnitionCount));
}

bool CActivationBackoff::IsEnable() const 
{
    return m_bIsEnable;
}

void CActivationBackoff::ProcessEvent(ic_core::CEventWrapper &rEvent)
{
    HCPLOG_METHOD() << "Event: " << rEvent.GetEventId();

    if (!IsEnable() || CTokenManager::IsActivated())
    {
        return;
    }

    std::string strEventId = rEvent.GetEventId();
    if ("IgnStatus" == strEventId)
    {
        ic_utils::CScopeLock lock(m_StateMutex);
        if ("run" == rEvent.GetString("state")) 
        {
            m_bIncrementIgnitionCount = true;
            m_pBackOffState->IgnitionStateChange(true);
        }
        else
        {
            m_pBackOffState->IgnitionStateChange(false);
        }
    }

    if ("DeviceRemoval" == strEventId)
    {
        ic_utils::CScopeLock lock(m_StateMutex);
        m_pBackOffState->DeviceStateChange(
                                      rEvent.GetString("status") == "attached");
    }

    CheckCurrentState();
}

void CActivationBackoff::IncrementIgnitionCount()
{
    if (!m_bIncrementIgnitionCount)
    {
        return;
    }

    m_lIgnitionCount++;
    ic_core::CLocalConfig::GetInstance()->Set(DB_IGNITION_COUNT,
           ic_utils::CIgniteStringUtils::NumberToString<int>(m_lIgnitionCount));
    m_bIncrementIgnitionCount = false;
    HCPLOG_C << "Ignition count:" << m_lIgnitionCount;
}

void CActivationBackoff::CheckCurrentState()
{
    ic_utils::CScopeLock lock(m_StateMutex);
    CBackOffStates *pBackoffStateObj = m_pBackOffState->TryStateChange();
    if (m_pBackOffState->m_bIsStateChanged)
    {
        delete m_pBackOffState;
        m_pBackOffState = pBackoffStateObj;
        HCPLOG_D << "State changed to:" << m_pBackOffState->m_eCurrBackoffState;
    }
}

void CActivationBackoff::ResetState()
{
    ic_utils::CScopeLock lock(m_StateMutex);
    if (m_pBackOffState)
    {
        //Delete old state object if exist.
        delete m_pBackOffState;
    }

    //Set state to initial.
    m_pBackOffState = new CInitialState(this);
}

/********************State Mgmt. abstract class*******************************/
CActivationBackoff::CBackOffStates::CBackOffStates(CActivationBackoff *pActBackoff)
{
    m_pActivBackoff = pActBackoff;
    m_bIsStateChanged = false;
    m_llStartTime = ic_utils::CIgniteDateTime::GetMonotonicTimeMs();
    m_llNextTime = m_llStartTime;
    m_lnFreq = 0;
    m_nMaxAttempts = 0;
    m_nAttempts = 0;
    m_nRetrySlotMax = 0;
    m_nRetrySlotMin = 0;
    m_bIsDeviceStateChanged = false;
    m_bIsIgnStateChanged = false;
    m_eCurrBackoffState = eINITIAL;
}

unsigned long long int CActivationBackoff::CBackOffStates::GetNextRetryTime()
{
    HCPLOG_C << "Delay:" << m_lnFreq;
    m_nRetrySlotMin = m_nRetrySlotMax;
    m_nRetrySlotMax += m_lnFreq;
    std::string strSeedKey = ic_core::CIgniteClient::GetProductImpl()->
                                 GetAttribute(ic_core::IProduct::eSerialNumber);
    int nRand = ic_core::CRNG::Get(strSeedKey, m_nRetrySlotMin, m_nRetrySlotMax);
    unsigned long long int llNext = m_llStartTime + nRand;
    unsigned long long int llCurrTime = 
                                ic_utils::CIgniteDateTime::GetMonotonicTimeMs();
    HCPLOG_C << "Random: " << nRand << ", " << llNext 
             << "-" << llCurrTime << "=" << llNext - llCurrTime << " ms.";
    return llNext;
}

void CActivationBackoff::CBackOffStates::ActivationFailed()
{
    m_nAttempts++;
    m_llNextTime = GetNextRetryTime();
}

bool CActivationBackoff::CBackOffStates::CheckIfProceed()
{
    unsigned long long llCurrMTime = 
                                ic_utils::CIgniteDateTime::GetMonotonicTimeMs();
    HCPLOG_I << "Current monoTimsMs:"<< llCurrMTime 
             << "; Next retry time:" << m_llNextTime;
    return llCurrMTime > m_llNextTime;
}

void CActivationBackoff::CBackOffStates::IgnitionStateChange(bool bIsOn)
{
    m_bIsIgnStateChanged = bIsOn;
}

void CActivationBackoff::CBackOffStates::DeviceStateChange(bool bIsConnected)
{
    m_bIsDeviceStateChanged = bIsConnected;
}

/********************Initial State handling class******************************/
CActivationBackoff::CInitialState::CInitialState(CActivationBackoff *pActBackoff)
                                 : CBackOffStates(pActBackoff)
{
    m_eCurrBackoffState = eINITIAL;
    m_nMaxAttempts = ic_core::CIgniteConfig::GetInstance()->
                         GetInt("HCPAuth.ActivationBackOffConf.initialAttempts",
                         MAX_INITIAL_ATTEMPTS);
    m_lnFreq = ic_core::CIgniteConfig::GetInstance()->
                             GetInt("HCPAuth.ActivationBackOffConf.initialFreq",
                             FIVE_SEC_IN_MS);
}

unsigned long long int CActivationBackoff::CInitialState::GetNextRetryTime()
{
    return m_llNextTime + m_lnFreq;
}

CActivationBackoff::CBackOffStates 
                             *CActivationBackoff::CInitialState::TryStateChange()
{
    if (m_bIsIgnStateChanged || m_bIsDeviceStateChanged)
    {
        m_bIsStateChanged = true;
        return new CInitialState(m_pActivBackoff);
    }
    if (m_pActivBackoff->m_lIgnitionCount > g_nIgnitionThreshold)
    {
        m_bIsStateChanged = true;
        return new COverIgnThresholdHighFreq(m_pActivBackoff);
    }
    if (m_nAttempts > m_nMaxAttempts -1)
    {
        m_bIsStateChanged = true;
        return new CHighFreqState(m_pActivBackoff);
    }

    return this;
}

/********************HighFreq State handling class*****************************/
CActivationBackoff::CHighFreqState::
     CHighFreqState(CActivationBackoff *pActBackoff) :CBackOffStates(pActBackoff)
{
    m_eCurrBackoffState = eHIGH_FREQ;
    CActivationState::GetInstance()->SetActivationState(false,
                                                 CActivationState::eSTATE_RESET);
    m_nMaxAttempts = ic_core::CIgniteConfig::GetInstance()->
                        GetInt("HCPAuth.ActivationBackOffConf.highFreqAttempts",
                        HIGH_FREQ_ATTEMPTS);
    m_lnFreq = ic_core::CIgniteConfig::GetInstance()->
                        GetInt("HCPAuth.ActivationBackOffConf.highFreqDuration",
                        ONE_MIN_IN_MS);
    m_nRetrySlotMax = MIN_VALID_TIME;
}

CActivationBackoff::CBackOffStates
                            *CActivationBackoff::CHighFreqState::TryStateChange()
{
    if (m_bIsIgnStateChanged || m_bIsDeviceStateChanged)
    {
        m_bIsStateChanged = true;
        return new CInitialState(m_pActivBackoff);
    }

    if (m_nAttempts > m_nMaxAttempts - 1)
    {
        m_bIsStateChanged = true;
        return new CNormalFreqState(m_pActivBackoff);
    }

    return this;
}

/********************NormalFreq State handling class***************************/
CActivationBackoff::CNormalFreqState::CNormalFreqState(
                   CActivationBackoff *pActBackoff) : CBackOffStates(pActBackoff)
{
    m_eCurrBackoffState = eNORMAL_FREQ;
    CActivationState::GetInstance()->SetActivationState(false,
                                                 CActivationState::eSTATE_RESET);
    m_nMaxAttempts = ic_core::CIgniteConfig::GetInstance()->
                      GetInt("HCPAuth.ActivationBackOffConf.normalFreqAttempts",
                      NORMAL_FREQ_ATTEMPTS);
    m_lnFreq = ic_core::CIgniteConfig::GetInstance()->
                      GetInt("HCPAuth.ActivationBackOffConf.normalFreqDuration",
                      FOUR_MIN_IN_MS);
}

CActivationBackoff::CBackOffStates 
                          *CActivationBackoff::CNormalFreqState::TryStateChange()
{
    if (m_bIsIgnStateChanged || m_bIsDeviceStateChanged)
    {
        m_bIsStateChanged = true;
        return new CInitialState(m_pActivBackoff);
    }

    if (m_nAttempts > m_nMaxAttempts - 1)
    {
        m_bIsStateChanged = true;
        return new CLowFreqState(m_pActivBackoff);
    }

    return this;
}

/********************LowFreq State handling class******************************/
CActivationBackoff::CLowFreqState::CLowFreqState(CActivationBackoff *pActBackoff)
                                                   :CBackOffStates(pActBackoff)
{
    m_eCurrBackoffState = eLOW_FREQ;
    CActivationState::GetInstance()->SetActivationState(false,
                                                 CActivationState::eSTATE_RESET);
    m_lnFreq = ic_core::CIgniteConfig::GetInstance()->
                         GetInt("HCPAuth.ActivationBackOffConf.lowFreqDuration",
                         TWELVE_MIN_IN_MS);
}

CActivationBackoff::CBackOffStates
                             *CActivationBackoff::CLowFreqState::TryStateChange()
{
    if (m_bIsIgnStateChanged || m_bIsDeviceStateChanged)
    {
        m_bIsStateChanged = true;
        return new CInitialState(m_pActivBackoff);
    }

    return this;
}

/**************OverIgnThresholdHighFreq State handling class*******************/

CActivationBackoff::COverIgnThresholdHighFreq::COverIgnThresholdHighFreq(
                   CActivationBackoff *pActBackoff) : CBackOffStates(pActBackoff)
{
    m_eCurrBackoffState = eOVER_IGN_THRESHOLD_HIGH_FREQ;
    CActivationState::GetInstance()->SetActivationState(false,
                                                 CActivationState::eSTATE_RESET);
    m_lnFreq = ic_core::CIgniteConfig::GetInstance()->
          GetInt("HCPAuth.ActivationBackOffConf.highFreqAfterIgnitionThreshold",
          TWELVE_SEC_IN_MS);
    m_nMaxAttempts = ic_core::CIgniteConfig::GetInstance()->GetInt(
          "HCPAuth.ActivationBackOffConf.highFreqAttemptAfterIgnitionThreshold",
           MAX_HIGH_FREQ_ATTEMPTS_OVER_IGN_THRESHOLD);
}

CActivationBackoff::CBackOffStates 
                 *CActivationBackoff::COverIgnThresholdHighFreq::TryStateChange()
{
    if (m_bIsIgnStateChanged || m_bIsDeviceStateChanged)
    {
        m_bIsStateChanged = true;
        return new COverIgnThresholdHighFreq(m_pActivBackoff);
    }
    if (m_nAttempts > m_nMaxAttempts - 1)
    {
        m_bIsStateChanged = true;
        return new COverIgnThresholdLowFreq(m_pActivBackoff);
    }

    return this;
}

unsigned long long int 
                CActivationBackoff::COverIgnThresholdHighFreq::GetNextRetryTime()
{
    return m_llNextTime + m_lnFreq;
}

/*****************OverIgnThresholdLowFreq State handling class*****************/

CActivationBackoff::COverIgnThresholdLowFreq::COverIgnThresholdLowFreq(
                   CActivationBackoff *pActBackoff) : CBackOffStates(pActBackoff)
{
    m_eCurrBackoffState = eOVER_IGN_THRESHOLD_LOW_FREQ;
    CActivationState::GetInstance()->SetActivationState(false,
                                                 CActivationState::eSTATE_RESET);
    m_lnFreq = ic_core::CIgniteConfig::GetInstance()->
           GetInt("HCPAuth.ActivationBackOffConf.lowFreqAfterIgnitionThreshold",
           FIFTEEN_MIN_IN_MS);
}

CActivationBackoff::CBackOffStates 
                  *CActivationBackoff::COverIgnThresholdLowFreq::TryStateChange()
{
    if (m_bIsIgnStateChanged || m_bIsDeviceStateChanged)
    {
        m_bIsStateChanged = true;
        return new COverIgnThresholdHighFreq(m_pActivBackoff);
    }
    return this;
}
}/* namespace ic_bl */
