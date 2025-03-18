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
#include "dam/CActivityDelay.h"
#include "CIgniteConfig.h"
#include "CIgniteDateTime.h"
#include "CIgniteLog.h"

//! Macro for "test_CIgniteFileUtils" string
#define PREFIX "test_CIgniteFileUtils"

namespace ic_bl 
{
//! Creating a CActivityDelay pointer
CActivityDelay *g_pActivityDelayObj = nullptr;

/**
 * Class CActivityDelayTest defines a test feature for CActivityDelay class
 */
class CActivityDelayTest : public ::testing::Test
{
public:
    /**
     * Constructor
     */
    CActivityDelayTest()
    {
        // Do nothing
    }

    /**
     * Destructor
     */
    ~CActivityDelayTest() override
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
        /* Expect true in case of successful memory allocation, 
         * will fail in case of bad allocation
         */
        ASSERT_TRUE(InitializeObject());
    }

    /**
     * TearDown method : Code here will be called immediately after
     * each test (right before the destructor)
     * @see testing::Test::TearDown()
     */
    void TearDown() override
    {
       if(g_pActivityDelayObj)
        {
            delete g_pActivityDelayObj;
        }
    }

    /**
     * Wrapper method to access the ComputeDeferUpload of CActivityDelay
     * @param void
     * @return Defer time
     */
    int ComputeDeferUpload();

    /**
     * Wrapper method to access the HandleEvent of CActivityDelay
     * @param[in] pEvent Event data
     * @return void
     */
    void HandleEvent(ic_core::CEventWrapper *pEvent);

protected:
    /**
     * Overriding Method of testing::Test class
     * @see testing::Test::TestBody()
     */
    void TestBody() override
    {
        // Do nothing
    }

    /**
    * This method will initialize the object.
    * @param void
    * @return true, in case of successful memory allocation
    *         false, in case of bad allocation.
    */
    bool InitializeObject()
    {
        try
        {
            /* CActivityDelay class is the part of event-processing-chain
            * Hence it expects the object reference of next handler to pass the
            * processed event. Since we are UTing only this class (and it's 
            * straight forward methods). We don't need to provide the next handler
            * reference but simply sending NULL.
            */
            g_pActivityDelayObj = new CActivityDelay(nullptr);
        }
        catch(std::bad_alloc& e)
        {
           HCPLOG_E << "bad_alloc caught: " << e.what();
           return false;
        }
        return true;
    }
};

int CActivityDelayTest::ComputeDeferUpload()
{
    return g_pActivityDelayObj->ComputeDeferUpload();
}

void CActivityDelayTest::HandleEvent(ic_core::CEventWrapper *pEvent)
{
    g_pActivityDelayObj->HandleEvent(pEvent);
}

// Tests

TEST_F(CActivityDelayTest, Test_ActivityDelayTest_Class_Creation) 
{
    CActivityDelayTest *pActivationDelay = nullptr;
    pActivationDelay = new CActivityDelayTest();
    
    //checking the existance of CActivityDelayTest class
    EXPECT_NE(nullptr,pActivationDelay);

    delete pActivationDelay;
    pActivationDelay = nullptr;
}

TEST_F(CActivityDelayTest, Test_ActivityDelayTest_Class_Creation_MultipleObject) 
{
    CActivityDelayTest *pActivationDelay1 = nullptr;
    pActivationDelay1 = new CActivityDelayTest();

    CActivityDelayTest *pActivationDelay2 = nullptr;
    pActivationDelay2 = new CActivityDelayTest();

    //Expecting the both object to be equal
    EXPECT_EQ(pActivationDelay1, pActivationDelay1);

    delete pActivationDelay1;
    pActivationDelay1 = nullptr;
    
    delete pActivationDelay2;
    pActivationDelay2 = nullptr;
}

