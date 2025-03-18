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
* \file CUploadUtils.h
*
* \brief containing various upload related data structures, utility APIs 
* and a Connection Notification Interface. This is shared across MQTTUploader
********************************************************************************
*/

#ifndef CUPLOAD_UTILS_H
#define CUPLOAD_UTILS_H

#include "CIgniteConfig.h"
#include "CIgniteClient.h"
#include "CIgniteStringUtils.h"
#include "CIgniteLog.h"
#include "crypto/CIgniteDataSecurity.h"
#include "core/CKeyGenerator.h"
#include "HttpErrorCodes.h"
#include "core/CAesSeed.h"
#include "CIgniteDateTime.h"
#include "config/CUploadMode.h"
#include "CIgniteFileUtils.h"
#include "db/CDataBaseFacade.h"
#include "db/CLocalConfig.h"

#include <unistd.h>
#include <vector>
#include <set>

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CUploadUtils"

using ic_network::HttpErrorCode;

namespace ic_bl 
{

namespace 
{

//! Constant for path separator
static const char PATH_SEPARATOR = ':';

//! Constant for file separator
static const char FILE_SEPARATOR = '/';

//! Constant for event id Location
static const std::string LOCATION_EVENT_ID = "Location";

//! Constant for originl timestamp
static const std::string ORIG_TIMESTAMP = "OriginalTimestamp";

//! Constant for timestamp
static const std::string ATTR_TIMESTAMP = "Timestamp";

//! Constant key for LastSuccessfulUpload
static const std::string KEY_LAST_SUCCESSFUL_UPLOAD = "UploaderService.LastSuccessfulUpload";

//! Constant key for AttachmentBacklog
static const std::string KEY_ATTACHMENT_BACKLOG = "UploaderService.AttachmentBacklog";

//! Constant key for LastKnownLocation
static const std::string KEY_LAST_KNOWN_LOCATION = "UploaderService.LastKnownLocation";

//! Constant key for lastLocation
static const std::string KEY_LAST_LOCATION = "lastLocation";

//! Constant key for startupTimeout
static const std::string KEY_INITIAL_DELAY = "DAM.Upload.CurlSender.startupTimeout";

//! Constant key for localUploadFailureTimeout
static const std::string KEY_LOCAL_UPLOAD_FAILURE_TIMEOUT =
    "DAM.Upload.CurlSender.localUploadFailureTimeout";

//! Constant key for remoteUploadFailureTimeout
static const std::string KEY_REMOTE_UPLOAD_FAILURE_TIMEOUT =
    "DAM.Upload.CurlSender.remoteUploadFailureTimeout";

//! Constant key for uploadDiffTime
static const std::string KEY_UPLOAD_PERIOD = "DAM.Upload.CPULoadConfig.uploadDiffTime";

//! Constant key for maxUploadChunk
static const std::string KEY_MAX_CHUNK_SIZE = "DAM.Upload.CurlSender.maxUploadChunk";

//! Constant key for fileAttachmentTotalSizeLimit
static const std::string KEY_ATTACHMENT_TOTAL_SIZE = "DAM.Upload.CurlSender.fileAttachmentTotalSizeLimit";

//! Constant key for fileAttachmentUploadChunkSize
static const std::string KEY_ATTACHMENT_CHUNK_SIZE = "DAM.Upload.CurlSender.fileAttachmentUploadChunkSize";

//! Constant key for serverUrl
static const std::string KEY_UPLOAD_URL = "DAM.Upload.CurlSender.serverUrl";

//! Constant key for HTTP timeout
static const std::string KEY_HTTP_TIMEOUT = "DAM.Upload.CurlSender.timeout";

//! Constant key for HTTP mx retries
static const std::string KEY_HTTP_RETRY_MAX_COUNT = "DAM.Upload.CurlSender.retries";

//! Constant key for fileAttachmentPath
static const std::string KEY_ATTACH_PATH = "DAM.Upload.CurlSender.fileAttachmentPath";

//! Constant key for maxCPULoad
static const std::string KEY_MAX_CPU_LOAD = "DAM.Upload.CPULoadConfig.maxCPULoad";

//! Constant key for timeOutForCPULoad
static const std::string KEY_CPU_LOAD_RETRY_TIME = "DAM.Upload.CPULoadConfig.timeOutForCPULoad";

//! Constant key for Campaign-Name
static const std::string KEY_CAMPAIGN_NAME = "License.Campaign-Name";

//! Constant default value for initial delay
static const time_t DEF_INITIAL_DELAY = 15;

//! Constant default value for local upload failure timeout
static const time_t DEF_LOCAL_UPLOAD_FAILURE_TIMEOUT = 60;

//! Constant default value for remote upload failure timeout
static const time_t DEF_REMOTE_UPLOAD_FAILURE_TIMEOUT = 600;

//! Constant default value for upload period
static const time_t DEF_UPLOAD_PERIOD = 24 * 3600; // a full day

//! Constant default value for max chunk size
static const time_t DEF_MAX_CHUNK_SIZE = 2400000;

//! Constant default value for total attachment size
static const time_t DEF_ATTACHMENT_TOTAL_SIZE = 15000000;

//! Constant default value for attachment chunk size
static const time_t DEF_ATTACHMENT_CHUNK_SIZE = 1600000;

//! Constant default value for HTTP timeout
static const time_t DEF_HTTP_TIMEOUT = 600;

//! Constant default value for max retry count
static const int DEF_HTTP_RETRY_MAX_COUNT = 3;

//! Constant default value for max CPU load
static const float DEF_MAX_CPU_LOAD = 90.0f;

//! Constant default value for CUP load retry time
static const time_t DEF_CPU_LOAD_RETRY_TIME = 60;

//! Constant default value for DATERANGE_DBCHUNK_PULL_LIMIT
static const time_t DEF_DATERANGE_DBCHUNK_PULL_LIMIT = 7000;

//! Constant default value for UPLOAD_DBCHUNK_PULL_LIMIT
static const time_t DEF_UPLOAD_DBCHUNK_PULL_LIMIT = 6000;

//! Constant for DAY_IN_MILLIS
static const long long DAY_IN_MILLIS = 24 * 3600 * 1000;

//! Constant for MAX_GAP_BETWEEN_CHUNKS
static const long long MAX_GAP_BETWEEN_CHUNKS = DAY_IN_MILLIS;

//! Constant for HTTP retry delay
static const time_t HTTP_RETRY_DELAY = 10;

//! Constant for database event store record size
static const std::string KEY_DB_EVENTSTORE_RECORD_SIZE = "DAM.Database.eventStoreRecordSize";

//! Constant default value for event store record size
static const int DEF_EVENTSTORE_RECORD_SIZE = 200;

//! Constant for no limit on number of rows
static const int NO_LIMIT_ON_NUMBER_OF_ROWS = 0;

//! Constant for KEY_LOCATION_OPTIN
static const std::string KEY_LOCATION_OPTIN = "LocationOptIn";

//! Constant for last upload status
static std::string mLastUploadStatus = "";
#define NO_OF_CORRUPTED_EVENTS_TO_DELETE_IN_SINGLE_QUERY 100
}

class CUploadUtils
{
public:
    /**
     * Enum of possible upload modes
     */
    enum UploadDataMode 
    {
        eBATCH, ///< Batch upload mode
        eSTREAM, ///< Stream upload mode
        eBATCH_AND_STREAM ///< Batch and Stream upload mode
    };

