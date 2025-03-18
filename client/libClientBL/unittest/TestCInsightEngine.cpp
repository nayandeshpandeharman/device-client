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
#include "dam/CInsightEngine.h"
#include "CIgniteEvent.h"

namespace ic_bl 
{
//! string constant for message queue name
static const std::string MESSAGE_QUEUE_NAME = "/tmp/ut_test";

/**
 * Class CInsightEngineTest defines a test feature
 * for CInsightEngine class
 */
class CInsightEngineTest : public ::testing::Test
{
public:
    /**
     * Method to check Object creation and member parameters initialized
     * @param void
     * @return true if object created and initialized successfully, else return
     * false
     */
    static bool CreateObjectAndInitialize();
 
    /**
     * Method to check DataAggregation is suspended
     * @param void
     * @return true if DataAggregation is suspended else return false
     */
    static bool SuspendDataAggregation();

    /**
     * Method to check DataAggregation is resumed
     * @param void
     * @return true if DataAggregation is resumed else return false
     */
    static bool ResumeDataAggregation();

protected:
    /**
     * Constructor
     */
    CInsightEngineTest()
    {
        // do nothing
    }

    /**
     * Destructor
     */
    ~CInsightEngineTest() override
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
};

bool CInsightEngineTest::CreateObjectAndInitialize()
{
    //Creating message queue
    ic_core::CMessageQueue msgQueueObj(MESSAGE_QUEUE_NAME);

    //create CInsightEngine instance with m_bIsDataAggregationActive set to true
    CInsightEngine inEngineObj(&msgQueueObj, true);

    //return false if member variables are not initilized
    if ((false == inEngineObj.m_bIsDataAggregationActive) ||
        (false == inEngineObj.m_bIsInitialEventsThreadLaunched) ||
        (nullptr == inEngineObj.m_pEvtReceiver))
    {
        return false;
    }

    return true;
}

bool CInsightEngineTest::SuspendDataAggregation()
{
    //Creating message queue
    ic_core::CMessageQueue msgQueueObj(MESSAGE_QUEUE_NAME);

    //create CInsightEngine instance with m_bIsDataAggregationActive set to true
    CInsightEngine inEngineObj(&msgQueueObj, true);

    //calling SuspendDataAggregation
    inEngineObj.SuspendDataAggregation();

    //data aggregation is suspended if m_bIsDataAggregationActive set to false
    return !inEngineObj.m_bIsDataAggregationActive;
}

bool CInsightEngineTest::ResumeDataAggregation()
{
    //Creating message queue
    ic_core::CMessageQueue msgQueueObj(MESSAGE_QUEUE_NAME);

    //create CInsightEngine instance with m_bIsDataAggregationActive set false
    CInsightEngine inEngineObj(&msgQueueObj, false);

    //calling ResumeDataAggregation
    inEngineObj.ResumeDataAggregation();

    //data aggregation is resumed if m_bIsDataAggregationActive set to true
    return inEngineObj.m_bIsDataAggregationActive;
}

TEST_F(CInsightEngineTest, Test_ZeroTetsCase_ForCInsightEngine) 
{
    //Expect true as the object should be created and initialized correctly
    EXPECT_TRUE(CInsightEngineTest::CreateObjectAndInitialize());
}

TEST_F(CInsightEngineTest, Test_SuspendDataAggregation_ForCInsightEngine) 
{
    //Expect true as suspending of DataAggregation will be success
    EXPECT_TRUE(CInsightEngineTest::SuspendDataAggregation());
}

TEST_F(CInsightEngineTest, Test_ResumeDataAggregation_ForCInsightEngine) 
{
    //Expect true as resuming of DataAggregation will be success
    EXPECT_TRUE(CInsightEngineTest::ResumeDataAggregation());
}
}