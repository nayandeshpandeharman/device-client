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
* \file CIgniteConfig.h
*
* \brief This class/module provide methods to implements ignite configurations 
* related utilities
*******************************************************************************
*/

#ifndef CIGNITE_CONFIG_H
#define CIGNITE_CONFIG_H

#include <string>
#include <list>
#include "jsoncpp/json.h"
#include "CIgniteMutex.h"

namespace ic_core 
{
//! Constant key for 'ConfigType' string
static const std::string CONFIG_TYPE = "ConfigType";

//! Constant key for 'ConfigFile' string
static const std::string CONFIG_FILE = "ConfigFile";

//! Constant key for 'SignatureFile' string
static const std::string SIGNATURE_FILE = "SignatureFile";

//! Constant key for 'Updatable' string
static const std::string UPDATABLE = "Updatable";

/**
 * Interface class expose APIs necessary for notifying about config update
 */
class IConfigUpdateNotification
{
public:
    /**
     * Method to notify subscribers about config update
     * @param void
     * @return void
     */
    virtual void NotifyConfigUpdate() = 0;

    /**
     * Destructor
     */
    virtual ~IConfigUpdateNotification() {}
};

/**
 * Enum of config update sources
 */
enum EconfigUpdateSource
{
    eCLOUD_CONFIG_UPDATE = 0 ///< Cloud config update
};

/**
 * Class CIgniteConfig implements methods to handle ignite 
 * configurations utilities
 */
class CIgniteConfig 
{
public:
    /**
     * Method to create instance of CIgniteConfig class
     * @param[in] rstrConfigfile String containing config file path
     * @param[in] bEnableLogging true if logging is enabled, default true
     * @return instance of CIgniteConfig class
     */
    static CIgniteConfig* CreateSingleton(const std::string &rstrConfigfile, 
                                    const bool bEnableLogging=true);

    /**
     * Method to get instance of CIgniteConfig class
     * @param void
     * @return pointer to the instance of CIgniteConfig class
     */
    static CIgniteConfig* GetInstance();

    /**
     * Method to clear instance of CIgniteConfig class
     * @param void
     * @return void
     */
    static void ClearInstance();

    /**
     * Method to get the version of config file
     * @param void
     * @return String containing version of config file
     */
    std::string GetConfigVersion();

    /**
     * Method to get string value based on input parameter
     * @param[in] rstrKey String containing key value
     * @param[in] rstrDefaultValue String containing default value
     * @param[in] nIndex Index value
     * @return String value if key is member else default value
     */
    std::string GetString(const std::string &rstrKey, 
                          const std::string &rstrDefaultValue = "",
                          const int nIndex = 0);

    /**
     * Method to get the integer value based on input parameter
     * @param[in] rstrKey String containing key value
     * @param[in] nDefaultValue Default integer value
     * @param[in] nIndex Index value
     * @return Integer value if key is member else default value
     */        
    int GetInt(const std::string &rstrKey,
               const int nDefaultValue = 0,
               const int nIndex = 0);

    /**
     * Method to get the long value based on input parameter
     * @param[in] rstrKey String containing key value
     * @param[in] lDefaultValue Default long value
     * @param[in] nIndex Index value
     * @return Long value if key is member else default value
     */
    long GetLong(const std::string &rstrKey,
                 const long lDefaultValue = 0,
                 const int nIndex = 0);

    /**
     * Method to get the boolean value based on input parameter
     * @param[in] rstrKey String containing key value
     * @param[in] bDefaultValue Default boolean value
     * @param[in] nIndex Index value
     * @return Boolean value based on key else default value
     */
    bool GetBool(const std::string &rstrKey,
                 const bool bDefaultValue = false,
                 const int nIndex = 0);
    
    /**
     * Method to get the double value based on input parameter
     * @param[in] rstrKey String containing key value
     * @param[in] dblDefaultValue Default double value
     * @param[in] nIndex Index value
     * @return Double value if key is member else default value
     */
    double GetDouble(const std::string &rstrKey,
                     const double dblDefaultValue = 0.0,
                     const int nIndex = 0);


    /**
     * Method to get the JSON value based on input parameter
     * @param[in] rstrKey String containing key value
     * @param[in] nIndex Index value
     * @return JSON value based on key
     */             
    const ic_utils::Json::Value& GetJsonValue(const std::string &rstrKey, 
                                    const int nIndex = 0);

