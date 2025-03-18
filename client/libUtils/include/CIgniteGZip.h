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
* \file CIgniteGZip.h
*
* \brief This file provides GZip utility
*******************************************************************************
*/

#ifndef CIGNITE_GZIP_H
#define CIGNITE_GZIP_H

namespace ic_utils
{
/**
 * A class representing a zipped message.
 */
class CZippedMsg 
{
public:  
    /**
     * Default no-argument constructor.
     */
    CZippedMsg();

    /**
     * Destructor.
     */
    ~CZippedMsg();

    //! Pointer to the buffer containing the zipped message.
    unsigned char* m_puchBuf;
    
    //! Size of the zipped message buffer.
    unsigned m_unSize;
};

/**
 * A utility class providing methods for GZip compression and decompression.
 */
class CIgniteGZip 
{
public:
    /**
     * Static method to gzip a message.
     * @param[in] puchInMsg Pointer to the input message buffer.
     * @param[in] unInMsgSize Size of the input message buffer.
     * @return CZippedMsg object containing the compressed message.
     */
    static CZippedMsg* GzipMsg(unsigned char* puchInMsg, 
                               unsigned int unInMsgSize);
};
}/* namespace ic_utils */
#endif /* CIGNITE_GZIP_H */
