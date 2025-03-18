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

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "db/CSqlException.h"
#include "CIgniteLog.h"
#include "CIgniteEvent.h"
#include "CIgniteFileUtils.h"
#include "CIgniteStringUtils.h"

namespace ic_core 
{
CSqlException::CSqlException(const int nErrCode, const char *pchErrMsg):
                           m_nErrCode(nErrCode)
{
    HCPLOG_METHOD();
    m_strErrMessage = ErrorCodeAsString(m_nErrCode);
    m_strErrMessage.append("[");
    m_strErrMessage.append(
                    ic_utils::CIgniteStringUtils::NumberToString(m_nErrCode));
    m_strErrMessage.append("]:");
    if (pchErrMsg)
    {
        m_strErrMessage.append(std::string(pchErrMsg));
    }
    HCPLOG_T << m_strErrMessage;
}

CSqlException::CSqlException(const CSqlException &rSqlException):
                            m_nErrCode(rSqlException.m_nErrCode),
                            m_strErrMessage(rSqlException.m_strErrMessage)
{
    HCPLOG_METHOD();
}

const std::string CSqlException::ErrorCodeAsString(const int nErrCode)
{
    HCPLOG_METHOD();
    switch (nErrCode)
    {
    case SQLITE_OK:
        return "SQLITE_OK";
    case SQLITE_ERROR:
        return "SQLITE_ERROR";
    case SQLITE_INTERNAL:
        return "SQLITE_INTERNAL";
    case SQLITE_PERM:
        return "SQLITE_PERM";
    case SQLITE_ABORT:
        return "SQLITE_ABORT";
    case SQLITE_BUSY:
        return "SQLITE_BUSY";
    case SQLITE_LOCKED:
        return "SQLITE_LOCKED";
    case SQLITE_NOMEM:
        return "SQLITE_NOMEM";
    case SQLITE_READONLY:
        return "SQLITE_READONLY";
    case SQLITE_INTERRUPT:
        return "SQLITE_INTERRUPT";
    case SQLITE_IOERR:
        return "SQLITE_IOERR";
    case SQLITE_CORRUPT:
        return "SQLITE_CORRUPT";
    case SQLITE_NOTFOUND:
        return "SQLITE_NOTFOUND";
    case SQLITE_FULL:
        return "SQLITE_FULL";
    case SQLITE_CANTOPEN:
        return "SQLITE_CANTOPEN";
    case SQLITE_PROTOCOL:
        return "SQLITE_PROTOCOL";
    case SQLITE_EMPTY:
        return "SQLITE_EMPTY";
    case SQLITE_SCHEMA:
        return "SQLITE_SCHEMA";
    case SQLITE_TOOBIG:
        return "SQLITE_TOOBIG";
    case SQLITE_CONSTRAINT:
        return "SQLITE_CONSTRAINT";
    case SQLITE_MISMATCH:
        return "SQLITE_MISMATCH";
    case SQLITE_MISUSE:
        return "SQLITE_MISUSE";
    case SQLITE_NOLFS:
        return "SQLITE_NOLFS";
    case SQLITE_AUTH:
        return "SQLITE_AUTH";
    case SQLITE_FORMAT:
        return "SQLITE_FORMAT";
    case SQLITE_RANGE:
        return "SQLITE_RANGE";
    case SQLITE_ROW:
        return "SQLITE_ROW";
    case SQLITE_DONE:
        return "SQLITE_DONE";
    case ACP_SQLITE_ERROR:
        return "ACP_SQLITE_ERROR";
    default:
        return "UNKNOWN_ERROR";
    }
}

const int CSqlException::ErrorCode()
{
    return m_nErrCode;
}

const std::string CSqlException::ErrorMessage()
{
    return m_strErrMessage;
}

CSqlException::~CSqlException()
{

}
} /* namespace ic_core */