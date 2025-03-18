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
* \file CAes.h
*
* \brief This class implements the AES encryption/decryption algorithm
********************************************************************************
*/

#ifndef CAES_H
#define CAES_H

#include <openssl/evp.h>
#include <openssl/aes.h>
#include <string>

using std::string;

namespace ic_core 
{
/**
 * Class CAes implements the AES encryption/decryption algorithm
 */
class CAes
{
public:
    /**
     * Parameterized Constructor
     * @param[in] strKey String containing key value
     * @param[in] strIv String containing IV value
     */
    CAes(string strKey, string strIv);

    /**
     * Destructor
     */
    virtual ~CAes();

    /**
     * Method to implement AES encryption Algorithm
     * @param[in] strPlaintext String containing data to be encrypted
     * @return Encrypted string
     */
    string Encrypt(string strPlaintext);

    /**
     * Method to implement AES decryption Algorithm
     * @param[in] strPlaintext String containing data to be encrypted
     * @return Decryption string
     */
    string Decrypt(string strBase64enctext);

private:
    //! Member variable to store encrypted cipher context
    EVP_CIPHER_CTX* m_pEncryptCtx;

    //! Member variable to store decrypted cipher context
    EVP_CIPHER_CTX* m_pDecryptCtx;

    //! Member variable to store key value
    string m_strKey;

    //! Member variable to store IV value
    string m_strIV;
};
} /* namespace ic_core */

#endif /* CAES_H */
