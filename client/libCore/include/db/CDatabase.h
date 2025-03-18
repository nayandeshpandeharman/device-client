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
* \file CDatabase.h
*
* \brief This class/module implements methods to manage the database file and 
* the data from sql query
*******************************************************************************
*/

#ifndef CDATABASE_H
#define CDATABASE_H

#include <map>
#include <sqlite3.h>
#include <string>
#include <vector>
#include <list>
#include "config/CUploadMode.h"
#include "CIgniteMutex.h"
#include "CContentValues.h"
#include "db/CDataBaseConst.h"

namespace ic_core 
{
/**
 * Structure of sql query
 */
struct SqlQuery
{
    std::string strTable; ///< table name
    std::vector<std::string> vecProjection; ///< vector of data projection
    std::string strSelection; ///< sql command
    std::vector<std::string> vecOrderBy; ///< vector of order by details
    int nLimit; ///< data limit
};

class CDatabase;

/**
 * This class provides methods to operate on data queried from database
 */
class CCursor 
{
public:
    /**
     * Method to move data position based on input parameter
     * @param[in] nPosition data position
     * @return true if data position successfully moved to given position, 
     * false otherwise
     */
    bool MoveToPosition(int nPosition);

    /**
     * Method to move data position to first position
     * @param void
     * @return true if position successfully moved to first, false otherwise
     */
    bool MoveToFirst();

    /**
     * Method to move data position to last position
     * @param void
     * @return true if position successfully moved to last, false otherwise
     */
    bool MoveToLast();

    /**
     * Method to move data position to next position
     * @param void
     * @return true if position successfully moved to next, false otherwise
     */
    bool MoveToNext();

    /**
     * Method to move data position to previous position
     * @param void
     * @return true if position successfully moved to previous, false otherwise
     */
    bool MoveToPrevious();

    /**
     * Method to get column names
     * @param void
     * @return vector of column names
     */
    std::vector<std::string> GetColumnNames();

    /**
     * Method to get column index based on input parameter
     * @param[in] strColName column name 
     * @return column index as integer
     */
    int GetColumnIndex(std::string strColName);

    /**
     * Method to read the value of the given column based on it's index
     * @param[in] nColIndex column index
     * @return returns column value as integer; if any error, returns default 
     * integer value i.e. 0
     */
    int GetInt(int nColIndex);

    /**
     * Method to read the value of the given column based on it's index
     * @param[in] nColIndex column index
     * @return returns column value as long; if any error, returns default 
     * double value i.e. 0l
     */
    long long GetLong(int nColIndex);

    /**
     * Method to read the value of the given column based on it's index
     * @param[in] nColIndex column index
     * @return returns column value as double; if any error, returns default 
     * double value i.e. 0.0f
     */
    double GetDouble(int nColIndex);

    /**
     * Method to read the value of the given column based on it's index
     * @param[in] nColIndex column index
     * @return returns column value as float; if any error, returns default 
     * float value i.e. 0.0f
     */
    float GetFloat(int nColIndex);

    /**
     * Method to read the value of the given column based on it's index
     * @param[in] nColIndex column index
     * @return returns column value as string; if any error, returns empty 
     * string
     */
    std::string GetString(int nColIndex);

    /**
     * Method to read the value of the given column based on it's index
     * @param[in] nColIndex column index
     * @return returns column value as boolean; if any error, returns false
     */
    bool GetBool(int nColIndex);

    /**
     * Method to read the count of how many rows that are returned as a result 
     * of the executed query
     * @param void
     * @return rows count
     */
    int Size();

    /**
     * Method to check if the no-of-records returned as a result of the executed 
     * query is empty or not
     * @param void
     * @return true if records returned from the query is empty, false otherwise 
     */
    bool Empty();

private:
    /**
     * This class implements list functionality
     */
    class CList : public std::list<CContentValues> 
    {
    public:
        /**
         * Method to get the list iterator based on given row index
         * @param[in] nRowIndex - row index
         * @return returns list iterator; if any error, returns the iterator
         * pointing to the first row (if any)
         */
        iterator GetIterator(int nRowIndex);

        /**
         * Method to overload operator []
         * @param[in] nIndex index value
         * @return content value
         */
        CContentValues operator [](int nIndex);
    };

    /**
     * Parameterized Constructor
     * @param[in] pDB Sql database
     * @param[in] pQuery Sql query
     */
    CCursor(sqlite3 *pDB, SqlQuery *pQuery);

    /**
     * Method to remove row from the records queried using sql query
     * @param void
     * @return void
     */
    void RemoveRow();

    /**
     * Method to generate sql statement based on given query
     * @param[in] pQuery Sql query
     * @return if successfully generated, it returns sql statement; 
     * if any error, returns empty string
     */
    std::string GenerateSqlStatement(SqlQuery *pQuery);

