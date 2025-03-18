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
#include "core/CPersistancyAndStateHandler.h"
#include "db/CLocalConfig.h"

namespace ic_bl 
{
//! Global variable to store instance of CPersistancyAndStateHandler
CPersistancyAndStateHandler* g_pPersisAndStateHndlrObj = NULL;

//! Constant key for 'TESTVIN_Make' string
static const std::string KEY_TESTVIN_MAKE = 
                    "TESTVIN_Make:GenMake_Model:GenDevice_Year:2024_Type:Linux";

//! Constant key for 'status' string
static const std::string KEY_STATUS = "status";

//! Constant key for 'state' string
static const std::string KEY_STATE = "state";

//! Constant key for 'vin' string
static const std::string KEY_VIN = "vin";

/**
 * Class CPersistancyAndStateHandlerTest defines a test feature for  
 * CPersistancyAndStateHandler class
 */
class CPersistancyAndStateHandlerTest : public ::testing::Test 
{
public:
    /**
     * Wrapper method for CPersistancyAndStateHandler::GetInstance()
     * @see CPersistancyAndStateHandler::GetInstance()
     */
    CPersistancyAndStateHandler* GetInstance();

    /**
     * Wrapper method for CPersistancyAndStateHandler::PersistActivationStatus()
     * @see CPersistancyAndStateHandler::PersistActivationStatus()
     */
    bool PersistActivationStatus(const std::string &rstrActivationDetails);

    /**
     * Wrapper method for CPersistancyAndStateHandler::PersistICstate()
     * @see CPersistancyAndStateHandler::PersistICstate()
     */
    bool PersistICstate(enum ic_core::IC_STATE eState);

    /**
     * Wrapper method for CPersistancyAndStateHandler::ClearActivationStatus()
     * @see CPersistancyAndStateHandler::ClearActivationStatus()
     */
    bool ClearActivationStatus();

    /**
     * Wrapper method for CPersistancyAndStateHandler::GetKeyFromId()
     * @see CPersistancyAndStateHandler::GetKeyFromId()
     */
    std::string GetKeyFromId(enum ic_core::IC_QUERY_ID eQId);

    /**
     * Wrapper method for CPersistancyAndStateHandler::GetICParam()
     * @see CPersistancyAndStateHandler::GetICParam()
     */
    ic_utils::Json::Value GetICParam(ic_core::IC_QUERY_ID eQId);

    /**
     * Constructor
     */
    CPersistancyAndStateHandlerTest() 
    {
        // Do nothing
    }

    /**
     * Destructor
     */
    ~CPersistancyAndStateHandlerTest() override 
    {
        // Do nothing
    }
    
    /**
     * Overriding Method of testing::Test class
     * @see testing::Test::SetUp()
     */
    void SetUp() override 
    {
        g_pPersisAndStateHndlrObj = CPersistancyAndStateHandler::GetInstance();
    }

