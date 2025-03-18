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

#include <zlib.h>
#include "CCrc32.h"

namespace ic_event 
{
    
unsigned int CCrc32::Calculate(const unsigned char* puchData, const unsigned int nLen)
{
    uLong crc = crc32((uLong)0L, (const Bytef*)Z_NULL, (uInt)0);
    return crc32((uLong)crc, (const Bytef*)puchData, (uInt)nLen);
}

unsigned int CCrc32::Calculate(const std::string& rstrInString)
{
    uLong crc = crc32((uLong)0L, (const Bytef*)Z_NULL, (uInt)0);
    return crc32((uLong)crc, (const Bytef*)rstrInString.c_str(), (uInt)rstrInString.length());
}

}
