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
#include "CProductImplController.h"
#include "CDefaultProductImpl.h"
#include "CIgniteClient.h"
#include "gtest/gtest.h"

namespace ic_app
{

// The fixture for testing class Foo.
class CProductImplControllerTest : public ::testing::Test {
protected:
// You can remove any or all of the following functions if their bodies would
// be empty.

CProductImplControllerTest() {
    // You can do set-up work for each test here.
}

~CProductImplControllerTest() override {
    // You can do clean-up work that doesn't throw exceptions here.
}

// If the constructor and destructor are not enough for setting up
// and cleaning up each test, you can define the following methods:

void SetUp() override {
    // Code here will be called immediately after the constructor (right
    // before each test).
    ic_device::CDefaultProductImpl::StartupInit();
}

void TearDown() override {
    // Code here will be called immediately after each test (right
    // before the destructor).
}

// Class members declared here can be used by all tests in the test suite

};

TEST_F(CProductImplControllerTest, Test_GetClientConnector)
{
    //under UT environment, the ClientConnector is not set, so
    //  expecting the API to return a NULL value
    EXPECT_EQ(NULL, ic_app::CProductImplController::GetInstance()->GetClientConnector());
}

TEST_F(CProductImplControllerTest, Test_handle_config_file_option_using_HandleConfilefileArg)
{
    //using below sample format to invoke deviceclient app with all command line arguments
    // "deviceclient -c config.conf";

    char *pchArgV[3];

    //populate inidivual values in the array
    //to avoid compiler warning, use exlicit char* conversion
    pchArgV[0] = (char*)"deviceclient";
    pchArgV[1] = (char*)"-c";

    //the API will check for the existing of the given config file.
    //   Though 'deviceclient' is not  a config file, use it just for testing
    //   as it is available in the current directory
    pchArgV[2] = (char*)"./deviceclient";

    //total argument count
    int nArgCnt = 3;

    //index of config option -c is 1
    int nConfigSwitchIndex = 1;

    //test valid config file option. expect true response
    EXPECT_EQ(true, ic_app::CProductImplController::GetInstance()->HandleConfilefileArg(nConfigSwitchIndex,nArgCnt,pchArgV));

    //test invalid config file. expect false response
    pchArgV[2] = (char*)"./ThisConfigNotExist.conf";
    EXPECT_EQ(false, ic_app::CProductImplController::GetInstance()->HandleConfilefileArg(nConfigSwitchIndex,nArgCnt,pchArgV));

    //test invalid config option index. 8 is max index. expect false response
    nConfigSwitchIndex = 10;
    EXPECT_EQ(false, ic_app::CProductImplController::GetInstance()->HandleConfilefileArg(nConfigSwitchIndex,nArgCnt,pchArgV));
}

TEST_F(CProductImplControllerTest, Test_handle_debug_option_using_HandleDebugLevelArg)
{
    //using below sample format to invoke deviceclient app with all command line arguments
    // "deviceclient -c config.conf -d 5 -re \"test reason\" 0";

    char *pchArgV[5];

    //populate inidivual values in the array
    //to avoid compiler warning, use exlicit char* conversion
    pchArgV[0] = (char*)"deviceclient";
    pchArgV[1] = (char*)"-c";

    //the API will check for the existing of the given config file.
    //   Though 'deviceclient' is not  a config file, use it just for testing
    //   as it is available in the current directory
    pchArgV[2] = (char*)"./deviceclient";

    pchArgV[3] = (char*)"-d";
    pchArgV[4] = (char*)"5";

    //total argument count
    int nArgCnt = 5;

    //index of config option -c is 1
    int nDebugSwitchIndex = 3;

    //test valid debug level option. expect true response
    EXPECT_EQ(true, ic_app::CProductImplController::GetInstance()->HandleDebugLevelArg(nDebugSwitchIndex,nArgCnt,pchArgV));

    //test missing debug level value. expect false response
    char *pchArgV2[4];

    pchArgV2[0] = (char*)"deviceclient";
    pchArgV2[1] = (char*)"-c";
    pchArgV2[2] = (char*)"./deviceclient";
    pchArgV2[3] = (char*)"-d";

    //total argument count
    int nArgCnt2 = 4;

    //index of config option -c is 1
    int nDebugSwitchIndex2 = 3;

    //as debug level is missing, expect false response
    EXPECT_EQ(false, ic_app::CProductImplController::GetInstance()->HandleDebugLevelArg(nDebugSwitchIndex2,nArgCnt2,pchArgV2));
}

TEST_F(CProductImplControllerTest, Test_handle_restart_count_option_using_HandleRestartCountArg)
{
    //using below sample format to invoke deviceclient app with all command line arguments
    // "deviceclient -c config.conf -d 5 -re \"test reason\" 0";

    char *pchArgV[6];

    //populate inidivual values in the array
    //to avoid compiler warning, use exlicit char* conversion
    pchArgV[0] = (char*)"deviceclient";
    pchArgV[1] = (char*)"-c";

    //the API will check for the existing of the given config file.
    //   Though 'deviceclient' is not  a config file, use it just for testing
    //   as it is available in the current directory
    pchArgV[2] = (char*)"./deviceclient";

    pchArgV[3] = (char*)"-re";
    pchArgV[4] = (char*)"\"test reason\"";
    pchArgV[5] = (char*)"0";

    //total argument count
    int nArgCnt = 6;

    //index of config option -c is 1
    int nRESwitchIndex = 3;

    //as both restart reason and count are available, expect true response
    EXPECT_EQ(true, ic_app::CProductImplController::GetInstance()->HandleRestartCountArg(nRESwitchIndex,nArgCnt,pchArgV));

    //test invalid -re options
    char *pchArgV2[4];

    pchArgV2[0] = (char*)"deviceclient";
    pchArgV2[1] = (char*)"-c";
    pchArgV2[2] = (char*)"./deviceclient";
    pchArgV2[3] = (char*)"-re";

    //total argument count
    int nArgCnt2 = 4;

    //index of config option -c is 1
    int nDebugSwitchIndex2 = 3;

    //as reason and count values are missing, expect false response
    EXPECT_EQ(false, ic_app::CProductImplController::GetInstance()->HandleRestartCountArg(nDebugSwitchIndex2,nArgCnt2,pchArgV2));
}


} //namespace ic_app
