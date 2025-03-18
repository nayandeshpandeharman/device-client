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
#include <cstdio>
#include <fstream>
#include <string>
#include "CIgniteConfig.h"
#include "CIgniteLog.h"
#include "CIgniteStringUtils.h"
#include "crypto/CSha.h"
#include "db/CLocalConfig.h"
#include "CIgniteFileUtils.h"
#include "db/CServiceSettingsStore.h"
#include "CPreIgniteLogger.h"

//! Macro for 'CIgniteConfig' string
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CIgniteConfig"

namespace ic_core 
{
namespace 
{
//! Constant key for 'Extensions' string
static const std::string EXTENSIONS = "Extensions";

//! Constant key for 'ProtectedMembers' string
static const std::string PROTECTED_MEMBERS = "ProtectedMembers";
}

CIgniteConfig* CIgniteConfig::m_pConfigSingleton = NULL;

void CIgniteConfig::ConfigureFileLogging()
{
    ic_utils::Json::Value jsonRoot = 
                        CIgniteConfig::GetInstance()->GetJsonValue("FileLogger");
    if (IsFileLoggingEnabled(jsonRoot))
    { 
        /*
         * Read log level from from database, if present apply same to the 
         * logging
         */ 
        int nLogLevel = 0;
        std::string strDbLogLevel = CLocalConfig::GetInstance()->Get("LogLevel");
        if (!strDbLogLevel.empty())
        {
            nLogLevel = atoi(strDbLogLevel.c_str());
            HCPLOG_C << "LogLevel from db "<< nLogLevel;
            if(nLogLevel == 0)
            {
                /*
                 * If logLevel captured from db is 0, it means logging should 
                 * be disabled.
                 */
                return;
            }
        }

        std::string strLogFilePath = GetLogFilePath(jsonRoot);
        int nLogFileSize = GetLogFileSize(jsonRoot);
        int nLogFileTruncateSize = GetLogFileTruncateSize(jsonRoot);
        
        /*
         * LogLevel == 0 means it was not stored in database so capture and set
         * it from conf file.
         */ 
        if (0 == nLogLevel)
        {
            nLogLevel = GetLogLevel(jsonRoot);
            HCPLOG_C << "LogLevel from config "<< nLogLevel;
        }

        // Successful Parsing of StreamLogging
        if (!strLogFilePath.empty() && 0 != nLogFileSize && 
            0 != nLogFileTruncateSize && HCPLOG_MAX_LEVEL >= nLogLevel )
        {
            ic_utils::CIgniteLog::SetFileOutputPath(strLogFilePath, 
                                         nLogFileSize,
                                         nLogFileTruncateSize);
            ic_utils::CIgniteLog::SetFileOutputLevel(static_cast<ic_utils::LogLevel>(nLogLevel));
        }
    }
    else
    {
        HCPLOG_E << "StreamLogging Config not found or invalid. Check config file";
    }
}

bool CIgniteConfig::IsFileLoggingEnabled(const ic_utils::Json::Value &rjsonRoot)
{
    bool bIsEnabled = false;
    if (!rjsonRoot.isNull() && rjsonRoot.isMember("enableLogging") &&
        rjsonRoot["enableLogging"].isBool() && 
        rjsonRoot["enableLogging"].asBool())
    {
        bIsEnabled = true;
    }
    return bIsEnabled;
}

std::string CIgniteConfig::GetLogFilePath(const ic_utils::Json::Value &rjsonRoot)
{
    std::string strLogFilePath = "";
    if (rjsonRoot.isMember("logFilePath") &&
        rjsonRoot["logFilePath"].isString())
    {
        strLogFilePath = rjsonRoot["logFilePath"].asString();
    }
    else
    {
        HCPLOG_W << "logFilePath not found";
    }
    return strLogFilePath;
}

int CIgniteConfig::GetLogFileSize(const ic_utils::Json::Value &rjsonRoot)
{
    int nLogFileSize = 0;
    if (rjsonRoot.isMember("logFileSize") && rjsonRoot["logFileSize"].isInt())
    {
        nLogFileSize = rjsonRoot["logFileSize"].asInt();
    }
    else
    {
        HCPLOG_W << "logFileSize not found";
    }
    return nLogFileSize;
}

int CIgniteConfig::GetLogFileTruncateSize(const ic_utils::Json::Value &rjsonRoot)
{
    int nLogFileTruncateSize = 0;
    if (rjsonRoot.isMember("logFileTruncateSize") &&
        rjsonRoot["logFileTruncateSize"].isInt())
    {
        nLogFileTruncateSize = rjsonRoot["logFileTruncateSize"].asInt();
    }
    else
    {
        HCPLOG_W << "logFileTruncateSize not found";
    }
    return nLogFileTruncateSize;
}

int CIgniteConfig::GetLogLevel(const ic_utils::Json::Value &rjsonRoot)
{
    int nLogLevel = 0;
    if (rjsonRoot.isMember("logLevel") && rjsonRoot["logLevel"].isInt())
    {
        nLogLevel = rjsonRoot["logLevel"].asInt();
    }
    else
    {
        HCPLOG_D << "LogLevel not found! ";
    }
    return nLogLevel;
}

std::string CIgniteConfig::GetString(const std::string &rstrKey, 
                               const std::string &rstrDefaultValue,
                               const int nIndex)
{
    HCPLOG_METHOD() << ", key=" << rstrKey 
                    << ", default=" << rstrDefaultValue
                    << ", index=" << nIndex;
    std::string strRet = rstrDefaultValue;
    const ic_utils::Json::Value &rjsonValue = GetJsonValue(rstrKey, nIndex);
    if (rjsonValue.isString())
    {
        strRet = rjsonValue.asString();
    }
    HCPLOG_METHOD() << ", return=" << strRet;
    return strRet;
}

int CIgniteConfig::GetInt(const std::string &rstrKey,
                    const int nDefaultValue,
                    const int nIndex)
{
    HCPLOG_METHOD() << ", key=" << rstrKey
                    << ", default=" << nDefaultValue
                    << ", index=" << nIndex;
    int nRet = nDefaultValue;
    const ic_utils::Json::Value &rjsonValue = GetJsonValue(rstrKey, nIndex);
    if (rjsonValue.isIntegral())
    {
        nRet = rjsonValue.asInt();
    }
    HCPLOG_METHOD() << ", return=" << nRet;
    return nRet;
}

long CIgniteConfig::GetLong(const std::string &rstrKey,
                      const long lDefaultValue,
                      const int nIndex)
{
    HCPLOG_METHOD() << ", key=" << rstrKey 
                    << ", default=" << lDefaultValue
                    << ", index=" << nIndex;
    long lRet = lDefaultValue;
    const ic_utils::Json::Value &rjsonValue = GetJsonValue(rstrKey, nIndex);
    if (rjsonValue.isIntegral())
    {
        lRet = rjsonValue.asInt64();
    }
    HCPLOG_METHOD() << ", return=" << lRet;
    return lRet;
}

bool CIgniteConfig::GetBool(const std::string &rstrKey,
                      const bool bDefaultValue,
                      const int nIndex)
{
    HCPLOG_METHOD() << ", key=" << rstrKey
                    << ", default=" << bDefaultValue
                    << ", index=" << nIndex;
    bool bRet = bDefaultValue;
    const ic_utils::Json::Value &rjsonValue = GetJsonValue(rstrKey, nIndex);
    if (rjsonValue.isBool())
    {
        bRet = rjsonValue.asBool();
    }
    HCPLOG_METHOD() << ", return=" << bRet;
    return bRet;
}
double CIgniteConfig::GetDouble(const std::string &rstrKey,
                          const double dblDefaultValue,
                          const int nIndex)
{
    HCPLOG_METHOD() << ", key=" << rstrKey
                    << ", default=" << dblDefaultValue
                    << ", index=" << nIndex;
    double dblRet = dblDefaultValue;
    const ic_utils::Json::Value &rjsonValue = GetJsonValue(rstrKey, nIndex);
    if (rjsonValue.isNumeric())
    {
        dblRet = rjsonValue.asDouble();
    }
    HCPLOG_METHOD() << ", return=" << dblRet;
    return dblRet;
}

int CIgniteConfig::GetArraySize(const std::string &rstrKey)
{
    HCPLOG_METHOD() << ", key=" << rstrKey ;
    int nRet = 0;
    const ic_utils::Json::Value &rjsonValue = GetJsonValue(rstrKey);
    if (rjsonValue.isArray())
    {
        nRet = rjsonValue.size();
    }
    HCPLOG_METHOD() << ", return=" << nRet;
    return nRet;
}

std::string CIgniteConfig::GetAsString(const std::string &rstrKey,
                                 const std::string &rstrDefaultValue,
                                 const int nIndex)
{
    HCPLOG_METHOD() << ", key=" << rstrKey
                    << ", default=" << rstrDefaultValue
                    << ", index=" << nIndex;
    std::string strRet = rstrDefaultValue;
    const ic_utils::Json::Value &rjsonValue = GetJsonValue(rstrKey, nIndex);
    if (rjsonValue.isInt())
    {
        strRet = ic_utils::CIgniteStringUtils::NumberToString(rjsonValue.asInt());
    }
    else if (rjsonValue.isDouble())
    {
        strRet = ic_utils::CIgniteStringUtils::NumberToString(rjsonValue.asDouble());
    }
    else if (rjsonValue.isString())
    {
        strRet = rjsonValue.asString();
    }
    else
    {
        // Do nothing
    }
    HCPLOG_METHOD() << ", return=" << strRet;
    return strRet;
}

const ic_utils::Json::Value& CIgniteConfig::GetJsonValue(const std::string &rstrKey, 
                                         const int nIndex)
{
    ic_utils::CScopeLock lock(m_InstanceMutex);
    const ic_utils::Json::Value &rjsonNullValue = ic_utils::Json::Value::nullRef;
    const char* pchCurrent = rstrKey.c_str();
    const char* pchEnd = pchCurrent + rstrKey.length();
    ic_utils::Json::Value *pjsonValue = &m_jsonRoot;

    while (pchCurrent != pchEnd)
    {
        if (*pchCurrent == '[')
        {
            if (rjsonNullValue == GetJsonArrayValue(&pchCurrent, pchEnd, 
                                                    &pjsonValue, rstrKey, nIndex))
            {
                return rjsonNullValue;
            }
            
        }
        else if (*pchCurrent == '.' )
        {
            // Skip key separator
            pchCurrent++;
        }
        else
        {
            if (rjsonNullValue == GetJsonObjectValue(&pchCurrent, pchEnd, 
                                                     &pjsonValue, rstrKey))
            {
                return rjsonNullValue;
            }  
        }
    }
    return *pjsonValue;
}

ic_utils::Json::Value CIgniteConfig::GetJsonArrayValue(const char **prchCurrent, 
                                       const char *pchEnd, 
                                       ic_utils::Json::Value **prjsonValue, 
                                       const std::string &rstrKey, 
                                       const int &rnIndex)
{
    const ic_utils::Json::Value &rjsonNullValue = ic_utils::Json::Value::nullRef;

    if (!(*prjsonValue)->isArray() )
    {
        HCPLOG_E << "FAIL: Not Array Type, key=" << rstrKey;
        return rjsonNullValue;
    }
    (*prchCurrent)++;

    int nArrayindex = 0;
    if (**prchCurrent == '%')
    {
        nArrayindex = rnIndex;
        (*prchCurrent)++; // Move to the next char (hopefully a ])
    }
    else
    {
        for ( ;
            *prchCurrent != pchEnd && **prchCurrent >='0' && **prchCurrent <='9'; 
            *(*prchCurrent)++)
        {
            nArrayindex = nArrayindex * 10 + **prchCurrent - '0';
        }
    }

    if (!ValidateGetJsonValue(*prchCurrent, pchEnd))
    {
        /*
         * Invalid!  We hit the end without a closing ] or it didn't 
         * follow the index.
         */
        HCPLOG_E << "FAIL: No closing ] ended unexpectedly!, key="
                    << rstrKey;
        return rjsonNullValue;
    }
    (*prchCurrent)++; // Move past the ]

    if (!(*prjsonValue)->isValidIndex(nArrayindex))
    {
        // Invalid index.  Return null value.
        HCPLOG_E << "FAIL: Not valid index! arrayindex=" << nArrayindex
                    << ", key=" << rstrKey;
        return rjsonNullValue;
    }
    /*
     * We have a valid index.  Let's go ahead and move on to the next 
     * element...
     */
    *prjsonValue = &((**prjsonValue)[nArrayindex]);

    return **prjsonValue;
}

ic_utils::Json::Value CIgniteConfig::GetJsonObjectValue(const char **prchCurrent, 
                                       const char *pchEnd, 
                                       ic_utils::Json::Value **prjsonValue, 
                                       const std::string &rstrKey)
{
    const ic_utils::Json::Value &rjsonNullValue = ic_utils::Json::Value::nullRef;

    // Parse normal key name
    const char* nameStart = *prchCurrent;
    while (*prchCurrent != pchEnd &&
           **prchCurrent != '[' &&
           **prchCurrent != '.')
    {
        (*prchCurrent)++;
    }

    if (!(*prjsonValue)->isObject() )
    {
        // Unable to resolve.  Object expected here.
        HCPLOG_E << "Unable to resolve.  Object expected at "  <<
                    std::string(nameStart, *prchCurrent) 
                    << ", key=" << rstrKey;
        return rjsonNullValue;
    }

    // Found whole key.  Go ahead and try to move deeper.
    *prjsonValue = &((**prjsonValue)[std::string(nameStart, *prchCurrent)]);

    if ((*prjsonValue)->isNull())
    {
        HCPLOG_D << "Key not present in JSON! " << rstrKey;
        // Expected key not found.  Error
        return rjsonNullValue;
    }

    return **prjsonValue;
}

bool CIgniteConfig::ValidateGetJsonValue(const char* pchCurrent,
                                   const char* pchEnd)
{
    if (pchCurrent == pchEnd || *pchCurrent != ']' )
    {
        /*
         * Invalid!  We hit the end without a closing ] or it didn't 
         * follow the index.
         */
        return false;
    }
    return true;
}

CIgniteConfig* CIgniteConfig::CreateSingleton(const std::string &rstrConfigfile,
                                  const bool bEnableLogging)
{
    try
    {
        HCP_PRELOG_W("Creating config...");
        if (NULL == m_pConfigSingleton)
        {
            m_pConfigSingleton = new CIgniteConfig(rstrConfigfile);
        }
        else
        {
            throw std::string("CIgniteConfig::create() ERROR: CIgniteConfig already created!");
        }
    }
    catch (std::string &rstrStr)
    {
        HCP_PRELOG_W(rstrStr.c_str());
    }
    return m_pConfigSingleton;
}

CIgniteConfig* CIgniteConfig::GetInstance(void)
{
    if (NULL == m_pConfigSingleton)
    {
        throw std::string("CIgniteConfig::GetInstance() ERR:Config not created yet!");
    }
    return m_pConfigSingleton;
}

void CIgniteConfig::ClearInstance()
{
    if (NULL != m_pConfigSingleton)
    {
        delete m_pConfigSingleton;
        m_pConfigSingleton = NULL;
    }
}

CIgniteConfig::CIgniteConfig(const std::string &rstrConfigfile)
{
    HCPLOG_I << " configfile=" << rstrConfigfile;
    m_mapConfigUpdateSubscribers.clear();
    if (rstrConfigfile.empty())
    {
        throw std::string("CIgniteConfig file path not provided!");
    }

    m_strPath = rstrConfigfile;
    LoadConfigFromFile();

    m_strConfigVersion = CSha::GetFileHash(rstrConfigfile);
    ic_utils::Json::FastWriter jsonWriter;
    HCPLOG_D << "New CIgniteConfig after Extns applied: "
             << jsonWriter.write(m_jsonRoot);
    HCPLOG_C << "inite config done";
}

CIgniteConfig::~CIgniteConfig()
{

}

std::string CIgniteConfig::GetConfigVersion()
{
    return m_strConfigVersion;
}

void CIgniteConfig::LoadConfigFromFile()
{
    std::ifstream file(m_strPath.c_str(), std::ifstream::binary);
    if (!file.good())
    {
        throw std::string("CIgniteConfig file path not valid");
    }

    ic_utils::Json::Reader jsonReader;
    bool bParsingSuccessful = jsonReader.parse(file, m_jsonRoot, false);
    if (!bParsingSuccessful)
    {
        HCPLOG_F << "config file parsing not successful, check format";
        std::exit(-6);
    }
}

void CIgniteConfig::ReloadConfigFromDB()
{
    HCP_PRELOG_W("Reloading config...");
    std::map<std::string,ic_utils::Json::Value> mapConfigUpdateMap;
    mapConfigUpdateMap.clear();
    if (CServiceSettingsStore::GetInstance()->
                                 GetAllConfigUpdateSettings(mapConfigUpdateMap))
    {
        /*
         * Dont store file config to db if its not available. otherwise it will 
         * use db storage unnecesary
         */
        if (!mapConfigUpdateMap.empty())
        {
            ic_utils::CScopeLock lock(m_InstanceMutex);
            CopyDBConfig(mapConfigUpdateMap);
            HCPLOG_T << "CIgniteConfig parsed and applied";
        }

        ic_utils::Json::FastWriter jsonWriter;
        HCPLOG_D << "New CIgniteConfig after Extns applied: "
                 << jsonWriter.write(m_jsonRoot);
    }
}

bool CIgniteConfig::StoreSettingsToDb(const std::map<std::string,ic_utils::Json::Value>
                                &rmapConfigUpdateMap, std::string &rstrSource,
                                int mParameterCount)
{

    HCPLOG_METHOD();
    bool bStatus = false;
    // write  to db
    if (!rmapConfigUpdateMap.empty() && mParameterCount > 0)
    {
        ic_utils::Json::FastWriter jsonWriter;
        std::map<std::string,ic_utils::Json::Value>::const_iterator iter;

        for (iter = rmapConfigUpdateMap.begin();
             iter != rmapConfigUpdateMap.end();
             iter++)
        {
            std::string strKey = iter->first;
            ic_utils::Json::Value jsonValue = iter->second;
            ic_utils::Json::Value jsonParameter = ic_utils::Json::Value::nullRef;
            jsonParameter[strKey] = jsonValue;
            HCPLOG_D << " Storing key " << iter->first
                     << " ~ value is " << jsonWriter.write(iter->second);
            bStatus = CServiceSettingsStore::GetInstance()->
                               StoreSettings(rstrSource, strKey, jsonParameter);
        }
    }
    return bStatus;
}

bool CIgniteConfig::UpdateConfig(const std::map<std::string,ic_utils::Json::Value> &
                           rmapConfigUpdateMap,
                           enum EconfigUpdateSource eSource,
                           int mParameterCount)
{
    ic_utils::CScopeLock _lock(m_InstanceMutex);
    bool bResult(false);
    std::string strSourceStr = GetConfigUpdateSourceInString(eSource);
    if (!strSourceStr.empty())
    {
        // Remove older config for source from db
        bResult = CServiceSettingsStore::GetInstance()->
                                                    ClearSettings(strSourceStr);
        // Set all values of root_ config to default
        LoadConfigFromFile();

        if (mParameterCount > 0 && !rmapConfigUpdateMap.empty())
        {
            bResult = StoreSettingsToDb(rmapConfigUpdateMap,
                                        strSourceStr,
                                        mParameterCount);
            if (bResult)
            {
                CopyDBConfig(rmapConfigUpdateMap);
                HCPLOG_I << "New config updated to db";
            }
            else
            {
                HCPLOG_E << "ConfigSettingUpdate failed to store in db";
                // Clear half baked settings
                CServiceSettingsStore::GetInstance()->ClearSettings(strSourceStr);
            }
        } 
        else 
        {
            HCPLOG_W << "Reset settings for source " << strSourceStr;
            ResetConfigSettings();
        }
    }
    ic_utils::Json::FastWriter jsonWriter;
    HCPLOG_D << "New CIgniteConfig after Extns applied: " << jsonWriter.write(m_jsonRoot);
    return bResult;
}

void CIgniteConfig::ResetConfigSettings()
{
    std::map<std::string,ic_utils::Json::Value> mapStoredConfigUpdate;
    if (CServiceSettingsStore::GetInstance()->
                            GetAllConfigUpdateSettings(mapStoredConfigUpdate))
    {
        if (!mapStoredConfigUpdate.empty())
        {
            HCPLOG_D << "Settings available for other source";
            CopyDBConfig(mapStoredConfigUpdate);
        }
        else
        {
            HCPLOG_E << "configUpdateMap is empty";
        }
    } 
    else
    {
        HCPLOG_D << "No config update settings present in db";
    }
}

void CIgniteConfig::NotifyConfigUpdateToSubscribers()
{
    HCPLOG_D <<"Number of subscribers: "<<m_mapConfigUpdateSubscribers.size();
    for (const auto& itr : m_mapConfigUpdateSubscribers)
    {
        try
        {
            if (NULL != itr.second)
            {
                HCPLOG_D << "Notifying config update to " << itr.first;
                itr.second->NotifyConfigUpdate();
            } 
            else
            {
                HCPLOG_E << "Invalid subscriber obj " << itr.first;
            }
        }
        catch (...)
        {
            HCPLOG_E<<"unexpected exception ";
        }
    }
}

bool CIgniteConfig::CopyDBConfig(const std::map<std::string,ic_utils::Json::Value> 
                           &rmapConfigUpdateMap)
{
    HCPLOG_METHOD();
    bool bUpdatesAvailable = false;
    std::map<std::string,ic_utils::Json::Value>::const_iterator iter;

    if (!rmapConfigUpdateMap.empty())
    {
        for (iter = rmapConfigUpdateMap.begin();
             iter != rmapConfigUpdateMap.end();
             iter ++)
        {
            std::string strPath = iter->first;
            ic_utils::Json::Value jsonValue = iter->second;

            HCPLOG_D << "Read path :" << strPath;
            std::list<std::string> listPathKeys;
            ic_utils::CIgniteStringUtils::Split(strPath, '.', listPathKeys);
            ic_utils::Json::Value *pjsonRootCopy = &m_jsonRoot;
            bUpdatesAvailable = ProcessDBConfigFile(listPathKeys, jsonValue,
                                                    pjsonRootCopy);
        }// End of for
    } 
    else
    {
        HCPLOG_C << "Discard invalid config update.";
    }

    return bUpdatesAvailable;
}

bool CIgniteConfig::ProcessDBConfigFile(std::list<std::string> &rlistPathKeys,
                                  const ic_utils::Json::Value &rjsonValue,
                                  ic_utils::Json::Value *pjsonRootCopy)
{
    bool bUpdatesAvailable = false;
    unsigned int nNumberOfPathKeyTraversed = 0;
    unsigned int nNumberOfPathKeys = rlistPathKeys.size();
    std::string strCurrentVal = "";
    for (std::list<std::string>::iterator listIter = rlistPathKeys.begin();
            listIter != rlistPathKeys.end();
            ++listIter)
    {
        HCPLOG_D << "parse :" << *listIter;
        if (ic_utils::Json::Value::nullRef != rjsonValue)
        {
            pjsonRootCopy = &(pjsonRootCopy->operator[](*listIter));
        }
        else if (nNumberOfPathKeyTraversed != (nNumberOfPathKeys - 1))
        {
            pjsonRootCopy = &(pjsonRootCopy->operator[](*listIter));
            ++nNumberOfPathKeyTraversed;
        }
        else
        {
            // Do nothing
        }
        strCurrentVal = *listIter;
        bUpdatesAvailable = true;
    }
    
    if (ic_utils::Json::Value::nullRef != rjsonValue)
    {
        *pjsonRootCopy = rjsonValue;
    }
    else 
    {
        if (pjsonRootCopy->isMember(strCurrentVal))
        {
            //! When value is set to NULL, removing key from in memory config
            HCPLOG_D << "Removing from config" << strCurrentVal;
            pjsonRootCopy->removeMember(strCurrentVal);
        }
    }
    return bUpdatesAvailable;
}

void CIgniteConfig::SubscribeForConfigUpdateNotification(std::string strSubscriberName,
                                    IConfigUpdateNotification *pSubscriber)
{
    HCPLOG_T <<"Subscribing for configUpdate " << strSubscriberName;
    if (!strSubscriberName.empty() && pSubscriber != nullptr) 
    {
        m_mapConfigUpdateSubscribers.insert(std::make_pair(
                                               strSubscriberName, pSubscriber));
    }
    else
    {
        HCPLOG_E << "Subcription failed";
    }
}

void CIgniteConfig::UnSubscribeForConfigUpdateNotification(std::string strSubscriberName)
{
    HCPLOG_T << "Unsubscribing for configUpdate "<<strSubscriberName;
    if (!strSubscriberName.empty())
    {
        std::map <std::string, IConfigUpdateNotification*>::iterator iter =
                           m_mapConfigUpdateSubscribers.find(strSubscriberName);
        if (iter != m_mapConfigUpdateSubscribers.end())
        {
            m_mapConfigUpdateSubscribers.erase(strSubscriberName);
        }
        else
        {
            HCPLOG_E << "Unsubcription failed";
        }
    }
}

std::string CIgniteConfig::GetConfigUpdateSourceInString(enum EconfigUpdateSource eSource)
{

    HCPLOG_METHOD();
    std::string strSourcestr = "";
    switch (eSource)
    {
        case EconfigUpdateSource::eCLOUD_CONFIG_UPDATE :
        {
            strSourcestr = "cloudConfigUpdate";
            break;
        }
        default:
        {
            break;
        }
    }
    return strSourcestr;
}

#if 0
void CIgniteConfig::Test(void)
{

    HCPLOG_T << "Version=" << GetString("Version");
    HCPLOG_T << "DAM.Upload.CurlSender.timeout=" <<
             GetInt("DAM.Upload.CurlSender.timeout");
    HCPLOG_T << "DAM.Upload.CurlSender.timeout(getAsString)=" <<
             GetAsString("DAM.Upload.CurlSender.timeout");
    HCPLOG_T << "DoesntExist=" << GetString("DoesntExist");
    HCPLOG_T << "DoesntExist=" << GetString("DoesntExist", "INVALID");
    HCPLOG_T << "DAM.DoesntExist=" << GetString("DAM.DoesntExist");
    HCPLOG_T << "DAM.PPS[0].ratio=" << GetDouble("DAM.PPS[0].ratio");
    HCPLOG_T << "DAM.PPS[1].ratio=" << GetDouble("DAM.PPS[%].ratio", 0, 1);
    HCPLOG_T << "DAM.PPS[2].ratio(asInt)=" << GetInt("DAM.PPS[0].ratio");
    HCPLOG_T << "DAM.PPS[2].ratio(getAsString)=" << GetAsString("DAM.PPS[0].ratio");


    // Examples showing how a client can loop over an array using the index.
    // Which one is cleanest?

    const double dblInvalid = -12345678;

    // 1) While loop with both variables declared outside.  I think this is
    //    easiest to understand.
    HCPLOG_T << "START Array while loop test";
    {
        int nI = 0;
        double dblOut;
        while ((dblOut = GetDouble("DAM.PPS[%].ratio", dblInvalid, nI))
               != dblInvalid )
        {
            HCPLOG_T << "DAM.PPS[" << nI << "].ratio=" << dblOut;
            nI++;
        }
    }

    HCPLOG_T << "START Array for loop test";
    // 2) For loop using Array size.
    {
        int nSize = GetArraySize("DAM.PPS");
        for (int nI = 0; nI < nSize; nI++ )
        {
            HCPLOG_T << "DAM.PPS[" << nI << "].ratio=" 
                     << GetDouble("DAM.PPS[%].ratio", dblInvalid, nI);
        }
    }
}
#endif
} /* namespace ic_core */
