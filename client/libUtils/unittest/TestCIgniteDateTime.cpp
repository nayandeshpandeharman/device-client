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

#include <unistd.h>
#include <string.h>
#include "gtest/gtest.h"
#include "CIgniteDateTime.h"

//! Define a test fixture for CIgniteDateTimeTest
class CIgniteDateTimeTest : public ::testing::Test 
{
protected:
   /**
    * Constructor
    */
   CIgniteDateTimeTest() 
   {
      // do nothing
   }

   /**
    * Destructor
    */ 
   ~CIgniteDateTimeTest() override 
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

//Tests

TEST_F(CIgniteDateTimeTest, Test_GetCurrentTimeMs) 
{
   long long llTime = 0;
   EXPECT_NE(llTime, ic_utils::CIgniteDateTime::GetCurrentTimeMs());
}

TEST_F(CIgniteDateTimeTest, Test_GetMonotonicTimeMs) 
{
   long long llTime = 0;
   EXPECT_NE(llTime,ic_utils::CIgniteDateTime::GetMonotonicTimeMs());
}

TEST_F(CIgniteDateTimeTest, Test_GetTimeSinceBootMs) 
{
   long long llTime = 0;
     EXPECT_NE(llTime,ic_utils::CIgniteDateTime::GetTimeSinceBootMs());
}

TEST_F(CIgniteDateTimeTest, Test_GetCurrentFormattedDateTime) 
{
   std::string strTime = "11-02-2021 05:55:55";
   EXPECT_NE(strTime,ic_utils::CIgniteDateTime::GetCurrentFormattedDateTime());
}

TEST_F(CIgniteDateTimeTest, Test_GetTimeZoneOffsetMinutes) 
{
   int nTime = -1;

   /* Expecting 'TimeZoneOffsetMinutes' value is equal to 0 or +ve or -ve number
    * , but not equal to -1.
    */
   EXPECT_NE(nTime, ic_utils::CIgniteDateTime::GetTimeZoneOffsetMinutes());
}

TEST_F(CIgniteDateTimeTest, Test_ConvertToNumber_1) 
{
   std::string strTime = "2020/10/28 03:10:13";
   unsigned long long ullTime = 1606162213000;
   EXPECT_NE(ullTime,ic_utils::CIgniteDateTime::ConvertToNumber(strTime));
}

TEST_F(CIgniteDateTimeTest, Test_ConvertToNumber_2) 
{
   std::string strTime = "2020/11/23 20:10:13";
   unsigned long long ullTime = 1606162213000;
   EXPECT_EQ(1606162213000,ic_utils::CIgniteDateTime::ConvertToNumber(strTime));
}

TEST_F(CIgniteDateTimeTest, Test_GetCurrentDay) 
{
   EXPECT_NE(32,ic_utils::CIgniteDateTime::GetCurrentDay());
}

TEST_F(CIgniteDateTimeTest, Test_GetCurrentMonth) 
{
   EXPECT_NE(13,ic_utils::CIgniteDateTime::GetCurrentMonth());
}