    /**
     * Structure containing various fields that are part of event chunk generated
     * for upload purposes
     */
    struct EventChunk
    {
        long long llStartTime; ///< Start time
        long long llEndTime; ///< End time
        long long llStartLocalTime; ///< Start local time
        long long llEndLocalTime; ///< End local time
        std::set<long long> setLocalDays; ///< Set of local days
        std::string strEvents; ///< Events
        std::set<long long> setEventIds; ///< Set of Event Ids
        std::set<long long> setAlertIds; ///< Set of Alert Ids
        std::set<std::string> setAttachments; ///< set of Attachments
        int nHighestGranLevel; ///< Highest granularity level
        long long llGranStartTime; ///< Granularity start time
        long long llGranEndTime; ///< Granularity end time
    };

    /**
     * Enum of possible upload failure types
     */
    enum FailureType
    {
        eFT_NONE = 0, ///< Failure type none
        eFT_AUTH, ///< Authentication failure type
        eFT_HIGH_CPU_LOAD, ///< High CPU load failure
        eFT_SYSTEM_ERROR, ///< System error failure type
        eFT_NO_CONNECTIVITY, ///< No connectivity failure type
        eFT_REMOTE ///< Remote failure type
    };

    /**
     * Enum of possible granularity check response
     */
    enum GranularityResponseCode
    {
        eG_SUCCESS, ///< Success response code
        eG_BETTER_DATA_EXISTS, ///< Better data exists response code
        eG_SAME_DATA_EXISTS, ///< Same data exists reesponse code
        eG_SERVER_STATE_ERROR, ///< Server state error response code
        eG_OTHER, ///< Other response code
        eG_UNKNOWN ///< Unknown response code
    };

    /**
     * Method to find out if given mode of events exists in db for the given duration
     * @param[in] llStartTime Timestamp to find the events
     * @param[in] llEndTime Timestamp until event is found
     * @param[in] uMode Upload mode for the event - Batch or Stream
     * @return True if event(s) is found, else false
     */
    static bool EventExist(long long llStartTime, long long llEndTime, UploadDataMode uMode)
    {
        HCPLOG_METHOD() << "startTime=" << llStartTime << "; endTime=" << llEndTime << "; UploadDataMode=" << uMode;

        std::vector<std::string> vectProjection;
        vectProjection.push_back("COUNT(*)");

        std::string strSelection = "(" + ic_core::CDataBaseConst::COL_TIMESTAMP + " >= " + ic_utils::CIgniteStringUtils::NumberToString(llStartTime);
        strSelection += " AND " + ic_core::CDataBaseConst::COL_TIMESTAMP + " <= " + ic_utils::CIgniteStringUtils::NumberToString(llEndTime);
        strSelection += ")";

        if (uMode == eBATCH)
        {
            strSelection += " AND " + ic_core::CDataBaseConst::COL_BATCH_SUPPORT + " == 1";
        }
        else 
        {
            strSelection += " AND " + ic_core::CDataBaseConst::COL_STREAM_SUPPORT + " == 1";
        }

        ic_core::CCursor* pC = ic_core::CDataBaseFacade::GetInstance()->Query(ic_core::CDataBaseConst::TABLE_EVENT_STORE, vectProjection, strSelection);

        int nRowCount = 0;
        if (pC)
        {
            if (pC->MoveToFirst())
            {
                nRowCount = pC->GetInt(0);
            }
            delete pC;
        }

        return (nRowCount>0);
    }