    /**
     * Method to run sql query based on input parameter
     * @param[in] pDB Sql database
     * @param[in] pQuery Sql query 
     * @return true if successfully run sql query, false otherwise
     */
    bool RunQuery(sqlite3 *pDB, SqlQuery *pQuery);

    //! Member variable to store vector of columns in data queried using sql query
    std::vector<std::string> m_vecColumns;
    
    //! Member variable to hold the query results
    CList m_Data;
    
    /*
     * Member variable to indicate the current position of the cursor within 
     * the query result.
     */ 
    int m_nPosition;

    /*
     * Declared CDatabase as a friend class to access private members of 
     * CCursor class
     */ 
    friend class CDatabase;
};

/**
 * This class provides methods to manage the database file in terms of
 * - managing database tables
 * - backup and recovery
 * - query transactions
 * - etc.
 */
class CDatabase 
{
public:
    /**
     * Method to get instance of CDatabase class
     * @param void
     * @return instance of CDatabase class
     */
    static CDatabase* GetInstance();

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
     * Method to get the size of the data in the database
     * @param void
     * @return returns the size of data in the database, if database is empty,
     *  -1 will be returned.
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
     * Method to vacuum the DB.
     * @param void
     * @return true if vacuum is successful; false otherwise.
     */
    bool VacuumDb();

private:
    /**
     * Default no-argument constructor.
     */
    CDatabase();

    /**
     * Method to open database
     * @param void
     * @return 0 if database is successfully opened; non-zero sqlite error-code
     * otherwise
     */
    int Open();

    /**
     * Method to check database integrity
     * @param void
     * @return true if database integrity is successfully passed; false 
     * otherwise
     */
    bool CheckIntegrity();

    /**
     * Method to recover database
     * @param void
     * @return true if database is successfully recovered, false otherwise
     */
    bool Recover();

    /**
     * Method to create database backup
     * @param void
     * @return void
     */
    void Backup();

    /**
     * Method to remove database
     * @param void
     * @return 0 if database is successfully removed; non-zero otherwise
     */
    int Remove();

    /**
     * Method to check database version
     * @param void
     * @return void
     */
    void CheckVersion();

    /**
     * Method to execute sql statements based on input parameter
     * @param[in] rstrSql SQL statements
     * @param[in] Callback callback function
     * @param[in] pData data
     * @return 0 if sql statement is successfully executed, non-zero sqlite 
     * error-code otherwise
     */
    int SqliteExec(std::string &rstrSql, 
                   int(*Callback)(void*, int, char**, char**), void *pData);
    
    /**
     * Method to get database version
     * @param void
     * @return 0 if sql statement is successfully executed, non-zero sqlite 
     * error-code otherwise
     */
    int GetVersion();

    /**
     * Method to set database version based on input parameter
     * @param[in] nVersion version details
     * @return void
     */
    void SetVersion(const int nVersion);

    /**
     * Method to create table in database
     * @param void
     * @return true if table is successfully created, false otherwise
     */
    bool OnCreate();

    /**
     * Method to upgrade database based on input parameter
     * @param[in] nOldVer old version
     * @param[in] nNewVer new version
     * @return true if table is successfully upgraded, false otherwise
     */
    bool OnUpgrade(int nOldVer, int nNewVer);

    /**
     * Method to downgrade database based on input parameter
     * @param[in] nOldVer old version
     * @param[in] nNewVer new version
     * @return true if table is successfully downgraded, false otherwise
     */
    bool OnDowngrade(int nOldVer, int nNewVer);

    /**
     * Method to create event store table
     * @param void
     * @return true if table is successfully created, false otherwise
     */
    bool CreateEventStoreTable();

    /**
     * Method to create invalid event store table
     * @param void
     * @return true if invalid table is successfully created, false otherwise
     */
    bool CreateInvalidEventStoreTable();

    /**
     * Method to create local config table
     * @param void
     * @return true if table is successfully created, false otherwise
     */
    bool CreateLocalConfigTable();

    /**
     * Method to create message sender store table
     * @param void
     * @return true if table is successfully created, false otherwise
     */
    bool CreateMessageSenderStoreTable();

    /**
     * Method to create upload file table
     * @param void
     * @return true if table is successfully created, false otherwise
     */
    bool CreateUploadFileTable();

    /**
     * Method to create ignite settings table
     * @param void
     * @return true if table is successfully created, false otherwise
     */
    bool CreateIgniteSettingsTable();

    /**
     * Method to create alert store table
     * @param void
     * @return true if table is successfully created, false otherwise
     */
    bool CreateAlertStoreTable();

    /**
     * Method to set page_size for sqlite.
     * @param void
     * @return void
     */
    void SetPagePragma();