    /**
     * Method to get the requested value as string based on input parameter
     * @param[in] rstrKey String containing key value
     * @param[in] rstrDefaultValue String containing default value
     * @param[in] nIndex Index value
     * @return String value
     */
    std::string GetAsString(const std::string &rstrKey,
                            const std::string &rstrDefaultValue = "",
                            const int nIndex = 0);

    /**
     * Method to get the size of array based on input parameter
     * @param[in] rstrKey String containing key value
     * @return Size of array
     */
    int GetArraySize(const std::string &rstrKey);

    /**
     * Method to update the configuration to database, so that same would be 
     * applied after restart
     * Note: This function does not apply any validation on received updates, 
     * so caller must ensure all the validation before sending config map.
     * @param[in] rmapUpdate Map of key and value as below
     * KEY (string)                                  ELEMENT (json)
     * "DAM.DriverBehavior.Threshold.HarshBraking"   "-5.5"
     * "uploadMode.events.stream"                    ["Speed","RPM","IgnStatus"]
     * "DAM.DriverBehavior.Threshold.SharpTurn"      null
     * @param[in] eSource Source of config update
     * @param[in] nParameterCount Parameter count
     * @return True if config updated successfully, false otherwise 
     */
    bool UpdateConfig(const std::map<std::string,ic_utils::Json::Value> &rmapUpdate,
                      enum EconfigUpdateSource eSource,
                      int nParameterCount = 0);

    /**
     * Method to configure fle logging based on 'FileLogger.enableLogging' 
     * status
     * @param void
     * @return void
     */
    void ConfigureFileLogging(void);

    /**
     * Method to reads and applies configuration stored in db
     * status
     * @param void
     * @return void
     */
    void ReloadConfigFromDB();

    /**
     * Method to notify config update to configUpdate subscribers
     * @param void
     * @return void
     */
    void NotifyConfigUpdateToSubscribers();

    /**
     * Method to subscribe for config update notification
     * @param[in] strSubscriberName Subscriber name
     * @param[in] pSubscriber Instance of subscriber
     * @return void
     */
    void SubscribeForConfigUpdateNotification(std::string strSubscriberName,
                                   IConfigUpdateNotification *pSubscriber);

    /**
     * Method to unsubscribe from config update notification
     * @param[in] strSubscriberName Subscriber name
     * @return void
     */
    void UnSubscribeForConfigUpdateNotification(std::string strSubscriberName);

    /**
     * Method to get config update source based on input parameter
     * @param[in] eSource Source enum
     * @return String w.r.t enum value
     */
    std::string GetConfigUpdateSourceInString(enum EconfigUpdateSource eSource);

private:
    /**
     * Parameterized Constructor
     * @param[in] rstrConfigfile String containing config file path
     */
    CIgniteConfig(const std::string &rstrConfigfile);

    /**
     * Destructor
     */
    ~CIgniteConfig();

    /**
     * Method to append protected member list
     * @param[in] jsonRootNode JSON object containing root node value
     * @param[in] rjsonPmList JSON object containing protected member list
     * @return void
     */
    void AppendProtectedMembersList(ic_utils::Json::Value jsonRootNode,
                                    ic_utils::Json::Value &rjsonPmList);

    /**
     * Method to check override member based on input parameter
     * @param[in] rjsonPmList JSON object containing protected member list
     * @param[in] strMemberName String containing member name
     * @param[in] strConfigType String containing config type
     * @return true if member is override, false otherwise
     */                              
    bool CanOverrideMember(const ic_utils::Json::Value &rjsonPmList,
                           std::string strMemberName,
                           std::string strConfigType);
    
    /**
     * Method to print testing logs
     * @param void
     * @return void
     */
    void Test(void);

    /**
     * Method to loads config from file
     * @param void
     * @return void
     */
    void LoadConfigFromFile();

    /**
     * Method to reads each stored config parameter and update the original 
     * config json object with same
     * @param[in] rmapConfigUpdateMap map of config parameters
     * @return True if any config parameter applied successfully, false 
     * otherwise
     */
    bool CopyDBConfig(
        const std::map<std::string,ic_utils::Json::Value> &rmapConfigUpdateMap);