    /**
     * Method to retrieve the max granularity level from EVENT_STORE table
     * @param[in] llStartTime Start event time to consider
     * @param[in] llEndTime End event time to consider
     * @param[in] uploadMode Upload mode for the event - Batch or Stream
     * @param[in] bAnonymMode Anonymous upload active status
     * @return 0 if error else max granularity level found according to given input params
     */
    static int FindEventStoreMaxGranularityLevel(long long llStartTime, long long llEndTime, UploadDataMode uploadMode, bool bAnonymMode)
    {
        int nMaxGranLevel = 0; // default value

        std::vector<std::string> vectProjection;
        vectProjection.push_back(ic_core::CDataBaseConst::COL_GRANULARITY);

        std::string strSelection = "(" + ic_core::CDataBaseConst::COL_TIMESTAMP + " >= " + ic_utils::CIgniteStringUtils::NumberToString(llStartTime);
        strSelection += " AND " + ic_core::CDataBaseConst::COL_TIMESTAMP + " <= " + ic_utils::CIgniteStringUtils::NumberToString(llEndTime);
        strSelection += ")";

        if (bAnonymMode) 
        {
            strSelection += " AND (" + ic_core::CDataBaseConst::COL_BATCH_SUPPORT + " == 1 OR " + ic_core::CDataBaseConst::COL_STREAM_SUPPORT + " == 1)";
        }
        else if (uploadMode == eBATCH)
        {
            strSelection += " AND (" + ic_core::CDataBaseConst::COL_BATCH_SUPPORT + " == 1)";
        }
        else if (uploadMode == eSTREAM)
        {
            strSelection += " AND (" + ic_core::CDataBaseConst::COL_STREAM_SUPPORT + " == 1)";
        }
        else
        {
            HCPLOG_E << "Unsupported upload mode!";
            return 0;
        }

        std::vector<std::string> vectOrderBy;
        vectOrderBy.push_back(ic_core::CDataBaseConst::COL_GRANULARITY + " DESC");
        int nRecordsToLoad = 1;

        ic_core::CCursor* pC = ic_core::CDataBaseFacade::GetInstance()->Query(ic_core::CDataBaseConst::TABLE_EVENT_STORE, vectProjection, strSelection, vectOrderBy, nRecordsToLoad);
        if (pC)
        {
            if (pC->MoveToFirst())
            {
                nMaxGranLevel = pC->GetInt(pC->GetColumnIndex(ic_core::CDataBaseConst::COL_GRANULARITY));
            }
            delete pC;
        }

        return nMaxGranLevel;
    }

    /**
     * Method to delete the given attachment files
     * @param[in] setAttachments Set of attachment files
     * @return void
     */
    static void DeleteAttachmentFiles(std::set<std::string> setAttachments)
    {
        HCPLOG_METHOD() << "File count:" << setAttachments.size();

        for (std::set<std::string>::iterator iter = setAttachments.begin(); iter != setAttachments.end(); iter++)
        {
            HCPLOG_T<<"Removing file " << *iter;
            ic_utils::CIgniteFileUtils::Remove(*iter);
        }
    }

    /**
     * Method to decrypt the given encrypted event
     * @param[in] rstrEventData Encrypted event
     * @return Decrypted event data
     */
    static std::string DecryptEventData(const std::string& rstrEventData)
    {
        ic_core::CIgniteDataSecurity securityObject(ic_core::CKeyGenerator::GetActivationKey(),
                           ic_core::CAesSeed::GetInstance()->GetIvRandom());
        return securityObject.Decrypt(rstrEventData);
    }

    /**
     * Method to delete the given corrupted events from the given table
     * @param[in] rstrTable Table name from which events are to be deleted
     * @param[in] rsetIds Event ids to delete from the given table
     * @return void
     */
    static void DeleteCorruptedEvents(const std::string &rstrTable, const std::set<long long>& rsetIds)
    {
        if (rsetIds.empty())
        {
            // Nothing to do
            return;
        }
        ic_core::CDataBaseFacade *pDb = ic_core::CDataBaseFacade::GetInstance();

        HCPLOG_T << "Deleting corrupted events...";
        std::string strSelection = ic_core::CDataBaseConst::COL_ID + " in (";
        for (std::set<long long>::iterator iter = rsetIds.begin(); iter != rsetIds.end(); iter++)
        {
            strSelection.append(ic_utils::CIgniteStringUtils::NumberToString(*iter));
            strSelection.append(",");
        }
        strSelection.replace(strSelection.size() - 1, 1, ")");
        pDb->Remove(rstrTable, strSelection);
    }

    /**
     * Method to load a cursor to retrieve EventStore data feom table
     * @param[in] rstrTable Table name from which to delete the events
     * @param[in] rsetIds Event ids to delete from the given table
     * @param[in] dataMode Upload data mode - Batch or Stream
     * @param[in] bAnonymousUploadActive Anonymous upload active status; default: false
     * @return void 
     */
    static void DeleteEventsFromDB(const std::string &rstrTable, const std::set<long long>& rsetIds,
            UploadDataMode dataMode, bool bAnonymousUploadActive = false)
    {
        HCPLOG_METHOD() << "Table: " << rstrTable << "; Total ids to delete: " << rsetIds.size();
        if (rsetIds.empty())
        {
            // Nothing to do
            HCPLOG_T << "No ids found in the request to delete!";
            return;
        }
        
        std::string strSelection = GetSelectionFromIdsToDelete(rsetIds);
        ic_core::CDataBaseFacade *pDb = ic_core::CDataBaseFacade::GetInstance();

        if (ic_core::CDataBaseConst::TABLE_ALERT_STORE == rstrTable) 
	    {
            pDb->Remove(ic_core::CDataBaseConst::TABLE_ALERT_STORE, strSelection);
        }
        else if (ic_core::CDataBaseConst::TABLE_EVENT_STORE == rstrTable) 
        {
            if ((ic_core::CUploadMode::GetInstance()->IsStreamModeSupported()) && !bAnonymousUploadActive) 
            {
                // Delete events which already uploaded or not supported by batch mode
                bool bTransactionStarted = pDb->StartTransaction();
                std::string strDeleteSelection(strSelection);

                UpdateDeleteSelectionAsPerMode(strDeleteSelection, dataMode);

                pDb->Remove(ic_core::CDataBaseConst::TABLE_EVENT_STORE, strDeleteSelection);

                //mark remaining events as uploaded
                ic_core::CContentValues data;
                MarkEventsAsUploaded(dataMode, data);

                pDb->Update(ic_core::CDataBaseConst::TABLE_EVENT_STORE, &data, strSelection);

                if (bTransactionStarted)
                {
                    pDb->EndTransaction(true);
                }
            }
            else 
            {
                pDb->Remove(ic_core::CDataBaseConst::TABLE_EVENT_STORE, strSelection);
            }
        }
        else 
        {
            HCPLOG_T << "Invalid table " << rstrTable;
        }
    }

