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

#include "CPersistancyAndStateHandler.h"
#include "db/CLocalConfig.h"
#include "CIgniteLog.h"
#include "CIgniteConfig.h"
#include "db/CDataBaseFacade.h"
#include "net/CIgniteMQTTClient.h"

//! Macro for 'CPersistancyAndStateHandler' string
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CPersistancyAndStateHandler"

namespace ic_bl
{
CPersistancyAndStateHandler::CPersistancyAndStateHandler()
{

}

CPersistancyAndStateHandler* CPersistancyAndStateHandler::GetInstance()
{
    static CPersistancyAndStateHandler instance;
    return &instance;
}

bool CPersistancyAndStateHandler::PersistActivationStatus(const std::string 
                                                        &rstrActivationDetails)
{
    const std::string &rstrItemName = GetKeyFromId(ic_core::IC_QUERY_ID::
                                                            eACTIVATION_STATUS);
    return ic_core::CLocalConfig::GetInstance()->Set(rstrItemName,
                                                         rstrActivationDetails);
}

bool CPersistancyAndStateHandler::PersistICstate(enum ic_core::IC_STATE eState)
{
    const std::string &rstrItemname = GetKeyFromId(ic_core::IC_QUERY_ID::
                                                            eIC_CURRENT_STATE);
    ic_utils::Json::Value jsonStateDetails = ic_utils::Json::Value::nullRef;
    jsonStateDetails["state"] = eState;

    ic_utils::Json::FastWriter jsonWriter;
    std::string strPersist = jsonWriter.write(jsonStateDetails);

    return ic_core::CLocalConfig::GetInstance()->Set(rstrItemname, strPersist);
}

bool CPersistancyAndStateHandler::ClearActivationStatus()
{
  return ic_core::CLocalConfig::GetInstance()->Remove(GetKeyFromId(
                                    ic_core::IC_QUERY_ID::eACTIVATION_STATUS));
}

std::string CPersistancyAndStateHandler:: GetKeyFromId(
                                            enum ic_core::IC_QUERY_ID eQueryId)
{
  switch (eQueryId)
  {
    case ic_core::IC_QUERY_ID::eACTIVATION_STATUS: 
      return KEY_ACTIVATION_STATUS;
    case ic_core::IC_QUERY_ID::eIC_CURRENT_STATE: 
      return KEY_IC_RUNNING_STATUS;
    case ic_core::IC_QUERY_ID::eDEVICE_ID:
      return KEY_LOGIN;
    case ic_core::IC_QUERY_ID::eMQTT_CONN_STATUS:
      return KEY_MQTT_CONN_STATUS;
    case ic_core::IC_QUERY_ID::eDB_SIZE:
      return KEY_DATABASE_SIZE;
    case ic_core::IC_QUERY_ID::eLAST_SUCCESSFUL_BATCH_UPLOAD:
      return KEY_UPLOAD_SERVICE_LAST_SUCCESSFUL_UPLOAD;
    default:
      HCPLOG_W << "Invalid ID:" << eQueryId;
      return KEY_INVALID_ID;
  }
}

ic_utils::Json::Value CPersistancyAndStateHandler::GetICParam(
                                                  ic_core::IC_QUERY_ID eQueryId)
{
    ic_utils::Json::Value jsonResult = ic_utils::Json::Value::nullRef;
    std::string strDetails = "" ;
    switch (eQueryId)
    {
        case ic_core::IC_QUERY_ID::eACTIVATION_STATUS:
        {
            strDetails = ic_core::CLocalConfig::GetInstance()->
                Get(GetKeyFromId(ic_core::IC_QUERY_ID::eACTIVATION_STATUS));

            if(strDetails.empty())
            {
              jsonResult[KEY_STATUS] = ic_core::IC_ACTIVATION_STATUS::
                                                              eNOT_ACTIVATED;
              jsonResult[KEY_VIN] = ic_core::CIgniteClient::
                                GetProductImpl()->GetActivationQualifierID();
            }
            break;
        }
        case ic_core::IC_QUERY_ID::eIC_CURRENT_STATE:
        {
            strDetails = ic_core::CLocalConfig::GetInstance()->
                Get(GetKeyFromId(ic_core::IC_QUERY_ID::eIC_CURRENT_STATE));
            break;
        }
        case ic_core::IC_QUERY_ID::eDB_SIZE:
        {
            jsonResult[KEY_DB_SIZE] = (unsigned long long)
                        ic_core::CDataBaseFacade::GetInstance()->GetSize();
            jsonResult[KEY_MAX_DB_SIZE] = ic_core::CIgniteConfig::
                          GetInstance()->GetInt("DAM.Database.dbSizeLimit");
            break;
        }
        case ic_core::IC_QUERY_ID::eMQTT_CONN_STATUS:
        {
            jsonResult = CIgniteMQTTClient::GetInstance()->
                                                GetCurrentConnectionState();
            break;
        }
        case ic_core::IC_QUERY_ID::eDEVICE_ID:
        {
            std::string strDeviceId = ic_core::CLocalConfig::GetInstance()->
                Get(GetKeyFromId(ic_core::IC_QUERY_ID::eDEVICE_ID));
            jsonResult[KEY_DEVICE_ID] = strDeviceId;
            break;
        }
        case ic_core::IC_QUERY_ID::eLAST_SUCCESSFUL_BATCH_UPLOAD:
        {
            std::string strSuccessfullUpload = ic_core::CLocalConfig::
            GetInstance()->
            Get(GetKeyFromId(ic_core::IC_QUERY_ID::
                                            eLAST_SUCCESSFUL_BATCH_UPLOAD));
            jsonResult[KEY_LAST_SUCCESSFUL_UPLOAD] = strSuccessfullUpload;
            break;
        }
        default:
          break;
    }

    ic_utils::Json::Reader jsonReader;

    if ((!strDetails.empty()) && (!jsonReader.parse(strDetails,jsonResult)))
    {
        HCPLOG_E << "could not parse jsonString!";
        jsonResult = ic_utils::Json::Value::nullRef;
    }
    return jsonResult;
}

CPersistancyAndStateHandler::~CPersistancyAndStateHandler()
{
     
}
}
