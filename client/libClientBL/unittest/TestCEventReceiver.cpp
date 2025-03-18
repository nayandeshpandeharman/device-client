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
#include "dam/CEventReceiver.h"
#include "CIgniteMessage.h"

namespace ic_bl 
{
/**
 * Class CEventReceiverTest defines a test feature for CEventReceiver class
 */
class CEventReceiverTest : public ::testing::Test 
{
public:
    /**
     * Constructor
     */
    CEventReceiverTest()
    {
        // do nothing
    }

    /**
     * Destructor
     */
    ~CEventReceiverTest() override
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

protected:
    /**
     * Overriding Method of testing::Test class
     * @see testing::Test::TestBody()
     */
    void TestBody() override 
    {
        // do nothing
    }
};

TEST_F(CEventReceiverTest, Test_HandleEvent)
{
    /* creating CEventReceiver instance. nullptr is used for messagequeue 
     * instance as it is not required for the scope of this test.
     */
    CEventReceiver evntRcvrObj(nullptr);

    //suspend event receiver
    evntRcvrObj.SuspendReceiver();

    //prepare an event string
    std::string strEvent = "{\"Data\":{\"Date\":\"02-07-2024\",\"Time\":"
    "\"1719916966000\"},\"UploadId\":"
    "[\"attachmentDummy1\",\"attachmentDummy2\"],\"EventID\":"
    "\"EventRecvr_UT\",\"Timestamp\":"
    "1641643200000,\"Timezone\":330,\"Version\":\"1.0\"}";

    //prepare CIgniteMessage with message type as 'eEVENT' and random connId as 1
    ic_event::CIgniteMessage ignMsgObj(ic_event::CMessageTypes::eEVENT, 1);
    ignMsgObj.SetMessage(strEvent);

    //expect false as event receiver is suspended to handle event
    EXPECT_FALSE(evntRcvrObj.Handle(ignMsgObj));

    //resume event receiver
    evntRcvrObj.ResumeReceiver();

    //expect true as event receiver is resumed to handle event
    EXPECT_TRUE(evntRcvrObj.Handle(ignMsgObj));
}
} /* namespace ic_bl*/
