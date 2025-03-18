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
* \file CInsightEngine.h
*
* \brief This class starts CEventReceiver with Ignite message publisher
********************************************************************************
*/

#include "CMessageQueue.h"
#include "dam/CEventReceiver.h"
namespace ic_bl
{

/**
 * Class CInsightEngine starts CEventReceiver with publisher.
 * Also, this class does the data aggregation
 */
class CInsightEngine 
{
public:

    /**
     * Parameterized constructor
     * @param[in] pPublisher Pointer to the event publisher's object
     * @param[in] bDataAggregationStatus Data aggregation status
     */
    CInsightEngine(ic_core::CMessageQueue* pPublisher, bool
    		bDataAggregationStatus=true);

    /**
     * Destructor
     */ 
    virtual ~CInsightEngine();

    /**
     * Method to suspend Insight Engine 
     * @param void
     * @return void
     */
    void SuspendInsightEngine();

    /**
     * Method to resume suspended Insight Engine 
     * @param void
     * @return void
     */
    void ResumeInsightEngine();

    /**
     * Method to suspend data aggregation.
     * @param void
     * @return void
     */
    void SuspendDataAggregation();
    
    /**
     * Method to resume data aggregation.
     * @param void
     * @return void
     */
    void ResumeDataAggregation();

    #ifdef IC_UNIT_TEST
        friend class CInsightEngineTest;
    #endif

private:

    //! Member variable to hold Event receiver's object
    CEventReceiver* m_pEvtReceiver;

    //! Member variable to keep track of data aggregation status
    bool m_bIsDataAggregationActive;

    //! Details
    /*!
     * Member variable to keep track if InitialEvents thread is launched once
     * when dynamically suspend/resume InsightEngine, no need to run InitialEvents thread more than once.
     */
    bool m_bIsInitialEventsThreadLaunched;
};
}/* namespace ic_bl */
