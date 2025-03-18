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
#include "auth/CActivationState.h"

namespace ic_bl 
{
//! Creating a CActivationState pointer
CActivationState *g_pActivationState = NULL;

/**
 * Class CActivationStateTest defines a test feature for CActivationState class
 */
class CActivationStateTest : public ::testing::Test 
{
public:
    /**
     * Constructor
     */
    CActivationStateTest()
    {
        // Do nothing
    }
 
    /**
     * Destructor
     */
    ~CActivationStateTest() override
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
        g_pActivationState = CActivationState::GetInstance();
    }

    /**
     * TearDown method : Code here will be called immediately after
     * each test (right before the destructor)
     * @see testing::Test::TearDown()
     */
    void TearDown() override 
    {
        g_pActivationState = NULL;
    }

    /**
     * Wrapper method to access the GetTokenValue of CActivationState
     * @param void
     * @return Token string
     */
    std::string GetTokenValue();

    /**
     * Wrapper method to access the SetToken of CActivationState
     * @param[in] strToken Token
     * @return void
     */
    void SetToken(std::string strToken);

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

std::string CActivationStateTest::GetTokenValue()
{
   return g_pActivationState->GetTokenValue();
}

void CActivationStateTest::SetToken(std::string strToken)
{
   return g_pActivationState->SetToken(strToken);
}

//Tests
TEST_F(CActivationStateTest, Test_ActivationState_ClassCreation)
{
    /* Check the existance of CActivationState class as Zero test case as 
     * per TDD approach
     */
    EXPECT_NE(g_pActivationState, nullptr);
}

TEST_F(CActivationStateTest, Test_getInstance_SingleInstanceValidation)
{
    /* Check the existance of CActivationState class as Zero test case as 
     * per TDD approach
     */
    EXPECT_EQ(g_pActivationState, CActivationState::GetInstance());
}

TEST_F(CActivationStateTest, Test_getInstance_ManyCaseValidation)
{
    CActivationState *pActivationState1 = CActivationState::GetInstance();
    CActivationState *pActivationState2 = CActivationState::GetInstance();
    CActivationState *pActivationState3 = CActivationState::GetInstance();

    //Expecting the same instance to be returned by the API every time
    EXPECT_EQ(pActivationState1, g_pActivationState);
    EXPECT_EQ(pActivationState2, g_pActivationState);
    EXPECT_EQ(pActivationState3, g_pActivationState);
}

TEST_F(CActivationStateTest, Test_getTokenValue_withTokenValueSet)
{
    CActivationStateTest obj;

    // Setting random token value
    std::string strToken = "eyJ4NXQiOiJaR0poWldaa05EZGhNREV3WXpjek5tSXhNRE5qW";
    obj.SetToken(strToken);

    //expecting string equal as token value set and token value returned are equal
    EXPECT_STREQ(strToken.c_str(), obj.GetTokenValue().c_str());
}

TEST_F(CActivationStateTest, Test_getTokenValue_withNoTokenValueSet)
{
    CActivationStateTest obj;
     
    // Setting random token value
    std::string strToken = "Zek0xWXpabFpEWmxaRGs1TW1RNU5qRXpOdyIsImtpZCI6IlpH";

    /*
     * Expecting string to not be equal as token value mentioned above has 
     * not been set
     */
    EXPECT_STRNE(strToken.c_str(), obj.GetTokenValue().c_str());
}

}