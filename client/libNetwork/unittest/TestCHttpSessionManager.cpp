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
#include "CHttpSessionManager.h"

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "Test_CHttpSessionManagerTest"

namespace ic_network
{

/**
 * Class CHttpSessionManagerTest defines a test methods for CHttpSessionManager
 */
class CHttpSessionManagerTest : public ::testing::Test
{
protected:
    /**
     * Constructor
     */
    CHttpSessionManagerTest()
    {
        // do nothing
    }

    /**
     * Destructor
     */
    ~CHttpSessionManagerTest() override
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

TEST_F(CHttpSessionManagerTest, Test_CHttpSessionManager_SetProxy)
{
    CHttpSessionManager::GetInstance()->SetProxy("dummyHost", 1999, "user1", "pwd1");

    std::string strHostName = "", strUser = "", strPwd = "";
    unsigned int unPort;

    CHttpSessionManager::GetInstance()->GetProxyValues(strHostName, unPort, strUser, strPwd);
    EXPECT_EQ(strHostName, "dummyHost");
    EXPECT_EQ(unPort, 1999);
    EXPECT_EQ(strUser, "user1");
    EXPECT_EQ(strPwd, "pwd1");
}

TEST_F(CHttpSessionManagerTest, Test_CHttpSessionManager_SetLocalPortRange)
{
    CHttpSessionManager::GetInstance()->SetLocalPortRange(90, 190);

    unsigned int unPortStart= 0,unPortEnd = 0;

    CHttpSessionManager::GetInstance()->GetLocalPortRange(unPortStart, unPortEnd);
    EXPECT_EQ(unPortStart, 90);
    EXPECT_EQ(unPortEnd, 190);
}


}
