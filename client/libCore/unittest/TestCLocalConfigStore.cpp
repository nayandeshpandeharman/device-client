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
#include "db/CLocalConfig.h"

namespace ic_core
{
/**
 * Class CLocalConfigTest defines a test feature for CLocalConfig class
 */
class CLocalConfigTest : public ::testing::Test
{
protected:
    /**
     * Constructor
     */
    CLocalConfigTest()
    {
        // Do nothing
    }

    /**
     * Destructor
     */ 
    ~CLocalConfigTest() override
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

//Tests

TEST_F(CLocalConfigTest, Test_ZeroTest)
{
    // Check the existance of CLocalConfigTest class as Zero test case
    EXPECT_NE(CLocalConfig::GetInstance(), nullptr);
}

TEST_F(CLocalConfigTest, Test_Set)
{
    // Assign a random value to the variable
    std::string strKey = "presetAdvance";
    std::string strVal = "selectPreset";

    // Expect the true value to be returned by 'set' method.
    EXPECT_TRUE(CLocalConfig::GetInstance()->Set(strKey, strVal));

    // Expect the same value to be returned by 'get' method.
    EXPECT_EQ(CLocalConfig::GetInstance()->Get(strKey), strVal);
}

TEST_F(CLocalConfigTest, Test_Remove)
{
    // Assign a random value to the variable
    std::string strKey = "presetAdvance";
    std::string strVal = "selectPreset";

    // Expect the true value to be returned by 'set' method.
    EXPECT_TRUE(CLocalConfig::GetInstance()->Set(strKey, strVal));

    // Expect the same value to be returned by 'get' method.
    EXPECT_EQ(CLocalConfig::GetInstance()->Get(strKey), strVal);

    // Expect true value to be returned by 'remove' method.
    EXPECT_EQ(CLocalConfig::GetInstance()->Remove(strKey), 1);

    // Expect empty value to be returned by 'get' method.
    EXPECT_EQ(CLocalConfig::GetInstance()->Get(strKey), "");
}

TEST_F(CLocalConfigTest, Test_CountRowsStartsWithKey)
{
    // Assign a random value to the variable
    std::string strKey = "presetAdvance";

    /* Expect zero row to be returned by 'countRowsStartsWithKey' method 
     * as no value is set.
     */
    EXPECT_EQ(CLocalConfig::GetInstance()->CountRowsStartsWithKey(strKey), 0);

    // Assign a random value to the variable
    std::string strVal = "selectPreset";

    // Expect the true value to be returned by 'set' method.
    EXPECT_TRUE(CLocalConfig::GetInstance()->Set(strKey, strVal));

    /* Expect row count to be returned by 'countRowsStartsWithKey' method
     * as value is set.
     */  
    EXPECT_EQ(CLocalConfig::GetInstance()->CountRowsStartsWithKey(strKey), 1);
}

TEST_F(CLocalConfigTest, Test_RemoveRowsStartsWithKey)
{
    // Assign a random value to the variable
    std::string strKey = "presetAdvance";
    std::string strVal = "selectPreset";

    // Expect the true value to be returned by 'set' method
    EXPECT_TRUE(CLocalConfig::GetInstance()->Set(strKey,strVal));

    /* Expect row count to be returned by 'countRowsStartsWithKey' method
     * as value is set.
     */  
    EXPECT_EQ(CLocalConfig::GetInstance()->CountRowsStartsWithKey(strKey), 1);

    // Expect true value to be returned by 'removeRowsStartsWithKey' method.
    EXPECT_EQ(CLocalConfig::GetInstance()->RemoveRowsStartsWithKey(strKey), 1);

    /* Expect zero row to be returned by 'countRowsStartsWithKey' method
     * as row is removed.
     */  
    EXPECT_EQ(CLocalConfig::GetInstance()->CountRowsStartsWithKey(strKey), 0);
}

TEST_F(CLocalConfigTest, Test_GetIvRandomNumber)
{
    // Assign a random value to the variable
    std::string strSeedKey = "presetAdvance";

    // Get IV Random number for the key 'presetAdvance'
    std::string strKey = CLocalConfig::GetInstance()->
                         GetIvRandomNumber(strSeedKey);

    // Get IV Random number for the key 'presetAdvance' from the db
    std::string strSeedRndNo = CLocalConfig::GetInstance()->
                               Get(DATA_ENCRYPT_RND_NO);

    // Expect the same value for the key 'presetAdvance'
    EXPECT_EQ(strKey, strSeedRndNo);
}
}