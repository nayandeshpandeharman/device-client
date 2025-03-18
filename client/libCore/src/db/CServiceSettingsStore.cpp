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

#include "CIgniteConfig.h"
#include "db/CServiceSettingsStore.h"
#include "db/CDataBaseFacade.h"
#include "CIgniteLog.h"
#include "CIgniteStringUtils.h"
#include "jsoncpp/json.h"

//! Macro for 'CServiceSettingsStore' string
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CServiceSettingsStore"

namespace ic_core
{
//! Constant key for 'value' string
static const std::string VALUE = "value";

//! Constant key for 'settingEnum' string
static const std::string SETTING_ENUM = "settingEnum";

//! Constant key for 'corrId' string
static const std::string CORR_ID = "corrId";

//! Constant key for 'responseStatus' string
static const std::string SETTING_RESPONSE_STATUS = "responseStatus";

//! Constant key for 'srcIsDevice' string
static const std::string SETTING_SRC_ISDEVICE = "srcIsDevice";

//! Constant key for 'SUCCESS' string
static const std::string STATUS_SUCCESS = "SUCCESS";

//! Constant key for 'FAILURE' string
static const std::string STATUS_FAILURE = "FAILURE";

//! Constant key for 'settingId' string
static const std::string SETTING_ID = "settingId";

CServiceSettingsStore *CServiceSettingsStore::GetInstance() 
{
    static CServiceSettingsStore instance;
    return &instance;
}

CServiceSettingsStore::CServiceSettingsStore() 
{

}

bool CServiceSettingsStore::StoreSettings(std::string strServiceId, 
                                          ic_utils::Json::Value &rjsonSettings,
                                          std::string strMsgId,
                                          bool bIsFromDevice)
{
    HCPLOG_METHOD();

    bool bSuccess = true;
    CContentValues contentValues;

    for (int itr = 0; itr < rjsonSettings.size(); itr++) 
    {
        ic_utils::Json::Value jsonSettingObj = rjsonSettings[itr];

        std::string strObjSettingEnum = jsonSettingObj[SETTING_ENUM].asString();
        std::string strObjValue = jsonSettingObj[VALUE].asString();

        ic_utils::Json::Value jsonObjVal;
        jsonObjVal[strObjSettingEnum] = jsonSettingObj[VALUE];
        ic_utils::Json::FastWriter jsonFastWriter;
        std::string strObjValStr = jsonFastWriter.write(jsonObjVal);

        contentValues.Put(CDataBaseConst::COL_SETTING_ID, strServiceId);
        contentValues.Put(CDataBaseConst::COL_SETTING_ENUM, strObjSettingEnum);
        contentValues.Put(CDataBaseConst::COL_SETTING_VALUE, strObjValStr);
        contentValues.Put(CDataBaseConst::COL_SETTING_CORR_ID, strMsgId);
        // default on receiving setting RESPONSE status is false
        contentValues.Put(CDataBaseConst::COL_SETTING_RESPONSE_STATUS, false);


        CDataBaseFacade* pDBFacade = CDataBaseFacade::GetInstance();
        std::vector<std::string> vecProjection;
        vecProjection.push_back(CDataBaseConst::COL_SETTING_ID);
        vecProjection.push_back(CDataBaseConst::COL_SETTING_ENUM);
        vecProjection.push_back(CDataBaseConst::COL_SETTING_VALUE);
        vecProjection.push_back(CDataBaseConst::COL_SETTING_RESPONSE_STATUS);
        vecProjection.push_back(CDataBaseConst::COL_SETTING_CORR_ID);
        vecProjection.push_back(CDataBaseConst::COL_SETTING_SRC_ISDEVICE);

        std::string strSelection;
        if (bIsFromDevice) 
        {
            contentValues.Put(CDataBaseConst::COL_SETTING_SRC_ISDEVICE, true);
            strSelection = CDataBaseConst::COL_SETTING_ID + " = '" + 
                    strServiceId + "' AND " + CDataBaseConst::COL_SETTING_ENUM +
                    " = '" + strObjSettingEnum +"' AND " +
                    CDataBaseConst::COL_SETTING_SRC_ISDEVICE+ "='true'";

        } 
        else 
        {
            contentValues.Put(CDataBaseConst::COL_SETTING_SRC_ISDEVICE, false);
            strSelection = CDataBaseConst::COL_SETTING_ID + " = '" + 
                    strServiceId + "' AND " + CDataBaseConst::COL_SETTING_ENUM +
                    " = '" + strObjSettingEnum + "' AND " + 
                    CDataBaseConst::COL_SETTING_SRC_ISDEVICE+ "='false'";
        }



        CCursor *pCursor = 
                 pDBFacade->Query(CDataBaseConst::TABLE_IGNITE_SERVICE_SETTINGS,
                                  vecProjection, strSelection);
        long nId = -1;
        if (pCursor) 
        {
            if (pCursor->MoveToFirst())
            {
                nId = pCursor->GetLong(pCursor->
                              GetColumnIndex(CDataBaseConst::COL_SETTING_ENUM));
            }
            delete pCursor;
        }

        if (-1 == nId)
        {
            HCPLOG_T << "New setting inserted in dB";
            bSuccess = (pDBFacade->Insert(
                                  CDataBaseConst::TABLE_IGNITE_SERVICE_SETTINGS, 
                                  &contentValues) > 0);
        }
        else
        {
            HCPLOG_T << "Setting is updated in dB" << nId;
            bSuccess = pDBFacade->Update(
                                  CDataBaseConst::TABLE_IGNITE_SERVICE_SETTINGS,
                                  &contentValues, strSelection);
        }

        if(!bSuccess)
        {
            return bSuccess;
        }
    }
    return bSuccess;
}

bool CServiceSettingsStore::StoreSettings(std::string strServiceId,
                                          std::string strSettingEnum,
                                          const ic_utils::Json::Value &rjsonSettings)
{
    bool bSuccess = false;
    ic_utils::Json::FastWriter jsonWriter;
    std::string strSettingsJsonString = jsonWriter.write(rjsonSettings);

    HCPLOG_T << "ServiceId-" << strServiceId
             << " ,settingEnum-" << strSettingEnum
             << " ,settingsJsonString-" << strSettingsJsonString;

    CContentValues contentValues;
    contentValues.Put(CDataBaseConst::COL_SETTING_ID, strServiceId);
    contentValues.Put(CDataBaseConst::COL_SETTING_ENUM, strSettingEnum);
    contentValues.Put(CDataBaseConst::COL_SETTING_VALUE, strSettingsJsonString);
    contentValues.Put(CDataBaseConst::COL_SETTING_CORR_ID,
                                        CDataBaseConst::SETTING_NOT_APPLICABLE);

    CDataBaseFacade* pDBFacade = CDataBaseFacade::GetInstance();
    std::vector<std::string> vecProjection;
    vecProjection.push_back(CDataBaseConst::COL_SETTING_ID);
    vecProjection.push_back(CDataBaseConst::COL_SETTING_ENUM);
    vecProjection.push_back(CDataBaseConst::COL_SETTING_VALUE);
    vecProjection.push_back(CDataBaseConst::COL_SETTING_CORR_ID);

    // If settingEnum present override its value
    std::string strSelection = CDataBaseConst::COL_SETTING_ENUM + 
                                                  " = '" + strSettingEnum + "'";
    CCursor *pCursor = pDBFacade->Query(
                                  CDataBaseConst::TABLE_IGNITE_SERVICE_SETTINGS,
                                  vecProjection, strSelection);
    long lId = -1;
    if (pCursor)
    {
        if (pCursor->MoveToFirst())
        {
            lId = pCursor->GetLong(pCursor->GetColumnIndex(
                                             CDataBaseConst::COL_SETTING_ENUM));
        }
        delete pCursor;
    }

    if (-1 == lId)
    {
        HCPLOG_T << "New setting inserted in dB";
        bSuccess = (pDBFacade->Insert(
                                  CDataBaseConst::TABLE_IGNITE_SERVICE_SETTINGS,
                                  &contentValues) > 0);
    }
    else
    {
        HCPLOG_T << "Setting is updated in dB" << lId;
        bSuccess = pDBFacade->Update(
                                  CDataBaseConst::TABLE_IGNITE_SERVICE_SETTINGS,
                                  &contentValues, strSelection);
    }
    return bSuccess;
}

bool CServiceSettingsStore::UpdateResponseReceivedForSettings(
                                                       std::string strServiceId,
                                                     std::string strSettingEnum)
{
    HCPLOG_T << "Updating Settings Response for " << strSettingEnum;
    std::string strSelection = CDataBaseConst::COL_SETTING_ID + " = '" + 
                               strServiceId + "' AND " + 
                               CDataBaseConst::COL_SETTING_ENUM + " = '" + 
                               strSettingEnum + "' AND " + 
                               CDataBaseConst::COL_SETTING_SRC_ISDEVICE +
                               "='false'";
    CContentValues data;
    data.Put(CDataBaseConst::COL_SETTING_RESPONSE_STATUS, true);
    bool bStatus = CDataBaseFacade::GetInstance()->Update(
                                  CDataBaseConst::TABLE_IGNITE_SERVICE_SETTINGS,
                                  &data, strSelection);
    return bStatus;
}

bool CServiceSettingsStore::UpdateAcknowledgementReceivedFromCloud(
                                                       std::string strServiceId,
                                                   std::string strCorrelationId,
                                                       std::string strResp) 
{
    HCPLOG_T << "Updating Settings Response for " << strCorrelationId;
    std::string strSelection = CDataBaseConst::COL_SETTING_ID + " = '" + 
                               strServiceId + "' AND " +
                               CDataBaseConst::COL_SETTING_CORR_ID + " = '" +
                               strCorrelationId + "' AND " +
                               CDataBaseConst::COL_SETTING_SRC_ISDEVICE + 
                               " ='true'";

    CContentValues data;
    if (strResp == STATUS_SUCCESS) 
    {
        HCPLOG_T << "Updating---- STATUS_SUCCESS ";
        data.Put(CDataBaseConst::COL_SETTING_RESPONSE_STATUS, true);
    } 
    else 
    {
        data.Put(CDataBaseConst::COL_SETTING_RESPONSE_STATUS, false);
        HCPLOG_T << "Updating--- STATUS_FAILURE ";
    }
    bool bStatus = CDataBaseFacade::GetInstance()->Update(
                                  CDataBaseConst::TABLE_IGNITE_SERVICE_SETTINGS,
                                  &data, strSelection);

    return bStatus;
}

std::string CServiceSettingsStore::GetCorrIdForSettings(std::string strServiceId,
                                                     std::string strSettingEnum)
{
    HCPLOG_METHOD();
    std::string strCorrId;
    strCorrId = Get(CORR_ID, strServiceId,strSettingEnum);
    HCPLOG_T << " Returning corrId " << strCorrId << " for "<< strSettingEnum;
    return strCorrId;
}

std::string CServiceSettingsStore::GetCorrIdForSettings(std::string strServiceId,
                                                        bool bIsFromDevice)
{
    HCPLOG_METHOD();
    std::string strCorrId;
    strCorrId = GetFromSource(CORR_ID, strServiceId, bIsFromDevice);
    HCPLOG_T << " Returning corrId " << strCorrId 
             << " for isFromDevice =" << bIsFromDevice;
    return strCorrId;
}

std::string CServiceSettingsStore::GetValueForSettings(std::string strServiceId,
                                                     std::string strSettingEnum)
{
    HCPLOG_METHOD();
    std::string strValue;
    strValue = Get(VALUE, strServiceId, strSettingEnum);
    HCPLOG_T << " Returning value " << strValue << " for " << strSettingEnum;
    return strValue;
}

std::vector<std::string> CServiceSettingsStore::GetValueForSettings(
                                                       std::string strServiceId,
                                                   std::string strCorrelationId,
                                                   bool bIsFromDevice) 
{
    HCPLOG_METHOD();
    std::vector<std::string> vecValues;
    vecValues = Get(VALUE, strServiceId, strCorrelationId, bIsFromDevice);
    HCPLOG_T << " Returning value Vector for strServiceId= " << strServiceId
             << " for " << strCorrelationId
             << "for isFromDevice = " << bIsFromDevice;
    return vecValues;
}

bool CServiceSettingsStore::GetStatusForSettings(std::string strServiceId,
                                                 bool bIsFromDevice) 
{
    HCPLOG_METHOD();
    bool bValue;
    bValue = GetBoolItem(SETTING_RESPONSE_STATUS, strServiceId, bIsFromDevice);
    HCPLOG_T << " Returning value " << bValue << " for " << bIsFromDevice;
    return bValue;
}

std::string CServiceSettingsStore::GetServiceIdFromCorId(
                                                   std::string strCorrelationId, 
                                                   bool bIsFromDevice) {
    HCPLOG_METHOD();
    std::string strValue;
    strValue = Get(SETTING_ID, strCorrelationId, bIsFromDevice);
    HCPLOG_T << " Returning value " << strValue << " for " << strCorrelationId;
    return strValue;
}

std::string CServiceSettingsStore::Get(std::string strKey, 
                                       std::string strServiceId,
                                       std::string strSettingEnum)
{
    HCPLOG_METHOD();
    std::string strColumn;
    if (strKey == CORR_ID)
    {
        strColumn = CDataBaseConst::COL_SETTING_CORR_ID;
    } 
    else if (strKey == VALUE) 
    {
        strColumn = CDataBaseConst::COL_SETTING_VALUE;
    }
    else 
    {
        // Do nothing
    }
    std::vector<std::string> vecProjection;
    vecProjection.push_back(CDataBaseConst::COL_SETTING_ID);
    vecProjection.push_back(CDataBaseConst::COL_SETTING_ENUM);
    vecProjection.push_back(CDataBaseConst::COL_SETTING_VALUE);
    vecProjection.push_back(CDataBaseConst::COL_SETTING_RESPONSE_STATUS);
    vecProjection.push_back(CDataBaseConst::COL_SETTING_CORR_ID);
    vecProjection.push_back(CDataBaseConst::COL_SETTING_SRC_ISDEVICE);

    std::string strSelection = CDataBaseConst::COL_SETTING_ID + " = '" + 
                                      strServiceId + "' AND " + 
                                      CDataBaseConst::COL_SETTING_ENUM +
                                      " = '" + strSettingEnum + "' AND " +
                                      CDataBaseConst::COL_SETTING_SRC_ISDEVICE +
                                      "='false'";

    CCursor *pCursor = CDataBaseFacade::GetInstance()->Query(
                                  CDataBaseConst::TABLE_IGNITE_SERVICE_SETTINGS,
                                  vecProjection, strSelection);

    std::string strVal;
    if (pCursor) 
    {
        if (pCursor->MoveToFirst()) 
        {
            strVal = pCursor->GetString(pCursor->GetColumnIndex(strColumn));
        }
        delete pCursor;
    }

    HCPLOG_T << " returning " << strVal;
    return strVal;
}
bool CServiceSettingsStore::ClearSettings(std::string strServiceId, 
                                          std::string strSettingEnum)
{
    bool bSettingCleared = false;
    std::vector<std::string> vecProjection;
    vecProjection.push_back(CDataBaseConst::COL_SETTING_ID);
    vecProjection.push_back(CDataBaseConst::COL_SETTING_ENUM);
    vecProjection.push_back(CDataBaseConst::COL_SETTING_VALUE);
    vecProjection.push_back(CDataBaseConst::COL_SETTING_CORR_ID);

    std::string strSelection = "";
    if (!strSettingEnum.empty()) 
    {
        strSelection =  CDataBaseConst::COL_SETTING_ID + " = '" + strServiceId +
                                   "' AND " + CDataBaseConst::COL_SETTING_ENUM +
                                   " = '"+strSettingEnum + "'";
    }
    else 
    {
        strSelection =  CDataBaseConst::COL_SETTING_ID + 
                             " = '" + strServiceId + "'";
    }


    CDataBaseFacade* pDBFacade = CDataBaseFacade::GetInstance();

    bSettingCleared = pDBFacade->Remove(
                                  CDataBaseConst::TABLE_IGNITE_SERVICE_SETTINGS,
                                  strSelection);
    return bSettingCleared;
}

bool CServiceSettingsStore::GetAllConfigUpdateSettings(
                     std::map<std::string,ic_utils::Json::Value> &mapConfigUpdateMap)
{
    HCPLOG_METHOD();
    mapConfigUpdateMap.clear();
    CIgniteConfig *pConfig = CIgniteConfig::GetInstance();
    
    // read settings updated from cloud
    std::string strServiceId = pConfig->GetConfigUpdateSourceInString(
                                EconfigUpdateSource::eCLOUD_CONFIG_UPDATE);
    bool bCloudSettingAvailable = GetConfigUpdateSettings(strServiceId, 
                                                            mapConfigUpdateMap);

    return bCloudSettingAvailable;
}

bool CServiceSettingsStore::GetConfigUpdateSettings(std::string strServiceId,
                    std::map<std::string, ic_utils::Json::Value> &mapConfigUpdateMap)
{
    HCPLOG_METHOD();
    bool bConfigSettingAvailable = false;
    std::string strColumn = CDataBaseConst::COL_SETTING_VALUE;
    std::vector<std::string> vecProjection;
    vecProjection.push_back(CDataBaseConst::COL_SETTING_ID);
    vecProjection.push_back(CDataBaseConst::COL_SETTING_ENUM);
    vecProjection.push_back(CDataBaseConst::COL_SETTING_VALUE);
    vecProjection.push_back(CDataBaseConst::COL_SETTING_CORR_ID);

    std::string strSelection = CDataBaseConst::COL_SETTING_ID + " = '" + 
                                                             strServiceId + "'";

    CCursor *pCursor = CDataBaseFacade::GetInstance()->Query(
                                  CDataBaseConst::TABLE_IGNITE_SERVICE_SETTINGS,
                                  vecProjection, strSelection);

    ic_utils::Json::Value jsonValues = ic_utils::Json::Value::nullRef;

    if (!pCursor)
    {
        // No configUpdate was received
        return bConfigSettingAvailable;
    }

    if (pCursor->MoveToFirst()) 
    {
        std::string strVal;
        bConfigSettingAvailable = true;
        do 
        {
            strVal = pCursor->GetString(pCursor->GetColumnIndex(strColumn));
            HCPLOG_T << " Adding configuration: " << strVal;
            ic_utils::Json::Reader jsonReader;
            ic_utils::Json::Value jsonSettingValue;
            if (!jsonReader.parse(strVal, jsonSettingValue))
            {
                HCPLOG_E << "Parsing error";
                continue;
            }

            for (ic_utils::Json::ValueIterator startItr = 
                        jsonSettingValue.begin();
                        startItr != jsonSettingValue.end(); startItr++) 
            {
                std::string strKey = startItr.key().asString();
                std::map<std::string,ic_utils::Json::Value>::iterator iter =
                                        mapConfigUpdateMap.find(strKey);
                if (iter != mapConfigUpdateMap.end()) 
                {
                    mapConfigUpdateMap.erase(strKey);
                }
                mapConfigUpdateMap.insert(
                                        std::make_pair(strKey,*startItr));
            }
        } while (pCursor->MoveToNext());
    }
    delete pCursor;

    return bConfigSettingAvailable;
}

bool CServiceSettingsStore::GetBoolItem(std::string strKey, 
                                        std::string strServiceId,
                                        bool bIsFromDevice) 
{
    HCPLOG_METHOD();
    std::string strColumn;
    if (strKey == SETTING_RESPONSE_STATUS) 
    {
        strColumn = CDataBaseConst::COL_SETTING_RESPONSE_STATUS;
    } 
    else if (strKey == SETTING_SRC_ISDEVICE) 
    {
        strColumn = CDataBaseConst::COL_SETTING_SRC_ISDEVICE;
    }
    else
    {
        // Do nothing
    }
    std::vector<std::string> vecProjection;
    vecProjection.push_back(CDataBaseConst::COL_SETTING_ID);
    vecProjection.push_back(CDataBaseConst::COL_SETTING_ENUM);
    vecProjection.push_back(CDataBaseConst::COL_SETTING_VALUE);
    vecProjection.push_back(CDataBaseConst::COL_SETTING_RESPONSE_STATUS);
    vecProjection.push_back(CDataBaseConst::COL_SETTING_CORR_ID);
    vecProjection.push_back(CDataBaseConst::COL_SETTING_SRC_ISDEVICE);
    std::string strSelection;
    if (bIsFromDevice) 
    {
        strSelection = CDataBaseConst::COL_SETTING_ID + " = '" + 
                         strServiceId + "' AND " +
                         CDataBaseConst::COL_SETTING_SRC_ISDEVICE + " = 'true'";
    } 
    else 
    {
        strSelection = CDataBaseConst::COL_SETTING_ID + " = '" + 
                        strServiceId + "' AND " +
                        CDataBaseConst::COL_SETTING_SRC_ISDEVICE + " = 'false'";
    }

    CCursor *pCursor = CDataBaseFacade::GetInstance()->Query(
                                  CDataBaseConst::TABLE_IGNITE_SERVICE_SETTINGS,
                                  vecProjection, strSelection);

    bool bVal = false;
    if (pCursor)
    {
        if (pCursor->MoveToFirst()) 
        {
            bVal = pCursor->GetBool(pCursor->GetColumnIndex(strColumn));

        }
        delete pCursor;
    }
    HCPLOG_T << " returning " << bVal;
    return bVal;
}

std::string CServiceSettingsStore::Get(std::string strKey, 
                                       std::string strCorrelationId, 
                                       bool bIsFromDevice) 
{
    HCPLOG_METHOD();
    std::string strColumn;
    if (strKey == SETTING_ID) 
    {
        strColumn = CDataBaseConst::COL_SETTING_ID;
    } 
    else if (strKey == SETTING_ENUM)
    {
        strColumn = CDataBaseConst::COL_SETTING_ENUM;
    }
    else
    {
        // Do nothing
    }
    std::vector<std::string> vecProjection;
    vecProjection.push_back(CDataBaseConst::COL_SETTING_ID);
    vecProjection.push_back(CDataBaseConst::COL_SETTING_ENUM);
    vecProjection.push_back(CDataBaseConst::COL_SETTING_VALUE);
    vecProjection.push_back(CDataBaseConst::COL_SETTING_RESPONSE_STATUS);
    vecProjection.push_back(CDataBaseConst::COL_SETTING_CORR_ID);
    vecProjection.push_back(CDataBaseConst::COL_SETTING_SRC_ISDEVICE);

    std::string strSelection;
    if (bIsFromDevice) 
    {
        strSelection = CDataBaseConst::COL_SETTING_CORR_ID + " = '" + 
                         strCorrelationId + "' AND " +
                         CDataBaseConst::COL_SETTING_SRC_ISDEVICE + " = 'true'";
    } 
    else 
    {
        strSelection = CDataBaseConst::COL_SETTING_CORR_ID + " = '" + 
                        strCorrelationId + "' AND " +
                        CDataBaseConst::COL_SETTING_SRC_ISDEVICE + " = 'false'";
    }

    CCursor *pCursor = CDataBaseFacade::GetInstance()->Query(
                                  CDataBaseConst::TABLE_IGNITE_SERVICE_SETTINGS,
                                  vecProjection, strSelection);

    std::string strVal;
    if (pCursor) 
    {
        if (pCursor->MoveToFirst()) 
        {
            strVal = pCursor->GetString(pCursor->GetColumnIndex(strColumn));
        }
        delete pCursor;
    }
    HCPLOG_T << " returning " << strVal;
    return strVal;
}

std::string CServiceSettingsStore::GetFromSource(std::string strKey, 
                                                 std::string strServiceId, 
                                                 bool bIsFromDevice) 
{
    HCPLOG_METHOD();
    std::string strColumn;
    if (strKey == CORR_ID) 
    {
        strColumn = CDataBaseConst::COL_SETTING_CORR_ID;
    } 
    else if (strKey == SETTING_ENUM) 
    {
        strColumn = CDataBaseConst::COL_SETTING_ENUM;
    }
    else
    {
        // Do nothing
    }
    std::vector<std::string> vecProjection;
    vecProjection.push_back(CDataBaseConst::COL_SETTING_ID);
    vecProjection.push_back(CDataBaseConst::COL_SETTING_ENUM);
    vecProjection.push_back(CDataBaseConst::COL_SETTING_VALUE);
    vecProjection.push_back(CDataBaseConst::COL_SETTING_RESPONSE_STATUS);
    vecProjection.push_back(CDataBaseConst::COL_SETTING_CORR_ID);
    vecProjection.push_back(CDataBaseConst::COL_SETTING_SRC_ISDEVICE);

    std::string strSelection;
    if (bIsFromDevice) 
    {
        strSelection = CDataBaseConst::COL_SETTING_ID + " = '" + 
                         strServiceId + "' AND " +
                         CDataBaseConst::COL_SETTING_SRC_ISDEVICE + " = 'true'";
    } else 
    {
        strSelection = CDataBaseConst::COL_SETTING_ID + " = '" + 
                        strServiceId + "' AND " +
                        CDataBaseConst::COL_SETTING_SRC_ISDEVICE + " = 'false'";
    }

    CCursor *pCursor = CDataBaseFacade::GetInstance()->Query(
                                  CDataBaseConst::TABLE_IGNITE_SERVICE_SETTINGS,
                                  vecProjection, strSelection);

    std::string strVal;
    if (pCursor) 
    {
        if (pCursor->MoveToFirst()) 
        {
            strVal = pCursor->GetString(pCursor->GetColumnIndex(strColumn));
        }
        delete pCursor;
    }
    HCPLOG_T << " returning " << strVal;
    return strVal;
}

std::vector<std::string> CServiceSettingsStore::Get(std::string strKey, 
                                                    std::string strServiceId, 
                                                    std::string strCorrelationId,
                                                    bool bIsFromDevice) 
{
    HCPLOG_METHOD();
    std::string strColumn;
    if (strKey == VALUE) 
    {
        strColumn = CDataBaseConst::COL_SETTING_VALUE;
    } 
    else if (strKey == SETTING_ENUM) 
    {
        strColumn = CDataBaseConst::COL_SETTING_ENUM;
    }
    else
    {
        // Do nothing
    }

    std::vector<std::string> vecProjection;
    vecProjection.push_back(CDataBaseConst::COL_SETTING_ID);
    vecProjection.push_back(CDataBaseConst::COL_SETTING_ENUM);
    vecProjection.push_back(CDataBaseConst::COL_SETTING_VALUE);
    vecProjection.push_back(CDataBaseConst::COL_SETTING_RESPONSE_STATUS);
    vecProjection.push_back(CDataBaseConst::COL_SETTING_CORR_ID);
    vecProjection.push_back(CDataBaseConst::COL_SETTING_SRC_ISDEVICE);

    std::string strSelection;
    if (bIsFromDevice) 
    {
        strSelection = CDataBaseConst::COL_SETTING_ID + " = '" + strServiceId + 
                               "' AND " + CDataBaseConst::COL_SETTING_CORR_ID + 
                               " = '" + strCorrelationId + "' AND " +
                               CDataBaseConst::COL_SETTING_SRC_ISDEVICE + 
                               " = 'true'";
    } 
    else 
    {
        strSelection = CDataBaseConst::COL_SETTING_ID + " = '" + strServiceId + 
                               "' AND " + CDataBaseConst::COL_SETTING_CORR_ID + 
                               " = '" + strCorrelationId + "' AND " +
                               CDataBaseConst::COL_SETTING_SRC_ISDEVICE + 
                               " = 'false'";
    }

    CCursor *pCursor = CDataBaseFacade::GetInstance()->Query(
                                  CDataBaseConst::TABLE_IGNITE_SERVICE_SETTINGS, 
                                  vecProjection, strSelection);

    std::string strVal;
    std::vector<std::string> vecValues;
    if (pCursor) 
    {
        if (pCursor->MoveToFirst()) 
        {
            do 
            {
                strVal = pCursor->GetString(pCursor->GetColumnIndex(strColumn));
                vecValues.push_back(strVal);
            } while (pCursor->MoveToNext());

        }
        delete pCursor;
    }
    HCPLOG_T << " returning " << strVal;
    return vecValues;
}
} /* namespace ic_core */
