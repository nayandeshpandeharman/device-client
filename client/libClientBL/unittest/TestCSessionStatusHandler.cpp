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
#include "dam/CSessionStatusHandler.h"
#include "CIgniteLog.h"

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "test_SessionStatusHandler"

namespace ic_bl 
{
//! Global variable to store instance of CSessionStatusHandler
CSessionStatusHandler *g_pSessionStatusHandlerObj = nullptr;

//! Define a test fixture for CSessionStatusHandler
class CSessionStatusHandlerTest : public ::testing::Test 
{
public:
    /**
     * Wrapper method for HandleEvent of CSessionStatusHandler class
     * @see CUploadController::HandleEvent()
     */
    void HandleEvent(ic_core::CEventWrapper* pEvent);

    /**
     * Wrapper method for IsAlert of CSessionStatusHandler class
     * @see CUploadController::IsAlert()
     */
    bool IsAlert(ic_core::CEventWrapper* pEvent);

    /**
     * Wrapper method for StartSession of CSessionStatusHandler class
     * @see CUploadController::StartSession()
     */
    int StartSession();
    
    /**
     * Wrapper method for EndSession of CSessionStatusHandler class
     * @see CUploadController::EndSession()
     */
    int EndSession();
    
    /**
     * Wrapper method for HandleRPMEvent of CSessionStatusHandler class
     * @see CUploadController::HandleRPMEvent()
     */
    int HandleRPMEvent(ic_core::CEventWrapper* pEvent);
    
    /**
     * Wrapper method for IsExceptionEvent of CSessionStatusHandler class
     * @see CUploadController::IsExceptionEvent()
     */
    bool IsExceptionEvent(const std::string& rstrEventID);
    
    /**
     * Wrapper method for GetSessionStatus of CSessionStatusHandler class
     * @see CUploadController::GetSessionStatus()
     */
    bool GetSessionStatus();
    
    /**
     * Wrapper method for getComponentStatus of CSessionStatusHandler class
     * @see CUploadController::GetComponentStatus()
     */
    std::string GetComponentStatus();

    /**
     * Constructor
     */
    CSessionStatusHandlerTest()
    {
        // do nothing
    }

    /**
     * Destructor
     */
    ~CSessionStatusHandlerTest()
    {
        // do nothing
    }

    /**
     * Overriding Method of testing::Test class
     * @see testing::Test::SetUp()
     */
    void SetUp() override 
    {
        /* Expect true in case of successful memory allocation, 
         * will fail in case of bad allocation
         */
        ASSERT_TRUE(InitializeObject());
    }

    /**
     * Overriding Method of testing::Test class
     * @see testing::Test::TearDown()
     */
    void TearDown() override 
    {
        delete g_pSessionStatusHandlerObj;
        g_pSessionStatusHandlerObj = nullptr;
    }

