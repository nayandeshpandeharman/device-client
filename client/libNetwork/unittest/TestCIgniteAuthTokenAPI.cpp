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
#include "CIgniteAuthTokenAPI.h"
#include "CIgniteHTTPConnector.h"
#include "CIgniteConfig.h"
#include "CIgniteClient.h"
#include "crypto/CIgniteDataSecurity.h"
#include "db/CLocalConfig.h"
#include "core/CKeyGenerator.h"

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "Test_CIgniteAuthTokenAPI"

namespace ic_network
{

/**
 * Class CIgniteAuthTokenAPITest defines a test methods for
 * CIgniteAuthTokenAPI
 */
class CIgniteAuthTokenAPITest : public ::testing::Test
{
public:
    /**
     * Constructor
     */
    CIgniteAuthTokenAPITest()
    {
        // do nothing
    }

    /**
     * Destructor
     */
    ~CIgniteAuthTokenAPITest() override
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
     * @see testing::Test::TearDown()
     */
    void TearDown() override
    {
        // do nothing
    }
    /**
     * Wrapper method to access the ParseAPIResponse of CIgniteAuthTokenAPI
     * @param[out] rResp Response class to hold the values sent from API
     * @return void
     */
    void ParseAPIResponse(CIgniteHTTPConnector::CAuthResponse &rResp);

protected:
    /**
     * Overriding Method of testing::Test class
     * @see testing::Test::TestBody()
     */
    void TestBody() override
    {

    }
};

void CIgniteAuthTokenAPITest::ParseAPIResponse(
    CIgniteHTTPConnector::CAuthResponse &rResp)
{
    CIgniteAuthTokenAPI classObj;
    classObj.ParseAPIResponse(rResp);
}

// Tests

TEST_F(CIgniteAuthTokenAPITest,
       Test_CIgniteAuthTokenAPITest_get_date_from_header)
{
    CIgniteAuthTokenAPI obj;
    std::string strDate;

    // Using the date/time section from a sample http response
    std::string strHttpRespHeader = "HTTP/1.1 200 OK Date: Fri, 14 Jun 2024 12:58:47 GMT \n";
    strDate = obj.GetDateFromHeader(strHttpRespHeader); 

    // covert the date format using get_time_format_as_login
    EXPECT_NE(strDate, "");
    EXPECT_NE(obj.GetTimeFormatAsLogin(strDate), "");
}

TEST_F(CIgniteAuthTokenAPITest, Test_ParseAPIResponse_invalidJsonData)
{
    CIgniteAuthTokenAPITest testObj;
    CIgniteHTTPConnector::CAuthResponse resp;
    resp.m_strHttpRespData = "{InvlidJSONData:";

    testObj.ParseAPIResponse(resp);
    EXPECT_EQ(resp.m_eHttpSessionErrCode, HttpErrorCode::eERR_RESPONSE_DATA);
    EXPECT_EQ(resp.m_strHttpRespData, "Parsing Failed");
}
}
