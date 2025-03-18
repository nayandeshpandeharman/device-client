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

#include <algorithm>
#include "db/CDataBaseFacade.h"
#include "CIgniteLog.h"

//! Macro for 'CDataBaseFacade' string
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CDataBaseFacade"

namespace ic_core 
{
//! Constant key for 'table' string
const std::string TABLE = "table";

CDataBaseFacade::CDataBaseFacade()
{
    m_pSQLiteDbInstance = CDatabase::GetInstance();
}

CDataBaseFacade* CDataBaseFacade::GetInstance()
{
    static CDataBaseFacade sSelf;
    return &sSelf;
}

CCursor* CDataBaseFacade::Query(const std::string strTable,
                const std::vector<std::string> &rvecProjection,
                const std::string &rstrSelection,
                const std::vector<std::string> &rvecOrderBy,
                const int nLimit)
{
    HCPLOG_METHOD();
    if (!strTable.empty())
    {
        return m_pSQLiteDbInstance->Query(strTable, rvecProjection, rstrSelection,
                                                          rvecOrderBy, nLimit);
    }
    return NULL;
}

long CDataBaseFacade::Insert(const std::string strTable, CContentValues *pData)
{
    long lRetValue = eINVALID_OPERATION;
    if (!strTable.empty() && pData != NULL)
    {
        lRetValue = m_pSQLiteDbInstance->Insert(strTable, pData);
    }
    return lRetValue;
}

bool CDataBaseFacade::Update(const std::string strTable, CContentValues *pData,
                                               const std::string &rstrSelection)
{
    bool bRetValue = false;
    if (!strTable.empty() && pData != NULL)
    {
        bRetValue = m_pSQLiteDbInstance->Update(strTable, pData, rstrSelection);
    }
    return bRetValue;
}

bool CDataBaseFacade::Remove(const std::string strTable, 
                                               const std::string &rstrSelection)
{
    HCPLOG_METHOD();
    bool retValue = false;
    if(!strTable.empty())
    {
        retValue = m_pSQLiteDbInstance->Remove(strTable,rstrSelection);
    }
    return retValue;
}

bool CDataBaseFacade::StartTransaction()
{
    return m_pSQLiteDbInstance->StartTransaction();
}

int CDataBaseFacade::EndTransaction(bool bSuccess)
{
    return m_pSQLiteDbInstance->EndTransaction(bSuccess);
}

size_t CDataBaseFacade::GetSize()
{
    return m_pSQLiteDbInstance->GetSize();
}

int CDataBaseFacade::ExecuteCommand(std::string &rstrCmd)
{
    int nRetValue = eINVALID_OPERATION;
    if(!rstrCmd.empty())
    {
        nRetValue = m_pSQLiteDbInstance->ExecuteCommand(rstrCmd);
    }
    return nRetValue;
}

bool CDataBaseFacade::VacuumDb()
{
    return m_pSQLiteDbInstance->VacuumDb();
}

int CDataBaseFacade::CloseConnection()
{
    HCPLOG_METHOD();
    int nRetValue = eINVALID_OPERATION;
    nRetValue = m_pSQLiteDbInstance->CloseConnection();
    HCPLOG_C << "connection closed-retValue=" << nRetValue;
    return nRetValue;
}

bool CDataBaseFacade::ClearTables()
{
    return m_pSQLiteDbInstance->ClearTables();
}

int CDataBaseFacade::ResetDatabase()
{
    HCPLOG_METHOD();
    return m_pSQLiteDbInstance->ResetDatabase();
}

int CDataBaseFacade::RemoveDatabase()
{
    return m_pSQLiteDbInstance->RemoveDatabase();
}

int CDataBaseFacade::GetRowCountOfTable(const std::string strTableName)
{
    std::vector<std::string> vecProjection;
    vecProjection.push_back("COUNT(*)");
    CCursor *pCursor = Query(strTableName, vecProjection);

    int nRowCount = -1;
    if (pCursor != NULL)
    {
        if (pCursor->MoveToFirst())
        {
            nRowCount = pCursor->GetInt(0);
        }
        delete pCursor;
    }

    return nRowCount;
}

std::string CDataBaseFacade::GetComponentStatus()
{
    size_t totalDbSize = 0;
    ic_utils::Json::Value jsonDiagJson;
    bool bRetVal = false;
    int nCount = 0;
    CCursor *pCursor = NULL;

    std::vector<std::string> vecProjection;
    vecProjection.push_back(CDataBaseConst::COL_NAME_ID);

    std::string strSelection = 
                             CDataBaseConst::COL_TYPE_ID + " = '" + TABLE + "'";
    bool bTransactionStarted = StartTransaction();

    totalDbSize = GetSize();
    jsonDiagJson["overallDbSize"] = (unsigned int)totalDbSize;

    try
    {
        pCursor = Query(CDataBaseConst::TABLE_SQLITE_MASTER, vecProjection,
                        strSelection);
        if (pCursor)
        {
            // iterating through tables and deleting rows of all the table
            for (bool bHasNext = pCursor->MoveToFirst(); bHasNext; 
                                 bHasNext = pCursor->MoveToNext())
            {
                std::string tableName = pCursor->GetString(pCursor->
                                   GetColumnIndex(CDataBaseConst::COL_NAME_ID));
                HCPLOG_D << "getting row count for table " << tableName;
                nCount = GetRowCountOfTable(tableName);
                jsonDiagJson[tableName] = nCount;
            }

            delete pCursor;
            pCursor = NULL;
        }
    }
    catch (...)
    {
        HCPLOG_E << "Exception occurred ";

        if (pCursor)
        {
            delete pCursor;
            pCursor = NULL;
        }
    }

    if (bTransactionStarted)
    {
        bRetVal = EndTransaction(bTransactionStarted);
    }

    ic_utils::Json::FastWriter jsonFastWriter;
    std::string strDiagString(jsonFastWriter.write(jsonDiagJson));

    // FastWriter introduces newline at the end , that needs to be truncated
    strDiagString.erase(std::remove(strDiagString.begin(),
                               strDiagString.end(), '\n'), strDiagString.end());

    return strDiagString;
}
} /* namespace ic_core */
