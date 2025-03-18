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
* \file CSigThread.h
*
* \brief This class has definition of Interface Signal Communication between 
* threads
********************************************************************************
*/

#ifndef CSIG_THREAD_H
#define CSIG_THREAD_H

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include "CIgniteLog.h"

namespace ic_core 
{
/**
 * Class CSigThread provides interface to communicate between threads using 
 * signal
 */
class CSigThread
{
public:
    /**
     * Parameterized Constructor to initialize SigThread class with required 
     * members Parameters. Constructor also blocks the signal which could be 
     * received in parent thread.
     * @param[in] rnSignum Signal to be sent from sender thread to receiver 
     * thread
     */
    CSigThread(const int &rnSignum);

    /**
     * Method to sends a signal to receiver thread
     * @param[in] rTid Thread id of the receiver thread
     * @return True if signal sent successfully, false otherwise
     */
    bool Notify(const pthread_t &rTid);

    /**
     * Method to wait for signal. If Timeout is provided in while instantiating
     * object then it it waits only until timeout given
     * @param[in] bTimout Timeout value, default true
     * @return True if signal received successfully, false otherwise
     */
    bool Wait(bool bTimout = true);

private:
    /**
     * Member variable Holds the signal number value which will be used from 
     * sender thread to notify receiver thread. In order to instantiate object 
     * of CSigThread class user has to provide user level signal value.
     */
    int m_nSignum;

    //! Member variable indicate that receiver is waiting to receive the signal
    bool m_bWaiting;
};
} /* namespace ic_core */
#endif /* CSIG_THREAD_H */
