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
* \file CLocalConfig.h
*
* \brief This class/module provide methods for local database utilities
*******************************************************************************
*/

#ifndef CLOCAL_CONFIG_H
#define CLOCAL_CONFIG_H

#include <string>

namespace ic_core 
{
//! Constant key for 'dataEncryRndNo' string
static const std::string DATA_ENCRYPT_RND_NO = "dataEncryRndNo";

/**
 * This class used to provides methods for local database utilities
 */
class CLocalConfig 
{
public:
    /**
     * Method to get instance of CLocalConfig class
     * @param void
     * @return instance of CLocalConfig class
     */
    static CLocalConfig* GetInstance();

    /**
     * Method to initialize local database
     * @param void
     * @return void
     */
    void Init();

    /**
     * Method to set data based on input parameter
     * @param[in] strKey string containing key value
     * @param[in] strVal string containing data
     * @return true if data is successfully setted, false otherwise
     */
    bool Set(std::string strKey, std::string strVal);

    /**
     * Method to get data string based on input parameter
     * @param[in] strKey string containing key value
     * @return data string
     */
    std::string Get(std::string strKey);

    /**
     * Method to remove data from database based on input parameter
     * @param[in] strKey string containing key value
     * @return 0 if data from database is successfully removed, non-zero 
     * otherwise
     */
    int Remove(std::string strKey);

    /**
     * Method to count rows from database table based on input parameter
     * @param[in] strKey string containing key value
     * @return 0 if rows are successfully counted, non-zero otherwise
     */
    int CountRowsStartsWithKey(std::string strKey);

    /**
     * Method to remove rows from database table based on input parameter
     * @param[in] strKey string containing key value
     * @return 0 if rows are successfully removed, non-zero otherwise
     */
    int RemoveRowsStartsWithKey(std::string strKey);

    /**
     * Method to get IV random number based on input parameter
     * @param[in] strSeedKey string containing seed key value
     * @return IV random number string
     */
    std::string GetIvRandomNumber(std::string strSeedKey);

private:
    /**
     * Default no-argument constructor.
     */
    CLocalConfig();

    /**
     * Destructor
     */
    ~CLocalConfig();

protected:
    //! Member variable to store table name
    static const std::string m_strTableName;
};
} /* namespace ic_core */
#endif /* CLOCAL_CONFIG_H */