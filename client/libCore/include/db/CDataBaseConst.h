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
* \file CDataBaseConst.h
*
* \brief This class/module holds const variables for database in igniteClient
*******************************************************************************
*/

#ifndef CDATA_BASE_CONST_H
#define CDATA_BASE_CONST_H

#include <string>

namespace ic_core 
{
/**
 * This class used to holds const variables for database class
 */
class CDataBaseConst 
{
public:
    //! Constant key for 'EVENT_STORE' string
    static const std::string TABLE_EVENT_STORE;

    //! Constant key for 'INVALID_EVENT_STORE' string
    static const std::string TABLE_INVALID_EVENT_STORE;

    //! Constant key for 'ALERT_STORE' string
    static const std::string TABLE_ALERT_STORE;

    //! Constant key for 'LocalConfig' string
    static const std::string TABLE_LOCAL_CONFIG;

    //! Constant key for 'UploadFile' string
    static const std::string TABLE_UPLOAD_FILE;

    //! Constant key for 'IGNITE_SERVICE_SETTINGS' string
    static const std::string TABLE_IGNITE_SERVICE_SETTINGS;

    //! Constant key for '_id' string
    static const std::string COL_ID;

    //! Constant key for 'EVENTID' string
    static const std::string COL_EVENT_ID;

    //! Constant key for 'TIMESTAMP' string
    static const std::string COL_TIMESTAMP;

    //! Constant key for 'TIMEZONE' string
    static const std::string COL_TIMEZONE;

    //! Constant key for 'PRIORITY' string
    static const std::string COL_PRIORITY;

    //! Constant key for 'EVENTS' string
    static const std::string COL_EVENTS;

    //! Constant key for 'key' string
    static const std::string COL_KEY_VAL;

    //! Constant key for 'value' string
    static const std::string COL_VALUE;

    //! Constant key for 'LAST_SUCCESSFUL_UPLOAD' string
    static const std::string COL_LAST_SUCCESSFUL_UPLOAD;

    //! Constant key for 'SIZE' string
    static const std::string COL_SIZE;

    //! Constant key for 'HAS_ATTACH' string
    static const std::string COL_HAS_ATTACH;

    //! Constant key for 'TOPIC' string
    static const std::string COL_TOPIC;

    //! Constant key for 'MID' string
    static const std::string COL_MID;

    //! Constant key for 'STREAM' string
    static const std::string COL_STREAM_SUPPORT;

    //! Constant key for 'BATCH' string
    static const std::string COL_BATCH_SUPPORT;

    //! Constant key for 'GRANULARITY' string
    static const std::string COL_GRANULARITY;

    //! Constant key for 'MESSAGE_SENDER_STORE' string
    static const std::string MESSAGE_SENDER_STORE;

    //! Constant key for 'CLIENT_APP_ID' string
    static const std::string COL_APPID;

    //! Constant key for 'CLIENT_EVENT_TYPE' string
    static const std::string COL_TYPE;

    //! Constant key for 'FILE_PATH' string
    static const std::string COL_FILE_PATH;

    //! Constant key for 'IS_FILE_FINAL_CHUNK' string
    static const std::string COL_IS_FILE_FINAL_CHUNK;

    //! Constant key for 'SPLIT_ID' string
    static const std::string COL_SPLIT_COUNT;

    //! Constant key for 'FILE_SIZE' string
    static const std::string COL_FILE_SIZE;

    //! Constant key for 'SETTING_ID' string
    static const std::string COL_SETTING_ID;

    //! Constant key for 'SETTING_ENUM' string
    static const std::string COL_SETTING_ENUM;

    //! Constant key for 'VALUE' string
    static const std::string COL_SETTING_VALUE;

    //! Constant key for 'SETTING_RESPONSE_STATUS' string
    static const std::string COL_SETTING_RESPONSE_STATUS;

    //! Constant key for 'SETTING_CORR_ID' string
    static const std::string COL_SETTING_CORR_ID;

    //! Constant key for 'SETTING_SRC_ISDEVICE' string
    static const std::string COL_SETTING_SRC_ISDEVICE;

    //! Constant key for 'NA' string
    static const std::string SETTING_NOT_APPLICABLE;

    //! Constant key for 'configUpdate' string
    static const std::string KEY_DB_CONFIG;

    //! Constant key for 'sqlite_master' string
    static const std::string TABLE_SQLITE_MASTER;

    //! Constant key for 'type' string
    static const std::string COL_TYPE_ID;

    //! Constant key for 'name' string
    static const std::string COL_NAME_ID;   
};
} /* namespace ic_core */
#endif //CDATA_BASE_CONST_H
