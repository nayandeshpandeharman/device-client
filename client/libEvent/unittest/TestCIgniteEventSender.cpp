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
#include "CIgniteEventSender.h"
#include "CIgniteMessage.h"
#include "CIgniteLog.h"
#include "CIgniteEvent.h"
#include "CIgniteDateTime.h"

// Macro for test_CIgniteEventSender string
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "test_CIgniteEventSender"

// Assigning sample data to socket port and IP address
#define SOCKETPORT 6000
static const std::string IPADDRESS = "127.0.0.1";

namespace ic_event 
{

// Class CIgniteEventSenderTest defines a test feature for CIgniteEventSender class
class CIgniteEventSenderTest : public ::testing::Test 
{
protected:
    /**
     * Constructor
     */
    CIgniteEventSenderTest()
    {
        // Do nothing
    }
 
    /**
     * Destructor
     */
    ~CIgniteEventSenderTest() override
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
};

//Test

TEST_F(CIgniteEventSenderTest, Test_EventSender_ZeroTestcase_defaultConstructor) 
{
    /* Checking the post conditions of CIgniteEventSender object for zero test
     * case as per TDD approach for a default constructor
     * Creating an object for CIgniteEventSender class
     */
    CIgniteEventSender eventSenderObj;

    // Expecting message queue id to be -1 as it is the default value
    EXPECT_EQ(eventSenderObj.GetMsqId(), -1);

    // Expecting startup events queue value to be true as it is the default value
    EXPECT_TRUE(eventSenderObj.GetQueueStartupEvents());
}

TEST_F(CIgniteEventSenderTest, Test_EventSender_ZeroTestcase_ParameterizedConstructor_forUNIXsocket)
{
    /* Checking the post conditions of CIgniteEventSender object for zero test
     * case as per TDD approach for a parameterized constructor with one parameter
     * Creating an object for CIgniteEventSender class
     */
    CIgniteEventSender eventSenderObj(HCP_MSGQUEUE_NAME);

    // Expecting message queue id to be -1 as it is the default value
    EXPECT_EQ(eventSenderObj.GetMsqId(), -1);

    // Expecting startup events queue value to be true as it is the default value
    EXPECT_TRUE(eventSenderObj.GetQueueStartupEvents());

    // Expecting socket type to be UNIXSOCKET
    EXPECT_EQ(SocketType::eUNIX_SOCKET, eventSenderObj.GetSocketType());

    // Expecting HCP_MSGQUEUE_NAME to be returned
    EXPECT_EQ(HCP_MSGQUEUE_NAME, eventSenderObj.GetSocketPath());
}

TEST_F(CIgniteEventSenderTest, Test_EventSender_ZeroTestcase_ParameterizedConstructor_IPsocket)
{
    /* Checking the post conditions of CIgniteEventSender object for zero test 
     * case as per TDD approach for a parameterized constructor with two parameters
     * Creating an object for CIgniteEventSender class
     */
    CIgniteEventSender eventSenderObj(IPADDRESS, SOCKETPORT);

    // Expecting message queue id to be -1 as it is the default value
    EXPECT_EQ(eventSenderObj.GetMsqId(), -1);

    // Expecting startup events queue value to be true as it is the default value
    EXPECT_TRUE(eventSenderObj.GetQueueStartupEvents());

    // Expecting socket type to be IPSOCKET
    EXPECT_EQ(SocketType::eIP_SOCKET, eventSenderObj.GetSocketType());

    // Expecting the returned ip_address to be same as the sample input for constructor
    EXPECT_EQ(IPADDRESS, eventSenderObj.GetIPAddress());

    // Expecting the returned port number to be same as the sample input for constructor
    EXPECT_EQ(SOCKETPORT, eventSenderObj.GetPort());
}

TEST_F(CIgniteEventSenderTest, Test_send_bySetting_socketTypeAs_unixSocket_byPassingSocketNameAsParameter)
{
    /* Checking logic of CIgniteEventSender object as one test case as per TDD approach 
     * Creating an object for CIgniteEventSender class
     * Passing socket name as a parameter
     */
    CIgniteEventSender eventSenderObj(HCP_MSGQUEUE_NAME);

    // Creating a dynamic event to send as sample data
    ic_utils::Json::Value jsonDynamicEvent = ic_utils::Json::objectValue;
    jsonDynamicEvent["EventID"] = "DynamicEvent1";
    jsonDynamicEvent["state"] = "deployed";
    unsigned long long ullTimestamp = ic_utils::CIgniteDateTime::GetCurrentTimeMs();
    jsonDynamicEvent["Timestamp"] = ullTimestamp;
    int nTimezone = ic_utils::CIgniteDateTime::GetTimeZoneOffsetMinutes();
    jsonDynamicEvent["Timezone"] = nTimezone;
    ic_utils::Json::FastWriter jsonWriter;
    std::string strSerializedEvent = jsonWriter.write(jsonDynamicEvent);

    // Expecting 0 to be returned as sending event to queue is successful with valid socket name
    EXPECT_EQ(0, eventSenderObj.Send(std::string(strSerializedEvent)));
}

TEST_F(CIgniteEventSenderTest, Test_send_bySetting_socketTypeAs_ipSocket_byPassingIPAddressAndPortAsParameters)
{
    /* Checking logic of CIgniteEventSender object as one test case as per TDD approach 
     * Creating an object for CIgniteEventSender class
     * Passing sample data for ip_address and port number as parameters
     */
    CIgniteEventSender eventSenderObj(IPADDRESS, SOCKETPORT);

    // Creating a dynamic event to send as sample data
    ic_utils::Json::Value jsonDynamicEvent = ic_utils::Json::objectValue;
    jsonDynamicEvent["EventID"] = "DynamicEvent1";
    jsonDynamicEvent["state"] = "deployed";
    unsigned long long ullTimestamp = ic_utils::CIgniteDateTime::GetCurrentTimeMs();
    jsonDynamicEvent["Timestamp"] = ullTimestamp;
    int nTimezone = ic_utils::CIgniteDateTime::GetTimeZoneOffsetMinutes();
    jsonDynamicEvent["Timezone"] = nTimezone;
    ic_utils::Json::FastWriter jsonWriter;
    std::string strSerializedEvent = jsonWriter.write(jsonDynamicEvent);

    /* Expecting -1 as the event is not delivered because both IP adrdress
     * and socket port value are sample data and hence socket connection is 
     * not created. Event is queued successfully.
     */
    EXPECT_EQ(-1, eventSenderObj.Send(strSerializedEvent));
}

}
