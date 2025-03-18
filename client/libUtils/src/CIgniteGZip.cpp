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
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "CIgniteGZip.h"
#include "CIgniteLog.h"

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CIgniteGZip"

namespace ic_utils
{
CZippedMsg::CZippedMsg()
{
    m_puchBuf = NULL;
    m_unSize = 0;
}

CZippedMsg::~CZippedMsg()
{
    if (m_puchBuf != NULL)
    {
        delete []m_puchBuf;
    }
}

CZippedMsg* CIgniteGZip::GzipMsg(unsigned char* puchInMsg, 
                                 unsigned int unInMsgSize)
{
    HCPLOG_METHOD();
    int ret;
    unsigned have;
    z_stream strm;

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    ret = deflateInit2(&strm, 
                        Z_DEFAULT_COMPRESSION, 
                        Z_DEFLATED, 
                        MAX_WBITS + 16, 
                        MAX_MEM_LEVEL,
                        Z_DEFAULT_STRATEGY);

    if (ret != Z_OK)
    {
        HCPLOG_E << "Cannot initiate gzip";
        return NULL;
    }
    if(ret == Z_STREAM_ERROR)
    {
        HCPLOG_E << "gzip stream error:" << ret;
        return NULL;
    }

    unsigned int reqSize = deflateBound(&strm, unInMsgSize);
    unsigned char* out = new unsigned char[reqSize];
    strm.avail_in = unInMsgSize;
    strm.next_in = puchInMsg;
    strm.avail_out = reqSize;
    strm.next_out = out;

    ret = deflate(&strm, Z_FINISH);
    if(ret == Z_STREAM_ERROR)
    {
        HCPLOG_E << "gzip stream error:" << ret;
    }

    HCPLOG_I << "Input size is " << unInMsgSize<< ", GZip output size is: " \
             << strm.avail_out << "!";

    have = reqSize - strm.avail_out;
    if (have != strm.total_out)
    {
        HCPLOG_E << "Have=" << have << ", total_out=" << strm.total_out << "!";
    }

    CZippedMsg* zm = new CZippedMsg();
    zm->m_puchBuf = out;
    zm->m_unSize = have;
    deflateEnd(&strm);

    return zm;
}

}/* namespace ic_utils */
