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
* \file: CInitialEvents.h
*
* \brief: This class is used to process initial events configured in the
          configuration file.
*******************************************************************************
*/

#ifndef CINITIAL_EVENTS_H
#define CINITIAL_EVENTS_H

#include <mutex>
#include <vector>
#include <condition_variable>
#include "CIgniteThread.h"
#include "IOnOffNotificationReceiver.h"
#include "jsoncpp/json.h"

namespace ic_bl
{
/**
 * Whenever Client is starting, there could be a need to report certain events
 * to report one time at the startup, such events are referred as initialevents. 
 * This file takes care of such use-case.
 */
class CInitialEvents : public ic_utils::CIgniteThread, public 
                       ic_core::IOnOffNotificationReceiver 
{
public:
    /**
     * Method to check if initial events related configurations are available,
     * If available, this method will initiate processing of them
     * @param void
     * @return void
     */
    static void StartInitialEvents();

    /**
     * Parameterized constructor with three parameters to create Initial 
     * Events object
     * @param[in] pvectReqIDs Vector of request Ids
     * @param[in] unPeriodInSecs Request period in seconds
     * @param[in] unPriority Request priority value
     */
    CInitialEvents(std::vector<std::string>* pvectReqIDs, unsigned int 
                   unPeriodInSecs, unsigned int unPriority);

    /**
     * Destructor
     */
    virtual ~CInitialEvents();

    /**
     * Overridding CIgniteThread::Run() method
     * @see CIgniteThread::Run()
     */
    void Run();

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

private:
    //! Member variable to store the flag to track device shutdown status
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
     * Method to process initial events configuration based on input parameters
     * @param[in] rjsonInitialEvents Initial events config
     * @return void
     */
    static void ProcessInitialEventsConfig(const ic_utils::Json::Value 
                                           &rjsonInitialEvents);
};
} /* namespace ic_bl */
#endif /* CINITIAL_EVENTS_H */
