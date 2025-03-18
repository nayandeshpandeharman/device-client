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
#include "analytics/CInternalMessageHandler.h"

namespace ic_bl
{
//! Global variable to store instance of CInternalMessageHandler
CInternalMessageHandler *g_pImhInstance = nullptr;

//! Define a test fixture for CInternalMessageHandler
class CInternalMessageHandlerTest : public ::testing::Test 
{
public:
    /**
     * Constructor
     */
    CInternalMessageHandlerTest () 
    {
        // do nothing
    }

    /**
     * Destructor
     */
    ~CInternalMessageHandlerTest () override 
    {
        // do nothing
    }

    /**
     * Overriding Method of testing::Test class
     * @see testing::Test::SetUp()
     */
    void SetUp() override 
    {
        g_pImhInstance = new CInternalMessageHandler();
    }

    /**
     * Overriding Method of testing::Test class
     * @see testing::Test::TearDown()
     */
    void TearDown() override 
    {
        delete g_pImhInstance;
        g_pImhInstance = nullptr;
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

// Test Case to check class creation
TEST_F(CInternalMessageHandlerTest, Test_InternalMessageHandler_ClassCreation)
{
    /* Check the existance of DeviceSetting class as Zero test case
     * as per TDD approach
     */
    EXPECT_NE(g_pImhInstance, nullptr);
}

} // namespace ic_bl
