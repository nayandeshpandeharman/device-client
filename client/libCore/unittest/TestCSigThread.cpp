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
#include "dam/CSigThread.h"
#include "CIgniteLog.h"

//! Macro for CSigThread test class
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "test_CSigThread"

namespace ic_core
{
/**
 * Class CSigThreadTest defines a test feature for CSigThread class
 */
class CSigThreadTest : public ::testing::Test
{
protected:
    /**
     * Constructor
     */
    CSigThreadTest()
    {
        // Do nothing
    }

    /**
     * Destructor
     */ 
    ~CSigThreadTest() override
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
     * @see testing::Test::TearDown()
     */
    void TearDown() override
    {
        // Do nothing
    }
};

// Test

TEST_F(CSigThreadTest, Test_Wait_Notify)
{
    // Creating an object for CSigThread class
    CSigThread obj(SIGUSR1);

    // Wait for signal until timeout(1000)
    obj.Wait(1000);

    // Send the signal
    EXPECT_FALSE(obj.Notify(pthread_self()));
}

}