    /**
     * Method to update selection for deletion of uploaded events as per UploadMode
     * @param[out] rstrDeleteSelection Selection of deletion
     * @param[in] rDataMode upload mode batch/stream
     * @return Selection string
     */
    static void UpdateDeleteSelectionAsPerMode(std::string& rstrDeleteSelection, const UploadDataMode &rDataMode)
    {
        if (rDataMode == eBATCH)
        {
            rstrDeleteSelection += " AND " + ic_core::CDataBaseConst::COL_STREAM_SUPPORT + " == 0";
        }
        else if (rDataMode == eSTREAM)
        {
            rstrDeleteSelection += " AND " + ic_core::CDataBaseConst::COL_BATCH_SUPPORT + " == 0";
        }
        else
        {
            // Do nothing
        }
    }

    /**
     * Method to get the selection string from event IDs to delete from table
     * @param[in] rsetEventIds Set of event ids to be deleted
     * @return Selection string
     */
    static std::string GetSelectionFromIdsToDelete(const std::set<long long>& rsetEventIds)
    {
        std::string strSelection = ic_core::CDataBaseConst::COL_ID + " in (";
        for (std::set<long long>::iterator iter = rsetEventIds.begin(); iter != rsetEventIds.end(); iter++)
        {
            strSelection.append(ic_utils::CIgniteStringUtils::NumberToString(*iter));
            strSelection.append(",");
        }
        strSelection.replace(strSelection.size() - 1, 1, ")");
        return strSelection;
    }

    /**
     * Method to mark event data as uploaded 
     * @param[in] rDataMode data mode of event - batch or stream
     * @param[out] rCContentValues updated CContentValues object
     * @return void
     */
    static void MarkEventsAsUploaded(const UploadDataMode& rDataMode, ic_core::CContentValues &rCContentValues)
    {
        if (rDataMode == eBATCH) 
        {
            rCContentValues.Put(ic_core::CDataBaseConst::COL_BATCH_SUPPORT,0);
        }
        else if (rDataMode == eSTREAM) 
        {
            rCContentValues.Put(ic_core::CDataBaseConst::COL_STREAM_SUPPORT,0);
        }
        else if (rDataMode == eBATCH_AND_STREAM)
        {
            rCContentValues.Put(ic_core::CDataBaseConst::COL_BATCH_SUPPORT,0);
            rCContentValues.Put(ic_core::CDataBaseConst::COL_STREAM_SUPPORT,0);
        }
        else
        {
            // Do nothing
        }
    }

    /**
     * Method to generate the upload properties
     * @param void
     * @return ic_utils::Json::Value containing the generated property attributes
     * 
     * Sample property string:
        {
            "Config-Version" : "ed9952510ba660704e3f0450c757b7466919ee8f9b955542994c746ae774f5ee",
            "HCP-Id" : "HUV481HY715422",
            "HCP-User" : "TESTVIN_Make:GenMake_Model:GenDevice_Year:2024_Type:Linux",
            "HW-Model" : "xiaomi-Redmi Note 5 Pro",
            "HW-SerialNumber" : "fc25e40c",
            "SW-Version" : "PKQ1.180904.001",
            "endLocalTS" : 1587470449676,
            "endTS" : 1587450649676,
            "localDays" : [ 1587427200000 ],
            "startLocalTS" : 1587470449676,
            "startTS" : 1587450649676
        }
     */
    static ic_utils::Json::Value GetUploadProperties()
    {
        static std::string strConfigVersion = ic_core::CIgniteConfig::GetInstance()->GetConfigVersion();

        ic_utils::Json::Value jsonRoot;
        ic_utils::Json::Reader jsonReader;
        ic_utils::Json::Value jsonProperties;

        ic_core::IProduct* pProd = ic_core::CIgniteClient::GetProductImpl();
        jsonProperties["HCP-User"] = pProd->GetAttribute(ic_core::IProduct::eVIN);

        jsonProperties["HCP-Id"] = ic_core::CLocalConfig::GetInstance()->Get("login");

        jsonRoot = ic_utils::Json::nullValue;
        std::string strSwVersion = pProd->GetAttribute(ic_core::IProduct::eSWVersion);
        if (jsonReader.parse(strSwVersion, jsonRoot))
        {
            jsonProperties["SW-Version"] = jsonRoot;
        }
        else
        {
            jsonProperties["SW-Version"] = strSwVersion;
        }

        jsonRoot = ic_utils::Json::nullValue;
        std::string strHwVersion = pProd->GetAttribute(ic_core::IProduct::eHWVersion);
        if (jsonReader.parse(strHwVersion, jsonRoot))
        {
            jsonProperties["HW-Model"] = jsonRoot;
        }
        else
        {
            jsonProperties["HW-Model"] = strHwVersion;
        }

        jsonProperties["HW-SerialNumber"] = pProd->GetAttribute(ic_core::IProduct::eSerialNumber);
        jsonProperties["Config-Version"] = strConfigVersion;

        std::string strDeviceInfo = ic_core::CLocalConfig::GetInstance()->Get("decodedFields");
        jsonRoot = ic_utils::Json::nullValue;
        if (jsonReader.parse(strDeviceInfo, jsonRoot))
        {
            for(ic_utils::Json::ValueIterator itr = jsonRoot.begin(); itr != jsonRoot.end(); ++itr)
            {
                jsonProperties[itr.name()] = jsonRoot[itr.name()];
            }
        }

        return jsonProperties;
    }

