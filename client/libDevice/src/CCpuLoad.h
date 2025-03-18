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
* \file CCpuLoad.h
*
* \brief CCpuLoad provides functionalities related to CPU load/usage
*******************************************************************************
*/

#ifndef CPULOAD_H
#define CPULOAD_H

#include <vector>
#include <map>
#include <string>

namespace ic_device
{
    /**
     * CCpuLoad provides functionalities related to CPU load/usage
     */
    class CCpuLoad
    {
    public:
        /**
         * Singleton method to get the class instance
         * @param void
         * @return Instance of CCpuLoad class
         */
        static CCpuLoad *GetCpuLoad();

        /**
         * Destructor
         */
        ~CCpuLoad();

        /**
         * This function returns CPU usage of device
         * @param void
         * @return CPU usage value
         */
        float GetCpuUsage();

        /**
         * This function logs CPU usage of device
         * @param void
         * @return void
         */
        void LogCpuUsage();

    private:
        /**
         * Default constructor
         */
        CCpuLoad();

#ifdef __QNX__
        typedef std::map<std::string, _uint64> processMap;
        typedef std::map<std::string, float> percentageUseMap;
        typedef std::map<std::string, int> processFdMap;
        int initialized;
        float loads;
        int numCpus;
        std::string path;
        processMap lastSutime;
        processMap lastNsec;
        percentageUseMap lastPercentage;
        processFdMap procFd;
        int logProcesses;
        int maxCPULoadThreshold;
        int cpuUtlization;
        int sampleCpus();
        int getNumCpus();
        _uint64 nanoseconds();
        int initCpu();
        void closeCpu();
        void setNewProcess(const std::string);
        float computeLoad();
        std::string getprocessName();
        void logCpuLoadForAll();
        void initializeProcesses();
        void updateProcessCount(std::vector<std::string> &filelist);

#elif defined(__gnu_linux__)
        //! Number of CPU Cores
        int nNumCpus;

        //! stores enableCpuStatus
        int nLogProcesses;

        //! store maximum CPU Load Threshold
        int nMaxCPULoadThreshold;

        //! stores processesCpuLoad
        int nCpuUtilization;

        //! stores highCPUTempLogFile path
        std::string strProcessLogFilePath;
        
        //! user: Time spent executing user applications (user mode).
        unsigned long long ullLastUser;
        
        //! nice: Time spent executing user applications with low priority(nice)
        unsigned long long ullLastUserLow;

        //! system: Time spent executing system calls (system mode).
        unsigned long long ullLastSys;
        
        //! idle: Idle time.
        unsigned long long ullLastIdle;

        //! total: last total time.
        unsigned long long ullLastTotal;

        //! percentage: last percentage
        double dblLastPercent;

        /**
         * This function initialize processes
         * @param void
         * @return voids
         */
        void InitProcesses();
        
        /**
         * This function get the process name
         * @param procPath Path of the process
         * @return process name
         */
        std::string GetProcessName(std::string procPath);

#endif
    };
}
#endif /* CPULOAD_H */
