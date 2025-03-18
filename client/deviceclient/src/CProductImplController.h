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
* \file ProductImplController.h
*
* \brief CProductImplController initiates and manages the porting layer
* components
*******************************************************************************
*/
#ifndef CPRODUCT_IMPL_CONTROLLER_H
#define CPRODUCT_IMPL_CONTROLLER_H

#include <mutex>
#include <condition_variable>
#include "CIgniteLog.h"
#include "CIgniteThread.h"
#include "CDefaultProductImpl.h"
#include "CDeviceCommandHandlerImpl.h"
#include "IOnOffNotificationReceiver.h"
#include "IClientMessageDispatcher.h"
#include "IClientConnector.h"

namespace ic_app
{

/**
 * class CProductImplController initiates and manages the porting layer
 * components
 */
class CProductImplController : public ic_utils::CIgniteThread,
                               public ic_core::IOnOffNotificationReceiver
{
public:
    /**
     * Singleton method to get the class instance
     * @param void
     * @return CProductImplController class instance
     */
    static CProductImplController *GetInstance();

    /**
     * Method to release the class instance
     * @param void
     * @return void
     */
    static void ReleaseInstance();

    /**
     * Virtual destructor
     */
    virtual ~CProductImplController();

    /**
     * Overridding CIgniteThread::Run() method
     * @see CIgniteThread::Run()
     */
    virtual void Run();

    /**
     * Overridding IOnOffNotificationReceiver::notifyShutdown method
     * @see IOnOffNotificationReceiver::NotifyShutdown()
     */
    void NotifyShutdown() override;

    /**
     * This function setup the product implementation code and starts the
     * Ignite Client.
     * @param[in] argc command line arg count
     * @param[in] argv command line arg vector
     * @return void
     */
    void StartController(int &iArgC, char *pchArgV[]);

    /**
     * This function returns client connector instance
     * Ignite Client.
     * @param void
     * @return IClientConnector instance
     */
    ic_core::IClientConnector *GetClientConnector();

#ifdef IC_UNIT_TEST
    friend class CProductImplControllerTest;

    /**
     * Method to handle the config file command-line argument.
     * @param[in] iIter current iterator position of the command-line arguments
     * @param[in] rnArgC count of command-line arguments
     * @param[in] pchArgV array of command-line arguments
     * @return true if config file is successfully read, false otherwise
     */
    bool HandleConfilefileArg(int &iIter, const int &rnArgC, char *pchArgV[]);

    /**
     * Method to handle the logging level command-line argument.
     * @param[in] iIter current iterator position of the command-line arguments
     * @param[in] rnArgC count of command-line arguments
     * @param[in] pchArgV array of command-line arguments
     * @return true if logging level is successfully read, false otherwise
     */
    bool HandleDebugLevelArg(int &iIter, const int &rnArgC, char *pchArgV[]);

    /**
     * Method to handle the command-line arguments for client restart details.
     * @param[in] iIter current iterator position of the command-line arguments
     * @param[in] rnArgC count of command-line arguments
     * @param[in] pchArgV array of command-line arguments
     * @return true if restart details are read successfully, false otherwise
     */
    bool HandleRestartCountArg(int &iIter, const int &rnArgC, char *pchArgV[]);


#endif

private:
    /**
     * This function deallocates product implementation resources
     * @param void
     * @return void
     */
    void ReleaseProdImplResoruces();

    /**
     * Default constructor
     */
    CProductImplController();

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

    //! Member variable to indicate shutdown is initiated
    bool m_bIsShutdownNotified;

    //! Member condition variable
    std::condition_variable m_WaitCondition;

    //! Member mutex variable
    std::mutex m_WaitMutex;

    //! Member pointer variable to CProduct
    ic_device::CDefaultProductImpl *m_pProd;

    //! Member pointer variable to IClientMessageDispatcher
    ic_core::IClientMessageDispatcher *m_pMsgDispatcher;

    //! Member pointer variable to IClientConnector
    ic_core::IClientConnector *m_pClientConnector;

    //! Member pointer variable to CDeviceCommandHandlerImpl
    ic_device::CDeviceCommandHandlerImpl *m_pDeviceCommandObject;
};
}
#endif /* CPRODUCT_IMPL_CONTROLLER_H */
