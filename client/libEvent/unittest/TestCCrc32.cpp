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
#include "CCrc32.h"
#include "CIgniteLog.h"

// Macro for test_CCrc32 string
const std::string PREFIX = "test_CCrc32";

namespace ic_event 
{

// Class CCrc32Test defines a test feature for CCrc32 class
class CCrc32Test : public ::testing::Test
{
protected:
    /**
     * Constructor
     */
    CCrc32Test()
    {
        // Do nothing
    }
 
    /**
     * Destructor
     */
    ~CCrc32Test() override
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
        // Do nothing
    }

    /**
     * TearDown method : Code here will be called immediately after
     * each test (right before the destructor)
     * @see testing::Test::TearDown()
     */
    void TearDown() override
    {
        // Do nothing
    }
};

// Test

TEST_F(CCrc32Test, Test_calculate_with_unsignedchar_data)
{
    // Creating an object for CCrc32 class
    CCrc32 cCrc32Obj;

    // Assigning the random value to the variable
    const unsigned char uchData[] = "123";
     
    // Getting the length of the data
    int nLen = sizeof(uchData)-1;

    // Expecting the API to return '2286445522' for the value '123' using crc32 algorithm.
    EXPECT_EQ(cCrc32Obj.Calculate(uchData, nLen),2286445522);
}

TEST_F(CCrc32Test, Test_calculate_with_string_data)
{
    // Creating an object for CCrc32 class
    CCrc32 cCrc32Obj;

    // Assigning the random value to the variable
    const std::string& rstrString = "12345";

    // Expecting the API to return '3421846044' for the value '12345' using crc32 algorithm.
    EXPECT_EQ(cCrc32Obj.Calculate(rstrString),3421846044);
}

} /* namespace ic_event */