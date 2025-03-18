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
#include "core/CEventIntervalValidator.h"

namespace ic_bl 
{
//! Creating a CActivityDelay pointer
ic_bl::CEventIntervalValidator *g_pEventIntervalValidator = NULL;

/** 
 * Class CEventIntervalValidatorTest defines a test feature 
 * for CEventIntervalValidator class
 */
class CEventIntervalValidatorTest : public ::testing::Test 
{
public:
    /**
     * Constructor
     */
    CEventIntervalValidatorTest() 
    {
        // Do nothing
    }

    /**
     * Destructor
     */
    ~CEventIntervalValidatorTest() override 
    {
        // Do nothing
    }

    /**
     * SetUp method : Code here will be called immediately after the
     * constructor (right before each test)
     * @see testing::Test::SetUp()
     */
    void SetUp() override 
    {
        g_pEventIntervalValidator =  CEventIntervalValidator::GetInstance();
    }

    /**
     * TearDown method : Code here will be called immediately after
     * each test (right before the destructor)
     * @see testing::Test::TearDown()
     */
    void TearDown() override 
    {
        g_pEventIntervalValidator = NULL;
    }

    /**
     * Wrapper method to access GetInstance of CEventIntervalValidator
     * @param void
     * @return Pointer to singleton object of CEventIntervalValidator
     */
    CEventIntervalValidator *GetInstance();

    /**
     * Wrapper method to access IsValidInterval of CEventIntervalValidator
     * @param[in] strEventId Event ID
     * @param[in] llTimeStamp Timestamp
     * @return True if valid interval or interval validation is disabled,
     *         false otherwise.
     */
    bool IsValidInterval(std::string strEventId, long long llTimeStamp);

    /**
     * Wrapper method to access ValidateInterval of CEventIntervalValidator
     * @param[in] llCurTimestamp Current timestamp of the event
     * @param[in] llPrevTimestamp Timestamp of the event stored prevously
     * @param[in] nInterval Interval value with which the difference between
     *                      two timestamps should be compared
     * @return True if difference between two timestamp is greater than or equal
     *         to nInterval else return false
     */
    bool ValidateInterval(long long llCurTimestamp, long long llPrevTimestamp,
                                                              int nInterval);

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

CEventIntervalValidator* CEventIntervalValidatorTest::GetInstance()
{
    return g_pEventIntervalValidator->GetInstance();
}

bool CEventIntervalValidatorTest::IsValidInterval(std::string strEventId, 
                                                        long long llTimeStamp)
{
    return g_pEventIntervalValidator->IsValidInterval(strEventId, llTimeStamp);
}

bool CEventIntervalValidatorTest::ValidateInterval(long long llCurTimestamp, 
                                      long long llPrevTimestamp, int nInterval)
{
    return g_pEventIntervalValidator->ValidateInterval(llCurTimestamp, 
                                                    llPrevTimestamp, nInterval);
}

//Tests

TEST_F(CEventIntervalValidatorTest, Test_getInstance) 
{
    CEventIntervalValidatorTest obj;

    /* check the existance of IgniteHTTPConnector class as Zero test case as 
     * per TDD approach
     */
    EXPECT_EQ(g_pEventIntervalValidator, obj.GetInstance());
}

TEST_F(CEventIntervalValidatorTest, Test_isValidIntervalFalse)
{
    CEventIntervalValidatorTest obj;

    // Assigning sample values to eventID and timestamp
    std::string strEventId =  "";
    long long llTimeStamp = 1554349111200;

    // Expecting true as interval is valid
    EXPECT_TRUE(obj.IsValidInterval(strEventId, llTimeStamp));
}

TEST_F(CEventIntervalValidatorTest, Test_isValidIntervalEmptyEventId)
{
    CEventIntervalValidatorTest obj;

    // Assigning sample values to eventID and timestamp
    std::string strEventId =  "";
    long long llTimeStamp = 1554349111300;

    // Expecting true as interval is valid
    EXPECT_TRUE(obj.IsValidInterval(strEventId, llTimeStamp));
}

TEST_F(CEventIntervalValidatorTest, Test_isValidIntervalts)
{
    CEventIntervalValidatorTest obj;
    std::string stdEventId =  "Speed";
    long long llTimeStamp = 1554349111210;

    /* If it is first time for interval check, true is always expected.
     * so, check two times so that 2nd check will get the actual result.
     */
    bool bIsValid = obj.IsValidInterval(stdEventId, llTimeStamp);

    /* 3secs(3000msecs) is configured in the configuration file as interval, so
     * increment the timestamp by 1sec as a sample.
     */

    //1sec*1000 msecs
    llTimeStamp += 1*1000; 
    bIsValid = obj.IsValidInterval(stdEventId, llTimeStamp);

    // Expecting true as interval is not valid
    EXPECT_FALSE(obj.IsValidInterval(stdEventId, llTimeStamp));
}

TEST_F(CEventIntervalValidatorTest, Test_isValidInterval_InvEventId)
{
    CEventIntervalValidatorTest obj;

    // Assigning sample values to eventID and timestamp
    std::string strEventId =  "DTCStored";
    long long llTimeStamp = 1554349111211;

    // Expecting true as interval is valid
    EXPECT_TRUE(obj.IsValidInterval(strEventId, llTimeStamp)); 
}

TEST_F(CEventIntervalValidatorTest, Test_validateInterval)
{
    CEventIntervalValidatorTest obj;

    // Assigning sample values to current and previous timestamps ad interval
    long long llCurTimeStamp = 1554349111200;
    long long llPrevTimeStamp = 1554349111100;
    int nInterval = 10;

    /* Expecting true as difference between both timestamps is not
     * less than interval
     */
    EXPECT_TRUE(obj.ValidateInterval(llCurTimeStamp,llPrevTimeStamp,nInterval));
}

TEST_F(CEventIntervalValidatorTest, Test_validateIntervalValue)
{
    CEventIntervalValidatorTest obj;

    // Assigning sample values to current and previous timestamps ad interval
    long long llCurTimeStamp = 1554349111400;
    long long llPrvTimeStamp = 1554349111200;
    int nInterval = 1000;

    /* Expecting false as difference between both timestamps is 
     * less than interval
     */
    EXPECT_FALSE(obj.ValidateInterval(llCurTimeStamp,llPrvTimeStamp,nInterval));
}

}
