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
#include "upload/CUploadController.h"

namespace ic_bl 
{
//! Global variable to store instance of CUploadController
CUploadController *g_pUploadController = NULL;

//! Define a test fixture for CUploadController
class CUploadControllerTest : public ::testing::Test 
{
public:
    /**
     * Wrapper method for GetInstance of CUploadController class
     * @see CUploadController::GetInstance()
     */
    CUploadController *GetInstance();

    /**
     * Wrapper method for IsEventToBeForceUplaoded of CUploadController class
     * @see CUploadController::IsEventToBeForceUplaoded()
     */
    bool IsEventToBeForceUplaoded(const std::string& rstrEventID);

    /**
     * Wrapper method for TriggerAlertsUpload of CUploadController class
     * @see CUploadController::TriggerAlertsUpload()
     */
    int TriggerAlertsUpload(const std::string& rstrAlert);

    /**
     * Wrapper method for SuspendStreamUpload of CUploadController class
     * @see CUploadController::SuspendStreamUpload()
     */
    bool SuspendStreamUpload();

    /**
     * Wrapper method for ReloadStreamPeriodicity of CUploadController class
     * @see CUploadController::ReloadStreamPeriodicity()
     */
    bool ReloadStreamPeriodicity(int nValue);

    /**
     * Wrapper method for ForceStreamUpload of CUploadController class
     * @see CUploadController::ForceStreamUpload()
     */
    int ForceStreamUpload(bool bExitWhenDone);

    /**
    * Constructor
    */
    CUploadControllerTest() 
    {
        // do nothing
    }

    /**
    * Destructor
    */
    ~CUploadControllerTest() override 
    {
        // do nothing
    }

    /**
     * Overriding Method of testing::Test class
     * @see testing::Test::SetUp()
     */
    void SetUp() override 
    {
        g_pUploadController =  CUploadController::GetInstance();
    }

    /**
     * Overriding Method of testing::Test class
     * @see testing::Test::TearDown()
     */
    void TearDown() override 
    {
        g_pUploadController = NULL;
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

CUploadController* CUploadControllerTest::GetInstance()
{
    return g_pUploadController->GetInstance();
}

bool CUploadControllerTest::IsEventToBeForceUplaoded
    (const std::string& rstrEventID)
{
    return g_pUploadController->IsEventToBeForceUplaoded(rstrEventID);
}

int CUploadControllerTest::TriggerAlertsUpload(const std::string& rstrAlert)
{
    return g_pUploadController->TriggerAlertsUpload(rstrAlert);
}

bool CUploadControllerTest::SuspendStreamUpload()
{
    return g_pUploadController->SuspendStreamUpload();
}

bool CUploadControllerTest::ReloadStreamPeriodicity(int nValue)
{
    return g_pUploadController->ReloadStreamPeriodicity(nValue);
}

int CUploadControllerTest::ForceStreamUpload(bool bExitWhenDone)
{
    return g_pUploadController->ForceStreamUpload(bExitWhenDone);
}

//Tests

TEST_F(CUploadControllerTest, Test_getInstance) 
{
    CUploadControllerTest obj;
    EXPECT_EQ(g_pUploadController, obj.GetInstance());
}

TEST_F(CUploadControllerTest, Test_EventToBeForceUploaded)
{
    CUploadControllerTest obj;

    //expect false as no event is configured as ForceUpload events
    std::string strEventID = "UploadControllerEvent";
    EXPECT_FALSE(obj.IsEventToBeForceUplaoded(strEventID));
}

TEST_F(CUploadControllerTest, Test_TriggerAlertsUpload)
{
    CUploadControllerTest obj;
    std::string strAlert = "{\"Data\":{\"battVolt\":9.5},\"Timezone\":"
    "0,\"EventID\":\"PreHibernate\",\"Version\":\"1.0\",\"Timestamp\":"
    "1444952510037}";
    EXPECT_EQ(0, obj.TriggerAlertsUpload(strAlert));
}

TEST_F(CUploadControllerTest, Test_triggerBlankAlertsUpload)
{
    CUploadControllerTest obj;
    std::string strAlert = "";
    EXPECT_EQ(0, obj.TriggerAlertsUpload(strAlert));
}

TEST_F(CUploadControllerTest, Test_triggerInvAlertsUpload)
{
    CUploadControllerTest obj;
    std::string strAlert = "uploadController";
    EXPECT_EQ(-1, obj.TriggerAlertsUpload(strAlert));
}

TEST_F(CUploadControllerTest, Test_TriggerAlertsUploadWrongCompare)
{
    CUploadControllerTest obj;
    std::string strAlert = "{\"Timezone\":0,\"EventID\":"
    "\"PreHibernate\",\"Version\":\"1.0\",\"Timestamp\":1444952510037}";
    EXPECT_NE(1, obj.TriggerAlertsUpload(strAlert));
}

TEST_F(CUploadControllerTest, Test_SuspendStreamUpload)
{
    CUploadControllerTest obj;
    EXPECT_TRUE(obj.SuspendStreamUpload());
}

TEST_F(CUploadControllerTest, Test_ReloadStreamPeriodicity)
{
    CUploadControllerTest obj;
    int nValue = 1;
    EXPECT_TRUE(obj.ReloadStreamPeriodicity(nValue));
}

TEST_F(CUploadControllerTest, Test_ForceStreamUpload)
{
    CUploadControllerTest obj;
    bool bExitWhenDone = false;
    EXPECT_EQ(0, obj.ForceStreamUpload(bExitWhenDone));
}

}

