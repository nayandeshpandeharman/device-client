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
* \file CServiceSettingsStore.h
*
* \brief This class/module provide methods to store service settings 
* in the database
*******************************************************************************
*/

#ifndef CSERVICE_SETTINGS_STORE_H
#define CSERVICE_SETTINGS_STORE_H

#include <string>
#include "jsoncpp/json.h"

namespace ic_core 
{
/**
 * This class provide methods to store service settings in the database
 */
class CServiceSettingsStore 
{
public:
    /**
     * Method to get instance of CServiceSettingsStore class
     * @param void
     * @return instance of CServiceSettingsStore class
     */
    static CServiceSettingsStore* GetInstance();

    /**
     * Method to store/update settings in the db based on input parameter
     * @param[in] strServiceId string containing service id
     * @param[in] strSettingEnum string containing service enum value
     * @param[in] rjsonSettingsValue JSON object containing settings value
     * @return true if successfully stored/updated settings in db, 
     * false otherwise
     */
    bool StoreSettings(std::string strServiceId, std::string strSettingEnum, 
                      const ic_utils::Json::Value &rjsonSettingsValue);

    /**
     * Method to store/update settings in the db based on input parameter
     * @param[in] strServiceId string containing service id
     * @param[in] rjsonSettingsValue JSON object containing settings value
     * @param[in] strMsgId string containing message id
     * @param[in] bIsFromDevice is from device status
     * @return true if successfully stored/updated settings in db, 
     * false otherwise
     */
    bool StoreSettings(std::string strServiceId, 
                       ic_utils::Json::Value &rjsonSettingsValue, 
                       std::string strMsgId,
                       bool bIsFromDevice);

    /**
     * Method to updates response field received for setting response from 
     * device based on input parameter
     * @param[in] strServiceId string containing service id
     * @param[in] strSettingEnum string containing service enum value
     * @return true if successfully updated response in db, false otherwise
     */
    bool UpdateResponseReceivedForSettings(std::string strServiceId, 
                                           std::string strSettingEnum);

    /**
     * Method to get corrId based on input parameter
     * @param[in] strServiceId string containing service id
     * @param[in] bIsFromDevice is from device status
     * @return returns the correlationId from db; if not, empty string will be 
     * returned
     */
    std::string GetCorrIdForSettings(std::string strServiceId, 
                                     bool bIsFromDevice);

    /**
     * Method to get corrId based on input parameter
     * @param[in] strServiceId string containing service id
     * @param[in] strSettingEnum string containing service enum value
     * @return returns the correlationId from db; if not, empty string will be 
     * returned
     */
    std::string GetCorrIdForSettings(std::string strServiceId,
                                     std::string strSettingEnum);

    /**
     * Method to get value of setting enum based on input parameter
     * @param[in] strServiceId string containing service id
     * @param[in] strSettingEnum string containing service enum value
     * @return returns the settings value from db; if not, empty string will be 
     * returned
     */
    std::string GetValueForSettings(std::string strServiceId,
                                    std::string strSettingEnum);

    /**
     * Method to updates ack response received from Cloud based on input 
     * parameter
     * @param[in] strServiceId string containing service id
     * @param[in] strCorrelationId string containing correlation id
     * @param[in] strResp string containing response
     * @return true if successfully updated ack in db, false otherwise
     */
    bool UpdateAcknowledgementReceivedFromCloud(std::string strServiceId,
                                                std::string strCorrelationId,
                                                std::string strResp);

    /**
     * Method to get serviceId based on input parameter
     * @param[in] strCorrelationId string containing correlation id
     * @param[in] bIsFromDevice is from device status
     * @return returns the serviceId for corId from db; if not, empty string 
     * will be returned
     */
    std::string GetServiceIdFromCorId(std::string strCorrelationId,
                                      bool bIsFromDevice);