TEST_F(CActivityDelayTest, 
                Test_ComputeDeferUpload_UploadEventConfigValue_WithFirstField)
{
    CActivityDelayTest obj;

    //To store the event ID read from config
    std::string strEventId;

    //To store the time read from the config 
    int nDefUpldtimeout; 

    //Read the defer upload time from config
    ic_utils::Json::Value jsonUploadEventConfigValue =
        ic_core::CIgniteConfig::GetInstance()->GetJsonValue
                                                    ("DAM.UploadEventConfig");

    if(jsonUploadEventConfigValue.isNull())
    {
       nDefUpldtimeout = 0;
    }
    else
    {
        //Reading index[0] value of UploadEventConfigValue payload
        ic_utils::Json::Value jsonConfigVal = jsonUploadEventConfigValue[0];
        strEventId = jsonConfigVal["eventID"].asString();
        nDefUpldtimeout = jsonConfigVal["timeoutSec"].asInt();
    }

    //Using random CActivityDelay event payload
    ic_utils::Json::Value jsonPayload;
    jsonPayload["bearing"] = 16611;
    jsonPayload["distance"] = 16621134;

    //Reading current time
    long long llCurrentTime = ic_utils::CIgniteDateTime::GetMonotonicTimeMs();

    ic_utils::Json::Value jsonConfigValue;
    jsonConfigValue["EventID"] = strEventId;
    jsonConfigValue["Version"] = "1.0";
    jsonConfigValue["Timestamp"] = llCurrentTime;
    jsonConfigValue["Data"] = jsonPayload;

    ic_utils::Json::FastWriter jsonWriter;
    std::string strEventString = jsonWriter.write(jsonConfigValue);

    //Converting the json string to event based format
    ic_core::CEventWrapper event;
    event.JsonToEvent(strEventString);
    obj.HandleEvent(&event);
    
    if(nDefUpldtimeout > 0)
    {
        /* Expecting the api to return the value greater than zero as 
         * event is configured
         */
        EXPECT_GT(obj.ComputeDeferUpload(),0);
    }
    else
    {
        /* Expecting the api to return the value equal to zero as event 
         * is not configured
         */
        EXPECT_EQ(0, obj.ComputeDeferUpload());
    }
    
}

TEST_F(CActivityDelayTest, 
                Test_ComputeDeferUpload_UploadEventConfigValue_WithSecondField)
 {
    CActivityDelayTest obj;

    //To store the event ID read from config
    std::string strEventId; 

    //To store the time read from the config
    int nDefUpldtimeout; 

    //Read the defer upload time from config
    ic_utils::Json::Value jsonUploadEventConfigValue =
        ic_core::CIgniteConfig::GetInstance()->GetJsonValue
                                                    ("DAM.UploadEventConfig");

    if(jsonUploadEventConfigValue.isNull())
    {
        nDefUpldtimeout = 0;
    }
    else
    {
        if(jsonUploadEventConfigValue.size() >= 1)
        {   
            //Reading index[1] value of UploadEventConfigValue payload
            ic_utils::Json::Value jsonConfigVal = jsonUploadEventConfigValue[1];
            strEventId = jsonConfigVal["eventID"].asString();
            nDefUpldtimeout = jsonConfigVal["timeoutSec"].asInt();
        }
        else
        {
            nDefUpldtimeout = 0;
        }
    }

    //Using random ActivityDelay event payload
    ic_utils::Json::Value jsonPayload;
    jsonPayload["bearing"] = 16611;
    jsonPayload["distance"] = 16621134;

    //Reading current time
    long long llCurrentTime = ic_utils::CIgniteDateTime::GetMonotonicTimeMs();

    ic_utils::Json::Value jsonConfigValue;
    jsonConfigValue["EventID"] = strEventId;
    jsonConfigValue["Version"] = "1.0";
    jsonConfigValue["Timestamp"] = llCurrentTime;
    jsonConfigValue["Data"] = jsonPayload;

    ic_utils::Json::FastWriter jsonWriter;
    std::string strEventString = jsonWriter.write(jsonConfigValue);

    //Converting the json string to event based format
    ic_core::CEventWrapper event;
    event.JsonToEvent(strEventString);
    obj.HandleEvent(&event);
    
    if(nDefUpldtimeout > 0)
    {
        /* Expecting the api to return the value greater than zero as 
         * event is configured
         */
        EXPECT_GT(obj.ComputeDeferUpload(),0);
    }
    else
    {
        /* Expecting the api to return the value equal to zero as event 
         * is not configured
         */
        EXPECT_EQ(0, obj.ComputeDeferUpload());
    }
 }
} /* namespace ic_bl*/