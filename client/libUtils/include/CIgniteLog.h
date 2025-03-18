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
* \file CIgniteLog.h
*
* \brief CIgniteLog provides all the log related utility functions.
*******************************************************************************
*/

#ifndef CIGNITE_LOG_H
#define CIGNITE_LOG_H

#include <sys/types.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <atomic>
#include <map>
#include <vector>
#include "CIgniteMutex.h"

#define HCPLOG_METHOD() HCPLOG_T
#define HCPLOG_LINE() HCPLOG_T
#define HCPLOG_EXCEPTION(e) HCPLOG(ic_utils::eHCP_LOG_ERROR) << \
    "EXCEPTION AT " << __PRETTY_FUNCTION__ << ":" << __LINE__ << ", Text=" << e

//Below flag can be enabled from CMakefile
#ifdef SHORT_LOG_MSG
    #define HCPLOG_RW(PREFIX) HCPLOG(ic_utils::eHCP_LOG_WARNING) << PREFIX \
        << "::" << __func__ << ":" << __LINE__ << " "
    #define HCPLOG_W HCPLOG_RW(PREFIX)

    #define HCPLOG_RF(PREFIX) HCPLOG(ic_utils::eHCP_LOG_FATAL) << PREFIX \
        << "::" << __func__ << ":" << __LINE__ << " "
    #define HCPLOG_F HCPLOG_RF(PREFIX)

    #define HCPLOG_RC(PREFIX) HCPLOG(ic_utils::eHCP_LOG_CRITICAL) << PREFIX \
        << "::" << __func__ << ":" << __LINE__ << " "
    #define HCPLOG_C HCPLOG_RC(PREFIX)

    #define HCPLOG_RE(PREFIX) HCPLOG(ic_utils::eHCP_LOG_ERROR) << PREFIX \
    << "::" << __func__ << ":" << __LINE__ << " "
    #define HCPLOG_E HCPLOG_RE(PREFIX)
#else
    #define HCPLOG_W HCPLOG(ic_utils::eHCP_LOG_WARNING) << __PRETTY_FUNCTION__ \
        << ":" << __LINE__ << " "
    #define HCPLOG_F HCPLOG(ic_utils::eHCP_LOG_FATAL) << __PRETTY_FUNCTION__ \
        << ":" << __LINE__ << " "
    #define HCPLOG_C HCPLOG(ic_utils::eHCP_LOG_CRITICAL) << __PRETTY_FUNCTION__ \
        << ":" << __LINE__ << " "
    #define HCPLOG_E HCPLOG(ic_utils::eHCP_LOG_ERROR) << __PRETTY_FUNCTION__ \
        << ":" << __LINE__ << " "
#endif

#define HCPLOG_I HCPLOG(ic_utils::eHCP_LOG_INFO) << __PRETTY_FUNCTION__ << " "
#define HCPLOG_D HCPLOG(ic_utils::eHCP_LOG_DEBUG) << __PRETTY_FUNCTION__ << \
    ":" << __LINE__ << " "
#define HCPLOG_T HCPLOG(ic_utils::eHCP_LOG_TRACE) << __PRETTY_FUNCTION__ << \
    ":" << __LINE__ << " "

#ifndef HCPLOG_MAX_LEVEL
    #define HCPLOG_MAX_LEVEL ic_utils::eHCP_LOG_TRACE
#endif

#define HCPLOG(level) \
if (level > HCPLOG_MAX_LEVEL || level == ic_utils::eHCP_LOG_NONE) ; \
else if (( level > ic_utils::CIgniteLog::GetReportingLevel()) && \
    ( level > ic_utils::CIgniteLog::GetFileOutputLevel())) ; \
else ic_utils::CIgniteLog().Get(level)

namespace ic_utils 
{
/**
 * Enum of possible Log Levels
 */
enum LogLevel 
{               
    eHCP_LOG_NONE,      ///< No Logs
    eHCP_LOG_FATAL,     ///< Fatal type logs
    eHCP_LOG_CRITICAL,  ///< Critical type logs
    eHCP_LOG_ERROR,     ///< Error type logs
    eHCP_LOG_WARNING,   ///< Warring type logs 
    eHCP_LOG_INFO,      ///< Info type logs
    eHCP_LOG_DEBUG,     ///< Debug type logs
    eHCP_LOG_TRACE      ///< Trace type logs
};

/**
 * class CIgniteLog provides a configurable logging mechanism with support for 
 * funtionalities like log truncation, compression, to initialize 
 * and update diagnostic headers for logging, etc.
 */
class CIgniteLog 
{
public:
    /**
     * Default no-argument constructor.
     */
    CIgniteLog();

