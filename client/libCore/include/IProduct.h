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
* \file Client.h
*
* \brief This class/module implements client related utilities.     
*******************************************************************************
*/

#ifndef IPRODUCT_H
#define IPRODUCT_H

#include <string>
#include "IClientMessageDispatcher.h"
#include "IMessageHandler.h"
#include "IOnOff.h"

namespace ic_core
{

/**
 * Class IProduct is an pure abstract class defines the interfaces to
 * interact with the product.
 */
class IProduct
{
public:
    /**
     * Enum of product attributes
     */
    enum ProductAttribute
    {
        eVIN,           ///< Vin attribute
        eSerialNumber,  ///< Serial number attribute
        eSWVersion,     ///< Software version attribute
        eHWVersion,     ///< Hardware version attribute
        eIMEI           ///< IMEI attribute
    };

    /**
     * Enum of product attributes status
     */
    enum ProductAttributeStatus
    {
        eNO_ERROR,      ///< Production no error status
        eNOT_AVAILABLE, ///< Production not available status
        eNOT_SUPPORTED, ///< Production not supported status
        eDEFAULT_STATUS ///< Production default status
    };

    /**
     * Virtual Destructor
     */
    virtual ~IProduct() {}

    /**
     * Method to get the product attribute based on input parameter
     * @param[in] eKey product attribute enum
     * @return product attribute value string
     */
    virtual std::string GetAttribute(const ProductAttribute eKey) = 0;

    /**
     * Method to get the client startup reason
     * @param void
     * @return client startup reason string
     */
    virtual std::string GetStartupReason() = 0;

    /**
     * Method to get the client restart count
     * @param void
     * @return client restart count
     */
    virtual unsigned int GetRestartCount() = 0;

    /**
     * Method to generate event based on input parameter
     * @param[in] rStrKey attribute key
     * @return true if event is generated, false otherwise
     */
    virtual bool GenerateEvent(const std::string &rStrKey) = 0;

    /**
     * Method to get cpu load value
     * @param void
     * @return cpu load value
     */
    virtual float GetCpuLoad() = 0;

    /**
     * Method to get activation qualifier ID
     * @param void
     * @return qualifier ID string
     */
    virtual std::string GetActivationQualifierID() = 0;

    /**
     * Method to set product attribute based on input parameter
     * @param[in] eAttribute attribute enum value
     * @param[in] strValue attribute value
     * @param[in] eStatus attribute status enum value
     * @return true if requested attribute is set, false otherwise
     */
    virtual bool SetAttribute(ProductAttribute eAttribute, std::string strValue, 
                              ProductAttributeStatus eStatus = eNOT_AVAILABLE) = 0;

};
} /* namespace acp */
#endif /* IPRODUCT_H */
