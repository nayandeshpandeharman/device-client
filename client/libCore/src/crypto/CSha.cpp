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

#include <openssl/sha.h>
#include "CIgniteLog.h"
#include "crypto/CSha.h"

namespace ic_core 
{
std::string CSha::GetFileHash(const std::string &rstrFilename)
{

    std::FILE* file = std::fopen(rstrFilename.c_str(), "rb");
    char* pchOutput = new char[(2 * SHA256_DIGEST_LENGTH) + 1];
    pchOutput[0] = '\0';

    if (file)
    {
        unsigned char* pchHash = new unsigned char[SHA256_DIGEST_LENGTH];
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        const int nReadSize = 4096;
        unsigned char* puchBuffer = new unsigned char[nReadSize];
        int nBytesRead = 0;
        while ((nBytesRead = std::fread(puchBuffer, 1, nReadSize, file)))
        {
            SHA256_Update(&sha256, puchBuffer, nBytesRead);
        }
        SHA256_Final(pchHash, &sha256);
        std::fclose(file);
        delete [] puchBuffer;

        for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        {
            std::sprintf(pchOutput + i * 2, "%02x", pchHash[i]);
        }
        pchOutput[2 * SHA256_DIGEST_LENGTH] = '\0';
        delete [] pchHash;
    }

    std::string strRet(pchOutput);
    delete [] pchOutput;
    return strRet;
}
} /* namespace ic_core */
