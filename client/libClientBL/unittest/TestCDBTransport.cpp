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
#include "dam/CDBTransport.h"
#include "db/CLocalConfig.h"
#include "upload/CUploadController.h"
#include "CIgniteLog.h"

//! Macro for key test_CDBTransport string
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "test_CDBTransport"

namespace ic_bl 
{
//! Pointer to the object of CDBTransport   
CDBTransport *g_pDBTransportObj = nullptr;

/**
 * Class for unit testing CDBTransport
 */
class CDBTransportTest : public ::testing::Test
{
public:

    /**
     * Wrapper method to call WaitForActivation of CDBTransport
     * @see CDBTransport::WaitForActivation
     */
    bool WaitForActivation(const std::string &rstrEventId);

    /**
     * Wrapper method to call InsertActivationEvents of CDBTransport
     * @see CDBTransport::InsertActivationEvents
     */
    void InsertActivationEvents(const std::string& rstrEventId);

    /**
     * Wrapper method to call SetUploadAfterActivationFlag of CDBTransport
     * @see CDBTransport::SetUploadAfterActivationFlag
     */
    void SetUploadAfterActivationFlag(bool bVal);

    /**
     * Wrapper method to call FlushCache of CDBTransport
     * @see CDBTransport::FlushCache
     */
    void FlushCache();

    /**
     * Wrapper method to call HandleEvent of CDBTransport
     * @see CDBTransport::HandleEvent
     */
    void HandleEvent(ic_core::CEventWrapper* pEvent);

    /**
     * Wrapper method to call InsertEvent of CDBTransport
     * @see CDBTransport::InsertEvent
     */
    void InsertEvent(const std::string& rstrSerialized);

    /**
     * Wrapper method to call HandleNonIgniteEvent of CDBTransport
     * @see CDBTransport::HandleNonIgniteEvent
     */
    void HandleNonIgniteEvent(ic_core::CEventWrapper* pEvent);

    /**
     * Wrapper method to call ProcessMessage of CDBTransport
     * @see CDBTransport::ProcessMessage
     */
    void ProcessMessage(const std::string& rstrSerialized);

    /**
     * Wrapper method to call InitIgnoredEvents of CDBTransport
     * @see CDBTransport::InitIgnoredEvents
     */
    void InitIgnoredEvents();

    /**
     * Wrapper method to call PurgeDB of CDBTransport
     * @see CDBTransport::PurgeDB
     */
    void PurgeDB(const size_t nDbSize);

    /**
     * Constructor
     */
    CDBTransportTest()
    {
    }

    /**
     * Destructor
     */
    ~CDBTransportTest() override
    {
    }

    /**
     * Overriding Method of testing::Test class
     * @see testing::Test::SetUp()
     */
    void SetUp() override 
    {
        ASSERT_TRUE(InitializeObject());
    }

    /**
     * Overriding Method of testing::Test class
     * @see testing::Test::TearDown()
     */
    void TearDown() override 
    {
        delete g_pDBTransportObj;
        g_pDBTransportObj = nullptr;
    }

protected:
    /**
     * Overriding Method of testing::Test class
     * @see testing::Test::TestBody()
     */
    void TestBody() override
    {
    }

