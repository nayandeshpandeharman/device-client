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
#include "config/CUploadMode.h"
#include "CIgniteConfig.h"
#include "CIgniteLog.h"

namespace ic_core
{
/**
 * Class CUploadModeTest defines a test feature for CUploadMode class
 */
class CUploadModeTest : public ::testing::Test
{
protected:
    /**
     * Constructor
     */
    CUploadModeTest()
    {
        // Do nothing
    }

    /**
     * Destructor
     */ 
    ~CUploadModeTest() override
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

//Tests

TEST_F(CUploadModeTest, Test_ZeroTest)
{
    // Check the existance of Config class
    EXPECT_NE(CUploadMode::GetInstance(), nullptr);
}

TEST_F(CUploadModeTest, Test_IsEventSupportedForBatch_eventIgnStatus)
{
    // sample eventID
    std::string strEventId = "MyEvent";

    /* Expect true as given eventID is not configured as batch mode in the
     * configuration file
     */  
    EXPECT_FALSE(CUploadMode::GetInstance()->
                IsEventSupportedForBatch(strEventId));
}

TEST_F(CUploadModeTest, Test_isEventSupportedForStream_eventRPM)
{
    // Valid stream mode Event
    std::string strEventId = "RPM";

    /* Expect true as RPM is configured as stream mode event in the
     * configuration file
     */ 
    EXPECT_TRUE(CUploadMode::GetInstance()->
                IsEventSupportedForStream(strEventId));
}

TEST_F(CUploadModeTest, Test_isEventSupportedForStream_invalidEvent)
{
    // Invalid EventId
    std::string strEventId = "BatchEvent1";

    /* if Batch mode is supported, expect FALSE as the event `BatchEvent1`
     * is not configured as batch event in the configuration file
     */
    if (CUploadMode::GetInstance()->IsBatchModeSupported())
    {
        EXPECT_FALSE(CUploadMode::GetInstance()->
                IsEventSupportedForStream(strEventId));
    }
    else
    {
        /* If Batch mode is not supported, default is STREAM. So, even though
         * 'BatchEvent1' is not configured as BATCH, due to default STREAM mode,
         * expect TRUE.
         */
        EXPECT_TRUE(CUploadMode::GetInstance()->
                IsEventSupportedForStream(strEventId));
    }
}

TEST_F(CUploadModeTest, Test_isAnonymousUploadSupported)
{
    ic_utils::Json::Value uploadConfig = 
                    CIgniteConfig::GetInstance()->GetJsonValue("uploadMode");

    /* Check if uploadConfig json value is null in config file and if not null,
     * check if anonymous Upload is supported
     */  
    
    if(uploadConfig != ic_utils::Json::Value::nullRef) 
    {
        if (uploadConfig.isMember("anonymousUpload") && \
            uploadConfig["anonymousUpload"].isBool() && \
            uploadConfig["anonymousUpload"].asBool() == true)
        {
            // Expect true as anonymous mode is configured
            EXPECT_TRUE(CUploadMode::GetInstance()->
                        IsAnonymousUploadSupported());
        }
        else
        {
            // Expect false as anonymousUpload mode is not configured
            EXPECT_FALSE(CUploadMode::GetInstance()->
                         IsAnonymousUploadSupported());
        }
    }
    else
    {
        HCPLOG_D << \
        "Stream will be supported by default if 'uploadMode' is not defined";
    }
}

TEST_F(CUploadModeTest, Test_ModesSupported)
{
    ic_utils::Json::Value uploadConfig = CIgniteConfig::GetInstance()->
                                         GetJsonValue("uploadMode");

    /* Check if uploadConfig json value is null in config file and if not null, 
     * check all the modes that are supported
     */  
    if(uploadConfig != ic_utils::Json::Value::nullRef) 
    {
        ic_utils::Json::Value supported = uploadConfig["supported"];
        if(supported != ic_utils::Json::Value::nullRef)
        {
            for (int i = 0; i < supported.size(); i++) 
            {
                std::string mode = supported[i].asString();
                if(mode == "stream")
                {
                    // Expect true as stream mode is supported
                    EXPECT_TRUE(CUploadMode::GetInstance()->
                                IsStreamModeSupported());
                }
                else
                {
                    // Expect true as stream mode is supported
                    EXPECT_TRUE(CUploadMode::GetInstance()->
                                IsBatchModeSupported());
                }
            }
        }
        else
        {
            HCPLOG_D << 
            "Stream will be supported by default if 'supported' is empty";
        }
    }
    else
    {
        HCPLOG_D << 
        "Stream will be supported by default if 'uploadMode' is not defined";
    }
}

TEST_F(CUploadModeTest, Test_DefaultModeSupported)
{
    // Obtain the default mode set in the config file
    std::string strDefMode = CIgniteConfig::GetInstance()->
                             GetString("uploadMode.events.default");
    if (strDefMode == "stream")
    {
        // Expect true if default mode set is stream
        EXPECT_TRUE(CUploadMode::GetInstance()->
                    IsStreamModeSupportedAsDefault());
    }
    else
    {
        // Expect true if default mode set is batch
        EXPECT_TRUE(CUploadMode::GetInstance()->
                    IsBatchModeSupportedAsDefault());
    }
}

TEST_F(CUploadModeTest, Test_IsStoreAndForwardSupported)
{
    ic_utils::Json::Value uploadConfig = 
                    CIgniteConfig::GetInstance()->GetJsonValue("uploadMode");

    /* Check if uploadConfig json value is null in config file and if not null,
     * check if storeAndForward is supported
     */  
    if(uploadConfig != ic_utils::Json::Value::nullRef) 
    {
        if (uploadConfig.isMember("storeAndForward") && \
            uploadConfig["storeAndForward"].isBool() && \
            uploadConfig["storeAndForward"].asBool() == true)
        {
            // Expect true as store and forward mode is supported
            EXPECT_TRUE(CUploadMode::GetInstance()->
                        IsStoreAndForwardSupported());
        }
        else
        {
            // Expect false as store and forward mode is not supported
            EXPECT_FALSE(CUploadMode::GetInstance()->
                         IsStoreAndForwardSupported());
        }
    }
    else
    {
        HCPLOG_D << 
        "Stream will be supported by default if 'uploadMode' is not defined";
    }
}

TEST_F(CUploadModeTest, Test_GetBatchModeEventList_notEmpty)
{
    /* Configuration doesn't supports batch mode
     * hence the size of list will be zero
     */
    int nSize = CUploadMode::GetInstance()->GetBatchModeEventList().size();

    /* As mentioned above the list size is zero
     */  
    EXPECT_EQ(nSize, 0);
}

TEST_F(CUploadModeTest, Test_GetStreamModeEventList_notEmpty)
{
    /* Configuration supports stream mode and has a list of events for
     * stream upload hence the size of list will be a non-zero value
     */  
    int nSize = CUploadMode::GetInstance()->GetStreamModeEventList().size();

    // The list size is non-zero, hence expect as not equal to 0
    EXPECT_NE(nSize, 0);
}

} // namespace ic_core
