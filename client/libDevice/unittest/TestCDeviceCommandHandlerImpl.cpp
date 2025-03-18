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
#include "CDeviceCommandHandlerImpl.h"

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CDeviceCommandHandlerImpl"

namespace ic_device
{
/**
 * Class CDeviceCommandHandlerImplTest defines a test feature
 * for CDeviceCommandHandlerImpl class
 */
class CDeviceCommandHandlerImplTest : public ::testing::Test
{
protected:
    /**
     * Constructor
     */
    CDeviceCommandHandlerImplTest()
    {
        // do nothing
    }

    /**
     * Destructor
     */
    ~CDeviceCommandHandlerImplTest() override
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
 * 1. Zero test case is not required as no initialization done in constructor
 * 2. Not written UT for functions for StartListening and StopListening as 
 *    these will invoke ZMQReceiver and creates new ZMQ socket while 
 *    zmq socket is already opened.
 * 3. Not written UT for HandleShutdownNotif as this closes IC
 * 4. Not written UT for HandleActivationStatusQuery, HandleDBSizeQuery
 *    HandleMQTTConnStatusQuery, HandleROResponse as these functions already
 *    covered when we write UT for ProcessDeviceCommand functions
 */

TEST_F(CDeviceCommandHandlerImplTest, 
       Test_ProcessDeviceCommand_SendActivationQuery)
{
    //Intialize Activation Status query json in string
    const std::string strActivationQuery = 
                      "{\"EventID\":\"ActivationStatusQuery\"}";

    //Create CDeviceCommandHandlerImpl obj
    CDeviceCommandHandlerImpl objCommandHandler;

    //Expects true as ProcessDeviceCommand processes activation query
    EXPECT_TRUE(objCommandHandler.ProcessDeviceCommand(strActivationQuery));
}

TEST_F(CDeviceCommandHandlerImplTest, Test_ProcessDeviceCommand_SendDBSizeQuery)
{
    //Intialize db size query json in string
    const std::string strDbSizeQuery = "{\"EventID\":\"DBSizeQuery\"}";

    //Create CDeviceCommandHandlerImpl obj
    CDeviceCommandHandlerImpl objCommandHandler;

    //Expects true as ProcessDeviceCommand processes dbsize query
    EXPECT_TRUE(objCommandHandler.ProcessDeviceCommand(strDbSizeQuery));
}

TEST_F(CDeviceCommandHandlerImplTest, 
       Test_ProcessDeviceCommand_SendMQTTStatusQuery)
{
    //Intialize mqtt status query json in string
    const std::string strMQTTStatusQuery = 
                      "{\"EventID\":\"MQTTConnectionStatusQuery\"}";

    //Create CDeviceCommandHandlerImpl obj
    CDeviceCommandHandlerImpl objCommandHandler;

    //Expects true as ProcessDeviceCommand processes matt status query
    EXPECT_TRUE(objCommandHandler.ProcessDeviceCommand(strMQTTStatusQuery));
}

TEST_F(CDeviceCommandHandlerImplTest, Test_ProcessDeviceCommand_SendROResponse)
{
    //Intialize ro response json in string
    const std::string strRoResponse = 
    "{\"EventID\": \"RemoteOperationResponse\",\"Version\":"
    "\"1.1\",\"Timestamp\": 1712750234010,\"BizTransactionId\":"
    "\"26dc72e0-fc0d-11ee-b1e1-4b9f763ec7c4\",\"CorrelationId\":"
    "\"300028\",\"Data\": {\"roRequestId\":"
    "\"26dc72e0-fc0d-11ee-b1e1-4b9f763ec7c4\",\"response\":"
    "\"SUCCESS\",\"topic\":\"DOBOQAMF264378/2c/ro\"},\"Timezone\": 60}";

    //Create CDeviceCommandHandlerImpl obj
    CDeviceCommandHandlerImpl objCommandHandler;

    //Expects true as ProcessDeviceCommand processes ro response
    EXPECT_TRUE(objCommandHandler.ProcessDeviceCommand(strRoResponse));
}

TEST_F(CDeviceCommandHandlerImplTest, Test_ProcessDeviceCommand_SendEmptyString)
{
    //Intialize empty string
    const std::string strEmpty = "";

    //Create CDeviceCommandHandlerImpl obj
    CDeviceCommandHandlerImpl objCommandHandler;

    //Expects false as ProcessDeviceCommand does not processes empty string
    EXPECT_FALSE(objCommandHandler.ProcessDeviceCommand(strEmpty));
}

TEST_F(CDeviceCommandHandlerImplTest, 
       Test_ProcessDeviceCommand_SendInvalidEvent)
{
    //Intialize Invalid event in string
    const std::string strInvalidEvent = "{\"EventID\":\"InvalidEvent\"}";

    //Create CDeviceCommandHandlerImpl obj
    CDeviceCommandHandlerImpl objCommandHandler;

    //Expects false as ProcessDeviceCommand does not processes invalid event
    EXPECT_FALSE(objCommandHandler.ProcessDeviceCommand(strInvalidEvent));
}
}