    /**
     * Destructor
     */
    virtual ~CIgniteLog();

    /**
     * Method to get the output stream associated with the CIgniteLog object.
     * @param elevel Log level (default is eHCP_LOG_INFO).
     * @return Reference to the output stream.
     */
    std::ostringstream& Get(LogLevel elevel = eHCP_LOG_INFO);

public:
    /**
     * Method to get the reporting log level.
     * @param void
     * @return Reference to the reporting log level.
     */
    static LogLevel& GetReportingLevel();

    /**
     * Method to set the reporting log level.
     * @param elevel New reporting log level.
     * @return void
     */
    static void SetReportingLevel(const LogLevel&);

    /**
     * Method to get the file output log level.
     * @param void
     * @return Reference to the file output log level.
     */
    static LogLevel& GetFileOutputLevel();
    
    /**
     * Method to set the file output log level.
     * @param elevel New file output log level.
     * @return void
     */
    static void SetFileOutputLevel(const LogLevel&);
    
    /**
     * Method to set the path for file output, along with options for 
     * truncation.
     * @param rstrPath New log file path.
     * @param nTruncateFileSize Truncate file size option.
     * @param nTruncateToSize Truncate to size option.
     * @return void
     */
    static void SetFileOutputPath(const std::string& rstrPath,
                                  const int nTruncateFileSize, 
                                  const int nTruncateToSize);
    
    /**
     * Move log file to upload path, with an option to compress.
     * @param strLogPath Current log file path.
     * @param strUploadPath New upload path.
     * @param rbCompress Flag indicating whether to compress the log file 
     *        (default is true).
     * @return true if successful, false otherwise.
     */
    static bool MoveToUpload(std::string strLogPath, std::string strUploadPath, 
                             const bool& rbCompress=true);

    /**
     * Method to enable log file truncation.
     * @param void
     * @return void
     */
    static void EnableTruncate();

    /**
     * Method to disable log file truncation.
     * @param void
     * @return void
     */
    static void DisableTruncate();

    /**
     * Method to close the log file.
     * @param void
     * @return void
     */
    static void CloseLogFile();

    /**
     * Method to set a status key-value pair.
     * @param[in] rstrKey Reference to status key.
     * @param[in] rstrValue Reference to status value.
     * @return void
     */
    static void SetStatus(const std::string& rstrKey, 
        const std::string& rstrValue);

    /**
     * Method to get the truncate file size.
     * @param void
     * @return Integer value of truncated file size.
     */
    static unsigned int GetTruncateFileSize(void);

    /**
     * Method to set the truncate file size
     * @param[in] rnFileTruncateSize Truncate log file size
     * @return true, if sets the truncate file size is success; 
     *         else return false
     */
    static bool SetTruncateFileSize(const unsigned int& rnFileTruncateSize);

    /** 
     * Method to get the truncate to size
     * @param void
     * @return truncate to size value.
     */
    static unsigned int GetTruncateToSize(void);

    /**
     * Method to set the truncate to size
     * @param[in] runLogFileTruncateToSize truncate to size value.
     * @return true, if sets the truncate to size is success; else return false
     */
    static bool SetTruncateToSize(const unsigned int& runLogFileTruncateToSize);

    /**
     * Method to get the log file path
     * @param void
     * @return log file path
     */
    static std::string GetFileOutputPath(void);

    /**
     * Method to update the log file path to path given in the paramter.
     * [Path Not Exists] it creates the path.
     * [File already exists] it moves log file in new path with .bak extension
     * and log file in old path moves to new path
     * [File not exists] it moves old log file to new log file path.
     * [log .bak file exists in old path] and [log file exists in new path]
     * it moves .bak file from old path to new path with .bak2 extension
     * [log .bak file exists in old path] and [log file not exists in new path]
     * it just moves .bak file from old path to new path
     * @param[in] rstrNewPath Changed log file path.
     * @return true, if file Update success; else return false
     */
    static bool UpdateFilePath(const std::string& rstrNewPath);
    
    /**
     * Method to close and flush log file
     * @param void
     * @return void
     */
    static void CloseAndFlushLogFile();
    
    /**
     * Method to move log files to new path
     * @param[in] rstrNewPath path to move log file
     * @return void
     */
    static void MoveLogFilesToNewPath(const std::string& rstrNewPath);
    