    /**
     * @brief Method to retrieve topic based events from given table
     * @param[in] rstrTable Table name
     * @param[in] nNumRowsRequested Number of rows to be retrieved from the table
     * @param[out] pvectRowIDs Vector of event ids loaded from the table
     * @param[out] rmapResults Loaded event details
     * @param[in] llLimitStarTimestamp Event timestamp to be conidered as start time 
     * @param[in] llLimitEndTimestamp Event timestamp to be conidered as cutoff, default - 0 i.e. no limit
     * @return void
     */
    static void GetTopicedStreamingEventsFromDB(const std::string &rstrTable, int nNumRowsRequested, std::vector<long long>* pvectRowIDs,
            std::map<long long, std::pair<std::string, std::string> >& rmapResults, long long llLimitStarTimestamp = 0, long long llLimitEndTimestamp=0)
    {
        HCPLOG_METHOD() << "numRowsRequested=" << nNumRowsRequested << "; limitStarTimestamp=" << llLimitStarTimestamp <<
                "; limitEndTimestamp=" << llLimitEndTimestamp;

        pvectRowIDs->clear();
        rmapResults.clear();
        
        std::string strTopicPrefix = "";
        bool bTopicPrefixRead = GetTopicPrefix(strTopicPrefix);

        std::string strDeviceId = ic_core::CLocalConfig::GetInstance()->Get("login");
        if (strDeviceId.empty()) 
        {
            HCPLOG_E << " *** DeviceID is not found in localconfig hence event will not be published to topic ***";
            return;
        }
                               
        std::vector<std::string> vectProjection;
        vectProjection.push_back(ic_core::CDataBaseConst::COL_ID);
        vectProjection.push_back(ic_core::CDataBaseConst::COL_EVENT_ID);
        vectProjection.push_back(ic_core::CDataBaseConst::COL_TIMESTAMP);
        vectProjection.push_back(ic_core::CDataBaseConst::COL_EVENTS);
        vectProjection.push_back(ic_core::CDataBaseConst::COL_TOPIC);

        std::vector<std::string> vectOrderBy;
        vectOrderBy.push_back(ic_core::CDataBaseConst::COL_TIMESTAMP + " ASC");

        std::string strSelection(ic_core::CDataBaseConst::COL_TIMESTAMP + " IS NOT NULL");

        if (llLimitEndTimestamp > 0) 
        {
            strSelection += " AND " + ic_core::CDataBaseConst::COL_TIMESTAMP + " <= " + ic_utils::CIgniteStringUtils::NumberToString(llLimitEndTimestamp);
        }

        if (llLimitStarTimestamp > 0) 
        {
            strSelection += " AND " + ic_core::CDataBaseConst::COL_TIMESTAMP + " >= " + ic_utils::CIgniteStringUtils::NumberToString(llLimitStarTimestamp);
        }

        strSelection += " AND " + ic_core::CDataBaseConst::COL_TOPIC + " IS NOT NULL";

        strSelection += " AND " + ic_core::CDataBaseConst::COL_MID + " == 0";


        std::string strActualSelection = strSelection;
        std::string strTotalCorruptedEvIds = "";
        std::set<long long> setCorruptedEvIds;

        bool bContinueQuery = true;
        while (bContinueQuery)
        {
            strActualSelection = strSelection + " AND (" + ic_core::CDataBaseConst::COL_ID + " NOT IN (" + strTotalCorruptedEvIds + ")) ";
            ic_core::CCursor* pC = ic_core::CDataBaseFacade::GetInstance()->Query(rstrTable, vectProjection, strActualSelection, vectOrderBy, nNumRowsRequested);

            if ((pC) && (pC->Size() > 0))
            {
                bContinueQuery = ValidateTopicedStreamingEvent(pC,strTotalCorruptedEvIds,setCorruptedEvIds,rmapResults,pvectRowIDs,rstrTable,strDeviceId,strTopicPrefix);
            }
            else
            {
                if(pC)
                {
                    delete pC;
                    pC = NULL;
                }

                bContinueQuery = false;
            } // If cursor
        } // While continuequery

        DeleteCorruptedEventsIfPresent(rstrTable,setCorruptedEvIds);
    }

