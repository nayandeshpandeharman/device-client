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

#include "crypto/CAes.h"
#include "crypto/CBase64.h"

#include <cstdlib>
#include <string>

namespace ic_core 
{
CAes::CAes(string strKey, string strIV)
{
    m_strKey = strKey;
    m_strIV = strIV;

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

CAes::~CAes()
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

string CAes::Encrypt(string strPlaintext)
{
    if (strPlaintext.empty())
    {
        return "";
    }

    int nOutlen = 0;
    int nTmplen = 0;
    int nInlen = strPlaintext.length();

    char* pchOutbuf = (char*)malloc(nInlen + AES_BLOCK_SIZE);
    if (pchOutbuf == NULL)
    {
        return "";
    }

    if(!EVP_EncryptInit_ex(m_pEncryptCtx, EVP_aes_128_cbc(), NULL,
                           (const unsigned char*)m_strKey.c_str(),
                           (const unsigned char*)m_strIV.c_str()))
    {
        free(pchOutbuf);
        return "";
    }

    if(!EVP_EncryptUpdate(m_pEncryptCtx, (unsigned char*)pchOutbuf, &nOutlen,
                          (const unsigned char*)strPlaintext.c_str(), nInlen))
    {
        free(pchOutbuf);
        return "";
    }

    /* 
     * Buffer passed to EVP_EncryptFinal() must be after data just
     * encrypted to avoid overwriting it.
     */
    if(!EVP_EncryptFinal_ex(m_pEncryptCtx, (unsigned char*)pchOutbuf + nOutlen,
                            &nTmplen))
    {
        free(pchOutbuf);
        return "";
    }

    nOutlen += nTmplen;
    string strRet = CBase64::Encode(pchOutbuf, nOutlen);
    free(pchOutbuf);
    return strRet;
}

string CAes::Decrypt(string strBase64enctext)
{

    if (strBase64enctext.empty())
    {
        return "";
    }

    int nOutlen = 0;
    int nTmplen = 0;

    //Initializing variable to default value
    int nEnclen = 0;

    char* pchEnc = CBase64::Decode(strBase64enctext, nEnclen);
    if (pchEnc == NULL)
    {
        return "";
    }

    char* pchOuttext = (char*)malloc(nEnclen + 1);
    if (pchOuttext == NULL)
    {
        free(pchEnc);
        return "";
    }

    if(!EVP_DecryptInit_ex(m_pDecryptCtx, EVP_aes_128_cbc(), NULL,
                           (unsigned char*)m_strKey.c_str(),
                           (unsigned char*)m_strIV.c_str()))
    {
        free(pchOuttext);
        free(pchEnc);
        return "";
    }

    if(!EVP_DecryptUpdate(m_pDecryptCtx, (unsigned char*)pchOuttext, 
                          (int*)&nOutlen,(const unsigned char*)pchEnc, nEnclen))
    {
        free(pchOuttext);
        free(pchEnc);
        return "";
    }

    if(!EVP_DecryptFinal_ex(m_pDecryptCtx,(unsigned char*)pchOuttext + nOutlen,
                           (int*)&nTmplen))
    {
        free(pchOuttext);
        free(pchEnc);
        return "";
    }

    nOutlen += nTmplen;

    string strRet(pchOuttext, nOutlen);
    free(pchOuttext);
    free(pchEnc);
    return strRet;
}
} // namespace ic_core 