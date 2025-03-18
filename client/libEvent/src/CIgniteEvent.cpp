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

#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <iomanip>
#include "CIgniteEvent.h"
#include "CIgniteEventSender.h"
#include "CIgniteDateTime.h"
#include "CIgniteFileUtils.h"
#include "CIgniteLog.h"
#include "CIgniteMessage.h"
#include "CClientInfo.h"
#include "jsoncpp/json.h"
#include "EventLibVersion.h"

#ifdef PREFIX
#undeef PREFIX
#endif
#define PREFIX "CIgniteEvent"

using std::string;

namespace ic_event 
{

//max length of final attachment file.  Reserve 3 chars for ".gz"
const unsigned int MAX_ATTACHMENT_FINAL_FILE_LEN =
        ic_utils::CIgniteFileUtils::MAX_FILENAME_LENGTH-3;

//minimum length of truncated file
const unsigned int TRUNCATE_MIN_LEN = 16;

//static member declaration
bool CIgniteEvent::bBenchMode = false;

namespace 
{
# define TZ_MIN_OFFSET -720 //UTC-12
# define TZ_MAX_OFFSET 840  //UTC+14

const std::string CONFIG_FILE_ATTACHMENT_PATH_KEY_ID = "DAM.Upload.CurlSender.fileAttachmentPath";
const std::string CONFIG_FILE_ATTACHMENT_TEMP_PATH_KEY_ID =
    "DAM.Upload.CurlSender.fileAttachmentTempPath";
const std::string CONFIG_FILE_ATTACHMENT_FILE_SIZE_LIMIT_KEY_ID =
    "DAM.Upload.CurlSender.fileAttachmentFileSizeLimit";
const std::string CONFIG_FILE_ATTACHMENT_TOTAL_SIZE_LIMIT_KEY_ID =
    "DAM.Upload.CurlSender.fileAttachmentTotalSizeLimit";
const std::string CONFIG_FILE_DELETE_SRC_ATTACHMENT_AFTER_ACCEPT =
                "DAM.Upload.CurlSender.deleteSrcAttachmentFileAfterAccept";
static const std::string DEFAULT_ATTACHMENT_TEMP_PATH = "/tmp";
static const int MAX_ATTACHMENT_LIMIT = 99;

} /* namespace */

CIgniteEvent::CIgniteEvent()
{
    HCPLOG_METHOD();
    HCPLOG_T << "libAcpEvent.so version = " << EVENT_LIB_VERSION;
    m_jsonValue = ic_utils::Json::objectValue;
    m_jsonPiiFields = ic_utils::Json::objectValue;
    m_nAttachmentCount = 0;
    m_jsonEventFields[TIMESTAMP_TAG] = ic_utils::CIgniteDateTime::GetCurrentTimeMs();
    m_jsonEventFields[TIMEZONE_TAG] = ic_utils::CIgniteDateTime::GetTimeZoneOffsetMinutes();
}

CIgniteEvent::CIgniteEvent(const string& rstrVersion, const string& rstrEventId, const double dblTimestamp)
{
    HCPLOG_METHOD();
    HCPLOG_T << "libAcpEvent.so version = " << EVENT_LIB_VERSION;
    m_jsonValue = ic_utils::Json::objectValue;
    m_jsonPiiFields = ic_utils::Json::objectValue;
    m_nAttachmentCount = 0;
    m_jsonEventFields[EVENT_ID_TAG] = rstrEventId;
    m_jsonEventFields[VERSION_TAG] = rstrVersion;
    m_jsonEventFields[TIMESTAMP_TAG] = dblTimestamp;
    m_jsonEventFields[TIMEZONE_TAG] = ic_utils::CIgniteDateTime::GetTimeZoneOffsetMinutes();
}

CIgniteEvent::CIgniteEvent(const string& rstrVersion, const string& rstrEventId)
{
    HCPLOG_METHOD();
    HCPLOG_T << "libAcpEvent.so version = " << EVENT_LIB_VERSION;
    m_jsonValue = ic_utils::Json::objectValue;
    m_jsonPiiFields = ic_utils::Json::objectValue;
    m_nAttachmentCount = 0;

    m_jsonEventFields[EVENT_ID_TAG] = rstrEventId;
    m_jsonEventFields[VERSION_TAG] = rstrVersion;
    m_jsonEventFields[TIMESTAMP_TAG] = ic_utils::CIgniteDateTime::GetCurrentTimeMs();
    m_jsonEventFields[TIMEZONE_TAG] = ic_utils::CIgniteDateTime::GetTimeZoneOffsetMinutes();
}

CIgniteEvent::~CIgniteEvent()
{
    HCPLOG_METHOD();
}

void CIgniteEvent::AddBizTransaction(const std::string& rstrVal)
{    
    m_jsonEventFields[BIZTRANCID] = rstrVal;
}

void CIgniteEvent::AddMessageId(const std::string& rstrVal)
{
    m_jsonEventFields[MSGID] = rstrVal;
}

void CIgniteEvent::AddCorrelationId(const std::string& rstrVal)
{
    m_jsonEventFields[CORID] = rstrVal;
}

void CIgniteEvent::AddDeviceId(const std::string& rstrName, const std::string& rstrVal)
{
    m_jsonEventFields[rstrName] = rstrVal;
}

void CIgniteEvent::AddField(const string& rstrName, const int nVal)
{
    HCPLOG_METHOD() << rstrName << " " << nVal;
    m_jsonValue[rstrName] = nVal;
}

void CIgniteEvent::AddField(const string& rstrName, const long long llVal)
{
    HCPLOG_METHOD() << rstrName << " " << llVal;
    m_jsonValue[rstrName] = llVal;
}

void CIgniteEvent::AddField(const string& rstrName, const double dblVal)
{
    HCPLOG_METHOD() << rstrName << " " << dblVal;
    m_jsonValue[rstrName] = dblVal;
}

void CIgniteEvent::AddField(const string& rstrName, const string& rstrVal)
{
    HCPLOG_METHOD() << rstrName << " " << rstrVal;
    m_jsonValue[rstrName] = rstrVal;
}

void CIgniteEvent::AddField(const string& rstrName, const bool bVal)
{
    HCPLOG_METHOD() << rstrName << " " << bVal;
    m_jsonValue[rstrName] = bVal;
}

void CIgniteEvent::AddField(const string& rstrName, const char* pchVal)
{
    HCPLOG_METHOD() << rstrName << " " << pchVal;
    std::string sVal = std::string(pchVal);
    m_jsonValue[rstrName] = sVal;
}

void CIgniteEvent::AddFieldAsRawJsonString(const std::string& rstrName, const std::string& rstrVal)
{
    HCPLOG_METHOD() << rstrName << " " << rstrVal;
    ic_utils::Json::Value jsonRoot;
    ic_utils::Json::Reader jsonReader;
    if(jsonReader.parse(rstrVal, jsonRoot))
    {
        m_jsonValue[rstrName] = jsonRoot;
    }
    else
    {
        HCPLOG_E << "Could not parse the event string " << rstrVal;
    }
}

void CIgniteEvent::AddFieldAsRawJsonString(const std::string& rstrName, const char* pchVal)
{
    HCPLOG_METHOD() << rstrName << " " << pchVal;
    ic_utils::Json::Value jsonRoot;
    ic_utils::Json::Reader jsonReader;
    if(jsonReader.parse(pchVal, jsonRoot))
    {
        m_jsonValue[rstrName] = jsonRoot;
    }
    else
    {
        HCPLOG_E << "Could not parse the event string " << pchVal;
    }
}

void CIgniteEvent::AddField(const std::string& rstrName, ic_utils::Json::Value jsonVal)
{
    HCPLOG_METHOD() << rstrName << " " << jsonVal;
    m_jsonValue[rstrName] = jsonVal;
}

void CIgniteEvent::RemoveField(const string& rstrFieldName)
{
    m_jsonValue.removeMember(rstrFieldName);
}

// deserialize a json string to event based on the format defined in CIgniteEvent:: toJson
void CIgniteEvent::JsonToEvent(const std::string& rstrJsonEvent)
{
    ic_utils::Json::Value jsonRoot;
    ic_utils::Json::Reader jsonReader;
    jsonReader.parse(rstrJsonEvent, jsonRoot);

    m_jsonEventFields[VERSION_TAG] = jsonRoot[VERSION_TAG].asString();
    m_jsonEventFields[EVENT_ID_TAG] = jsonRoot[EVENT_ID_TAG].asString();
    m_jsonEventFields[TIMESTAMP_TAG] = jsonRoot[TIMESTAMP_TAG].asUInt64();

    if (jsonRoot.isMember(TIMEZONE_TAG))
    {
        m_jsonEventFields[TIMEZONE_TAG] = jsonRoot[TIMEZONE_TAG].asInt();
    }

    if (jsonRoot.isMember(FILE_ATTACHMENT_TAG) && jsonRoot[FILE_ATTACHMENT_TAG].isArray())
    {
        ic_utils::Json::Value array = jsonRoot[FILE_ATTACHMENT_TAG];
        for (unsigned int i = 0; i < array.size(); i++)
        {
            m_vectAttachment.push_back(array[i].asString());
        }
    }

    m_jsonValue = jsonRoot[VALUE_TAG];

    if (jsonRoot.isMember(PII_TAG))
    {
        m_jsonPiiFields = jsonRoot[PII_TAG];
    }

    if (jsonRoot.isMember(BIZTRANCID)) 
    {
        m_jsonEventFields[BIZTRANCID] = jsonRoot[BIZTRANCID].asString();
    }

    if (jsonRoot.isMember(CORID)) 
    {
        m_jsonEventFields[CORID] = jsonRoot[CORID].asString();
    }

    if (jsonRoot.isMember(MSGID)) 
    {
        m_jsonEventFields[MSGID] = jsonRoot[MSGID].asString();
    }
}

void CIgniteEvent::AttachFile(const string& rstrFileType, const string& rstrPath, const bool bCompressFile, const bool bDeleteAttachment)
{
    AttachRequest req;
    req.strType = rstrFileType;
    req.strPath = rstrPath;
    req.bCompress = bCompressFile;
    req.bDeleteAttachment = bDeleteAttachment;
    req.nStatusCode = ERROR_NOFILE;
    m_vectorAttachReqs.push_back(req);
}

void CIgniteEvent::GetFileNameAndExtension(const std::string &rstrPath, std::string &rstrExtn, std::string &rstrFilename)
{
    size_t nFound;

    if ( (nFound = rstrPath.find_last_of("/") ) != string::npos)
    {
        size_t last = rstrPath.rfind('.', rstrPath.length());
        if (last != string::npos)
        {
            rstrExtn = rstrPath.substr(last + 1, rstrPath.length() - last);
            rstrFilename = rstrPath.substr(nFound + 1, last - (nFound + 1));
            HCPLOG_LINE() << " - Determined file extension= " << rstrExtn << "; strFileName=" << rstrFilename;
        }
        else
        {
            //filename without extension
            rstrFilename = rstrPath.substr(nFound + 1, rstrPath.length() - nFound);
            HCPLOG_LINE() << " - file without extension. strFileName= " << rstrFilename;
        }
    }
}

bool CIgniteEvent::SetupFileAttachmentPath(std::string &rstrAttachTempPath)
{
    bool bSetupStatus = false;

    if (rstrAttachTempPath.empty())
    {
        rstrAttachTempPath = DEFAULT_ATTACHMENT_TEMP_PATH;
    }
    if (!ic_utils::CIgniteFileUtils::Exists(rstrAttachTempPath))
    {
        bSetupStatus =
                ic_utils::CIgniteFileUtils::MakeDirectory(rstrAttachTempPath);
    }

    return bSetupStatus;
}

std::string CIgniteEvent::SetupCompressedFilePath(
                                const std::string &rstrAttachTempPath,
                                const std::string &rstrFileName,
                                const std::string &rstrFileExtension)
{
    std::string strCompressedPath = "";
    if (!rstrFileExtension.empty())
    {
        strCompressedPath = rstrAttachTempPath + "/" + rstrFileName + "." + rstrFileExtension + ".gz";
    }
    else
    {
        strCompressedPath = rstrAttachTempPath + "/" + rstrFileName + ".gz";

    }

    return strCompressedPath;
}

bool CIgniteEvent::CompressFile(const bool &rbCompressFile, const std::string &rstrPath,
                    const std::string &strCompressedPath, int &rnNewAttachSize)
{
    bool bIsCompressed = false;
    if (rbCompressFile && !ic_utils::CIgniteFileUtils::Compress(rstrPath, strCompressedPath))
    {
        rnNewAttachSize = ic_utils::CIgniteFileUtils::GetSize(strCompressedPath);
        if (rnNewAttachSize > ic_utils::CIgniteFileUtils::GetSize(rstrPath) )
        {
            HCPLOG_LINE() << " - File expanded in Compression!";
            rnNewAttachSize = ic_utils::CIgniteFileUtils::GetSize(rstrPath); //Uncompressed original file
            ic_utils::CIgniteFileUtils::Remove(strCompressedPath); //delete the commpressed file
        }
        else
        {
            HCPLOG_LINE() << " - File Compressed less in size!";
            bIsCompressed = true;
        }
    }
    else //
    {
        // Determine file size and size of all existing attachments
        rnNewAttachSize = ic_utils::CIgniteFileUtils::GetSize(rstrPath);
    }

    return bIsCompressed;
}

int CIgniteEvent::GetExistingAttachmentSize(const std::string &rnstrFileAttachmentPath)
{
    std::vector<string> vectFileList;
    int nExistingAttachSize = 0;

    ic_utils::CIgniteFileUtils::ListFiles(rnstrFileAttachmentPath, vectFileList);
    if (vectFileList.size() != 0)
    {
        for (std::vector <string>::iterator it = vectFileList.begin(); it != vectFileList.end(); it++)
        {
            string itPath = rnstrFileAttachmentPath + "/" + *it;
            int itSize = ic_utils::CIgniteFileUtils::GetSize(itPath);
            HCPLOG_LINE() << " - nFound existing attachment file=" << itPath << ", size=" << itSize;
            nExistingAttachSize += itSize;
        }
    }

    return nExistingAttachSize;
}

int CIgniteEvent::ValidateOutFileName(
                                    const int &rnNewAttachSize,
                                    const int &rnExistingAttachSize,
                                    const int &rnFileSizeLimit,
                                    const int &rnTotalFileSizeLimit,
                                    const std::string &rstrFileType,
                                    const std::string &rstrEventId,
                                    std::string &rstrOutFileName)
{
    int nErrorCode = ATTACHMENT_OK;

    if(m_nAttachmentCount > MAX_ATTACHMENT_LIMIT)
    {
        rstrOutFileName += "_ATTACHLIMIT";
        nErrorCode = ERROR_ATTACHLIMIT;
    }
    else if(m_strFileAttachmentPath.empty())
    {
        rstrOutFileName += "_NOCONFIG";
        nErrorCode = ERROR_NOCONFIG;
    }
    else if(m_strDeviceId.empty())
    {
        rstrOutFileName += "_NODEVICEID";
        nErrorCode = ERROR_NODEVICEID;
    }
    else if(rnNewAttachSize > rnFileSizeLimit)
    {
        rstrOutFileName += "_FILETOOLARGE";
        nErrorCode = ERROR_FILETOOLARGE;
    }
    else if((rnNewAttachSize + rnExistingAttachSize) > rnTotalFileSizeLimit)
    {
        rstrOutFileName += "_TOTALOVERLIMIT";
        nErrorCode = ERROR_TOTALOVERLIMIT;
    }
    else if ((rstrFileType.find("_") != std::string::npos) ||
            (rstrEventId.find("_") != std::string::npos))
    {
        HCPLOG_D << "underscore rejected";
        rstrOutFileName += "_UNDERSCOREREJECTED";
        nErrorCode = ERROR_UNDERSCOREREJECTED;
    }
    else
    {
        //do nothing
    }
    return nErrorCode;
}

void CIgniteEvent::AddExtnToOutFileName(
                            const std::string &rstrFileExtension,
                            std::string &rstrOutputFileName)
{
    if (!rstrFileExtension.empty())
    {
        rstrOutputFileName += ".";
        rstrOutputFileName += rstrFileExtension;
    }
}

void CIgniteEvent::TruncateFileByExtn(std::string &rstrOutputFileName,
                                    std::string &rstrFileExtension)
{
    if (rstrOutputFileName.length() > MAX_ATTACHMENT_FINAL_FILE_LEN)
    {
        HCPLOG_LINE() << " - Filename length is " << rstrOutputFileName.length()
                        << ", truncating extension to 3 chars...filename="
                        << rstrOutputFileName;

        // First try to truncate file extension:
        if (rstrFileExtension.length() > 3)
        {
            rstrOutputFileName.resize(
                    rstrOutputFileName.length()-rstrFileExtension.length()+3);
            rstrFileExtension.resize(3);
        }
    }
}

void CIgniteEvent::TruncateFileByFileName(std::string &rstrOutputFileName,
        std::string &rstrTruncFileName,
        const std::string &rstrFileExtension,
        const std::string &rstrDeviceId,
        const std::string &rstrFileType,
        const std::string &rstrEventId,
        const std::string &rstrTimeConvert,
        const std::ostringstream &rstrstreamAttachCountConvert)
{
    if (rstrOutputFileName.length() > MAX_ATTACHMENT_FINAL_FILE_LEN)
    {
        int nOverBy = rstrOutputFileName.length() - MAX_ATTACHMENT_FINAL_FILE_LEN;
        if (rstrTruncFileName.length() > (TRUNCATE_MIN_LEN - 1))
        {
            rstrTruncFileName.resize(std::max((int)(TRUNCATE_MIN_LEN - 1),
                    (int)rstrTruncFileName.length() - nOverBy));
        }
        rstrOutputFileName = "v3_" + rstrDeviceId + "_" + rstrFileType + "_" + rstrEventId + "_"
                         + rstrTimeConvert + "_" + rstrstreamAttachCountConvert.str() + "_" + rstrTruncFileName;
        if (!rstrFileExtension.empty())
        {
            rstrOutputFileName += ".";
            rstrOutputFileName += rstrFileExtension;
        }
    }
}

void CIgniteEvent::TruncateFileByFileType(std::string &rstrOutputFileName,
        std::string &rstrTruncFileType,
        const std::string &rstrTruncFileName,
        const std::string &rstrFileExtension,
        const std::string &rstrDeviceId,
        const std::string &rstrTimeConvert,
        const std::ostringstream &rstrstreamAttachCountConvert)
{
    if (rstrOutputFileName.length() > MAX_ATTACHMENT_FINAL_FILE_LEN)
    {
        int nOverBy = rstrOutputFileName.length() - MAX_ATTACHMENT_FINAL_FILE_LEN;
        if (rstrTruncFileType.length() > TRUNCATE_MIN_LEN)
        {
            rstrTruncFileType.resize(std::max((int)TRUNCATE_MIN_LEN, (int)rstrTruncFileType.length() - nOverBy));
        }
        rstrOutputFileName = "v3_" + rstrDeviceId + "_" + rstrTruncFileType + "_" + rstrDeviceId + "_"
                         + rstrTimeConvert + "_" + rstrstreamAttachCountConvert.str() + "_" + rstrTruncFileName;
        if (!rstrFileExtension.empty())
        {
            rstrOutputFileName += ".";
            rstrOutputFileName += rstrFileExtension;
        }
    }
}

void CIgniteEvent::TruncateFileByEventName(std::string &rstrOutputFileName,
        std::string &rstrTruncEventName,
        const std::string &rstrTruncFileName,
        const std::string &rstrFileExtension,
        const std::string &rstrTruncfileType,
        const std::string &rstrTimeConvert,
        const std::ostringstream &rstrstreamAttachCountConvert)
{
    if (rstrOutputFileName.length() > MAX_ATTACHMENT_FINAL_FILE_LEN)
    {
        int nOverBy = rstrOutputFileName.length() - MAX_ATTACHMENT_FINAL_FILE_LEN;
        if (rstrTruncEventName.length() > TRUNCATE_MIN_LEN)
        {
            rstrTruncEventName.resize(std::max((int)TRUNCATE_MIN_LEN, (int)rstrTruncEventName.length() - nOverBy));
        }
        rstrOutputFileName = "v3_" + m_strDeviceId + "_" + rstrTruncfileType + "_" + rstrTruncEventName + "_"
                         + rstrTimeConvert + "_" + rstrstreamAttachCountConvert.str() + "_" + rstrTruncFileName;
        if (!rstrFileExtension.empty())
        {
            rstrOutputFileName += ".";
            rstrOutputFileName += rstrFileExtension;
        }
    }
}

void CIgniteEvent::FinalizeOutFileName(std::string &rstrOutputFileName,
        std::string &rstrFinalOutFileName,
        int &rnErrorCode,
        const bool &rbIsCompressed,
        const std::string &rstrCfgDeleteAttachment,
        const std::string &rstrCompressedPath,
        const std::string &rstrFileAttachmentPath,
        const bool &rbDeleteAttachment,
        const std::string &rstrPath)
{
    if (rstrOutputFileName.length() <= MAX_ATTACHMENT_FINAL_FILE_LEN)
    {
        int nRet = -1;
        // Yay! We made it!
        HCPLOG_LINE() << " - Filename length is OK!: " << rstrOutputFileName.length();
        if (rbIsCompressed) //compression successful
        {
            rstrOutputFileName += ".gz";
            nRet =ic_utils::CIgniteFileUtils::Copy(rstrCompressedPath, rstrFileAttachmentPath + "/" +
                               rstrOutputFileName);
        }
        else
        {
            nRet = ic_utils::CIgniteFileUtils::Copy(rstrPath, rstrFileAttachmentPath + "/" + rstrOutputFileName);
        }
        rstrFinalOutFileName = rstrOutputFileName;
        m_nAttachmentCount++;

        /* Attachment delete scenarios.
         * Delete the attachment only if
         *     CONFIG_FILE_DELETE_SRC_ATTACHMENT_AFTER_ACCEPT setting is yes &
         *     deleteAttachment parameeter is true.

         */

        if (!rstrCfgDeleteAttachment.empty())
        {
            HCPLOG_LINE() << "Delete File Attachment - Config setting = " << rstrCfgDeleteAttachment;
            if( rstrCfgDeleteAttachment.compare("yes")==0)
            {
                if (rbDeleteAttachment && (nRet == 0))
                {
                    ic_utils::CIgniteFileUtils::Remove(rstrPath);
                    HCPLOG_LINE() << "Deleting attachment. Path: " << rstrPath;
                }
            }
        }
    }
    else
    {
        HCPLOG_LINE() << " - We still have a filename length error.  length="
                      << rstrOutputFileName.length() << ". This shouldn't be possible so I'm giving up!";
        rstrFinalOutFileName += "_FILELENGTHLIMIT";
        rnErrorCode = ERROR_FILELENGTHLIMIT;
    }
}

int CIgniteEvent::AcceptAttach(const string& rstrFileType, const string& rstrPath, const bool bCompressFile, bool bDeleteAttachment)
{
    HCPLOG_METHOD() << " fileType=" << rstrFileType << " path=" << rstrPath << " compressFile=" << bCompressFile
                                       << " deleteAttachment=" << bDeleteAttachment;
    string strOutFileName = rstrPath;
    std::string strEventId = m_jsonEventFields[EVENT_ID_TAG].asString();
    int nErrorCode = ATTACHMENT_OK;

    if (ic_utils::CIgniteFileUtils::Exists(rstrPath))
    {
        CClientInfo info;

        if (m_strFileAttachmentPath.empty())
        {
            m_strFileAttachmentPath = info.GetConfigValue(CONFIG_FILE_ATTACHMENT_PATH_KEY_ID);
        }

        if (m_strFileAttachmentPath.empty())
        {
            HCPLOG_LINE() << " - Attachment path is not configured, ignoring attach file request";
            return ERROR_NOCONFIG;
        }
        HCPLOG_LINE() << " - Got file attachment path of " << m_strFileAttachmentPath;

        m_strDeviceId = info.GetDeviceId();
        HCPLOG_LINE() << " - Got Device ID (" << m_strDeviceId << ") of " << m_strFileAttachmentPath;

        // Determine file extension of original file
        string strFileExtension;
        string strFileName;

        GetFileNameAndExtension(rstrPath, strFileExtension, strFileName);

        if (!strFileName.empty())
        {
            bool bIsCompressed = false;
            int nNewAttachSize = 0;

            std::string strAttachTempPath = info.GetConfigValue(CONFIG_FILE_ATTACHMENT_TEMP_PATH_KEY_ID);

            SetupFileAttachmentPath(strAttachTempPath);

            std::string strCompressedPath =
                    SetupCompressedFilePath(strAttachTempPath, strFileName, strFileExtension);
            HCPLOG_LINE() << "srcPath = " << rstrPath << "; strCompressedPath= " << strCompressedPath ;

            bIsCompressed = CompressFile(bCompressFile, rstrPath, strCompressedPath,nNewAttachSize);
            HCPLOG_LINE() << " - New file attachment size=" << nNewAttachSize;

            int nExistingAttachSize =
                    GetExistingAttachmentSize(m_strFileAttachmentPath);
            HCPLOG_LINE() << " - Total existing attachment size=" << nExistingAttachSize;

            int nFileSizeLimit =
                    std::atoi(info.GetConfigValue(
                          CONFIG_FILE_ATTACHMENT_FILE_SIZE_LIMIT_KEY_ID).c_str());

            int nTotalFileSizeLimit =
                    std::atoi(info.GetConfigValue(
                         CONFIG_FILE_ATTACHMENT_TOTAL_SIZE_LIMIT_KEY_ID).c_str());

            nErrorCode=ValidateOutFileName(nNewAttachSize,nExistingAttachSize,
                            nFileSizeLimit, nTotalFileSizeLimit,
                            rstrFileType, strEventId, strOutFileName);

            if (ATTACHMENT_OK == nErrorCode)
            {
                // Get event timestamp as string
                std::string strTimeConvert = m_jsonEventFields[TIMESTAMP_TAG].asString();

                // Get attachment count as string
                std::ostringstream strAttachCountConvert;
                strAttachCountConvert << m_nAttachmentCount;

                // Build up filename
                std::string strOutputFileName = "v3_" + m_strDeviceId + "_" + rstrFileType + "_" + strEventId + "_"
                                             + strTimeConvert + "_" + strAttachCountConvert.str() + "_" + strFileName;

                AddExtnToOutFileName(strFileExtension, strOutputFileName);

                //Check filename length and truncate as needed to keep globally unique filename.
                TruncateFileByExtn(strOutputFileName, strFileExtension);

                // If still over, truncate file name then file type then event name and rebuild name:
                std::string strTruncFileName = strFileName;
                TruncateFileByFileName(strOutputFileName,
                        strTruncFileName,
                        strFileExtension,
                        m_strDeviceId,
                        rstrFileType,
                        strEventId,
                        strTimeConvert,
                        strAttachCountConvert);

                // If still over, truncate by file type
                std::string strTruncFileType = rstrFileType;
                TruncateFileByFileType(strOutputFileName,
                        strTruncFileType,
                        strTruncFileName,
                        strFileExtension,
                        m_strDeviceId,
                        strTimeConvert,
                        strAttachCountConvert);

                // If still over, truncate by event name
                std::string rstrTruncEventName = strEventId;
                TruncateFileByEventName(strOutputFileName,
                        rstrTruncEventName,
                        strTruncFileName,
                        strFileExtension,
                        strTruncFileType,
                        strTimeConvert,
                        strAttachCountConvert);

                std::string strCfgDeleteAttachment =
                        info.GetConfigValue(CONFIG_FILE_DELETE_SRC_ATTACHMENT_AFTER_ACCEPT);

                FinalizeOutFileName(
                    strOutputFileName,
                    strOutFileName,
                    nErrorCode,
                    bIsCompressed,
                    strCompressedPath,
                    strCfgDeleteAttachment,
                    m_strFileAttachmentPath,
                    bDeleteAttachment,
                    rstrPath);
            }
            //Deletes the compressed file
            if (bIsCompressed)
            {
                ic_utils::CIgniteFileUtils::Remove(strCompressedPath);
            }
        }
        else
        {
            strOutFileName += "_NOFILENAME";
            nErrorCode = ERROR_NOFILENAME;
        }
    }
    else
    {
        strOutFileName += "_NOFILE";
        nErrorCode = ERROR_NOFILE;
    }

    m_vectAttachment.push_back(strOutFileName);
    system("sync");
    return nErrorCode;
}

string CIgniteEvent::GetEventId()
{
    //return m_event_id;
    return m_jsonEventFields[EVENT_ID_TAG].asString();
}

double CIgniteEvent::GetTimestamp()
{
    return m_jsonEventFields[TIMESTAMP_TAG].asDouble();
}

void CIgniteEvent::SetTimestamp(const double dblTimestamp)
{
    m_jsonEventFields[TIMESTAMP_TAG] = dblTimestamp;
}

void CIgniteEvent::SetTimezone (const int nTzoffset)
{
    if (nTzoffset >= TZ_MIN_OFFSET && nTzoffset <= TZ_MAX_OFFSET)
    {
        m_jsonEventFields[TIMEZONE_TAG] = nTzoffset;
    }
    else
    {
        HCPLOG_E << " Invalid Timezone Offset!!";
    }
}

void CIgniteEvent::SetVersion(const string& rstrVersion)
{
    m_jsonEventFields[VERSION_TAG] = rstrVersion;
}

string CIgniteEvent::GetVersion()
{
    return m_jsonEventFields[VERSION_TAG].asString();
}

bool CIgniteEvent::operator == (CIgniteEvent& event)
{
    std::string strEventid = m_jsonEventFields[EVENT_ID_TAG].asString();
    std::string strCEventId = event.GetEventId();
    return (strEventid.compare(strCEventId) == 0 && m_jsonValue == event.m_jsonValue);
}

bool CIgniteEvent::operator != (CIgniteEvent& event)
{
    return !(*this == event);
}

void CIgniteEvent::EventToJson(string& json)
{
    ic_utils::Json::Value jsonRoot;
    jsonRoot[EVENT_ID_TAG] = m_jsonEventFields[EVENT_ID_TAG];
    jsonRoot[VERSION_TAG] = m_jsonEventFields[VERSION_TAG];
    if(bBenchMode)
    {
        jsonRoot[MODE_TAG] = 1;
    }

    jsonRoot[TIMESTAMP_TAG] = m_jsonEventFields[TIMESTAMP_TAG];

    ic_utils::Json::Value jsonFileArray;
    if (!m_vectAttachment.empty())
    {
        std::vector<string>::iterator it = m_vectAttachment.begin();
        string strName = *it;
        jsonFileArray.append(strName);

        while (++it != m_vectAttachment.end())
        {
            strName = *it;
            if(strName.find("v3_") == 0)
            {
                jsonFileArray.append(strName);
            }
            else
            {
                ExtractName(strName);
            }
        }
        jsonRoot[FILE_ATTACHMENT_TAG] = jsonFileArray;
    }

    jsonRoot[VALUE_TAG] = m_jsonValue;

    if (!m_jsonPiiFields.empty())
    {
        jsonRoot[PII_TAG] = m_jsonPiiFields;
    }

    jsonRoot[TIMEZONE_TAG] = m_jsonEventFields[TIMEZONE_TAG];

    if (m_jsonEventFields.isMember(BIZTRANCID)) 
    {
        jsonRoot[BIZTRANCID] = m_jsonEventFields[BIZTRANCID];
    }

    if (m_jsonEventFields.isMember(MSGID)) 
    {
        jsonRoot[MSGID] = m_jsonEventFields[MSGID];
    }

    if (m_jsonEventFields.isMember(CORID)) 
    {
        jsonRoot[CORID] = m_jsonEventFields[CORID];
    }

    ic_utils::Json::FastWriter jsonWriter;
    json += jsonWriter.write(jsonRoot);
    HCPLOG_T << "JSON: " << json;
}


void CIgniteEvent::ExtractName(string& rstrExtractName)
{
    std::string strStr2 ("_");
    std::size_t nFound = rstrExtractName.find(strStr2);
    string strFileName = rstrExtractName.substr(0, nFound );
    string fileError = rstrExtractName.substr(nFound);
    m_jsonValue[strFileName] = fileError;
}

/* Sends the event to the Insight Engine.
 * Returns an error code. Error code 0 means success (no errors)
 */
int CIgniteEvent::Send()
{
    HCPLOG_METHOD();
    
    ProcessAttachment();

    string strSerialized;
    EventToJson(strSerialized);
    int nRet = -1;
    if(eventSender)
    {
        HCPLOG_T << "   Sending event:" << strSerialized;
        nRet = eventSender->Send(strSerialized);
    }
    return nRet;
}

void CIgniteEvent::SetSender(IEventSender* pSender)
{
    HCPLOG_METHOD();
    CIgniteEvent:: eventSender = pSender;
}

IEventSender* CIgniteEvent::eventSender = new CIgniteEventSender();

std::string CIgniteEvent::GetBizTransactionId()
{    
    if(m_jsonEventFields.isMember(BIZTRANCID))
    {
        return m_jsonEventFields[BIZTRANCID].asString();
    }
    return "";
}

std::string CIgniteEvent::GetMessageId()
{
    if(m_jsonEventFields.isMember(MSGID))
    {
        return m_jsonEventFields[MSGID].asString();
    }
    return "";
}

std::string CIgniteEvent::GetCorrelationId()
{
    if(m_jsonEventFields.isMember(CORID))
    {
        return m_jsonEventFields[CORID].asString();
    }
    return "";
}

int CIgniteEvent::GetInt(const std::string& rstrKey, int nDefaultValue)
{
    if (m_jsonValue.isMember(rstrKey))
    {
        return m_jsonValue[rstrKey].asInt();
    }

    return nDefaultValue;
}

bool CIgniteEvent::GetBool(const std::string& rstrKey, bool bDefaultValue)
{
    if (m_jsonValue.isMember(rstrKey))
    {
        return m_jsonValue[rstrKey].asBool();
    }

    return bDefaultValue;
}

std::string CIgniteEvent::GetString(const std::string& rstrKey, std::string strDefaultValue)
{
    if (m_jsonValue.isMember(rstrKey))
    {
        return m_jsonValue[rstrKey].asString();
    }

    return strDefaultValue;
}

long long CIgniteEvent::GetLong(const std::string& rstrKey, long long llDefaultValue)
{
    if (m_jsonValue.isMember(rstrKey))
    {
#if defined(JSON_HAS_INT64)
        return m_jsonValue[rstrKey].asInt64();
#else
        return m_jsonValue[rstrKey].asLong();
#endif
    }

    return llDefaultValue;
}

double CIgniteEvent::GetDouble(const std::string& rstrKey, double dblDefaultValue)
{
    if (m_jsonValue.isMember(rstrKey))
    {
        return m_jsonValue[rstrKey].asDouble();
    }

    return dblDefaultValue;
}

ic_utils::Json::Value CIgniteEvent::GetJson(const std::string &rstrKey, ic_utils::Json::Value jsonDefaultValue)
{
    if(m_jsonValue.isMember(rstrKey))
    {
        return  m_jsonValue[rstrKey];
    }

    return jsonDefaultValue;
}

ic_utils::Json::Value CIgniteEvent::GetData() 
{
    return m_jsonValue;
}

void CIgniteEvent::ProcessAttachment() 
{
    int nErrorCode = 0;
    for (int i = 0; i < m_vectorAttachReqs.size(); i++)
    {
        nErrorCode = AcceptAttach(m_vectorAttachReqs[i].strType,
            m_vectorAttachReqs[i].strPath, m_vectorAttachReqs[i].bCompress,
            m_vectorAttachReqs[i].bDeleteAttachment);
        HCPLOG_T<< "acceptAttach:Return:" << nErrorCode;
        m_vectorAttachReqs[i].nStatusCode = nErrorCode;
    }
}

/* Function : getattachFileStatus
 * Description : Returns status of the attachment updated in m_attach_reqs
 * Parameter : file with path
 * return : int.
 */
int CIgniteEvent::GetAttachFileStatus(const string& rstrPath)
{
    HCPLOG_METHOD()<< "getattachFileStatus for " << rstrPath;
    for (int i = 0; i < m_vectorAttachReqs.size(); i++)
    {
        if(rstrPath == m_vectorAttachReqs[i].strPath) 
        {
            HCPLOG_METHOD() << m_vectorAttachReqs[i].nStatusCode;
            return m_vectorAttachReqs[i].nStatusCode;
        }
    }
    return ERROR_NOFILE;//If file info is not nFound m_attach_reqs
}

void CIgniteEvent::ClearAttachMentDetails()
{
    HCPLOG_METHOD()<< "clearAttachDetails";
    m_vectorAttachReqs.clear();
}

void CIgniteEvent::ClearSender()
{
    if (eventSender) 
    {
        delete eventSender;
        eventSender = NULL;
    }
}

} /* namespace ic_event */