    /**
     * Method to validated topiced streaming events
     * @param[in/out] pCursor pointer to the object of CCursor
     * @param[in/out] rstrTotalCorruptedEvIds total corrupted eventIds
     * @param[in/out] rsetCorruptedEvIds set of corrupted eventIds 
     * @param[out] rmapResults loaded event details
     * @param[out] pvectRowIDs number of rows retrieved
     * @param[in] rstrTable table name string
     * @param[in] rstrDeviceId device ID string 
     * @param[in] rstrTopicPrefix topic prefix from config
     * @return True if no records are fetched , false otherwise
     */
    static bool ValidateTopicedStreamingEvent(ic_core::CCursor* pCursor,std::string &rstrTotalCorruptedEvIds,
        std::set<long long> &rsetCorruptedEvIds,std::map<long long, std::pair<std::string, std::string> >& rmapResults, 
        std::vector<long long>* pvectRowIDs,const std::string &rstrTable,const std::string &rstrDeviceId, const std::string &rstrTopicPrefix)
    {
        bool bContinueQuery = true;
        for (bool bWorking = pCursor->MoveToFirst(); bWorking; bWorking = pCursor->MoveToNext())
        {
            long long llId = pCursor->GetLong(pCursor->GetColumnIndex(ic_core::CDataBaseConst::COL_ID));
            std::string strEventData = pCursor->GetString(pCursor->GetColumnIndex(ic_core::CDataBaseConst::COL_EVENTS));
            strEventData = DecryptEventData(strEventData);

            ic_utils::Json::Value jsonEvent;
            ic_utils::Json::Reader jsonReader;
            //making sure if the event decryption is success and the event json is parseable
            if (strEventData.empty() || !(jsonReader.parse(strEventData,jsonEvent)))
            {
                //add the corrupted event's id to the set to delete them in one shot
                HCPLOG_E << "Corrupted event(id=" << llId << "):" << strEventData;
                rsetCorruptedEvIds.insert(llId);

                //add the corrupted event to the list to exclude them from next query
                if (rstrTotalCorruptedEvIds.empty()) 
                {
                    rstrTotalCorruptedEvIds = ic_utils::CIgniteStringUtils::NumberToString(llId);
                }
                else 
                {
                    rstrTotalCorruptedEvIds += "," + ic_utils::CIgniteStringUtils::NumberToString(llId);
                }
                continue;
            }

            std::string strTopic = pCursor->GetString(pCursor->GetColumnIndex(ic_core::CDataBaseConst::COL_TOPIC));
            HCPLOG_D << "GOT TOPICED EVENT = " << strEventData << "  AND   topic = " << strTopic;

            // Extract the substring from 2c to end of topic string ex: 2c/abc/xyz
            size_t nPosOf2c = strTopic.find("2c/"); 
            if (nPosOf2c != std::string::npos) 
            {
                strTopic = rstrTopicPrefix + rstrDeviceId + "/" + strTopic.substr(nPosOf2c);
                HCPLOG_D << "Topic after reconstruction: " << strTopic;
                pvectRowIDs->push_back(llId);
                rmapResults[llId] = std::make_pair(strTopic, strEventData);
            }
            else 
            {
                HCPLOG_E << "ERROR: 2c not found in topic: "<< strTopic;
            } // nPosOf2c
            
        } // for loop

        delete pCursor;
        pCursor = NULL;

        /* If corrupted event ids are more than the limit, delete them
         * immediately to avoid huge query later
         */
        if (rsetCorruptedEvIds.size() >= NO_OF_CORRUPTED_EVENTS_TO_DELETE_IN_SINGLE_QUERY) 
        {
            DeleteCorruptedEvents(rstrTable, rsetCorruptedEvIds);

            // Reset the set
            rsetCorruptedEvIds.clear();
            rstrTotalCorruptedEvIds = "";
        }

        // If no records are fetched (all corrupted records?), query further
        if (pvectRowIDs->size() > 0)
        {
            bContinueQuery = false;
        }

        return bContinueQuery;
    }

    /**
     * Method to get topic prefix
     * @param[out] rstrTopicPrefix Topic prefix from config
     * @return True if topic prefix already read, false otherwise
     */
    static bool GetTopicPrefix(std::string &rstrTopicPrefix)
    {          
        static std::string strTopicPrefix = "";
        static bool bTopicPrefixRead = false;
        if (!bTopicPrefixRead) 
        {
            ic_utils::Json::Value jsonMqRoot = ic_core::CIgniteConfig::GetInstance()->GetJsonValue("MQTT");
            if ((jsonMqRoot != ic_utils::Json::Value::nullRef) && (jsonMqRoot.isMember("topicprefix")) && (jsonMqRoot["topicprefix"].isString())) 
            {
                strTopicPrefix = jsonMqRoot["topicprefix"].asString();
                bTopicPrefixRead = true;
            }
            else 
            {
                HCPLOG_E << "topicprefix not-found/invalid in config!";
            }
        }

        rstrTopicPrefix = strTopicPrefix;
        return bTopicPrefixRead;
    }

