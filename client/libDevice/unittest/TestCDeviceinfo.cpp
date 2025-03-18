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
#include "CDeviceInfo.h"

namespace ic_device
{
// Assigning sample data to snap shot reason string
static const std::string SNAP_SHOT_REASON = "Testing";

/**
 * Class CDeviceinfoTest defines a test feature for CDeviceinfo class
 */
class CDeviceinfoTest : public ::testing::Test
{
protected:
    /**
     * Constructor
     */
    CDeviceinfoTest()
    {
        // do nothing
    }

    /**
     * Destructor
     */
    ~CDeviceinfoTest() override
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

/**
 * Zero test case is not required as CDeviceinfo contains static methods only
 *
 */

TEST_F(CDeviceinfoTest, Test_Init_WithAllGetFunctions)
{
    //use some random device attributes
    CDeviceInfo::Init("TestSerial","TestVin","TestSWVer","TestHWVer",
                      "TestIMEI","TestReason",0);

    EXPECT_STREQ("TestSerial",CDeviceInfo::GetSerialNumber().c_str());
    EXPECT_STREQ("TestVin",CDeviceInfo::GetVIN().c_str());
    EXPECT_STREQ("TestSWVer",CDeviceInfo::GetSWVersion().c_str());
    EXPECT_STREQ("TestHWVer",CDeviceInfo::GetHWVersion().c_str());
    EXPECT_STREQ("TestIMEI",CDeviceInfo::GetIMEI().c_str());
    EXPECT_STREQ("TestReason",CDeviceInfo::GetStartupReason().c_str());
    EXPECT_EQ(0,CDeviceInfo::GetRestartCount());
}

TEST_F(CDeviceinfoTest, Test_SetSerialNumber_Using_GetSerialNumber)
{
    //use a random device serial
    CDeviceInfo::SetSerialNumber("SerialTest1");

    EXPECT_STREQ("SerialTest1",CDeviceInfo::GetSerialNumber().c_str());
}

TEST_F(CDeviceinfoTest, Test_SetVIN_Using_GetVIN)
{
    //use a random VIN
    CDeviceInfo::SetVIN("VINTest1");

    EXPECT_STREQ("VINTest1",CDeviceInfo::GetVIN().c_str());
}

TEST_F(CDeviceinfoTest, Test_SetIMEI_Using_GetIMEI)
{
    //use a random IMEI
    CDeviceInfo::SetIMEI("IMEITest1");

    EXPECT_STREQ("IMEITest1",CDeviceInfo::GetIMEI().c_str());
}

TEST_F(CDeviceinfoTest, Test_SetSWVersion_Using_GetSWVersion)
{
    //use a random SW version
    CDeviceInfo::SetSWVersion("SWVerTest1");

    EXPECT_STREQ("SWVerTest1",CDeviceInfo::GetSWVersion().c_str());
}

TEST_F(CDeviceinfoTest, Test_SetHWVersion_Using_GetHWVersion)
{
    //use a random HW version
    CDeviceInfo::SetHWVersion("HWVerTest1");

    EXPECT_STREQ("HWVerTest1",CDeviceInfo::GetHWVersion().c_str());
}

TEST_F(CDeviceinfoTest, Test_SetStartupReason_Using_GetStartupReason)
{
    //use a random Startup reason
    CDeviceInfo::SetStartupReason("StartupReasonTest1");

    EXPECT_STREQ("StartupReasonTest1",CDeviceInfo::GetStartupReason().c_str());
}

TEST_F(CDeviceinfoTest, Test_SetRestartCount_Using_GetRestartCount)
{
    //use a random startup count
    CDeviceInfo::SetRestartCount(4);

    EXPECT_EQ(4,CDeviceInfo::GetRestartCount());
}

TEST_F(CDeviceinfoTest, Test_GetCpuLoad)
{
    //as current CPU load must be greater than 0 in general,
    // expecting the API to return non-zero value
    EXPECT_NE(0,CDeviceInfo::GetCpuLoad());
}

TEST_F(CDeviceinfoTest, Test_GetFreeMem)
{
    //assuming current free memory should be greater than 0 in general,
    // expecting the API to return non-zero value
    EXPECT_NE(0,CDeviceInfo::GetFreeMem());
}

TEST_F(CDeviceinfoTest, Test_CreateSystemSnapshotEvent_with_empty_reason)
{
    //as empty reason string is passed as an argument, expecting the API
    //  to return the error code -2
    EXPECT_EQ(-2, CDeviceInfo::CreateSystemSnapshotEvent(""));
}

TEST_F(CDeviceinfoTest, Test_CreateSystemSnapshotEvent_with_valid_reason)
{
    //as valid reason string is passed as an argument, expecting the API
    //  to return zero as success.
    EXPECT_EQ(0, CDeviceInfo::CreateSystemSnapshotEvent("TestSnapshot"));
}

} //namespace
