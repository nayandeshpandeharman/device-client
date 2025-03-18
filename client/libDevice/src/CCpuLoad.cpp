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

#ifdef __QNX__
#include <atomic.h>
#include <libc.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/iofunc.h>
#include <sys/neutrino.h>
#include <sys/resmgr.h>
#include <sys/syspage.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/debug.h>
#include <sys/procfs.h>
#include <sys/neutrino.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include <devctl.h>
#include <errno.h>
#endif

#if defined(__gnu_linux__)
#include <sys/sysinfo.h>
#include <unistd.h>
#include <regex>
#endif
#include "CCpuLoad.h"
#include "CIgniteFileUtils.h"
#include "CIgniteLog.h"
#include "CIgniteEvent.h"
#include "CIgniteConfig.h"
#include <algorithm>

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CCpuLoad"

using ic_event::CIgniteEvent;
using std::string;
using std::vector;

namespace ic_device
{

    string convert_string(float fltNumber)
    {
        std::stringstream ss; // create a stringstream
        ss << fltNumber;      // add number to the stream
        return ss.str();      // return a string with the contents of the stream
    }

    CCpuLoad *CCpuLoad::GetCpuLoad()
    {
        static CCpuLoad cpuLoad;
        return &cpuLoad;
    }

#ifdef __QNX__

    CCpuLoad::CCpuLoad()
    {
        HCPLOG_METHOD();
        initializeProcesses();
    }
    CCpuLoad::~CCpuLoad()
    {
        closeCpu();
    }

    pthread_mutex_t cpuloadMutex = PTHREAD_MUTEX_INITIALIZER;

    void CCpuLoad::setNewProcess(const std::string processPath)
    {
        HCPLOG_METHOD();
        initialized = 0;
        path = processPath;
        initCpu();
    }

    int CCpuLoad::sampleCpus(void)
    {
        HCPLOG_METHOD();
        if (lastSutime.find(path) != lastSutime.end() && 
            lastNsec.find(path) != lastNsec.end())
        {
            procfs_info debug_data;
            _uint64 current_nsec, sutime_delta, time_delta;
            memset(&debug_data, 0, sizeof(debug_data));
            devctl(procFd[path], DCMD_PROC_INFO, &debug_data, 
                   sizeof(debug_data), NULL);
            /* Get the current time */
            current_nsec = nanoseconds();
            /* Get the deltas between now and the last samples */
            sutime_delta = 
                (debug_data.stime + debug_data.utime) - lastSutime[path];
            if (sutime_delta == 0)
            {
                initCpu();
                loads = 0;
                return EOK;
            }
            time_delta = current_nsec - lastNsec[path];
            // if the time difference is >= 2sec OR if this is the first sample
            if ((time_delta >= 2000000000) || 
                (lastPercentage.find(path) == lastPercentage.end()))
            {
                loads = ((float)(sutime_delta * 100) / (float)time_delta);

                /* (loads/nNumCpus) is the CPU idle %; in order to get CPU 
                 * utilization, it is to be deducted from 100 
                 */
                if ((loads / nNumCpus) > 100) /*-ve cpu load value!*/
                {
                    HCPLOG_E << "Invalid CPU load value! loads=" << loads <<
                             "; sutime_delta=" << sutime_delta << 
                             "; time_delta=" << time_delta << path;

                    // lastPercentage[path] is a division of nNumCpus
                    loads = lastPercentage[path] * nNumCpus;
                    HCPLOG_D << "Using previous cpu load " << loads;
                    return EOK;
                }
            }
            else
            {
                HCPLOG_E << "using previous percentage...";
                // lastPercentage[path] is a division of nNumCpus
                loads = lastPercentage[path] * nNumCpus; 
            }
            HCPLOG_E << "Loads" << loads << "sutime_delta" << sutime_delta <<
                     "time_delta " << time_delta << path;
            lastSutime[path] = debug_data.stime + debug_data.utime;
            lastNsec[path] = current_nsec;
        }
        else
        {
            HCPLOG_D << "Process will be added in next time::" << path;
            return ESRCH; // no such process error
        }
        return EOK;
    }

