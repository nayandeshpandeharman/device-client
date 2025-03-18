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
* \file CDeviceInfo.h
*
* \brief CDeviceInfo provide interface to store and access device related
* information
*******************************************************************************
*/

#ifndef CDEVICEINFO_H_
#define CDEVICEINFO_H_

#include <string>
#include "IProduct.h"

namespace ic_device
{

/**
 * ProductAttribute Structure to store device related parameters
 */
struct ProductAttribute
{
	std::string strValue;   ///< value of the attribute
	ic_core::IProduct::ProductAttributeStatus paStatus; ///< attribute status

	/**
	 * Constructor
	 */
	ProductAttribute()
	{
		strValue= "";
		paStatus = ic_core::IProduct::ProductAttributeStatus::eNOT_AVAILABLE;
	}
};

/**
 * class CDeviceInfo provide interface to store and access device related
 * information
 */
class CDeviceInfo
{
public:
	/**
	 * Method to set serial number of device
	 * @param[in] strSerial serial number
	 * @param[in] paStatus attribute availability status
	 * @return void
	 */
	static void SetSerialNumber(const std::string strSerial,
			ic_core::IProduct::ProductAttributeStatus paStatus =
					ic_core::IProduct::ProductAttributeStatus::eDEFAULT_STATUS);

	/**
	 * Method to read serial number of device
	 * @param void
	 * @return Serial number string
	 */
	static std::string GetSerialNumber();

	/**
	 * Method to set VIN of device
	 * @param[in] strVIN Vehicle identification number
	 * @param[in] paStatus attribute availability status
	 * @return void
	 */
	static void SetVIN(const std::string strVIN,
			ic_core::IProduct::ProductAttributeStatus paStatus =
					ic_core::IProduct::ProductAttributeStatus::eDEFAULT_STATUS);

	/**
	 * Method to read serial number of device
	 * @param void
	 * @return Vehicle identification number string
	 */
	static std::string GetVIN();

	/**
	 * Method to set IMEI of device
	 * @param[in] strIMEI IMEI number
	 * @param[in] paStatus attribute availability status
	 * @return void
	 */
	static void SetIMEI(const std::string strIMEI,
			ic_core::IProduct::ProductAttributeStatus paStatus =
					ic_core::IProduct::ProductAttributeStatus::eDEFAULT_STATUS);

	/**
	 * Method to read IMEI of device
	 * @param void
	 * @return IMEI number of device in string format
	 */
	static std::string GetIMEI();

	/**
	 * Method to set the SWVersion of device
	 * @param[in] strSWVersion SW version string
	 * @param[in] paStatus attribute availability status
	 * @return void
	 */
	static void SetSWVersion(const std::string strSWVersion,
			ic_core::IProduct::ProductAttributeStatus paStatus =
					ic_core::IProduct::ProductAttributeStatus::eDEFAULT_STATUS);

	/**
	 * Method to read SW version of device
	 * @param void
	 * @return Software version of device in string format
	 */
	static std::string GetSWVersion();

	/**
	 * Method to set the HWVersion
	 * @param[in] strHWVersion HW version string
	 * @param[in] paStatus attribute availability status
	 * @return void
	 */
	static void SetHWVersion(const std::string strHWVersion,
			ic_core::IProduct::ProductAttributeStatus paStatus =
					ic_core::IProduct::ProductAttributeStatus::eDEFAULT_STATUS);

	/**
	 * Method to read HW Version of device
	 * @param void
	 * @return Hardware version of device in string format
	 */
	static std::string GetHWVersion();

	/**
	 * Method to set the client startup reason
	 * @param[in] strStartupReason client startup reason
	 * @return void
	 */
	static void SetStartupReason(const std::string strStartupReason);

	/**
	 * Method to read client startup reason
	 * @param void
	 * @return Client startup reason
	 */
	static std::string GetStartupReason();

	/**
	 * Method to set the client restart count
	 * @param[in] unRestartCount client restart count
	 * @return void
	 */
	static void SetRestartCount(const unsigned int unRestartCount);

	/**
	 * Method to read the client restart count
	 * @param void
	 * @return Client restart count
	 */
	static unsigned int GetRestartCount();

	/**
	 * Method to read CPU usage by device
	 * @param void
	 * @return CPU usage of device
	 */
	static float GetCpuLoad();

	/**
	 * Method to log current CPU usage
	 * @param void
	 * @return void
	 */
	static void LogCpuUsage();

	/**
	 * Method to read current device' free memory
	 * @param void
	 * @return Value of free memory in the device
	 */
	static unsigned long long GetFreeMem();

	/**
	 * Method to generate system snapshot by capturing CPU usage and
	 * free memory values
	 * @param[in] rstrReason Reason string
	 * @return returns 0 if snapshot event is generated else it returns
	 * -1 - not able to create heap memory
	 * -2 - if reason string is empty
	 * greater than zero - if any system error occurred
	 */
	static int CreateSystemSnapshotEvent(const std::string &rstrReason);

	/**
	 * Method to initialize device attributes
	 * @param[in] strSerial Serial number
	 * @param[in] strVIN Vehicle Identification Number
	 * @param[in] strSWVer Software version
	 * @param[in] strHWVer Hardware version
	 * @param[in] strIMEI IMEI number
	 * @param[in] strStartupReason startup reason
	 * @param[in] unRestartCount client restart count
	 * @return void
	 */
	static void Init(const std::string strSerial="",
			const std::string strVIN="", const std::string strSWVer="",
			const std::string strHWVer="", const std::string strIMEI="",
			const std::string strStartupReason="",
			const unsigned int unRestartCount=0);
protected:
	/**
	 * Member variable to store serial number
	 */
	static ProductAttribute m_paSerialNumber;
	/**
	 * Member variable to store vehicle identification Number
	 */
	static ProductAttribute m_paVIN;
	/**
	 * Member variable to store IMEI number
	 */
	static ProductAttribute m_paIMEI;

	/**
	 * Member variable to store software version
	 */
	static ProductAttribute m_paSWversion;

	/**
	 * Member variable to store hardware version
	 */
	static ProductAttribute m_paHWversion;

	/**
	 * Member variable to store client startup reason
	 */
	static std::string m_strStartupReason;

	/**
	 * Member variable to store client restart count
	 */
	static unsigned int m_unRestartCount;
};
} /* namespace ic_device */
#endif /* CDEVICEINFO_H_ */