    /**
     * Method to retrieve events/alerts from the database
     * @param[in] rstrLogEvent String to be updated for logging purpose
     * @param[in] rstrTable Table name
     * @param[in] nNumRowsRequested Requested number of rows to be retrieve
     * @param[out] pvectRowIDs Number of rows retrieved
     * @param[out] rstrResult Json formatted string containing events/alerts
     * @param[in] externalApplicationUpload Flag denoting whether to apply for external application events/alerts
     * @param[in] rsetExtAppEvent Set of External application events to be filtered as per 'externalApplicationUpload' flag and vendor
     * @param[in] rstrVendor Name of vendor
     * @param[in] llLimitStartTimestamp Get the events from DB starting from given timestamp
     * @param[in] llLimitEndTimestamp Get the events from DB until given timestamp
     * @param[in] llAfterId Get the events from DB before given id
     * @param[in] llBeforeId Get the events from DB after given id
     * @return void
     */
    static void GetStreamingEventsFromDB(std::string &rstrLogEvent,
                                         const std::string &rstrTable,
                                         int nNumRowsRequested,
                                         std::vector<long long>* pvectRowIDs,
                                         std::string& rstrResult,
                                         const std::set<std::string>& rsetExtAppEvent = std::set<std::string>(),
                                         const std::string& rstrVendor="",
                                         long long llLimitStartTimestamp = 0,
                                         long long llLimitEndTimestamp=0,
                                         long long llAfterId=-1,
                                         long long llBeforeId=-1)
    {
        HCPLOG_METHOD() << "Table: " << rstrTable << ";rowsReqsted: " << nNumRowsRequested << ";vendor: " << rstrVendor
                << ";startTime: " << llLimitStartTimestamp << ";endTime: " << llLimitEndTimestamp
                << ";afterId: " << llAfterId << "; beforeId: " << llBeforeId;
        static ic_utils::CIgniteMutex mGetEventsFromDBMutex;
        ic_utils::CScopeLock sLock(mGetEventsFromDBMutex);

        std::vector<std::string> vectProjection;
        vectProjection.push_back(ic_core::CDataBaseConst::COL_ID);
        vectProjection.push_back(ic_core::CDataBaseConst::COL_EVENT_ID);
        vectProjection.push_back(ic_core::CDataBaseConst::COL_TIMESTAMP);
        vectProjection.push_back(ic_core::CDataBaseConst::COL_EVENTS);

        std::vector<std::string> vectOrderBy;
        vectOrderBy.push_back(ic_core::CDataBaseConst::COL_TIMESTAMP + " ASC");

        std::string strSelection = GetSelectionForStreamingEvents(rstrTable,llLimitStartTimestamp,llLimitEndTimestamp,llAfterId,llBeforeId);
       
        HCPLOG_D << "vendor:"<<rstrVendor;

        pvectRowIDs->clear();
        rstrResult.clear();
        rstrResult.append("[");

        std::string strActualSelection = strSelection;
        std::string strTotalCorruptedEvIdsStr = "";
        std::set<long long> setCorruptedEvIds;

        std::stringstream strEvent;
        strEvent << "Loaded~[";

        bool bContinueQuery = true;
        while (bContinueQuery)
        {
            // Add a condition to exclude the corrupted events
            strActualSelection = strSelection + " AND (" + ic_core::CDataBaseConst::COL_ID + " NOT IN (" + strTotalCorruptedEvIdsStr + ")) ";
            ic_core::CCursor* pC = ic_core::CDataBaseFacade::GetInstance()->Query(rstrTable, vectProjection, strActualSelection, vectOrderBy, nNumRowsRequested);

            // Check if any records are fetched
            if ((pC) && (pC->Size() > 0))
            {

                ValidateStreamingEvent(pC,strEvent , strTotalCorruptedEvIdsStr, setCorruptedEvIds, pvectRowIDs, rstrResult);
                delete pC;
                pC = NULL;

                ValidateCorruptedEventIds(rstrTable,setCorruptedEvIds,strTotalCorruptedEvIdsStr);

                // If no records are fetched (all corrupted records?), query further
                bContinueQuery = IsFetchedRecordSizeZero(pvectRowIDs);
            }
            else
            {
                if(pC)
                {
                    delete pC;
                    pC = NULL;
                }

                bContinueQuery = false;
            }
        }

        DeleteCorruptedEventsIfPresent(rstrTable, setCorruptedEvIds);

        rstrLogEvent = strEvent.str();
        UpdateStreamEventUploadLogStr(rstrLogEvent);

        UpdateFinalResultJsonStr(rstrResult);
    }

    /**
     * Method to validate streaming event
     * @param[in/out] pCursor pointer to the object of CCursor
     * @param[out] rStrEvent string capturing log statement of event
     * @param[in/out] rstrTotalCorruptedEvIds total corrupted eventIds
     * @param[in/out] rsetCorruptedEvIds set of corrupted eventIds 
     * @param[out] pvectRowIDs : Number of rows retrieved
     * @param[out] rStrRsult : Json formatted string containing events/alerts
     * @return void
     */
    static void ValidateStreamingEvent(ic_core::CCursor* pCursor, std::stringstream &rStrEvent, std::string &rstrTotalCorruptedEvIds, 
        std::set<long long> &rsetCorruptedEvIds, std::vector<long long>* pvectRowIDs, std::string& rStrRsult)
    {
        for (bool bWorking = pCursor->MoveToFirst(); bWorking; bWorking = pCursor->MoveToNext())
        {
            long long llId = pCursor->GetLong(pCursor->GetColumnIndex(ic_core::CDataBaseConst::COL_ID));
            std::string strEventData = pCursor->GetString(pCursor->GetColumnIndex(ic_core::CDataBaseConst::COL_EVENTS));
            std::string strEventType = pCursor->GetString(pCursor->GetColumnIndex(ic_core::CDataBaseConst::COL_EVENT_ID));
            long long llTimestamp = pCursor->GetLong(pCursor->GetColumnIndex(ic_core::CDataBaseConst::COL_TIMESTAMP));
            strEventData = DecryptEventData(strEventData);

            ic_utils::Json::Value jsonEvent;
            ic_utils::Json::Reader jsonReader;
            // Making sure the decryption is success and event json is parseable
            if (strEventData.empty() || !(jsonReader.parse(strEventData,jsonEvent)))
            {
                // Add the corrupted event's id to the set to delete them in one shot
                HCPLOG_E << "Corrupted event(id=" << llId << "):" << strEventData;
                rsetCorruptedEvIds.insert(llId);

                AddCorruptedEventToList(llId,rstrTotalCorruptedEvIds);
                continue;
            }

            strEventData.append(",");
            rStrRsult.append(strEventData);
            pvectRowIDs->push_back(llId);

            rStrEvent << "{\"" << strEventType<< "\":" << llTimestamp <<"},";
        }
    }

