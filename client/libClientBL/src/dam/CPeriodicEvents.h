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
* \file: CPeriodicEvents.h
*
* \brief: This class is used to process periodic events configured in the
          configuration file.
*******************************************************************************
*/

#ifndef CPERIODIC_EVENTS_H
#define CPERIODIC_EVENTS_H

#include <unistd.h>
#include <string>
#include <vector>
#include <condition_variable>
#include <mutex>
#include "CIgniteThread.h"
#include "IOnOffNotificationReceiver.h"

namespace ic_bl
{
/**
 * Whenever Client is starting, there could be a need to report regarding
 * certain events periodically, such events are referred as periodic events.
 * This file takes care of such use-case.
 */
class CPeriodicEvents : public ic_utils::CIgniteThread, public 
                        ic_core::IOnOffNotificationReceiver 
{
public:
    /**
     * Parameterized constructor with three parameters to create Periodic 
     * Events object
     * @param[in] pvectReqIDs Vector of request Ids
     * @param[in] unPeriodInSecs Request period in seconds
     * @param[in] unPriority Request priority value
     */
    CPeriodicEvents(std::vector<std::string>* pvectReqIDs, unsigned int 
                    unPeriodInSecs, unsigned int unPriority);
    
    /**
     * Destructor
     */
    virtual ~CPeriodicEvents();

    /**
     * Overridding CIgniteThread::Run() method
     * @see CIgniteThread::Run()
     */
    void Run();

    /**
     * Method to stop the running thread
     * @param void
     * @return 0 on success, error code otherwise.
     */
    int Stop();

    /**
     * Method to check if periodic events related configurations are available
     * If available, this method will initiate processing of them
     * @param void
     * @return void
     */
    static void StartPeriodicEvents();

    /**
     * Method to stop processing the threads of periodic events
     * @param void
     * @return void
     */
    static void StopPeriodicEvents();

    /**
     * Overridding IOnOffNotificationReceiver::notifyShutdown method
     * @see IOnOffNotificationReceiver::NotifyShutdown()
     */
    void NotifyShutdown() override;

protected:
    //! Member variable to store period value in seconds
    unsigned int m_unPeriodSec;

    //! Member variable to store vector of request Ids
    std::vector<std::string>* m_pvectRequestIDs;

    //! Member variable to store the request priority value
    unsigned int m_unPriority;

    //! Member variable to store the stop thread value
    bool m_bStopThread;
private:
    //! Member variable to store vector of all the periodic event threads
    static std::vector<CPeriodicEvents*> m_pvectThreads;

    //! Member variable flag to track the device shutdown status
    bool m_bIsShutdownInitiated;

    //! Member variable to store the thread name generated dynamically
    std::string m_strThreadName;

    //! Condition variable
    std::condition_variable m_WaitCondition;

    //! Mutex variable
    std::mutex m_WaitMutex;

    /**
     * This function blocks the current executing thread by calling
     * wait_for() on condition variable.
     * The thread will be unblocked when "notify()" is executed or when
     * timeout occurs
     * @param[in] unTimeInMs Timeout value in miliseconds to block the thread
     * @return void
     */
    void Wait(unsigned int unTimeInMs);

    /**
     * This function unblocks all threads currently waiting on
     * condition variable
     * @param void
     * @return void
     */
    void Notify();

    /**
     * Method to process periodic events configuration based on input parameters
     * @param[in] rjsonPeriodicEvent periodic events config
     * @param[out] rnNoOfPeriodicEvnt number of periodic events
     * @return void
     */
    static void ProcessPeriodicEvents(const ic_utils::Json::Value 
                            &rjsonPeriodicEvent, int &rnNoOfPeriodicEvnt);
};
} /* namespace ic_bl */
#endif /* CPERIODIC_EVENTS_H */
