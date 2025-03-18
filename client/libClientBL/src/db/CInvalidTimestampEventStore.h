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
* \file CInvalidTimestampEventStore.h
*
* \brief This class implements event store invalid timestamp validation related
* functionalities.
********************************************************************************
*/

#include <string>
#include <list>
#include "CIgniteEvent.h"
#include "dam/CEventWrapper.h"

#ifndef CINVALID_TIMESTAMP_EVENT_STORE_H
#define CINVALID_TIMESTAMP_EVENT_STORE_H

namespace ic_bl
{
/**
 * Class CInvalidTimestampEventStore implements event store invalid timestamp
 * validation related functionalities.
 */
class CInvalidTimestampEventStore
{
public:
    /**
     * Method to get Instance of CInvalidTimestampEventStores
     * @param void
     * @return Pointer to Singleton Object of CInvalidTimestampEventStore
     */
    static CInvalidTimestampEventStore* GetInstance();

    /**
     * Method to insert event
     * @param[out] rlistInitialEvents events list
     * @return true if the given events are successfully inserted, 
     * false otherwise
     */
    bool InsertEvents(std::list <ic_core::CEventWrapper*> &rlistInitialEvents);

    /**
     * Method to retrieve events based on input parameters
     * @param[in] nNumRowsRequested no of rows requested
     * @param[out] rlistRowIDs rowIds list
     * @param[out] rlistEventDataList event data list
     * @return void
     */
    void RetriveEvents(int nNumRowsRequested, std::list<int> &rlistRowIDs,
                       std::list<std::string> &rlistEventDataList);

    /**
     * Method to delete events
     * @param[out] rlistRowIds rowIds list
     * @return true if event is successfully deleted, false otherwise
     */
    bool DeleteEvents(std::list<int> &rlistRowIds);

    /**
     * Method to get event row count
     * @param void
     * @return rowCout
     */
    int GetEventRowCount();

#ifdef IC_UNIT_TEST
    //declared as a friend class
    friend class CInvalidTimestampEventStoreTest;

    /**
     * Method to get 'DBOverLimit' event list
     * @param void
     * @return 'DBOverLimit' event list
     */
    std::list<ic_event::CIgniteEvent*> GetDBOverLimitEventList()
    {
        return m_listDBOverLimitEventList;
    }
#endif

private:
    /**
     * Default no-argument constructor.
     */
	CInvalidTimestampEventStore();

    /**
     * Method to insert event into database
     * @param[in] rstrSerialized event data
     * @return returns the inserted data's row id if the insertion is successful;
     * if not, -1 will be returned.
     */
    long InsertIntoDb(const std::string &rstrSerialized);

    /**
     * Method to purge invalid event
     * @param[in] nEvSize event size
     * @return void
     */
	void PurgeInvalidEvents(int nEvSize);

    //! Member variable to stores database size limit
	size_t m_unDbSizeLimit;
    
    //! Member variable to stores database over limit for event list
	std::list<ic_event::CIgniteEvent*> m_listDBOverLimitEventList;
};
}/* namespace ic_bl */
#endif /* CINVALID_TIMESTAMP_EVENT_STORE_H */