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
#include "CZMQDeviceMessageSenderImpl.h"
#include "CIgniteClient.h"
#include <jsoncpp/json.h>

namespace ic_device
{
//Initializing ROResponse for testing purpose with randomn values
static const std::string RORESPONSE = "{\"EventID\": \
                                        \"RemoteOperationResponse\",\"Version\": \"1.1\",\"Timestamp\": 1712750234010, \
                                        \"BizTransactionId\":\"26dc72e0-fc0d-11ee-b1e1-4b9f763ec7c4\",\
                                        \"CorrelationId\":\"300028\",\"Data\": \
                                        {\"roRequestId\":\"26dc72e0-fc0d-11ee-b1e1-4b9f763ec7c4\", \
                                        \"response\":\"SUCCESS\",\"topic\":\"DOBOQAMF264378/2c/ro\"},\"Timezone\": 60}";

/**
 * Class CZMQDeviceMessageSenderImplTest defines a test feature
 * for CZMQDeviceMessageSenderImpl class
 */
class CZMQDeviceMessageSenderImplTest : public ::testing::Test
{
public:
    /**
     * Wrapper method for DispatchMessage of CZMQDeviceMessageSenderImpl class
     * @see ic_device::CZMQDeviceMessageSenderImpl::PublishMessage
     */
    static bool PublishMessage(const std::string &rstrMsg);

protected:
    /**
     * Constructor
     */
    CZMQDeviceMessageSenderImplTest()
    {
        // do nothing
    }

    /**
     * Destructor
     */
    ~CZMQDeviceMessageSenderImplTest() override
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

bool CZMQDeviceMessageSenderImplTest::PublishMessage(const std::string &rstrMsg)
{
    //Fetching IClientMessageDispatcher object
    ic_device::CClientMessageDispatcherImpl *pClientMsgDispatcher = 
                                            (ic_device::CClientMessageDispatcherImpl *)
                                            ic_core::CIgniteClient::GetClientMessageDispatcher();

    //Fetching CZMQDeviceMessageSenderImpl object
    ic_device::CZMQDeviceMessageSenderImpl *pZMQMessageSender =
                                            (ic_device::CZMQDeviceMessageSenderImpl *)
                                            pClientMsgDispatcher->GetIDeviceMessageSender();

    return pZMQMessageSender->PublishMessage(rstrMsg);
}

/**
 * 1. We cannot create object of CZMQDeviceMessageSenderImplTest as object 
 *    is already created by Client internally. If we create one more object then
 *    a new zmq sender will be created which may lead to confllict in 
 *    device communicatuion.
 * 2. UT for DeliverMessage is not written as it is taken care by DispatchMessage
 *    internally.
 */

TEST_F(CZMQDeviceMessageSenderImplTest, Test_DispatchMessage_RemoteOperationMessageID)
{
    //initializeing ROResponse
    const std::string strRoResponse = RORESPONSE;

    //Fetching IClientMessageDispatcher object
    ic_device::CClientMessageDispatcherImpl *pClientMsgDispatcher = 
                                            (ic_device::CClientMessageDispatcherImpl *)
                                            ic_core::CIgniteClient::GetClientMessageDispatcher();

    //Fetching CZMQDeviceMessageSenderImpl object
    ic_device::CZMQDeviceMessageSenderImpl *pZMQMessageSender =
                                            (ic_device::CZMQDeviceMessageSenderImpl *)
                                            pClientMsgDispatcher->GetIDeviceMessageSender();

    //Expect true as message in strRoResponse should be dispatched.
    EXPECT_TRUE(pZMQMessageSender->DispatchMessage(eRemoteOperationMessage,strRoResponse));
}

TEST_F(CZMQDeviceMessageSenderImplTest, Test_DispatchMessage_UnkownMessageID)
{
    //initializeing ROResponse
    const std::string strRoResponse = RORESPONSE;

    //Fetching IClientMessageDispatcher object
    ic_device::CClientMessageDispatcherImpl *pClientMsgDispatcher = 
                                            (ic_device::CClientMessageDispatcherImpl *)
                                            ic_core::CIgniteClient::GetClientMessageDispatcher();

    //Fetching CZMQDeviceMessageSenderImpl object
    ic_device::CZMQDeviceMessageSenderImpl *pZMQMessageSender =
                                            (ic_device::CZMQDeviceMessageSenderImpl *)
                                            pClientMsgDispatcher->GetIDeviceMessageSender();

    //Expect false as message dispatch should fail as message Id is Invalid
    EXPECT_FALSE(pZMQMessageSender->DispatchMessage(eUnkonwnMessageId,strRoResponse));
}

TEST_F(CZMQDeviceMessageSenderImplTest, Test_PublishMessage_ROMessage)
{
    //initializeing ROResponse
    const std::string strRoResponse = RORESPONSE;

    //Expect true as message in strRoResponse should be published.
    EXPECT_TRUE(PublishMessage(strRoResponse));
}

}