    int CCpuLoad::initCpu()
    {
        HCPLOG_METHOD();
        // Get the sutime and Nsec for the new process
        if (lastSutime.find(path) == lastSutime.end() && 
            lastNsec.find(path) == lastNsec.end())
        {
            int i = 0, fd;
            procfs_info debug_data;
            memset(&debug_data, 0, sizeof(debug_data));
            // Open a connection to proc to talk over.
            fd = open(path.c_str(), O_RDONLY);
            if (fd == -1)
            {
                HCPLOG_E << "Unable to access " << path;
                return -1;
            }
            i = fcntl(fd, F_GETFD);
            if (i != -1)
            {
                i |= FD_CLOEXEC;
                if (fcntl(fd, F_SETFD, i) != -1)
                {
                    /* Get a starting point for the comparisons */
                    devctl(fd, DCMD_PROC_INFO, &debug_data, sizeof(debug_data), 
                           NULL);
                    lastSutime.insert(std::make_pair(path, 
                        (debug_data.stime + debug_data.utime)));
                    lastNsec.insert(std::make_pair(path, nanoseconds()));
                    procFd.insert(std::make_pair(path, fd));
                    initialized = 1;
                    close(fd);
                    return (EOK);
                }
            }
            close(fd);
        }
        return (-1);
    }

    void CCpuLoad::closeCpu()
    {
        HCPLOG_METHOD();

        for (processFdMap::iterator iter = procFd.begin(); 
            iter != procFd.end(); iter++)
        {
            if (iter->second != -1)
            {
                close(iter->second);
            }
        }
    }

    float CCpuLoad::computeLoad()
    {
        HCPLOG_METHOD();
        float load = 0.0;

        if (!initialized)
        {
            initCpu();
        }

        if (EOK == sampleCpus())
        {
            load = loads / nNumCpus;
        }
        if (lastPercentage.find(path) != lastPercentage.end())
        {
            lastPercentage[path] = load;
        }
        else
        {
            lastPercentage.insert(std::make_pair(path, load));
        }
        return load;
    }

    int CCpuLoad::getNumCpus()
    {
        if (!initialized)
        {
            initCpu();
        }
        return nNumCpus;
    }

    _uint64 CCpuLoad::nanoseconds()
    {
        _uint64 sec, usec;
        struct timeval tval;

        gettimeofday(&tval, NULL);
        sec = tval.tv_sec;
        usec = tval.tv_usec;

        return (((sec * 1000000) + usec) * 1000);
    }

    std::string CCpuLoad::getprocessName()
    {
        HCPLOG_METHOD();
        string retString = "";
        static struct
        {
            procfs_debuginfo info;
            char buff[PATH_MAX];
        } name;
        if (devctl(procFd[path], DCMD_PROC_MAPDEBUG_BASE, &name,
                   sizeof(name), 0) != EOK)
        {
            HCPLOG_E << "Unable to get process name";
            return retString;
        }
        return (name.info.path);
    }

