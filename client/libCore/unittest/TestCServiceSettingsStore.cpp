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
#include "db/CServiceSettingsStore.h"

namespace ic_core
{
/**
 * Class CServiceSettingsStoreTest defines a test feature for 
 * CServiceSettingsStore class
 */
class CServiceSettingsStoreTest : public ::testing::Test
{
protected:
    /**
     * Constructor
     */
    CServiceSettingsStoreTest()
    {
        // Do nothing
    }

    
    /**
     * Destructor
     */ 
    ~CServiceSettingsStoreTest() override
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

TEST_F(CServiceSettingsStoreTest, Test_ZeroTest)
{
    // Checking the existance of CServiceSettingsStore class
    EXPECT_NE(CServiceSettingsStore::GetInstance(), nullptr);
}

TEST_F(CServiceSettingsStoreTest, Test_StoreSettings_True_case1value)
{
    // Assign values to the variables
    std::string strServiceId = "mediaManagement";
    std::string strSettingEnum = "ADAS_RECORDING_TIME";

    // Creating the Json payload with "value" parameter
    ic_utils::Json::Value settingValue = ic_utils::Json::objectValue;
    settingValue["value"] = "OFF";

    // Making the Json payload an array type
    ic_utils::Json::Value data(ic_utils::Json::arrayValue);
    data.append(settingValue);

    // Expect the API to return true on successfully storing values
    EXPECT_TRUE(CServiceSettingsStore::GetInstance()->
                StoreSettings(strServiceId,strSettingEnum,data));
}

TEST_F(CServiceSettingsStoreTest, Test_StoreSettings_True_case2strSettingEnum)
{
    // Assign values to the variables
    std::string strServiceId = "mediaManagement";
    std::string strMsgId = "8b002add-4daf-4ec1-9f6e-4f7ebef993af";
    bool bIsFromDevice = true;
    
    // Creating the Json payload with "strSettingEnum" and "value" Parameters
    ic_utils::Json::Value settingValue = ic_utils::Json::objectValue;
    settingValue["strSettingEnum"] = "ADAS_RECORDING_TIME";
    settingValue["value"] = "ON";

    // Making the Json payload an array type
    ic_utils::Json::Value data(ic_utils::Json::arrayValue);
    data.append(settingValue);

    // Expect the API to return true on successfully storing values
    EXPECT_TRUE(CServiceSettingsStore::GetInstance()->
                StoreSettings(strServiceId,data,strMsgId,bIsFromDevice));
}

TEST_F(CServiceSettingsStoreTest, Test_UpdateResponseReceivedForSettings)
{
    // Assign values to the variables
    std::string strServiceId = "mediaManagement";
    std::string strSettingEnum = "ADAS_RECORDING_TIME";

    // Expect the API to return true on successfully storing response to dB
    EXPECT_TRUE(CServiceSettingsStore::GetInstance()->
                UpdateResponseReceivedForSettings(strServiceId, 
                                                  strSettingEnum));
}

TEST_F(CServiceSettingsStoreTest, Test_GetCorrIdForSettings_True_Case)
{
    // Assign values to the variables
    std::string strServiceId = "mediaManagement";
    std::string strCorID = "8b002add-4daf-4ec1-9f6e-4f7ebef993af";
    bool bIsFromDevice = true;
    
    // Creating the Json payload with "strSettingEnum" and "value" Parameters
    ic_utils::Json::Value settingValue = ic_utils::Json::objectValue;
    settingValue["strSettingEnum"] = "ADAS_RECORDING_TIME";
    settingValue["value"] = "ON";

    // Making the Json payload an array type
    ic_utils::Json::Value data(ic_utils::Json::arrayValue);
    data.append(settingValue);

    // Storing the values to the db
    CServiceSettingsStore::GetInstance()->
                           StoreSettings(strServiceId, data, strCorID, 
                                         bIsFromDevice);

    /* Expect the API to return the same "strCorID" stored
     * by the "StoreSettings"
     */  
    std::string strCorrID = CServiceSettingsStore::GetInstance()->
                            GetCorrIdForSettings(strServiceId,bIsFromDevice);
    EXPECT_STREQ(strCorrID.c_str(),strCorID.c_str());
}

TEST_F(CServiceSettingsStoreTest, Test_GetCorrIdForSettings_False_Case)
{
    // Assign values to the variables
    std::string strServiceId = "mediaManagement";
    std::string strSettingEnum = "ADAS_RECORDING_TIME";

    /* As a pre-requisite for executing this false case, clearing the 
     * settings for mediaManagement (strServiceId) and
     * ADAS_RECORDING_TIME (strSettingEnum) from DB
     */
    EXPECT_TRUE(CServiceSettingsStore::GetInstance()->
                                       ClearSettings(strServiceId, 
                                                     strSettingEnum));

    /* Expect the API to return empty value as no "strCorrID" is stored 
     * in DB for the given 'mediaManagement' (strServiceId) with 
     * ADAS_RECORDING_TIME (strSettingEnum)
     */
    std::string strCorrID = CServiceSettingsStore::GetInstance()->
                            GetCorrIdForSettings(strServiceId, strSettingEnum);
    EXPECT_STREQ(strCorrID.c_str(),"");
}

TEST_F(CServiceSettingsStoreTest, Test_UpdateAcknowledgementReceivedFromCloud)
{
    // Assign values to the variables
    std::string strServiceId = "mediaManagement";
    std::string strCorrelationId = "812e38c3-c507-488c-8bfd-dda85dc16e2c";
    std::string strResp = "SUCCESS";

    /* Expect the API to update respons field on receiving setting Ack from
     * Cloud and return true on successfully storing response to dB
     */
    EXPECT_TRUE(CServiceSettingsStore::GetInstance()->
                UpdateAcknowledgementReceivedFromCloud(strServiceId, 
                                                       strCorrelationId,
                                                       strResp));
}

TEST_F(CServiceSettingsStoreTest, Test_GetServiceIdFromCorId_Positive_Case)
{
    // Assign values to the variables
    std::string strServiceId = "mediaManagement";
    std::string strCorrId = "8b002add-4daf-4ec1-9f6e-4f7ebef993af";
    bool bIsFromDevice = true;
    
    // Creating the Json payload with "strSettingEnum" and "value" Parameters
    ic_utils::Json::Value settingValue = ic_utils::Json::objectValue;
    settingValue["strSettingEnum"] = "ADAS_RECORDING_TIME";
    settingValue["value"] = "ON";

    // Making the Json payload an array type
    ic_utils::Json::Value data(ic_utils::Json::arrayValue);
    data.append(settingValue);

    // Storing the values to the db
    CServiceSettingsStore::GetInstance()->StoreSettings(strServiceId, data, 
                                                        strCorrId, 
                                                        bIsFromDevice);

    // Assign values to the variables
    std::string strCorrelationId = "8b002add-4daf-4ec1-9f6e-4f7ebef993af";

    /* Expect the API to return the same "strServiceId" stored 
     * by the "StoreSettings"
     */ 
        
    EXPECT_STREQ(CServiceSettingsStore::GetInstance()->
                 GetServiceIdFromCorId(strCorrelationId,
                                       bIsFromDevice).c_str(),
                                       strServiceId.c_str());
}

TEST_F(CServiceSettingsStoreTest, Test_GetServiceIdFromCorId_Negative_Case)
{
    // Assign values to the variables
    std::string strCorrelationId = "8b002add-4daf-4ec1-9f6e-4f7ebef993af";
    bool bIsFromDevice = false;

    /* Expect the API to return empty value as no "strServiceId" is 
     * stored in the db
     */
    EXPECT_STREQ(CServiceSettingsStore::GetInstance()->
                 GetServiceIdFromCorId(strCorrelationId,
                                       bIsFromDevice).c_str(),"");
}

TEST_F(CServiceSettingsStoreTest, Test_GetValueForSettings_IsFromDeviceFalseValue)
{
    /* As a pre-requisite, store a setting with 'mediaManagement' 
     * (as strServiceId) and 'bIsFromDevice' as true
     * along with a sample strCorrId and the setting value
     */
    std::string strServiceId = "mediaManagement";
    bool bIsFromDevice = true;
    std::string strCorrId = "8b002add-4daf-4ec1-9f6e-4f7ebef993af";

    // Creating settingValue Json payload with the sample data
    ic_utils::Json::Value settingValue = ic_utils::Json::objectValue;
    settingValue["strSettingEnum"] = "ADAS_RECORDING_TIME";
    settingValue["value"] = "ON";

    // Add the sample settingValue into an array object
    ic_utils::Json::Value arrayData(ic_utils::Json::arrayValue);
    arrayData.append(settingValue);

    // Storing the settings in the DB
    CServiceSettingsStore::GetInstance()->StoreSettings(strServiceId, 
                                                        arrayData, 
                                                        strCorrId, 
                                                        bIsFromDevice);

    /* Set value to false which further used to request status for
     * mediaManagement from DB
     */  
    bIsFromDevice = false;

    /* Expect false as 'mediaManagement' strServiceId was stored
     * with bIsFromDevice = true
     */  
    EXPECT_FALSE(CServiceSettingsStore::GetInstance()->
                 GetStatusForSettings(strServiceId, 
                                      bIsFromDevice));                    
}

TEST_F(CServiceSettingsStoreTest, Test_GetValueForSettings_case2)
{
    // Assign the invalid value to the variable
    std::string strServiceId = "dummyserviceid";
    std::string strSettingEnum = "dummyenum";

    /* Expect the API to return empty value as invalid value is
     * assigned to the variable
     */  
    EXPECT_STREQ(CServiceSettingsStore::GetInstance()->
                 GetValueForSettings(strServiceId, strSettingEnum).c_str(),"");
}

TEST_F(CServiceSettingsStoreTest, Test_ClearSettings)
{
    // Assign the invalid value to the variable
    std::string strServiceId = "mediaManagement";
    std::string strSettingEnum = "ADAS_RECORDING_TIME";

    /* Expect the API to return true when clear the setting from table 
     * when settings are present in the db
     */
    EXPECT_TRUE(CServiceSettingsStore::GetInstance()->
                ClearSettings(strServiceId, strSettingEnum));
}
}