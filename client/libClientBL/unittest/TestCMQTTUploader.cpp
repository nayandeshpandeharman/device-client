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
#include "upload/CMQTTUploader.h"

namespace ic_bl
{
//! Global variable for minimum upload event count
static const int MIN_UPLOAD_EVENT_COUNT = 20;

//! Global variable for maximum upload event count
static const int MAX_UPLOAD_EVENT_COUNT = 175;

//! Const string for "MQTT.pub_topics.events.uploadEventCount"
static const std::string UPLOAD_EVENT_COUNT_JSON_PATH = 
                                     "MQTT.pub_topics.events.uploadEventCount";

//! Pointer to the object of CMQTTUploader                                    
CMQTTUploader *g_pMQTTUploader = nullptr;

/**
 * Class for unit testing CMQTTUploader
 */
class CMQTTUploaderTest : public ::testing::Test 
{
public:

    /**
     * Wrapper method to call InitEventsUploadCnt of CMQTTUploader
     * @see CMQTTUploader::InitEventsUploadCnt
     */
    void InitEventsUploadCnt();

    /**
     * Wrapper method to read m_unMaxEventUploadCnt of CMQTTUploader
     * @param void
     * @return Max event upload count
     */
    unsigned int FetchMaxUploadEventCnt();

    /**
     * Constructor
     */
    CMQTTUploaderTest() 
    {
    }

    /**
     * Destructor
     */
    ~CMQTTUploaderTest() override 
    {
    }

    /**
     * Overriding Method of testing::Test class
     * @see testing::Test::SetUp()
     */
    void SetUp() override 
    {
        g_pMQTTUploader =  CMQTTUploader::GetInstance();
    }

    /**
     * Overriding Method of testing::Test class
     * @see testing::Test::TearDown()
     */
    void TearDown() override 
    {
        g_pMQTTUploader = NULL;
    }
protected:
    /**
     * Overriding Method of testing::Test class
     * @see testing::Test::TestBody()
     */
    void TestBody() override 
    {
    }

};

void CMQTTUploaderTest::InitEventsUploadCnt()
{
    return g_pMQTTUploader->InitEventsUploadCnt();
}

unsigned int CMQTTUploaderTest::FetchMaxUploadEventCnt()
{
    return g_pMQTTUploader->m_unMaxEventUploadCnt;
}

TEST_F(CMQTTUploaderTest,
                        test_getComponentStatus_checkCurrentStateOfMqttUploader)
{
    std::string strDiagString = g_pMQTTUploader->GetComponentStatus();

    std::cout << strDiagString << std::endl;

    // string returned from the getComponentStatus should not be null or empty
    EXPECT_NE("", strDiagString);
}

// Tests

TEST_F(CMQTTUploaderTest, Test_initEventsUploadCnt)
{
    CMQTTUploaderTest obj;

    //Fetching UploadEventCount from memory
    int nConfiguredMaxEventUploadCnt = 
    ic_core::CIgniteConfig::GetInstance()->GetInt(UPLOAD_EVENT_COUNT_JSON_PATH,
                                                                            20);

    /* Checking if nConfiguredMaxEventUploadCnt < MIN_UPLOAD_EVENT_COUNT then 
     * setting to MIN_UPLOAD_EVENT_COUNT. If nConfiguredMaxEventUploadCnt > 
     * MAX_UPLOAD_EVENT_COUNT then setting to MAX_UPLOAD_EVENT_COUNT 
     */
    if(nConfiguredMaxEventUploadCnt <= MIN_UPLOAD_EVENT_COUNT)
    {
        nConfiguredMaxEventUploadCnt = MIN_UPLOAD_EVENT_COUNT;
    }
    else if(nConfiguredMaxEventUploadCnt >= MAX_UPLOAD_EVENT_COUNT)
    {
        nConfiguredMaxEventUploadCnt = MAX_UPLOAD_EVENT_COUNT;
    }
    else
    {
        //do nothing
    }

    obj.InitEventsUploadCnt();

    //Expecting mMaxUploadEventCnt equal to nConfiguredMaxEventUploadCnt
    EXPECT_EQ((unsigned int)nConfiguredMaxEventUploadCnt, 
              obj.FetchMaxUploadEventCnt());
}

} //namespace ic_bl