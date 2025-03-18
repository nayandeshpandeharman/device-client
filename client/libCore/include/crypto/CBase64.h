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
* \file CBase64.h
*
* \brief This class implements the Base64 encryption algorithm
********************************************************************************
*/

#ifndef CBASE64_H
#define CBASE64_H

#include <string>

using std::string;

namespace ic_core 
{
/**
 * Class CBase64 implements the Base64 encoding/decoding algorithm
 */
class CBase64 
{
public:
    /**
     * Method to implement base-64 encoding Algorithm
     * @param[in] pchData Char pointer data to encrypt
     * @param[in] nLen Data length
     * @return Encoded string
     */
    static string Encode(char* pchData, int nLen);

    /**
     * Method to implement base-64 decoding Algorithm
     * @param[in] strInput String containing input data
     * @param[in] rnLenoutput Data length
     * @return Decoded string
     */
    static char* Decode(string strInput, int &rnLenoutput);
};
} /* namespace ic_core */

#endif /* CBASE64_H */
