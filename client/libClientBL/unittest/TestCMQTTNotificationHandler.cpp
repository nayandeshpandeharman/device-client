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
#include "notif/CMQTTNotificationHandler.h"
#include "notif/CNotificationListener.h"

// Macro for "test_CMQTTNotificationHandler" string
const std::string PREFIX = "test_CMQTTNotificationHandler";

namespace ic_bl 
{
/**
 * Class CMQTTNotificationHandlerTest defines a test feature
 * for CMQTTNotificationHandler class
 */
class CMQTTNotificationHandlerTest : public ::testing::Test 
{
protected:
    /**
     * Constructor
     */
    CMQTTNotificationHandlerTest()
    {
        // Do nothing
    }
 
    /**
     * Destructor
     */
    ~CMQTTNotificationHandlerTest() override
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

public:
    /**
     * Wrapper method to access the GetDomainFromTopic of CMQTTNotificationHandler
     * @param[in] rstrTopic Topic name of type Remote Operation
     * @return Domain name corresponding to topic
     */
    std::string GetDomainFromROTopic(const std::string& rstrTopic);

    /**
     * Wrapper method to access the GetDomainFromTopic of CMQTTNotificationHandler
     * @param[in] rstrTopic Invalid topic name
     * @return Domain name corresponding to topic
     */
    std::string GetDomainFromInvalidTopic(const std::string& rstrTopic);
};

std::string CMQTTNotificationHandlerTest::GetDomainFromROTopic(
                                                const std::string &rstrTopic)
{
    CNotificationListener *pNotifListner = CNotificationListener::
                                            GetNotificationListener();
    CMQTTNotificationHandler mqttNotifHandlerObj(pNotifListner);
    std::string strTopic = "DOBOQAMF264378/2c/ro";
    return mqttNotifHandlerObj.GetDomainFromTopic(strTopic); 
}

std::string CMQTTNotificationHandlerTest::GetDomainFromInvalidTopic(
                                                const std::string &rstrTopic)
{
    CNotificationListener *pNotifListner = CNotificationListener::
                                            GetNotificationListener();
    CMQTTNotificationHandler mqttNotifHandlerObj(pNotifListner);
    std::string strTopic = "DOBOQAMF264378/2c/stolen";
    return mqttNotifHandlerObj.GetDomainFromTopic(strTopic); 
}

//Tests

TEST_F(CMQTTNotificationHandlerTest, Test_parametrizedConstructor_ZeroTest)
{
    /* Checking the post conditions of CMQTTNotificationHandler object for
     * zero test case as per TDD approach for parameterized constructor
     */
    CNotificationListener *pNotifListener = CNotificationListener::
                                                GetNotificationListener();
    CMQTTNotificationHandler *pMqttNotif = new CMQTTNotificationHandler
                                                    (pNotifListener);

    //Expecting the pointer created to not be null                                                
    EXPECT_NE(nullptr, pMqttNotif);

    delete pMqttNotif;
}

TEST_F(CMQTTNotificationHandlerTest, Test_GetDomainFromTopic_roTopic)
{
    //Setting topic name for RO
    std::string strTopic = "DOBOQAMF264378/2c/ro";

    //Expecting domain name returned to be same as RemoteOperation
    EXPECT_EQ("RemoteOperation", GetDomainFromROTopic(strTopic));
}

TEST_F(CMQTTNotificationHandlerTest, Test_GetDomainFromTopic_InvalidTopic)
{
    //Setting invalid topic name
    std::string strTopic = "";

    //Expecting domain name to be empty as topic is empty or invalid
    EXPECT_EQ("", GetDomainFromInvalidTopic(strTopic));
}

}
