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
* \file CDataBaseFacade.h
*
* \brief This class is facade for Database in IgniteClient
*******************************************************************************
*/

#ifndef CDATA_BASE_FACADE_H
#define CDATA_BASE_FACADE_H

#include <sqlite3.h>
#include <string>
#include <vector>
#include "CContentValues.h"
#include "db/CDatabase.h"
#include "db/CDataBaseConst.h"

namespace ic_core 
{
/**
 * Enum of SQLite
 */
enum DbType 
{
    eSQLITE ///< SQLite flag
};

/**
 * Enum of DB errors
 */
enum DbErrors 
{
    eINVALID_OPERATION = -1 ///< Invalid operation
};

/**
 * This class is facade for Database in IgniteClient
 */
class CDataBaseFacade 
{
public:
    /**
     * Method to get instance of CDataBaseFacade class
     * @param void
     * @return instance of CDataBaseFacade class
     */
    static CDataBaseFacade* GetInstance();

    /**
     * Method to query data from the database based on input parameter
     * @param[in] strTable string containing table name
     * @param[in] rvecProjection vector containing data projection
     * @param[in] rstrSelection sql command to be executed
     * @param[in] rvecOrderBy vector containing order by details
     * @param[in] nLimit query data limit
     * @return instance of CCursor class
     */
    CCursor* Query(const std::string strTable,
                  const std::vector<std::string> &rvecProjection = 
                                                     std::vector<std::string>(),
                  const std::string &rstrSelection = "",
                  const std::vector<std::string> &rvecOrderBy = 
                                                     std::vector<std::string>(),
                  const int nLimit = 0);

    /**
     * Method to insert data in database based on input parameter
     * @param[in] strTable string containing table name
     * @param[in] pData data to be inserted
     * @return returns the inserted data's row id if the insertion is successful;
     * if not, -1 will be returned.
     */
    long Insert(const std::string strTable, CContentValues *pData);

    /**
     * Method to update data in database based on input parameter
     * @param[in] strTable string containing table name
     * @param[in] pData data to be updated
     * @param[in] rstrSelection sql command to be executed
     * @return true if data is successfully updated, false otherwise
     */
    bool Update(const std::string strTable, CContentValues *pData, 
                const std::string &rstrSelection = "");

    /**
     * Method to remove data from database based on input parameter
     * @param[in] strTable string containing table name
     * @param[in] rstrSelection sql command to be executed
     * @return true if data is successfully removed, false otherwise
     */
    bool Remove(const std::string strTable, 
                const std::string &rstrSelection = "");

    /**
     * Method to start transaction
     * @param void
     * @return true if transaction is successfully started, false otherwise
     */
    bool StartTransaction();

    /**
     * Method to end the transaction based on input parameter
     * @param[in] bSuccess transaction end status
     * @return 0 if transaction is successfully ended, non-zero otherwise
     */
    int EndTransaction(bool bSuccess);

    /**
     * Method to get the size of data
     * @param void
     * @return size of data
     */
    size_t GetSize();

    /**
     * Method to execute given DB command
     * @param[in] rstrCmd DB command
     * @return 0 if the given command is successfully executed, non-zero otherwise
     */
    int ExecuteCommand(std::string &rstrCmd);
    
    /**
     * Method to close database connection
     * @param void
     * @return 0 if connection is successfully closed, non-zero otherwise
     */
    int CloseConnection();

    /**
     * Method to clear tables from the database
     * @param void
     * @return true if tables are successfully cleared, false otherwise
     */
    bool ClearTables();
    
    /**
     * Method to reset the database by deleting the existing one and create the
     * tables again.
     * @param void
     * @return 0 if database is successfully resetted, non-zero otherwise
     */
    int ResetDatabase();

    /**
     * Method to remove the existing database.
     * @param void
     * @return 0 if database is successfully removed; non-zero sqlite error-code
     *  otherwise
     */
    int RemoveDatabase();

    /**
     * Method to get the row count of the table form the database based on input
     * parameter
     * @param[in] strTableName name of the table for which count needs to be taken
     * @return if successfully retrieved, it returns the number of rows in the 
     * given table; if any error, returns -1.
     */
    int GetRowCountOfTable(const std::string strTableName);

    /**
     * Method to get the component status related to diagnostics information 
     * from the database
     * @param void
     * @return component related diagnostics info string
     */
    std::string GetComponentStatus();

    /**
     * Method to vacuum the DB.
     * @param void
     * @return true if vacuum is successful; false otherwise.
     */
    bool VacuumDb();

private:
    /**
     * Default no-argument constructor.
     */
    CDataBaseFacade();

    //! Member variable to instance of CDatabase class
    CDatabase *m_pSQLiteDbInstance;
};
} /* namespace ic_core */
#endif