    void CCpuLoad::logCpuLoadForAll()
    {
        HCPLOG_METHOD();
        string totalProcess;
        CIgniteEvent *event = new CIgniteEvent("1.1", "CpuUsage");
        int procCount = (int)lastSutime.size();
        float currentCpuLoad = GetCpuUsage();
        if (currentCpuLoad >= nMaxCPULoadThreshold && nLogProcesses == 1)
        {
            vector<string> fileList;
            string path = "/proc/";
            ic_utils::CFileUtils::ListFiles(path.c_str(), fileList);
            for (vector<string>::iterator it = 
                fileList.begin(); it != fileList.end(); it++)
            {
                string temp(*it);
                int load;
                if (isdigit(temp.at(0)))
                {
                    *it = "/proc/" + temp + "/as";
                    temp = *it;
                    if (temp.compare("/proc/1/as") == 0)
                    {
                        continue;
                    }
                    else
                    {
                        setNewProcess(temp);
                        load = computeLoad();
                    }
                    string processName = getprocessName();
                    HCPLOG_D << "Load :" << load << "ProcessName :" 
                             << processName;
                    if ((load >= cpuUtlization) && !processName.empty())
                    {
                        totalProcess.append("{\"name\":\"" + processName + 
                                            "\",\"percentUsed\":\"" + 
                                            convert_string(load) + "\"},");
                    }
                }
            }
            if (!totalProcess.empty())
            {
                totalProcess.insert(0, "[");
                totalProcess.at(totalProcess.length() - 1) = ']';
                event->addFieldAsRawJsonString("processes", 
                                                totalProcess.c_str());
            }
            updateProcessCount(fileList);
        }
        event->AddField("totalProcesses", procCount);
        event->AddField("percentUsed", (int)currentCpuLoad);
        event->Send();
        delete event;
    }

    float CCpuLoad::GetCpuUsage()
    {
        HCPLOG_METHOD();
        pthread_mutex_lock(&cpuloadMutex);
        setNewProcess("/proc/1/as");
        float currentCpuLoad = 100 - computeLoad();
        HCPLOG_D << "Load :" << currentCpuLoad;
        pthread_mutex_unlock(&cpuloadMutex);
        return currentCpuLoad;
    }

    void CCpuLoad::LogCpuUsage()
    {
        HCPLOG_METHOD();
        logCpuLoadForAll();
    }

    void CCpuLoad::initializeProcesses()
    {
        HCPLOG_METHOD();
        ic_core::CIgniteConfig *config = ic_core::CIgniteConfig::GetInstance();
        nLogProcesses = config->GetInt("DAM.CpuProcessesLog.enableCpuStatus");
        nMaxCPULoadThreshold = config->GetInt("DAM.CpuProcessesLog.maxCPULoad");
        cpuUtlization = config->GetInt("DAM.CpuProcessesLog.processesCpuLoad");
        nNumCpus = _syspage_ptr->num_cpu;
        vector<string> fileList;
        string totalProcess;
        string path = "/proc/";
        ic_utils::CFileUtils::ListFiles(path.c_str(), fileList);
        for (vector<string>::iterator iter = fileList.begin();
             iter != fileList.end(); iter++)
        {
            string temp(*iter);
            if (isdigit(temp.at(0)))
            {
                temp = "/proc/" + temp + "/as";
                setNewProcess(temp);
            }
        }
    }

    void CCpuLoad::updateProcessCount(std::vector<std::string> &filelist)
    {
        HCPLOG_METHOD();
        if (lastSutime.size() > filelist.size())
        {
            processMap::iterator iter = lastSutime.begin();
            string procName;
            while (iter != lastSutime.end())
            {
                if (std::find(filelist.begin(), filelist.end(), 
                    iter->first) == filelist.end())
                {
                    procName = iter->first;
                    processMap::iterator toErase = iter;
                    ++iter;
                    lastSutime.erase(toErase);
                    lastNsec.erase(procName);
                    lastPercentage.erase(procName);
                    if (procFd[procName] != -1)
                    {
                        close(procFd[procName]);
                        procFd.erase(procName);
                    }
                    HCPLOG_T << "Processed Died :" << procName;
                }
                ++iter;
            }
        }
    }

#elif defined(__gnu_linux__)

    namespace
    {
        /// proc/stat file descriptor
        FILE *statFd = NULL;
        /// proc/<pid>/stat file descriptor
        FILE *statProcFd = NULL;
        /// sdcard/harman/process.log file descriptor
        FILE *procLogFd = NULL;
        const std::string defaultHighCpuLog = "/tmp/process.log";
    }

