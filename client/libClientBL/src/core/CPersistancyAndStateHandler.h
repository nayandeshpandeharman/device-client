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
* \file CPersistancyAndStateHandler.h
*
* \brief  This class provides methods to persists and fetches the data from
* the local config
********************************************************************************
*/

#ifndef CPERSISTANCY_AND_STATE_HANDLER_H
#define CPERSISTANCY_AND_STATE_HANDLER_H

#include <string>
#include "CIgniteClient.h"

//! Constant key for 'ICP.ActivationStatus' string
static const std::string KEY_ACTIVATION_STATUS = "ICP.ActivationStatus";

//! Constant key for 'ICP.ICRunningStatus' string
static const std::string KEY_IC_RUNNING_STATUS = "ICP.ICRunningStatus";

//! Constant key for 'login' string
static const std::string KEY_LOGIN = "login";

//! Constant key for 'UploaderService.LastSuccessfulUpload' string
static const std::string KEY_UPLOAD_SERVICE_LAST_SUCCESSFUL_UPLOAD = 
                                        "UploaderService.LastSuccessfulUpload";

//! Constant key for 'InvalidId' string
static const std::string KEY_INVALID_ID = "InvalidId";

//! Constant key for 'status' string
static const std::string KEY_STATUS = "status";

//! Constant key for 'vin' string
static const std::string KEY_VIN = "vin";

//! Constant key for 'DataBaseSize' string
static const std::string KEY_DB_SIZE = "DataBaseSize";

//! Constant key for 'MaxDataBaseSize' string
static const std::string KEY_MAX_DB_SIZE = "MaxDataBaseSize";

//! Constant key for 'DeviceId' string
static const std::string KEY_DEVICE_ID = "DeviceId";

//! Constant key for 'LastSuccessfulUpload' string
static const std::string KEY_LAST_SUCCESSFUL_UPLOAD = "LastSuccessfulUpload";

//! Constant key for 'MqttConnStatus' string
static const std::string KEY_MQTT_CONN_STATUS = "MqttConnStatus";

//! Constant key for 'DbSize' string
static const std::string KEY_DATABASE_SIZE = "DbSize";

namespace ic_bl 
{
/**
 * Class CPersistancyAndStateHandler provides methods to persists and fetches 
 * the data from the local config 
 */
class CPersistancyAndStateHandler
{
public:
    /**
     * Method to get instance of CPersistancyAndStateHandler
     * @param void
     * @return Pointer to singleton object of CPersistancyAndStateHandler
     */
    static CPersistancyAndStateHandler* GetInstance();

    /**
     * Method to persist activation status
     * @param[in] rstrActivationDetails activation status details
     * @return true if activation status is stored in local config
     *         else return false
     */
    bool PersistActivationStatus(const std::string &rstrActivationDetails);

    /**
     * Method to persist client state
     * @param[in] eState client state
     * @return true if client status is stored in local config
     *         else return false
     */
    bool PersistICstate(const enum ic_core::IC_STATE eState);

    /**
     * Method to remove activation status from local config
     * @param void
     * @return true if activation status is removed from local config
     *         else return false
     */
    bool ClearActivationStatus();

    /**
     * Method to return string value of the IC_QUERY_ID enum
     * @param[in] eQueryId IC_QUERY_ID enum for which string value should be
     *                      obtained.
     * @return string value of the eQueryId
     */
    std::string GetKeyFromId(enum ic_core::IC_QUERY_ID eQueryId);

    /**
     * Method to fetch payload for IC_QUERY_ID enum
     * @param[in] eQueryId IC_QUERY_ID enum for which payload value should be
     *                      obtained.
     * @return json payload of IC_QUERY_ID enum
     */
    ic_utils::Json::Value GetICParam(ic_core::IC_QUERY_ID eQueryId);

    /**
     * Destructor.
     */
    ~CPersistancyAndStateHandler();

private:
    /**
     * Default no-argument constructor.
     */
    CPersistancyAndStateHandler();
};
}
#endif // CPERSISTANCY_AND_STATE_HANDLER_H
