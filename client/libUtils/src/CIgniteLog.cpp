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

#include <iostream>
#include <sstream>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdio.h>
#include <zlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <iterator>
#include "CIgniteLog.h"
#include "CIgniteFileUtils.h"
#include "CPreIgniteLogger.h"
#include "CIgniteMutex.h"
#include "CIgniteDateTime.h"

//! macro for log
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CIgniteLog"

/**
 * RESULT_SIZE used to set the maximum size for result-related buffers or arrays
 */
#define RESULT_SIZE 100

/** 
 * Following variable indicates max number of diagnostic info entries in the 
 * ignite log 
 */
static unsigned int MAX_DIAG_ENTRIES = 20;
 
/** 
 * Following variable indicates upper limit for MAX_DIAG_ENTRIES , used in set 
 * method 
 */
const int MAX_DIAG_ENTRIES_UPPER_LIMIT = 30;

//! Following variable indiactes lower limit for MAX_DIAG_ENTRIES 
const int MAX_DIAG_ENTRIES_LOWER_LIMIT = 5;

//! g_vectorDiagHeader store diagnostic headers as strings in a vector.
static std::vector<std::string> g_vectorDiagHeader;

namespace ic_utils 
{
/**
 * Mehtod to retrieve a formatted string representing the current time.
 * @param void
 * @return A string containing the formatted time.
 */
inline std::string get_time_formatted();

/**
 * Method to print a log message with a specified prefix.
 * @param log The log message to be printed.
 * @return void
 */
inline void print_pre_logger(const std::string &log);

//! m_eReportingLevel for the default reporting log level.
LogLevel CIgniteLog::m_eReportingLevel = eHCP_LOG_FATAL;

//! m_eFileOutputLevel for the default log level for file output.
LogLevel CIgniteLog::m_eFileOutputLevel = eHCP_LOG_NONE;

//! Mutex for controlling access to log-related operations.
CIgniteMutex CIgniteLog::m_logMutex;

//! Flag indicating whether log truncation is enabled.
bool CIgniteLog::m_bTruncateLog(true);

//! Output file stream used for logging.
std::ofstream CIgniteLog::m_of;

//! Size (in bytes) at which log truncation occurs.
unsigned long CIgniteLog::m_ulTruncateFileSize(0);

//! Target size (in bytes) to truncate the log file to.
unsigned long CIgniteLog::m_ulTruncateToSize(0);

//! File path where the log file is stored.
std::string CIgniteLog::m_strFilePath("");

//! Atomic variable to store the current size of the log file.
std::atomic<unsigned long> CIgniteLog::m_atomicFileSize(0);

//! Map to store status information in the log.
std::map<std::string, std::string> CIgniteLog::m_mapStatus;

//! Mutex for controlling access to the status map in the log.
ic_utils::CIgniteMutex CIgniteLog::m_statusMutex;

std::ostringstream& CIgniteLog::Get(LogLevel eLevel)
{
    m_eLogLevel = eLevel;
    m_os << get_time_formatted();
    m_os << " " << ToString(eLevel);
    m_os << " " << pthread_self();
    m_os << ": ";

    //m_os << std::string(eLevel > logDEBUG ? 0 : eLevel - logDEBUG, '\t');
    return m_os;
}

CIgniteLog::CIgniteLog()
{
    m_bIsLast = false;
}

CIgniteLog::CIgniteLog(bool bIsLast) : m_bIsLast(bIsLast)
{
    //do nothing
}

CIgniteLog::~CIgniteLog()
{
    m_os << std::endl;

    if (m_eLogLevel <= m_eReportingLevel)
    {
        fputs(m_os.str().c_str(),stdout);
        fflush(stdout);
    }

    if (m_eLogLevel <= m_eFileOutputLevel) 
    {
        if(m_of.is_open()) 
        {
            m_atomicFileSize += m_os.str().size();
            m_of << m_os.str();
            m_of.flush();
            TruncateIfRequired();

            if (m_bIsLast && m_of.is_open()) 
            {
                m_of.close();
            }
        }
    }

}

void CIgniteLog::CloseLogFile()
{
    ic_utils::CIgniteLog(true).Get(eHCP_LOG_CRITICAL) \
        << "****Closing the LogFile*******";
    
    //clear status to be printed when new file is created
    m_mapStatus.clear();
}

LogLevel& CIgniteLog::GetReportingLevel()
{
    return m_eReportingLevel;
}

void CIgniteLog::SetReportingLevel(const LogLevel& reLevel)
{
    if ( reLevel <= HCPLOG_MAX_LEVEL )
    {
        m_eReportingLevel = reLevel;
    }
}

LogLevel& CIgniteLog::GetFileOutputLevel()
{
    return m_eFileOutputLevel;
}

void CIgniteLog::SetFileOutputLevel(const LogLevel& reLevel)
{
    if ( reLevel <= HCPLOG_MAX_LEVEL )
    {
        m_eFileOutputLevel = reLevel;
    }
}
void CIgniteLog::EnableTruncate()
{
    m_bTruncateLog = true;
}

bool CIgniteLog::UpdateFilePath(const std::string& rstrNewPath)
{
    bool bResult = false;

    /* Check if path already set in CIgniteLog class and path sent as parameter  
     * are not same. If they are same then do nothing 
     */
    if((!rstrNewPath.empty() && (rstrNewPath != m_strFilePath)))
    {
        HCPLOG_C << "New file path :" << rstrNewPath;

        std::string strNewParentDir = rstrNewPath.substr(0, 
                                      rstrNewPath.find_last_of("/"));
        /* Check if directory exists with the strNewParentDir path, if not then 
         * create the directory 
         */
        if (!ic_utils::CIgniteFileUtils::Exists(strNewParentDir))
        {
            ic_utils::CIgniteFileUtils::MakeDirectory(strNewParentDir);
        }

        // Flush the contents to log file and close the log file
        CloseAndFlushLogFile();

        std::string strExistingParentDir = m_strFilePath.substr(0, \
                                           m_strFilePath.find_last_of("/"));
            
        /* Check if directory of previous opened log path and new path are not 
         * same, then move all log files to new Path else move previous log 
         * file to new path 
         */
        if(strExistingParentDir != strNewParentDir)
        {
            MoveLogFilesToNewPath(rstrNewPath);
        }
        else
        {
            // Move previous log file to new Path
            MoveFile(m_strFilePath, const_cast<std::string &>(rstrNewPath));
        }

        SetFileOutputPath(rstrNewPath, m_ulTruncateFileSize, 
                          m_ulTruncateToSize);
        bResult = true;
    }
    else
    {
        HCPLOG_E << "Update failed:new path:" << rstrNewPath  
        << " existing path:" << m_strFilePath;
    }

    return bResult;
}

void CIgniteLog::CloseAndFlushLogFile()
{
    if (m_of.is_open())
    {
        m_of.flush();
        m_of.close();
        print_pre_logger("Log file closed");
    }
}

void CIgniteLog::MoveLogFilesToNewPath(const std::string& rstrNewPath)
{
    std::string strDestPath = rstrNewPath;
    std::string strSrcbakfile = m_strFilePath + ".bak";
    std::string strDestbakfile = "";

    /* Check if file already exists in new Path then
    * move the old log file to new log file path 
    */
    if(ic_utils::CIgniteFileUtils::Exists(rstrNewPath))
    {
        std::string strMvfileInSameDir = rstrNewPath + ".bak";

        /* Check if .bak file exists in old Path, then assign 
        * strDestbakfile with .bak2 extension so that the .bak file
        * is moved with .bak2 extn to new path 
        */
        if(ic_utils::CIgniteFileUtils::Exists(strSrcbakfile))
        {
            strDestbakfile = rstrNewPath + ".bak2";
        }

        // Move log file in new Path with .bak extension 
        MoveFile(strDestPath, strMvfileInSameDir);
    }
    else
    {
        /* Check if .bak file exists, then assign strDestbakfile with 
        * .bak extension so that .bak file is moved with .bak 
        * extension to new path 
        */
        if(ic_utils::CIgniteFileUtils::Exists(strSrcbakfile))
        {
            strDestbakfile = rstrNewPath + ".bak";
        }
    }
    //Check if .bak file exists, then move the .bak file to new path
    if(ic_utils::CIgniteFileUtils::Exists(strSrcbakfile))
    {
        // Move .bak file to new Path
        MoveFile(strSrcbakfile, strDestbakfile);      
    }

    // Move log file to new Path
    MoveFile(m_strFilePath, strDestPath); 
}

void CIgniteLog::MoveFile(std::string& rstrSrcPath, 
                          std::string& rstrDestPath)
{
    if (ic_utils::CIgniteFileUtils::Move(rstrSrcPath, rstrDestPath))
    {
        print_pre_logger(rstrSrcPath + " file moved successfully to " + 
                         rstrDestPath);
    }
    else
    {
        print_pre_logger("Error in moving " + rstrSrcPath + " file");
    }
}

std::string CIgniteLog::GetFileOutputPath()
{
    return m_strFilePath;
}

void CIgniteLog::DisableTruncate()
{
    m_bTruncateLog = false;
    //wait if log file truncation is in progress
    m_logMutex.Lock();
    m_logMutex.Unlock();
}

void CIgniteLog::TruncateIfRequired()
{
    if(m_bTruncateLog) 
    {
        if (m_atomicFileSize > m_ulTruncateFileSize) 
        {
            if (m_logMutex.TryLock()) 
            {
                /* if lock failed that means some other thread already doing 
                 * log file truncation 
                 */
                return;
            }
            TruncateFile();
            m_logMutex.Unlock();
        }
    }
}

void CIgniteLog::TruncateFile()
{
    if(m_of.is_open())
    {
        m_of <<"\nTRUNCATING FILE \n";
        m_of.flush();
        m_of.close();
        std::string strTmpFile = m_strFilePath + ".bak";
        int nRetVal;
        std::string strBakfilesecond = m_strFilePath + ".bak2";

        //Check if log file with .bak2 extension exists then delete the file
        if(ic_utils::CIgniteFileUtils::Exists(strBakfilesecond))
        {
            if((ic_utils::CIgniteFileUtils::Remove(strBakfilesecond)) == 0)
            {
                print_pre_logger(strBakfilesecond + " file deletion success");
            }
            else
            {
                print_pre_logger("Failed to delete " + strBakfilesecond);
            }
        }

        nRetVal = access(m_strFilePath.c_str(), F_OK);

        if(nRetVal == 0)
        {   //File exists
            ic_utils::CIgniteFileUtils::Remove(strTmpFile);
        }
        ic_utils::CIgniteFileUtils::Move(m_strFilePath , strTmpFile);
    }

    m_atomicFileSize=0;
    m_of.open(m_strFilePath.c_str(), std::ios::app);
    CIgniteLog::LogStatus();
}

unsigned int CIgniteLog::GetTruncateFileSize()
{
    return m_ulTruncateFileSize;
}

bool CIgniteLog::SetTruncateFileSize(const unsigned int& rnFileTruncateSize)
{
    bool bRet = false;

    if(rnFileTruncateSize > 0)
    {
        m_ulTruncateFileSize = rnFileTruncateSize;
        bRet = true;
    }

    return bRet;
}

unsigned int CIgniteLog::GetTruncateToSize()
{
    return m_ulTruncateToSize;
}

bool CIgniteLog::SetTruncateToSize(const unsigned int& runLogFileTruncateToSize)
{
    bool bRet = false;

    if(runLogFileTruncateToSize > 0)
    {
        m_ulTruncateToSize = runLogFileTruncateToSize;
        bRet = true;
    }

    return bRet;
}

void CIgniteLog::SetFileOutputPath(const std::string& rstrPath, 
                                   const int nTruncateFileSize,
                                   const int nTruncateToSize)
{
    m_strFilePath = rstrPath;
    m_atomicFileSize = 0;
    m_ulTruncateFileSize = nTruncateFileSize;
    m_ulTruncateToSize = nTruncateToSize;

    if (!rstrPath.empty())
    {
        std::string strParentDir = rstrPath.substr(0, 
            rstrPath.find_last_of("/"));
        if (!ic_utils::CIgniteFileUtils::Exists(strParentDir))
        {
            ic_utils::CIgniteFileUtils::MakeDirectory(strParentDir);
        }

        if(m_of.is_open())
        {
            m_of.flush();
            m_of.close();
        }

        m_atomicFileSize = ic_utils::CIgniteFileUtils::GetSize(rstrPath);
        m_of.open(rstrPath.c_str(), std::ios::app);

        if(!m_of.is_open())
        {
            print_pre_logger("failed to open the file :" + rstrPath);
        }
    }
}

bool CIgniteLog::MoveToUpload(std::string strLogPath, std::string strUploadPath, 
                              const bool& rbCompress) 
{
    bool bIsMoved = false;

    if (!ic_utils::CIgniteFileUtils::Exists(strLogPath)) 
    {
        HCPLOG_E << "Log path does not exist.";
        return bIsMoved;
    }

    if(m_of.is_open()) 
    {
        //Add timestamp as marker.
        unsigned long long int ullnCurrTime = 
            ic_utils::CIgniteDateTime::GetCurrentTimeMs();

        //This closes the log file.
        ic_utils::CIgniteLog(true).Get() << "Log moving at Time:" \
            << ullnCurrTime;
    }

    //Move
    if(rbCompress)
    {
        if(Z_OK == ic_utils::CIgniteFileUtils::Compress(strLogPath, 
                                                        strUploadPath))
        {
            bIsMoved = true;
            ic_utils::CIgniteFileUtils::Remove(strLogPath);
        }
        else
        {
            //Continue logging in same file
            return bIsMoved;
        }
    }
    else
    {
        bIsMoved = ic_utils::CIgniteFileUtils::Move(strLogPath, strUploadPath);
    }

    //start logging into the log file
    m_of.open(strLogPath, std::ios::app);
    CIgniteLog::LogStatus();
    
    return bIsMoved;
}

std::string CIgniteLog::ToString(const LogLevel eLevel)
{
    /* static const char* const cstrBuffer[] = {"NONE ", "FATAL", "CRIT ",  
     * "ERROR", "WARN ", "INFO ", "DEBUG", "TRACE"};
     */
    static const char* const cstrBuffer[] = {"N ", "F", "C", "E", "W", "I", "D", 
                                         "T"};
    return cstrBuffer[eLevel];
}

inline void print_pre_logger(const std::string &rstrLog)
{
    HCP_PRELOG_W(rstrLog.c_str());
}

inline std::string get_time_formatted()
{
    char cstrBuffer[11];
    time_t t;
    time(&t);
    tm r = {0};
    strftime(cstrBuffer, sizeof(cstrBuffer), "%H:%M:%S", localtime_r(&t, &r));
    struct timeval stTv;
    gettimeofday(&stTv, 0);
    char cstrResult[RESULT_SIZE] = {0};
    snprintf(cstrResult, RESULT_SIZE, "%s.%03ld", cstrBuffer, (long)stTv.tv_usec 
        / 1000);
    return cstrResult;
}

void CIgniteLog::SetStatus(const std::string& rstrKey, 
                           const std::string& rstrValue)
{
    ic_utils::CScopeLock scope_lock(m_statusMutex);
    m_mapStatus[rstrKey] = rstrValue;
}

void CIgniteLog::LogStatus()
{
    ic_utils::CScopeLock scope_lock(m_statusMutex);
    std::map<std::string, std::string>::iterator mapIter;

    if(m_of.is_open())
    {
        m_of <<"****************************************\n";
        m_of << "Time :"
             << ic_utils::CIgniteDateTime::GetCurrentFormattedDateTime()
             << std::endl;
        for(mapIter = m_mapStatus.begin(); mapIter != m_mapStatus.end(); 
            ++mapIter)
        {
            m_of << mapIter->first <<"="<<mapIter->second<<std::endl;
        }
        PrintDiagHeader();
        m_of << "****************************************\n";
    }
}

bool CIgniteLog::InitDiagHeader(
                 const std::vector<std::string> &rvectorDiagHeaderInfo)
{
    //initialize
    bool bRetVal = false;
    if(rvectorDiagHeaderInfo.empty())
    {
        // ERROR case       
    }
    else if (rvectorDiagHeaderInfo.size() > MAX_DIAG_ENTRIES)
    {
        g_vectorDiagHeader.clear();
        for (int nJ = (rvectorDiagHeaderInfo.size() - MAX_DIAG_ENTRIES); nJ 
            < rvectorDiagHeaderInfo.size(); nJ++)
        {
            g_vectorDiagHeader.push_back(rvectorDiagHeaderInfo[nJ]);
        }
        bRetVal = true;
    }
    else
    {
        g_vectorDiagHeader.clear();
        for (int nI = 0; nI < rvectorDiagHeaderInfo.size(); nI++)
        {
            g_vectorDiagHeader.push_back(rvectorDiagHeaderInfo[nI]);
        }
        bRetVal = true;
    }
    return bRetVal;
}

bool CIgniteLog::UpdateDiagHeader(const std::string &rDiagString)
{
    bool bRetVal = false;

    if(rDiagString.empty())
    {
        //Error Case
    }
    else
    {
        if(!g_vectorDiagHeader.empty() && 
            g_vectorDiagHeader.size() >= MAX_DIAG_ENTRIES)
        {
            g_vectorDiagHeader.erase(g_vectorDiagHeader.begin());
        }
        g_vectorDiagHeader.push_back(rDiagString);
        bRetVal = true;
    }
    return bRetVal;
}

bool CIgniteLog::PrintDiagHeader(void)
{
    bool bRetVal = false;

    if (m_of.is_open())
    {       
        m_of << "Diagnostics Information\n";
        if (!g_vectorDiagHeader.empty())
        {
            for (int nI = 0; nI < g_vectorDiagHeader.size(); nI++)
            {
                m_of << g_vectorDiagHeader[nI] << std::endl;
            }
            
#ifdef IC_UNIT_TEST
            std::cout << "Diagnostics Information\n";
            for (int nJ = 0; nJ < g_vectorDiagHeader.size(); nJ++)
            {
                std::cout << g_vectorDiagHeader[nJ] << std::endl;
            }
#endif     
        }
        else // empty Diag header
        {
            m_of << "No Diag info available/disabled\n";
#ifdef IC_UNIT_TEST
            std::cout << "Diagnostics Information\n";
            std::cout << "No Diag info available/disabled"<<std::endl;
#endif
        }
        bRetVal = true;
    }
    else
    {

    }

    return bRetVal;
}

unsigned int CIgniteLog::GetMaxDiagEntryValue()
{
    return MAX_DIAG_ENTRIES;
}

bool CIgniteLog::SetMaxDiagEntryValue(const unsigned int &rnMaxEntries)
{
    bool bRetVal = false;

    if ((rnMaxEntries >= MAX_DIAG_ENTRIES_LOWER_LIMIT) && 
        (rnMaxEntries <= MAX_DIAG_ENTRIES_UPPER_LIMIT))
    {
        MAX_DIAG_ENTRIES = rnMaxEntries;
        bRetVal = true;
    }
    else
    {
        //Error case
    }

    return bRetVal;
}

void CIgniteLog::ResetDiagHeader(void)
{
    g_vectorDiagHeader.clear();
}

} /* namespace ic_utils */

