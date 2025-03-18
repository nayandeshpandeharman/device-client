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
* \file CEventReceiver.h
*
* \brief This class handles received event and sends it to CCacheTransport for 
* further processing
********************************************************************************
*/

#ifndef CEVENT_RECEIVER_H
#define CEVENT_RECEIVER_H

#include "CIgniteThread.h"
#include "CMessageQueue.h"

namespace ic_bl
{
/**
 * Class CEventReceiver handles event injected to IC and hands over the received
 * events to the CCacheTransport for processing.
 */
class CEventReceiver : public ic_core::IMessageReceiver 
{
public:
    /**
     * Parameterized constructor
     * @param[in] pPublisher Pointer to the event publisher's object
     * @param[in] m_bReceiverSuspended Receiver's suspend status
     */
    CEventReceiver(ic_core::CMessageQueue *pPublisher,
                   bool bReceiverSuspended = false);

    /**
     * Destructor
     */ 
    virtual ~CEventReceiver();

    /**
     * Overriding ic_core::IMessageReceiver::Handle
     * @see ic_core::IMessageReceiver::Handle()
     */
    virtual bool Handle(const ic_event::CIgniteMessage &rMsg) override;

    /**
     * Method to suspend receiver 
     * @param void
     * @return void
     */
    void SuspendReceiver();

    /**
     * Method to resume suspended receiver 
     * @param void
     * @return void
     */
    void ResumeReceiver();

#ifdef IC_UNIT_TEST
    //declared as a friend class
    friend class CEventReceiverTest;
#endif

private:
    //! Member variable to hold publisher's object
    ic_core::CMessageQueue *m_pPublisher;
    
    //! Member variable to hold receiver's suspended status
    bool m_bReceiverSuspended;
};
}/* namespace ic_bl */
#endif /* CEVENT_RECEIVER_H */