    CCpuLoad::CCpuLoad()
        : ullLastUser(0), ullLastUserLow(0), ullLastSys(0), ullLastIdle(0), ullLastTotal(0),
          dblLastPercent(0.0)
    {
        HCPLOG_METHOD();
        statFd = fopen("/proc/stat", "r");
        if (statFd == NULL)
        {
            HCPLOG_T << "fopen error for /proc/stat !";
        }
        else
        {
            fscanf(statFd, "cpu %Ld %Ld %Ld %Ld", &ullLastUser, &ullLastUserLow, 
                   &ullLastSys, &ullLastIdle);
            fclose(statFd);
            statFd = NULL;

            ullLastTotal = ullLastUser + ullLastUserLow + ullLastSys + ullLastIdle;
        }
        InitProcesses();
    }

    CCpuLoad::~CCpuLoad()
    {
        if (statFd)
        {
            fclose(statFd);
            statFd = NULL;
        }
        if (statProcFd)
        {
            fclose(statProcFd);
            statProcFd = NULL;
        }
        if (procLogFd)
        {
            fclose(procLogFd);
            procLogFd = NULL;
        }
    }

    void CCpuLoad::InitProcesses()
    {
        HCPLOG_METHOD();
        ic_core::CIgniteConfig *config = ic_core::CIgniteConfig::GetInstance();
        nLogProcesses = config->GetInt("DAM.CpuProcessesLog.enableCpuStatus");
        nMaxCPULoadThreshold = config->GetInt("DAM.CpuProcessesLog.maxCPULoad");
        nCpuUtilization = config->GetInt("DAM.CpuProcessesLog.processesCpuLoad");
        strProcessLogFilePath = config-> \
                            GetString("DAM.CpuProcessesLog.highCPUTempLogFile", 
                            defaultHighCpuLog);
        nNumCpus = sysconf(_SC_NPROCESSORS_ONLN);
        HCPLOG_T << "Number of CPU Cores " << nNumCpus;
    }

    std::string CCpuLoad::GetProcessName(std::string procPath)
    {
        HCPLOG_METHOD();
        char cstrProcName[1024];
        statProcFd = fopen(procPath.c_str(), "r");
        if (statProcFd != NULL)
        {
            fscanf(statProcFd, "%*d %s", cstrProcName);
            fclose(statProcFd);
            statProcFd = NULL;
        }
        HCPLOG_T << "Current process name is " << cstrProcName;
        return cstrProcName;
    }

    void CCpuLoad::LogCpuUsage()
    {
        int nPercentUsed = (int)GetCpuUsage();

        ic_event::CIgniteEvent event("1.2", "CpuUsage");
        if (nPercentUsed >= nMaxCPULoadThreshold && nLogProcesses == 1)
        {
            char buffer[1024];
            HCPLOG_D << "High CPU load detected. Percent used: " 
                     << nPercentUsed << ", max CPU load:" << nMaxCPULoadThreshold;

            std::string strProcessDetails = "";
            // storing full top cmd output into the file
            std::string strTopCmd = "top -b n1 | grep \"^ \" > " + 
                strProcessLogFilePath; 
            system(strTopCmd.c_str());

            procLogFd = fopen(strProcessLogFilePath.c_str(), "r");
            if (procLogFd == NULL)
            {
                HCPLOG_E << "fopen error " << strProcessLogFilePath;
                strProcessDetails = "Unable to get Process List";
            }
            else
            {
                /* to get top 5 process details, read top 6 entries from file 
                 * 'procLogFd' which included header detials at top
                 */  
                for (int i = 0; i < 6 && fgets(buffer, 1024, procLogFd); i++)
                {
                    strProcessDetails.append(buffer);
                    strProcessDetails = std::regex_replace(strProcessDetails, 
                                                        std::regex("\n"), ",");
                }
                // erase last char ',' from the string
                strProcessDetails.erase(strProcessDetails.size() - 1); 
                fclose(procLogFd);
                procLogFd = NULL;
            }
            event.AddField("processes", strProcessDetails.c_str());
        }
        event.AddField("percentUsed", nPercentUsed);
        struct sysinfo sys_info;
        if (sysinfo(&sys_info) != 0)
        {
            HCPLOG_T << "Error reading sysinfo";
        }
        int nProcCount = (int)(sys_info.procs);
        event.AddField("totalProcesses", nProcCount);
        event.Send();
    }