    /**
     * This method will initialize the object.
     * @param void
     * @return true, in case of successful memory allocation
     *         false, in case of bad allocation.
     */
    bool InitializeObject()
    {
        try
        {
            /* CSessionStatusHandler class is the part of event-processing-chain
             * Hence it expects the object reference of next handler to 
             * pass the processed event.
             * Since we are UTing only this class 
             * (and it's straight forward methods), We don't need to provide 
             * the next handler reference but simply sending NULL.
            */
            g_pSessionStatusHandlerObj = new CSessionStatusHandler(nullptr);
        }
        catch(std::bad_alloc& e)
        {
           HCPLOG_E << "bad_alloc caught: " << e.what();
           return false;
        }
        return true;
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

void CSessionStatusHandlerTest::HandleEvent(ic_core::CEventWrapper* pEvent)
{
    g_pSessionStatusHandlerObj->HandleEvent(pEvent);
}

bool CSessionStatusHandlerTest::IsAlert(ic_core::CEventWrapper* pEvent)
{
    return g_pSessionStatusHandlerObj->IsAlert(pEvent);
}

int CSessionStatusHandlerTest::StartSession()
{
    return g_pSessionStatusHandlerObj->StartSession();
}

int CSessionStatusHandlerTest::EndSession()
{
    return g_pSessionStatusHandlerObj->EndSession();
}

int CSessionStatusHandlerTest::HandleRPMEvent(ic_core::CEventWrapper* pEvent)
{
    return g_pSessionStatusHandlerObj->HandleRPMEvent(pEvent);
}

bool CSessionStatusHandlerTest::IsExceptionEvent(const std::string& rstrEventID)
{
    return g_pSessionStatusHandlerObj->IsExceptionEvent(rstrEventID);
}

bool CSessionStatusHandlerTest::GetSessionStatus()
{
    return g_pSessionStatusHandlerObj->GetSessionStatus();
}

std::string CSessionStatusHandlerTest::GetComponentStatus()
{
    return g_pSessionStatusHandlerObj->GetComponentStatus();
}

//Test

TEST_F(CSessionStatusHandlerTest, Test_check_SessionStatusHandler_Class_Creation) 
{
    // Checking the existance of CSessionStatusHandler class
    // nullptr is not expected as memory allocation is successful.
    EXPECT_NE(nullptr, g_pSessionStatusHandlerObj);
}

TEST_F(CSessionStatusHandlerTest, Test_handleEvent_RPM)
{
    CSessionStatusHandlerTest objSessionStatusHandler;
    
    // Using random CSessionStatusHandler event payload
    std::string strEvent = "{\"Data\":{\"value\":3200},\"EventID\":"
    "\"RPM\",\"Timestamp\":1649851984309,\"Timezone\":330,\"Version\":\"1.0\"}";
     
    // Converting the json string to event based format
    ic_core::CEventWrapper event;
    event.JsonToEvent(strEvent);
    objSessionStatusHandler.HandleEvent(&event);

    /* Expecting the API to return false as 'session' not in progress as 
     * 'state' is not defined in the payload
     */
    EXPECT_FALSE(objSessionStatusHandler.GetSessionStatus());
}

TEST_F(CSessionStatusHandlerTest, Test_handleEvent_ClientLaunched_Starting_State)
{
    CSessionStatusHandlerTest objSessionStatusHandler;

    // Using random CSessionStatusHandler event payload
    std::string strEvent = "{\"Data\":{\"state\":\"starting\"},\"EventID\":"
    "\"IgniteClientLaunched\",\"Timestamp\":1649851984309,\"Timezone\":"
    "330,\"Version\":\"1.0\"}";
    
    // Converting the json string to event based format
    ic_core::CEventWrapper event;
    event.JsonToEvent(strEvent);
    objSessionStatusHandler.HandleEvent(&event);

    /* Expecting the API to return false as 'session' not in progress as 
     * state is "starting" in the payload
     */
    EXPECT_FALSE(objSessionStatusHandler.GetSessionStatus());
}

TEST_F(CSessionStatusHandlerTest, Test_handleEvent_ClientLaunched_Shutdown_State)
{
    CSessionStatusHandlerTest objSessionStatusHandler;

    // Using random CSessionStatusHandler event payload
    std::string strEvent = "{\"Data\":{\"state\":\"shuttingdown\"},\"EventID\":"
    "\"IgniteClientLaunched\",\"Timestamp\":1649851984309,\"Timezone\":"
    "330,\"Version\":\"1.0\"}";
    
    // Converting the json string to event based format
    ic_core::CEventWrapper event;
    event.JsonToEvent(strEvent);
    objSessionStatusHandler.HandleEvent(&event);

    /* Expecting the API to return false as 'session' is not in progress as
     * 'state' is "shuttingdown" in the payload
     */
    EXPECT_FALSE(objSessionStatusHandler.GetSessionStatus());
}

TEST_F(CSessionStatusHandlerTest, Test_handleEvent_IgnStatus)
{
    CSessionStatusHandlerTest objSessionStatusHandler;

    // Using random CSessionStatusHandler event payload
    std::string strEvent = "{\"Data\":{\"state\":\"run\"},\"EventID\":"
    "\"IgnStatus\",\"Timestamp\":1554349111215,\"Timezone\":330,\"Version\":"
    "\"1.0\"}";
    
    // Converting the json string to event based format
    ic_core::CEventWrapper event;
    event.JsonToEvent(strEvent);
    objSessionStatusHandler.HandleEvent(&event);

    /* Expecting the API to return false as 'session' not in progress as 
     * "state" is "run" in the payload
     */
    EXPECT_FALSE(objSessionStatusHandler.GetSessionStatus());
}

TEST_F(CSessionStatusHandlerTest, Test_handleEvent_IgnStatus_InvalidState)
{
    CSessionStatusHandlerTest objSessionStatusHandler;

    // Using random CSessionStatusHandler event payload
    std::string strEvent = "{\"Data\":{\"state\":\"run_ABC\"},\"EventID\":"
    "\"IgnStatus\",\"Timestamp\":1554349111215,\"Timezone\":330,\"Version\":"
    "\"1.0\"}";
    
    // Converting the json string to event based format
    ic_core::CEventWrapper event;
    event.JsonToEvent(strEvent);
    objSessionStatusHandler.HandleEvent(&event);

    /* Expecting the API to return false as 'session' not in progress as state 
     * is "run_ABC" in the payload
     */
    EXPECT_EQ(objSessionStatusHandler.EndSession(),0);
}

TEST_F(CSessionStatusHandlerTest, Test_handleEvent_SessionStatus)
{
    CSessionStatusHandlerTest objSessionStatusHandler;

    //Using random CSessionStatusHandler event payload
    std::string strEvent = "{\"Data\":{\"status\":\"startup\"},\"EventID\":"
    "\"SessionStatus\",\"Timestamp\":1554349111215,\"Timezone\":"
    "330,\"Version\":\"1.0\"}";
    
    //Converting the json string to event based format
    ic_core::CEventWrapper event;
    event.JsonToEvent(strEvent);
    objSessionStatusHandler.HandleEvent(&event);

    /* Expecting the API to return true as 'session' is in progress as "state" 
     * is "startup" in the payload
     */
    EXPECT_TRUE(objSessionStatusHandler.GetSessionStatus());
}

TEST_F(CSessionStatusHandlerTest, Test_handleRPMEvent_EngineRPM)
{
    CSessionStatusHandlerTest objSessionStatusHandler;

    //Using random CSessionStatusHandler event payload
    std::string strEvent = "{\"Data\":{\"value\":1000},\"EventID\":"
    "\"EngineRPM\",\"Timestamp\":1649851984309,\"Timezone\":330,\"Version\":"
    "\"1.0\"}";
    
    //Converting the json string to event based format
    ic_core::CEventWrapper event;
    event.JsonToEvent(strEvent);

    //Expecting the API to return success as event is EngineRPM
    EXPECT_EQ(objSessionStatusHandler.HandleRPMEvent(&event), 0);
}

TEST_F(CSessionStatusHandlerTest, Test_startSession_WithZeroCount_normalStartup)
{
    CSessionStatusHandlerTest objSessionStatusHandler;

    //Expecting the API to return success as session will start
    EXPECT_EQ(0 ,objSessionStatusHandler.StartSession());
}

TEST_F(CSessionStatusHandlerTest, Test_isAlert)
{
    CSessionStatusHandlerTest objSessionStatusHandler;

    //Using random CSessionStatusHandler event payload
    std::string strEvent = "{\"BenchMode\":1,\"Data\":{},\"EventID\":"
    "\"Activation\",\"Timestamp\":1535480305551,\"Timezone\":-420,\"Version\":"
    "\"1.0\"}";
    
    //Converting the json string to event based format
    ic_core::CEventWrapper event;
    event.JsonToEvent(strEvent);

    //Expecting the API to return true as event set in the payload is an alert
    EXPECT_TRUE(objSessionStatusHandler.IsAlert(&event)); 
}

TEST_F(CSessionStatusHandlerTest, Test_isAlert_InvalidAlert)
{
    CSessionStatusHandlerTest objSessionStatusHandler;

    // Using random CSessionStatusHandler event payload
    std::string strEvent = "{\"BenchMode\":1,\"Data\":{},\"EventID\":"
    "\"FirmwareDownloaded_ABC\",\"Timestamp\":1535480305551,\"Timezone\":"
    "-420,\"Version\":\"1.0\"}";
    
    // Converting the json string to event based format
    ic_core::CEventWrapper event;
    event.JsonToEvent(strEvent);

    /* Expecting the API to return false as event set in the payload 
     * is not an alert
     */
    EXPECT_FALSE(objSessionStatusHandler.IsAlert(&event)); 
}

TEST_F(CSessionStatusHandlerTest, Test_isExceptionEvent)
{
    CSessionStatusHandlerTest objSessionStatusHandler;

    // Assigning the 'SessionStatus' event to eventID
    std::string strEventID = "SessionStatus";

    /* Expecting the API to return true as "SessionStatus" is 
     * not an exception event
     */
    EXPECT_TRUE(objSessionStatusHandler.IsExceptionEvent(strEventID));
}

TEST_F(CSessionStatusHandlerTest, Test_getComponentStatus_False)
{
    CSessionStatusHandlerTest objSessionStatusHandler;

    // Reading the session status
    std::string strStatus = objSessionStatusHandler.GetComponentStatus();

    ic_utils::Json::Value sessionStatus;
    ic_utils::Json::Reader reader;

    /* Expecting parsing to be successful. This ensures that payload is 
     * syntactically correct i.e. payload is valid JSON.
     */
    EXPECT_TRUE(reader.parse(strStatus, sessionStatus));

    // Reading the value related with the member "SessionInProgress"
    std::string strVal = sessionStatus["SessionInProgress"].asString();

    // Expecting the api to return false because session is not in progress
    EXPECT_EQ(strVal,"false");
}

TEST_F(CSessionStatusHandlerTest, Test_getComponentStatus_True)
{
    CSessionStatusHandlerTest objSessionStatusHandler;

    // Using random CSessionStatusHandler event payload
    std::string strEvent = "{\"Data\":{\"status\":\"startup\"},\"EventID\":"
    "\"SessionStatus\",\"Timestamp\":1554349111215,\"Timezone\":"
    "330,\"Version\":\"1.0\"}";
    
    /* Converting the json string to event based format and calling
     * "handleEvent" to start the session
     */
    ic_core::CEventWrapper event;
    event.JsonToEvent(strEvent);
    objSessionStatusHandler.HandleEvent(&event);

    //Reading the session status
    std::string strStatus = objSessionStatusHandler.GetComponentStatus();

    ic_utils::Json::Value sessionStatus;
    ic_utils::Json::Reader reader;

    /* Expecting parsing to be successful. This ensures that payload is 
     * syntactically correct i.e. payload is valid JSON.
     */
    EXPECT_TRUE(reader.parse(strStatus, sessionStatus));

    //Reading the value related with the member "SessionInProgress"
    std::string strVal = sessionStatus["SessionInProgress"].asString();

    //Expecting the api to return true because session is in progress
    EXPECT_EQ(strVal,"true");
}
} /* namespace ic_bl*/
