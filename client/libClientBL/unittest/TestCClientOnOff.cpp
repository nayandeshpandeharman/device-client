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
#include "core/CClientOnOff.h"
#include "CIgniteThread.h"
#include "CIgniteMutex.h"
#include "IOnOffNotificationReceiver.h"
#include "IOnOff.h"

/**
 * Class CIgniteThreadTest defines a test feature for CIgniteThread class 
 * for testing CClientOnOff
 */
class CIgniteThreadTest : public ic_utils::CIgniteThread, 
                          public ic_core::IOnOffNotificationReceiver 
{
public:
    /**
     * Method to get instance of CIgniteThreadTest class
     * @param void
     * @return instance of CIgniteThreadTest class
     */
    static CIgniteThreadTest* GetInstance();

    /**
     * Wrapper method for Run of CIgniteThread class
     * @see CIgniteThread::Run()
     */
    virtual void Run();
private:
    /**
     * Constructor
     */
    CIgniteThreadTest();

    /**
     * Destructor
     */
    virtual ~CIgniteThreadTest();

    /**
     * Overridding IOnOffNotificationReceiver::NotifyShutdown method
     * @see IOnOffNotificationReceiver::NotifyShutdown()
     */
    void NotifyShutdown() override;

    //! Member variable to track the device shutdown status
    bool mShutdownRequested;
};

CIgniteThreadTest::CIgniteThreadTest()
{
    mShutdownRequested = false;
}

CIgniteThreadTest::~CIgniteThreadTest()
{
    // do nothing
}

CIgniteThreadTest* CIgniteThreadTest::GetInstance()
{
    static CIgniteThreadTest utThread;
    return &utThread;
}

void CIgniteThreadTest::NotifyShutdown()
{
    mShutdownRequested = true;
}

void CIgniteThreadTest::Run()
{
    while(!mShutdownRequested)
    {
        sleep(10);
    }
    Detach();
}

//UT
namespace ic_bl 
{
//! Global variable to store instance of CClientOnOff class
CClientOnOff *g_pClientOnOff = NULL;

/**
 * Class CClientOnOffTest defines a test feature for CClientOnOff class
 */
class CClientOnOffTest : public ::testing::Test 
{
public:
    /**
     * Wrapper method for GetInstance of CClientOnOff class
     * @see CClientOnOff::GetInstance()
     */
    CClientOnOff *GetInstance();

    /**
     * Wrapper method for RegisterForShutdownNotification of CClientOnOff class
     * @see CClientOnOff::RegisterForShutdownNotification()
     */
    bool RegisterForShutdownNotification(
                                   ic_core::IOnOffNotificationReceiver  *pRcvr,
                                   ic_core::IOnOff::NotifReceiverCode eRcvrCode,
                                   std::string strRcvrName="");
    
    /**
     * Wrapper method for UnregisterForShutdownNotification of CClientOnOff
     * @see CClientOnOff::UnregisterForShutdownNotification()
     */
    bool UnregisterForShutdownNotification(ic_core::IOnOff::NotifReceiverCode
                                       eRcvrCode, std::string strRcvrName = "");

    /**
     * Wrapper method for ReadyForShutdown of CClientOnOff class
     * @see CClientOnOff::ReadyForShutdown()
     */
    bool ReadyForShutdown(ic_core::IOnOff::NotifReceiverCode eRcvrCode,
                          std::string strRcvrName="");

    /**
     * Wrapper method for GetReceiverName of CClientOnOff class
     * @see CClientOnOff::GetReceiverName()
     */
    std::string GetReceiverName(ic_core::IOnOff::NotifReceiverCode eRcvrCode);

    /**
     * Constructor
     */
    CClientOnOffTest()
    {
        // do nothing
    }

    /**
     * Destructor
     */
    ~CClientOnOffTest() override 
    {
        // do nothing
    }

    /**
     * Overriding Method of testing::Test class
     * @see testing::Test::SetUp()
     */
    void SetUp() override 
    {
        g_pClientOnOff =  CClientOnOff::GetInstance();
    }

