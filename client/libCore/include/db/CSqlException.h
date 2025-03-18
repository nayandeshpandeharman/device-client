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
* \file CSqlException.h
*
* \brief This class/module implements sqlite exception printing logs utilities
*******************************************************************************
*/

#ifndef CSQL_EXCEPTION_H
#define CSQL_EXCEPTION_H

#include <string>

namespace ic_core 
{
//! Macro for 'SqlException' integer value
#define ACP_SQLITE_ERROR 1000

class CSqlException 
{
public:
    /**
     * Parameterized Constructor
     * @param[in] nErrCode SqlException error code
     * @param[in] pchErrMsg SqlException error message
     */
    CSqlException(const int nErrCode, const char *pchErrMsg);

    /**
     * Copy Constructor
     * @param[in] rSqlException instance of CSqlException class
     */
    CSqlException(const CSqlException &rSqlException);

    /**
     * Destructor
     */
    virtual ~CSqlException();

    /**
     * Method to get SqlException error code
     * @param void
     * @return SqlException error code
     */
    const int ErrorCode();

    /**
     * Method to get SqlException error message
     * @param void
     * @return SqlException error message string
     */
    const std::string ErrorMessage();

    /**
     * Method to get SqlException error string based on input error code
     * @param[in] nErrCode SqlException error code
     * @return SqlException error string
     */
    const std::string ErrorCodeAsString(const int nErrCode);

private:
    //! Member variable to store SqlException error code
    int m_nErrCode;

    //! Member variable to store SqlException error message
    std::string m_strErrMessage;
};
} /* namespace ic_core */
#endif /* CSQL_EXCEPTION_H */
