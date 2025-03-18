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

#include "gtest/gtest.h"
#include "net/CExponentialBackoff.h"

#ifdef PREFIX
#undef PREFIX
#endif

//! Macro for "test_CExponentialBackoff" string
#define PREFIX "test_CExponentialBackoff"

namespace ic_bl 
{

/**
 * Class CExponentialBackoffTest defines a test feature for 
 * CExponentialBackoff class
 */
class CExponentialBackoffTest : public ::testing::Test {
public:
    /**
     * Constructor
     */
    CExponentialBackoffTest() 
    {
        //Do nothing
    }

    /**
     * Destructor
     */
    ~CExponentialBackoffTest() override 
    {
        //Do nothing
    }

    /**
     * SetUp method : Code here will be called immediately after the
     * constructor (right before each test)
     * @see testing::Test::SetUp()
     */
    void SetUp() override 
    {
        //Do nothing
    }

    /**
     * TearDown method : Code here will be called immediately after
     * each test (right before the destructor)
     * @see testing::Test::TearDown()
     */
    void TearDown() override 
    {
        //Do nothing
    }

    /**
     * Wrapper method to access CalculateBackOffRetryTime of 
     * CExponentialBackoff class
     * @param void
     * @return Back Off retry time value
     */
    int CalculateBackOffRetryTime();

protected:
    /**
     * Overriding Method of testing::Test class
     * @see testing::Test::TestBody()
     */
    void TestBody() override 
    {
        // Do nothing
    }

};

int CExponentialBackoffTest::CalculateBackOffRetryTime()
{
    CExponentialBackoff objEB(0,60,300);
    return objEB.CalculateBackOffRetryTime();
}


//Tests
TEST_F(CExponentialBackoffTest, Test_GetNextRetryTime_errCode_OK)
{
    CExponentialBackoff obj(0,100);

    /* Setting errorCode as E_OK
     * returns 0 as successTime is set as 0
     */
    EXPECT_EQ(0, obj.GetNextRetryTime(HttpErrorCode::eERR_OK));
}

TEST_F(CExponentialBackoffTest, Test_GetNextRetryTime_errCode_other)
{
    CExponentialBackoff obj(0,100);

    /* Setting errorCode as E_OTHER
     * returns 100 as failureTime is set as 100
     */
    EXPECT_EQ(100, obj.GetNextRetryTime(HttpErrorCode::eERR_OTHER));
}

TEST_F(CExponentialBackoffTest, Test_GetNextRetryTime_errCode_serverError)
{
    CExponentialBackoff obj(0,100);

    //setting sample data 400 as failure time
    obj.SetFailureTime(400);

    //setting sample data 200 as maximum retry time
    obj.SetMaxRetryTime(200);

    /* setting errorCode as E_SERVER
     * expecting not equal to 0 as the next retry time returned is a rand number
     */
    EXPECT_NE(0, obj.GetNextRetryTime(HttpErrorCode::eERR_SERVER));
}

TEST_F(CExponentialBackoffTest, CalculateBackOffRetryTime)
{
    CExponentialBackoffTest objTest;

    /* expecting greater or equal to 0 as the backoff retry time is a random 
     * number generated using RNG
     */
    EXPECT_GE(objTest.CalculateBackOffRetryTime(),0);
}

}
