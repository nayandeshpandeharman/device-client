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
* \file CIgniteDataSecurity.h
*
* \brief This class is Wrapper for Encryption/decryption functionalities
********************************************************************************
*/

#ifndef CIGNITE_DATA_SECURITY_H
#define CIGNITE_DATA_SECURITY_H

#include <string.h>
#include "crypto/CAesGcm.h"

using std::string;

namespace ic_core
{
/**
 * Class CIgniteDataSecurity is Wrapper for Encryption/decryption functionalities
 */
class CIgniteDataSecurity 
{
public:
    /**
     * Parameterized Constructor
     * @param[in] rstrKey String containing secret key used for AESGCM
     * @param[in] rstrIV String containing initialization vector used for AESGCM
     */
    CIgniteDataSecurity(const std::string &rstrKey, const std::string &rstrIV);

    /**
     * Destructor
     */
    ~CIgniteDataSecurity();

    /**
     * Method to encrypt data
     * @param[in] rstrPlaintext String containing plain text to encrypt
     * @param[in] bIsTagAppendEnabled True for encryption with tagging, false
     * for without tagging
     * @param[in] rstrAADKey String containing AAD key used for AAD generation
     * @return Base64 encoded GCM encrypted data
     */
    string Encrypt(const string &rstrPlaintext, 
                   bool bIsTagAppendEnabled = false,
                   const string &rstrAADKey = "");

    /**
     * Method to decrypt base64 encoded GCM Ciphertext, If AES-GCM decryption 
     * fails, fall back to AES decryption
     * @param[in] rstrBase64enctext String containing base64 encoded AesGcm 
     * encrypted data
     * @return Decrypted data
     */
    string Decrypt(const string &rstrBase64enctext);
    
    #ifdef IC_UNIT_TEST
        friend class CIgniteDataSecurityTest;

        /**
         * @brief Method to get key string value
         * @param void
         * @return string key value
         */
        string GetKey()
        {
            return m_strKey;
        }

        /**
         * @brief Method to get IV string value
         * @param void
         * @return string IV value
         */
        string GetIV()
        {
            return m_strIV;
        }
    #endif
private:
    //! Member variable to store key value
    string m_strKey;

    //! Member variable to store IV value
    string m_strIV;

    //! Member variable to store Instance of CAesGcm class
    CAesGcm* m_pAesGcm;
};
} /* namespace ic_core */
#endif// CIGNITE_DATA_SECURITY_H