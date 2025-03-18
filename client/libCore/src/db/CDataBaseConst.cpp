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

#include <string>
#include "db/CDataBaseConst.h"

namespace ic_core 
{
//! Constant key for 'EVENT_STORE' string
const std::string CDataBaseConst::TABLE_EVENT_STORE = "EVENT_STORE";

//! Constant key for 'INVALID_EVENT_STORE' string
const std::string CDataBaseConst::TABLE_INVALID_EVENT_STORE = 
                                                          "INVALID_EVENT_STORE";

//! Constant key for 'ALERT_STORE' string
const std::string CDataBaseConst::TABLE_ALERT_STORE = "ALERT_STORE";

//! Constant key for 'LocalConfig' string
const std::string CDataBaseConst::TABLE_LOCAL_CONFIG = "LocalConfig";

//! Constant key for 'UploadFile' string
const std::string CDataBaseConst::TABLE_UPLOAD_FILE = "UploadFile";

//! Constant key for 'IGNITE_SERVICE_SETTINGS' string
const std::string CDataBaseConst::TABLE_IGNITE_SERVICE_SETTINGS = 
                                                      "IGNITE_SERVICE_SETTINGS";

//! Constant key for '_id' string
const std::string CDataBaseConst::COL_ID = "_id";

//! Constant key for 'EVENTID' string
const std::string CDataBaseConst::COL_EVENT_ID = "EVENTID";

//! Constant key for 'TIMESTAMP' string
const std::string CDataBaseConst::COL_TIMESTAMP = "TIMESTAMP";

//! Constant key for 'TIMEZONE' string
const std::string CDataBaseConst::COL_TIMEZONE = "TIMEZONE";

//! Constant key for 'PRIORITY' string
const std::string CDataBaseConst::COL_PRIORITY = "PRIORITY";

//! Constant key for 'EVENTS' string
const std::string CDataBaseConst::COL_EVENTS = "EVENTS";

//! Constant key for 'key' string
const std::string CDataBaseConst::COL_KEY_VAL = "key";

//! Constant key for 'value' string
const std::string CDataBaseConst::COL_VALUE = "value";

//! Constant key for 'LAST_SUCCESSFUL_UPLOAD' string
const std::string CDataBaseConst::COL_LAST_SUCCESSFUL_UPLOAD = 
                                                       "LAST_SUCCESSFUL_UPLOAD";

//! Constant key for 'SIZE' string
const std::string CDataBaseConst::COL_SIZE = "SIZE";

//! Constant key for 'HAS_ATTACH' string
const std::string CDataBaseConst::COL_HAS_ATTACH = "HAS_ATTACH";

//! Constant key for 'TOPIC' string
const std::string CDataBaseConst::COL_TOPIC = "TOPIC";

//! Constant key for 'MID' string
const std::string CDataBaseConst::COL_MID = "MID";

//! Constant key for 'STREAM' string
const std::string CDataBaseConst::COL_STREAM_SUPPORT = "STREAM";

//! Constant key for 'BATCH' string
const std::string CDataBaseConst::COL_BATCH_SUPPORT = "BATCH";

//! Constant key for 'GRANULARITY' string
const std::string CDataBaseConst::COL_GRANULARITY = "GRANULARITY";

//! Constant key for 'MESSAGE_SENDER_STORE' string
const std::string CDataBaseConst::MESSAGE_SENDER_STORE = "MESSAGE_SENDER_STORE";

//! Constant key for 'CLIENT_APP_ID' string
const std::string CDataBaseConst::COL_APPID = "CLIENT_APP_ID";

//! Constant key for 'CLIENT_EVENT_TYPE' string
const std::string CDataBaseConst::COL_TYPE = "CLIENT_EVENT_TYPE";

//! Constant key for 'FILE_PATH' string
const std::string CDataBaseConst::COL_FILE_PATH = "FILE_PATH";

//! Constant key for 'IS_FILE_FINAL_CHUNK' string
const std::string CDataBaseConst::COL_IS_FILE_FINAL_CHUNK = "IS_FILE_FINAL_CHUNK";

//! Constant key for 'SPLIT_ID' string
const std::string CDataBaseConst::COL_SPLIT_COUNT = "SPLIT_ID";

//! Constant key for 'FILE_SIZE' string
const std::string CDataBaseConst::COL_FILE_SIZE = "FILE_SIZE";

//! Constant key for 'SETTING_ID' string
const std::string CDataBaseConst::COL_SETTING_ID = "SETTING_ID";

//! Constant key for 'SETTING_ENUM' string
const std::string CDataBaseConst::COL_SETTING_ENUM = "SETTING_ENUM";

//! Constant key for 'VALUE' string
const std::string CDataBaseConst::COL_SETTING_VALUE = "VALUE";

//! Constant key for 'SETTING_RESPONSE_STATUS' string
const std::string CDataBaseConst::COL_SETTING_RESPONSE_STATUS = 
                                                      "SETTING_RESPONSE_STATUS";

//! Constant key for 'SETTING_CORR_ID' string
const std::string CDataBaseConst::COL_SETTING_CORR_ID = "SETTING_CORR_ID";

//! Constant key for 'SETTING_SRC_ISDEVICE' string
const std::string CDataBaseConst::COL_SETTING_SRC_ISDEVICE = 
                                                         "SETTING_SRC_ISDEVICE";

//! Constant key for 'NA' string
const std::string CDataBaseConst::SETTING_NOT_APPLICABLE = "NA";

//! Constant key for 'configUpdate' string
const std::string CDataBaseConst::KEY_DB_CONFIG = "configUpdate";

//! Constant key for 'sqlite_master' string
const std::string CDataBaseConst::TABLE_SQLITE_MASTER = "sqlite_master";

//! Constant key for 'type' string
const std::string CDataBaseConst::COL_TYPE_ID = "type";

//! Constant key for 'name' string
const std::string CDataBaseConst::COL_NAME_ID = "name";
} /* namespace ic_core */