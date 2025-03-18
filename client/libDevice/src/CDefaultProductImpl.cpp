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

#include "CIgniteConfig.h"
#include "CIgniteEvent.h"
#include "CIgniteLog.h"
#include "CDeviceInfo.h"
#include <map>
#include <string>
#include "CDefaultProductImpl.h"

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CDefaultProductImpl"

namespace ic_device
{

CDefaultProductImpl::CDefaultProductImpl()
{
}

CDefaultProductImpl::~CDefaultProductImpl()
{
}

void CDefaultProductImpl::StartupInit(const std::string strSerial,
        const std::string strVIN, const std::string strSWVer,
        const std::string strHWVer, const std::string strIMEI,
        const std::string strStartupReason, const unsigned int unRestartCount)
{
    CDeviceInfo::Init(strSerial, strVIN, strSWVer,
                    strHWVer, strIMEI,strStartupReason, unRestartCount);
}

float CDefaultProductImpl::GetCpuLoad()
{
    return 0.0;
}

bool CDefaultProductImpl::SetAttribute(ProductAttribute attribute,
                            std::string strValue,
                            ProductAttributeStatus status)
{
    bool bRetVal = true;

    if (attribute == ic_core::IProduct::ProductAttribute::eVIN)
    {
        CDeviceInfo::SetVIN(strValue, status);
    }
    else if (attribute == ic_core::IProduct::ProductAttribute::eSerialNumber)
    {
        CDeviceInfo::SetSerialNumber(strValue, status);
    }
    else if (attribute == ic_core::IProduct::ProductAttribute::eSWVersion)
    {
        CDeviceInfo::SetSWVersion(strValue, status);
    }
    else if (attribute == ic_core::IProduct::ProductAttribute::eHWVersion)
    {
        CDeviceInfo::SetHWVersion(strValue, status);
    }
    else if (attribute == ic_core::IProduct::ProductAttribute::eIMEI)
    {
        CDeviceInfo::SetIMEI(strValue, status);
    }
    else
    {
        HCPLOG_E << "Unknown attribute " << attribute
                 << " ; value " << strValue << "; status " << status;
        bRetVal = false;
    }

    return bRetVal;
}

std::string CDefaultProductImpl::GetAttribute(const ProductAttribute rstrkey)
{
    HCPLOG_I << "rstrkey=" << rstrkey;
    std::string strAttrValue = "";

    if (rstrkey == ic_core::IProduct::eVIN)
    {
        strAttrValue = CDeviceInfo::GetVIN();
    }
    else if (rstrkey == ic_core::IProduct::eSerialNumber)
    {
        strAttrValue = CDeviceInfo::GetSerialNumber();
    }
    else if (rstrkey == ic_core::IProduct::eSWVersion)
    {
        strAttrValue = CDeviceInfo::GetSWVersion();
    }
    else if (rstrkey == ic_core::IProduct::eHWVersion)
    {
        strAttrValue = CDeviceInfo::GetHWVersion();
    }
    else if (rstrkey == ic_core::IProduct::eIMEI)
    {
        strAttrValue = CDeviceInfo::GetIMEI();
    }
    else
    {
        strAttrValue = "Unsupported Attribute";
    }

    return strAttrValue;
}

std::string CDefaultProductImpl::GetStartupReason()
{
    return CDeviceInfo::GetStartupReason();
}

unsigned int CDefaultProductImpl::GetRestartCount()
{
    return CDeviceInfo::GetRestartCount();
}

bool CDefaultProductImpl::GenerateEvent(const std::string &rstrkey)
{
    bool bResult = true;

    if (rstrkey == "MemoryUsage")
    {
        ic_event::CIgniteEvent memEvent("1.0", "MemoryUsage");
        double freeMem = (double)CDeviceInfo::GetFreeMem();
        memEvent.AddField("free", freeMem);
        memEvent.Send();
    }
    else if (rstrkey == "CpuUsage")
    {
        CDeviceInfo::LogCpuUsage();
    }
    else
    {
        HCPLOG_E << "Invalid event requested to generate. " << rstrkey;
        bResult = false;
    }
    return bResult;
}

std::string CDefaultProductImpl::GetActivationQualifierID()
{
    return GetAttribute(eVIN);
}

}//namespace ic_device