    /**
     * This method will initialize the CDBTransport object.
     * @return true if CDBTransport is created else return false
     */
    bool InitializeObject()
    {
        try
        {
            /*
             * CDBTransport class is the part of event-processing-chain.
             * Hence it expects the object reference of next handler to pass
             * the processed event. Since we are Using only this class
             * (and it's straight forward methods),
             * We don't need to provide the next handler reference
             * but simply sending nullptr.
             */
            g_pDBTransportObj = new CDBTransport(nullptr);
        }
        catch(std::bad_alloc& e)
        {
           HCPLOG_E << "bad_alloc caught: " << e.what();
           return false;
        }
        return true;
    }

};

void CDBTransportTest::FlushCache()
{
    g_pDBTransportObj->FlushCache();
}

void CDBTransportTest::HandleEvent(ic_core::CEventWrapper* pEvent)
{
    g_pDBTransportObj->HandleEvent(pEvent);
}

void CDBTransportTest::InsertEvent(const std::string& rstrSerialized)
{
    g_pDBTransportObj->InsertEvent(rstrSerialized);
}

void CDBTransportTest::HandleNonIgniteEvent(ic_core::CEventWrapper* pEvent)
{
    g_pDBTransportObj->HandleNonIgniteEvent(pEvent);
}

void CDBTransportTest::ProcessMessage(const std::string& rstrSerialized)
{
    g_pDBTransportObj->ProcessMessage(rstrSerialized);
}

void CDBTransportTest::InitIgnoredEvents()
{
    g_pDBTransportObj->InitIgnoredEvents();
}

void CDBTransportTest::PurgeDB(const size_t nDbSize)
{
    g_pDBTransportObj->PurgeDB(nDbSize);
}

bool CDBTransportTest::WaitForActivation(const std::string &rstrEventId)
{
    return g_pDBTransportObj->WaitForActivation(rstrEventId);
}

void CDBTransportTest::InsertActivationEvents(const std::string& rstrEventId)
{
    g_pDBTransportObj->InsertActivationEvents(rstrEventId);
}

void CDBTransportTest::SetUploadAfterActivationFlag(bool bVal)
{
    g_pDBTransportObj->SetUploadAfterActivationFlag(bVal);
}

//Tests

TEST_F(CDBTransportTest, Test_check_DBTransport_Class_Creation) 
{
    //checking the existance of CDBTransport class
    //nullptr is not expected as memory allocation is successful.
    EXPECT_NE(nullptr,g_pDBTransportObj);
}

TEST_F(CDBTransportTest, Test_handleEvent) 
{
    CDBTransportTest objDbTransport;

    //Using random CDBTransport event payload
    std::string strEvent =  "{\"Data\":{\"active\":true},\"domain\":\
    \"EngineStatus\",\"version\":\"v1\",\"EventID\":\"EngineStatus\",\
    \"timestamp\":1662114223769}";
   
    //Converting the json string to event based format
    ic_core::CEventWrapper* pEvent = new ic_core::CEventWrapper;
    pEvent->JsonToEvent(strEvent);

    //Storing the data into the database
    objDbTransport.HandleEvent(pEvent);
    
    //Getting the db instance
    ic_core::CDataBaseFacade* pDb = ic_core::CDataBaseFacade::GetInstance();

    //Reading the size of the DB
    size_t nDbSIze = pDb->GetSize();

    /* Expects the API that pDb size should not be equal to zero as datas
     * is stored into the database.
     */
    EXPECT_NE(0,nDbSIze);
}

TEST_F(CDBTransportTest, Test_processMessage) 
{
    CDBTransportTest objDbTransport;

    //Using random CDBTransport event payload
    const std::string& rSerialized = "{\"Data\":{\"latitude\":42.2363501,\
    \"longitude\":-87.9428014,\"topic\":\"/2c/media\",\"id\":\
    \"b49b776c-fa25-11e6-bc64-92361f002671\"},\"EventID\":\
    \"IgniteClientLaunched\",\"Version\":\"1.1\",\"Timestamp\":1454674114815,\
    \"Timezone\":60}";
   
    //Processing the data
    objDbTransport.ProcessMessage(rSerialized);
    ic_core::IClientMessageDispatcher *pDispatcher = 
        ic_core::CIgniteClient::GetClientMessageDispatcher();

    //Expecting the API that dispatcher should not be null as data is processed.
    EXPECT_NE(nullptr,pDispatcher);
}

TEST_F(CDBTransportTest, Test_purgeDB)
{
    CDBTransportTest objDbTransport;

    //Flushing the cache
    objDbTransport.FlushCache();

    //Using random CDBTransport event payload
    std::string strEvent =  "{\"Data\":{\"active\":true},\"domain\":\
    \"EngineStatus\",\"version\":\"v1\",\"EventID\":\"EngineStatus\",\
    \"timestamp\":1662114223769}";
   
    //Converting the json string to event based format
    ic_core::CEventWrapper* pEvent = new ic_core::CEventWrapper;
    pEvent->JsonToEvent(strEvent);

    //Storing the data into the database
    objDbTransport.HandleEvent(pEvent);

    //Getting the db instance
    ic_core::CDataBaseFacade* pDb = ic_core::CDataBaseFacade::GetInstance();

    //Reading the size of the DB
    size_t nNewSizeDb = pDb->GetSize();

    //Purguing the DB
    objDbTransport.PurgeDB(nNewSizeDb);

    //Reading the new size of the DB
    size_t nNewSize = pDb->GetSize();

    /* Though purge is expected to reduce the size, it depends on number of
     * tables and records present in the DB before purge. If not much data is
     * found, purge may not free up any space. Also sqlite may increase the
     * db size due to it's internal operations post purge request.
     * So expecting the new size to be always lower or equal to the
     * previous size may not be correct. For testing sake,
     * check at-least if the new size is valid non-zero value.
     */
    EXPECT_GT(nNewSize,0);
}

TEST_F(CDBTransportTest, Test_waitForActivation_False) 
{
    CDBTransportTest objDbTransport;

    //Assigning the random event to strEventId
    std::string strEventId = "Activation";

    //Sending the random event
    objDbTransport.InsertActivationEvents(strEventId);

    bool bVal = true;

    //Setting the m_bUploadAfterActivation value to be true.
    objDbTransport.SetUploadAfterActivationFlag(bVal);

    /* Expects the API to return false as event found in the list and
     * no need to update.
     */
    EXPECT_FALSE(objDbTransport.WaitForActivation(strEventId));
}

TEST_F(CDBTransportTest, Test_waitForActivation_True_Case)
{
    CDBTransportTest objDbTransport;

    //Assigning the random event to strEventId
    std::string strEventId = "Activation";

    //Sending the random event
    objDbTransport.InsertActivationEvents(strEventId);

    //Assigning the random event to strEvent
    std::string strEvent = "Activation_ABC";
    ic_core::CLocalConfig* pConfig = 
        (ic_core::CLocalConfig*) ic_core::CLocalConfig::GetInstance();
    pConfig->Set("login", "");
    pConfig->Set("passcode", "");

    bool bVal = true;

    objDbTransport.SetUploadAfterActivationFlag(bVal);

    /* Expecting the API to return true as event not found in the list and
     * need to update.
     */
    EXPECT_TRUE(objDbTransport.WaitForActivation(strEvent));
}

TEST_F(CDBTransportTest, Test_waitForActivation_False_Case) 
{
    CDBTransportTest objDbTransport;

    //Assigning the random event to strEventId
    std::string strEventId = "Activation";

    //Sending the random event
    objDbTransport.InsertActivationEvents(strEventId);

    bool bVal = false;

    objDbTransport.SetUploadAfterActivationFlag(bVal);

    /* Expecting the API to return false as event found in the list and
     * no need to update.
     */
    EXPECT_FALSE(objDbTransport.WaitForActivation(strEventId));
}
} /* namespace ic_bl*/
