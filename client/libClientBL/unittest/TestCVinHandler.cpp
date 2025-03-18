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
#include "analytics/CVinHandler.h"
#include "db/CLocalConfig.h"

namespace ic_bl
{
//! Global variable to store instance of CVinHandler
CVinHandler *g_pVinHandlerObj = NULL;

//! Define a test fixture for CVinHandlerTest
class CVinHandlerTest : public ::testing::Test 
{
public:
    /**
     * Wrapper method for GetInstance of CVinHandler class
     * @see CVinHandler::GetInstance()
     */
    CVinHandler *GetInstance();

    /**
     * Wrapper method for ProcessEvent of CVinHandler class
     * @see CVinHandler::ProcessEvent()
     */
    void ProcessEvent(ic_core::CEventWrapper& rEvent);
    
    /**
     * Constructor
     */
    CVinHandlerTest() 
    {
        // do nothing
    }

    /**
     * Destructor
     */ 
    ~CVinHandlerTest() override 
    {
        // do nothing
    }

    /**
     * SetUp method.
     * Code here will be called immediately after the constructor (right
     * before each test).
     */
    void SetUp() override 
    {
        g_pVinHandlerObj =  CVinHandler::GetInstance();
    }

    /**
     * TearDown method
     * Code here will be called immediately after each test (right
     * before the destructor).
     */
    void TearDown() override 
    {
        g_pVinHandlerObj = NULL;
    }

protected:
    /**
     * Overriding Method of testing::Test class
     * @see testing::Test::TestBody()
     */
    void TestBody() override 
    {
        // do nothing
    }
};

CVinHandler* CVinHandlerTest::GetInstance()
{
    return g_pVinHandlerObj->GetInstance();
}

void CVinHandlerTest::ProcessEvent(ic_core::CEventWrapper& rEvent)
{
    return g_pVinHandlerObj->ProcessEvent(rEvent);
}

//Tests

TEST_F(CVinHandlerTest, Test_checkVinHandlerClassCreation) 
{
    /* Check the existance of CVinHandler class as Zero test case 
     * as per TDD approach
     */
    CVinHandlerTest obj;
}

TEST_F(CVinHandlerTest, Test_getInstance) 
{
    CVinHandlerTest obj;
    EXPECT_EQ(g_pVinHandlerObj, obj.GetInstance());
}

TEST_F(CVinHandlerTest, Test_getInstanceManyCaseValidation) 
{
    CVinHandler *pVinHandlerObj1 = CVinHandler::GetInstance();
    CVinHandler *pVinHandlerObj2 = CVinHandler::GetInstance();
    CVinHandler *pVinHandlerObj3 = CVinHandler::GetInstance();
    CVinHandler *pVinHandlerObj4 = CVinHandler::GetInstance();
    CVinHandlerTest obj;
    EXPECT_EQ(pVinHandlerObj1,obj.GetInstance());
    EXPECT_EQ(pVinHandlerObj2,obj.GetInstance());
    EXPECT_EQ(pVinHandlerObj3,obj.GetInstance());
    EXPECT_EQ(pVinHandlerObj4,obj.GetInstance());
}

TEST_F(CVinHandlerTest, Test_processEventEmptyVin)
{
    CVinHandlerTest obj;
    std::string strEvent = "{\"Data\":{\"value\":\"\"},\"EventID\":"
    "\"VIN\",\"Timestamp\":1657100235572,\"Timezone\":0,\"Version\":\"1.0\"}";
    ic_core::CEventWrapper event;
    event.JsonToEvent(strEvent);
    obj.ProcessEvent(event);
    EXPECT_STREQ("NA", 
                 ic_core::CLocalConfig::GetInstance()->Get("VIN").c_str());
}

TEST_F(CVinHandlerTest, Test_processEventVin)
{
    CVinHandlerTest obj;
    std::string strEvent = "{\"Data\":{\"value\":"
    "\"TESTVIN1234567898AA\"},\"EventID\":\"VIN\",\"Timestamp\":"
    "1657100235572,\"Timezone\":0,\"Version\":\"1.0\"}";
    ic_core::CEventWrapper event;
    event.JsonToEvent(strEvent);
    obj.ProcessEvent(event);
    EXPECT_STREQ("TESTVIN1234567898AA", 
                 ic_core::CLocalConfig::GetInstance()->Get("VIN").c_str());
}

}