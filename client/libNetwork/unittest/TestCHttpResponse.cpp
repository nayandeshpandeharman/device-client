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

#include <gmock/gmock.h>
#include "gtest/gtest.h"
#include "CHttpResponse.h"
#include "../include/http/HttpErrorCodes.h"

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "Test_CHttpRequestTest"

using ::testing::MatchesRegex;

namespace ic_network
{

/**
 * Class CHttpResponseTest defines a test methods for CHttpResponse
 */
class CHttpResponsetTest : public ::testing::Test
{
protected:
    /**
     * Constructor
     */
    CHttpResponsetTest()
    {
        // do nothing
    }

    /**
     * Destructor
     */
    ~CHttpResponsetTest() override
    {
        // do nothing
    }

    /**
     * Overriding Method of testing::Test class
     * @see testing::Test::SetUp()
     */
    void SetUp() override
    {
        // do nothing
    }

    /**
     * Overriding Method of testing::Test class
     * @see testing::Test::SetUp()
     */
    void TearDown() override
    {
        // do nothing
    }

};

// Tests

TEST_F(CHttpResponsetTest, Test_CHttpResponseTest_ZeroUsecase)
{
    CHttpResponse httpResp;

    // check the last http error , it should be eERR_UNKNOWN by default
    EXPECT_EQ(httpResp.GetLastError(), HttpErrorCode::eERR_UNKNOWN);

    // check the http code, it should be -1 by default
    EXPECT_EQ(httpResp.GetHttpCode(), -1);

    // check the http response header, it should be empty by default
    EXPECT_EQ(httpResp.GetHttpResponseHeader(), "");

    // check the http response data, it should be empty by default
    EXPECT_EQ(httpResp.GetRespData(), "");

    // check the auth error, it should be false by default
    EXPECT_EQ(httpResp.IsAuthError(), false);

}

TEST_F(CHttpResponsetTest, Test_CHttpResponseTest_SetAuthError)
{
    CHttpResponse httpResp;

    httpResp.SetAuthError(true);

    // check the auth error, it should be as set before
    EXPECT_EQ(httpResp.IsAuthError(), true);
}

TEST_F(CHttpResponsetTest, Test_CHttpResponseTest_GetHttpErrorCodeString)
{

    //Check the strings corresponding to http error codes
    EXPECT_EQ(CHttpResponse::GetHttpErrorCodeString(HttpErrorCode::eERR_OK), 
              "ERR_OK");

    EXPECT_EQ(CHttpResponse::GetHttpErrorCodeString(HttpErrorCode::eERR_ACCESS),
              "ERR_ACCESS");

    EXPECT_EQ(CHttpResponse::GetHttpErrorCodeString(HttpErrorCode::eERR_TOKEN),
              "ERR_TOKEN");

    EXPECT_EQ(CHttpResponse::GetHttpErrorCodeString(HttpErrorCode::eERR_NETWORK),
              "ERR_NETWORK");

    EXPECT_EQ(CHttpResponse::GetHttpErrorCodeString(HttpErrorCode::eERR_RESPONSE_FORMAT),
              "ERR_RESPONSE_FORMAT");

    EXPECT_EQ(CHttpResponse::GetHttpErrorCodeString(HttpErrorCode::eERR_RESPONSE_DATA),
              "ERR_RESPONSE_DATA");

    EXPECT_EQ(CHttpResponse::GetHttpErrorCodeString(HttpErrorCode::eERR_TIMEOUT),
              "ERR_TIMEOUT");

    EXPECT_EQ(CHttpResponse::GetHttpErrorCodeString(HttpErrorCode::eERR_SERVER),
              "ERR_SERVER");

    EXPECT_EQ(CHttpResponse::GetHttpErrorCodeString(HttpErrorCode::eERR_BACKOFF),
              "ERR_BACKOFF");

    EXPECT_EQ(CHttpResponse::GetHttpErrorCodeString(HttpErrorCode::eERR_OTHER),
              "ERR_OTHER");
              
    EXPECT_EQ(CHttpResponse::GetHttpErrorCodeString(HttpErrorCode::eERR_UNKNOWN),
              "ERR_UNKNOWN");

    EXPECT_EQ(CHttpResponse::GetHttpErrorCodeString((HttpErrorCode)9999),
              "INVALID_ERRORCODE");
}

}
