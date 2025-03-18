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
* \file CAesGcm.h
*
* \brief This class implements the AES-GCM encryption algorithm
********************************************************************************
*/

#ifndef CAES_GCM_H
#define CAES_GCM_H

#include <openssl/evp.h>
#include <openssl/aes.h>
#include <sstream>
#include "CIgniteLog.h"

using std::string;

namespace ic_core 
{
/**
 * Enum of GCM encryption/decryption status
 */
typedef enum 
{
    eGCM_SUCCESS,      ///< GCM success 
    eGCM_FAILURE,      ///< GCM failure
    eGCM_AADKEY_EMPTY, ///< GCM_AAD key empty
    eGCM_PROCESS_CONT  ///< GCM processing continue
} AesGcmReturnCode;

//! Constant key for 'AAD key min length' value
const int MIN_AADKEY_LENGTH = 5;

//! Constant key for 'AAD key pending char' value
const int AADKEY_PADDING_CHAR = 'x';

//! Constant key for 'aadstr' string
const std::string CONST_AAD_STR = "aadstr";

/**
 * Class CAesGcm implements the AES-GCM encryption algorithm
 */
class CAesGcm 
{
public:
    /**
     * Parameterized Constructor
     * @param[in] rstrKey String containing key value
     * @param[in] rstrIV String containing IV value
     */
    CAesGcm(const std::string &rstrKey, const std::string &rstrIV);

    /**
     * Destructor
     */
    ~CAesGcm();
        
    /**
     * Method to implement AES-GCM encryption Algorithm
     * @param[in] rstrPlaintext String containing plain text to encrypt
     * @param[out] rstrBase64EncodedOutput String containing base64 
     * encodedAES-GCM encrypted data
     * @param[in] bIsAADTagEnabled AAD Tag enablement status
     * @param[in] rstrAADKey String containing AAD key used for AAD generation 
     * @return eGCM_SUCCESS for Encryption Successful, eGCM_AADKEY_EMPTY when 
     * aad-key is empty, else eGCM_FAILURE for Encryption Failed
     */
    int GcmEncrypt(const std::string &rstrPlaintext,
                   std::string &rstrBase64EncodedOutput,
                   bool bIsAADTagEnabled = false,
                   const string &rstrAADKey = "");
    
    /**
     * Method to implement AES-GCM decryption Algorithm 
     * @param[in] rstrBase64enctext String containing base64 encoded data to be 
     * decrypted
     * @param[out] rstrDecyptedOut String containing AES-GCM decrypted data
     * @return eGCM_SUCCESS for Encryption Successful, else eGCM_FAILURE for 
     * Encryption Failed
     */
    int GcmDecrypt(const string &rstrBase64enctext, 
                   std::string &rstrDecyptedOut);

    #ifdef IC_UNIT_TEST
        friend class CAesGcmTest;

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
    /**
     * Method to generate AAD string using inputString
     * @param[in] rstrAADKey String containing key for generation of AAD
     * @return AAD String of 16 bytes to be used for encryption
     */
    std::string GenerateAAD(const string &rstrAADKey);

    /**
     * Method to add AAD tag based on input parameter
     * @param[in] rbIsAADTagEnabled AAD tag enablement status
     * @param[in] rstrAADKey String containing key for generation of AAD
     * @param[out] rnLen data length after updating AAD tag
     * @return eGCM_PROCESS_CONT on successful processing, otherwise return 
     * eGCM_AADKEY_EMPTY or eGCM_FAILURE enum value
     */
    AesGcmReturnCode AddAADTag(const bool &rbIsAADTagEnabled,
                               const string &rstrAADKey,
                               int &rnLen);

    /**
     * Method to encrypt data with AAD tag based on input parameter
     * @param[in] puchCipherText String containing cipher text
     * @param[in] rstrBase64ciphertext String containing base64 cipher text
     * @param[in] rnLen data length
     * @param[in] puchTag AAD tag
     * @return Encrypted data with AAD tag, empty string otherwise
     */
    string GcmEncryptWithAadTag(unsigned char *puchCipherText,
                                const string &rstrBase64ciphertext,
                                int &rnLen, unsigned char *puchTag);

    /**
     * Method to initialize initial gcm encryption operations
     * @param void
     * @return eGCM_PROCESS_CONT on successful initialization, otherwise return
     * eGCM_FAILURE enum value
     */
    AesGcmReturnCode InitGcmEncryptOperation();

    //! Member variable to store encrypted cipher context
    EVP_CIPHER_CTX* m_pEncryptCtx;

    //! Member variable to store decrypted cipher context
    EVP_CIPHER_CTX* m_pDecryptCtx;

    //! Member variable to store key value
    string m_strKey;

    //! Member variable to store IV value
    string m_strIV;
};
} // namespace ic_core
#endif // CAES_GCM_H