    /**
     * Method to store configuration to setting store table of database
     * config json object with same
     * @param[in] rmapConfigUpdateMap Map of key and value as below
     * KEY (string)                                  ELEMENT (json)
     * "DAM.DriverBehavior.Threshold.HarshBraking"   "-5.5"
     * "uploadMode.events.stream"                    ["Speed","RPM","IgnStatus"]
     * "DAM.DriverBehavior.Threshold.SharpTurn"      null
     * @param[in] rstrSource String containing source of config update
     * @param[in] nParameterCount Parameter count value
     * @return True if config stored successfully, false otherwise
     */
    bool StoreSettingsToDb(
              const std::map<std::string,ic_utils::Json::Value> &rmapConfigUpdateMap,
              std::string &rstrSource,
              int nParameterCount = 0);

    /**
     * Method to process config file based on input parameter
     * @param[in] rlistPathKeys list of path keys
     * @param[in] rjsonValue JSON object containing config value
     * @param[in] pjsonRootCopy JSON object containing config root copy
     * @return True if any config parameter process successfully, false 
     * otherwise
     */
    bool ProcessDBConfigFile(std::list<std::string> &rlistPathKeys,
                             const ic_utils::Json::Value &rjsonValue,
                             ic_utils::Json::Value *pjsonRootCopy);

    /**
     * Method to reload config from db to ensure settings from other source are 
     * applied in case of reset
     * @param void
     * @return void
     */
    void ResetConfigSettings();

    /**
     * Method to get log file path
     * @param[in] rjsonRoot File logging configuration
     * @return log file path string
     */
    std::string GetLogFilePath(const ic_utils::Json::Value &rjsonRoot);

    /**
     * Method to get log file size
     * @param[in] rjsonRoot File logging configuration
     * @return log file size
     */
    int GetLogFileSize(const ic_utils::Json::Value &rjsonRoot);

    /**
     * Method to get log file truncate size
     * @param[in] rjsonRoot File logging configuration
     * @return log file truncate size
     */
    int GetLogFileTruncateSize(const ic_utils::Json::Value &rjsonRoot);

    /**
     * Method to get log level
     * @param[in] rjsonRoot File logging configuration
     * @return Log level stored in DB
     */
    int GetLogLevel(const ic_utils::Json::Value &rjsonRoot);
    
    /**
     * Method to validate valid json value
     * @param[in] pchCurrent JSON object containing current value
     * @param[in] pchEnd JSON object containing end value
     * @return True is json is valid, false otherwise
     */
    bool ValidateGetJsonValue(const char* pchCurrent, const char* pchEnd);

    /**
     * Method to check if file logging is enabled or not
     * @param[in] rjsonRoot JSON object containing file logger configuration
     * @return true is file logging is enabled, false otherwise
     */
    bool IsFileLoggingEnabled(const ic_utils::Json::Value &rjsonRoot);

    /**
     * Method to get json array value based on input parameter
     * @param[in]/[out] prchCurrent GetJsonValue key value
     * @param[in] pchEnd end value of GetJsonValue key
     * @param[in]/[out] prjsonValue actual config file data
     * @param[in] rstrKey GetJsonValue key
     * @param[in] rnIndex Index value
     * @return returns json value of key; if any error, returns json null 
     * reference
     */ 
    ic_utils::Json::Value GetJsonArrayValue(const char **prchCurrent, 
                                            const char *pchEnd, 
                                            ic_utils::Json::Value **prjsonValue, 
                                            const std::string &rstrKey, 
                                            const int &rnIndex);

    /**
     * Method to get json object value based on input parameter
     * @param[in]/[out] prchCurrent GetJsonValue key value
     * @param[in] pchEnd end value of GetJsonValue key
     * @param[in]/[out] prjsonValue actual config file data
     * @param[in] rstrKey GetJsonValue key
     * @return returns json value of key; if any error, returns json null 
     * reference
     */ 
    ic_utils::Json::Value GetJsonObjectValue(const char **prchCurrent, 
                                             const char *pchEnd, 
                                             ic_utils::Json::Value **prjsonValue, 
                                             const std::string &rstrKey);
    
    //! Member variable to stores config root value
    ic_utils::Json::Value m_jsonRoot;

    //! Member variable to stores config file path
    std::string m_strPath;

    //! Member variable to stores config version
    std::string m_strConfigVersion;
    
    //! Mutex variable
    ic_utils::CIgniteMutex m_InstanceMutex;

    //! Member variable to stores map of configUpdate subscribers list
    std::map <std::string, IConfigUpdateNotification*> 
                                                   m_mapConfigUpdateSubscribers;
                                                   
    //! Member variable to stores instance of 'Config' class
    static CIgniteConfig* m_pConfigSingleton;
};
} /* namespace ic_core */

#endif /* CIGNITE_CONFIG_H */
