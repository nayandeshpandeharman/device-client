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

#include <cstdlib>
#include <climits>
#include "CIgniteConfig.h"
#include "db/CLocalConfig.h"
#include "crypto/CRNG.h"
#include "CIgniteLog.h"
#include "CIgniteStringUtils.h"
#include "db/CDataBaseFacade.h"

namespace ic_core 
{
CLocalConfig* CLocalConfig::GetInstance(void)
{
    static CLocalConfig sLocalConfig;
    return &sLocalConfig;
}

CLocalConfig::CLocalConfig()
{
}

CLocalConfig::~CLocalConfig()
{
    
}

std::string CLocalConfig::GetIvRandomNumber(std::string strSeedKey)
{
    std::string strSeedRndNo = Get(DATA_ENCRYPT_RND_NO);

    if (strSeedRndNo.empty())
    {
        // Delete existing events in db
        CDataBaseFacade::GetInstance()->
                                      Remove(CDataBaseConst::TABLE_EVENT_STORE);

        // Generate a random number between 1 and INT_MAXg
        strSeedRndNo = CRNG::GetString(strSeedKey,INT_MAX);
        HCPLOG_METHOD() << DATA_ENCRYPT_RND_NO << " = " << strSeedRndNo;

        /*
         * Since we use this random number as IV for encryption, pad it with 
         * 'X' for length 16
         */
        if (strSeedRndNo.length() < 16)
        {
            strSeedRndNo.insert(strSeedRndNo.end(), 16 - strSeedRndNo.length(), 'X');
        }

        //store the random number in DB
        Set(DATA_ENCRYPT_RND_NO, strSeedRndNo);
    }

    return strSeedRndNo;
}

std::string CLocalConfig::Get(std::string strKey)
{
    HCPLOG_METHOD();
    
    std::vector<std::string> vecProjection;
    vecProjection.push_back(CDataBaseConst::COL_VALUE);
    CCursor* pCursor = CDataBaseFacade::GetInstance()->
                        Query(CDataBaseConst::TABLE_LOCAL_CONFIG, vecProjection,
                            CDataBaseConst::COL_KEY_VAL + "='" + strKey + "'");
    
    std::string strValue;
    if (pCursor)
    {
        if (pCursor->MoveToFirst())
        {
            strValue = pCursor->GetString(pCursor->GetColumnIndex(CDataBaseConst::COL_VALUE));
        }
        delete pCursor;
    }
    
    HCPLOG_T << " returning " << strValue;
    return strValue;
}

bool CLocalConfig::Set(std::string strKey, std::string strValue)
{
    HCPLOG_METHOD() << "Key=" << strKey << "; value=" << strValue;
    CDataBaseFacade* pDBFacade = CDataBaseFacade::GetInstance();
    std::vector<std::string> vecProjection;
    vecProjection.push_back(CDataBaseConst::COL_ID);
    CCursor* pCursor = pDBFacade->Query(CDataBaseConst::TABLE_LOCAL_CONFIG,
              vecProjection, CDataBaseConst::COL_KEY_VAL + "='" + strKey + "'");
    
    long lId = -1;
    if (pCursor)
    {
        if (pCursor->MoveToFirst())
        {
            lId = pCursor->GetLong(pCursor->
                                        GetColumnIndex(CDataBaseConst::COL_ID));
        }
        delete pCursor;
    }
    
    bool bSuccess;
    CContentValues data;
    data.Put(CDataBaseConst::COL_KEY_VAL, strKey);
    data.Put(CDataBaseConst::COL_VALUE, strValue);
    if (-1 == lId)
    {
        bSuccess = (pDBFacade->Insert(
                                CDataBaseConst::TABLE_LOCAL_CONFIG, &data) > 0);
    }
    else
    {
        bSuccess = pDBFacade->Update(CDataBaseConst::TABLE_LOCAL_CONFIG, &data,
                                                 CDataBaseConst::COL_ID + "=" + 
                                        ic_utils::CIgniteStringUtils::NumberToString(lId));
    }
    
    return bSuccess;
}

int CLocalConfig::CountRowsStartsWithKey(std::string strKey)
{
    HCPLOG_METHOD();
    
    std::vector<std::string> vecProjection;
    vecProjection.push_back("COUNT(*)");
    CCursor* pCursor = CDataBaseFacade::GetInstance()->
                        Query(CDataBaseConst::TABLE_LOCAL_CONFIG, vecProjection,
                       CDataBaseConst::COL_KEY_VAL + " LIKE '" + strKey + "%'");
    
    int nRows = 0;
    if (pCursor)
    {
        if (pCursor->MoveToFirst())
        {
            nRows = pCursor->GetInt(0);
        }
        delete pCursor;
    }

    return nRows;
}

int CLocalConfig::RemoveRowsStartsWithKey(std::string strKey)
{
    return CDataBaseFacade::GetInstance()->Remove(
                                             CDataBaseConst::TABLE_LOCAL_CONFIG,
                       CDataBaseConst::COL_KEY_VAL + " LIKE '" + strKey + "%'");
}

int CLocalConfig::Remove(std::string strKey)
{
    HCPLOG_METHOD();
    return CDataBaseFacade::GetInstance()->Remove(
                                             CDataBaseConst::TABLE_LOCAL_CONFIG,
                             CDataBaseConst::COL_KEY_VAL + "='" + strKey + "'");
}

#ifdef UNIT_TEST
#include "config/ClientConfig.h"
#include "dam/CInsightEngine.h"

#define OUTPUT cout

int main(int argc, char* argv[])
{\
    HCPLOG_LINE() << " - Creating Config to initialize it now...";
    CIgniteConfig::CreateSingleton(sHCPConfigPath);

    string type = LOCAL_CONFIG;
    CLocalConfig* config = (CLocalConfig*) CLocalConfig::GetInstance();

    config->Set("login", "asdf");
    config->Set("PWD_VAL", "jkl;");
    OUTPUT << "Login=" << config->Get("login") << endl;
    OUTPUT << "Passcode=" << config->Get("PWD_VAL") << endl;
    config->Set("login", "bteg");
    OUTPUT << "Login after update=" << config->Get("login") << endl;
    config->Remove("password");
    OUTPUT << "Passcode after removal=" << config->Get("password") << endl;
    std::exit(0);
}
#endif
} /* namespace ic_core */