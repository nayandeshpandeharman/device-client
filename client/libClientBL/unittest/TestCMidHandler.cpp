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
#include "upload/CMidHandler.h"

namespace ic_bl
{
//! Pointer to the instance of CMidHandler class
CMidHandler *g_pMidHandler = NULL;

/**
 * Class for unit testing CMidHandler
 */
class CMidHandlerTest : public ::testing::Test 
{
public:
    /**
     * Wrapper method to get instance of CMidHandler
     * @param void
     * @return Pointer to the object of CMidHandler
     */
    CMidHandler* GetInstance();

    /** 
     * Wrapper method to call GetTableOfPublishedMid of CMidHandler
     * @see CMidHandler::GetTableOfPublishedMid
     */
    std::string GetTableOfPublishedMid(int nMid);

    /**
     * Wrapper method to call ProcessPublishedMid of CMidHandler
     * @see CMidHandler::ProcessPublishedMid
     */
    bool ProcessPublishedMid(int nMid);

    /**
     * Wrapper method to call SetMidTable of CMidHandler
     * @see CMidHandler::SetMidTable
     */
    bool SetMidTable(int nMid, const std::string& rstrTable);

    /**
     * Wrapper method to call InitMid of CMidHandler
     * @see CMidHandler::InitMid
     */
    bool InitMid();

    /**
     * Constructor
     */
    CMidHandlerTest() 
    {
    }

    /**
     * Destructor
     */
    ~CMidHandlerTest() override 
    {
    }

    /**
     * Overriding Method of testing::Test class
     * @see testing::Test::SetUp()
     */
    void SetUp() override 
    {
        g_pMidHandler =  CMidHandler::GetInstance();
    }

    /**
     * Overriding Method of testing::Test class
     * @see testing::Test::SetUp()
     */
    void TearDown() override 
    {
        g_pMidHandler = NULL;
    }
protected:
    /**
     * Overriding Method of testing::Test class
     * @see testing::Test::TestBody()
     */
    void TestBody() override 
    {
    }

};

CMidHandler* CMidHandlerTest::GetInstance()
{
    return g_pMidHandler->GetInstance();
}

std::string CMidHandlerTest::GetTableOfPublishedMid(int nMid)
{
    return g_pMidHandler->GetTableOfPublishedMid(nMid);
}

bool CMidHandlerTest::ProcessPublishedMid(int nMid)
{
    return g_pMidHandler->ProcessPublishedMid(nMid);
}

bool CMidHandlerTest::SetMidTable(int nMid, const std::string& rstrTable)
{
    return g_pMidHandler->SetMidTable(nMid, rstrTable);
}

bool CMidHandlerTest::InitMid()
{
    return g_pMidHandler->InitMid();
}

//Tests

TEST_F(CMidHandlerTest, Test_getInstance) 
{
    CMidHandlerTest obj;
    EXPECT_EQ(g_pMidHandler, obj.GetInstance());
}

TEST_F(CMidHandlerTest, Test_getTableOfPublishedMid) 
{
    CMidHandlerTest obj;
    int nMid = 1;
    std::string strTableName = "handleTable";
    obj.SetMidTable(nMid,strTableName);
    int nMid2 = 2;
    std::string strTableName2 = "midTableMid";
    obj.SetMidTable(nMid2,strTableName2);
    EXPECT_STREQ(strTableName.c_str(),
                            g_pMidHandler->GetTableOfPublishedMid(nMid).c_str());
}

TEST_F(CMidHandlerTest, getTableOfPublishedMidEmptyTable) 
{
    CMidHandlerTest obj;
    int nMid = 3;
    std::string strTableName = "";
    EXPECT_STREQ(strTableName.c_str(),
                            g_pMidHandler->GetTableOfPublishedMid(nMid).c_str());
}

TEST_F(CMidHandlerTest, Test_getNotTableOfPublishedMid) 
{
    CMidHandlerTest obj;
    int nMid = 1;
    std::string strTableName = "handleTable";
    obj.ProcessPublishedMid(nMid);
    obj.SetMidTable(nMid,strTableName);
    int nMid2 = 2;
    std::string strTableName2 = "midTableMid";
    ProcessPublishedMid(nMid2);
    obj.SetMidTable(nMid2,strTableName2);
    int nMid3 = 2;   
    std::string strTableName3 = "emptyTable";
    EXPECT_STRNE(strTableName3.c_str(),
                            g_pMidHandler->GetTableOfPublishedMid(nMid3).c_str());
}

TEST_F(CMidHandlerTest, Test_processPublishedMid) 
{
    CMidHandlerTest obj;
    int nMid = 5;
    EXPECT_TRUE(obj.ProcessPublishedMid(nMid));
}

TEST_F(CMidHandlerTest, Test_processRepeatedPublishedMid)
{
    CMidHandlerTest obj;
    int nMid = 1;
    std::string strTable = "handleTable";
    obj.ProcessPublishedMid(nMid);
    obj.SetMidTable(nMid,strTable);
    int nMid1 = 1;
    std::string strTable1 = "emptyTable";
    obj.SetMidTable(nMid1,strTable1);
    EXPECT_TRUE(obj.ProcessPublishedMid(nMid1));
}

TEST_F(CMidHandlerTest, Test_setMidTable) 
{
    CMidHandlerTest obj;
    int nMid = 10;
    std::string strTable = "midTable";
    EXPECT_TRUE(obj.SetMidTable(nMid,strTable));
}

TEST_F(CMidHandlerTest, Test_setRepeatMidTable)
{
    CMidHandlerTest obj;
    int nMid = 1;
    std::string strTable = "handleTable";
    ProcessPublishedMid(nMid);
    obj.SetMidTable(nMid,strTable);
    int nMid2 = 2;
    std::string strTable2 = "midTableMid";
    ProcessPublishedMid(nMid2);
    int nMid3 = 1;
    std::string strTable3 = "emptyTable";
    ProcessPublishedMid(nMid3);
    EXPECT_TRUE(obj.SetMidTable(nMid3,strTable3));
}

TEST_F(CMidHandlerTest, Test_initMid) 
{
    CMidHandlerTest obj;
    int nMid = 7;
    std::string strTable = "handleTable";
    ProcessPublishedMid(nMid);
    obj.SetMidTable(nMid,strTable);
    int nMid2 = 8;
    std::string strTable2 = "midTable";
    ProcessPublishedMid(nMid2);
    obj.SetMidTable(nMid2,strTable2);
    EXPECT_TRUE(obj.InitMid());
}

} //namespace ic_bl
