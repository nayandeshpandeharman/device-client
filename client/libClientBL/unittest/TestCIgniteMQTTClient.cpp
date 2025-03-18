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
#include "net/CIgniteMQTTClient.h"
#include "jsoncpp/json.h"
#include "CIgniteConfig.h"
#include "db/CLocalConfig.h"
#include "CIgniteGZip.h"
#include "CIgniteClient.h"
#include "CMQTTClient.h"

namespace ic_bl
{
//! Global variable to store instance of CIgniteMQTTClient
CIgniteMQTTClient *g_pIgniteMQTTClientObj = NULL;

/**
 * Class CIgniteMQTTClientTest defines a test feature for CIgniteMQTTClient 
 * class
 */
class CIgniteMQTTClientTest : public ::testing::Test
{
public:
    /**
     * Wrapper method for CIgniteMQTTClient::GetInstance()
     * @see CIgniteMQTTClient::GetInstance()
     */
    CIgniteMQTTClient *GetInstance();

    /**
     * Wrapper method for CIgniteMQTTClient::BroadcastConnStatus()
     * @see CIgniteMQTTClient::BroadcastConnStatus()
     */
    bool BroadcastConnStatus(CIgniteMQTTClient::MQTT_CONNECTION_STEPS 
                             eConnState);
    
    /**
     * Wrapper method for CIgniteMQTTClient::GetUnameprefix()
     * @see CIgniteMQTTClient::GetUnameprefix()
     */
    std::string GetUnameprefix();

    /**
     * Wrapper method for CIgniteMQTTClient::GetDeviceId()
     * @see CIgniteMQTTClient::GetDeviceId()
     */
    std::string GetDeviceId();

    /**
     * Wrapper method for CIgniteMQTTClient::OnMessageReceived()
     * @see CIgniteMQTTClient::OnMessageReceived()
     */
    bool OnMessageReceived(const std::string &rstrTopic,
                           const std::string &rstrPayload);

    /**
     * Constructor
     */
    CIgniteMQTTClientTest()
    {
        // Do nothing
    }

    /**
     * Destructor
     */
    ~CIgniteMQTTClientTest() override
    {
        // Do nothing
    }

    /**
     * Overriding Method of testing::Test class
     * @see testing::Test::SetUp()
     */
    void SetUp() override
    {
        g_pIgniteMQTTClientObj = CIgniteMQTTClient::GetInstance();
    }

