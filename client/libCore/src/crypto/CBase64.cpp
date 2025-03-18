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

#include <string>
#include <algorithm>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include "CIgniteLog.h"
#include "crypto/CBase64.h"

namespace ic_core 
{
string CBase64::Encode( char* pchData, int nLen )
{
    if ( (pchData == NULL) || (nLen <= 0))
    {
        return "";
    }

    int nLenoutput = 0;
    /*
     * Bio is simply a class that wraps BIO* and it free the BIO in the 
     * destructor.
     * Create BIO to perform base64
     */ 
    BIO* pB64 = BIO_new(BIO_f_base64()); 

    // Ignore newlines - write everything in one line
    BIO_set_flags(pB64, BIO_FLAGS_BASE64_NO_NL);  

    // Create BIO that holds the result
    BIO* pMem = BIO_new(BIO_s_mem()); 

    /*
     * Chain base64 with mem, so writing to b64 will encode base64 and write to 
     * memory.
     */
    BIO_push(pB64, pMem);

    // write data
    bool bDone = false;
    int nRes = 0;
    while(!bDone)
    {
        nRes = BIO_write(pB64, pchData, nLen);

        if(nRes <= 0) // if failed
        {
            if(BIO_should_retry(pB64))
            {
                continue;
            }
            else // encoding failed
            {
                /* Handle Error?? */
            }
        }
        else // success!
        {
            bDone = true;
        }
    }

    BIO_flush(pB64);

    // get a pointer to mem's data
    char* pchOutput;
    nLenoutput = BIO_get_mem_data(pMem, &pchOutput);
    string strRet(pchOutput, nLenoutput);

    // Make sure we call free_all on the *start* of the chain
    BIO_free_all(pB64);

    return strRet;
}

/**
 * Global function to check data is ending or not
 * @param[in] strText String containing text data
 * @param[in] strEnding String containing end data
 * @return True if successfully ended, false otherwise
     */
static bool has_ending (string strText, string strEnding)
{
    if (strText.length() >= strEnding.length())
    {
        return (0 == strText.compare(strText.length() - strEnding.length(), 
                 strEnding.length(), strEnding));
    }
    else
    {
        return false;
    }
}

char* CBase64::Decode( string strInput, int &rnLenoutput)
{
    if (strInput.empty())
    {
        return NULL;
    }

    // Remove special chars
    strInput.erase(std::remove(strInput.begin(), strInput.end(), '\n'),
                               strInput.end());
    strInput.erase(std::remove(strInput.begin(), strInput.end(), '\r'),
                               strInput.end());

    // Check padding and determine output length
    rnLenoutput = (int)strInput.length() * 0.75;
    if (has_ending(strInput, "=="))
    {
        // Two characters of padding
        rnLenoutput -= 2;
    }
    else if (has_ending(strInput, "="))
    {
        // One character of padding
        rnLenoutput--;
    }
    else
    {
        // Do nothing
    }

    // Allocate output buffer.  Caller is responsible for freeing this memory!
    char* pchOutput = (char*)malloc(rnLenoutput + 1); 

    // Create BIO to perform base64
    BIO* pB64 = BIO_new(BIO_f_base64());

    // Ignore newlines - write everything in one line
    BIO_set_flags(pB64, BIO_FLAGS_BASE64_NO_NL);

    // create BIO that holds the result
    BIO* pMem = BIO_new_mem_buf((void*)strInput.c_str(), strInput.length()); 

    /*
     * Chain base64 with mem, so reading from b64 will decode base64 and write 
     * to mem.
     */ 
    BIO_push(pB64, pMem);

    // read data
    bool bDone = false;
    int nRes = 0;
    while(!bDone)
    {
        nRes = BIO_read(pB64, pchOutput, rnLenoutput);

        if(nRes <= 0) // if failed
        {
            if(BIO_should_retry(pB64))
            {
                continue;
            }
            else // encoding failed
            {
                /*
                 * Breake the loop in case of failure. Since ret is 0 here a
                 * NULL will be returned indicating an error
                 */ 
                break;
            }
        }
        else
        {
            bDone = true;
        }
    }

    BIO_flush(pB64);
    BIO_free_all(pB64);

    if (nRes != rnLenoutput)
    {
        HCPLOG_T << "Bad Base64 Decode.  decodedlen=" << nRes 
                 << ", calculatedlen=" << rnLenoutput;
        free(pchOutput);
        return NULL;
    }

    pchOutput[rnLenoutput] = '\0';
    return pchOutput;
}
} /* namespace ic_core */