    float CCpuLoad::GetCpuUsage()
    {
        double dblPercent;
        unsigned long long ullCurrUser = 0, ullCurrUserLow = 0, ullCurrSys = 0,
                           ullCurrIdle = 0, ullTotal = 0;

        statFd = fopen("/proc/stat", "r");
        if (statFd == NULL)
        {
            HCPLOG_T << "fopen error for /proc/stat !";
            dblPercent = 0.0;
        }
        else
        {
            fscanf(statFd, "cpu %Ld %Ld %Ld %Ld", &ullCurrUser, &ullCurrUserLow,
                   &ullCurrSys, &ullCurrIdle);
            fclose(statFd);
            statFd = NULL;
            HCPLOG_T << "curr_User :" << ullCurrUser << " ullLastUser :" << ullLastUser;
            HCPLOG_T << "curr_UserLow :" << ullCurrUserLow << " ullLastUserLow :" 
                     << ullLastUserLow;
            HCPLOG_T << "curr_Sys :" << ullCurrSys << " ullLastSys :" << ullLastSys;
            HCPLOG_T << "curr_Idle :" << ullCurrIdle << " ullLastIdle :" << ullLastIdle;

            if (ullCurrUser < ullLastUser || ullCurrUserLow < ullLastUserLow ||
                ullCurrSys < ullLastSys || ullCurrIdle < ullLastIdle)
            {
                HCPLOG_T << "Over flow detected";
                // Overflow detection. Initialize total percent to Zero
                dblPercent = dblLastPercent;
                ullTotal = ullLastTotal;
            }
            else
            {
                ullTotal = (ullCurrUser - ullLastUser) + (ullCurrUserLow - ullLastUserLow) +
                        (ullCurrSys - ullLastSys);

                // if no change in the attributes
                if (ullTotal == 0)
                {
                    HCPLOG_T << "No change in the load!";

                    // use previous idle value
                    ullCurrIdle = ullLastIdle;

                    ullTotal = ullLastTotal;
                    dblPercent = dblLastPercent;
                }
                else
                {
                    dblPercent = ullTotal;
                    ullTotal += (ullCurrIdle - ullLastIdle);
                    dblPercent /= ullTotal;
                    dblPercent *= 100;
                }
            }

            if (dblPercent > 100)
            {
                HCPLOG_T << "Percent has crossed beyond 100; percent = " 
                         << dblPercent << "% : Total = " << ullTotal;
                dblPercent = dblLastPercent;
            }
            else
            {
                ullLastUser = ullCurrUser;
                ullLastUserLow = ullCurrUserLow;
                ullLastSys = ullCurrSys;
                ullLastIdle = ullCurrIdle;
                ullLastTotal = ullTotal;
                dblLastPercent = dblPercent;
            }
        }

        HCPLOG_T << "Returning " << dblPercent;
        return dblPercent;
    }
#else //#elif defined(__gnu_linux__)

    CCpuLoad::CCpuLoad()
    {
    }

    CCpuLoad::~CCpuLoad()
    {
    }

    float CCpuLoad::GetCpuUsage()
    {
        HCPLOG_METHOD() << "Not supported!";
        return 0;
    }

    void CCpuLoad::LogCpuUsage()
    {
        HCPLOG_METHOD() << "Not supported!";
        ic_event::CIgniteEvent event("1.0", "CpuUsage");
        event.AddField("info", "Not supported by Product!");
        event.AddField("percentUsed", 0);
        event.AddField("totalProcesses", 0);
        event.Send();
    }
#endif //End of #ifdef __QNX__
} /* namespace ic_device */
