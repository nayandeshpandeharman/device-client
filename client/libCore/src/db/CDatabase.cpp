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

#include <string.h>
#include <sys/stat.h>
#include "core/CAesSeed.h"
#include <fstream>
#include <iosfwd>
#include <algorithm>
#include "db/CDatabase.h"
#include "CIgniteConfig.h"
#include "CIgniteDateTime.h"
#include "CIgniteFileUtils.h"
#include "CIgniteLog.h"
#include "db/CSqlException.h"
#include "crypto/CIgniteDataSecurity.h"
#include "core/CKeyGenerator.h"
#include "CIgniteStringUtils.h"
#include "dam/CEventWrapper.h"
#include "CIgniteStringUtils.h"
#include "CIgniteDateTime.h"

#if defined(ANDROID)|| defined(__ANDROID__)
#define IGNITE_USE_PRAGMA_TEMP_DIRECTORY
#endif

//! Macro for 'CDatabase' string
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CDatabase"

namespace ic_core 
{
namespace 
{
//! Macro for 'vendor' string
#define KEY_VENDOR "vendor"

//! Constant key for 'db version' value
static const int DB_VERSION = 17;

//! Constant key array for 'event store columns' value
static const char* EVENT_STORE_COLUMNS[] =
{
    CDataBaseConst::COL_ID.c_str(),
    CDataBaseConst::COL_EVENT_ID.c_str(),
    CDataBaseConst::COL_TIMESTAMP.c_str(),
    CDataBaseConst::COL_TIMEZONE.c_str(),
    CDataBaseConst::COL_SIZE.c_str(),
    CDataBaseConst::COL_HAS_ATTACH.c_str(),
    CDataBaseConst::COL_PRIORITY.c_str(),
    CDataBaseConst::COL_EVENTS.c_str(),
    CDataBaseConst::COL_APPID.c_str(),
    CDataBaseConst::COL_TOPIC.c_str(),
    CDataBaseConst::COL_MID.c_str(),
    CDataBaseConst::COL_BATCH_SUPPORT.c_str(),
    CDataBaseConst::COL_STREAM_SUPPORT.c_str(),
    CDataBaseConst::COL_GRANULARITY.c_str()
};

//! Constant key for 'event store columns number' value
static const int EVENT_STORE_COL_NUM = 
                                    sizeof(EVENT_STORE_COLUMNS) / sizeof(char*);

//! Constant key array for 'invalid event store columns' value
static const char* INVALID_EVENT_STORE_COLUMNS[] =
{
        CDataBaseConst::COL_ID.c_str(),
        CDataBaseConst::COL_TIMESTAMP.c_str(),
        CDataBaseConst::COL_EVENTS.c_str()
};

//! Constant key for 'invalid event store columns number' value
static const int INVALID_EVENT_STORE_COL_NUM = 
                            sizeof(INVALID_EVENT_STORE_COLUMNS) / sizeof(char*);

//! Constant key array for 'local config columns' value
static const char* LOCAL_CONFIG_COLUMNS[] =
{
    CDataBaseConst::COL_ID.c_str(),
    CDataBaseConst::COL_KEY_VAL.c_str(),
    CDataBaseConst::COL_VALUE.c_str()
};

//! Constant key for 'local config columns number' value
static const int LOCAL_CONFIG_COL_NUM = 
                                   sizeof(LOCAL_CONFIG_COLUMNS) / sizeof(char*);

//! Constant key array for 'message sender store columns' value
static const char* MESSAGE_SENDER_STORE_COLUMNS[] =
{
    CDataBaseConst::COL_ID.c_str(),
    CDataBaseConst::COL_TIMESTAMP.c_str(),
    CDataBaseConst::COL_APPID.c_str(),
    CDataBaseConst::COL_TYPE.c_str(),
    CDataBaseConst::COL_EVENTS.c_str()
};

//! Constant key for 'message sender store number' value
static const int MESSAGE_SENDER_STORE_NUM = 
                           sizeof(MESSAGE_SENDER_STORE_COLUMNS) / sizeof(char*);

//! Constant key array for 'upload file columns' value
static const char* UPLOAD_FILE_COLUMNS[] =
{
    CDataBaseConst::COL_ID.c_str(),
    CDataBaseConst::COL_FILE_PATH.c_str(),
    CDataBaseConst::COL_SPLIT_COUNT.c_str(),
    CDataBaseConst::COL_IS_FILE_FINAL_CHUNK.c_str(),
    CDataBaseConst::COL_FILE_SIZE.c_str()
};

//! Constant key for 'upload file number' value
static const int UPLOAD_FILE_NUM = sizeof(UPLOAD_FILE_COLUMNS) / sizeof(char*);

//! Constant key array for 'ignite service settings columns' value
static const char* IGNITE_SERVICE_SETTINGS_COLUMNS[] =
{
        CDataBaseConst::COL_SETTING_ID.c_str(),
        CDataBaseConst::COL_SETTING_ENUM.c_str(),
        CDataBaseConst::COL_SETTING_VALUE.c_str(),
        CDataBaseConst::COL_SETTING_RESPONSE_STATUS.c_str(),
        CDataBaseConst::COL_SETTING_CORR_ID.c_str(),
        CDataBaseConst::COL_SETTING_SRC_ISDEVICE.c_str()
};

//! Constant key for 'ignite service settings number' value
static const int IGNITE_SERVICE_SETTINGS_NUM = 
                        sizeof(IGNITE_SERVICE_SETTINGS_COLUMNS) / sizeof(char*);

//! Mutex variable to synchronize database transaction
static ic_utils::CIgniteMutex g_TransactionMutex;

//! Mutex variable to synchronize sql query execution
static ic_utils::CIgniteMutex g_SqlMutex;

/**
 * Global method to use as a callback function while executing sql query and
 * perform database integrity check
 * @param[in] pData data value
 * @param[in] nArgc argument count to check the data integrity
 * @param[in] pchArgv argument having the actual data value
 * @param[in] pchColNames column name
 * @return 0 if database integrity check is successfully passed, non-zero 
 * otherwise
 */
static int callback_dbcheck(void *pData, int nArgc, 
                            char** pchArgv, char** pchColNames)
{
    HCPLOG_METHOD();

    if (nArgc != 1)
    {
        HCPLOG_E << "Integrity check query error:";
        return -1;
    }

    *((std::string*)pData) = pchArgv[0];
    return 0;
}

/**
 * Global method to use as a callback function while executing sql query and 
 * perform LocalConfig query error check
 * @param[in] pData data value
 * @param[in] nArgc argument count to check the LocalConfig query error
 * @param[in] pchArgv argument having the actual data value
 * @param[in] pchColNames column name
 * @return 0 if LocalConfig query check is successfully passed, non-zero 
 * otherwise
 */
static int callback_schema(void *pData, int nArgc, 
                           char **pchArgv, char** pchColNames)
{
    HCPLOG_METHOD();

    if (nArgc != 1)
    {
        HCPLOG_E << 
                 "LocalConfig query error: More than one matching row for key?";
        return -1;
    }

    *((std::string*)pData) = pchArgv[0];
    return 0;
}

/**
 * Global method to get the column names of the given table name
 * @param[in] strTable table name
 * @return returns vector of column names; if any error, returns empty vector
 */
static std::vector<std::string> get_column_names(std::string strTable)
{
    char **pchColNames = NULL;
    int nColNum = 0;
    std::vector<std::string> vecHeaders;

    if (0 == CDataBaseConst::TABLE_EVENT_STORE.compare(strTable))
    {
        pchColNames = (char**)EVENT_STORE_COLUMNS;
        nColNum = EVENT_STORE_COL_NUM;
    }
    else if (0 == CDataBaseConst::TABLE_INVALID_EVENT_STORE.compare(strTable))
    {
        pchColNames = (char**)INVALID_EVENT_STORE_COLUMNS;
        nColNum = INVALID_EVENT_STORE_COL_NUM;
    }
    else if (0 == CDataBaseConst::TABLE_ALERT_STORE.compare(strTable))
    {
        pchColNames = (char**)EVENT_STORE_COLUMNS;
        nColNum = EVENT_STORE_COL_NUM;
    }
    else if (0 == CDataBaseConst::TABLE_LOCAL_CONFIG.compare(strTable))
    {
        pchColNames = (char**)LOCAL_CONFIG_COLUMNS;
        nColNum = LOCAL_CONFIG_COL_NUM;
    }
    else if (0 == CDataBaseConst::MESSAGE_SENDER_STORE.compare(strTable))
    {
        pchColNames = (char**)MESSAGE_SENDER_STORE_COLUMNS;
        nColNum = MESSAGE_SENDER_STORE_NUM;
    }
    else if (0 == CDataBaseConst::TABLE_UPLOAD_FILE.compare(strTable))
    {
        pchColNames = (char**)UPLOAD_FILE_COLUMNS;
        nColNum = UPLOAD_FILE_NUM;
    }
    else if (0 == CDataBaseConst::TABLE_IGNITE_SERVICE_SETTINGS.compare(strTable))
    {
        pchColNames = (char**)IGNITE_SERVICE_SETTINGS_COLUMNS;
        nColNum = IGNITE_SERVICE_SETTINGS_NUM;
    }
    else
    {
        try
        {
            std::string strMsg = "Table " + strTable + " does not exist";
            throw CSqlException(ACP_SQLITE_ERROR, strMsg.c_str());
        }
        catch (CSqlException& e)
        {
            HCPLOG_E << e.ErrorMessage();
            return vecHeaders;
        }
    }

    for (int i = 0; i < nColNum; i++)
    {
        vecHeaders.push_back(pchColNames[i]);
    }
    return vecHeaders;
}

/**
 * Global method to decrypt event data based on given input parameter
 * @param[in] pDB database instance
 * @param[in] strInput data to decrypt
 * @return returns the decrypted data; if decryption failed, return the original
 * input i.e. strInput
 */
static std::string decrypt_event_data(CDatabase *pDB, std::string strInput)
{
    std::string strSeedRndNo;
    std::string strKeystore = CIgniteConfig::GetInstance()->GetString(
                                                       "DAM.Database.keystore");
    if (strKeystore == "android")
    {
        strSeedRndNo = CAesSeed::GetInstance()->GetIvRandom();
    }
    else
    {
        std::vector<std::string> vecProjection;
        vecProjection.push_back(CDataBaseConst::COL_VALUE);
        CCursor *pCursor = pDB->Query(CDataBaseConst::TABLE_LOCAL_CONFIG,
                                    vecProjection, CDataBaseConst::COL_KEY_VAL + 
                                    "='dataEncryRndNo'");
        if (pCursor) 
        {
            if (pCursor->MoveToFirst()) 
            {
                strSeedRndNo = pCursor->GetString(0);
            }
            delete pCursor;
        }
    }
    
    if (!strSeedRndNo.empty())
    {
        CIgniteDataSecurity securityObject(
                          CKeyGenerator::GetActivationKey(), strSeedRndNo);
        std::string strOutput = securityObject.Decrypt(strInput);
        if (!strOutput.empty())
        {
            return strOutput;
        }
    }

    // If decryption failed best we can do is return original input
    return strInput;
}

/**
 * Global method to check column aliases
 * @param[in]/[out] rvecColumns vector of columns
 * @return void
 */
void check_column_aliases(std::vector<std::string> &rvecColumns)
{
    static const std::string strAS = " as ";
    static const std::string strDOT = ".";

    for (int i = 0; i < rvecColumns.size(); i++)
    {
        std::string strColumn(rvecColumns[i]);
        std::transform(strColumn.begin(), strColumn.end(), strColumn.begin(),
                       (int(*)(int))std::tolower); // to lower case

        // check for column alisases
        size_t pos = strColumn.rfind(strAS);
        if (std::string::npos != pos)
        {
            // AS keyword detected
            size_t start = pos + strAS.length();
            std::string strTmp = rvecColumns[i].substr(start, 
                                               rvecColumns[i].length() - start);

            if (std::string::npos == strTmp.find_first_of(" "))
            {
                /*
                 * Recognizing alias only if last AS is not followed by any 
                 * spaces, otherwise it's an AS from a subquery
                 */
                rvecColumns[i] = strTmp;
                continue;
            }
        }

        // check for table aliases
        pos = strColumn.find(strDOT);
        if (std::string::npos != pos++)
        {
            /*
             * Dot detected, actual column name should be to the right from 
             * the dot
             */
            if (pos < strColumn.length())
            {
                rvecColumns[i] = rvecColumns[i].substr(
                                            pos, rvecColumns[i].length() - pos);
                continue;
            }
        }
    }
}
}

//! Constant key for 'table' string
const std::string TABLE = "table";

CDatabase::CDatabase() : m_pUploadMode(CUploadMode::GetInstance())
{
    CIgniteConfig *pConfig = CIgniteConfig::GetInstance();
    m_strDBPath = pConfig->GetString("DAM.Database.dbStore");

    Open();
}

CDatabase* CDatabase::GetInstance()
{
    static CDatabase sSelf;
    return &sSelf;
}

CCursor* CDatabase::Query(const std::string strTable, 
                          const std::vector<std::string> &rvecProjection,
                          const std::string &rstrSelection, 
                          const std::vector<std::string> &rvecOrderBy, 
                          const int nLimit)
{
    if (m_bCloseRequested)
    {
        HCPLOG_C << "Wait";
        /*
         * If database connection and file deletion is in progress
         * then all other db queries must be stopped
         */
        m_WaitFlag.ConditionWait(m_WaitMutex);
    }
    ic_utils::CScopeLock sLock(m_QueryLock);

    SqlQuery stQuery;
    stQuery.strTable = strTable;
    stQuery.vecProjection = rvecProjection.empty() ? get_column_names(strTable):
                                                                 rvecProjection;
    stQuery.strSelection = rstrSelection;
    stQuery.vecOrderBy = rvecOrderBy;
    stQuery.nLimit = nLimit;

    try
    {
        return new CCursor(m_pSQLiteDB, &stQuery);
    }
    catch (CSqlException ex)
    {
        HCPLOG_E << ex.ErrorMessage();
        return NULL;
    }
}

long CDatabase::Insert(const std::string strTable, CContentValues *pData)
{
    long long llId = -1;

    std::string strColumns, strValues;
    const std::string strSeparator = ", ";

    std::vector<std::string> vecColNames = pData->GetKeys();
    for (int i = 0; i < vecColNames.size(); i++)
    {
        strColumns.append(vecColNames[i]);
        strColumns.append(strSeparator);
        strValues.append("'");
        strValues.append(pData->GetAsString(vecColNames[i]));
        strValues.append("'");
        strValues.append(strSeparator);
    }
    strColumns = strColumns.substr(0, strColumns.length() - 
                                                         strSeparator.length());
    strValues = strValues.substr(0, strValues.length() - strSeparator.length());

    std::ostringstream statement;
    statement << "INSERT INTO " << strTable << " (" 
              << strColumns << ") VALUES (" << strValues << ");";

    HCPLOG_I << statement.str();

    std::string strSql = statement.str();
    bool bSuccess = (SQLITE_OK == SqliteExec(strSql, NULL, 0));

    HCPLOG_T <<"result : " << bSuccess;

    if (bSuccess)
    {
        llId = sqlite3_last_insert_rowid(m_pSQLiteDB);
        HCPLOG_T <<"id : " << llId;
    }

    return llId;
}

bool CDatabase::Update(const std::string strTable, CContentValues *pData, 
                       const std::string &rstrSelection)
{
    std::string strValues;
    const std::string strSeparator = ", ";
    std::vector<std::string> vecColNames = pData->GetKeys();
    for (int i = 0; i < vecColNames.size(); i++)
    {
        strValues.append(vecColNames[i]);
        strValues.append(" = '");
        strValues.append(pData->GetAsString(vecColNames[i]));
        strValues.append("'");
        strValues.append(strSeparator);
    }
    strValues = strValues.substr(0, strValues.length() - strSeparator.length());

    std::ostringstream statement;
    statement << "UPDATE " << strTable << " SET " << strValues;
    if (!rstrSelection.empty())
    {
        statement << " WHERE " << rstrSelection;
    }
    statement << ";";

    std::string strSql = statement.str();
    HCPLOG_T << strSql;
    bool bSuccess = (SQLITE_OK == SqliteExec(strSql, NULL, 0));

    return bSuccess;
}

bool CDatabase::Remove(const std::string strTable,
                       const std::string &rstrSelection)
{
    std::ostringstream statement;
    statement << "DELETE FROM " << strTable;
    if (!rstrSelection.empty())
    {
        statement << " WHERE " << rstrSelection;
    }
    statement << ";";

    std::string strSql = statement.str();
    bool bSuccess = (SQLITE_OK == SqliteExec(strSql, NULL, 0));

    return bSuccess;
}

int CDatabase::Open()
{
    bool bExists = ic_utils::CIgniteFileUtils::Exists(m_strDBPath);

    if (!bExists)
    {
        std::string strDbDir = m_strDBPath.substr(
                                            0, m_strDBPath.find_last_of("/\\"));
        ic_utils::CIgniteFileUtils::MakeDirectory(strDbDir);
    }

    g_SqlMutex.Lock();
    int nSqlRes = sqlite3_open(m_strDBPath.c_str(), &m_pSQLiteDB);
    g_SqlMutex.Unlock();

    if (SQLITE_OK != nSqlRes)
    {
        HCPLOG_F << "Unable to open DB file: " << m_strDBPath;
        
        /*
         * Considering the criticality of DB-OPEN-FAILURE, let us not handle it 
         * gracefully so that this will cause the application to exit to get 
         * attention.
         */
        throw CSqlException(ACP_SQLITE_ERROR, "Database not open");
    }

    if (bExists)
    {
        if (!CheckIntegrity())
        {
            Recover();
        }
        CheckVersion();
        if (StartTransaction())
        {
            EndTransaction(OnCreate());
        }
    }
    else
    {
        if (StartTransaction())
        {
            EndTransaction(OnCreate());
        }
        SetVersion(DB_VERSION);
    }

#ifdef IGNITE_USE_PRAGMA_TEMP_DIRECTORY
    SetTempDirPragma();
#endif

    m_bCloseRequested = false;
    m_WaitFlag.ConditionBroadcast();

    return nSqlRes;
}

void CDatabase::SetTempDirPragma()
{
    HCPLOG_METHOD();
    std::ofstream file;
    std::string strTempDbPath = "";
    CIgniteConfig *pConfig = CIgniteConfig::GetInstance();
    
    // Path needs to be configured in config file , default is /sdcard/harman
    strTempDbPath = pConfig->GetString("DAM.Database.tempDbStore");

    if (strTempDbPath.empty())
    {
        strTempDbPath = "/sdcard/harman/";
    }

    std::string strFn = "/sdcard/harman/DB_setTempDirPragma";

    file.open(strFn.c_str());
    file << "FileOpened-SetTempDirPragma\n";
    file << "TimeStamp : " << ic_utils::CIgniteStringUtils::NumberToString(
                                    ic_utils::CIgniteDateTime::GetCurrentTimeMs()) << "\n";
    std::string strResponse = "";
    if (m_pSQLiteDB)
    {
        std::string strSql = "pragma temp_store_directory=\'" + 
                                                          strTempDbPath + "\';";
        file << "sql=" << strSql << "\n";
        int nRc = SqliteExec( strSql , callback_dbcheck , (void*)&strResponse);
        if (nRc != SQLITE_OK)
        {
            HCPLOG_E << "pragma temp_store_directory=\'" + strTempDbPath+"\';";
            file << strSql << "\n";
        }
    }
    file << "SetTempDirPragma response :" << strResponse;
    file.close();
}

void CDatabase::SetPagePragma()
{
    HCPLOG_METHOD();
    std::ofstream file;
    std::string strFn = "/sdcard/harman/vac_setPagePragma_";
    strFn += ic_utils::CIgniteStringUtils::NumberToString(
                                            ic_utils::CIgniteDateTime::GetCurrentTimeMs());
    strFn += ".txt";
    file.open(strFn.c_str(), std::ios::app);
    file << "FileOpened-setPagePragma\n";

    std::string strResponse="";
    if (m_pSQLiteDB)
    {
        std::string strSql = "pragma page_size=65536;";
        file << "sql=" << strSql << "\n";
        int nRc = SqliteExec( strSql , callback_dbcheck , (void*)&strResponse);
        if (nRc != SQLITE_OK)
        {
            HCPLOG_E << "error in setting pragma page_size=65536;";
            file << "setPagePragma error\n";
        }
    }
    file << "setPagePragma response :" << strResponse;
    file.close();
}

bool CDatabase::CheckIntegrity()
{
    HCPLOG_METHOD();
    std::string strResponse;
    if (m_pSQLiteDB)
    {
        std::string strSql = "pragma integrity_check;";
        int nRc = SqliteExec( strSql , callback_dbcheck , (void*)&strResponse);
        if (nRc != SQLITE_OK)
        {
            HCPLOG_E << "DB integrity check get error";
        }
    }
    HCPLOG_I << "Database check response :" << strResponse;
    if (strcasecmp(strResponse.c_str(), "ok") == 0)
    {
        HCPLOG_I << "Database passed the integrity test..!";
        return true;
    }
    else
    {
        HCPLOG_I << "Database failed the integrity test..!";
        return false;
    }
}

void CDatabase::CheckVersion()
{
    bool bTransactionInProgress = StartTransaction();

    bool bSuccess = false;
    int nCurrVer = GetVersion();

    if (DB_VERSION != nCurrVer)
    {
        if (DB_VERSION > nCurrVer)
        {
            bSuccess = OnUpgrade(nCurrVer, DB_VERSION);
        }
        else
        {
            bSuccess = OnDowngrade(nCurrVer, DB_VERSION);
        }
        SetVersion(DB_VERSION);
    }

    if (bTransactionInProgress)
    {
        EndTransaction(bSuccess);
    }
}

bool CDatabase::Recover()
{
    HCPLOG_METHOD();
    bool bRet = false;
    std::string strSql = "vacuum;";

    if (SqliteExec( strSql , NULL , 0) != SQLITE_OK)
    {
        HCPLOG_E << "DB Vaccum error";
    }
    else
    {
        if (CheckIntegrity())
        {
            HCPLOG_I << "Database recovered by Vaccum";
            return true;
        }
    }

    strSql = "DROP TABLE EVENT_STORE;";
    if (SqliteExec( strSql , NULL , 0) != SQLITE_OK)
    {
        HCPLOG_E << "DB drop event_store error";
    }
    else
    {
        if (CheckIntegrity())
        {
            HCPLOG_I << "Database recovered by dropping EVEN_STORE table";
            return true;
        }
    }
    
    if (!bRet)
    {
        HCPLOG_E << "Database recovery failed deleting the entire DB!!";
        sqlite3_close(m_pSQLiteDB);
        m_pSQLiteDB = NULL;
        Backup();
        Open();
        HCPLOG_I << "Database recreated!!";
    }
    return bRet;
}

void CDatabase::Backup()
{
    std::string strDest = m_strDBPath + ".bk";
    ic_utils::CIgniteFileUtils::Move(m_strDBPath, strDest);
}

int CDatabase::Remove()
{
    HCPLOG_METHOD();
    int nRc;
    nRc = ic_utils::CIgniteFileUtils::Remove(m_strDBPath);
    if (ic_utils::CIgniteFileUtils::Exists(m_strDBPath + "-journal"))
    {
        nRc = ic_utils::CIgniteFileUtils::Remove(m_strDBPath + "-journal");
    }
    return nRc;
}

int CDatabase::CloseConnection()
{
    int nRc = SQLITE_ERROR;
    HCPLOG_C << "Closing connection";
    m_bCloseRequested = true;

    if (m_pSQLiteDB!=NULL) 
    {
        nRc = sqlite3_close(m_pSQLiteDB);
        m_pSQLiteDB = NULL;
    }

    HCPLOG_C << "connection closed";
    return nRc;
}

int CDatabase::ResetDatabase()
{
    int nRc = SQLITE_ERROR;
    nRc = CloseConnection();

    if (SQLITE_OK == nRc)
    {
        nRc = Remove();
    }

    if (SQLITE_OK == nRc)
    {
        nRc = Open();
    }

    return nRc;
}

int CDatabase::RemoveDatabase()
{
    HCPLOG_METHOD();
    int nRc = SQLITE_ERROR;
    nRc = CloseConnection();

    if (nRc == SQLITE_OK)
    {
        nRc = Remove();
    }
    return nRc;
}

bool CDatabase::VacuumDb()
{
    HCPLOG_METHOD();
    g_TransactionMutex.Lock();

    std::string vacuum = "vacuum;";
    bool result = (SQLITE_OK == ExecuteCommand(vacuum));

    g_TransactionMutex.Unlock();

    return result;
}

int CDatabase::ExecuteCommand(std::string &rstrCmd)
{
    if (m_bCloseRequested)
    {
        HCPLOG_C << "Wait";
        /*
         * If database connection and file deletion is in progress then all 
         * other db queries must be stopped
         */
        m_WaitFlag.ConditionWait(m_WaitMutex);
    }

    return SqliteExec(rstrCmd, NULL, 0);
}

int CDatabase::SqliteExec(std::string &rstrSql, 
                          int(*callback)(void*, int, char**, char**), 
                          void *pData)
{
    HCPLOG_METHOD() << rstrSql;
    int nRc = SQLITE_ERROR;
    char *pchZErrMsg = 0;
    try
    {
        g_SqlMutex.Lock();
        nRc = sqlite3_exec(m_pSQLiteDB, rstrSql.c_str(), 
                                                  callback, pData, &pchZErrMsg);
        g_SqlMutex.Unlock();
        if ( nRc != SQLITE_OK )
        {
            throw CSqlException(nRc, pchZErrMsg);
        }
        if (pchZErrMsg)
        {
            sqlite3_free(pchZErrMsg);
        }
    }
    catch (CSqlException& e)
    {
        HCPLOG_EXCEPTION(e.ErrorMessage());
        if (e.ErrorCode() == SQLITE_CANTOPEN || e.ErrorCode() == SQLITE_CORRUPT)
        {
            HCPLOG_F << "Database Corrupted !!";
            Remove();
            exit(EXIT_FAILURE);
        } 
        else
        {
            CheckAndCreateTable(e.ErrorMessage());
        }
        
        if (pchZErrMsg)
        {
            sqlite3_free(pchZErrMsg);
        }
    }

    return nRc;
}

int CDatabase::GetVersion()
{
    std::string strVersion;
    std::string strSql = "PRAGMA user_version;";
    int nRc = SqliteExec(strSql , callback_schema , (void*)&strVersion);
    if (nRc != SQLITE_OK)
    {
        HCPLOG_E << "DB Version get error";
    }

    HCPLOG_T << "Version :" << strVersion;
    return atoi(strVersion.c_str());
}

void CDatabase::SetVersion(const int nVersion)
{
    std::string strSql = "PRAGMA user_version = " + 
                              ic_utils::CIgniteStringUtils::NumberToString(nVersion) + ";";
    int nRc = SqliteExec(strSql , NULL , 0);
    if (nRc != SQLITE_OK)
    {
        HCPLOG_E << "DB Version set error , exiting!!";
        exit(-1);
    }
}

bool CDatabase::CreateEventStoreTable()
{
    std::string strQuery = "CREATE TABLE IF NOT EXISTS " + 
            CDataBaseConst::TABLE_EVENT_STORE + " (" +
            CDataBaseConst::COL_ID + " INTEGER PRIMARY KEY AUTOINCREMENT, " +
            CDataBaseConst::COL_EVENT_ID + " TEXT NOT NULL, " +
            CDataBaseConst::COL_TIMESTAMP + " INTEGER, " +
            CDataBaseConst::COL_TIMEZONE + " INTEGER DEFAULT 0, " +
            CDataBaseConst::COL_SIZE + " INTEGER DEFAULT 0, " +
            CDataBaseConst::COL_HAS_ATTACH + " INTEGER DEFAULT 0, " +
            CDataBaseConst::COL_PRIORITY + " INTEGER DEFAULT 3, " +
            CDataBaseConst::COL_EVENTS + " TEXT NOT NULL, " +
            CDataBaseConst::COL_APPID + " TEXT DEFAULT '', " +
            CDataBaseConst::COL_TOPIC + " TEXT DEFAULT NULL, " +
            CDataBaseConst::COL_MID  + " INTEGER DEFAULT 0, " +
            CDataBaseConst::COL_STREAM_SUPPORT + "  TINYINT DEFAULT 0, " +
            CDataBaseConst::COL_BATCH_SUPPORT + " TINYINT DEFAULT 0, " +
            CDataBaseConst::COL_GRANULARITY + " TINYINT DEFAULT 0);";

    return (SQLITE_OK == SqliteExec(strQuery, NULL, 0));
}

bool CDatabase::CreateInvalidEventStoreTable()
{
    std::string strQuery = "CREATE TABLE IF NOT EXISTS " + 
            CDataBaseConst::TABLE_INVALID_EVENT_STORE + " (" +
            CDataBaseConst::COL_ID + " INTEGER PRIMARY KEY AUTOINCREMENT, " +
            CDataBaseConst::COL_TIMESTAMP + " INTEGER, " +
            CDataBaseConst::COL_EVENTS + " TEXT NOT NULL);";

    return (SQLITE_OK == SqliteExec(strQuery, NULL, 0));
}

bool CDatabase::CreateMessageSenderStoreTable()
{
    std::string strQuery = "CREATE TABLE IF NOT EXISTS " + 
            CDataBaseConst::MESSAGE_SENDER_STORE + " (" +
            CDataBaseConst::COL_ID + " INTEGER PRIMARY KEY AUTOINCREMENT, " +
            CDataBaseConst::COL_APPID + " TEXT NOT NULL, " +
            CDataBaseConst::COL_TYPE + " INTEGER DEFAULT 0, " +
            CDataBaseConst::COL_TIMESTAMP + " INTEGER, " +
            CDataBaseConst::COL_EVENTS + " TEXT NOT NULL);";

    return (SQLITE_OK == SqliteExec(strQuery, NULL, 0));
}

bool CDatabase::CreateAlertStoreTable()
{
    std::string strQuery = "CREATE TABLE IF NOT EXISTS " + 
            CDataBaseConst::TABLE_ALERT_STORE + " (" +
            CDataBaseConst::COL_ID + " INTEGER PRIMARY KEY AUTOINCREMENT, " +
            CDataBaseConst::COL_EVENT_ID + " TEXT NOT NULL, " +
            CDataBaseConst::COL_TIMESTAMP + " INTEGER, " +
            CDataBaseConst::COL_TIMEZONE + " INTEGER DEFAULT 0, " +
            CDataBaseConst::COL_SIZE + " INTEGER DEFAULT 0, " +
            CDataBaseConst::COL_HAS_ATTACH + " INTEGER DEFAULT 0, " +
            CDataBaseConst::COL_PRIORITY + " INTEGER DEFAULT 3, " +
            CDataBaseConst::COL_EVENTS + " TEXT NOT NULL, " +
            CDataBaseConst::COL_APPID + " TEXT DEFAULT '', " +
            CDataBaseConst::COL_TOPIC + " TEXT DEFAULT NULL, " +
            CDataBaseConst::COL_MID  + " INTEGER DEFAULT 0);";

    return (SQLITE_OK == SqliteExec(strQuery, NULL, 0));
}

bool CDatabase::CreateLocalConfigTable()
{
    std::string strQuery = "CREATE TABLE IF NOT EXISTS " + 
            CDataBaseConst::TABLE_LOCAL_CONFIG + " (" +
            CDataBaseConst::COL_ID + " INTEGER PRIMARY KEY AUTOINCREMENT, " +
            CDataBaseConst::COL_KEY_VAL + " TEXT UNIQUE NOT NULL, " +
            CDataBaseConst::COL_VALUE + " TEXT NOT NULL);";

    return (SQLITE_OK == SqliteExec(strQuery, NULL, 0));
}

bool CDatabase::CreateUploadFileTable()
{
    std::string strQuery = "CREATE TABLE IF NOT EXISTS " + 
            CDataBaseConst::TABLE_UPLOAD_FILE + " (" +
            CDataBaseConst::COL_ID + " INTEGER PRIMARY KEY AUTOINCREMENT, " +
            CDataBaseConst::COL_FILE_PATH + " TEXT NOT NULL, " +
            CDataBaseConst::COL_SPLIT_COUNT + " INTEGER DEFAULT 1, " +
            CDataBaseConst::COL_IS_FILE_FINAL_CHUNK + " TINYINT DEFAULT 1, " +
            CDataBaseConst::COL_FILE_SIZE + " BIGINT DEFAULT 0 );";

    return (SQLITE_OK == SqliteExec(strQuery, NULL, 0));
}

bool CDatabase::CreateIgniteSettingsTable()
{
    std::string strQuery = "CREATE TABLE IF NOT EXISTS " + 
            CDataBaseConst::TABLE_IGNITE_SERVICE_SETTINGS + " (" +
            CDataBaseConst::COL_SETTING_ID + " TEXT NOT NULL, " +
            CDataBaseConst::COL_SETTING_ENUM + " TEXT NOT NULL, " +
            CDataBaseConst::COL_SETTING_VALUE + " TEXT NOT NULL, " +
            CDataBaseConst::COL_SETTING_RESPONSE_STATUS + 
                                                   " BOOLEAN DEFAULT false, " +
            CDataBaseConst::COL_SETTING_CORR_ID + " TEXT NOT NULL,"+
            CDataBaseConst::COL_SETTING_SRC_ISDEVICE +" BOOLEAN DEFAULT false);";

    return (SQLITE_OK == SqliteExec(strQuery, NULL, 0));
}


bool CDatabase::OnCreate() 
{
    bool isCreated = (CreateEventStoreTable() && CreateLocalConfigTable() &&
                      CreateMessageSenderStoreTable() && 
                      CreateInvalidEventStoreTable() &&
                      CreateIgniteSettingsTable());

    if (m_pUploadMode->IsStreamModeSupported())
    {
        isCreated = isCreated && CreateAlertStoreTable();
    }
    if (m_pUploadMode->IsBatchModeSupported())
    {
        isCreated = isCreated && CreateUploadFileTable();
    }
    return isCreated;
}

bool CDatabase::OnUpgrade(int nOldVer, int nNewVer)
{
    HCPLOG_C << "Upgrading DB " << nOldVer << "->" << nNewVer;
    switch (nOldVer)
    {
    case 1:
    {
        UpgradeVersionFrom1To2();
    }
    case 2:
    {
        UpgradeVersionFrom2To3();
    }
    case 3:
    {
        UpgradeVersionFrom3To4();
    }
    case 4:
    {
        UpgradeVersionFrom4To5();
    }
    case 5:
    {
        UpgradeVersionFrom5To6();
    }
    case 6: 
    {
        UpgradeVersionFrom6To7();
    }
    case 7: 
    {
        UpgradeVersionFrom7To8();
    }
    case 8: 
    {
        UpgradeVersionFrom8To9();
    }
    case 9:
    {
        UpgradeVersionFrom9To10();
    }
    case 10:
    {
        UpgradeVersionFrom10To11();
    }
    case 11:
    {
        UpgradeVersionFrom11To12();
    }
    case 12:
    {
        UpgradeVersionFrom12To13();
    }
    case 13:
    {
        UpgradeVersionFrom13To14();
    }
    case 14: 
    {
        UpgradeVersionFrom14To15();
    }
    case 15:
    {
        UpgradeVersionFrom15To16();
    }
    case 16:
    {
        UpgradeVersionFrom16To17();
    }
    }
    return true;
}

void CDatabase::UpgradeVersionFrom1To2()
{
    std::string strQuery = "DROP TABLE IF EXISTS " + 
                                        CDataBaseConst::TABLE_EVENT_STORE + ";";
    SqliteExec(strQuery, NULL, 0);
    CreateEventStoreTable();

    if (m_pUploadMode->IsStreamModeSupported()) 
    {
        strQuery = "DROP TABLE IF EXISTS " + 
                                        CDataBaseConst::TABLE_ALERT_STORE + ";";
        SqliteExec(strQuery, NULL, 0);
        CreateAlertStoreTable();
    }

    strQuery = "DROP TABLE IF EXISTS " + 
                                     CDataBaseConst::MESSAGE_SENDER_STORE + ";";
    SqliteExec(strQuery, NULL, 0);
    CreateMessageSenderStoreTable();
}

void CDatabase::UpgradeVersionFrom2To3()
{
    std::string strQuery = "ALTER TABLE " + 
                            CDataBaseConst::TABLE_EVENT_STORE + " ADD COLUMN " +
                            CDataBaseConst::COL_TIMEZONE + " INTEGER;";
    if (SQLITE_OK == SqliteExec(strQuery, NULL, 0))
    {
        long long timeZone = 
                         ic_utils::CIgniteDateTime::GetTimeZoneOffsetMinutes() * 60 * 1000;
        strQuery = "UPDATE " + CDataBaseConst::TABLE_EVENT_STORE + " SET " + 
                              CDataBaseConst::COL_TIMEZONE + " = " +
                              ic_utils::CIgniteStringUtils::NumberToString(timeZone) + ";";

        SqliteExec(strQuery, NULL, 0);
    }
    if (m_pUploadMode->IsStreamModeSupported())
    {
        strQuery = "DROP TABLE IF EXISTS " + 
                                        CDataBaseConst::TABLE_ALERT_STORE + ";";
        SqliteExec(strQuery, NULL, 0);
        CreateAlertStoreTable();
    }
}

void CDatabase::UpgradeVersionFrom3To4()
{
    std::string strSql = "ALTER TABLE " + CDataBaseConst::TABLE_EVENT_STORE + 
                        " ADD COLUMN " + CDataBaseConst::COL_SIZE + " INTEGER;";
    
    if (SQLITE_OK == SqliteExec(strSql, NULL, 0))
    {
        std::vector<std::string> vecProjection;
        vecProjection.push_back(CDataBaseConst::COL_ID);
        vecProjection.push_back(CDataBaseConst::COL_EVENTS);
        CCursor *pCursor = Query(CDataBaseConst::TABLE_EVENT_STORE, 
                                 vecProjection);

        if (pCursor)
        {
            for (bool bWorking = pCursor->MoveToFirst(); 
                 bWorking; bWorking = pCursor->MoveToNext())
            {
                std::string strId = pCursor->GetString(pCursor->
                                        GetColumnIndex(CDataBaseConst::COL_ID));

                std::string strEventData = pCursor->GetString(
                           pCursor->GetColumnIndex(CDataBaseConst::COL_EVENTS));
                strEventData = decrypt_event_data(this, strEventData);

                CContentValues data;
                data.Put(CDataBaseConst::COL_SIZE, 
                                                (long long)strEventData.size());

                Update(CDataBaseConst::TABLE_EVENT_STORE, &data, 
                                          CDataBaseConst::COL_ID + "=" + strId);
            }
            delete pCursor;
        }
    }

    if (m_pUploadMode->IsStreamModeSupported()) 
    {
        std::string strQuery = "DROP TABLE IF EXISTS " + 
                                        CDataBaseConst::TABLE_ALERT_STORE + ";";
        SqliteExec(strQuery, NULL, 0);
        CreateAlertStoreTable();
    }
}

void CDatabase::UpgradeVersionFrom4To5()
{
    std::string strSql = "ALTER TABLE " + CDataBaseConst::TABLE_EVENT_STORE + 
                  " ADD COLUMN " + CDataBaseConst::COL_HAS_ATTACH + " INTEGER;";
    
    if (SQLITE_OK == SqliteExec(strSql, NULL, 0))
    {
        std::vector<std::string> vecProjection;
        vecProjection.push_back(CDataBaseConst::COL_ID);
        vecProjection.push_back(CDataBaseConst::COL_EVENTS);
        CCursor* pCursor = 
                        Query(CDataBaseConst::TABLE_EVENT_STORE, vecProjection);
        if (pCursor)
        {
            for (bool bWorking = pCursor->MoveToFirst(); 
                 bWorking; bWorking = pCursor->MoveToNext())
            {
                std::string strId = pCursor->GetString(
                               pCursor->GetColumnIndex(CDataBaseConst::COL_ID));
                std::string strEventData = pCursor->GetString(pCursor->
                                    GetColumnIndex(CDataBaseConst::COL_EVENTS));
                strEventData = decrypt_event_data(this, strEventData);
                
                CEventWrapper event;
                event.JsonToEvent(strEventData);
                CContentValues data;
                data.Put(CDataBaseConst::COL_HAS_ATTACH, 
                                        event.GetAttachments().empty() ? 0 : 1);
                Update(CDataBaseConst::TABLE_EVENT_STORE, &data, 
                                          CDataBaseConst::COL_ID + "=" + strId);
            }
            delete pCursor;
        }
    }
    if (m_pUploadMode->IsStreamModeSupported()) 
    {
        std::string strQuery = "DROP TABLE IF EXISTS " + 
                                        CDataBaseConst::TABLE_ALERT_STORE + ";";
        SqliteExec(strQuery, NULL, 0);
        CreateAlertStoreTable();
    }
}

void CDatabase::UpgradeVersionFrom5To6()
{
    if (m_pUploadMode->IsStreamModeSupported()) 
    {
        std::string strQuery = "DROP TABLE IF EXISTS " + 
                                        CDataBaseConst::TABLE_ALERT_STORE + ";";
        SqliteExec(strQuery, NULL, 0);
        CreateAlertStoreTable();
    }
}

void CDatabase::UpgradeVersionFrom6To7()
{
    HCPLOG_I << "Updating Table for external application ids";
    std::string strSql = "ALTER TABLE " + CDataBaseConst::TABLE_EVENT_STORE + 
               " ADD COLUMN " + CDataBaseConst::COL_APPID + " TEXT DEFAULT '';";
    int nResult = SqliteExec(strSql, NULL, 0);
    if (m_pUploadMode->IsStreamModeSupported()) 
    {
        strSql = "ALTER TABLE " + CDataBaseConst::TABLE_ALERT_STORE + 
               " ADD COLUMN " + CDataBaseConst::COL_APPID + " TEXT DEFAULT '';";
        nResult = SqliteExec(strSql, NULL, 0);
    }
}

void CDatabase::UpgradeVersionFrom7To8()
{
    if (m_pUploadMode->IsStreamModeSupported())
    {
        HCPLOG_I << "Updating Event table Table for event topic";
        std::string strSql = "ALTER TABLE " + 
                            CDataBaseConst::TABLE_EVENT_STORE + " ADD COLUMN " +
                            CDataBaseConst::COL_TOPIC + " TEXT DEFAULT NULL;";
        int nResult = SqliteExec(strSql, NULL, 0);

        strSql = "ALTER TABLE " + CDataBaseConst::TABLE_ALERT_STORE + 
             " ADD COLUMN " + CDataBaseConst::COL_TOPIC + " TEXT DEFAULT NULL;";
        nResult = SqliteExec(strSql, NULL, 0);
    }
}

void CDatabase::UpgradeVersionFrom8To9()
{
    if (m_pUploadMode->IsStreamModeSupported()) 
    {
        HCPLOG_I << "Updating Event table, add MID column";
        std::string strSql = "ALTER TABLE " + 
                            CDataBaseConst::TABLE_EVENT_STORE + " ADD COLUMN " +
                            CDataBaseConst::COL_MID + " INTEGER DEFAULT 0;";
        int nResult = SqliteExec(strSql, NULL, 0);

        HCPLOG_I << "Updating Alert table, add MID column";
        strSql = "ALTER TABLE " + CDataBaseConst::TABLE_ALERT_STORE + 
               " ADD COLUMN " + CDataBaseConst::COL_MID + " INTEGER DEFAULT 0;";
        nResult = SqliteExec(strSql, NULL, 0);
    }
}

void CDatabase::UpgradeVersionFrom9To10()
{
    HCPLOG_I << "Create Invalid event store table";
    CreateInvalidEventStoreTable();
}

void CDatabase::UpgradeVersionFrom10To11()
{
    std::string strSteamModeAltersql = "ALTER TABLE " + 
                     CDataBaseConst::TABLE_EVENT_STORE + " ADD COLUMN " +
                     CDataBaseConst::COL_STREAM_SUPPORT + " TINYINT DEFAULT 0;";

    std::string strBatchModeAltersql = "ALTER TABLE " + 
                      CDataBaseConst::TABLE_EVENT_STORE + " ADD COLUMN " +
                      CDataBaseConst::COL_BATCH_SUPPORT + " TINYINT DEFAULT 0;";

    int nResult = SqliteExec(strSteamModeAltersql, NULL, 0);
    nResult = SqliteExec(strBatchModeAltersql, NULL, 0);

    HCPLOG_T << "Update mode column according to configuration.";
    std::set<std::string> setStreamEventList = 
                                        m_pUploadMode->GetStreamModeEventList();
    std::set<std::string> setBatchEventlist = 
                                         m_pUploadMode->GetBatchModeEventList();
    std::string strStreamEvents("");
    std::string strBatchEvents("");
    std::string strAllConfiguredModesEventList("");

    if (setStreamEventList.size())
    {
        strStreamEvents="(";
        for (std::set<std::string>::iterator iter = setStreamEventList.begin(); 
             iter != setStreamEventList.end(); ++iter) 
        {
            strStreamEvents += "'" + *iter + "',";
            strAllConfiguredModesEventList += "'" + *iter + "',";
        }
        strStreamEvents.replace(strStreamEvents.size() - 1, 1, ")");
    }

    if (setBatchEventlist.size())
    {
        strBatchEvents="(";
        for (std::set<std::string>::iterator iter = setBatchEventlist.begin(); 
             iter != setBatchEventlist.end(); ++iter) 
        {
            strBatchEvents += "'" + *iter + "',";
            strAllConfiguredModesEventList += "'" + *iter + "',";
        }
        strBatchEvents.replace(strBatchEvents.size() - 1, 1, ")");
    }

    if (!strAllConfiguredModesEventList.empty())
    {
        strAllConfiguredModesEventList = "(" + strAllConfiguredModesEventList;
        strAllConfiguredModesEventList.replace(
                             strAllConfiguredModesEventList.size() - 1, 1, ")");
    }

    // Update for events belongs to defaultMode
    UpdateEventsDefaultMode(strAllConfiguredModesEventList);

    if (m_pUploadMode->IsStreamModeSupported())
    {
        CContentValues data;
        data.Put(CDataBaseConst::COL_STREAM_SUPPORT, 1);
        std::string strSelection(CDataBaseConst::COL_TOPIC + " IS NOT NULL " + 
                                  " OR " + CDataBaseConst::COL_APPID +" != ''");
        if (!strStreamEvents.empty())
        {
            strSelection += " OR "+ CDataBaseConst::COL_EVENT_ID + 
                                                       " IN " + strStreamEvents;
        }
        Update(CDataBaseConst::TABLE_EVENT_STORE,&data,strSelection);
    }

    if (!strBatchEvents.empty())
    {
        CContentValues data;
        data.Put(CDataBaseConst::COL_BATCH_SUPPORT, 1);
        std::string strSelection(CDataBaseConst::COL_EVENT_ID + " IN " + 
                                                                strBatchEvents);
        Update(CDataBaseConst::TABLE_EVENT_STORE,&data,strSelection);
    }
}

void CDatabase::UpdateEventsDefaultMode(const std::string &
                                        rstrConfiguredModesEvList)
{
    string strDefaultModeEventSelection("");
    if (!rstrConfiguredModesEvList.empty()) 
    {
        strDefaultModeEventSelection = CDataBaseConst::COL_EVENT_ID + 
                                         " NOT IN " + rstrConfiguredModesEvList;
    }

    if (m_pUploadMode->IsStreamModeSupportedAsDefault()) 
    {
        CContentValues data;
        data.Put(CDataBaseConst::COL_STREAM_SUPPORT, 1);
        Update(CDataBaseConst::TABLE_EVENT_STORE, &data,
                               strDefaultModeEventSelection);
    }

    if (m_pUploadMode->IsBatchModeSupportedAsDefault())
    {
        if (!strDefaultModeEventSelection.empty())
        {
            strDefaultModeEventSelection = "( " + strDefaultModeEventSelection +
                                                                      " ) AND ";
        }

        if (m_pUploadMode->IsStreamModeSupported()) 
        {
            strDefaultModeEventSelection += "( " + CDataBaseConst::COL_TOPIC + 
                 " IS NULL " + " AND " + CDataBaseConst::COL_APPID + " == '' )";
        }
        else
        {
            // For non-stream upload mode, put topiced and app events to batch
            strDefaultModeEventSelection += "( " + CDataBaseConst::COL_TOPIC + 
              " IS NOT NULL " + " OR " + CDataBaseConst::COL_APPID + " != '' )";
        }
        CContentValues data;
        data.Put(CDataBaseConst::COL_BATCH_SUPPORT, 1);
        Update(CDataBaseConst::TABLE_EVENT_STORE, &data, 
                               strDefaultModeEventSelection);
    }
}

void CDatabase::UpgradeVersionFrom11To12()
{
    // Uploadservice copying the attachments according to bandwidth manager.
}

void CDatabase::UpgradeVersionFrom12To13()
{
    //create new table for Ignite Settings
    CreateIgniteSettingsTable();
}

void CDatabase::UpgradeVersionFrom13To14()
{
    if (m_pUploadMode->IsBatchModeSupported())
    {
        std::string strSql = "ALTER TABLE " + CDataBaseConst::TABLE_UPLOAD_FILE +
          " ADD COLUMN " + CDataBaseConst::COL_FILE_SIZE + " BIGINT DEFAULT 0;";
        int nResult = SqliteExec(strSql, NULL, 0);
        HCPLOG_C << " Upgrade result : " << nResult ;
    }
}

void CDatabase::UpgradeVersionFrom14To15()
{
    std::string strSql = "ALTER TABLE " + 
           CDataBaseConst::TABLE_IGNITE_SERVICE_SETTINGS + " ADD COLUMN " +
           CDataBaseConst::COL_SETTING_SRC_ISDEVICE + " BOOLEAN DEFAULT false;";
    
    int nResult = SqliteExec(strSql, NULL, 0);
    HCPLOG_C << " Upgrade result : " << nResult ;
}

void CDatabase::UpgradeVersionFrom15To16()
{
    std::string strSql = "ALTER TABLE " + 
           CDataBaseConst::TABLE_IGNITE_SERVICE_SETTINGS + " ADD COLUMN " +
           CDataBaseConst::COL_SETTING_SRC_ISDEVICE + " BOOLEAN DEFAULT false;";

    int nResult = SqliteExec(strSql, NULL, 0);
    HCPLOG_C << " Upgrade (15->16)-stage-1: result : " << nResult ;

    // Add granularity reduction column
    std::string strQuery = "ALTER TABLE " + CDataBaseConst::TABLE_EVENT_STORE + 
       " ADD COLUMN " + CDataBaseConst::COL_GRANULARITY + " TINYINT DEFAULT 0;";

    nResult = SqliteExec(strQuery, NULL, 0);
    HCPLOG_C << " Upgrade (15->16)-stage-2: result : " << nResult ;
}

void CDatabase::UpgradeVersionFrom16To17()
{
    HCPLOG_I << " Upgrade (16->17) ";
    
    //configUpdate to be stored in SettingStore instead of LocalConfig
    std::vector<std::string> vecProjection;
    vecProjection.push_back(CDataBaseConst::COL_VALUE);
    CCursor* pCursor = Query(CDataBaseConst::TABLE_LOCAL_CONFIG, vecProjection, 
                             CDataBaseConst::COL_KEY_VAL + "= '" + 
                             CDataBaseConst::KEY_DB_CONFIG + "';");

    if (!pCursor)
    {
        // No configUpdate was received
        return;
    }

    if (pCursor->MoveToFirst())
    {
        std::string strValue = "";
        strValue = pCursor->GetString(pCursor->GetColumnIndex(
                                                    CDataBaseConst::COL_VALUE));
        ic_utils::Json::Reader jsonReader;
        ic_utils::Json::Value jsonConfigValue = ic_utils::Json::Value::nullRef;

        if (!jsonReader.parse(strValue, jsonConfigValue))
        {
            HCPLOG_E << "Parsing error";
            delete pCursor;
            return;
        }

        if (jsonConfigValue != ic_utils::Json::Value::nullRef && 
            jsonConfigValue.isArray())
        {
            std::vector<std::string> vecProjection;
            vecProjection.push_back(CDataBaseConst::COL_SETTING_ID);
            vecProjection.push_back(CDataBaseConst::COL_SETTING_ENUM);
            vecProjection.push_back(CDataBaseConst::COL_SETTING_VALUE);
            vecProjection.push_back(CDataBaseConst::COL_SETTING_CORR_ID);

            CContentValues contentValues;
            contentValues.Put(CDataBaseConst::COL_SETTING_ID, 
                              CDataBaseConst::KEY_DB_CONFIG);

            int nStatus = ProcessSettingStoreConfigValue(
                                                contentValues, jsonConfigValue);

            if (nStatus > 0)
            {
                std::string strSql = "DELETE FROM " + 
                                CDataBaseConst::TABLE_LOCAL_CONFIG + " WHERE " +
                                CDataBaseConst::COL_KEY_VAL + " ='" + 
                                CDataBaseConst::KEY_DB_CONFIG + "';";

                nStatus = SqliteExec(strSql, NULL, 0);
                HCPLOG_C << " Upgrade (16->17) result : " << nStatus ;
            }
        }
    }
    delete pCursor;
}

int CDatabase::ProcessSettingStoreConfigValue(CContentValues &rcontentValues,
                                             ic_utils::Json::Value &rjsonConfigValue)
{
    int nStatus = 0;
    ic_utils::Json::FastWriter jsonWriter;
    for (unsigned int nIindex = 0; nIindex < rjsonConfigValue.size(); 
            nIindex++) 
    {
        ic_utils::Json::Value &rjsonParameter = rjsonConfigValue[nIindex];
        for (ic_utils::Json::ValueIterator itrStart = rjsonParameter.begin(); 
                itrStart != rjsonParameter.end(); itrStart++) 
        {
            std::string strPath = itrStart.key().asString();
            rcontentValues.Put(CDataBaseConst::COL_SETTING_ENUM,strPath);
            rcontentValues.Put(CDataBaseConst::COL_SETTING_VALUE, 
                                jsonWriter.write(rjsonParameter));
            rcontentValues.Put(CDataBaseConst::COL_SETTING_CORR_ID,
                                CDataBaseConst::SETTING_NOT_APPLICABLE);
            nStatus = Insert(
                            CDataBaseConst::TABLE_IGNITE_SERVICE_SETTINGS, 
                            &rcontentValues);
        }
    }
    return nStatus;
}

bool CDatabase::OnDowngrade(int nOldVer, int nNewVer)
{
    // not used
    return false;
}

bool CDatabase::StartTransaction()
{
    g_TransactionMutex.Lock();
    std::string strSql = "BEGIN TRANSACTION;";
    bool bSuccess = (SQLITE_OK == SqliteExec(strSql, NULL, 0));
    if (!bSuccess)
    {
        g_TransactionMutex.Unlock();
    }

    return bSuccess;
}

int CDatabase::EndTransaction(bool bSuccess)
{
    int nRc = SQLITE_ERROR;
    std::string strSql = bSuccess ? "END TRANSACTION;" : "ROLLBACK;";
    nRc = SqliteExec(strSql, NULL, 0);
    g_TransactionMutex.Unlock();
    return nRc;
}

size_t CDatabase::GetSize()
{
    struct stat stStat_buf;
    int nRc = stat(m_strDBPath.c_str(), &stStat_buf);
    return nRc == 0 ? stStat_buf.st_size : -1;
}

bool CDatabase::ClearTables() 
{
    HCPLOG_METHOD();
    bool bRetVal = false;
    // "SELECT "name" FROM "sqlite_master" WHERE type = 'table';";
    std::vector<std::string> vecProjection;
    vecProjection.push_back(CDataBaseConst::COL_NAME_ID);
    std::string strSelection = CDataBaseConst::COL_TYPE_ID + " = '" + 
                                                         TABLE + "'";
    bool bTransactionStarted = StartTransaction();
    try 
    {
        CCursor *pCursor = Query(CDataBaseConst::TABLE_SQLITE_MASTER, 
                                 vecProjection, strSelection);
        if (pCursor) 
        {
            // Iterating through tables and deleting rows of all the table
            for (bool hasNext = pCursor->MoveToFirst(); 
                 hasNext; hasNext = pCursor->MoveToNext()) 
            {
                std::string strTableName = pCursor->GetString(pCursor->
                                   GetColumnIndex(CDataBaseConst::COL_NAME_ID));
                HCPLOG_C <<"Clearing table "<<strTableName;
                bRetVal = Remove(strTableName);
            }
            delete pCursor;
        }
    }
    catch(...)
    {
        HCPLOG_E << "Exception occurred ";
    }
    if (bTransactionStarted)
    {
        bRetVal = EndTransaction(bTransactionStarted);
    }
    
    //vacuum
    try 
    {
        std::string strSql = "vacuum;";
        if (SqliteExec(strSql, NULL, 0) != SQLITE_OK) 
        {
            HCPLOG_E << "DB Vaccum error";
            bRetVal = false;
        }
    }
    catch (CSqlException ex)
    {
        bRetVal = false;
        HCPLOG_E << ex.ErrorMessage();
    }
    return bRetVal;
}

void CDatabase::CheckAndCreateTable(const std::string strErrMsg) 
{

    std::string strTableName = "";
    std::string strErrorType = "";

    std::vector<std::string> vecResult;
    std::stringstream ss (strErrMsg);
    string strItem;

    while (getline (ss, strItem, ':')) 
    {
        vecResult.push_back(strItem);
    }

    /*
     * Expecting the exception message to have minimum 3 lines and the 2nd line 
     * indicates the table name, and the 1st line indicates the error message of
     *  'no such table'.
     */
    if (vecResult.size() >= 3) 
    {
        strErrorType = vecResult.at(1);
        if (strErrorType.compare("no such table") == 0) 
        {
            strTableName = vecResult.at(2);
            if (!strTableName.empty()) 
            {
                strTableName.erase(std::remove_if(strTableName.begin(), 
                               strTableName.end(), (int (*)(int)) std::isspace), 
                               strTableName.end());

                CreateTables(strTableName);
            }
        }
    }
}

void CDatabase::CreateTables(const std::string &rstrTableName)
{
    if (rstrTableName == CDataBaseConst::TABLE_EVENT_STORE) 
    {
        CreateEventStoreTable();
    } 
    else if (rstrTableName == CDataBaseConst::TABLE_LOCAL_CONFIG) 
    {
        CreateLocalConfigTable();
    } 
    else if (rstrTableName == CDataBaseConst::TABLE_ALERT_STORE) 
    {
        CreateAlertStoreTable();
    } 
    else if (rstrTableName == CDataBaseConst::TABLE_UPLOAD_FILE) 
    {
        CreateUploadFileTable();
    } 
    else if (rstrTableName == CDataBaseConst::TABLE_INVALID_EVENT_STORE) 
    {
        CreateInvalidEventStoreTable();
    }
    else if (rstrTableName == CDataBaseConst::TABLE_IGNITE_SERVICE_SETTINGS) 
    {
        CreateIgniteSettingsTable();
    } 
    else 
    {
        HCPLOG_E << "Unrecognized table name " << rstrTableName;
    }
}

CCursor::CCursor(sqlite3 *pDB, SqlQuery *pstQuery)
{
    m_vecColumns = pstQuery->vecProjection;
    check_column_aliases(m_vecColumns);
    m_Data.clear();
    m_nPosition  = -1;

    if (!RunQuery(pDB, pstQuery))
    {
        // The caller is expected to handle this exception
        throw CSqlException(ACP_SQLITE_ERROR, "Failed to execute SQL statement");
    }
    MoveToFirst();
}

bool CCursor::RunQuery(sqlite3 *pDB, SqlQuery *pstQuery)
{
    bool bSuccess = true;
    std::string strSql = GenerateSqlStatement(pstQuery);

    HCPLOG_I << strSql;

    g_SqlMutex.Lock();

    sqlite3_stmt *pSelstmt;
    if (SQLITE_OK == sqlite3_prepare_v2(pDB, strSql.c_str(), -1,
                                        &pSelstmt, NULL))
    {
        CContentValues row;
        while (SQLITE_ROW == sqlite3_step(pSelstmt))
        {
            row.Clear();
            for (int col = 0; col < m_vecColumns.size(); col++)
            {
                row.Put(m_vecColumns[col], 
                               (const char*)sqlite3_column_text(pSelstmt, col));
            }
            m_Data.push_back(row);
        }
    }
    else
    {
        bSuccess = false;
    }
    sqlite3_finalize(pSelstmt);
    g_SqlMutex.Unlock();

    return bSuccess;
}

std::string CCursor::GenerateSqlStatement(SqlQuery *pstQuery)
{
    const std::string strSeparator = ", ";
    std::string strColumns;
    if (pstQuery->vecProjection.empty())
    {
        strColumns = "*";
    }
    else
    {
        strColumns.clear();
        for (int i = 0; i < pstQuery->vecProjection.size(); i++)
        {
            strColumns.append(pstQuery->vecProjection[i]);
            strColumns.append(strSeparator);
        }
        strColumns = strColumns.substr(0, strColumns.length() - 
                                       strSeparator.length());
    }

    std::ostringstream statement;
    statement.clear();
    statement << "SELECT " << strColumns << " FROM " << pstQuery->strTable;
    if (!pstQuery->strSelection.empty())
    {
        statement << " WHERE " << pstQuery->strSelection;
    }

    if (!pstQuery->vecOrderBy.empty())
    {
        std::string strOrderBy;
        strOrderBy.clear();
        for (int i = 0; i < pstQuery->vecOrderBy.size(); i++)
        {
            strOrderBy.append(pstQuery->vecOrderBy[i]);
            strOrderBy.append(strSeparator);
        }
        strOrderBy = strOrderBy.substr(0, strOrderBy.length() - 
                                                         strSeparator.length());

        statement << " ORDER BY " << strOrderBy;
    }

    if (pstQuery->nLimit > 0)
    {
        statement << " LIMIT " << pstQuery->nLimit;
    }

    statement << ";";
    return statement.str();
}

void CCursor::RemoveRow()
{
    CList::iterator it = m_Data.GetIterator(m_nPosition);
    m_Data.erase(it);
    if (m_nPosition >= m_Data.size())
    {
        m_nPosition = m_Data.size() - 1;
    }
}

bool CCursor::MoveToPosition(int nPosition)
{
    if ((nPosition < 0) || (nPosition >= m_Data.size()))
    {
        return false;
    }

    m_nPosition = nPosition;
    return true;
}

bool CCursor::MoveToFirst()
{
    return MoveToPosition(0);
}

bool CCursor::MoveToLast()
{
    return MoveToPosition(m_Data.size() - 1);
}

bool CCursor::MoveToNext()
{
    return MoveToPosition(m_nPosition + 1);
}

bool CCursor::MoveToPrevious()
{
    return MoveToPosition(m_nPosition - 1);
}

std::vector<std::string> CCursor::GetColumnNames()
{
    return m_vecColumns;
}

int CCursor::GetColumnIndex(std::string strColName)
{
    for (int i = 0; i < m_vecColumns.size(); i++)
    {
        if (strColName == m_vecColumns[i])
        {
            return i;
        }
    }

    return -1;
}

std::string CCursor::GetString(int nColIndex)
{
    try
    {
        if ((nColIndex < 0) || (nColIndex >= m_vecColumns.size()))
        {
            throw std::range_error("Invalid column index");
        }
        CContentValues row = m_Data[m_nPosition];
        return row.GetAsString(m_vecColumns[nColIndex]);
    }
    catch (std::range_error err) 
    {
        HCPLOG_E << err.what();
    }
    return "";
}

long long CCursor::GetLong(int nColIndex)
{
    try
    {
        if ((nColIndex < 0) || (nColIndex >= m_vecColumns.size()))
        {
            throw std::range_error("Invalid column index");
        }
        CContentValues row = m_Data[m_nPosition];
        return row.GetAsLong(m_vecColumns[nColIndex]);
    }
    catch (std::range_error err) 
    {
        HCPLOG_E << err.what();
    }
    return 0l;
}

int CCursor::GetInt(int nColIndex)
{
    try
    {
        if ((nColIndex < 0) || (nColIndex >= m_vecColumns.size()))
        {
            throw std::range_error("Invalid column index");
        }
        CContentValues row = m_Data[m_nPosition];
        return row.GetAsInt(m_vecColumns[nColIndex]);
    }
    catch (std::range_error err) 
    {
        HCPLOG_E << err.what();
    }
    return 0;
}

bool CCursor::GetBool(int nColIndex)
{
    try
    {
        if ((nColIndex < 0) || (nColIndex >= m_vecColumns.size()))
        {
            throw std::range_error("Invalid column index");
        }
        CContentValues row = m_Data[m_nPosition];
        return row.GetAsBool(m_vecColumns[nColIndex]);
    }
    catch (std::range_error err) 
    {
        HCPLOG_E << err.what();
    }
    return false;
}

double CCursor::GetDouble(int nColIndex)
{
    try
    {
        if ((nColIndex < 0) || (nColIndex >= m_vecColumns.size()))
        {
            throw std::range_error("Invalid column index");        
        }
        CContentValues row = m_Data[m_nPosition];
        return row.GetAsDouble(m_vecColumns[nColIndex]);
    }
    catch (std::range_error err) 
    {
        HCPLOG_E << err.what();
    }
    return 0.0f;
}

float CCursor::GetFloat(int nColIndex)
{
    try
    {
        if ((nColIndex < 0) || (nColIndex >= m_vecColumns.size()))
        {
            throw std::range_error("Invalid column index");
        }
        CContentValues row = m_Data[m_nPosition];
        return row.GetAsFloat(m_vecColumns[nColIndex]);
    }
    catch (std::range_error err) 
    {
        HCPLOG_E << err.what();
    }
    return 0.0f;
}

int CCursor::Size()
{
    return m_Data.size();
}

bool CCursor::Empty()
{
    return m_Data.empty();
}

CCursor::CList::iterator CCursor::CList::GetIterator(int index)
{
    iterator it = begin();
    try
    {
        if ((index < 0) || (index >= size()))
        {
            throw std::range_error("Invalid cursor position");
        }

        std::advance(it, index);
    }
    catch (std::range_error err) 
    {
        HCPLOG_E << err.what();
    }
    return it;
}

CContentValues CCursor::CList::operator [](int index)
{
    return *(GetIterator(index));
}
} /* namespace ic_core */