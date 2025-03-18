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
#include "CIgniteConfig.h"
#include "CIgniteLog.h"

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CIgniteConfigTest"

namespace ic_core
{
/**
 * Class CIgniteConfigTest defines a test feature for CIgniteConfig class
 */
class CIgniteConfigTest : public ::testing::Test
{
protected:
    /**
     * Constructor
     */
    CIgniteConfigTest()
    {
        // Do nothing
    }

    /**
     * Destructor
     */ 
    ~CIgniteConfigTest() override
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

TEST_F(CIgniteConfigTest, Test_ZeroTest)
{
    // Check the existance of CIgniteConfig class as Zero test case
    CIgniteConfig *pConfig = CIgniteConfig::GetInstance();
    EXPECT_NE(pConfig, nullptr);
}

TEST_F(CIgniteConfigTest, Test_GetInstance_SingleInstanceValidation)
{
    // Expect the same instance to be returned by the API
    CIgniteConfig *pConfig = CIgniteConfig::GetInstance();
    EXPECT_EQ(pConfig, CIgniteConfig::GetInstance());
}

TEST_F(CIgniteConfigTest, Test_GetConfigVersion)
{
   CIgniteConfig *pObj = CIgniteConfig::GetInstance();

   /* version field is always present in the configuration file
    * hence, expect the version to not be empty
    */
   EXPECT_NE(pObj->GetConfigVersion(), "");
}

TEST_F(CIgniteConfigTest, Test_GetString_emptyKey)
{
   CIgniteConfig *pObj = CIgniteConfig::GetInstance();
   std::string strKey = "";
   std::string strDefaultValue = "";
   int nIndex = 0;

   // Expect empty string as strKey and strDefaultValue passed are empty
   EXPECT_EQ(pObj->GetString(strKey, strDefaultValue, nIndex), "");
}

TEST_F(CIgniteConfigTest, Test_GetString_activationKey)
{
   CIgniteConfig *pObj = CIgniteConfig::GetInstance();
   std::string strKey = "HCPAuth.activate_url";
   std::string strDefaultValue = "";
   int nIndex = 0;

   // Expect string not empty as activation strKey is not empty
   EXPECT_STRNE(pObj->GetString(strKey, strDefaultValue, nIndex).c_str(), "");
}

TEST_F(CIgniteConfigTest, Test_GetInt_MQTTKeyPort)
{
    CIgniteConfig *pObj = CIgniteConfig::GetInstance();
    std::string strKey = "MQTT.port";
    int nDefaultValue = 0;
    int nIndex = 0;

    // Expect not equal to as MQTT port has a value
    EXPECT_NE(pObj->GetInt(strKey, nDefaultValue, nIndex), 0);
}

TEST_F(CIgniteConfigTest, Test_GetInt_invalidKey)
{
    CIgniteConfig *pObj = CIgniteConfig::GetInstance();
    std::string strKey = "ConfigurationFila";
    int nDefaultValue = 0; 
    int nIndex = 0;

    // Expect 0 as strKey is not valid
    EXPECT_EQ(pObj->GetInt(strKey, nDefaultValue, nIndex), 0);
}

TEST_F(CIgniteConfigTest, Test_GetLong_KeyIsEmpty)
{
    CIgniteConfig *pObj = CIgniteConfig::GetInstance();
    std::string strKey = "";
    long lDefaultValue = 0;
    int nIndex = 0;

    // Expect 0 as strKey is empty
    EXPECT_EQ(pObj->GetLong(strKey, lDefaultValue, nIndex), 0);
}

TEST_F(CIgniteConfigTest, Test_GetLong_KeyIsMaxUploadChunk)
{
    CIgniteConfig *pObj = CIgniteConfig::GetInstance();
    std::string strKey = "DAM.Upload.CurlSender.maxUploadChunk";
    long lDefaultValue = 2400000;
    int nIndex = 0;

    // Expect equal as value returned is same as 2400000
    EXPECT_EQ(pObj->GetLong(strKey, lDefaultValue, nIndex), 2400000);
}

TEST_F(CIgniteConfigTest, Test_GetBool_emptyKey)
{
    CIgniteConfig *pObj = CIgniteConfig::GetInstance();
    std::string strKey = "";
    bool bDefaultValue = true;
    int nIndex = 0;

    // Expect true as strKey is empty and hence returns default value
    EXPECT_TRUE(pObj->GetBool(strKey, bDefaultValue, nIndex));
}

TEST_F(CIgniteConfigTest, Test_GetBool_keyFileLogging) 
{
    CIgniteConfig *pObj = CIgniteConfig::GetInstance();
    std::string strKey = "FileLogger.enableLogging";
    bool bDefaultValue = false;
    int nIndex = 0;

    // Expect true as value in json is true
    EXPECT_TRUE(pObj->GetBool(strKey, bDefaultValue, nIndex));
}

TEST_F(CIgniteConfigTest, Test_GetDouble_keyIsCPULogConfig)
{
    CIgniteConfig *pObj = CIgniteConfig::GetInstance();
    std::string strKey = "DAM.Upload.CPULoadConfig.";
    double dDefaultValue = 90;
    int nIndex = 0;

    // Expect equal as value in json is equal to value returned
    EXPECT_EQ(pObj->GetDouble(strKey, dDefaultValue, nIndex), 90);
}

TEST_F(CIgniteConfigTest, Test_GetAsString_databasePath)
{
    CIgniteConfig *pObj = CIgniteConfig::GetInstance();
    std::string strKey = "DAM.Database.dbStore";
    std::string strDefaultValue = "";
    int nIndex = 0;

    // Expect string not empty as database path has value
    EXPECT_STRNE("", pObj->
                 GetAsString(strKey, strDefaultValue, nIndex).c_str());
}

TEST_F(CIgniteConfigTest, Test_GetArraySize_invalidKey)
{
    CIgniteConfig *pObj = CIgniteConfig::GetInstance();
    std::string strKey = "MessageQueueThreadPriority";

    // Expect 0 as strKey is invalid
    EXPECT_EQ(0, pObj->GetArraySize(strKey));
}

TEST_F(CIgniteConfigTest, Test_GetArraySize_fileLogging)
{
    CIgniteConfig *pObj = CIgniteConfig::GetInstance();
    std::string strKey = 
                    "FileLogger.inflowEventLogging.criticalEventLoggingCount";

    // Expect not equal to 0 as strKey is a valid array with values
    EXPECT_NE(0, pObj->GetArraySize(strKey));
}

TEST_F(CIgniteConfigTest, Test_GetConfigUpdateSource_cloudConfigUpdate)
{
    CIgniteConfig *pObj = CIgniteConfig::GetInstance();
    EconfigUpdateSource source = EconfigUpdateSource::eCLOUD_CONFIG_UPDATE; 

    /* string equal as EconfigUpdateSource source and its respective string
     * value are same
     */  
    EXPECT_EQ("cloudConfigUpdate", pObj->GetConfigUpdateSourceInString(source));
}

TEST_F(CIgniteConfigTest, Test_GetAsString_ProductType)
{
    CIgniteConfig *pObj = CIgniteConfig::GetInstance();

    // String containing Product type config key
    std::string strKeyProdTyp = "ProductType";

    // String containing configuration for Product type key in config file
    std::string strResponse = "hu";

    std::string strResPayload = pObj->GetAsString(strKeyProdTyp);

    // Expecting product type as 'hu' as configured in the config file
    EXPECT_STREQ(strResponse.c_str(), strResPayload.c_str());
}

TEST_F(CIgniteConfigTest, Test_GetJsonValue_forValidCase)
{
    CIgniteConfig *pObj = CIgniteConfig::GetInstance();

    // Valid case 01
    // String containing key value of inflow events logging from config file
    std::string strInflowEventsLogKey = "FileLogger.inflowEventLogging";

    ic_utils::Json::FastWriter writer;
    std::string strResPayload = 
                writer.write(pObj->GetJsonValue(strInflowEventsLogKey));

    // Expecting configuration returned for inflow events logging as response
    EXPECT_NE("", strResPayload);

    // Valid case 02
    // String containing key value of mqtt sub topics from config file
    std::string strSubTopicsKey = "MQTT.sub_topics";

    strResPayload = writer.write(pObj->GetJsonValue(strSubTopicsKey));

    // Expect configuration returned for mqtt subtopics as response 
    EXPECT_NE("", strResPayload);
}

TEST_F(CIgniteConfigTest, Test_GetJsonValue_forInValidCase)
{
    CIgniteConfig *pObj = CIgniteConfig::GetInstance();

    const ic_utils::Json::Value &rjsonNullValue = 
          ic_utils::Json::Value::nullRef;

    // String containing incorrect json key
    std::string strIncorrectKey = "IncorrectKey";

    // Expecting 'ic_utils::Json::Value::nullRef' for invalid json key
    EXPECT_EQ(rjsonNullValue, pObj->GetJsonValue(strIncorrectKey));

    // String containing incorrect json key
    strIncorrectKey = "[]";

    // Expecting 'ic_utils::Json::Value::nullRef' for invalid json key
    EXPECT_EQ(rjsonNullValue, pObj->GetJsonValue(strIncorrectKey));

    // String containing incorrect json key
    strIncorrectKey = 
        "FileLogger.inflowEventLogging.criticalEventLoggingCount.InvalidEvent";

    // Expecting 'ic_utils::Json::Value::nullRef' for invalid json key
    EXPECT_EQ(rjsonNullValue, pObj->GetJsonValue(strIncorrectKey));
}

/* The functions updateConfig, canOverrideMember, copyDBConfig and 
 * storeSettingsToDb are not covered in UT as they can modify the 
 * actual config file
 */

}