    /**
     * Method to get status of settings based on input parameter
     * @param[in] strServiceId string containing service id
     * @param[in] bIsFromDevice is from device status
     * @return true if successfully get settings status from db, false otherwise
     */
    bool GetStatusForSettings(std::string strServiceId, bool bIsFromDevice);

    /**
     * Method to get value for settings based on input parameter
     * @param[in] strServiceId string containing service id
     * @param[in] strCorrelationId string containing correlationId
     * @param[in] bIsFromDevice is from device status
     * @return returns the vector of settings value from db; if not, empty 
     * vector will be returned
     */
    std::vector<std::string> GetValueForSettings(std::string strServiceId,
                                                 std::string strCorrelationId,
                                                 bool bIsFromDevice);

    /**
     * Method to get all config update settings based on input parameter
     * @param[out] rmapConfigUpdaterMap map of settings
     * @return true if settings are present in db, false otherwise
     */
    bool GetAllConfigUpdateSettings(std::map<std::string, ic_utils::Json::Value> &
                                                          rmapConfigUpdaterMap);

    /**
     * Method to clear the setting from table
     * @param[in] strServiceId string containing service id
     * @param[in] strSettingEnum string containing service enum value
     * @return true if settings are cleared from db, false otherwise
     */
    bool ClearSettings(std::string strServiceId,std::string strSettingEnum = "");

    /**
     * Method to get config update settings based on input parameter
     * @param[in] strServiceId string containing service id
     * @param[out] rmapConfigUpdaterMap map of settings
     * @return true if successfully get the settings from db, false otherwise
     */
    bool GetConfigUpdateSettings(std::string strServiceId,
                  std::map<std::string,ic_utils::Json::Value> &rmapConfigUpdaterMap);

    #ifdef IC_UNIT_TEST
        friend class ServiceSettingsStoreTest;
    #endif

private:
    /**
     * Default no-argument constructor.
     */
    CServiceSettingsStore();

    /**
     * Method to get string value based on input parameter
     * @param[in] strServiceId string containing service id
     * @param[in] strSettingEnum string containing service enum value
     * @return returns the settings value from db; if not, empty string will be 
     * returned
     */
    std::string Get(std::string key, std::string strServiceId,
                    std::string strSettingEnum);

    /**
     * Method to get vector value based on input parameter
     * @param[in] strKey string containing key value
     * @param[in] strServiceId string containing service id
     * @param[in] strCorrelationId string containing correlation id
     * @param[in] bIsFromDevice is from device status
     * @return returns the vector of settings value from db; if not, empty 
     * vector will be returned
     */
    std::vector<std::string> Get(std::string strKey, std::string strServiceId,
                              std::string strCorrelationId, bool bIsFromDevice);

    /**
     * Method to get vector value based on input parameter
     * @param[in] strKey string containing key value
     * @param[in] strCorrelationId string containing correlation id
     * @param[in] bIsFromDevice is from device status
     * @return returns the settings value from db; if not, empty string will be 
     * returned
     */
    std::string Get(std::string strKey, std::string strCorrelationId,
                    bool bIsFromDevice);

    /**
     * Method to get settings from source based on input parameter
     * @param[in] strKey string containing key value
     * @param[in] strServiceId string containing service id
     * @param[in] bIsFromDevice is from device status
     * @return returns the source from settings from db; if not, empty string
     * will be returned
     */
    std::string GetFromSource(std::string strKey, std::string strServiceId,
                              bool bIsFromDevice);

    /**
     * Method to get boolean status of settings available in the database based 
     * on input parameter
     * @param[in] strKey string containing key value
     * @param[in] strServiceId string containing service id
     * @param[in] bIsFromDevice source is from device status
     * @return true if successfully get the column index from db, false 
     * otherwise
     */
    bool GetBoolItem(std::string strKey, std::string strServiceId,
                     bool bIsFromDevice);
};
} /* namespace ic_core */
#endif //CSERVICE_SETTINGS_STORE_H
