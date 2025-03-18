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
* \file CDBTransport.h
*
* \brief This class handles the incoming events and persist them in the database
* accordingly.
********************************************************************************
*/

#ifndef CDB_TRANSPORT_H
#define CDB_TRANSPORT_H

#include <iostream>
#include <fstream>
#include <set>
#include "dam/CTransportHandlerBase.h"
#include "CIgniteThread.h"
#include "CConcurrentQueue.h"
#include "db/CGranularityReductionHandler.h"
#include "IOnOffNotificationReceiver.h"

namespace ic_bl 
{

/**
 * Class CDBTransport handles the incoming events and persist them in the database accordingly.
 */
class CDBTransport : public ic_utils::CIgniteThread, public CTransportHandlerBase, public ic_core::IOnOffNotificationReceiver
{
public:

    /**
     * Parameterized constructor
     * @param[in] pNextHandler Pointer to the CTransportHandlerBase class
     */
    CDBTransport(CTransportHandlerBase* handler);

    /**
     * Destructor
     */
    virtual ~CDBTransport();

    /**
     * Overridding CIgniteThread::Run() method
     * @see CIgniteThread::Run()
     */
    virtual void Run();

    /**
     * This method to take out event payload from m_queEvent and send it for processing
     * @param void
     * @return void
     */
    void FlushCache();

    /**
     * Method to handle event to store in database
     * @param[in] pEvent Event data
     * @return void
     */
    virtual void HandleEvent(ic_core::CEventWrapper* pEvent);

    /**
     * Method to insert event into database
     * @param[in] rstrSerialized Serialized event string to be inserted in db
     * @return void
     */
    static void InsertEvent(const std::string& rstrSerialized);

    /**
     * Overriding Method of IOnOffNotificationReceiver class
     * @see IOnOffNotificationReceiver::NotifyShutdown()
     */
    void NotifyShutdown() override;

    /**
     * Method to insert non ignite event directly into database
     * @param[in] rstrSerialized Serialized event string to be inserted in db
     * @return void
     */
    void HandleNonIgniteEvent(ic_core::CEventWrapper* pEvent);
    #ifdef IC_UNIT_TEST
        friend class CDBTransportTest;

        /**
         * Method to insert the event(s) into the m_SetActivationExceptionEvents set
         * @param[in] rstrEventId EventId to be inserted
         * @return void
         */
        void InsertActivationEvents(const std::string& rstrEventId)
        {
            m_SetActivationExceptionEvents.insert(rstrEventId);
        }

        /**
         * Method to set the m_bUploadAfterActivation value true or false.
         * If device is not activated then check for m_bUploadAfterActivation value.
         * if m_bUploadAfterActivation false, event is not configured to be ignored hence set to false
         * @param[in] bVal value to be set to m_bUploadAfterActivation
         * @return void
         */
        void SetUploadAfterActivationFlag(bool bVal)
        {
            m_bUploadAfterActivation = bVal;
        }
    #endif

private:


    /**
     * Method to process the given event payload by persisting it in the database.
     * @param[in] rstrEvntPayload Serialized event payload
     * @return void
     */
    void ProcessMessage(const std::string& rstrEvntPayload);

    /**
     * Method to initialize ignored events count
     * @param void
     * @return void
     */
    void InitIgnoredEvents();

    /**
     * Method to trigger Db vacuum followed by DB purge operation provided the DB size is crossing the threshold.
     * @param[in] nDbSize size of db
     * @return void
     */
    void PurgeDB(const size_t nDbSize);

    /**
     * Method to check whether the events needs to be added in DB only after activation
     * @param[in] rstrEventId Event Id 
     * @return true if device Configured to wait till activation before sending event to Igntie platform, false otherwise
     */
    bool WaitForActivation(const std::string &rstrEventId);

    /**
     * Method to calculate maximum limit of events to be inserted in one transaction based on unQueSize and avgEventRecordSize
     * @param[in] unQueSize current queue size
     * @return maxLimit of Events to be inserted in one transaction (max=50)
     */
    uint16_t GetMaxEventToInsertInOneTxn(const unsigned int unQueSize);

    /**
     * Method to print CDBTransport event queue logs
     * @param void
     * @return void
     */
    void PrintEvntQueueLogs();

    /**
     * Method to process event queue data
     * @param[in] runQueSize event queue size
     * @return void
     */
    void ProcessQueueData(const unsigned int &runQueSize);

    /**
     * Method to process event for stream mode
     * @param[in] rEvent event data
     * @param[in] rbBatchModeSupported 'BatchModeSupported' flag
     * @param[out] rData data content object
     * @return void
     */
    static void ProcessEventForStreamMode(ic_core::CEventWrapper &rEvent,
                                          const bool &rbBatchModeSupported,
                                          ic_core::CContentValues &rData);

    /**
     * Method to get topiced event data
     * @param[in] pMode upload mode
     * @param[in] rEvent event data
     * @param[out] rData topiced event data
     * @return void
     */
    static void GetTopicedEventData(const ic_core::CUploadMode *pMode,
                                    ic_core::CEventWrapper &rEvent,
                                    ic_core::CContentValues &rData);

    /**
     * Method to send ignite start message
     * @param void
     * @return void
     */
    static void SendIgniteStartMessage();

    //! Member variable pointing to ofstream object
    std::ofstream* m_pStreamLog;

    //! Member variable to hold queue of event
    ic_utils::CConcurrentQueue<std::string> m_queEvent;

    //! Member variable to hold ignored event count
    int m_nIgnoredEventCnt;

    //! Member variable to hold max size of event queue
    unsigned long m_ulEventQueueMaxSize;

    //! Member variable to hold event insert window size
    unsigned long m_ulEventInsertWindowSize;

    //! Member variable to hold event insert window size
    unsigned long m_ulEventInsertThresholdSize;
    
    //! Member variable to hold ignored event size
    unsigned long m_ulIgnoredEventSize;

    //! Member variable to hold event size in queue
    unsigned long m_ulEventSizeInQueue;

    //! Member variable to hold start timestamp of ignoted event
    double m_dblEventIgnoreStartTs;

    //! Member variable to hold db size limit 
    size_t m_nDbSizeLimit;

    //! Member variable to hold pointer of Granularity Reduction Handler class
    CGranularityReductionHandler *m_pGranReducHndlr;

    //! Member variable flag indicated upload after activation enablement status   
    bool m_bUploadAfterActivation = false;

    //! Member variable to hold set of eventIds with activation exception for upload
    std::set<std::string> m_SetActivationExceptionEvents;

    //! Member variable flag to track the device shutdown status
    bool m_bIsShutdownInitiated;

    //! Member variable to store configurable average event record size in database
    uint16_t m_nDbEventStoreRecordAvgSize;

     //! Member variable to store configurable maximum limit for event to be inserted in one transaction*
    uint16_t m_nMaxEventInsertInOneTxn;
};
} /* namespace ic_bl*/
#endif /* CDB_TRANSPORT_H */
