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
* \file CMidHandler.h
*
* \brief This class provides methods for MID related updation/operation to 
* database , to aid events upload over MQTT 
********************************************************************************
*/

#ifndef CMID_HANDLER_H
#define CMID_HANDLER_H

#include "CIgniteThread.h"
#include "CConcurrentQueue.h"
#include "CIgniteLog.h"
#include "CIgniteClient.h"
#include "CIgniteStringUtils.h"
#include <string.h>
#include <CIgniteMutex.h>
#include <set>
#include <map>
#include "IOnOffNotificationReceiver.h"
#include "db/CDataBaseFacade.h"

using ic_core::IOnOffNotificationReceiver;

namespace ic_bl
{

/**
 * This class provides methods for MID related updation/operation to 
 * database , to aid events upload over MQTT 
 */
class CMidHandler : public ic_utils::CIgniteThread ,  
                    public ic_core::IOnOffNotificationReceiver
{

public:
    /**
     * Method to get Instance of CMidHanlder
     * @param void
     * @return Pointer to an instance of CMidHandler
     */
    static CMidHandler* GetInstance(void);

    /**
     * Method to process the MID of published events , events marked against 
     * this mid will be cleared/updated in the database
     * @param[in] nMid Message Id
     * @return True on successful processing, false otherwise
     */
    bool ProcessPublishedMid(int nMid);

    /**
     * Method to update the the MID of published events in the table specified
     * @param[in] nMid Message Id
     * @param[in] rstrTable Table name in which the mid needs to be updated
     * @return True on success, false otherwise
     */
    bool SetMidTable(int nMid, const std::string& rstrTable);

    /**
     * Method to intitalize the CMidHandler and clearing the preexisting mids 
     * from table
     * @param void
     * @return True on success, false otherwise
     */
    bool InitMid();

    /**
     * Method to get the tableName for mid input
     * @param[in] nMid Message Id
     * @return Table name for the mid input
     */
    std::string GetTableOfPublishedMid(int nMid);

    /**
     * Overriding IOnOffNotificationReceiver::NotifyShutdown
     * @see ic_core::IOnOffNotificationReceiver::NotifyShutdown()
     */
    void NotifyShutdown() override;
    
    #ifdef IC_UNIT_TEST
        friend class CMidHandlerTest;
    #endif

private:
    /**
     * Default Constructor
     */
    CMidHandler();

    /**
     * Destructor
     */
    ~CMidHandler();

    /**
     * Overriding CIgniteThread::Run
     * @see ic_utils::CIgniteThread::Run()
     */
    virtual void Run();

    /**
     * Method to clear the MID entry in the specified table
     * @param[in] nMid Message Id to be cleared form table
     * @param[in] strTable Table name from which the MID needs to be cleared
     * @return True on success , fail otherwise
     */
    bool ClearMid(int nMid,std::string strTable);

    //! Member variable to indicate if shutdown is intitated
    bool m_bShutdownInitiated = false;

    /**
     * Structure for holding the information about the mid and coresponding
     * table toegether
     */
    struct MidTable
    {
        //! Message Id
        int m_nMid; 

        //! Table name
        std::string m_strTable;

        /**
         * Parameterized constructor to initialize the member variables
         */
        MidTable(int nMid, std::string strTable) : m_nMid(nMid),
                                                   m_strTable(strTable)
        {}
    };

    //! Member variable to hold the published MID values
    std::set<int> m_setPublishedMidSet;

    //! Concurrent Queue to hold the MIDs to be deleted
    ic_utils::CConcurrentQueue<MidTable*> m_queMidTobeDeleted;

    //! Map of MID to DB Table
    std::map<int, std::string> m_mapMidDBTableMap;

    //! Mutex for synchronizing map related operations
    ic_utils::CIgniteMutex m_MapMutex;

    //! Mutex for synchronizing concurrent queue related operations
    ic_utils::CIgniteMutex m_QueueMutex;

    //! Conditional variable for notification about queue operations
    ic_utils::CThreadCondition m_condQueueHandler;

};

} //namespace ic_bl

#endif // CMID_HANDLER_H
