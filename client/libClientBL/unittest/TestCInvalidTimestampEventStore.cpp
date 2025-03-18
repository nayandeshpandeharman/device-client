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
#include "db/CInvalidTimestampEventStore.h"
#include "CIgniteLog.h"

//! Macro for CInvalidTimestampEventStoreTest string
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CInvalidTimestampEventStoreTest"

namespace ic_bl
{
//! Global variable to store instance of CInvalidTimestampEventStore
CInvalidTimestampEventStore *g_pInvalidTimestampEventStore = NULL;

/**
 * Define a test fixture for CInvalidTimestampEventStore
 */
class CInvalidTimestampEventStoreTest : public ::testing::Test 
{
public:
    /**
     * Wrapper method for GetInstance of CInvalidTimestampEventStore class
     * @see CInvalidTimestampEventStore::GetInstance()
     */
    CInvalidTimestampEventStore* GetInstance();

    /**
     * Wrapper method for InsertIntoDb of CInvalidTimestampEventStore class
     * @see CInvalidTimestampEventStore::InsertIntoDb()
     */
    long InsertIntoDb(const std::string& rstrSerialized);
    
    /**
     * Wrapper method for PurgeInvalidEvents of CInvalidTimestampEventStore class
     * @see CInvalidTimestampEventStore::PurgeInvalidEvents()
     */
    void PurgeInvalidEvents(int nEvSize)
    {
        CInvalidTimestampEventStore::GetInstance()->PurgeInvalidEvents(nEvSize);
    }

    /**
     * Constructor
     */
    CInvalidTimestampEventStoreTest() 
    {
        // do nothing
    }

    /**
     * Destructor
     */ 
    ~CInvalidTimestampEventStoreTest() override 
    {
        // do nothing
    }

    /**
     * Overriding Method of testing::Test class
     * @see testing::Test::SetUp()
     */
    void SetUp() override 
    {
        g_pInvalidTimestampEventStore = 
            CInvalidTimestampEventStore::GetInstance();
    }

