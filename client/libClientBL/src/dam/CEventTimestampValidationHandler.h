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
* \file CEventTimestampValidationHandler.h
*
* \brief This class validates received event's timestamp. If timestamp is
* invalid, it fixes the timestamp and sends back to CCacheTransport to let it go
* through chain of responsibility.
********************************************************************************
*/

#ifndef CEVENT_TIMESTAMP_VALIDATION_HANDLER_H
#define CEVENT_TIMESTAMP_VALIDATION_HANDLER_H

#include <list>
#include <string>
#include <set>
#include "dam/CTransportHandlerBase.h"
#include "dam/CEventWrapper.h"

namespace ic_bl
{
/**
 * Class CEventTimestampValidationHandler receives event from CCacheTransport
 * and validates the timestamp. If timestamp is invalid, it fixes the timestamp
 * and sends back to CacheTransport.
 */
class CEventTimestampValidationHandler : public CTransportHandlerBase
{
public:
    /**
     * Parameterized constructor
     * @param[in] pNextHandler Pointer to the CTransportHandlerBase class
     */
    CEventTimestampValidationHandler(CTransportHandlerBase *pNextHandler);

    /**
     * Destructor
     */
    virtual ~CEventTimestampValidationHandler();

    /**
     * Method to handle event for validating timestamp
     * @param[in] pEvent Event data
     * @return void
     */
    virtual void HandleEvent(ic_core::CEventWrapper *pEvent);

    /* Method to check if event's timestamp is valid or not based on
     * cut off timestamp
     * @param[in] rdblTimestamp timestamp to be validated
     * @return true if timestamp is valid, false otherwise
     */
    bool IsEventTimestampValid(double &rdblTimestamp);

private:
    /* Method to fix the invalid timestamp of the event
    * @param[in] rstrEventID const eventId
    * @param[in] rdblEventTimestamp timestamp of the event
    * @param[in] pEvent Event object
    * @return void
    */
    void FixAndSend(const std::string &rstrEventID, 
                    const double &rdblEventTimestamp,
                    ic_core::CEventWrapper *pEvent);

    /* Method to validate the eventId and send event to next handler(ActiveDelay)
    * @param[in] rstrEventID const eventId
    * @param[in] pEvent Event object
    * @return void
    */
    void ValidateAndSendEvent(const std::string &rstrEventID,
                              ic_core::CEventWrapper *pEvent);

    /**
     * Method to load event list for timestamp customization from config
     * @param[in] rjsonTsExpEvents Events with timestamp validation exception from config
     * @return void
     */
    void LoadTSExceptionEvents(const ic_utils::Json::Value &rjsonTsExpEvents);

    /**
     * Method to check if event has timestamp in it
     * @param[in] pEvent event object
     * @return true if event has timestamp, false otherwise
     */
    bool IsEventHasValidTS(ic_core::CEventWrapper *pEvent);

    /**
     * Method to validate timestamp of the event
     * @param[in] pEvent event object
     * @return void
     */
    void ValidateTsOfEvent(ic_core::CEventWrapper* pEvent);

    /**
     * Method to validate timestamp of the event based on CutOff timestamp
     * @param[in] pEvent event object
     * @return void
     */
    void ValidateEventTSBasedOnCutOff(ic_core::CEventWrapper *pEvent);

    /**
     * Method to handle events in the initial event queue with invalid timestamp
     * i.e. before CutOff
     * @param void
     * @return void
     */
    void HandleInvalidEventTS();

    /**
     * Method to process first event with valid timestamp i.e. after CutOff
     * @param void
     * @return void
     */
    void HandleFirstValidTs();

    //! Member variable to store last invalid timestamp
    double m_dblLastInvalidTimestamp;

    //! Member variable to store first valid timestamp
    double m_dblFrstValidTimestamp;

    //! Member variable to store cutoff timestamp
    double m_dblTimestampCutoff;

    //! Member variable to store value of initial queue limit
    int m_nInitialEventQueueLimit;

    //! Member variable to store list of initial events
    std::list <ic_core::CEventWrapper*> m_listInitialEvents;

    //! Member variable to store value of flag of invalid events available in db
    bool m_bInvalidEventsAvailableInDb;

    //! Member variable to store value of set of exception eventIds
    std::set<std::string> m_setExceptionEvents;
};
}/* namespace ic_bl */
#endif /* CEVENT_TIMESTAMP_VALIDATION_HANDLER_H */