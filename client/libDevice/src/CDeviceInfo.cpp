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
#include <sys/stat.h> /* for statbuf in getFreeMem() */
#elif defined(__gnu_linux__)
#include <sys/sysinfo.h> /* for statbuf in getFreeMem() */
#endif
#include <jsoncpp/json.h>

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <locale>
#include <cstring>
#include <string>
#include <fstream>
#include <unistd.h>

#include "CIgniteLog.h"
#include "CIgniteDateTime.h"
#include "EventLibVersion.h"
#include "CIgniteEvent.h"
#include "CIgniteFileUtils.h"
#include "CIgniteClient.h"
#include "CIgniteConfig.h"
#include "CCpuLoad.h"
#include "CDeviceInfo.h"

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CDeviceInfo"

namespace ic_device
{

// static members initialization
ProductAttribute CDeviceInfo::m_paSerialNumber;
ProductAttribute CDeviceInfo::m_paVIN;
ProductAttribute CDeviceInfo::m_paSWversion;
ProductAttribute CDeviceInfo::m_paIMEI;
ProductAttribute CDeviceInfo::m_paHWversion;
std::string CDeviceInfo::m_strStartupReason = "";
unsigned int CDeviceInfo::m_unRestartCount = 0;

void CDeviceInfo::Init(const std::string strSerial,
        const std::string strVIN, const std::string strSWVer,
        const std::string strHWVer, const std::string strIMEI,
        const std::string strStartupReason, const unsigned int unRestartCount)
{

    //if the values are not empty, use it;
    //   otherwise, let the default value remains.
    if (!strVIN.empty())
    {
        SetVIN(strVIN, ic_core::IProduct::ProductAttributeStatus::eNO_ERROR);
    }

    if (!strSerial.empty())
    {
        SetSerialNumber(strSerial,
                    ic_core::IProduct::ProductAttributeStatus::eNO_ERROR);
    }

    if (!strSWVer.empty())
    {
        SetSWVersion(strSWVer,
                    ic_core::IProduct::ProductAttributeStatus::eNO_ERROR);
    }

    if (!strHWVer.empty())
    {
        SetHWVersion(strHWVer,
                    ic_core::IProduct::ProductAttributeStatus::eNO_ERROR);
    }

    if (!strIMEI.empty())
    {
        SetIMEI(strIMEI, ic_core::IProduct::ProductAttributeStatus::eNO_ERROR);
    }

    //no attribute status to maintain for startup reason,
    //    so assign the given value.
    m_strStartupReason = strStartupReason;

    //no attribute status to maintain for client restart count
    //    so assign the given value.
    m_unRestartCount = unRestartCount;
}

void CDeviceInfo::SetVIN(const std::string strVIN,
        ic_core::IProduct::ProductAttributeStatus paStatus)
{
    m_paVIN.strValue = strVIN;
    m_paVIN.paStatus = paStatus;
}

std::string CDeviceInfo::GetVIN()
{
    HCPLOG_METHOD();

    //if current value is not empty, use it
    if (!m_paVIN.strValue.empty())
    {
        return m_paVIN.strValue;
    }

    //if current value is empty, check for override option
    std::string strOverrideVIN = ic_core::CIgniteConfig::GetInstance()-> \
               GetAsString("VinOverride");
    if (!strOverrideVIN.empty())
    {
        HCPLOG_I << "Using overridden VIN " << strOverrideVIN;
        SetVIN(strOverrideVIN,
                ic_core::IProduct::ProductAttributeStatus::eNO_ERROR);
    }
    else
    {
        //if override option is not set, check for default value
        ic_utils::Json::Value defRoot = ic_core::CIgniteConfig:: \
                                    GetInstance()->GetJsonValue("Default");
        if (defRoot.isMember("Vin") && defRoot["Vin"].isString())
        {
            std::string strDefVIN = defRoot["Vin"].asString();

            //if default value is not empty, use it
            if (!strDefVIN.empty())
            {
                HCPLOG_I << "Using default VIN " << strDefVIN;
                SetVIN(strDefVIN,
                        ic_core::IProduct::ProductAttributeStatus::eNO_ERROR);
            }
        }
    }
    HCPLOG_I << "VIN read " << m_paVIN.strValue;
    return m_paVIN.strValue;
}

void CDeviceInfo::SetSerialNumber(const std::string strSerial,
        ic_core::IProduct::ProductAttributeStatus paStatus)
{
    m_paSerialNumber.strValue = strSerial;
    m_paSerialNumber.paStatus = paStatus;
}

std::string CDeviceInfo::GetSerialNumber()
{
    HCPLOG_METHOD();

    //if current value is not empty, use it
    if (!m_paSerialNumber.strValue.empty())
    {
        return m_paSerialNumber.strValue;
    }

    //if current value is empty, check for override option
    std::string strOverrideSerial = ic_core::CIgniteConfig::GetInstance()-> \
               GetAsString("SerialOverride");
    if (!strOverrideSerial.empty())
    {
        HCPLOG_I << "Using overridden Serial " << strOverrideSerial;
        SetSerialNumber(strOverrideSerial,
                ic_core::IProduct::ProductAttributeStatus::eNO_ERROR);
    }
    else
    {
        //if override option is not set, check for default value
        ic_utils::Json::Value defRoot = ic_core::CIgniteConfig:: \
                                    GetInstance()->GetJsonValue("Default");
        if (defRoot.isMember("Serial") && defRoot["Serial"].isString())
        {
            std::string strDefSerial  = defRoot["Serial"].asString();

            //if default value is not empty, use it
            if (!strDefSerial.empty())
            {
                HCPLOG_I << "Using default Serial " << strDefSerial;
                SetSerialNumber(strDefSerial,
                        ic_core::IProduct::ProductAttributeStatus::eNO_ERROR);
            }
        }
    }
    HCPLOG_I << "Serial read " << m_paSerialNumber.strValue;
    return m_paSerialNumber.strValue;
}

void CDeviceInfo::SetIMEI(const std::string strIMEI,
        ic_core::IProduct::ProductAttributeStatus paStatus)
{
    m_paIMEI.strValue = strIMEI;
    m_paIMEI.paStatus = paStatus;
}

std::string CDeviceInfo::GetIMEI()
{
    HCPLOG_METHOD();

    //if current value is not empty, use it
    if (!m_paIMEI.strValue.empty())
    {
        return m_paIMEI.strValue;
    }

    //if current value is empty, check for override option
    std::string strOverrideIMEI = ic_core::CIgniteConfig::GetInstance()-> \
               GetAsString("IMEIOverride");
    if (!strOverrideIMEI.empty())
    {
        HCPLOG_I << "Using overridden IMEI " << strOverrideIMEI;
        SetIMEI(strOverrideIMEI,
                ic_core::IProduct::ProductAttributeStatus::eNO_ERROR);
    }
    else
    {
        //if override option is not set, check for default value
        ic_utils::Json::Value defRoot = ic_core::CIgniteConfig:: \
                                    GetInstance()->GetJsonValue("Default");
        if (defRoot.isMember("IMEI") && defRoot["IMEI"].isString())
        {
            std::string strDefIMEI  = defRoot["IMEI"].asString();

            //if default value is not empty, use it
            if (!strDefIMEI.empty())
            {
                HCPLOG_I << "Using default IMEI " << strDefIMEI;
                SetIMEI(strDefIMEI,
                        ic_core::IProduct::ProductAttributeStatus::eNO_ERROR);
            }
        }
    }
    HCPLOG_I << "IMEI read " << m_paIMEI.strValue;
    return m_paIMEI.strValue;
}

void CDeviceInfo::SetSWVersion(const std::string strSWVersion,
        ic_core::IProduct::ProductAttributeStatus paStatus)
{
    m_paSWversion.strValue = strSWVersion;
    m_paSWversion.paStatus = paStatus;
}

std::string CDeviceInfo::GetSWVersion()
{
    HCPLOG_METHOD();

    //if current value is not empty, use it
    if (!m_paSWversion.strValue.empty())
    {
        return m_paSWversion.strValue;
    }

    //if current value is empty, check for override option
    std::string strOverrideSWVersion = ic_core::CIgniteConfig::GetInstance()-> \
               GetAsString("SwVersionOverride");
    if (!strOverrideSWVersion.empty())
    {
        HCPLOG_I << "Using overridden SWVersion " << strOverrideSWVersion;
        SetSWVersion(strOverrideSWVersion,
                ic_core::IProduct::ProductAttributeStatus::eNO_ERROR);
    }
    else
    {
        //if override option is not set, check for default value
        ic_utils::Json::Value defRoot = ic_core::CIgniteConfig:: \
                                    GetInstance()->GetJsonValue("Default");
        if (defRoot.isMember("SwVersion") && defRoot["SwVersion"].isString())
        {
            std::string strDefSWVersion  = defRoot["SwVersion"].asString();

            //if default value is not empty, use it
            if (!strDefSWVersion.empty())
            {
                HCPLOG_I << "Using default IMEI " << strDefSWVersion;
                SetSWVersion(strDefSWVersion,
                        ic_core::IProduct::ProductAttributeStatus::eNO_ERROR);
            }
        }
    }
    HCPLOG_I << "SWVersion read " << m_paSWversion.strValue;
    return m_paSWversion.strValue;
}

void CDeviceInfo::SetHWVersion(const std::string strHWVersion,
        ic_core::IProduct::ProductAttributeStatus paStatus)
{
    m_paHWversion.strValue = strHWVersion;
    m_paHWversion.paStatus = paStatus;
}

std::string CDeviceInfo::GetHWVersion()
{
    HCPLOG_METHOD();

    //if current value is not empty, use it
    if (!m_paHWversion.strValue.empty())
    {
        return m_paHWversion.strValue;
    }

    //if current value is empty, check for override option
    std::string strOverrideHWVersion = ic_core::CIgniteConfig::GetInstance()-> \
               GetAsString("HwVersionOverride");
    if (!strOverrideHWVersion.empty())
    {
        HCPLOG_I << "Using overridden HWVersion " << strOverrideHWVersion;
        SetHWVersion(strOverrideHWVersion,
                ic_core::IProduct::ProductAttributeStatus::eNO_ERROR);
    }
    else
    {
        //if override option is not set, check for default value
        ic_utils::Json::Value defRoot = ic_core::CIgniteConfig:: \
                                    GetInstance()->GetJsonValue("Default");
        if (defRoot.isMember("HwVersion") && defRoot["HwVersion"].isString())
        {
            std::string strDefHWVersion  = defRoot["HwVersion"].asString();

            //if default value is not empty, use it
            if (!strDefHWVersion.empty())
            {
                HCPLOG_I << "Using default IMEI " << strDefHWVersion;
                SetHWVersion(strDefHWVersion,
                        ic_core::IProduct::ProductAttributeStatus::eNO_ERROR);
            }
        }
    }
    HCPLOG_I << "HWVersion read " << m_paHWversion.strValue;
    return m_paHWversion.strValue;
}

void CDeviceInfo::SetStartupReason(const std::string strStartupReason)
{
    m_strStartupReason = strStartupReason;
}

std::string CDeviceInfo::GetStartupReason()
{
    return m_strStartupReason;
}

void CDeviceInfo::SetRestartCount(const unsigned int unRestartCount)
{
    m_unRestartCount = unRestartCount;
}

unsigned int CDeviceInfo::GetRestartCount()
{
    return m_unRestartCount;
}

unsigned long long CDeviceInfo::GetFreeMem()
{
#if defined(__gnu_linux__)
    struct sysinfo sys_info;
    if (sysinfo(&sys_info) != 0)
    {
        HCPLOG_T << "Error reading sysinfo";
        return 0;
    }
    else
    {
        return (sys_info.freeram * (unsigned long long)sys_info.mem_unit);
    }
#else
    HCPLOG_E << "Undefined implementation";
    return 0;
#endif
}

float CDeviceInfo::GetCpuLoad()
{
    float fltCpuLoad = ic_device::CCpuLoad::GetCpuLoad()->GetCpuUsage();
    if (fltCpuLoad > 100)
    {
        return 100.0;
    }
    else
    {
        return fltCpuLoad;
    }
}

void CDeviceInfo::LogCpuUsage()
{
    ic_device::CCpuLoad::GetCpuLoad()->LogCpuUsage();
}

void *send_system_snapshot_event(void *pvoidReason)
{
    ic_event::CIgniteEvent systemSnapshotEvent("1.0", "SystemSnapshot");
    std::string *pstrReasonStr = static_cast<std::string *>(pvoidReason);

    HCPLOG_I << "Creating system snapshot event.  Reason="
             << *pstrReasonStr;
    systemSnapshotEvent.AddField("reason", *pstrReasonStr);
    systemSnapshotEvent.AddField("percentCpuUsed",
                                (int)CDeviceInfo::GetCpuLoad());
    systemSnapshotEvent.AddField("freeMemory",
                                (double)CDeviceInfo::GetFreeMem());
    systemSnapshotEvent.Send();

    pthread_detach(pthread_self());

    delete pstrReasonStr;
    return nullptr;
}

int CDeviceInfo::CreateSystemSnapshotEvent(const std::string &rstrReason)
{
    if(rstrReason.empty())
    {
        HCPLOG_E << "Reason string cannot be rmpty";
        return -2;
    }

    pthread_t snapshotThread;
    std::string *pstrReasonStr = new std::string(rstrReason);

    if(nullptr == pstrReasonStr)
    {
        HCPLOG_E << "Failed to allocate memory";
        return -1;
    }

    return pthread_create(&snapshotThread, nullptr, &send_system_snapshot_event,
                          (void *)pstrReasonStr);
}

} // namespace ic_device
