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
#include "db/CGranularityReductionHandler.h"

namespace ic_bl
{

/**
 * Class CGranularityReductionHandlerTest defines a test feature 
 * for CGranularityReductionHandler class
 */
class CGranularityReductionHandlerTest : public ::testing::Test
{
public:
    /**
     * Constructor
     */
    CGranularityReductionHandlerTest() 
    {
        //Do nothing
    }

    /**
     * Destructor
     */
    ~CGranularityReductionHandlerTest() override 
    {
        //Do nothing
    }

    /**
     * Wrapper method for CGranularityReductionHandler::GetEventRowCount()
     * @see CGranularityReductionHandler::GetEventRowCount()
     */
    int GetEventRowCount()
    {
        return m_grObj.GetEventRowCount();
    }

    /**
     * Wrapper method for CGranularityReductionHandler::ReadDefaultGRPolicy()
     * @see CGranularityReductionHandler::ReadDefaultGRPolicy()
     */
    bool ReadDefaultGRPolicy(const ic_utils::Json::Value &rjsonGR)
    {
        return m_grObj.ReadDefaultGRPolicy(rjsonGR);
    }

    /**
     * Wrapper method for CGranularityReductionHandler::ReadGRPolicies()
     * @see CGranularityReductionHandler::ReadGRPolicies()
     */
    bool ReadGRPolicies(const ic_utils::Json::Value &rjsonGR)
    {
        return m_grObj.ReadGRPolicies(rjsonGR);
    }

    /**
     * Wrapper method for CGranularityReductionHandler::
     *                                          ReadGRFreeStorageGainPercent()
     * @see CGranularityReductionHandler::ReadGRFreeStorageGainPercent()
     */
    int ReadGRFreeStorageGainPercent(const ic_utils::Json::Value &rjsonGR)
    {
        return m_grObj.ReadGRFreeStorageGainPercent(rjsonGR);
    }

    /**
     * Wrapper method for CGranularityReductionHandler::ReadGRExemptedEvents()
     * @see CGranularityReductionHandler::ReadGRExemptedEvents()
     */
    bool ReadGRExemptedEvents(const ic_utils::Json::Value &rjsonGR)
    {
        return m_grObj.ReadGRExemptedEvents(rjsonGR);
    }

    /**
     * Wrapper method for CGranularityReductionHandler::
     *                                      ReadGRPreTriggerEventIdSuffix()
     * @see CGranularityReductionHandler::ReadGRPreTriggerEventIdSuffix()
     */
    std::string ReadGRPreTriggerEventIdSuffix(const ic_utils::Json::Value
                                              &rjsonGR)
    {
        return m_grObj.ReadGRPreTriggerEventIdSuffix(rjsonGR);
    }

    /**
     * Wrapper method for CGranularityReductionHandler::
     *                                      ReadGRPostTriggerEventIdSuffix()
     * @see CGranularityReductionHandler::ReadGRPostTriggerEventIdSuffix()
     */
    std::string ReadGRPostTriggerEventIdSuffix(const ic_utils::Json::Value
                                               &rjsonGR)
    {
        return m_grObj.ReadGRPostTriggerEventIdSuffix(rjsonGR);
    }

    /**
     * Wrapper method for CGranularityReductionHandler::
     *                                          LoadMandatoryExemptedEvents()
     * @see CGranularityReductionHandler::LoadMandatoryExemptedEvents()
     */
    void LoadMandatoryExemptedEvents()
    {
        m_grObj.LoadMandatoryExemptedEvents();
    }

    /**
     * Wrapper method for CGranularityReductionHandler::
     *                                          ReadEventTypeToDeleteDuringGR()
     * @see CGranularityReductionHandler::ReadEventTypeToDeleteDuringGR()
     */
    bool ReadEventTypeToDeleteDuringGR(const ic_utils::Json::Value &rjsonGR)
    {
        return m_grObj.ReadEventTypeToDeleteDuringGR(rjsonGR);
    }

    /**
     * Wrapper method to access the private variable
     *     CGranularityReductionHandler::m_bIsPolicyReadFromConfig
     * @see CGranularityReductionHandler::m_bIsPolicyReadFromConfig
     */
    bool IsPolicyReadFromConfig()
    {
        return m_grObj.IsPolicyReadFromConfig();
    }

    /**
     * Wrapper method to access the private variable
     *     CGranularityReductionHandler::m_stLastGrRecord
     * @see CGranularityReductionHandler::m_stLastGrRecord
     */
    CGranularityReductionHandler::LastGRPerformedRecord GetLastGRRecord()
    {
        return m_grObj.GetLastGRRecord();
    }

    /**
     * Wrapper method to access the private variable
     *     CGranularityReductionHandler::m_strPreTrgrEventIdSuffix
     * @see CGranularityReductionHandler::m_strPreTrgrEventIdSuffix
     */
    std::string GetPreTrgrEventIdSuffix()
    {
        return m_grObj.GetPreTrgrEventIdSuffix();
    }