    /**
     * Method to move files
     * @param[in] rstrSrcPath source path of log file
     * @param[in] rstrDestPath destination path of log file
     * @return void
     */
    static void MoveFile(std::string& rstrSrcPath, std::string& rstrDestPath);

    /**
     * Method to initialize the diag header , to be used later for logging
     * When the entries in input vector are more than MAX_DIAG_ENTRIES allowed, 
     * then last MAX_DIAG_ENTRIES from the input vector are used to initialize 
     * diagHeader.
     * @param[in] rvectorDiagHeaderInfo Vector of diag strings 
     *            (maximum of MAX_DIAG_ENTRIES size)
     * @return true, if initialization is successful; false otherwise.
     */
    static bool InitDiagHeader(const std::vector<std::string> 
        &rvectorDiagHeaderInfo);

    /**
     * Method to update the diag header
     * @param rDiagString String to be updated as part of the diag header
     * @return true if successfully updated , false otherwise
     */
    static bool UpdateDiagHeader(const std::string &rDiagString);

    /**
     * Method to print the diag info into Ignite Log
     * @param void
     * @return true, if info is printed in Ignite log successfully.
     */
    static bool PrintDiagHeader(void);

    /**
     * Method to get the number of max diag entries to be put in Ignite log.
     * @param void
     * @return max number of diag entries to be printed in Ignite log.
     */
    static unsigned int GetMaxDiagEntryValue(void);

    /**
     * Method to get the number of max diag entries to be put in Ignite log.
     * @param[in] rnMaxEntries Reference to the max number of entries to be put 
     * in Ignite log.
     * @return true if successful , false otherwise.
     */
    static bool SetMaxDiagEntryValue(const unsigned int &rnMaxEntries);

    /**
     * Method to reset the diagHeader vector, thus clearing the diag entries to 
     * be put in Ignite log as empty.
     * @param void
     * @return void
     */
    static void ResetDiagHeader(void);

    #ifdef IC_UNIT_TEST
        friend class CIgniteLogTest;

        /**
         * Method to get the status of log truncation
         * @param void
         * @return True if enabled else False
         */
        bool GetTruncateLogStatus() 
        {
            return m_bTruncateLog;
        }
    #endif
    
private:
    /**
     * Method to truncate the log file.
     * @param void
     * @return void
     */
    void TruncateFile();

    /**
     * Method to truncate the log file if required based on configured settings
     * @param void
     * @return void
     */
    void TruncateIfRequired();

    /**
     * Constructor for CIgniteLog class
     * @param bIsLast Flag indicating if it is the last log entry.
     */
    CIgniteLog(bool bIsLast);

    //! Flag indicating if the CIgniteLog object is the last log entry.
    bool m_bIsLast;

    //! Reporting log level.
    static LogLevel m_eReportingLevel;

    //! File output log level.
    static LogLevel m_eFileOutputLevel;

    //! Output file stream for file logging.
    static std::ofstream m_of;

    //! Log level associated with the current CIgniteLog object.
    LogLevel m_eLogLevel;

    //! Output string stream for building log messages.
    std::ostringstream m_os;

    /**
     * Method to convert LogLevel enum value to a string
     * @param[in] elevel LogLevel value
     * @return string associated with provided level.
     */
    std::string ToString(const LogLevel elevel);

    /**
     * CIgniteLog Constructor, the CIgniteLog class is non-copyable.
     * @param [in] Reference to CIgniteLog obj
     */
    CIgniteLog(const CIgniteLog&);

    /**
     * Overloaded assignment operator for CIgniteLog class.
     * @param CIgniteLog& Constant reference to CIgniteLog object.
     * @return reference to a CIgniteLog object.
     */
    CIgniteLog& operator =(const CIgniteLog&);

    //! Mutex for controlling access to log operations.
    static CIgniteMutex m_logMutex;

    //! Size at which log files should be truncated.
    static unsigned long m_ulTruncateFileSize;

    //! Size to which log files should be truncated.
    static unsigned long m_ulTruncateToSize;

    //! Path to the log file.
    static std::string m_strFilePath;

    //! Current file size.
    static std::atomic<unsigned long> m_atomicFileSize;

    //! Flag indicating whether log truncation is enabled.
    static bool m_bTruncateLog;

    //! Status information represented as key(EventId)-value pairs 
    static std::map<std::string,std::string> m_mapStatus;

    //! Status information of Mutex.
    static ic_utils::CIgniteMutex m_statusMutex;

    /**
     * Method to log the current status information.
     * @param void
     * @return void
     */
    static void LogStatus();
};

} /* namespace ic_utils */

#endif //CLOG_H_