    /**
     * Overriding Method of testing::Test class
     * @see testing::Test::TearDown()
     */
    void TearDown() override 
    {
        g_pClientOnOff = NULL;
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

CClientOnOff* CClientOnOffTest::GetInstance()
{
    return g_pClientOnOff->GetInstance();
}

bool CClientOnOffTest::RegisterForShutdownNotification(
                                   ic_core::IOnOffNotificationReceiver *pRcvr, 
                                   ic_core::IOnOff::NotifReceiverCode eRcvrCode,
                                   std::string strRcvrName)
{
    return g_pClientOnOff->RegisterForShutdownNotification(pRcvr, eRcvrCode,
                                                           strRcvrName);
}

bool CClientOnOffTest::UnregisterForShutdownNotification(
                                   ic_core::IOnOff::NotifReceiverCode eRcvrCode,
                                   std::string strRcvrName)
{
    return g_pClientOnOff->UnregisterForShutdownNotification(eRcvrCode,
                                                             strRcvrName);
}

bool CClientOnOffTest::ReadyForShutdown(ic_core::IOnOff::NotifReceiverCode
                                        eRcvrCode, std::string strRcvrName)
{
    return g_pClientOnOff->ReadyForShutdown(eRcvrCode, strRcvrName);
}

std::string CClientOnOffTest::GetReceiverName(
                                   ic_core::IOnOff::NotifReceiverCode eRcvrCode)
{
    return g_pClientOnOff->GetReceiverName(eRcvrCode);
}

//Tests
TEST_F(CClientOnOffTest, Test_getInstance) 
{
    CClientOnOffTest obj;
    
    //Expecting the singleton instance to be returned by the API
    EXPECT_EQ(g_pClientOnOff, obj.GetInstance());
}

TEST_F(CClientOnOffTest,
     Test_registerAndUnregisterForShutdownNotification_valid_receiverTestThread)
{
    CClientOnOffTest obj;
    
    //reference to the thread class object created for testing purpose
    ic_core::IOnOffNotificationReceiver* pRcvr = 
                                               CIgniteThreadTest::GetInstance();
    ic_core::IOnOff::NotifReceiverCode eRcvrCode = 
                               ic_core::IOnOff::NotifReceiverCode::eR_UT_THREAD;
    
    //expecting true as receiver regstered successfully
    EXPECT_TRUE(obj.RegisterForShutdownNotification(pRcvr, eRcvrCode));

    //expecting true as receiver found and ready to shutdown
    EXPECT_TRUE(obj.ReadyForShutdown(eRcvrCode));

    //expecting true as receiver code is registered and available to unregister
    EXPECT_TRUE(obj.UnregisterForShutdownNotification(eRcvrCode));
}

TEST_F(CClientOnOffTest, 
          Test_registerAndUnregisterForShutdownNotification_valid_receiverOther)
{
    CClientOnOffTest obj;
    
    //reference to the thread class object created for testing purpose
    ic_core::IOnOffNotificationReceiver *pRcvr = 
                                               CIgniteThreadTest::GetInstance();
    ic_core::IOnOff::NotifReceiverCode eRcvrCode = 
                                   ic_core::IOnOff::NotifReceiverCode::eR_OTHER;
    std::string strRcvrName = "TestThread";
    
    //expecting true as receiver regstered successfully
    EXPECT_TRUE(obj.RegisterForShutdownNotification(pRcvr, eRcvrCode,
                                                    strRcvrName));

    //expecting true as receiver found and ready to shutdown
    EXPECT_TRUE(obj.ReadyForShutdown(eRcvrCode, strRcvrName));

    /* unregestering as eR_OTHER has been mapped with TestThread in this case
     *expecting true as receiver code is registered and available to unregister
     */
    EXPECT_TRUE(obj.UnregisterForShutdownNotification(eRcvrCode, strRcvrName));
}

TEST_F(CClientOnOffTest, 
         Test_registerAndUnregisterForShutdownNotification_invalid_receiverNull)
{
    CClientOnOffTest obj;
    
    //receiver object is null
    ic_core::IOnOffNotificationReceiver *pRcvr = NULL;
    ic_core::IOnOff::NotifReceiverCode eRcvrCode = 
                                   ic_core::IOnOff::NotifReceiverCode::eR_OTHER;
    std::string strRcvrName = "ThreadForTesting";
    
    //expecting false as NULL receiver reference received
    EXPECT_FALSE(obj.RegisterForShutdownNotification(pRcvr, eRcvrCode, 
                                                     strRcvrName));

    //expecting false as receiver is non-defined and not registered
    EXPECT_FALSE(obj.ReadyForShutdown(eRcvrCode, strRcvrName));
    
    /* unregestering as eR_OTHER has been mapped with TestThread1 in this case
     * expecting false as receiver code is not registered
     */
    EXPECT_FALSE(obj.UnregisterForShutdownNotification(eRcvrCode, strRcvrName));
}

TEST_F(CClientOnOffTest, Test_getReceiverName_equalStringValues)
{
    CClientOnOffTest obj;
    ic_core::IOnOff::NotifReceiverCode eRcvrCode = 
                            ic_core::IOnOff::NotifReceiverCode::eR_DB_TRANSPORT;
    
    //expecting equal as enum value is equal to the string passed
    EXPECT_STREQ("DBTransport", obj.GetReceiverName(eRcvrCode).c_str());
}

TEST_F(CClientOnOffTest, Test_getReceiverName_unequalStringValues)
{
    CClientOnOffTest obj;
    ic_core::IOnOff::NotifReceiverCode eRcvrCode = 
                   ic_core::IOnOff::NotifReceiverCode::eR_NOTIFICATION_LISTENER;
    
    //expecting not equal as enum value is not equal to the string passed
    EXPECT_STRNE("StoreAndForward", obj.GetReceiverName(eRcvrCode).c_str());
}

TEST_F(CClientOnOffTest, Test_getReceiverName_equalStringsPassed)
{
    CClientOnOffTest obj;
    ic_core::IOnOff::NotifReceiverCode eRcvrCode = 
                               ic_core::IOnOff::NotifReceiverCode::eR_UT_THREAD;
    
    //expecting equal as enum value is equal to the string passed
    EXPECT_STREQ("UnitTestThread", obj.GetReceiverName(eRcvrCode).c_str()); 
}

TEST_F(CClientOnOffTest, Test_getReceiverName_unequalStringsPassed)
{
    CClientOnOffTest obj;
    ic_core::IOnOff::NotifReceiverCode eRcvrCode = 
                                   ic_core::IOnOff::NotifReceiverCode::eR_OTHER;
    
    //expecting not equal as enum value is not equal to the string passed
    EXPECT_STRNE("HealthMonitor", obj.GetReceiverName(eRcvrCode).c_str()); 
}
}