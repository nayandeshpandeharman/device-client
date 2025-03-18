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
#include "db/CContentValues.h"
#include "CIgniteLog.h"

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "test_ContentValues"

namespace ic_core
{
/**
 * Class CContentValuesTest defines a test feature for CContentValues class
 */
class CContentValuesTest : public ::testing::Test
{
protected:
    /**
     * Constructor
     */
    CContentValuesTest()
    {
        // Do nothing
    }

    /**
     * Destructor
     */ 
    ~CContentValuesTest() override
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

// Tests

TEST_F(CContentValuesTest, Test_ZeroTest) 
{
    // Check the existance of CContentValues class
    CContentValues *pObj = new CContentValues;
    EXPECT_NE(nullptr, pObj);
    delete pObj;
}

TEST_F(CContentValuesTest, Test_Put_case1)
{
    CContentValues obj;

    // Assign the random value to the variable
    std::string strKey = "seekUp";
    int nValue = 10;

    // Set the value to the member variable with 'Put' member function
    obj.Put(strKey, nValue);

    // Expect the same value to be returned by 'getAsInt' method.
    EXPECT_EQ(obj.GetAsInt(strKey), nValue);
}

TEST_F(CContentValuesTest, Test_Put_case2)
{
    CContentValues obj;

    // Assign the random value to the variable
    std::string strKey = "presetAdvance";
    long long llValue = 1410065408;

    // Set the value to the member variable with 'Put' member function
    obj.Put(strKey, llValue);

    // Expect the same value to be returned by 'GetAsLong' method.
    EXPECT_EQ(obj.GetAsLong(strKey), llValue);
}

TEST_F(CContentValuesTest, Test_Put_case3)
{
    CContentValues obj;

    // Assign a random value to the variable
    std::string strKey = "selectPreset";
    std::string strValue = "Ignite";

    // Set the value to the member variable with 'Put' member function
    obj.Put(strKey, strValue);

    // Expect the same value to be returned by 'getAsString' method.
    EXPECT_EQ(obj.GetAsString(strKey), strValue);
}

TEST_F(CContentValuesTest, Test_Put_case4)
{
    CContentValues obj;

    // Assign a random value to the variable
    std::string strKey = "phone";
    const char* pchValue = "volumeChange";

    // Set the value to the member variable with 'Put' member function
    obj.Put(strKey, pchValue);

    // Expect the same value to be returned by 'getString' method.
    EXPECT_EQ(obj.GetAsString(strKey), pchValue);
}

TEST_F(CContentValuesTest, Test_Put_case5)
{
    CContentValues obj;

    // Assign a random value to the variable
    std::string strKey = "presetSeekUp";
    bool bValue = true;

    // Set the value to the member variable with 'Put' member function
    obj.Put(strKey, bValue);

    // Expect the same value to be returned by 'getAsBool' method.
    EXPECT_EQ(obj.GetAsBool(strKey), bValue);
}

TEST_F(CContentValuesTest, Test_Put_case6)
{
    CContentValues obj;

    // Assign a random value to the variable
    std::string strKey = "presetSeekDown";
    float fValue = 5.9;

    // Set the value to the member variable with 'Put' member function
    obj.Put(strKey, fValue);

    // Expect the same value to be returned by 'getAsFloat' method.
    EXPECT_EQ(obj.GetAsFloat(strKey), fValue);
}

TEST_F(CContentValuesTest, Test_Put_case7)
{
    CContentValues obj;

    // Assign a random value to the variable
    std::string strKey = "PTT";
    double dValue = 12.987;

    // Set the value to the member variable with 'Put' member function
    obj.Put(strKey, dValue);

    // Expect the same value to be returned by 'getAsDouble' method.
    EXPECT_EQ(obj.GetAsDouble(strKey), dValue);
}


TEST_F(CContentValuesTest, Test_Clear)
{
    CContentValues obj;

    // Assign a random value to the variable
    std::string strKey = "selectPreset";
    std::string strValue = "Ignite";

    // Set the value to the member variable with 'Put' member function
    obj.Put(strKey, strValue);

    // Expect the API to return value greater than 0 as value is set
    EXPECT_GT(obj.Size(), 0);

    // Clear the data
    obj.Clear();

    // Expect the API to return value 0 as data is reset
    EXPECT_EQ(obj.Size(), 0);
}
}