    /**
     * Overriding Method of testing::Test class
     * @see testing::Test::TearDown()
     */
    void TearDown() override 
    {
        g_pInvalidTimestampEventStore = NULL;
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

CInvalidTimestampEventStore* CInvalidTimestampEventStoreTest::GetInstance()
{
    return g_pInvalidTimestampEventStore->GetInstance();
}

long CInvalidTimestampEventStoreTest::InsertIntoDb
                                             (const std::string& rstrSerialized)
{
    return g_pInvalidTimestampEventStore->InsertIntoDb(rstrSerialized);
}

//Tests
TEST_F(CInvalidTimestampEventStoreTest, Test_getInstance) 
{
    CInvalidTimestampEventStoreTest obj;
    EXPECT_EQ(g_pInvalidTimestampEventStore, obj.GetInstance());
}

TEST_F(CInvalidTimestampEventStoreTest, Test_deleteEvents)
{
    //prepare list with random rowids
    std::list<int> listRowIds = {12, 5, 10, 9};

    //expecting true as events get deleted based on rowids
    EXPECT_TRUE(CInvalidTimestampEventStore::GetInstance()->
                                                      DeleteEvents(listRowIds));
}

TEST_F(CInvalidTimestampEventStoreTest, Test_deleteEmptyEvents)
{
    std::list<int> listRowIds;

    //expecting false as no events is deleted for empty rowids
    EXPECT_FALSE(CInvalidTimestampEventStore::GetInstance()->
                                                      DeleteEvents(listRowIds));
}

TEST_F(CInvalidTimestampEventStoreTest, Test_getEventRowCount)
{
    //expecting 0 events row count to be returned during initialization of class
    EXPECT_EQ(0, CInvalidTimestampEventStore::GetInstance()->
                                                            GetEventRowCount());
}

TEST_F(CInvalidTimestampEventStoreTest, Test_insertIntoDb)
{
    CInvalidTimestampEventStoreTest obj;
    std::string strSerialized = "{\"Data\":{\"value\":0},\"EventID\":"
    "\"InvalidTimestampEventStore\",\"Timestamp\":1635853382039,\"Timezone\":"
    "330,\"Version\":\"1.0\"}";

    /* expecting new events get inserted into the database and returned 
     * insert db status
     */
    EXPECT_TRUE(obj.InsertIntoDb(strSerialized) > 0);
}

TEST_F(CInvalidTimestampEventStoreTest, Test_insertEmptyIntoDb)
{
    CInvalidTimestampEventStoreTest obj;
    std::string strSerialized = "";

    /* expecting empty events gets rejected from inserting into the database 
     * and returned insert db status
     */
    EXPECT_NE(-1, obj.InsertIntoDb(strSerialized));
}

TEST_F(CInvalidTimestampEventStoreTest, Test_insertEvents_ValidEventCase)
{
    CInvalidTimestampEventStoreTest obj;
    std::list <ic_core::CEventWrapper*> listEvents;
    std::string eventStr1 = "{\"Data\":{\"value\":0},\"EventID\":"
    "\"InvalidTimestampEventStore\",\"Timestamp\":1635853382039,\"Timezone\":"
    "330,\"Version\":\"1.0\"}";
    
    /* new CEventWrapper ptr is created as InsertEvents is expecting a list of 
     * CEventWrapper ptrs as input and releasing the memory is being taken care 
     * of inside the function body of InsertEvents in the cpp file
     */
    ic_core::CEventWrapper* pEvent1 = new ic_core::CEventWrapper();
    if(pEvent1)
    {
        pEvent1->JsonToEvent(eventStr1);
        listEvents.push_back(pEvent1);
    }
    else
    {
        HCPLOG_E << "pEvent1 is null";
    }
    std::string strEvent2 = "{\"Data\":{\"value\":1},\"EventID\":"
    "\"InvalidTimestampEventStore\",\"Timestamp\":1635853382039,\"Timezone\":"
    "330,\"Version\":\"1.0\"}";
    ic_core::CEventWrapper* pEvent2 = new ic_core::CEventWrapper();
    if(pEvent2)
    {
        pEvent2->JsonToEvent(strEvent2);
        listEvents.push_back(pEvent2);
    }
    else
    {
        HCPLOG_E << "pEvent2 is null";
    }
    std::string strEvent3 = "{\"Data\":{\"value\":0},\"EventID\":"
    "\"InvalidTimestampEventStore\",\"Timestamp\":1635853382039,\"Timezone\":"
    "330,\"Version\":\"1.0\"}";
    ic_core::CEventWrapper* pEvent3 = new ic_core::CEventWrapper();
    if(pEvent3)
    {
        pEvent3->JsonToEvent(strEvent3);
        listEvents.push_back(pEvent3);
    }
    else
    {
        HCPLOG_E << "pEvent3 is null";
    }

    //expecting true to inserted events data into db
    EXPECT_TRUE(CInvalidTimestampEventStore::GetInstance()->
                                                      InsertEvents(listEvents));
}

TEST_F(CInvalidTimestampEventStoreTest, Test_insertEvents_EmptyEventCase)
{
    std::list <ic_core::CEventWrapper*> listEvents;

    //expecting false as 'InsertEvents' will not process empty event list
    EXPECT_FALSE(CInvalidTimestampEventStore::GetInstance()->
                                                      InsertEvents(listEvents));
}

TEST_F(CInvalidTimestampEventStoreTest, Test_PurgeInvalidEvents)
{
    CInvalidTimestampEventStoreTest obj;
    int nRandEvSize = 100;//random event size

    /* calling PurgeInvalidEvents with random event size to purger event from 
     * db to generate and store 'DBOverLimit' event in local list.
     */
    obj.PurgeInvalidEvents(nRandEvSize);

    /* expecting 'DBOverLimit' event list is generated and store in list by 
     * calling PurgeInvalidEvents()
     */
    EXPECT_FALSE(CInvalidTimestampEventStore::GetInstance()->
                                             GetDBOverLimitEventList().empty());
}

TEST_F(CInvalidTimestampEventStoreTest, Test_RetriveEvents)
{
    std::list <ic_core::CEventWrapper*> listEvents;

    //preparing event list
    std::string strEvent = "{\"Data\":{\"value\":0},\"EventID\":"
    "\"InvalidTimestampEventStore\",\"Timestamp\":1635853382039,\"Timezone\":"
    "330,\"Version\":\"1.0\"}";

    ic_core::CEventWrapper *pEvent = new ic_core::CEventWrapper();
    pEvent->JsonToEvent(strEvent);
    listEvents.push_back(pEvent);
    
    //preparing next event
    strEvent = "{\"Data\":{\"value\":1},\"EventID\":"
    "\"InvalidTimestampEventStore\",\"Timestamp\":1635853382039,\"Timezone\":"
    "330,\"Version\":\"1.0\"}";

    ic_core::CEventWrapper *pEvent2 = new ic_core::CEventWrapper();
    pEvent2->JsonToEvent(strEvent);
    listEvents.push_back(pEvent2);
    
    //preparing next event
    strEvent = "{\"Data\":{\"value\":0},\"EventID\":"
    "\"InvalidTimestampEventStore\",\"Timestamp\":1635853382039,\"Timezone\":"
    "330,\"Version\":\"1.0\"}";

    ic_core::CEventWrapper *pEvent3 = new ic_core::CEventWrapper();
    pEvent3->JsonToEvent(strEvent);
    listEvents.push_back(pEvent3);

    //expecting true to inserted events data into db
    ASSERT_TRUE(CInvalidTimestampEventStore::GetInstance()->
                                                      InsertEvents(listEvents));

    int nRowsToProcess = 3;//no of rows to process
    std::list<int> listRowIDs;
    std::list<std::string> listEventData;

    /* expecting 'listRowIDs' & 'listEventData' list updated with returned value
     * while calling RetriveEvents
     */
    CInvalidTimestampEventStore::GetInstance()->RetriveEvents(nRowsToProcess,
                                                     listRowIDs, listEventData);

    //expecting non-empty 'listRowIDs' as it is updated with retrieve event ids
    ASSERT_FALSE(listRowIDs.empty());
    
    //expecting total 3 row ids returned
    EXPECT_EQ(3, listRowIDs.size());

    //expecting non-empty 'listEventData' as it is updated with retrieved data 
    ASSERT_FALSE(listEventData.empty());
}
} /* namespace ic_bl */
