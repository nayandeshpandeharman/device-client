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

#include "CHttpResponse.h"

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CHttpResponse"

namespace ic_network
{

CHttpResponse::CHttpResponse()
{
    Reset();
}

CHttpResponse::~CHttpResponse()
{

}

void CHttpResponse::SetLastError(HttpErrorCode eErrCode)
{
    m_eLastError = eErrCode;
}

HttpErrorCode CHttpResponse::GetLastError()
{
    return m_eLastError;
}

void CHttpResponse::SetHttpCode(long lHttpCode)
{
    m_lHttpCode = lHttpCode;
}

long CHttpResponse::GetHttpCode()
{
    return m_lHttpCode;
}

void CHttpResponse::SetHttpResponseHeader(string strRespHdr)
{
    m_strHttpResponseHeader = strRespHdr;
}

string CHttpResponse::GetHttpResponseHeader()
{
    return m_strHttpResponseHeader;
}

void CHttpResponse::SetRespData(string strRespData)
{
    m_strHttpRespdata = strRespData;
}

string CHttpResponse::GetRespData()
{
    return m_strHttpRespdata;
}

void CHttpResponse::SetAuthError(bool bErr)
{
    m_bAuthError = bErr;
}

bool CHttpResponse::IsAuthError()
{
    return m_bAuthError;
}

void CHttpResponse::Reset()
{
    m_eLastError = HttpErrorCode::eERR_UNKNOWN;
    m_lHttpCode = -1;
    m_strHttpResponseHeader = "";
    m_strHttpRespdata = "";
    m_bAuthError = false;
}

const std::string CHttpResponse::GetHttpErrorCodeString(HttpErrorCode eErr)
{
    switch (eErr)
    {
        case HttpErrorCode::eERR_OK:
            return "ERR_OK";
        case HttpErrorCode::eERR_ACCESS:
            return "ERR_ACCESS";
        case HttpErrorCode::eERR_TOKEN:
            return "ERR_TOKEN";
        case HttpErrorCode::eERR_NETWORK:
            return "ERR_NETWORK";
        case HttpErrorCode::eERR_RESPONSE_FORMAT:
            return "ERR_RESPONSE_FORMAT";
        case HttpErrorCode::eERR_RESPONSE_DATA:
            return "ERR_RESPONSE_DATA";
        case HttpErrorCode::eERR_TIMEOUT:
            return "ERR_TIMEOUT";
        case HttpErrorCode::eERR_SERVER:
            return "ERR_SERVER";
        case HttpErrorCode::eERR_BACKOFF:
            return "ERR_BACKOFF";
        case HttpErrorCode::eERR_OTHER:
            return "ERR_OTHER";
        case HttpErrorCode::eERR_UNKNOWN:
            return "ERR_UNKNOWN";
        default:
            return "INVALID_ERRORCODE";
    }
}

}
