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
* \file CRNG.h
*
* \brief This class provides random number generator algorithms
********************************************************************************
*/

#ifndef CRNG_H
#define CRNG_H

#include <string>

namespace ic_core 
{
/**
 * Class CRNG provides random number generator algorithms
 */
class CRNG
{
public:
    /**
     * Method to get random number based on input parameter
     * @param[in] strSeedKey Seed_key bytes
     * @param[in] nMin Minumum value range
     * @param[in] nMax Maximum value range
     * @return Random number
     */
    static int Get(std::string strSeedKey, int nMin, int nMax);

    /**
     * Method to get random number based on input parameter
     * @param[in] strSeedKey Seed_key bytes
     * @param[in] nMax Maximum value range
     * @return Random number
     */
    static int Get(std::string strSeedKey, int nMax);

    /**
     * Method to get string based on input parameter
     * @param[in] strSeedKey Seed_key bytes
     * @param[in] nMin Minimum value range
     * @param[in] nMax Maximum value range
     * @return Generated string
     */
    static std::string GetString(std::string strSeedKey, int nMin, int nMax);

    /**
     * Method to get string based on input parameter
     * @param[in] strSeedKey Seed_key bytes
     * @param[in] nMax Maximum value range
     * @return Generated string
     */
    static std::string GetString(std::string seed_key, int max);
};
} /* namespace ic_core */
#endif /* CRNG_H */
