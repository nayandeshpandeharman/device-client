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

#include <cstring>
#include "CIgniteLog.h"
#include "crypto/CAesGcm.h"
#include "crypto/CBase64.h"
#include "openssl/err.h"

using std::string;

//! Macro for 'CAesGcm' string
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CAesGcm"

//! Macro for 'Tag Length' integer value
#define TAG_LENGTH 16

namespace ic_core 
{
CAesGcm::CAesGcm(const std::string &rstrKey, const std::string &rstrIV):
                 m_strKey(rstrKey), m_strIV(rstrIV)
{
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    m_pEncryptCtx = (EVP_CIPHER_CTX*) malloc(sizeof(EVP_CIPHER_CTX));
    EVP_CIPHER_CTX_init(m_pEncryptCtx);

    m_pDecryptCtx = (EVP_CIPHER_CTX*) malloc(sizeof(EVP_CIPHER_CTX));
    EVP_CIPHER_CTX_init(m_pDecryptCtx);
#else //ssl 1.1.x support
    m_pEncryptCtx = EVP_CIPHER_CTX_new();
    m_pDecryptCtx = EVP_CIPHER_CTX_new();
#endif
}

CAesGcm::~CAesGcm() 
{
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    EVP_CIPHER_CTX_cleanup(m_pEncryptCtx);
    free(m_pEncryptCtx);
    EVP_CIPHER_CTX_cleanup(m_pDecryptCtx);
    free(m_pDecryptCtx);
#else //ssl 1.1.x support
    EVP_CIPHER_CTX_free(m_pEncryptCtx);
    EVP_CIPHER_CTX_free(m_pDecryptCtx);
#endif
}

int CAesGcm::GcmEncrypt(const std::string &rstrPlaintext, 
                        std::string &rstrBase64EncodedOutput,
                        bool bIsAADTagEnabled, const string & rstrAADKey)
{
    HCPLOG_METHOD();
    if (rstrPlaintext.empty())
    {
        HCPLOG_E << "puchPlaintext is empty";
        return eGCM_FAILURE;
    }
    unsigned char uchStrTag[TAG_LENGTH] = {0}; // Buffer for the tag
    int nLen= -1, nCiphertextLen= -1;  
    int nPlaintTextLen = rstrPlaintext.length();

    if (eGCM_FAILURE == InitGcmEncryptOperation())
    {
        // Since one of the initialization steps has failed, return from here
        return eGCM_FAILURE;
    }

    AesGcmReturnCode eRetCode = AddAADTag(bIsAADTagEnabled, rstrAADKey, nLen);
    if (eGCM_PROCESS_CONT != eRetCode)
    {
        // AAD processing failed, return eGCM_AADKEY_EMPTY or eGCM_FAILURE
        return eRetCode;
    }

    //Disable padding
    if (1 != EVP_CIPHER_CTX_set_padding(m_pEncryptCtx,0))
    {
        HCPLOG_E << "set_padding failed";
        return eGCM_FAILURE;
    }

    unsigned char *puchCipherText = nullptr;
    try
    {
        puchCipherText = new unsigned char[nPlaintTextLen];
    }
    catch(std::bad_alloc & ba)
    {
        HCPLOG_E << "bad_alloc caught";
        return eGCM_FAILURE;
    }

    // Provide the message to be encrypted and obtain the encrypted output
    if (1 != EVP_EncryptUpdate(m_pEncryptCtx, puchCipherText, &nLen, 
                  reinterpret_cast<const unsigned char*>(rstrPlaintext.c_str()), 
                  rstrPlaintext.size()))
    {
        delete [] puchCipherText;
        HCPLOG_E << "EVP_EncryptUpdate failed";
        return eGCM_FAILURE;
    }
    nCiphertextLen = nLen;

    // Convert puchCipherText to base64
    string strBase64ciphertext = CBase64::Encode(reinterpret_cast<char*>
                                              (puchCipherText), nCiphertextLen);
    if (strBase64ciphertext.empty())
    {
        delete [] puchCipherText;
        HCPLOG_E << "Base64 encode failed";
        return eGCM_FAILURE;
    }

    if(bIsAADTagEnabled)
    {
        string strEncryptData = GcmEncryptWithAadTag(puchCipherText,
                                                          strBase64ciphertext,
                                                          nLen, uchStrTag);
        if (strEncryptData.empty())
        {
            return eGCM_FAILURE;
        }
        
        // PuchCipherText + strTag
        rstrBase64EncodedOutput = strEncryptData;
    }
    else 
    {
        // Only encrypted cipher text
        rstrBase64EncodedOutput = strBase64ciphertext;
    }
    delete [] puchCipherText;
    HCPLOG_I << "GCM Encryption Successful: "<<rstrBase64EncodedOutput;
    return eGCM_SUCCESS;
}

AesGcmReturnCode CAesGcm::InitGcmEncryptOperation()
{
    // Initialize the encryption operation
    if (1 != EVP_EncryptInit_ex(m_pEncryptCtx, EVP_aes_128_gcm(), 
                                NULL, NULL, NULL))
    {
        HCPLOG_E << "EVP_EncryptInit_ex failed";
        return eGCM_FAILURE;
    }

    // Set IV length
    if (1 != EVP_CIPHER_CTX_ctrl(m_pEncryptCtx, EVP_CTRL_GCM_SET_IVLEN, 
                                 m_strIV.size(), NULL))
    {
        HCPLOG_E << "EVP_CTRL_GCM_SET_IVLEN failed";
        return eGCM_FAILURE;
    }

    // Initialize key and IV
    if (1 != EVP_EncryptInit_ex(m_pEncryptCtx, NULL, NULL, 
                       reinterpret_cast<const unsigned char*>(m_strKey.c_str()), 
                       reinterpret_cast<const unsigned char*>(m_strIV.c_str())))
    {
        HCPLOG_E << "EVP_EncryptInit_ex key,iv failed";
        return eGCM_FAILURE;
    }
    return eGCM_PROCESS_CONT;
}

AesGcmReturnCode CAesGcm::AddAADTag(const bool &rbIsAADTagEnabled,
                                    const string & rstrAADKey,
                                    int &rnLen)
{
    // Provide any AAD data required
    if(rbIsAADTagEnabled) 
    {
        // Use AAD only if rstrAADKey not empty and for ActivationUsecase.
        if (rstrAADKey.empty())
        {
            HCPLOG_E << "AAD key is empty";
            return eGCM_AADKEY_EMPTY;
        }
        std::string strAAD = "";
        strAAD = GenerateAAD(rstrAADKey);
        HCPLOG_I << "AAD= "<<strAAD;
        if (1 != EVP_EncryptUpdate(m_pEncryptCtx, NULL, &rnLen, 
                         reinterpret_cast<const unsigned char*>(strAAD.c_str()),
                         strAAD.length()))
        {
            HCPLOG_E << "EVP_EncryptUpdate aad failed";
            return eGCM_FAILURE;
        }
    }
    return eGCM_PROCESS_CONT;
}

string CAesGcm::GcmEncryptWithAadTag(unsigned char *puchCipherText,
                                     const string &rstrBase64ciphertext,
                                     int &rnLen, unsigned char *puchTag)
{
    if (1 != EVP_EncryptFinal_ex(m_pEncryptCtx, puchCipherText + rnLen,&rnLen))
    {
        delete [] puchCipherText;
        HCPLOG_E << "EVP_EncryptFinal_ex failed";
        return "";
    }
    
    //Get the tag
    if (1 != EVP_CIPHER_CTX_ctrl(m_pEncryptCtx, EVP_CTRL_GCM_GET_TAG, 
        TAG_LENGTH, puchTag))
    {
        delete [] puchCipherText;
        HCPLOG_E << "EVP_CTRL_GCM_GET_TAG failed";
        return "";
    }

    // Convert strTag to base64
    string strBase64tag = CBase64::Encode(reinterpret_cast<char*>
                                            (puchTag), TAG_LENGTH);
    if (strBase64tag.empty()) 
    {
        delete [] puchCipherText;
        HCPLOG_E << "strBase64tag encode failed";
        return "";
    }

    // PuchCipherText + strTag
    return (rstrBase64ciphertext + strBase64tag);
}

int CAesGcm::GcmDecrypt(const string &rstrBase64enctext, string &rstrDecyptedOut)
{
    HCPLOG_METHOD();
    // Decode the base64-encoded puchCipherText and strTag
    int nCiphertextLen= -1;
    char *pchBase64DecodedText = 
                   CBase64::Decode(rstrBase64enctext, nCiphertextLen);

    if (nullptr == pchBase64DecodedText) 
    {
        HCPLOG_E << "decoded pchBase64DecodedText is empty";
        return eGCM_FAILURE;
    }

    unsigned char *puchCipherText = reinterpret_cast<unsigned char*>
                                      (const_cast<char*>(pchBase64DecodedText));
    
    int nLen= -1, nplaintext_len= -1;

    // Initialize the decryption operation
    if (!EVP_DecryptInit_ex(m_pDecryptCtx, EVP_aes_128_gcm(), NULL, NULL, NULL))
    {
        free(pchBase64DecodedText);
        HCPLOG_E << "Error in Init";
        return eGCM_FAILURE;
    }

    //Disable padding
    if (1 != EVP_CIPHER_CTX_set_padding(m_pDecryptCtx,0))
    {
        free(pchBase64DecodedText);
        HCPLOG_E << "set_padding failed";
        return eGCM_FAILURE;
    }
    
    // Set IV length
    if (!EVP_CIPHER_CTX_ctrl(m_pDecryptCtx, EVP_CTRL_GCM_SET_IVLEN, 
                             m_strIV.size(), NULL))
    {
        free(pchBase64DecodedText);
        HCPLOG_E << "Error in SET_IVLEN";
        return eGCM_FAILURE;
    }

    // Initialize key and IV
    if (!EVP_DecryptInit_ex(m_pDecryptCtx, NULL, NULL, 
                       reinterpret_cast<const unsigned char*>(m_strKey.c_str()), 
                       reinterpret_cast<const unsigned char*>(m_strIV.c_str())))
    {
        free(pchBase64DecodedText);
        HCPLOG_E << "Error in init key & iv";
        return eGCM_FAILURE;
    }

    unsigned char *puchPlaintext = nullptr;
    try
    {
        puchPlaintext = new unsigned char[nCiphertextLen];
    }
    catch(std::bad_alloc & ba)
    {
        HCPLOG_E << "bad_alloc caught";
        free(pchBase64DecodedText);
        return eGCM_FAILURE;
    }

    if (!EVP_DecryptUpdate(m_pDecryptCtx, puchPlaintext, &nLen, puchCipherText, 
                        nCiphertextLen))
    {
        HCPLOG_E << "Error in DecryptUpdate";
        free(pchBase64DecodedText);
        delete [] puchPlaintext;
        return eGCM_FAILURE;
    }
    nplaintext_len = nLen;
    rstrDecyptedOut = string(reinterpret_cast<char*>(puchPlaintext),
                                                     nplaintext_len);

    HCPLOG_I << "GCM Decryption Successful" << rstrDecyptedOut;

    free(pchBase64DecodedText);
    delete [] puchPlaintext;
    return eGCM_SUCCESS;
}

std::string CAesGcm::GenerateAAD(const string &rstrAADKey)
{
    std::string strTemp = rstrAADKey;
    std::string strAAD = "";
    if (strTemp.size() < MIN_AADKEY_LENGTH)
    {
        //if Key is less than MIN_AADKEY_LENGTH pad it with AADKEY_PADDING_CHAR-'x'
        strTemp.insert(strTemp.end(), MIN_AADKEY_LENGTH - strTemp.length(),
                       AADKEY_PADDING_CHAR);
        strAAD = strTemp + CONST_AAD_STR + strTemp;
    }
    else
    {
        //FirstNchars of rstrAADKey +CONST_AAD_STR +LastNcharacters of rstrAADKey
        strAAD = strTemp.substr(0,MIN_AADKEY_LENGTH) + CONST_AAD_STR
                    + strTemp.substr(rstrAADKey.size() - MIN_AADKEY_LENGTH);
    }
    return strAAD;
}
} // namespace ic_core