    /**
     * Overriding Method of testing::Test class
     * @see testing::Test::TearDown()
     */
    void TearDown() override
    {
        g_pIgniteMQTTClientObj = NULL;
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

bool CIgniteMQTTClientTest::BroadcastConnStatus(
                            CIgniteMQTTClient::MQTT_CONNECTION_STEPS eConnState)
{
    return g_pIgniteMQTTClientObj->BroadcastConnStatus(eConnState, true);
}

std::string CIgniteMQTTClientTest::GetUnameprefix()
{
    return g_pIgniteMQTTClientObj->GetUnameprefix();
}

std::string CIgniteMQTTClientTest::GetDeviceId()
{
    return g_pIgniteMQTTClientObj->GetDeviceId();
}

bool CIgniteMQTTClientTest::OnMessageReceived(const std::string &rstrTopic,
                                              const std::string &rstrPayload)
{
    return g_pIgniteMQTTClientObj->TestOnMessageReceived(rstrTopic, 
                                                         rstrPayload);
}

// Tests
TEST_F(CIgniteMQTTClientTest, Test_getCurrentConnectionState)
{
    //Fetch connection state of MQTT
    ic_utils::Json::Value jsonMqttConnectionstate = 
                            g_pIgniteMQTTClientObj->GetCurrentConnectionState();
    ic_core::MQTT_CONNECTION_STATE connectionState = 
                       (ic_core::MQTT_CONNECTION_STATE)(jsonMqttConnectionstate[
                        "status"].asInt());

    //expecting one of the mqtt states to be returned 
    EXPECT_EQ(1,((connectionState == 
                  ic_core::MQTT_CONNECTION_STATE::eSTATE_NOT_CONNECTED) 
                  || (connectionState == 
                  ic_core::MQTT_CONNECTION_STATE::eSTATE_CONNECTION_COMPLETE)
                  || (connectionState == 
       ic_core::MQTT_CONNECTION_STATE::eSTATE_CONNECTED_BUT_TOPICS_SUBS_PENDING)
                  || (connectionState == 
                 ic_core::MQTT_CONNECTION_STATE::eSTATE_CONNECTION_TEARING_DOWN)
                )
             );
}

TEST_F(CIgniteMQTTClientTest, Test_getComponentStatus_checkCurrentMQTTState)
{
    std::string strMqttDiagInfoStr;
    strMqttDiagInfoStr = CIgniteMQTTClient::GetInstance()->GetComponentStatus();

    // string returned from the getComponentStatus should not be null or empty
    EXPECT_NE("", strMqttDiagInfoStr);
}

TEST_F(CIgniteMQTTClientTest, Test_broadcastConnStatus_disconnected)
{
    CIgniteMQTTClientTest obj;

    //sending MQTT connection status as disconnected
    CIgniteMQTTClient::MQTT_CONNECTION_STEPS eConnState =
                   CIgniteMQTTClient::MQTT_CONNECTION_STEPS::eSTEP_DISCONNECTED;
    
    //expecting true to broadcast the status
    EXPECT_TRUE(obj.BroadcastConnStatus(eConnState));
}

TEST_F(CIgniteMQTTClientTest, Test_broadcastConnStatus_connectedSubPending)
{
    CIgniteMQTTClientTest obj;

    //sending MQTT connection status as connected but subscription pending
    CIgniteMQTTClient::MQTT_CONNECTION_STEPS eConnState =
     CIgniteMQTTClient::MQTT_CONNECTION_STEPS::eSTEP_CONNECTED_BUT_SUBS_PENDING;
    
    //expecting true to broadcast the status
    EXPECT_TRUE(obj.BroadcastConnStatus(eConnState));
}

TEST_F(CIgniteMQTTClientTest, Test_broadcastConnStatus_connectedSubAckReceived)
{
    CIgniteMQTTClientTest obj;

    /* sending MQTT connection status as connected and subscription
     * acknowledgement received
     */
    CIgniteMQTTClient::MQTT_CONNECTION_STEPS eConnState =
    CIgniteMQTTClient::MQTT_CONNECTION_STEPS::eSTEP_CONNECTED_AND_SUBS_ACK_RCVD;
    
    //expecting true to broadcast the status
    EXPECT_TRUE(obj.BroadcastConnStatus(eConnState));
}

TEST_F(CIgniteMQTTClientTest, Test_broadcastConnStatus_connectionTeardown)
{
    CIgniteMQTTClientTest obj;

    //sending MQTT connection status as connection teardown
    CIgniteMQTTClient::MQTT_CONNECTION_STEPS eConnState =
        CIgniteMQTTClient::MQTT_CONNECTION_STEPS::eSTEP_CONNECTION_TEARING_DOWN;
    
    //expecting true to broadcast the status
    EXPECT_TRUE(obj.BroadcastConnStatus(eConnState));
}

TEST_F(CIgniteMQTTClientTest, Test_validatingCredentialsIndividually)
{
    /* getCredentials is a critical function hence testing the out parameters
     * clientID and username
     */
    CIgniteMQTTClientTest obj;

    //read the device id
    std::string strDeviceID = obj.GetDeviceId();

    //read the user name prefix from config
    ic_utils::Json::Value jsonMqttRoot = ic_core::CIgniteConfig::GetInstance()->
                                                         GetJsonValue("MQTT");
    std::string strUnameprefix = jsonMqttRoot["unameprefix"].asString();

    //construct the user name
    std::string strUsername = strUnameprefix + strDeviceID;

    //first, device id should be a valid one
    EXPECT_NE("",strDeviceID);

    //second, the username constructed should be same
    EXPECT_EQ((obj.GetUnameprefix()+strDeviceID), strUsername);
}

TEST_F(CIgniteMQTTClientTest, Test_OnMessageReceived)
{
    CIgniteMQTTClientTest obj;
    bool bRetVal;
    std::string strTopic = "DOJZMUTL277522/2c/ro";
    std::string strMessage = "{\"BizTransactionId\":"
    "\"2f405f20-1bf5-11ef-a2b8-97dee4353937\",\"Data\":{\"duration\":"
    "8,\"origin\":\"Cloud\",\"roRequestId\":"
    "\"2f405f20-1bf5-11ef-a2b8-97dee4353937\",\"state\":\"STARTED\",\"topic\":"
    "\"DOJZMUTL277522/2c/ro\",\"userId\":"
    "\"lakshmi215@yopmail.com\",\"vehicleArchType\":\"hu\"},\"EventID\":"
    "\"RemoteOperationEngine\",\"MessageId\":\"215001\",\"Timestamp\":"
    "1716792534562,\"Timezone\":0,\"UserContext\":[{\"role\":\"VO\",\"userId\":"
    "\"lakshmi215@yopmail.com\"}],\"Version\":\"1.1\"}";

    // Sending Valid payload , so expecting OnMessageReceived to return true 
    bRetVal = obj.OnMessageReceived(strTopic, strMessage);
    EXPECT_EQ(bRetVal, true);
}
} //namespace ic_bl