    /**
     * Wrapper method to access the private variable
     *     CGranularityReductionHandler::m_strPostTrgrEventIdSuffix
     * @see CGranularityReductionHandler::m_strPostTrgrEventIdSuffix
     */
    std::string GetPostTrgrEventIdSuffix()
    {
       return m_grObj.GetPostTrgrEventIdSuffix();
    }

    /**
     * Wrapper method to access the private variable
     *     CGranularityReductionHandler::m_nFreeStorageGainPercent
     * @see CGranularityReductionHandler::m_nFreeStorageGainPercent
     */
    int GetFreeStorageGainPercent()
    {
        return m_grObj.GetFreeStorageGainPercent();
    }

    /**
     * Wrapper method to access the private variable
     *     CGranularityReductionHandler::m_strMandatoryExemptedEvents
     * @see CGranularityReductionHandler::m_strMandatoryExemptedEvents
     */
    std::string GetMandatoryExemptedEvents()
    {
        return m_grObj.GetMandatoryExemptedEvents();
    }

    /**
     * Wrapper method to access the private variable
     *     CGranularityReductionHandler::m_eGReductionEventType
     * @see CGranularityReductionHandler::m_eGReductionEventType
     */
    CUploadUtils::UploadDataMode GetGReductionEventType()
    {
        return m_grObj.GetGReductionEventType();
    }

    /**
     * Wrapper method to access DEF_FREE_STORAGE_GAIN_PERCENT
     * @see DEF_FREE_STORAGE_GAIN_PERCENT in CGranularityReductionHandler.cpp
     */
    int GetDefaultFreeStorageGainPercent()
    {
        return m_grObj.GetDefaultFreeStorageGainPercent();
    }

    /**
     * Wrapper method to access DEF_POST_TRIGGER_EVENT_ID_SUFFIX
     * @see DEF_POST_TRIGGER_EVENT_ID_SUFFIX in CGranularityReductionHandler.cpp
     */
    std::string GetDefaultPostTriggerEventIDSuffix()
    {
        return m_grObj.GetDefaultPostTriggerEventIDSuffix();
    }

    /**
     * Wrapper method to access DEF_PRE_TRIGGER_EVENT_ID_SUFFIX
     * @see DEF_PRE_TRIGGER_EVENT_ID_SUFFIX in CGranularityReductionHandler.cpp
     */
    std::string GetDefaultPreTriggerEventIDSuffix()
    {
        return m_grObj.GetDefaultPreTriggerEventIDSuffix();
    }

protected:
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

