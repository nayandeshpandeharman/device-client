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
* \file CDefaultProductImpl.h
*
* \brief CDefaultProductImpl implements IProduct interface to provide concrete
* implementation
*******************************************************************************
*/

#ifndef CDEFAULT_PRODUCT_IMPL_H
#define CDEFAULT_PRODUCT_IMPL_H

#include "CIgniteClient.h"
#include <string>

namespace ic_device
{
/**
 * class CDefaultProductImpl implements IProduct interface to provide concrete
 * implementation
 */
class CDefaultProductImpl : public ic_core::IProduct
{
public:
    /**
     * Default constructor
     */
    CDefaultProductImpl();

    /**
     * Virtual destructor
     */
    virtual ~CDefaultProductImpl();

    /**
     * This function performs initialization steps
     * @param[in] strSerial Serial number
     * @param[in] strVIN Vehicle Identification Number
     * @param[in] strSWVer Software version
     * @param[in] strHWVer Hardware version
     * @param[in] strIMEI IMEI number
     * @param[in] strStartupReason client startup reason
     * @param[in] unRestartCount client restart count
     * @return void
     */
    static void StartupInit(const std::string strSerial="",
            const std::string strVIN="", const std::string strSWVer="",
            const std::string strHWVer="", const std::string strIMEI="",
            const std::string strStartupReason="",
            const unsigned int unRestartCount=0);

    /**
     * Overriding Method of ic_core::IProduct class
     * @see ic_core::IProduct::GetAttribute(const ProductAttribute eKey)
     */
    virtual std::string GetAttribute(const ProductAttribute key) override;

    /**
     * Overriding Method of ic_core::IProduct class
     * @see ic_core::IProduct::GetStartupReason()
     */
    virtual std::string GetStartupReason();

    /**
     * Overriding Method of ic_core::IProduct class
     * @see ic_core::IProduct::GetRestartCount()
     */
    virtual unsigned int GetRestartCount();

    /**
     * Overriding Method of ic_core::IProduct class
     * @see ic_core::IProduct::GenerateEvent(const std::string &rStrKey)
     */
    virtual bool GenerateEvent(const std::string &rstrKey) override;

    /**
     * Overriding Method of ic_core::IProduct class
     * @see ic_core::IProduct::GetActivationQualifierID
     */
    virtual std::string GetActivationQualifierID() override;

    /**
     * Overriding Method of ic_core::IProduct class
     * @see ic_core::IProduct::GetCpuLoad()
     */
    virtual float GetCpuLoad() override;

    /**
     * Overriding Method of ic_core::IProduct class
     * @see ic_core::IProduct::SetAttribute(
     *                      ProductAttribute eAttribute, std::string strValue,
     *                      ProductAttributeStatus eStatus = eNOT_AVAILABLE)
     */
    virtual bool SetAttribute(ProductAttribute attribute,
                            std::string strValue,
                            ProductAttributeStatus status = eNOT_AVAILABLE) override;

    #ifdef IC_UNIT_TEST
        friend class CDefaultProductImplTest;
    #endif

};
} //namespace
#endif /* CDEFAULT_PRODUCT_IMPL_H */