    /**
     * Method to update json formatted result string containing events/alerts
     * @param[in/out] rstrResult String containing events
     * @return void
     */
    static void UpdateFinalResultJsonStr(std::string &rstrResult)
    {
        /* If any records are fetched, replace the last character ',' with ']'
         * If no records are fetched, just suffix the result string with ']'
         */ 

        if (rstrResult.size() > 1)
        {
            rstrResult.replace(rstrResult.size() - 1, 1, "]");
        }
        else
        {
            rstrResult += "]";
        }
    }

    /**
     * Method to update log string for stream upload mode events
     * @param[in/out] rstrLog String to be updated for logging purpose
     * @return void
     */
    static void UpdateStreamEventUploadLogStr(std::string &rstrLog)
    {
        if (rstrLog.length() > 10)
        {
            rstrLog.erase(rstrLog.size() - 1, 1);
        }
        rstrLog.append("]");
    }

    /**
     * Method to get selection for streaming events from db
     * @param[in] rstrTable table name
     * @param[in] rsetCorruptedEvIds Set of corrupted eventIds
     * @return void
     */
    static void DeleteCorruptedEventsIfPresent(const std::string &rstrTable, std::set<long long> &rsetCorruptedEvIds)
    {
        // If still there are corrupted events, delete them.
        if (rsetCorruptedEvIds.size() > 0)
        {
            DeleteCorruptedEvents(rstrTable, rsetCorruptedEvIds);
        }
    }

    /**
     * Method to get selection for streaming events from db
     * @param[in] rstrTable Table name
     * @param[in] rllLimitStartTimestamp Get the events from DB starting from given timestamp
     * @param[in] rllLimitEndTimestamp Get the events from DB until given timestamp
     * @param[in] rllAfterId Get the events from DB before given id
     * @param[in] rllBeforeId Get the events from DB after given id
     * @return Selection string for streaming events
     */
    static std::string GetSelectionForStreamingEvents(const std::string &rstrTable,
                                         long long& rllLimitStartTimestamp,
                                         long long& rllLimitEndTimestamp,
                                         long long& rllAfterId,
                                         long long& rllBeforeId)
    {
        std::string strSelection(ic_core::CDataBaseConst::COL_TIMESTAMP + " IS NOT NULL");

        if (rllAfterId !=-1)
        {
            strSelection += " AND " + ic_core::CDataBaseConst::COL_ID + " > " + ic_utils::CIgniteStringUtils::NumberToString(rllAfterId);
        }

        if (rllBeforeId !=-1)
        {
            strSelection += " AND " + ic_core::CDataBaseConst::COL_ID + " < " + ic_utils::CIgniteStringUtils::NumberToString(rllBeforeId);
        }

        if (rllLimitEndTimestamp > 0)
        {
            strSelection += " AND " + ic_core::CDataBaseConst::COL_TIMESTAMP + " <= " + ic_utils::CIgniteStringUtils::NumberToString(rllLimitEndTimestamp);
        }

        if (rllLimitStartTimestamp > 0)
        {
            strSelection += " AND " + ic_core::CDataBaseConst::COL_TIMESTAMP + " >= " + ic_utils::CIgniteStringUtils::NumberToString(rllLimitStartTimestamp);
        }

        strSelection += " AND " + ic_core::CDataBaseConst::COL_MID + " == 0";
		strSelection += " AND " + ic_core::CDataBaseConst::COL_TOPIC + " IS NULL";

        if(rstrTable == ic_core::CDataBaseConst::TABLE_EVENT_STORE)
        {
            strSelection += " AND " + ic_core::CDataBaseConst::COL_STREAM_SUPPORT + " == 1";
        }

        return strSelection;
    }

    /**
     * Method to add corrupted event to the corrupted event list 
     * @param[in] rllId id of the event from db
     * @param[out] rstrTotalCorruptedEvIds - string list of corrupted event ids
     * @return void
     */
    static void AddCorruptedEventToList(const long long& rllId,std::string &rStrTotalCorruptedEvIdsStr)
    {
        //add the corrupted event to the list to exclude them from next query
        if (rStrTotalCorruptedEvIdsStr.empty())
        {
            rStrTotalCorruptedEvIdsStr = ic_utils::CIgniteStringUtils::NumberToString(rllId);
        }
        else
        {
            rStrTotalCorruptedEvIdsStr += "," + ic_utils::CIgniteStringUtils::NumberToString(rllId);
        }
    }

    /**
     * Method to validate corrupted event ids
     * @param[in] rstrTable Table name
     * @param[out] rsetCorruptedEvIds Set of corrupted event ids
     * @param[out] rstrTotalCorruptedEvIds List of toal corrupted event ids
     * @return void
     */
    static void ValidateCorruptedEventIds(const std::string &rstrTable, std::set<long long>& rsetCorruptedEvIds,
        std::string &rstrTotalCorruptedEvIds)
    {
        /* If corrupted event ids are more than the limit, delete them
         * immediately to avoid huge query later
         */
        if (rsetCorruptedEvIds.size() >= NO_OF_CORRUPTED_EVENTS_TO_DELETE_IN_SINGLE_QUERY) 
        {
            DeleteCorruptedEvents(rstrTable, rsetCorruptedEvIds);

            //reset the set
            rsetCorruptedEvIds.clear();
            rstrTotalCorruptedEvIds = "";
        }
    }

    /**
     * Method to check if fetched records size is zero
     * @param[out] pvectRowIds Vector of row ids
     * @return True if size of rowIds is greater than zero, false otherwise
     */
    static bool IsFetchedRecordSizeZero(std::vector<long long>* pvectRowIds)
    {
        bool bIsRecordSizeZero = true;
        if (pvectRowIds->size() > 0)
        {
            bIsRecordSizeZero = false;
        }
        return bIsRecordSizeZero;
    }

};//class UploadUtils
}//namespace ic_bl
#endif /* CUPLOAD_UTILS_H */