    /**
     * Overriding Method of testing::Test class
     * @see testing::Test::TearDown()
     */
    void TearDown() override 
    {
        g_pPersisAndStateHndlrObj = NULL;
    }

protected:
    /**
     * Overriding Method of testing::Test class
     * @see testing::Test::TestBody()
     */
    void TestBody() override 
    {
        // Do nothing
    }
};

CPersistancyAndStateHandler* CPersistancyAndStateHandlerTest::GetInstance()
{
    return g_pPersisAndStateHndlrObj->GetInstance();
}

bool CPersistancyAndStateHandlerTest::PersistActivationStatus(
                                   const std::string &rstrActivationDetailsJson)
{
    return g_pPersisAndStateHndlrObj->
                             PersistActivationStatus(rstrActivationDetailsJson);
}

bool CPersistancyAndStateHandlerTest::PersistICstate(ic_core::IC_STATE eState)
{
    return g_pPersisAndStateHndlrObj->PersistICstate(eState);
}

bool CPersistancyAndStateHandlerTest::ClearActivationStatus()
{
    return g_pPersisAndStateHndlrObj->ClearActivationStatus();
}

std::string CPersistancyAndStateHandlerTest::GetKeyFromId(ic_core::IC_QUERY_ID
                                                          eQId)
{
    return g_pPersisAndStateHndlrObj->GetKeyFromId(eQId);
}

ic_utils::Json::Value CPersistancyAndStateHandlerTest::GetICParam(
                                                      ic_core::IC_QUERY_ID eQId)
{
    return g_pPersisAndStateHndlrObj->GetICParam(eQId);
}

//Tests
TEST_F(CPersistancyAndStateHandlerTest, 
                             Test_checkCPersistancyAndStateHandlerClassCreation) 
{
    /* check the existence of CPersistancyAndStateHandler class as Zero test 
     * case as per TDD approach
     */
    CPersistancyAndStateHandlerTest obj;
}

TEST_F(CPersistancyAndStateHandlerTest, Test_getInstance) 
{
    CPersistancyAndStateHandlerTest obj;
    EXPECT_EQ(g_pPersisAndStateHndlrObj,obj.GetInstance());
}

TEST_F(CPersistancyAndStateHandlerTest, Test_getInstanceManyCaseValidation) 
{
    CPersistancyAndStateHandler *pCPersistancyAndStateHandlerObj1 = 
                                     CPersistancyAndStateHandler::GetInstance();
    CPersistancyAndStateHandler *pCPersistancyAndStateHandlerObj2 = 
                                     CPersistancyAndStateHandler::GetInstance();
    CPersistancyAndStateHandler *pCPersistancyAndStateHandlerObj3 = 
                                     CPersistancyAndStateHandler::GetInstance();
    CPersistancyAndStateHandler *pCPersistancyAndStateHandlerObj4 = 
                                     CPersistancyAndStateHandler::GetInstance();
    CPersistancyAndStateHandlerTest obj;

    /* Expect pCPersistancyAndStateHandlerObj1 equals to obj.GetInstance() 
     * since CPersistancyAndStateHandler is singleton class
     */
    EXPECT_EQ(pCPersistancyAndStateHandlerObj1, obj.GetInstance());

    /* Expect pCPersistancyAndStateHandlerObj2 equals to obj.GetInstance() since
     * CPersistancyAndStateHandler is singleton class
     */
    EXPECT_EQ(pCPersistancyAndStateHandlerObj2, obj.GetInstance());

    /* Expect pCPersistancyAndStateHandlerObj3 equals to obj.GetInstance() since
     * CPersistancyAndStateHandler is singleton class
     */
    EXPECT_EQ(pCPersistancyAndStateHandlerObj3, obj.GetInstance());

    /* Expect mpPersistancyAndStateHandlerObj4 equals to obj.GetInstance() since
     * CPersistancyAndStateHandler is singleton class
     */
    EXPECT_EQ(pCPersistancyAndStateHandlerObj4, obj.GetInstance());
}

TEST_F(CPersistancyAndStateHandlerTest, Test_persistActivationStatus) 
{
    std::string strActDetails = "{\"deviceId\":\"DOX88Z35159722\",\"status\":"
    "1,\"vin\":\"TESTVIN_Make:GenMake_Model:GenDevice_Year:2024_Type:Linux\"}";

    CPersistancyAndStateHandlerTest obj;

    //Expect true since persistActivationStatus able to store Activation details
    EXPECT_TRUE(obj.PersistActivationStatus(strActDetails));
}

TEST_F(CPersistancyAndStateHandlerTest, Test_persistICstateSTARTED) 
{
    CPersistancyAndStateHandlerTest obj;

    //Expect true since PersistICstate able to store IC state as started
    EXPECT_TRUE(obj.PersistICstate(ic_core::IC_STATE::eSTARTED));
}

TEST_F(CPersistancyAndStateHandlerTest, Test_persistICstateEXITING) 
{
    CPersistancyAndStateHandlerTest obj;

    //Expect true since PersistICstate able to store IC state as exiting
    EXPECT_TRUE(obj.PersistICstate(ic_core::IC_STATE::eEXITING));
}

TEST_F(CPersistancyAndStateHandlerTest, Test_clearActivationStatus) 
{
    ic_utils::Json::Value jsonResult = ic_utils::Json::Value::nullRef;
    CPersistancyAndStateHandlerTest obj;

    //Persiting Activation status as ACTIVATED in payload as "status\":1
    std::string strActDetails = "{\"deviceId\":\"DOX88Z35159722\",\"status\":"
     "1,\"vin\":\"TESTVIN_Make:GenMake_Model:GenDevice_Year:2024_Type:Linux\"}";
    EXPECT_TRUE(obj.PersistActivationStatus(strActDetails));
    
    //checking the activation status from local config
    jsonResult = obj.GetICParam(ic_core::IC_QUERY_ID::eACTIVATION_STATUS);
    EXPECT_EQ(jsonResult[KEY_STATUS].asInt(), 
              ic_core::IC_ACTIVATION_STATUS::eACTIVATED);

    //clearing Activation status from local config
    EXPECT_TRUE(obj.ClearActivationStatus());

    //checking activation status when it is not available in local config
    jsonResult = obj.GetICParam(ic_core::IC_QUERY_ID::eACTIVATION_STATUS);
    EXPECT_EQ(jsonResult[KEY_STATUS].asInt(),
              ic_core::IC_ACTIVATION_STATUS::eNOT_ACTIVATED);
}

TEST_F(CPersistancyAndStateHandlerTest, Test_getKeyFromIdforIC)
{
    CPersistancyAndStateHandlerTest obj;

    //Fetch Key w.r.t IC current state
    std::string strIcCurrentStateKeyStr = 
                      obj.GetKeyFromId(ic_core::IC_QUERY_ID::eIC_CURRENT_STATE);

    //Expect the IC current state key string equals to "ICP.ICRunningStatus"
    EXPECT_EQ(strIcCurrentStateKeyStr.c_str(), KEY_IC_RUNNING_STATUS);
}

TEST_F(CPersistancyAndStateHandlerTest, Test_getKeyFromIdforActivation)
{
    CPersistancyAndStateHandlerTest obj;

    //Fetch Key w.r.t activation status
    std::string strActivationStatusKeyStr = 
                     obj.GetKeyFromId(ic_core::IC_QUERY_ID::eACTIVATION_STATUS);

    //Expect the activation status key string equals to "ICP.ActivationStatus"
    EXPECT_EQ(strActivationStatusKeyStr.c_str(), KEY_ACTIVATION_STATUS);
}

TEST_F(CPersistancyAndStateHandlerTest, Test_getKeyFromId_for_DBSize)
{
    CPersistancyAndStateHandlerTest obj;

    //Fetch Key w.r.t DBSize
    std::string strDbSizeKeyStr = 
                               obj.GetKeyFromId(ic_core::IC_QUERY_ID::eDB_SIZE);

    //Expect the db size key string equals to "DbSize"
    EXPECT_EQ(strDbSizeKeyStr, KEY_DATABASE_SIZE);
}

TEST_F(CPersistancyAndStateHandlerTest, Test_getKeyFromId_for_MqttConnStatus)
{
    CPersistancyAndStateHandlerTest obj;

    //Fetch Key w.r.t MQTT Connection Status
    std::string strMqttConnStatusKeyStr = 
                      obj.GetKeyFromId(ic_core::IC_QUERY_ID::eMQTT_CONN_STATUS);

    //Expect the MQTT Connection status key equals to "MqttConnStatus"
    EXPECT_EQ(strMqttConnStatusKeyStr, KEY_MQTT_CONN_STATUS);
}

TEST_F(CPersistancyAndStateHandlerTest, Test_getKeyFromId_for_DeviceID)
{
    CPersistancyAndStateHandlerTest obj;

    //Fetch Key w.r.t device Id
    std::string strDeviceIdKeyStr = 
                             obj.GetKeyFromId(ic_core::IC_QUERY_ID::eDEVICE_ID);

    //Expect the Device ID key equals to "login"
    EXPECT_EQ(strDeviceIdKeyStr, KEY_LOGIN);
}

TEST_F(CPersistancyAndStateHandlerTest,
                          Test_getKeyFromIdfor_for_Last_Successful_batch_upload)
{
    CPersistancyAndStateHandlerTest obj;

    //Fetch Key w.r.t Last successful batch upload
    std::string strLastSuccessfulUploadKeyStr = 
          obj.GetKeyFromId(ic_core::IC_QUERY_ID::eLAST_SUCCESSFUL_BATCH_UPLOAD);

    /* Expect the Last successful batch upload key equals to 
     * "UploaderService.LastSuccessfulUpload"
     */
    EXPECT_EQ(strLastSuccessfulUploadKeyStr, 
              KEY_UPLOAD_SERVICE_LAST_SUCCESSFUL_UPLOAD);
}

TEST_F(CPersistancyAndStateHandlerTest, Test_getICParamForICRunningStatus) 
{
    CPersistancyAndStateHandlerTest obj;
    ic_utils::Json::Value jsonResult = ic_utils::Json::Value::nullRef;
    
    obj.PersistICstate(ic_core::IC_STATE::eSTARTED);
    jsonResult = obj.GetICParam(ic_core::IC_QUERY_ID::eIC_CURRENT_STATE);

    /* Expect IC state with state as started since we stored in PersistICstate
     * function the IC state as started
     */
    EXPECT_EQ(jsonResult[KEY_STATE].asInt(), ic_core::IC_STATE::eSTARTED);

    obj.PersistICstate(ic_core::IC_STATE::eEXITING);
    jsonResult = obj.GetICParam(ic_core::IC_QUERY_ID::eIC_CURRENT_STATE);

    /* Expect IC state with state as exiting since we stored in PersistICstate
     * function the IC state as exiting
     */
    EXPECT_EQ(jsonResult[KEY_STATE].asInt(), ic_core::IC_STATE::eEXITING);
}

TEST_F(CPersistancyAndStateHandlerTest, Test_getICParamForActivationStatus) 
{
    CPersistancyAndStateHandlerTest obj;
    ic_utils::Json::Value jsonResult = ic_utils::Json::Value::nullRef;
    std::string strActDetails = "";

    //ActivationStatus = ACTIVATED
    strActDetails ="{\"deviceId\":\"DOX88Z35159722\",\"status\":1,\"vin\":"
    "\"TESTVIN_Make:GenMake_Model:GenDevice_Year:2024_Type:Linux\"}";
    obj.PersistActivationStatus(strActDetails);

    jsonResult = obj.GetICParam(ic_core::IC_QUERY_ID::eACTIVATION_STATUS);

    /* Expect Actiivation status with status as Activated since we stored in 
     * persistActivationStatus function the activation status as 1
     */
    EXPECT_EQ(jsonResult[KEY_STATUS].asInt(),
              ic_core::IC_ACTIVATION_STATUS::eACTIVATED);

    /* Expect true since Device details contians vin attribute with value as
     * "TESTVIN_Make:GenMake_Model:GenDevice_Year:2024_Type:Linux"
     */
    EXPECT_EQ(jsonResult[KEY_VIN].asCString(),KEY_TESTVIN_MAKE);

    //ActivationStatus = NOT_ACTIVATED
    strActDetails ="{\"deviceId\":\"DOX88Z35159722\",\"status\":0,\"vin\":"
    "\"TESTVIN_Make:GenMake_Model:GenDevice_Year:2024_Type:Linux\"}";
    obj.PersistActivationStatus(strActDetails);

    jsonResult = obj.GetICParam(ic_core::IC_QUERY_ID::eACTIVATION_STATUS);

    /* Expect Actiivation status with status as not Activated since we stored in
     * persistActivationStatus function the activation status as 0
     */
    EXPECT_EQ(jsonResult[KEY_STATUS].asInt(), 
              ic_core::IC_ACTIVATION_STATUS::eNOT_ACTIVATED);

    /* Expect true since device details contians vin attribute with value as
     * "TESTVIN_Make:GenMake_Model:GenDevice_Year:2024_Type:Linux"
     */
    EXPECT_EQ(jsonResult[KEY_VIN].asCString(),KEY_TESTVIN_MAKE);
}

TEST_F(CPersistancyAndStateHandlerTest, Test_getICParam_for_DBSize)
{
    CPersistancyAndStateHandlerTest obj;

    //Fetch DB Size using GetICParam
    ic_utils::Json::Value jsonResult = 
                                  obj.GetICParam(ic_core::IC_QUERY_ID::eDB_SIZE);

    /* Check the returned json has member DataBaseSize and DataBaseSize has 
     * value in Uint64 format else stop the test
     */
    ASSERT_TRUE((jsonResult.isMember(KEY_DB_SIZE) && 
                 jsonResult[KEY_DB_SIZE].isUInt64()));

    /* Expect DB Size greater than 0 since Database is already created before
     * performing test and even if there is no data in db there will be some 
     * size already allocated to db.
     */
    EXPECT_GT(jsonResult[KEY_DB_SIZE].asUInt64(), 0);

    /* Check the returned json has member MaxDataBaseSize and MaxDataBaseSize
     * has value in int format else stop the test
     */
    ASSERT_TRUE((jsonResult.isMember(KEY_MAX_DB_SIZE) && 
                 jsonResult[KEY_MAX_DB_SIZE].isInt()));

    /* Expect Max DBSize greater than 0 as Max DB Size value greater than 0 is
     * already configured in configuration file.
     */
    EXPECT_GT(jsonResult[KEY_MAX_DB_SIZE].asInt(), 0);
}

TEST_F(CPersistancyAndStateHandlerTest, Test_getICParam_for_DeviceId)
{
    CPersistancyAndStateHandlerTest obj;
    
    //Fetch stored deviceId from DB
    std::string strStoredDeviceId = 
                           ic_core::CLocalConfig::GetInstance()->Get(KEY_LOGIN);

    //Fetch Device Id using GetICParam
    ic_utils::Json::Value jsonResult = 
                               obj.GetICParam(ic_core::IC_QUERY_ID::eDEVICE_ID);

    /* Check the returned json has member DeviceId and DeviceId has value in 
     * String format else stop the test
     */
    ASSERT_TRUE((jsonResult.isMember(KEY_DEVICE_ID) && 
                 jsonResult[KEY_DEVICE_ID].isString()));

    /* Expect DeviceId fetched from GetICParam and also from DB stored DeviceId
     * same
     */
    std::string strDeviceId = jsonResult[KEY_DEVICE_ID].asString();
    EXPECT_EQ(strDeviceId, strStoredDeviceId);
}

TEST_F(CPersistancyAndStateHandlerTest, Test_getICParam_for_MqttConnStatus)
{
    CPersistancyAndStateHandlerTest obj;

    //Fetch mqtt Connection Status using GetICParam
    ic_utils::Json::Value jsonResult = 
                        obj.GetICParam(ic_core::IC_QUERY_ID::eMQTT_CONN_STATUS);

    /* Check the returned json has member status and status has value in int 
     * format else stop the test
     */
    ASSERT_TRUE((jsonResult.isMember(KEY_STATUS) && 
                 jsonResult[KEY_STATUS].isInt()));

    /* Expect mqtt Connection status fetched from getICParame equals to 
     * eSTATE_CONNECTION_COMPLETE as mqtt gets connected before performing
     * below test
     */
    ic_core::MQTT_CONNECTION_STATE mqttConnStatus = 
                 (ic_core::MQTT_CONNECTION_STATE)jsonResult[KEY_STATUS].asInt();
    
    //expecting one of the mqtt states to be returned 
    EXPECT_EQ(1,((mqttConnStatus == ic_core::eSTATE_NOT_CONNECTED)||
                 (mqttConnStatus == ic_core::eSTATE_CONNECTION_COMPLETE)||
         (mqttConnStatus == ic_core::eSTATE_CONNECTED_BUT_TOPICS_SUBS_PENDING)||
                 (mqttConnStatus == ic_core::eSTATE_CONNECTION_TEARING_DOWN)
                )
            );
}

TEST_F(CPersistancyAndStateHandlerTest, Test_getICParam_for_LastSuccessfulUpload)
{
    CPersistancyAndStateHandlerTest obj;

    //Fetch Last successful Upload from DB
    std::string strStoredLastSuccessfulUpload = 
                                     ic_core::CLocalConfig::GetInstance()->Get(
                                     KEY_UPLOAD_SERVICE_LAST_SUCCESSFUL_UPLOAD);

    //Fetch LAST_SUCCESSFULL_UPLOAD using GetICParam
    ic_utils::Json::Value jsonResult = 
            obj.GetICParam(ic_core::IC_QUERY_ID::eLAST_SUCCESSFUL_BATCH_UPLOAD);

    /* Check the returned json has member LastSuccessfulUpload and
     * LastSuccessfulUpload has value in String format else stop the test
     */
    ASSERT_TRUE((jsonResult.isMember(KEY_LAST_SUCCESSFUL_UPLOAD) && 
                 jsonResult[KEY_LAST_SUCCESSFUL_UPLOAD].isString()));

    /* Expect LastSuccessfulUpload value fetched from GetICParam and 
     * strStoredLastSuccessfulUpload db value same
     */
    std::string strLastSuccessfulUpload = 
                              jsonResult[KEY_LAST_SUCCESSFUL_UPLOAD].asString();
    EXPECT_EQ(strLastSuccessfulUpload, strStoredLastSuccessfulUpload);
}
}