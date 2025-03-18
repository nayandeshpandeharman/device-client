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
* \file CClientOnOff.h
*
* \brief This class handles the client shutdown usecase. It manages the 
* subscribers (subscribed for shutdown-notification) to have clean client 
* shutdown.
********************************************************************************
*/

#ifndef CCLIENT_ON_OFF_H
#define CCLIENT_ON_OFF_H

#include <map>
#include <list>
#include "CIgniteThread.h"
#include "CIgniteMutex.h"
#include "IOnOffNotificationReceiver.h"
#include "IOnOff.h"

namespace ic_bl
{
/**
 * Enum for shutdown notification acknowledgement
 */
typedef enum
{
    eSHUTDOWN_INTIATED  = 1, ///< shutdown initiated
    eREADY_FOR_SHUTDOWN  = 2 ///< Ready for shutdown
}ShutdownNotifAckEnum;

/**
 * class CClientOnOff provides methods that handles the client shutdown usecase.
 * It manages the subscribers (subscribed for shutdown-notification) to have 
 * clean client shutdown.
 */
class CClientOnOff : public ic_core::IOnOff, public ic_utils::CIgniteThread
{
public:
    /**
     * Method to get Instance of CClientOnOff
     * @param void
     * @return Pointer to Singleton Object of CClientOnOff
     */
    static CClientOnOff* GetInstance();

    /**
     * Overriding Method of ic_core::IOnOff class
     * @see ic_core::IOnOff::RegisterForShutdownNotification(
     *                                        IOnOffNotificationReceiver* pRcvr,
     *                                        NotifReceiverCode eRcvrCode,
     *                                        std::string strRcvrName="")
     */
    bool RegisterForShutdownNotification(ic_core::
                                         IOnOffNotificationReceiver* pRcvr,
                                         NotifReceiverCode eRcvrCode,
                                         std::string strRcvrName="");

    /**
     * Overriding Method of ic_core::IOnOff class
     * @see ic_core::IOnOff::UnregisterForShutdownNotification(
     *                                          NotifReceiverCode eRcvrCode,
     *                                          std::string strRcvrName="")
     */
    bool UnregisterForShutdownNotification(NotifReceiverCode eRcvrCode,
                                            std::string strRcvrName="");

    /**
     * Overriding Method of ic_core::IOnOff class
     * @see ic_core::IOnOff::ReadyForShutdown(
     *                                        NotifReceiverCode eRcvrCode,
     *                                        std::string strRcvrName="")
     */
    bool ReadyForShutdown(NotifReceiverCode eRcvrCode, 
                          std::string strRcvrName="");

    /**
     * Overridding ic_utils::CIgniteThread::Run() method
     * @see ic_utils::CIgniteThread::Run()
     */
    void Run();

    /**
     * Destructor.
     */
    virtual ~CClientOnOff();

    #ifdef IC_UNIT_TEST
        //! friend class for CClientOnOff 
        friend class CClientOnOffTest;
    #endif 

private:
    /**
     * Default no-argument constructor.
     */
    CClientOnOff();

    /**
     * Method to return the receiver name based on given receiver code
     * @param[in] eRcvrCode notification receiver's enum code
     * @return string value of eRcvrCode
     */
    std::string GetReceiverName(NotifReceiverCode eRcvrCode);

    /**
     * Enum for subscription status 
     */
    enum SubscribeStatus 
    {
        eS_SUBSCRIBED,         ///< subscriber is just subscribed
        eS_NOTIFIED,           ///< subscriber is notified for shutdown
        eS_SHUTDOWN_COMPLETED, ///< subscriber completed the shutdown process
        eS_UNSUBSCRIBED        ///< subscriber is unsubscribed
    };

    /**
     * Receiver detail structure stores status of each Notification subscriber
     */
    typedef struct 
    {
        ic_core::IOnOffNotificationReceiver* pRcvrRef;///< Stores subscriber obj
        SubscribeStatus eStatus; ///< Subscribed status of the subscriber
    }ReceiverDetail;

    /**
     * Method to notify non-defined receivers
     * @param void
     * @return void
     */
    void NotifyNonDefinedOnOffRcvrs();

    /**
     * Method to notify pre-defined receivers
     * @param void
     * @return void
     */
    void NotifyPreDefinedOnOffRcvrs();

    /**
     * Method to check the shutdown is completed for non-defined receivers
     * @param void
     * @return true if all non-defined receivers completed shutdown, 
     *          false otherwise
     */
    bool CheckStatusOfNonDefinedRcvrs();

    /**
     * Method to check the shutdown is completed for pre-defined receivers
     * @param void
     * @return true if all pre-defined receivers completed shutdown,
     *          false otherwise
     */
    bool CheckStatusOfPreDefinedRcvrs();

    //! Map to track the predefined subscriber list
    std::map<NotifReceiverCode, ReceiverDetail> m_mapReceivers;

    //! Map to track the non-predefined subscriber list
    std::map<std::string, ReceiverDetail> m_mapNonDefinedReceivers;

    //! Map to track the order to notify the subscribers
    std::map<int, std::list <NotifReceiverCode>> m_mapNotifOrder;

    //! Mutex variable
    ic_utils::CIgniteMutex m_onoffMutex;
};
}
#endif // CCLIENT_ON_OFF_H
