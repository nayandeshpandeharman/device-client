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

#include "net/CExponentialBackoff.h"
#include "crypto/CRNG.h"
#include <math.h>
#include "CIgniteClient.h"

using namespace std;

namespace ic_bl 
{

CExponentialBackoff::CExponentialBackoff(int nSuccessTime, int nFailureTime, 
    int nMaxRetryTime): m_nSuccessTime(nSuccessTime), 
    m_nFailureRetryTime(nFailureTime), m_nMaxRetryTime(nMaxRetryTime)
{
    m_nRetryCount = 0;
}

CExponentialBackoff::~CExponentialBackoff()
{

}


int CExponentialBackoff::GetNextRetryTime(HttpErrorCode eErrorCode)
{
    // In case of success just return the default success time
    if (eErrorCode == HttpErrorCode::eERR_OK)
    {
        // Reset the counter in case of success
        m_nRetryCount = 0;
        return m_nSuccessTime;
    }

    /* In case of server error or server time out calculate the
     * back off retry time
     */
    if ((eErrorCode == HttpErrorCode::eERR_SERVER) ||
            (eErrorCode == HttpErrorCode::eERR_TIMEOUT))
    {
        return CalculateBackOffRetryTime();
    }

    /* In every other case just return the failed retry time
     * reason being it is not a server connection failure
     * and hence just do the usual retry.
     */ 
    return m_nFailureRetryTime;
}

/*
 * If the m_nFailureRetryTime = 60
 * nNextRetryTime = 60 if m_nRetryCount == 0
 * nNextRetryTime = 120 if m_nRetryCount == 1
 * nNextRetryTime = 240 if m_nRetryCount == 2
 */
int CExponentialBackoff::CalculateBackOffRetryTime()
{
    int nNextRetryTime = (m_nFailureRetryTime * pow(2, m_nRetryCount));

    if (nNextRetryTime > m_nMaxRetryTime)
    {
        nNextRetryTime = m_nMaxRetryTime;
    }

    m_nRetryCount++;
    // Now return the random number between 0 and nNextRetryTime
    std::string strSeedKey = 
        ic_core::CIgniteClient::GetProductImpl()->GetAttribute(
                                            ic_core::IProduct::eSerialNumber);
    nNextRetryTime = ic_core::CRNG::Get(strSeedKey, nNextRetryTime);
    return nNextRetryTime;
}

}

