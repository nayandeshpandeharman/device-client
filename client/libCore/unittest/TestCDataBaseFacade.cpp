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

#include <unistd.h>
#include <string.h>
#include "gtest/gtest.h"
#include "db/CDataBaseFacade.h"
#include "crypto/CIgniteDataSecurity.h"
#include "core/CKeyGenerator.h"
#include "core/CAesSeed.h"
#include "dam/CEventWrapper.h"
#include "CIgniteStringUtils.h"

namespace ic_core
{
/**
 * Class CDataBaseFacadeTest defines a test feature for CDataBaseFacade class
 */
class CDataBaseFacadeTest : public ::testing::Test 
{
protected:
   /**
    * Constructor
    */
   CDataBaseFacadeTest () 
   {
      // Do nothing
   }

   /**
    * Destructor
    */ 
   ~CDataBaseFacadeTest () override 
   {
      // Do nothing
   }

   /**
    * Overriding Method of testing::Test class
    * @see testing::Test::SetUp()
    */
   void SetUp() override 
   {
      // Do nothing
   }

   /**
    * Overriding Method of testing::Test class
    * @see testing::Test::SetUp()
    */
   void TearDown() override 
   {
      // Do nothing
   }

};

// Tests

TEST_F(CDataBaseFacadeTest, Test_ZeroTest)
{
   // Check the existance of CDataBaseFacadeTest class as Zero test case
   EXPECT_NE(CDataBaseFacade::GetInstance(), nullptr);
}

/* Following test case is to get the current state of Db ,
 * this is called before any test case so that snapshot is clear
 */  
TEST_F(CDataBaseFacadeTest, Test_GetComponentStatus_checkCurrentDbState)
{
   std::string strDbDiagInfo;
   /* get the component status related to diagnostics information 
    * from the database
    */ 
   strDbDiagInfo = CDataBaseFacade::GetInstance()->GetComponentStatus();

   HCPLOG_D << strDbDiagInfo;

   // string returned from the GetComponentStatus should not be null or empty
   EXPECT_NE("", strDbDiagInfo);
}

TEST_F(CDataBaseFacadeTest, Test_GetRowCountOfTable_checkRowCountOfValidTable)
{
   int nCount = -1;
   // get the row count of the table form the database
   nCount = CDataBaseFacade::GetInstance()->
            GetRowCountOfTable(CDataBaseConst::TABLE_EVENT_STORE);

   HCPLOG_D << "Row Count of Event Table "<< nCount;
   /* Expect no rows (0) or some count if the GetRowCountOfTable is successful,
    * GetRowCountOfTable returns -1 in case table doesnt exists 
    * of any otehr sqlite issue
    */ 
   EXPECT_NE(-1, nCount);
}

TEST_F(CDataBaseFacadeTest, Test_GetRowCountOfTable_checkRowCountOfInValidTable)
{
   int nCount = -1;
   /* There is no table called INVALID_STORE ,
    * try to get the row count for this table
    */  
   nCount = CDataBaseFacade::GetInstance()->GetRowCountOfTable("INVALID_STORE");

   /* GetRowCountOfTable returns -1 in case table doesnt exists
    * of any otehr sqlite issue
    */  
   EXPECT_EQ(-1, nCount);
}

TEST_F(CDataBaseFacadeTest , Test_ExecuteCommand_Positive) 
{
   std::string strSQL = "DROP TABLE EVENT_STORE;";
   // execute given DB command which is valid, hence expect SQLITE_OK
   EXPECT_EQ(SQLITE_OK, CDataBaseFacade::GetInstance()->ExecuteCommand(strSQL));
}

TEST_F(CDataBaseFacadeTest , Test_ExecuteCommand_Negative) 
{
   std::string strSQL = "";
   // execute given DB command which is invalid, hence expect SQLITE_OK
   EXPECT_NE(SQLITE_OK, CDataBaseFacade::GetInstance()->ExecuteCommand(strSQL));
}

TEST_F(CDataBaseFacadeTest , Test_Insert) 
{
   CContentValues data;
   data.Put(CDataBaseConst::COL_KEY_VAL, "key-test");
   data.Put(CDataBaseConst::COL_VALUE, "value-test");

   // insert data in db, expect the row's id to be returned which must not be 0
   EXPECT_NE(0, CDataBaseFacade::GetInstance()->
             Insert(CDataBaseConst::TABLE_LOCAL_CONFIG, &data));
}

TEST_F(CDataBaseFacadeTest , Test_Insert_1) 
{
   CEventWrapper event;
   std::string strSerialized = "{\"Data\":{},\"EventID\":\"ActivationStateRequest\",\"Timestamp\":1646444909901,\"Timezone\":330,\"Version\":\"1.0\"}";
   event.JsonToEvent(strSerialized);

   long long llTimestamp= event.GetTimestamp();
   CContentValues data;
   data.Put(CDataBaseConst::COL_TIMESTAMP, llTimestamp);
   data.Put(CDataBaseConst::COL_EVENTS, strSerialized);

   // insert data in db, expect the row's id to be returned which must not be 0
   EXPECT_NE(0, CDataBaseFacade::GetInstance()->
             Insert(CDataBaseConst::TABLE_INVALID_EVENT_STORE, &data));
}

TEST_F(CDataBaseFacadeTest , Test_Insert_Negative_1) 
{
   CContentValues data;
   std::string strSerialized = "{\"Data\":{},\"EventID\":\"ActivationStateRequest\",\"Timestamp\":1646444145620,\"Timezone\":330,\"Version\":\"1.0\"}";
   data.Put(CDataBaseConst::COL_TIMESTAMP, 164644);
   data.Put(CDataBaseConst::COL_EVENTS, strSerialized);

   // insert data in db, expect Insert() to fail as table name is empty
   EXPECT_EQ(eINVALID_OPERATION, CDataBaseFacade::GetInstance()->
                                                  Insert(" ", &data));
}

TEST_F(CDataBaseFacadeTest , Test_Insert_Negative_2) 
{
   CContentValues data;
   // insert data in db, expect Insert() to fail as invalid data
   EXPECT_EQ(eINVALID_OPERATION, CDataBaseFacade::GetInstance()->
             Insert(" ",NULL));
}

TEST_F(CDataBaseFacadeTest , Test_Query_Negative_1) 
{
   std::vector<std::string> vectProjection;
   vectProjection.push_back("COUNT(*)");
   //  query data from the database, expect NULL as table name is empty
   EXPECT_EQ(NULL, CDataBaseFacade::GetInstance()->Query(" ",vectProjection));
}

TEST_F(CDataBaseFacadeTest , Test_Update_Positive) 
{
   CContentValues data;
   data.Put(CDataBaseConst::COL_MID, 0);
   // update data in database, expect true for valid inputs
   EXPECT_EQ(true, CDataBaseFacade::GetInstance()->
            Update(CDataBaseConst::TABLE_ALERT_STORE, &data,
                   CDataBaseConst::COL_MID + "!=0"));
}

TEST_F(CDataBaseFacadeTest , Test_Update_Negative) 
{
   // update data in database, expect true for invalid inputs
   EXPECT_EQ(false, CDataBaseFacade::GetInstance()->Update(" ",NULL,""));
}

TEST_F(CDataBaseFacadeTest , Test_StartTransaction) 
{
   // start transaction, expect to succeed
   EXPECT_EQ(true, CDataBaseFacade::GetInstance()->StartTransaction());
   CDataBaseFacade::GetInstance()->EndTransaction(true);
}

TEST_F(CDataBaseFacadeTest , Test_EndTransaction) 
{
   CDataBaseFacade::GetInstance()->StartTransaction();
   // end transaction, expect to succeed
   EXPECT_EQ(SQLITE_OK, CDataBaseFacade::GetInstance()->EndTransaction(true));
}

TEST_F(CDataBaseFacadeTest , Test_GetSize) 
{
   // Test the GetSize()
   EXPECT_NE(-1, CDataBaseFacade::GetInstance()->GetSize());  
}

TEST_F(CDataBaseFacadeTest , Test_removeNegative_1) 
{
   // Remove data from database functionality, expect fasle for invalid input
   EXPECT_EQ(false, CDataBaseFacade::GetInstance()->Remove(" ",""));
}

TEST_F(CDataBaseFacadeTest , Test_ClearTables) 
{
   // Test the clear tables from database functionality
   EXPECT_EQ(SQLITE_OK, CDataBaseFacade::GetInstance()->ClearTables());
}

/*
 * UT is not written for ResetDatabase() API as it should not be done
 * in the middle of the application.
 */
}
