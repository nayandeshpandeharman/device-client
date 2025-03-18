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

#include <string.h>
#include "CIgniteLog.h"
#include "crypto/CIgniteDataSecurity.h"
#include "crypto/CAes.h"
#include "crypto/CAesGcm.h"

//! Macro for 'CIgniteDataSecurity' string
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CIgniteDataSecurity"

namespace ic_core 
{
CIgniteDataSecurity::CIgniteDataSecurity(const std::string &rstrKey, 
                                         const std::string &rstrIV)
{
    m_strKey = rstrKey;
    m_strIV = rstrIV;
    m_pAesGcm = new CAesGcm(m_strKey, m_strIV);
}

CIgniteDataSecurity::~CIgniteDataSecurity() 
{
    if (m_pAesGcm) 
    {
        delete m_pAesGcm;
    }
}

string CIgniteDataSecurity::Encrypt(const string &rstrPlaintext,
                                    bool bIsTagAppendEnabled,
                                    const string &rstrAADKey) 
{
    if (rstrPlaintext.empty()) 
    {
        return "";
    }
    
    // Use AES-GCM encryption algorithm to encrypt the plaintext
    string strEncryptedText = "";
    if (eGCM_SUCCESS != m_pAesGcm->GcmEncrypt(rstrPlaintext, strEncryptedText, 
                                              bIsTagAppendEnabled,rstrAADKey))
    {
        HCPLOG_E << "GcmEncrypt Failed!";
    }
    return strEncryptedText;
}

string CIgniteDataSecurity::Decrypt(const string &rstrBase64enctext)
{
    if (rstrBase64enctext.empty()) 
    {
        return "";
    }
    
    // Use AES-GCM decryption algorithm to decrypt the base64-encoded text
    std::string strDecryptedText;

    int nDecryptStatus = m_pAesGcm->GcmDecrypt(rstrBase64enctext,
                                               strDecryptedText);
    HCPLOG_D << "decryptStatus: " << nDecryptStatus;

    // If AES-GCM decryption fails, fall back to AES decryption
    if (eGCM_FAILURE == nDecryptStatus) 
    {
        CAes aes(m_strKey, m_strIV);
        strDecryptedText = aes.Decrypt(rstrBase64enctext);
    }

    return strDecryptedText;
}
} // namespace ic_core
