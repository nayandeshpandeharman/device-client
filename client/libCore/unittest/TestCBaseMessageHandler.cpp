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
#include "analytics/CBaseMessageHandler.h"

namespace ic_core
{
/**
 * Class CBaseMessageHandlerTest defines a test feature for 
 * CBaseMessageHandler class
 */
class CBaseMessageHandlerTest : public ::testing::Test 
{
public:
   /**
    * Constructor
    */
   CBaseMessageHandlerTest () 
   {
      // Do nothing
   }

   /**
    * Destructor
    */ 
   ~CBaseMessageHandlerTest () override 
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

   bool ProcessNotificationTypeMessage(const IMessageHandler::MsgPayload 
      &rstMsgPayload);

   void AddEventDomainReferenceToMap(
      ic_utils::Json::Value &rjsonEventDomainMap,
      const std::string &rstrDomain, const std::string &rstrEventName);

   void RemoveEventDomainReferenceFromMap(
      ic_utils::Json::Value &rjsonEventDomainMap,
      const std::string &rstrDomain, const std::string &rstrEventName);

    void UpdateEventDomainMapForArrayType(
      ic_utils::Json::Value &rjsonUpdatedEventDomainMap,
      const std::string &rstrDomain,
      const ic_utils::Json::Value &rjsonCurrDomainSec,
      const ic_utils::Json::Value &rjsonNewDomainSec);

