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
* \file CIgniteThread.h
*
* \brief CIgniteThread provides thread management utility funtions.
*******************************************************************************
*/

#ifndef CIGNITE_THREAD_H
#define CIGNITE_THREAD_H

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>

namespace ic_utils
{
/**
 * class CIgniteThread encapsulates basic thread management functionality.
 */
class CIgniteThread
{
public:
    /**
     * Default no-argument constructor.
     */
    CIgniteThread() : m_pthreadId(0), m_bIsRunning(false), m_bIsDetached(false)
    {
        // do nothing
    }

    /**
     * Destructor
     */
    virtual ~CIgniteThread();

    /**
     * Method to start the thread.
     * @param void
     * @return 0 on success, error code otherwise.
     */
    virtual int Start();

    /**
     * Pure virtual method, executes when the thread starts
     * @param void
     * @return void
     */
    virtual void Run() = 0;

    /**
     * Method to stop the running thread
     * @param void
     * @return 0 on success, error code otherwise.
     */
    virtual int Stop();

    /**
     * Method to exit the thread.
     * @param[in] pData Data to pass to the exit.
     * @return void
     */
    virtual void Exit(void *pData);

    /**
     * Method to wait for the thread to finish.
     * If the thread is not running or already detached, an error is logged.
     * @param void
     * @return 0 on success, error code otherwise.
     */
    virtual int Join();

    /**
     * Method to detach the running thread if not already detached.
     * Sets the flag and attribute when the thread starts if not running.
     * @param void
     * @return 0 on success, error code otherwise.
     */
    virtual int Detach();

    /**
     * Static method to set the priority of the current thread.
     * @param[in] nPrio Priority to set.
     * @return 0 on success, error code otherwise.
     */
    static int SetCurrentThreadPriority(const int nPrio);

    /**
     * Static method to set the name of the current thread.
     * @param[in] rstrName Name to set.
     * @return 0 on success, error code otherwise.
     */
    static int SetCurrentThreadName(const std::string &rstrName);

protected:
    /**
     * Static method, called when starting a thread.
     * @param[in] p Pointer to the CIgniteThread object.
     * @return NULL.
     */
    static void *StartThread(void *p);

    //! Identifier for the thread.
    pthread_t m_pthreadId;

    //! Boolean indicating whether the thread is currently running.
    bool m_bIsRunning;

    //! Boolean indicating whether the thread is detached.
    bool m_bIsDetached;
};

} /* namespace ic_utils */
#endif /* CIGNITE_THREAD_H */
