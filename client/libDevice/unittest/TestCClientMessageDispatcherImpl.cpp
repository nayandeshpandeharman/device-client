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
#include "CClientMessageDispatcherImpl.h"
#include "CIgniteClient.h"
#include <jsoncpp/json.h>

namespace ic_device
{
/**
 * Class CClientMessageDispatcherImplTest defines a test feature
 * for CClientMessageDispatcherImpl class
 */
class CClientMessageDispatcherImplTest : public ::testing::Test
{
protected:
    /**
     * Constructor
     */
    CClientMessageDispatcherImplTest()
    {
        // do nothing
    }

    /**
     * Destructor
     */
    ~CClientMessageDispatcherImplTest() override
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
 * We cannot create object of CClientMessageDispatcherImpl as object is already
 * created by Client internally. If we create one more object then 
 * a new zmq sender will be created which may lead to confllict in 
 * device communicatuion
 */

TEST_F(CClientMessageDispatcherImplTest, Test_DeliverIgniteStartMessage_ExpectFalse)
{
    //Fetching IClientMessageDispatcher object
    ic_core::IClientMessageDispatcher *pClientMsgDispatcher = 
                                            ic_core::CIgniteClient::GetClientMessageDispatcher();

    //Expect false as this method is not supported
    EXPECT_FALSE(pClientMsgDispatcher->DeliverIgniteStartMessage());
}

TEST_F(CClientMessageDispatcherImplTest, Test_DeliverVINRequestToDevice_ExpectFalse)
{
    //Fetching IClientMessageDispatcher object
    ic_core::IClientMessageDispatcher *pClientMsgDispatcher = 
                                            ic_core::CIgniteClient::GetClientMessageDispatcher();

    //Expect false as this method is not supported
    EXPECT_FALSE(pClientMsgDispatcher->DeliverVINRequestToDevice());
}

TEST_F(CClientMessageDispatcherImplTest, Test_DeliverDeviceActivationStatusMessage_ExpectFalse)
{
    //Fetching IClientMessageDispatcher object
    ic_core::IClientMessageDispatcher *pClientMsgDispatcher = 
                                            ic_core::CIgniteClient::GetClientMessageDispatcher();

    //initialize bool and int status 0 as they need to send to function
    bool bActivationStatus = false;
    int nReason = 1; //activated denotes 1

    //Expect false as this method is not supported
    EXPECT_FALSE(pClientMsgDispatcher->DeliverDeviceActivationStatusMessage(bActivationStatus,nReason));
}

TEST_F(CClientMessageDispatcherImplTest, Test_DeliverICStatusToDevice_ICStatusEvent)
{
    //just for testing purpose initialize with randomn values
    std::string strICStatus = "{\"Data\":{\"state\":0}}";
    //initialize json to null
    ic_utils::Json::Value jsonData = ic_utils::Json::Value::nullRef;
    ic_utils::Json::Reader jsonReader;

    //proceed if parsing is successfull
    ASSERT_TRUE(jsonReader.parse(strICStatus, jsonData));

    //Fetching IClientMessageDispatcher object
    ic_core::IClientMessageDispatcher *pClientMsgDispatcher = 
                                            ic_core::CIgniteClient::GetClientMessageDispatcher();

    //Expect true as this method sends IC statue to device
    EXPECT_TRUE(pClientMsgDispatcher->DeliverICStatusToDevice(jsonData));
}

TEST_F(CClientMessageDispatcherImplTest, Test_DeliverShutdownNotifAckToDevice_shutdownInitiated)
{
    //initialize json string that to be sent as part of IC Status
    ic_utils::Json::Value ackJson = ic_utils::Json::Value::nullRef;
    ackJson["state"] = 1; // 1 repesents shutdown initiated

    //Fetching IClientMessageDispatcher object
    ic_core::IClientMessageDispatcher *pClientMsgDispatcher = 
                                            ic_core::CIgniteClient::GetClientMessageDispatcher();

    //Expect true as this method sends DeviceShutdownNotifAck statue to device
    EXPECT_TRUE(pClientMsgDispatcher->DeliverShutdownNotifAckToDevice(ackJson));
}

TEST_F(CClientMessageDispatcherImplTest, Test_DeliverActivationDetails_ActivationStatusEvent)
{
    //just for testing purpose initialize with randomn values for ACtivation status
    std::string strActivationStatusStatus = "{\"Data\":{\"deviceId\":\
                                            \"DOMH9EBA277220\",\"status\":1,\
                                             \"vin\":\"TESTVIN_Make:GenMake_Model:GenDevice_Year:2024_Type:Linux\"}}";
    //initialize json to null
    ic_utils::Json::Value jsonData = ic_utils::Json::Value::nullRef;
    ic_utils::Json::Reader jsonReader;

    //proceed if parsing is successfull
    ASSERT_TRUE(jsonReader.parse(strActivationStatusStatus, jsonData));

    //Fetching IClientMessageDispatcher object
    ic_core::IClientMessageDispatcher *pClientMsgDispatcher = 
                                            ic_core::CIgniteClient::GetClientMessageDispatcher();

    //Expect true as this method sends Activation Status to device
    EXPECT_TRUE(pClientMsgDispatcher->DeliverActivationDetails(jsonData));
}

TEST_F(CClientMessageDispatcherImplTest, Test_DeliverDBSizeToDevice_DBSizeEvent)
{
    //just for testing purpose initialize with randomn values for Json string
    std::string strDBSize = "{\"Data\":{\"DataBaseSize\":40960,\"MaxDataBaseSize\":15728640}}";
    //initialize json to null
    ic_utils::Json::Value jsonData = ic_utils::Json::Value::nullRef;
    ic_utils::Json::Reader jsonReader;

    //proceed if parsing is successfull
    ASSERT_TRUE(jsonReader.parse(strDBSize, jsonData));

    //Fetching IClientMessageDispatcher object
    ic_core::IClientMessageDispatcher *pClientMsgDispatcher = 
                                            ic_core::CIgniteClient::GetClientMessageDispatcher();

    //Expect true as this method sends DBSize event to device
    EXPECT_TRUE(pClientMsgDispatcher->DeliverDBSizeToDevice(jsonData));
}

TEST_F(CClientMessageDispatcherImplTest, Test_DeliverMQTTConnectionStatusToDevice_MQTTConnectionStatusEvent)
{
    //initialize json string that to be sent as part of MQTTConnectionStatus event
    std::string strMQTTConnectionStatus = "{\"Data\":{\"status\":0}}";
    //initialize json to null
    ic_utils::Json::Value jsonData = ic_utils::Json::Value::nullRef;
    ic_utils::Json::Reader jsonReader;

    //proceed if parsing is successfull
    ASSERT_TRUE(jsonReader.parse(strMQTTConnectionStatus, jsonData));

    //Fetching IClientMessageDispatcher object
    ic_core::IClientMessageDispatcher *pClientMsgDispatcher = 
                                            ic_core::CIgniteClient::GetClientMessageDispatcher();

    //Expect true as this method sends MQTTConnectionStatus event to device
    EXPECT_TRUE(pClientMsgDispatcher->DeliverMQTTConnectionStatusToDevice(jsonData));
}

TEST_F(CClientMessageDispatcherImplTest, Test_DeliverRemoteOperationMessage_RemoteOperationDoorsEvent)
{
    //just for testing purpose initialize with randomn values
    std::string strROEvent = "{\"BizTransactionId\":\"52a9e5b0-fb22-11ee-a779-374c51b6529f\", \
                            \"Data\":{\"origin\":\"ServiceConfiguration,\
                            ManageNotifications,AssociateMyselfToVehicle,SelfManage,ViewMySubscriptions, \
                            SubscribeMyselfToProducts\",\
                            \"roRequestId\":\"52a9e5b0-fb22-11ee-a779-374c51b6529f\",\"state\":\"UNLOCKED\",\
                            \"topic\":\"DOBOQAMF264378/2c/ro\",\
                            \"userId\":\"new_robot_user_1707297004@yopmail.com\",\
                            \"vehicleArchType\":\"tcu\"},\
                            \"EventID\":\"RemoteOperationDoors\",\"MessageId\":\"260015\",\
                            \"Timestamp\":1713183532951,\"Timezone\":0,\"UserContext\":\
                            [{\"role\":\"VO\",\"userId\":\"new_robot_user_1707297004@yopmail.com\"}],\
                            \"Version\":\"1.1\"}";
    //initialize json to null
    ic_utils::Json::Value jsonData = ic_utils::Json::Value::nullRef;
    ic_utils::Json::Reader jsonReader;

    //just for testing purpose initialize with randomn topic
    std::string strTopic = "DOBOQAMF264378/2c/ro";

    //proceed if parsing is successfull
    ASSERT_TRUE(jsonReader.parse(strROEvent, jsonData));

    //Fetching IClientMessageDispatcher object
    ic_core::IClientMessageDispatcher *pClientMsgDispatcher = 
                                            ic_core::CIgniteClient::GetClientMessageDispatcher();

    //Expect true as this method sends RemoteOperationDoors event to device
    EXPECT_TRUE(pClientMsgDispatcher->DeliverRemoteOperationMessage(jsonData,strTopic));
}
}