    /**
     * Method to set temp_store_directory
     * @param void
     * @return void
     */
    void SetTempDirPragma();

    /**
     * Method to check and create table based on given error message
     * @param[in] strErrMsg string containing error message
     * @return void
     */
    void CheckAndCreateTable(const std::string strErrMsg);
    
    /**
     * Method to upgrade database version from 01 to 02 by making required 
     * adjustment to the schema
     * @param void
     * @return void
     */
    void UpgradeVersionFrom1To2();

    /**
     * Method to upgrade database version from 02 to 03 by making required 
     * adjustment to the schema
     * @param void
     * @return void
     */
    void UpgradeVersionFrom2To3();

    /**
     * Method to upgrade database version from 03 to 04 by making required 
     * adjustment to the schema
     * @param void
     * @return void
     */
    void UpgradeVersionFrom3To4();

    /**
     * Method to upgrade database version from 04 to 05 by making required 
     * adjustment to the schema
     * @param void
     * @return void
     */
    void UpgradeVersionFrom4To5();

    /**
     * Method to upgrade database version from 05 to 06 by making required 
     * adjustment to the schema
     * @param void
     * @return void
     */
    void UpgradeVersionFrom5To6();

    /**
     * Method to upgrade database version from 06 to 07 by making required 
     * adjustment to the schema
     * @param void
     * @return void
     */
    void UpgradeVersionFrom6To7();

    /**
     * Method to upgrade database version from 07 to 08 by making required 
     * adjustment to the schema
     * @param void
     * @return void
     */
    void UpgradeVersionFrom7To8();

    /**
     * Method to upgrade database version from 08 to 09 by making required 
     * adjustment to the schema
     * @param void
     * @return void
     */
    void UpgradeVersionFrom8To9();

    /**
     * Method to upgrade database version from 09 to 10 by making required 
     * adjustment to the schema
     * @param void
     * @return void
     */
    void UpgradeVersionFrom9To10();

    /**
     * Method to upgrade database version from 10 to 11 by making required 
     * adjustment to the schema
     * @param void
     * @return void
     */
    void UpgradeVersionFrom10To11();

    /**
     * Method to update events upload mode to default mode(as per the config) 
     * for the events which doesn't have modes assigned
     * @param[in] rstrAllConfiguredModesEventList event list
     * @return void
     */
    void UpdateEventsDefaultMode(const std::string 
                                 &rstrAllConfiguredModesEventList);

    /**
     * Method to upgrade database version from 11 to 12 by making required 
     * adjustment to the schema
     * @param void
     * @return void
     */
    void UpgradeVersionFrom11To12();

    /**
     * Method to upgrade database version from 12 to 13 by making required 
     * adjustment to the schema
     * @param void
     * @return void
     */
    void UpgradeVersionFrom12To13();

    /**
     * Method to upgrade database version from 13 to 14 by making required 
     * adjustment to the schema
     * @param void
     * @return void
     */
    void UpgradeVersionFrom13To14();

    /**
     * Method to upgrade database version from 14 to 15 by making required 
     * adjustment to the schema
     * @param void
     * @return void
     */
    void UpgradeVersionFrom14To15();

    /**
     * Method to upgrade database version from 15 to 16 by making required 
     * adjustment to the schema
     * @param void
     * @return void
     */
    void UpgradeVersionFrom15To16();

    /**
     * Method to upgrade database version from 16 to 17 by making required 
     * adjustment to the schema
     * @param void
     * @return void
     */
    void UpgradeVersionFrom16To17();

    /**
     * Method to process settings store config value based on input parameter
     * @param[in] rcontentValues content value
     * @param[in] rjsonConfigValue settings store config value
     * @return 0 if config value is successfully processed, non-zero otherwise
     */
    int ProcessSettingStoreConfigValue(CContentValues &rcontentValues,
                                       ic_utils::Json::Value &rjsonConfigValue);

    /**
     * Method to create tables for the given table name
     * @param[in] rstrTableName table name
     * @return void
     */
    void CreateTables(const std::string &rstrTableName);

    //! Mutex variable
    ic_utils::CIgniteMutex m_QueryLock;

    //! Member variable to store db path
    std::string m_strDBPath;

    //! Member variable to store Sql command
    sqlite3 *m_pSQLiteDB;

    //! Member variable to store close request status
    bool m_bCloseRequested;

    //! Thread condition wait variable
    ic_utils::CThreadCondition m_WaitFlag;

    //! Mutex variable
    ic_utils::CIgniteMutex m_WaitMutex;

    //! Member variable to instance of CUploadMode class
    CUploadMode *m_pUploadMode;
};
} /* namespace ic_core */
#endif /* CDATABASE_H */