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

#include <unistd.h>
#include "CIgniteLog.h"
#include "CIgniteStringUtils.h"
#include "core/CKeyGenerator.h"
#include "CDeviceDetails.h"
#include "CIgniteClient.h"

//! Macro for KeyGenerator string
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CKeyGenerator"

namespace ic_core 
{
namespace 
{
//! Macro for serial number bytes 
# define NUM_SERIAL_BYTES 3

//! Macro for harmanId bytes
# define NUM_HARMANID_BYTES 5

//! Macro for passcode key bytes
# define TOTAL_PASSKEY_BYTES 2*(NUM_SERIAL_BYTES+NUM_HARMANID_BYTES) // 16 BYTES
}

CKeyGenerator::~CKeyGenerator()
{

}

std::string CKeyGenerator::GetPasscodeKey(const std::string &rstrDeviceID)
{
    HCPLOG_METHOD();
    static std::string strDeviceID;
    static std::string strPassKEY;

    if (strPassKEY.empty() || strDeviceID.compare(rstrDeviceID) != 0)
    {
        strDeviceID = rstrDeviceID;

        std::string strSerial = 
            CDeviceDetails::GetInstance()->GetSerialNumber();

        if (!strSerial.empty())
        {
          if (strSerial.length() < NUM_SERIAL_BYTES)
            {
                // Pad serial with X
                strSerial.insert(strSerial.end(), 
                                 NUM_SERIAL_BYTES - strSerial.length(), 'X');
            }
            else
            {
                strSerial = strSerial.substr(0, NUM_SERIAL_BYTES);
            }  
        }
        
        std::string strDeviceId = rstrDeviceID;

        if (!strDeviceId.empty())
        {
            if (strDeviceId.length() < NUM_HARMANID_BYTES)
            {
                // Pad DeviceId with X char
                strSerial.insert(strDeviceId.end(),
                                NUM_HARMANID_BYTES - strDeviceId.length(), 'X');
            }
            else
            {
                strDeviceId = strDeviceId.substr(strDeviceId.length() -
                                      NUM_HARMANID_BYTES, strDeviceId.length());
            }
        }
        strPassKEY = ic_utils::CIgniteStringUtils::ToHexString(strDeviceId + strSerial);
        HCPLOG_D << "Passcode Encryption Key : " << strPassKEY;
    }

    HCPLOG_T << "returning passKey : " << strPassKEY;
    return strPassKEY;
}

std::string CKeyGenerator::GetActivationKey()
{
    HCPLOG_METHOD();
    static std::string strActKEY;

    if (strActKEY.empty())
    {
        std::string strVin = 
            CDeviceDetails::GetInstance()->GetDeviceSpecificIdentifier();

        if (strVin.length() < 5)
        {
            // Pad VIN with X char
            strVin.insert(strVin.end(), 5 - strVin.length(), 'X');
        }
        else
        {
            strVin = strVin.substr(0, 5);
        }

        std::string strSerial = 
            CDeviceDetails::GetInstance()->GetSerialNumber();

        if (strSerial.length() < 2)
        {
            // Pad serial with X char
            strSerial.insert(strSerial.end(), 2 - strSerial.length(), 'X');
        }
        else
        {
            strSerial = strSerial.substr(0, 2);
        }

        strActKEY = "HarmanAct" + strVin + strSerial;
        HCPLOG_D << "activation Encryption Key : " << strActKEY;
    }

    HCPLOG_T << "returning actKEY : " << strActKEY;
    return strActKEY;
}

std::string CKeyGenerator::GetActivationQualifierKey(std::string strVin,
                                                     std::string strSerial)
{
    HCPLOG_METHOD();
    HCPLOG_C <<"vin:" << strVin <<"~serial:"<<strSerial;
    std::string strActQualKEY = "";

    if (strVin.length() < 5)
    {
        // Pad VIN with X
        strVin.insert(strVin.end(), 5 - strVin.length(), 'X');
    }
    else
    {
        strVin = strVin.substr(0, 5);
    }


    if (strSerial.length() < 2)
    {
        // Pad serial with X
        strSerial.insert(strSerial.end(), 2 - strSerial.length(), 'X');
    }
    else
    {
        strSerial = strSerial.substr(0, 2);
    }

    strActQualKEY = "HarmanAct" + strVin + strSerial;
    HCPLOG_D << "returning qualifier Encryption Key : " << strActQualKEY;

    return strActQualKEY;
}
}
