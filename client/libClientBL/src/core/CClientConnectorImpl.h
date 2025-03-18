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
* \file CClientConnectorImpl.h
*
* \brief This class implements 'IExtendedClientConnector' interface to provide 
* implementations for the APIs
********************************************************************************
*/

#ifndef CCLIENT_CONNECTOR_IMPL_H
#define CCLIENT_CONNECTOR_IMPL_H

#include "IExtendedClientConnector.h"

namespace ic_bl
{
/**
 * Class CClientConnectorImpl provides interfaces to implements 
 * 'IExtendedClientConnector' APIs
 */
class CClientConnectorImpl : public IExtendedClientConnector 
{
public:
    /**
     * Destructor
     */
    ~CClientConnectorImpl();

    /**
     * Overriding Method of IExtendedClientConnector
     * @see IExtendedClientConnector::GetActivationState()
     */
    void GetActivationState(bool &rbState, int &rnReason) override;

    /**
     * Method to get location
     * @param void
     * @return location string
     */
    std::string GetLocation();

    /**
     * Method to get attribute based on input parameter
     * @param[in] eAttr device attribute enum
     * @return attribute value
     */
    std::string GetAttribute(DeviceAttribute eAttr);

    /**
     * Method to get config value based on key
     * @param[in] strConfigKey config key
     * @return config value
     */
    std::string GetConfig(std::string strConfigKey);

    /**
     * Method to persist configuration based on input parameters
     * @param[in] strConfigKey config key
     * @param[in] strConfigValue config value
     * @return true on success, false otherwise
     */
    bool PersistConfig(std::string strConfigKey, std::string strConfigValue);

    /**
     * Method to get Instance of CClientConnectorImpl
     * @param void
     * @return Pointer to an instance of CClientConnectorImpl
     */
    static CClientConnectorImpl* GetInstance();

private:
    /**
     * Default no-argument constructor.
     */
    CClientConnectorImpl();
};
} /* namespace ic_bl */
#endif /* CCLIENT_CONNECTOR_IMPL_H */