    /**
     * Overriding Method of testing::Test class
     * @see testing::Test::TestBody()
     */
    void TestBody() override 
    {
        //Do nothing
    }

private:
     CGranularityReductionHandler m_grObj;

};

//Tests
TEST_F(CGranularityReductionHandlerTest, 
                                    Test_CGranularityReductionHandler_zeroTest)
{
    ic_bl::CGranularityReductionHandlerTest grObj;

    //as the policy is not yet read, expect false as default value
    EXPECT_EQ(false, grObj.IsPolicyReadFromConfig());

    //read the last GR performed record
    CGranularityReductionHandler::LastGRPerformedRecord lastGRRecord =
                                                       grObj.GetLastGRRecord();
    //as the policy is not yet read, expect zeros as default value
    EXPECT_EQ(0, lastGRRecord.llTimeStamp);
    EXPECT_EQ(0, lastGRRecord.llLastGRPerRecId);

    //expect empty string as no pre-trigger eventID suffix is read yet
    EXPECT_EQ("", grObj.GetPreTrgrEventIdSuffix());

    //expect empty string as no pre-trigger eventID suffix is read yet
    EXPECT_EQ("", grObj.GetPostTrgrEventIdSuffix());

    //expect DEF_FREE_STORAGE_GAIN_PERCENT as default free storage gain %
    int freeGainPercent = grObj.GetDefaultFreeStorageGainPercent();
    EXPECT_EQ(freeGainPercent, grObj.GetFreeStorageGainPercent());

    //expect empty string as no mandatory exempted events are read yet
    EXPECT_EQ("",grObj.GetMandatoryExemptedEvents());

    //expect CUploadUtils::UploadDataMode::eBATCH as default event type
    EXPECT_EQ(CUploadUtils::UploadDataMode::eBATCH,
                                  grObj.GetGReductionEventType());
}

TEST_F(CGranularityReductionHandlerTest, Test_GetEventRowCount)
{
    ic_bl::CGranularityReductionHandlerTest grObj;

    /* Expect the API to return greater than equal to 0, assuming the db has
     * at-least one record or empty at the time of this API invocation
     */
    EXPECT_GE(grObj.GetEventRowCount(), 0);
}

TEST_F(CGranularityReductionHandlerTest, 
                                    Test_ReadGRPolicies_after_loading_GR_config)
{
    ic_bl::CGranularityReductionHandlerTest grObj;

    ic_utils::Json::Value jsonGR = ic_core::CIgniteConfig::GetInstance()->
                              GetJsonValue("DAM.Database.granularityReduction");

    //expect the API to return true as the policy is supposed to be
    //   read from the given configuration
    EXPECT_EQ(true, grObj.ReadGRPolicies(jsonGR));
}

TEST_F(CGranularityReductionHandlerTest, 
                               Test_ReadDefaultGRPolicy_after_loading_GR_config)
{
    ic_bl::CGranularityReductionHandlerTest grObj;

    ic_utils::Json::Value jsonGR = ic_core::CIgniteConfig::GetInstance()->
                              GetJsonValue("DAM.Database.granularityReduction");

    //expect the API to return greater than 0 assuming the db has
    // at-least one record at the time of this API invocation
    EXPECT_EQ(true, grObj.ReadDefaultGRPolicy(jsonGR));
}

TEST_F(CGranularityReductionHandlerTest, 
                       Test_LoadMandatoryExemptedEvents_after_loading_GR_config)
{
    ic_bl::CGranularityReductionHandlerTest grObj;

    ic_utils::Json::Value jsonGR = ic_core::CIgniteConfig::GetInstance()->
                              GetJsonValue("DAM.Database.granularityReduction");

    //load mandatory exempted events
    grObj.LoadMandatoryExemptedEvents();

    //expect the API to return non-empty string as the configuration used for UT
    // has some events configured as STREAM type events that are mandatorily to be
    // exempted
    EXPECT_NE("", grObj.GetMandatoryExemptedEvents());
}

TEST_F(CGranularityReductionHandlerTest, 
                     Test_ReadEventTypeToDeleteDuringGR_after_loading_GR_config)
{
    ic_bl::CGranularityReductionHandlerTest grObj;

    ic_utils::Json::Value jsonGR = ic_core::CIgniteConfig::GetInstance()->
                              GetJsonValue("DAM.Database.granularityReduction");

    //expect true as the API should successfully read the event type
    EXPECT_EQ(true, grObj.ReadEventTypeToDeleteDuringGR(jsonGR));

    //expect CUploadUtils::UploadDataMode::eBATCH_AND_STREAM as default event type
    //  configured in the UT config file
    EXPECT_EQ(CUploadUtils::UploadDataMode::eBATCH_AND_STREAM,
                              grObj.GetGReductionEventType());
}

TEST_F(CGranularityReductionHandlerTest, 
                      Test_ReadGRFreeStorageGainPercent_after_loading_GR_config)
{
    ic_bl::CGranularityReductionHandlerTest grObj;

    ic_utils::Json::Value jsonGR = ic_core::CIgniteConfig::GetInstance()->
                              GetJsonValue("DAM.Database.granularityReduction");

    //expect the value 15 which is configured in the UT config file
    EXPECT_EQ(15, grObj.ReadGRFreeStorageGainPercent(jsonGR));
}

TEST_F(CGranularityReductionHandlerTest, 
                              Test_ReadGRExemptedEvents_after_loading_GR_config)
{
    ic_bl::CGranularityReductionHandlerTest grObj;

    ic_utils::Json::Value jsonGR = ic_core::CIgniteConfig::GetInstance()->
                              GetJsonValue("DAM.Database.granularityReduction");

    //expect true as GR exempted events are already configured in UT config file
    EXPECT_EQ(true, grObj.ReadGRExemptedEvents(jsonGR));
}

TEST_F(CGranularityReductionHandlerTest, 
                     Test_ReadGRPreTriggerEventIdSuffix_after_loading_GR_config)
{
    ic_bl::CGranularityReductionHandlerTest grObj;

    ic_utils::Json::Value jsonGR = ic_core::CIgniteConfig::GetInstance()->
                              GetJsonValue("DAM.Database.granularityReduction");

    //expect the default suffix to be returned as it is not explicitly
    //  configured in the UT config file
    EXPECT_EQ(grObj.GetDefaultPreTriggerEventIDSuffix(),
                    grObj.ReadGRPreTriggerEventIdSuffix(jsonGR));
}

TEST_F(CGranularityReductionHandlerTest, 
                    Test_ReadGRPostTriggerEventIdSuffix_after_loading_GR_config)
{
    ic_bl::CGranularityReductionHandlerTest grObj;

    ic_utils::Json::Value jsonGR = ic_core::CIgniteConfig::GetInstance()->
                              GetJsonValue("DAM.Database.granularityReduction");

    //expect the default suffix to be returned as it is not explicitly
    //  configured in the UT config file
    EXPECT_EQ(grObj.GetDefaultPostTriggerEventIDSuffix(),
                    grObj.ReadGRPostTriggerEventIdSuffix(jsonGR));
}

}//namespace ic_bl
