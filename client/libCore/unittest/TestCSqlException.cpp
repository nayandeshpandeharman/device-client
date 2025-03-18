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
#include <string>
#include "gtest/gtest.h"
#include "db/CSqlException.h"
#include "CIgniteLog.h"

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "test_CSqlException"

namespace ic_core
{
/**
 * Class CSqlExceptionTest defines a test feature for CSqlException class
 */
class CSqlExceptionTest : public ::testing::Test
{
protected:
    /**
     * Constructor
     */
    CSqlExceptionTest()
    {
        // Do nothing
    }

    /**
     * Destructor
     */ 
    ~CSqlExceptionTest() override
    {
        // Do nothing
    }

    /**
     * Overriding Method of testing::Test class
     * @see testing::Test::SetUp()
     */
    void SetUp() override
    { 
        // Do nothing
    }

    /**
     * Overriding Method of testing::Test class
     * @see testing::Test::SetUp()
     */
    void TearDown() override
    {
        // Do nothing
    }
};

// Tests

TEST_F(CSqlExceptionTest, Test_ErrorCodeAsString1)
{
    int nErrCode = SQLITE_OK;           // SQLite Result code
    std::string strErrMsg= "ErrorMsg";  // Dummy error msg
    CSqlException obj(nErrCode, strErrMsg.c_str());

    EXPECT_EQ("SQLITE_OK", obj.ErrorCodeAsString(nErrCode));
}

TEST_F(CSqlExceptionTest, Test_ErrorCodeAsString2)
{
    int nErrCode = SQLITE_ERROR;        // SQLite Result code
    std::string strErrMsg= "ErrorMsg";  // Dummy error msg
    CSqlException obj(nErrCode, strErrMsg.c_str());

    EXPECT_EQ("SQLITE_ERROR", obj.ErrorCodeAsString(nErrCode));
}

TEST_F(CSqlExceptionTest, Test_ErrorCodeAsString3)
{
    int nErrCode = SQLITE_INTERNAL;     // SQLite Result code
    std::string strErrMsg= "ErrorMsg";  // Dummy error msg
    CSqlException obj(nErrCode, strErrMsg.c_str());

    EXPECT_EQ("SQLITE_INTERNAL", obj.ErrorCodeAsString(nErrCode));
}

TEST_F(CSqlExceptionTest, Test_ErrorCodeAsString4)
{
    int nErrCode = SQLITE_PERM;         // SQLite Result code
    std::string strErrMsg= "ErrorMsg";  // Dummy error msg
    CSqlException obj(nErrCode, strErrMsg.c_str());

    EXPECT_EQ("SQLITE_PERM", obj.ErrorCodeAsString(nErrCode));
}

TEST_F(CSqlExceptionTest, Test_ErrorCodeAsString5)
{
    int nErrCode = SQLITE_ABORT;        // SQLite Result code
    std::string strErrMsg= "ErrorMsg";  // Dummy error msg
    CSqlException obj(nErrCode, strErrMsg.c_str());

    EXPECT_EQ("SQLITE_ABORT", obj.ErrorCodeAsString(nErrCode));
}

TEST_F(CSqlExceptionTest, Test_ErrorCodeAsString6)
{
    int nErrCode = SQLITE_BUSY;         // SQLite Result code
    std::string strErrMsg= "ErrorMsg";  // Dummy error msg
    CSqlException obj(nErrCode, strErrMsg.c_str());

    EXPECT_EQ("SQLITE_BUSY", obj.ErrorCodeAsString(nErrCode));
}

TEST_F(CSqlExceptionTest, Test_ErrorCodeAsString7)
{
    int nErrCode = SQLITE_LOCKED;       // SQLite Result code
    std::string strErrMsg= "ErrorMsg";  // Dummy error msg
    CSqlException obj(nErrCode, strErrMsg.c_str());

    EXPECT_EQ("SQLITE_LOCKED", obj.ErrorCodeAsString(nErrCode));
}

TEST_F(CSqlExceptionTest, Test_ErrorCodeAsString8)
{
    int nErrCode = SQLITE_NOMEM;        // SQLite Result code
    std::string strErrMsg= "ErrorMsg";  // Dummy error msg
    CSqlException obj(nErrCode, strErrMsg.c_str());

    EXPECT_EQ("SQLITE_NOMEM", obj.ErrorCodeAsString(nErrCode));
}

TEST_F(CSqlExceptionTest, Test_ErrorCodeAsString9)
{
    int nErrCode = SQLITE_READONLY;     // SQLite Result code
    std::string strErrMsg= "ErrorMsg";  // Dummy error msg
    CSqlException obj(nErrCode, strErrMsg.c_str());

    EXPECT_EQ("SQLITE_READONLY", obj.ErrorCodeAsString(nErrCode));
}

TEST_F(CSqlExceptionTest, Test_ErrorCodeAsString10)
{
    int nErrCode = SQLITE_INTERRUPT;    // SQLite Result code
    std::string strErrMsg= "ErrorMsg";  // Dummy error msg
    CSqlException obj(nErrCode, strErrMsg.c_str());

    EXPECT_EQ("SQLITE_INTERRUPT", obj.ErrorCodeAsString(nErrCode));
}

TEST_F(CSqlExceptionTest, Test_ErrorCodeAsString11)
{
    int nErrCode = SQLITE_IOERR;        // SQLite Result code
    std::string strErrMsg= "ErrorMsg";  // Dummy error msg
    CSqlException obj(nErrCode, strErrMsg.c_str());

    EXPECT_EQ("SQLITE_IOERR", obj.ErrorCodeAsString(nErrCode));
}

TEST_F(CSqlExceptionTest, Test_ErrorCodeAsString12)
{
    int nErrCode = SQLITE_CORRUPT;      // SQLite Result code
    std::string strErrMsg= "ErrorMsg";  // Dummy error msg
    CSqlException obj(nErrCode, strErrMsg.c_str());

    EXPECT_EQ("SQLITE_CORRUPT", obj.ErrorCodeAsString(nErrCode));
}

TEST_F(CSqlExceptionTest, Test_ErrorCodeAsString13)
{
    int nErrCode = SQLITE_NOTFOUND;     // SQLite Result code
    std::string strErrMsg= "ErrorMsg";  // Dummy error msg
    CSqlException obj(nErrCode, strErrMsg.c_str());

    EXPECT_EQ("SQLITE_NOTFOUND", obj.ErrorCodeAsString(nErrCode));
}

TEST_F(CSqlExceptionTest, Test_ErrorCodeAsString14)
{
    int nErrCode = SQLITE_FULL;         // SQLite Result code
    std::string strErrMsg= "ErrorMsg";  // Dummy error msg
    CSqlException obj(nErrCode, strErrMsg.c_str());

    EXPECT_EQ("SQLITE_FULL", obj.ErrorCodeAsString(nErrCode));
}

TEST_F(CSqlExceptionTest, Test_ErrorCodeAsString15)
{
    int nErrCode = SQLITE_CANTOPEN;     // SQLite Result code
    std::string strErrMsg= "ErrorMsg";  // Dummy error msg
    CSqlException obj(nErrCode, strErrMsg.c_str());

    EXPECT_EQ("SQLITE_CANTOPEN", obj.ErrorCodeAsString(nErrCode));
}

TEST_F(CSqlExceptionTest, Test_ErrorCodeAsString16)
{
    int nErrCode = SQLITE_PROTOCOL;     // SQLite Result code
    std::string strErrMsg= "ErrorMsg";  // Dummy error msg
    CSqlException obj(nErrCode, strErrMsg.c_str());
    EXPECT_EQ("SQLITE_PROTOCOL", obj.ErrorCodeAsString(nErrCode));
}

TEST_F(CSqlExceptionTest, Test_ErrorCodeAsString17)
{
    int nErrCode = SQLITE_EMPTY;        // SQLite Result code
    std::string strErrMsg= "ErrorMsg";  // Dummy error msg
    CSqlException obj(nErrCode, strErrMsg.c_str());

    EXPECT_EQ("SQLITE_EMPTY", obj.ErrorCodeAsString(nErrCode));
}

TEST_F(CSqlExceptionTest, Test_ErrorCodeAsString18)
{
    int nErrCode = SQLITE_SCHEMA;       // SQLite Result code
    std::string strErrMsg= "ErrorMsg";  // Dummy error msg
    CSqlException obj(nErrCode, strErrMsg.c_str());

    EXPECT_EQ("SQLITE_SCHEMA", obj.ErrorCodeAsString(nErrCode));
}

TEST_F(CSqlExceptionTest, Test_ErrorCodeAsString19)
{
    int nErrCode = SQLITE_TOOBIG;       // SQLite Result code
    std::string strErrMsg= "ErrorMsg";  // Dummy error msg
    CSqlException obj(nErrCode, strErrMsg.c_str());

    EXPECT_EQ("SQLITE_TOOBIG", obj.ErrorCodeAsString(nErrCode));
}

TEST_F(CSqlExceptionTest, Test_ErrorCodeAsString20)
{
    int nErrCode = SQLITE_CONSTRAINT;   // SQLite Result code
    std::string strErrMsg= "ErrorMsg";  // Dummy error msg
    CSqlException obj(nErrCode, strErrMsg.c_str());

    EXPECT_EQ("SQLITE_CONSTRAINT", obj.ErrorCodeAsString(nErrCode));
}

TEST_F(CSqlExceptionTest, Test_ErrorCodeAsString21)
{
    int nErrCode = SQLITE_MISMATCH;     // SQLite Result code
    std::string strErrMsg= "ErrorMsg";  // Dummy error msg
    CSqlException obj(nErrCode, strErrMsg.c_str());

    EXPECT_EQ("SQLITE_MISMATCH", obj.ErrorCodeAsString(nErrCode));
}

TEST_F(CSqlExceptionTest, Test_ErrorCodeAsString22)
{
    int nErrCode = SQLITE_MISUSE;       // SQLite Result code
    std::string strErrMsg= "ErrorMsg";  // Dummy error msg
    CSqlException obj(nErrCode, strErrMsg.c_str());

    EXPECT_EQ("SQLITE_MISUSE", obj.ErrorCodeAsString(nErrCode));
}

TEST_F(CSqlExceptionTest, Test_ErrorCodeAsString23)
{
    int nErrCode = SQLITE_NOLFS;        // SQLite Result code
    std::string strErrMsg= "ErrorMsg";  // Dummy error msg
    CSqlException obj(nErrCode, strErrMsg.c_str());

    EXPECT_EQ("SQLITE_NOLFS", obj.ErrorCodeAsString(nErrCode));
}

TEST_F(CSqlExceptionTest, Test_ErrorCodeAsString24)
{
    int nErrCode = SQLITE_AUTH;         // SQLite Result code
    std::string strErrMsg= "ErrorMsg";  // Dummy error msg
    CSqlException obj(nErrCode, strErrMsg.c_str());

    EXPECT_EQ("SQLITE_AUTH", obj.ErrorCodeAsString(nErrCode));
}

TEST_F(CSqlExceptionTest, Test_ErrorCodeAsString25)
{
    int nErrCode = SQLITE_FORMAT;       // SQLite Result code
    std::string strErrMsg= "ErrorMsg";  // Dummy error msg
    CSqlException obj(nErrCode, strErrMsg.c_str());

    EXPECT_EQ("SQLITE_FORMAT", obj.ErrorCodeAsString(nErrCode));
}

TEST_F(CSqlExceptionTest, Test_ErrorCodeAsString26)
{
    int nErrCode = SQLITE_RANGE;        // SQLite Result code
    std::string strErrMsg= "ErrorMsg";  // Dummy error msg
    CSqlException obj(nErrCode, strErrMsg.c_str());

    EXPECT_EQ("SQLITE_RANGE", obj.ErrorCodeAsString(nErrCode));
}

TEST_F(CSqlExceptionTest, Test_ErrorCodeAsString27)
{
    int nErrCode = SQLITE_ROW;          // SQLite Result code
    std::string strErrMsg= "ErrorMsg";  // Dummy error msg
    CSqlException obj(nErrCode, strErrMsg.c_str());

    EXPECT_EQ("SQLITE_ROW", obj.ErrorCodeAsString(nErrCode));
}

TEST_F(CSqlExceptionTest, Test_ErrorCodeAsString28)
{
    int nErrCode = SQLITE_RANGE;        // SQLite Result code
    std::string strErrMsg= "ErrorMsg";  // Dummy error msg
    CSqlException obj(nErrCode, strErrMsg.c_str());

    EXPECT_EQ("SQLITE_RANGE", obj.ErrorCodeAsString(nErrCode));
}

TEST_F(CSqlExceptionTest, Test_ErrorCodeAsString29)
{
    int nErrCode = SQLITE_ROW;          // SQLite Result code
    std::string strErrMsg= "ErrorMsg";  // Dummy error msg
    CSqlException obj(nErrCode, strErrMsg.c_str());

    EXPECT_EQ("SQLITE_ROW", obj.ErrorCodeAsString(nErrCode));
}

TEST_F(CSqlExceptionTest, Test_ErrorCodeAsString30)
{
    int nErrCode = SQLITE_DONE;         // SQLite Result code
    std::string strErrMsg= "ErrorMsg";  // Dummy error msg
    CSqlException obj(nErrCode, strErrMsg.c_str());

    EXPECT_EQ("SQLITE_DONE", obj.ErrorCodeAsString(nErrCode));
}

TEST_F(CSqlExceptionTest, Test_ErrorCodeAsString31)
{
    int nErrCode = ACP_SQLITE_ERROR;    // SQLite Result code
    std::string strErrMsg= "ErrorMsg";  // Dummy error msg
    CSqlException obj(nErrCode, strErrMsg.c_str());

    EXPECT_EQ("ACP_SQLITE_ERROR", obj.ErrorCodeAsString(nErrCode));
}

TEST_F(CSqlExceptionTest, Test_ErrorCodeAsString_default)
{
    int nErrCode = 10000000;            // SQLite Result code
    std::string strErrMsg= "ErrorMsg";  // Dummy error msg
    CSqlException obj(nErrCode, strErrMsg.c_str());
    
    EXPECT_EQ("UNKNOWN_ERROR", obj.ErrorCodeAsString(nErrCode));
}

}
