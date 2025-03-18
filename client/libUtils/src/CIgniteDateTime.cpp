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

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <algorithm>
#if defined(__QNX__)
#include <iomanip.h>
#include <inttypes.h>
#include <sys/syspage.h>
#elif defined(__MACH__)
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach/mach_time.h>
#elif defined(__gnu_linux__) || defined(__ANDROID__)
#include <sys/sysinfo.h>
#endif
#include "CIgniteDateTime.h"
#include "CIgniteLog.h"

namespace ic_utils 
{
#define ALL_FIELDS "all"

unsigned long long CIgniteDateTime::GetCurrentTimeMs()
{
    HCPLOG_METHOD();
    long long lCurrentTime = 0;

#ifdef __MACH__
    /* clock_gettime not fully implemented on Mac.  gettimeofday is marked 
     * obsolete in the current version of POSIX so only use it where necessary.
     * http://pubs.opengroup.org/onlinepubs/9699919799/functions/gettimeofday.html
     */
    struct timeval stTimeVal;
    struct timezone stTimeZone;
    if (gettimeofday(&stTimeVal, &stTimeZone) == - 1)
    {
        return 0l;
    }
    // In GMT
    lCurrentTime = (unsigned long long)
        (stTimeVal.tv_sec * 1000 + stTimeVal.tv_usec / 1000);
    HCPLOG_T << "tv_sec=" << stTimeVal.tv_sec << ", tv_usec=" \
             << stTimeVal.tv_usec << ", lCurrentTime=" << lCurrentTime;
    HCPLOG_T << "tz_minuteswest=" << stTimeZone.tz_minuteswest \
             << ", tz_dsttime=" << stTimeZone.tz_dsttime;

#else
    struct timespec stTimeBuf;
    if( clock_gettime( CLOCK_REALTIME, &stTimeBuf) == -1 )
    {
        return 0l;
    }
    lCurrentTime = (((unsigned long long)stTimeBuf.tv_sec) * 1000) + 
        (((unsigned long long)stTimeBuf.tv_nsec) / 1000000);
#endif
    return lCurrentTime;

}

unsigned long long CIgniteDateTime::GetMonotonicTimeMs()
{

    long long lCurrentTime = 0;
#ifdef __MACH__

    unsigned long long ulTime = mach_absolute_time();

    static unsigned long long lScalingFactor = 0;
    if(!lScalingFactor)
    {
        mach_timebase_info_data_t info;
        kern_return_t ret = mach_timebase_info(&info);
        if(ret != 0)
        {
            HCPLOG_F << "mach_timebase_info failed";
        }
        lScalingFactor = info.numer / info.denom;
    }

    lCurrentTime = ulTime * lScalingFactor;


#else
    struct timespec stTimeBuf;
    /* Use CLOCK_MONOTONIC if issue in CLOCK_BOOTTIME.
     * Issues with CLOCK_MONOTONIC is if device in sleep/suspend state the clock 
     * is not updated.
     */
    if( clock_gettime( CLOCK_BOOTTIME, &stTimeBuf) == -1 )
    {
        return 0l;
    }
    lCurrentTime = (((unsigned long long)stTimeBuf.tv_sec) * 1000) + 
        (((unsigned long long)stTimeBuf.tv_nsec) / 1000000);
#endif
    return lCurrentTime;
}

int CIgniteDateTime::GetTimeZoneOffsetMinutes()
{
#if defined(__QNX__) || defined(__gnu_linux__) || defined(__MACH__) \
|| defined(__ANDROID__)
    time_t t = time(0);
    struct tm* pstTime = (struct tm*) malloc (sizeof (struct tm));
    int nTimeZoneOffsetMinutes = 0;
    if (NULL != pstTime)
    {
        localtime_r(&t, pstTime);
        nTimeZoneOffsetMinutes = (pstTime->tm_gmtoff) / 60;
        HCPLOG_D << "TZ off minutes: " << nTimeZoneOffsetMinutes;
        free (pstTime);
    }
    return nTimeZoneOffsetMinutes;
#else
    return 0;
#endif
}


unsigned long long CIgniteDateTime::ConvertToNumber(std::string strDate)
{
#if defined(__QNX__) || defined(__gnu_linux__) || defined(__ANDROID__)
    //expected strDate format is "{"date":"2015/06/26","time":"00:10:47:576"}"
    tm tm1;
    sscanf(strDate.c_str(), "%4d/%2d/%2d %2d:%2d:%2d", &tm1.tm_year, 
        &tm1.tm_mon, &tm1.tm_mday, &tm1.tm_hour, &tm1.tm_min, &tm1.tm_sec);
    tm1.tm_year = tm1.tm_year - 1900;
    tm1.tm_mon = tm1.tm_mon - 1;
    
    /* given strDate is just getting converted to a number here, 
     * so no dst is needed.
     */
    tm1.tm_isdst=0;
    
    //use timegm() instead of mktime() to treat the given strDate as UTC
    time_t tt = timegm(&tm1);
    unsigned long long lCurrentTime = (((unsigned long long)tt) * 1000);
    HCPLOG_T << "Date:" << strDate << "; time_t:" << tt;
    return lCurrentTime;
#else
    return 0;
#endif
}

#if SET_TIMEZONE_FOR_TESTING
void CIgniteDateTime::SetTimeZoneOffset(string strTimeZone)
{
    char* pchBuffer = getenv("TZ");
    if (NULL == pchBuffer)
    {
        HCPLOG_D << "Setting time zone: " << endl;
        string strTmpTz = "TZ=";
        strTmpTz.append(strTimeZone);

        char* pchPostFields = new char [strTmpTz.nSize() + 1];
        memcpy(pchPostFields, strTmpTz.c_str(), strTmpTz.nSize());
        pchPostFields[strTmpTz.nSize()] = 0;
        putenv(pchPostFields);
        delete [] pchPostFields;
    }
}
#endif

int CIgniteDateTime::GetCurrentDay()
{
    time_t theTime = time(NULL);
    struct tm* pstTime = localtime(&theTime);
    return pstTime->tm_mday;
}

int CIgniteDateTime::GetCurrentMonth()
{
    time_t theTime = time(NULL);
    struct tm* pstTime = localtime(&theTime);
    return pstTime->tm_mon;
}

unsigned long long CIgniteDateTime::GetTimeSinceBootMs()
{
    unsigned long long lTimeSinceBoot = 0;
#ifdef __QNX__
    unsigned long ulBootTime = SYSPAGE_ENTRY(qtime)->boot_time;
    time_t currentTime = time(NULL);
    double dblDiff = difftime(currentTime, (time_t) ulBootTime);
    if (dblDiff > 0 )
    {
        lTimeSinceBoot = (unsigned long long)(dblDiff * 1000);
    }
#elif defined(__MACH__)
    const int nMIB_SIZE = 2;
    int nMib[nMIB_SIZE];
    size_t nSize;
    struct timeval stBoottime;

    nMib[0] = CTL_KERN;
    nMib[1] = KERN_BOOTTIME;
    nSize = sizeof(stBoottime);
    if (sysctl(nMib, nMIB_SIZE, &stBoottime, &nSize, NULL, 0) < 0)
    {
        return -1.0;
    }
    unsigned long ulBootTime = stBoottime.tv_sec;
    time_t currentTime = time(NULL);
    double dblDiff = difftime(currentTime, (time_t) ulBootTime);
    if (dblDiff > 0 )
    {
        lTimeSinceBoot = (unsigned long long)(dblDiff * 1000);
    }
#elif defined(__gnu_linux__) || defined(__ANDROID__)
    struct sysinfo stSysInfo;
    sysinfo(&stSysInfo);
    lTimeSinceBoot = (unsigned long long) stSysInfo.uptime;
    lTimeSinceBoot = lTimeSinceBoot * 1000 ; //Converting to milliseconds
#endif
    //HCPLOG_T << "returning " << lTimeSinceBoot;
    return lTimeSinceBoot;
}

std::string CIgniteDateTime::GetCurrentFormattedDateTime()
{
    time_t rawtime;
    struct tm * pstTimeinfo;
    char cstrBuffer[80];

    time (&rawtime);
    pstTimeinfo = localtime(&rawtime);

    strftime(cstrBuffer, sizeof(cstrBuffer), "%d-%m-%Y %I:%M:%S", pstTimeinfo);
    std::string strResult(cstrBuffer);

    return strResult;
}

} /* namespace ic_utils */