    void UpdateEventDomainMapForStringType(
         ic_utils::Json::Value &rjsonUpdatedEventDomainMap,
         const std::string &rstrDomain,
         const ic_utils::Json::Value &rjsonCurrDomainSec,
         const ic_utils::Json::Value &rjsonNewDomainSec);

protected:
    void TestBody() override 
    {
        //empty body
    }
};

bool CBaseMessageHandlerTest::ProcessNotificationTypeMessage(
   const IMessageHandler::MsgPayload &rstMsgPayload)
{
   CBaseMessageHandler bmhInstance;
   return bmhInstance.ProcessNotificationTypeMessage(rstMsgPayload);
}

void CBaseMessageHandlerTest::AddEventDomainReferenceToMap(
   ic_utils::Json::Value &rjsonEventDomainMap,
   const std::string &rstrDomain, const std::string &rstrEventName)
{
   CBaseMessageHandler bmhInstance;
   bmhInstance.AddEventDomainReferenceToMap(rjsonEventDomainMap,rstrDomain,
      rstrEventName);
}

void CBaseMessageHandlerTest::RemoveEventDomainReferenceFromMap(
   ic_utils::Json::Value &rjsonEventDomainMap,
   const std::string &rstrDomain, const std::string &rstrEventName)
{
   CBaseMessageHandler bmhInstance;
   bmhInstance.RemoveEventDomainReferenceFromMap(rjsonEventDomainMap,rstrDomain,
      rstrEventName);
}

void CBaseMessageHandlerTest::UpdateEventDomainMapForArrayType(
   ic_utils::Json::Value &rjsonUpdatedEventDomainMap,
   const std::string &rstrDomain,
   const ic_utils::Json::Value &rjsonCurrDomainSec,
   const ic_utils::Json::Value &rjsonNewDomainSec)
{
   CBaseMessageHandler bmhInstance;
   bmhInstance.UpdateEventDomainMapForArrayType(rjsonUpdatedEventDomainMap,
      rstrDomain,rjsonCurrDomainSec,rjsonNewDomainSec);
}

void CBaseMessageHandlerTest::UpdateEventDomainMapForStringType(
   ic_utils::Json::Value &rjsonUpdatedEventDomainMap,
   const std::string &rstrDomain,
   const ic_utils::Json::Value &rjsonCurrDomainSec,
   const ic_utils::Json::Value &rjsonNewDomainSec)
{
   CBaseMessageHandler bmhInstance;
   bmhInstance.UpdateEventDomainMapForStringType(rjsonUpdatedEventDomainMap,
      rstrDomain,rjsonCurrDomainSec,rjsonNewDomainSec);
}
// Tests 

TEST_F(CBaseMessageHandlerTest, Test_IsHandlerSubscribedForEvent) 
{
   CBaseMessageHandler bmhInstance;
   // Expect false as eventId is dummy 
   EXPECT_FALSE(bmhInstance.IsHandlerSubscribedForEvent("dummy"));
}

TEST_F(CBaseMessageHandlerTest, Test_IsHandlerSubscribedForNotification) 
{
   CBaseMessageHandler bmhInstance;
   // Expect false as notification is dummy 
   EXPECT_FALSE(bmhInstance.IsHandlerSubscribedForNotification("dummy"));
}

TEST_F(CBaseMessageHandlerTest,
   Test_ProcessNotificationTypeMessage_InvalidDomain) 
{

   ic_utils::Json::Value notifPayload;

   notifPayload["domain"] = "TestValue"; //Invalid domain name
   notifPayload["notif"] = "TestValue";

   ic_utils::Json::Value jsonTemp;
   jsonTemp["domain"] = "TestValue";
   jsonTemp["notif"] = notifPayload;

   ic_core::IMessageHandler::MsgPayload payload;
   payload.eType = ic_core::IMessageHandler::eMSG_TYPE_NOTIFICATION;
   ic_utils::Json::FastWriter writer;
   payload.strPayloadJson = writer.write(jsonTemp);

   CBaseMessageHandlerTest bmhTestInstance;
   // Expect false as domain of the notification is invalid 
   EXPECT_FALSE(bmhTestInstance.ProcessNotificationTypeMessage(payload));
}

TEST_F(CBaseMessageHandlerTest,
   Test_ProcessNotificationTypeMessage_InvalidPayload) 
{
   ic_core::IMessageHandler::MsgPayload payload;
   payload.eType = ic_core::IMessageHandler::eMSG_TYPE_NOTIFICATION;
   payload.strPayloadJson = "RandomPayload";

   CBaseMessageHandlerTest bmhTestInstance;

   // Expect false as notification is invalid 
   EXPECT_FALSE(bmhTestInstance.ProcessNotificationTypeMessage(payload));
}

TEST_F(CBaseMessageHandlerTest,
   Test_ProcessNotificationTypeMessage_DomainMissing) 
{
   ic_utils::Json::Value jsonTemp;
   jsonTemp["domain"] = "RemoteOperation";
   jsonTemp["notif"] = "TestValue";

   ic_core::IMessageHandler::MsgPayload payload;
   payload.eType = ic_core::IMessageHandler::eMSG_TYPE_NOTIFICATION;
   ic_utils::Json::FastWriter writer;
   payload.strPayloadJson = writer.write(jsonTemp);

   CBaseMessageHandlerTest bmhTestInstance;

   // Expect false as domain missing in notic field
   EXPECT_FALSE(bmhTestInstance.ProcessNotificationTypeMessage(payload));
}

TEST_F(CBaseMessageHandlerTest, Test_AddEventDomainReferenceToMap_validCases) 
{
   CBaseMessageHandlerTest bmhTestInstance;
   ic_utils::Json::Value jsonEventDomainMap;
   bmhTestInstance.AddEventDomainReferenceToMap(jsonEventDomainMap,
      "DummyDomain","DummyEvent");
   /* Expect size of jsonEventDomainMap 1, as "DummyDomain" is added to key 
    * which is event "DummyEvent"
   */
   EXPECT_EQ(1,jsonEventDomainMap.size());

   bmhTestInstance.AddEventDomainReferenceToMap(jsonEventDomainMap,
      "DummyDomain1","DummyEvent");
   /* Expect size of jsonEventDomainMap 1, as "DummyDomain1" is added to key 
    * which is event "DummyEvent"
   */
   EXPECT_EQ(1,jsonEventDomainMap.size());

   bmhTestInstance.AddEventDomainReferenceToMap(jsonEventDomainMap,
      "DummyDomain2","DummyEvent");

   /* Expect size of jsonEventDomainMap 1, as "DummyDomain2" is added to key 
    * which is event "DummyEvent"     
   */
   EXPECT_EQ(1,jsonEventDomainMap.size());

   bmhTestInstance.AddEventDomainReferenceToMap(jsonEventDomainMap,
      "DummyDomain","DummyEvent1");

   /* Expect size of jsonEventDomainMap 2, as "DummyDomain" is added to key 
    * which is event "DummyEvent1"
   */
   EXPECT_EQ(2,jsonEventDomainMap.size());
}

TEST_F(CBaseMessageHandlerTest, 
   Test_RemoveEventDomainReferenceFromMap_validCases) 
{
   CBaseMessageHandlerTest bmhTestInstance;

   //Adding entries into jsonEventDomainMap
   ic_utils::Json::Value jsonEventDomainMap;
   bmhTestInstance.AddEventDomainReferenceToMap(jsonEventDomainMap,
      "DummyDomain","DummyEvent");
   bmhTestInstance.AddEventDomainReferenceToMap(jsonEventDomainMap,
      "DummyDomain","DummyEvent1");


   //removing one entry
   bmhTestInstance.RemoveEventDomainReferenceFromMap(jsonEventDomainMap,
      "DummyDomain","DummyEvent");

   /* Expect size of jsonEventDomainMap 0, as pair of "DummyEvent" as key and 
    * "DummyDomain" as value is removed.
   */
   EXPECT_EQ(1,jsonEventDomainMap.size());

   //removing one entry
   bmhTestInstance.RemoveEventDomainReferenceFromMap(jsonEventDomainMap,
      "DummyDomain","DummyEvent1");

   /* Expect size of jsonEventDomainMap 0, as pair of "DummyEvent1" as key and 
    * "DummyDomain" as value is removed.
   */
   EXPECT_EQ(0,jsonEventDomainMap.size());
}

TEST_F(CBaseMessageHandlerTest, 
   Test_UpdateEventDomainMapForArrayType_newDomainEventIsArray) 
{
   ic_utils::Json::Reader reader;
   ic_utils::Json::FastWriter writer;

   /*
   "EventDomainMap": {
      "dummyEvent1": ["dummyDomain2","dummyDomain3"],
      "dummyEvent2": "dummyDomain3",
      "dummyEvent3": "dummyDomain3",
      "dummyEvent4": "dummyDomain3"
   }
   */

   //Input 1
   std::string strEventDomainMap = "{\"dummyEvent1\":[\"dummyDomain2\",\
   \"dummyDomain3\"],\"dummyEvent2\":\"dummyDomain3\",\
   \"dummyEvent3\":\"dummyDomain3\",\"dummyEvent4\":\"dummyDomain3\"}";
   
   ic_utils::Json::Value jsonUpdatedEventDomainMap;
   ASSERT_TRUE(reader.parse(strEventDomainMap, jsonUpdatedEventDomainMap));

   ic_utils::Json::Value jsonTemp;

   /*Existing mapping for "dummyDomain3" from domainEventMap config with 
    * "dummyEvent4"
    */
   //Input 2
   std::string strCurrDomainEventMap = "{\"dummyDomain3\":[\"dummyEvent1\",\
   \"dummyEvent2\",\"dummyEvent3\",\"dummyEvent4\"]}";
   ASSERT_TRUE(reader.parse(strCurrDomainEventMap, jsonTemp));

   ic_utils::Json::Value jsonArrayCurrDomainSec = jsonTemp["dummyDomain3"];

   strCurrDomainEventMap = "";
   jsonTemp.clear();

   /* New mapping for"dummyDomain3" from domainEventMap config update w/o 
    * "dummyEvent4"
    */
   //Input 3
   strCurrDomainEventMap = "{\"dummyDomain3\":[\"dummyEvent1\",\"dummyEvent2\",\
   \"dummyEvent3\"]}";
   ASSERT_TRUE(reader.parse(strCurrDomainEventMap, jsonTemp));

   ic_utils::Json::Value jsonNewDomainSec = jsonTemp["dummyDomain3"];


   CBaseMessageHandlerTest bmhTestInstance;
   //Update new mapping for "dummyDomain3" in EventDomainMap
   bmhTestInstance.UpdateEventDomainMapForArrayType(jsonUpdatedEventDomainMap,
      "dummyDomain3",jsonArrayCurrDomainSec,jsonNewDomainSec);

   strCurrDomainEventMap = "{\"dummyEvent1\":[\"dummyDomain2\",\
   \"dummyDomain3\"],\"dummyEvent2\":\"dummyDomain3\",\"dummyEvent3\":\
   \"dummyDomain3\"}";
   ASSERT_TRUE(reader.parse(strCurrDomainEventMap, jsonTemp));

   //Expecting new mapping i.e.removing entry of "dummyEvent4" for"dummyDomain3"
   EXPECT_EQ(jsonUpdatedEventDomainMap,jsonTemp);
}

TEST_F(CBaseMessageHandlerTest, 
   Test_UpdateEventDomainMapForArrayType_newDomainEventIsString) 
{
   ic_utils::Json::Reader reader;
   ic_utils::Json::FastWriter writer;

   /*
   "EventDomainMap": {
      "dummyEvent1": ["dummyDomain2","dummyDomain3"],
      "dummyEvent2": "dummyDomain3",
      "dummyEvent3": "dummyDomain3",
      "dummyEvent4": "dummyDomain3"
   }
   */

   //Input 1
   std::string strEventDomainMap = "{\"dummyEvent1\":[\"dummyDomain2\",\
   \"dummyDomain3\"],\"dummyEvent2\":\"dummyDomain3\",\"dummyEvent3\":\
   \"dummyDomain3\",\"dummyEvent4\":\"dummyDomain3\"}";
   
   ic_utils::Json::Value jsonUpdatedEventDomainMap;
   ASSERT_TRUE(reader.parse(strEventDomainMap, jsonUpdatedEventDomainMap));

   ic_utils::Json::Value jsonTemp;

   /* Existing mapping for "dummyDomain3" from domainEventMap config with 
    * "dummyEvent4"
    */
   //Input 2
   std::string strCurrDomainEventMap = "{\"dummyDomain3\":[\"dummyEvent1\",\
   \"dummyEvent2\",\"dummyEvent3\",\"dummyEvent4\"]}";
   ASSERT_TRUE(reader.parse(strCurrDomainEventMap, jsonTemp));

   ic_utils::Json::Value jsonArrayCurrDomainSec = jsonTemp["dummyDomain3"];

   strCurrDomainEventMap = "";
   jsonTemp.clear();

   /* New mapping for"dummyDomain3" from domainEventMap config update w/o
    * "dummyEvent4"
    */
   
   //Input 3
   strCurrDomainEventMap = "{\"dummyDomain3\":\"dummyEvent1\"}";
   ASSERT_TRUE(reader.parse(strCurrDomainEventMap, jsonTemp));

   ic_utils::Json::Value jsonNewDomainSec = jsonTemp["dummyDomain3"];


   CBaseMessageHandlerTest bmhTestInstance;
   //Update new mapping for "dummyDomain3" in EventDomainMap
   bmhTestInstance.UpdateEventDomainMapForArrayType(jsonUpdatedEventDomainMap,
      "dummyDomain3",jsonArrayCurrDomainSec,jsonNewDomainSec);

   strCurrDomainEventMap = "{\"dummyEvent1\":[\"dummyDomain2\",\
   \"dummyDomain3\"]}";
   ASSERT_TRUE(reader.parse(strCurrDomainEventMap, jsonTemp));

   /*Expecting new mapping i.e.removing entry of "dummyEvent2", "dummyEvent3", 
    * "dummyEvent4" for "dummyDomain3"
    */
   EXPECT_EQ(jsonUpdatedEventDomainMap,jsonTemp);
}


TEST_F(CBaseMessageHandlerTest, 
   Test_UpdateEventDomainMapForStringType_newDomainEventIsArray) 
{
   ic_utils::Json::Reader reader;
   ic_utils::Json::FastWriter writer;
/*
   "EventDomainMap": {
      "dummyEvent1": "dummyDomain1",
      "dummyEvent2": "dummyDomain2",
      "dummyEvent3": "dummyDomain3",
      "dummyEvent4": "dummyDomain4"
   }
*/

   //Input 1
   std::string strEventDomainMap = "{\"dummyEvent1\":\"dummyDomain1\",\
   \"dummyEvent2\":\"dummyDomain2\",\"dummyEvent3\":\"dummyDomain3\",\
   \"dummyEvent4\":\"dummyDomain4\"}";
   
   ic_utils::Json::Value jsonUpdatedEventDomainMap;
   ASSERT_TRUE(reader.parse(strEventDomainMap, jsonUpdatedEventDomainMap));

   ic_utils::Json::Value jsonTemp;

   /* Existing mapping for "dummyDomain3" from domainEventMap config with 
    * "dummyEvent3" string
    */
   //Input 2
   std::string strCurrDomainEventMap = "{\"dummyDomain3\":\"dummyEvent3\"}";
   ASSERT_TRUE(reader.parse(strCurrDomainEventMap, jsonTemp));

   ic_utils::Json::Value jsonArrayCurrDomainSec = jsonTemp["dummyDomain3"];

   strCurrDomainEventMap = "";
   jsonTemp.clear();

   /*New mapping for"dummyDomain3" from domainEventMap config update with 
    * array of events - "dummyEvent3" and "dummyEvent3" 
    */
   //Input 3
   strCurrDomainEventMap = "{\"dummyDomain3\":[\"dummyEvent3\",\
   \"dummyEvent5\"]}";
   ASSERT_TRUE(reader.parse(strCurrDomainEventMap, jsonTemp));

   ic_utils::Json::Value jsonNewDomainSec = jsonTemp["dummyDomain3"];


   CBaseMessageHandlerTest bmhTestInstance;
   //Update new mapping for "dummyDomain3" in EventDomainMap
   bmhTestInstance.UpdateEventDomainMapForStringType(jsonUpdatedEventDomainMap,
      "dummyDomain3",jsonArrayCurrDomainSec,jsonNewDomainSec);

   strCurrDomainEventMap = "{\"dummyEvent1\":\"dummyDomain1\",\
   \"dummyEvent2\":\"dummyDomain2\",\"dummyEvent3\":\"dummyDomain3\",\
   \"dummyEvent4\":\"dummyDomain4\",\"dummyEvent5\":\"dummyDomain3\"}";
   ASSERT_TRUE(reader.parse(strCurrDomainEventMap, jsonTemp));

   //Expecting new mapping with "dummyEvent5" for "dummyDomain3"
   EXPECT_EQ(jsonUpdatedEventDomainMap,jsonTemp);
}

TEST_F(CBaseMessageHandlerTest, 
   Test_UpdateEventDomainMapForStringType_newDomainEventIsString) 
{
   ic_utils::Json::Reader reader;
   ic_utils::Json::FastWriter writer;

   /*
   "EventDomainMap": {
      "dummyEvent1": "dummyDomain1",
      "dummyEvent2": "dummyDomain2",
      "dummyEvent3": "dummyDomain3",
      "dummyEvent4": "dummyDomain4"
   }
   */ 

   //Input 1
   std::string strEventDomainMap = "{\"dummyEvent1\":\"dummyDomain1\",\
   \"dummyEvent2\":\"dummyDomain2\",\"dummyEvent3\":\"dummyDomain3\",\
   \"dummyEvent4\":\"dummyDomain4\"}";
   
   ic_utils::Json::Value jsonUpdatedEventDomainMap;
   ASSERT_TRUE(reader.parse(strEventDomainMap, jsonUpdatedEventDomainMap));

   ic_utils::Json::Value jsonTemp;

   /*Existing mapping for "dummyDomain3" from domainEventMap config with 
    *"dummyEvent3" string
    */
   //Input 2
   std::string strCurrDomainEventMap = "{\"dummyDomain3\":\"dummyEvent3\"}";
   ASSERT_TRUE(reader.parse(strCurrDomainEventMap, jsonTemp));

   ic_utils::Json::Value jsonArrayCurrDomainSec = jsonTemp["dummyDomain3"];

   strCurrDomainEventMap = "";
   jsonTemp.clear();

   /*New mapping for"dummyDomain3" from domainEventMap config update with 
    *array of events - "dummyEvent3" and "dummyEvent3" 
    */
   //Input 3
   strCurrDomainEventMap = "{\"dummyDomain3\":\"dummyEvent5\"}";
   ASSERT_TRUE(reader.parse(strCurrDomainEventMap, jsonTemp));

   ic_utils::Json::Value jsonNewDomainSec = jsonTemp["dummyDomain3"];

   CBaseMessageHandlerTest bmhTestInstance;
   //Update new mapping for "dummyDomain3" in EventDomainMap
   bmhTestInstance.UpdateEventDomainMapForStringType(jsonUpdatedEventDomainMap,
      "dummyDomain3",jsonArrayCurrDomainSec,jsonNewDomainSec);

   strCurrDomainEventMap = "{\"dummyEvent1\":\"dummyDomain1\",\"dummyEvent2\":\
   \"dummyDomain2\",\"dummyEvent4\":\"dummyDomain4\",\"dummyEvent5\":\
   \"dummyDomain3\"}";
   ASSERT_TRUE(reader.parse(strCurrDomainEventMap, jsonTemp));

   //Expecting new mapping with "dummyEvent5" for "dummyDomain3"
   EXPECT_EQ(jsonUpdatedEventDomainMap,jsonTemp);
}

}