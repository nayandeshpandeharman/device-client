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
* \file CAesSeed.h
*
* \brief This class provide AesSeed key
********************************************************************************
*/

#ifndef CAES_SEED_H
#define CAES_SEED_H

#include <string>

namespace ic_core 
{
/**
 * Class CAesSeed provide AesSeed key
 */
class CAesSeed 
{
public:
    /**
     * Method to initialize AesSeed content based on input parameter
     * @param[in] strContent String contain AesSeed content
     * @param[in] bIsChanged True if AesSeed status changed, default value false
     * @return void
     */
    void Init(std::string strContent, bool bIsChanged = false);

    /**
     * Method to get the IV random number
     * @param void
     * @return random number string
     */
    std::string GetIvRandom();

    /**
     * Method to get the instance of CAesSeed class
     * @param void
     * @return instance of CAesSeed class
     */
    static CAesSeed* GetInstance();

private:
    /**
     * Default no-argument constructor.
     */
    CAesSeed();

    //! Member variable to stores AesSeed content value
    std::string m_strSeedContent;

    //! Member variable to stores used android keystore status
    bool m_bUseKeystoreAndroid;
};
} /* namespace ic_core */
#endif //CAES_SEED_H
