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
* \file CContentValues.h
*
* \brief This class/module implements methods to prepare data content utilities
*******************************************************************************
*/

#ifndef CCONTENT_VALUES_H
#define CCONTENT_VALUES_H

#include <string>
#include <vector>
#include "jsoncpp/json.h"

namespace ic_core 
{
/**
 * This class implements data content preparation utilities
 */
class CContentValues 
{
public:
    /**
     * Default no-argument constructor.
     */
    CContentValues();
    
    /**
     * Method to put value based on key value
     * @param[in] strKey string containing key value
     * @param[in] nValue content value
     * @return void
     */
    void Put(std::string strKey, int nValue);

    /**
     * Method to put value based on key value
     * @param[in] strKey string containing key value
     * @param[in] llValue content value
     * @return void
     */
    void Put(std::string strKey, long long llValue);

    /**
     * Method to put value based on key value
     * @param[in] strKey string containing key value
     * @param[in] strValue string containing content value
     * @return void
     */
    void Put(std::string strKey, std::string strValue);

    /**
     * Method to put value based on key value
     * @param[in] strKey string containing key value
     * @param[in] pchValue content value pointer
     * @return void
     */
    void Put(std::string strKey, const char *pchValue);

    /**
     * Method to put value based on key value
     * @param[in] strKey string containing key value
     * @param[in] bValue content boolean value
     * @return void
     */
    void Put(std::string strKey, bool bValue);

    /**
     * Method to put value based on key value
     * @param[in] strKey string containing key value
     * @param[in] fltValue content float value
     * @return void
     */
    void Put(std::string strKey, float fltValue);

    /**
     * Method to put value based on key value
     * @param[in] strKey string containing key value
     * @param[in] dblValue content double value
     * @return void
     */
    void Put(std::string strKey, double dblValue);
    
    /**
     * Method to get data as integer based on input parameter
     * @param[in] strKey string containing key value
     * @param[in] nDef default value
     * @return value as integer
     */
    int GetAsInt(std::string strKey, int nDef = 0);

    /**
     * Method to get data as long based on input parameter
     * @param[in] strKey string containing key value
     * @param[in] llDef default value
     * @return value as long long
     */
    long long GetAsLong(std::string strKey, long long llDef = 0l);

    /**
     * Method to get data as string based on input parameter
     * @param[in] strKey string containing key value
     * @param[in] strDef string containing default value
     * @return value as string
     */
    std::string GetAsString(std::string strKey, std::string strDef = "");

    /**
     * Method to get data as boolean based on input parameter
     * @param[in] strKey string containing key value
     * @param[in] bDef default value
     * @return boolean value
     */
    bool GetAsBool(std::string strKey, bool bDef = false);

    /**
     * Method to get data as float based on input parameter
     * @param[in] strKey string containing key value
     * @param[in] fltDef default value
     * @return float value
     */
    float GetAsFloat(std::string strKey, float fltDef = 0.0f);

    /**
     * Method to get data as double based on input parameter
     * @param[in] strKey string containing key value
     * @param[in] dblDef default value
     * @return double value
     */
    double GetAsDouble(std::string strKey, double dblDef = 0.0f);

    /**
     * Method to get key
     * @param void
     * @return vector containing key value
     */
    std::vector<std::string> GetKeys();

    /**
     * Method to clear data
     * @param void
     * @return void
     */
    void Clear();

    /**
     * Method to get the size of content data
     * @param void
     * @return content data size
     */
    int Size();

    #ifdef IC_UNIT_TEST
        friend class CContentValuesTest;
    #endif
private:
    /**
     * Method to get string based on input parameter
     * @param[in] strKey string containing key value
     * @return string value
     */
    std::string GetString(std::string strKey);

    //! Member variable to instance of CDatabase class
    ic_utils::Json::Value m_jsonData;
};
} /* namespace ic_core */
#endif /* CCONTENT_VALUES_H */