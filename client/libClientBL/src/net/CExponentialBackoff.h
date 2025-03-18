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
* \file CExponentialBackoff.h
*
* \brief This class provides methods which would be used for getting the 
* exponential back off time out. Which will be used for retry wait during 
* http calls.                                                                  
********************************************************************************
*/

#ifndef CEXPONENTIAL_BACKOFF_H
#define CEXPONENTIAL_BACKOFF_H

#include "HttpErrorCodes.h"
using ic_network::HttpErrorCode;

namespace ic_bl 
{

/**
 * Max value(Time in secs) for a back off retry time out 
 */
static const int MAX_BACKOFF_RETRY_TIMEOUT = 300; 

/**
 * Class implements methods which would be used for getting the exponential 
 * back off time out. Which will be used for retry wait during http calls.
 */
class CExponentialBackoff
{
public:
    /**
     * Parameterized Constructor
     * @param[in] nSuccessTime Time value to be returned in success case
     * @param[in] nFailureTime Time value to be returned in failure case
     * @param[in] nMaxRetryTime Max time value for a retry time
     * @return No return
     */
    CExponentialBackoff(int nSuccessTime, int nFailureTime, 
                        int nMaxRetryTime = MAX_BACKOFF_RETRY_TIMEOUT);

    /**
     * Destructor
     */
    ~CExponentialBackoff();

    /**
     * Method to get next retry time
     * @param[in] errorCode Http Error code based on which retry time will be
     * calculated
     * @return Next retry time
     */
    int GetNextRetryTime(HttpErrorCode eErrorCode);

    /**
     * Method to reset the retry count
     * @param void
     * @return void
     */
    void ResetCounter() 
    {
        m_nRetryCount = 0;
    }

    /**
     * Method to set time value for success case
     * @param[in] nSuccessTime Time in seconds
     * @return void
     */
    void SetSuccessTime(int nSuccessTime) 
    {
        m_nSuccessTime = nSuccessTime;
    }

    /**
     * Method to set time value for failure(other than server failures) case
     * @param[in] nFailureTime Time in seconds
     * @return void
     */
    void SetFailureTime(int nFailureTime) 
    {
        m_nFailureRetryTime = nFailureTime;
    }

    /**
     * Method to set Max time value for a retry time
     * @param[in] nMaxRetryTime Time in seconds
     * @return void
     */
    void SetMaxRetryTime(int nMaxRetryTime) 
    {
        m_nMaxRetryTime = nMaxRetryTime;
    }

    #ifdef IC_UNIT_TEST
        friend class CExponentialBackoffTest;
    #endif

private:

    //! Retry time value for success case
    int m_nSuccessTime;

    //! Retry time value for failure case
    int m_nFailureRetryTime;

    //! Max value for backoff retry time
    int m_nMaxRetryTime;

    //! Next back off retry time
    int m_nNextRetryTime;

    //! Retry counter
    int m_nRetryCount;

    /**
     * Method to calculate the random backoff retry time , based on the failure 
     * retry time and retry count.
     * @param void
     * @return Back Off retry time value
     */
    int CalculateBackOffRetryTime();
};

} // namespace ic_bl

#endif /* CEXPONENTIAL_BACKOFF_H */
