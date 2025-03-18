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
#include <analytics/CRemoteService.h>
#include "db/CLocalConfig.h"

#define ENGINE_EVENT_ID "EngineStatus"
#define DOOR_EVENT_ID "DoorStatus"
#define ACK_EVENT_ID "Acknowledgement"
#define SUCCESS 0

namespace ic_auto 
{
CRemoteService* mRemoteServiceObj = NULL;
class RemoteServiceTest : public ::testing::Test {
public:
    CRemoteService* getInstance();
    void ApplyConfig(ic_utils::Json::Value& configValue);
    int handleRemoteCommand(const ic_utils::Json::Value& configVal);
    int sendResponse(std::string& reqId, std::string& msgId, std::string& bizId);
    RemoteServiceTest() {
    }
    ~RemoteServiceTest() override {
    }
    void SetUp() override {
        mRemoteServiceObj = CRemoteService::GetInstance();
    }
    void TearDown() override {
        mRemoteServiceObj = NULL;
    }
protected:
    void TestBody() override {

    }

};

CRemoteService* RemoteServiceTest::getInstance()
{
    return mRemoteServiceObj->GetInstance();
}

void RemoteServiceTest::ApplyConfig(ic_utils::Json::Value& configValue)
{
    return mRemoteServiceObj->ApplyConfig(configValue);
}

int RemoteServiceTest::handleRemoteCommand(const ic_utils::Json::Value& configVal)
{
    return mRemoteServiceObj->HandleRemoteCommand(configVal);
}

int RemoteServiceTest::sendResponse(std::string& reqId, std::string& msgId, std::string& bizId)
{
    return mRemoteServiceObj->SendResponse(reqId,msgId,bizId);
}

//Tests

TEST_F(RemoteServiceTest, Test_getInstance) 
{
    RemoteServiceTest obj;
    EXPECT_EQ(mRemoteServiceObj,obj.getInstance());
}

TEST_F(RemoteServiceTest, Test_applyConfig) 
{
    RemoteServiceTest obj;
    ic_utils::Json::Value eventList = ic_utils::Json::objectValue;
    eventList["LocationRequestState"] = 1;
    eventList["EventID"] = "EngineStatus";
    eventList["timestamp"] = "1662114223769";
    ic_utils::Json::Value Data = ic_utils::Json::objectValue;
    Data["state"] = "starting";
    Data["roRequestId"] = "123234";
    Data["MessageId"] = "8b002add-4daf-4ec1-9f6e-4f7ebef993af";
    Data["BizTransactionId"] = "812e38c3-c507-488c-8bfd-dda85dc16e2d";
    eventList["Data"] = Data;
    ic_core::CLocalConfig* config = (ic_core::CLocalConfig*) ic_core::CLocalConfig::GetInstance();
    config->Set("login", "DOM0QQKD175200");
    obj.ApplyConfig(eventList);
    EXPECT_EQ(SUCCESS,obj.handleRemoteCommand(eventList));
}

TEST_F(RemoteServiceTest, Test_handleRemoteCommandWithEmptyDevice) 
{
    RemoteServiceTest obj;
    ic_utils::Json::Value eventList = ic_utils::Json::objectValue;
    eventList["LocationRequestState"] = 1;
    eventList["EventID"] = "EngineStatus";
    eventList["timestamp"] = "1662114223769";
    EXPECT_EQ(SUCCESS,obj.handleRemoteCommand(eventList));
}

TEST_F(RemoteServiceTest, Test_handleRemoteCommandWithAck) 
{
    RemoteServiceTest obj;
    ic_core::CLocalConfig* config = (ic_core::CLocalConfig*) ic_core::CLocalConfig::GetInstance();
    config->Set("login", "DOM0QQKD175200");

    ic_utils::Json::Value eventList = ic_utils::Json::objectValue;
    eventList["LocationRequestState"] = 1;
    eventList["EventID"] = ACK_EVENT_ID;
    eventList["timestamp"] = "1662114223769";
    ic_utils::Json::Value Data = ic_utils::Json::objectValue;
    Data["state"] = "starting";
    Data["roRequestId"] = "123234";
    Data["MessageId"] = "8b002add-4daf-4ec1-9f6e-4f7ebef993af";
    Data["BizTransactionId"] = "812e38c3-c507-488c-8bfd-dda85dc16e2d";
    eventList["Data"] = Data;
    EXPECT_EQ(ACK_RCVD,obj.handleRemoteCommand(eventList));
}

TEST_F(RemoteServiceTest, Test_handleRemoteCommandWithEngine) 
{
    RemoteServiceTest obj;
    ic_core::CLocalConfig* config = (ic_core::CLocalConfig*) ic_core::CLocalConfig::GetInstance();
    config->Set("login", "DOM0QQKD175200");

    ic_utils::Json::Value eventList = ic_utils::Json::objectValue;
    eventList["LocationRequestState"] = 1;
    eventList["EventID"] = ENGINE_EVENT_ID;
    eventList["timestamp"] = "1662114223769";
    ic_utils::Json::Value Data = ic_utils::Json::objectValue;
    Data["state"] = "starting";
    Data["roRequestId"] = "123234";
    Data["MessageId"] = "8b002add-4daf-4ec1-9f6e-4f7ebef993af";
    Data["BizTransactionId"] = "812e38c3-c507-488c-8bfd-dda85dc16e2d";
    eventList["Data"] = Data;
    EXPECT_EQ(SUCCESS,obj.handleRemoteCommand(eventList));
}

TEST_F(RemoteServiceTest, Test_handleRemoteCommandWithDoors) 
{
    RemoteServiceTest obj;
    ic_core::CLocalConfig* config = (ic_core::CLocalConfig*) ic_core::CLocalConfig::GetInstance();
    config->Set("login", "DOM0QQKD175200");

    ic_utils::Json::Value eventList = ic_utils::Json::objectValue;
    eventList["LocationRequestState"] = 1;
    eventList["EventID"] = DOOR_EVENT_ID;
    eventList["timestamp"] = "1662114223769";
    ic_utils::Json::Value Data = ic_utils::Json::objectValue;
    Data["state"] = "starting";
    Data["roRequestId"] = "123234";
    Data["MessageId"] = "8b002add-4daf-4ec1-9f6e-4f7ebef993af";
    Data["BizTransactionId"] = "812e38c3-c507-488c-8bfd-dda85dc16e2d";
    eventList["Data"] = Data;
    EXPECT_EQ(SUCCESS,obj.handleRemoteCommand(eventList));
}

TEST_F(RemoteServiceTest, Test_sendResponse) 
{
    RemoteServiceTest obj;
    std::string roRequestId = "123234";
    std::string msgId = "8b002add-4daf-4ec1-9f6e-4f7ebef993af";
    std::string bizTxnId = "812e38c3-c507-488c-8bfd-dda85dc16e2d";
    EXPECT_EQ(SUCCESS,obj.sendResponse(roRequestId,msgId,bizTxnId));
}

}