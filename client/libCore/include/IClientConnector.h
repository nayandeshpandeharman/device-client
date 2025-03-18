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
* \file IClientConnector.h
*
* \brief This class/module provides base interface using which message-handlers
* can interact with the client for message handling purposes and responses
********************************************************************************
*/

#ifndef ICLIENT_CONNECTOR_H
#define ICLIENT_CONNECTOR_H

#include <string>

namespace ic_core 
{
/**
 * Class IClientConnector is interface class to be implemented by Ignite product
 * for connection
 */
class IClientConnector 
{
public:
    /**
     * Default no-argument constructor.
     */
    IClientConnector()
    {

    };

    /**
     * Destructor
     */
    virtual ~IClientConnector()
    {

    };

    /**
     * Enum of client/device attributes
     */
    typedef enum 
    {
        eVIN,           ///< VIN
        eSERIAL_NUMBER, ///< Serial number
        eIMEI,          ///< IMEI number
        eDEVICE_ID      ///< Device id
    } DeviceAttribute;

    /**
     * Method to get the location payload.
     * @param void
     * @return location payload in string
     */
    virtual std::string GetLocation() = 0;

    /**
     * Method to get the requested attribute
     * @param[in] eDeviceAttr attribute enum
     * @return attribute value in string
     */
    virtual std::string GetAttribute(DeviceAttribute eDeviceAttr) = 0;

    /**
     * Method to get the requested config value
     * @param[in] strConfigKey config key
     * @return config value in string
     */
    virtual std::string GetConfig(std::string strConfigKey) = 0;

    /**
     * Method to persist config key and config value.
     * @param[in] strConfigKey config key
     * @param[in] strConfigValue config value
     * @return true if value is persisted, false otherwise
     */
    virtual bool PersistConfig(std::string strConfigKey, 
        std::string strConfigValue) = 0;
};
} /* namespace  ic_core */
#endif /* ICLIENT_CONNECTOR_H */
