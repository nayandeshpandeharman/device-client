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
* \file: CCrc32.h
*
* \brief: This class is used to calculate a 32-bit CRC (Cyclic 
          Redundancy Checksum) value for a string
*******************************************************************************
*/

#ifndef CCRC32_H
#define CCRC32_H
#include <string>

namespace ic_event 
{
/**
 * Crc32 class is to calculate the 32-bit CRC value for a given data
 */
class CCrc32 
{
public:
    /**
     * Method to calculate a 32-bit CRC using data and its length
     * @param[in] puchData String for which CRC is to be calculated 
     * @param[in] nLen Length of the data
     * @return 32-bit CRC value of the data
     */
    static unsigned int Calculate(const unsigned char* puchData, const unsigned int nLen);

    /**
     * Method to calculate a 32-bit CRC using data
     * @param[in] rstrInString String for which CRC is to be calculated 
     * @return 32-bit CRC value of the data
     */
    static unsigned int Calculate(const std::string& rstrInString);
};

}

#endif /* CCRC32_H */
