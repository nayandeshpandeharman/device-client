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
* \file CDeviceDetails.h
*
* \brief This class/module provides device details to make it available to core
********************************************************************************
*/

#ifndef CDEVICE_DETAILS_H
#define CDEVICE_DETAILS_H

#include <string>

namespace ic_core 
{
/**
 * Class CDeviceDetails provides device details
 */
class CDeviceDetails
{
public:
    /**
     * Destructor
     */
    ~CDeviceDetails();

    /**
     * Method to get instance of CDeviceDetails
     * @param void
     * @return pointer object of CDeviceDetails
     */
    static CDeviceDetails* GetInstance();

    /**
     * Method to set device specific identifier
     * @param[in] strDeviceSpecificIdentifier device specific identifier
     * @return void
     */
    void SetDeviceSpecificIdentifier(std::string strDeviceSpecificIdentifier);

    /**
     * Method to set device serial number
     * @param[in] strSerialNumber serial number
     * @return void
     */
    void SetSerialNumber(std::string strSerialNumber);

    /**
     * Method to get device specific identifier
     * @param void
     * @return device specific identifier in string
     */
    std::string GetDeviceSpecificIdentifier();

    /**
     * Method to get device serial number
     * @param void
     * @return serial number in string
     */
    std::string GetSerialNumber();

private:
    /**
     * Default no-argument constructor.
     */
    CDeviceDetails();

    //! Member variable to stores device serial number
    std::string m_strSerialNumber;

    //! Member variable to stores device specific identifier
    std::string m_strDeviceSpecificIdentifier;    
};
} /* namespace ic_core */
#endif // CDEVICE_DETAILS_H
