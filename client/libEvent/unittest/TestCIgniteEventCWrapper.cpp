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
#include "CIgniteEventCWrapper.h"
#include "CIgniteMessage.h"

namespace ic_event 
{

/**
 * Class CIgniteEventCWrapperTest defines a test feature
 * for CIgniteEventCWrapper class
 */
class CIgniteEventCWrapperTest : public ::testing::Test 
{
protected:
    /**
     * Constructor
     */
    CIgniteEventCWrapperTest()
    {
        // Do nothing
    }
 
    /**
     * Destructor
     */
    ~CIgniteEventCWrapperTest() override
    {
        // Do nothing
    }

    /**
     * SetUp method : Code here will be called immediately after the
     * constructor (right before each test)
     * @see testing::Test::SetUp()
     */
    void SetUp() override 
    {
        // Do nothing
    }

    /**
     * TearDown method : Code here will be called immediately after
     * each test (right before the destructor)
     * @see testing::Test::TearDown()
     */
    void TearDown() override 
    {
        // Do nothing
    }
};

//Ignite Event Wrapper functions
TEST_F(CIgniteEventCWrapperTest, Test_ievent_create)
{
    // Passing sample values for verion and event name
    const std::string strVersion = "1.0";
    const std::string strEventName = "IgniteEvent";

    // Converting json payload into an event
    IgniteEvent* pstIE = ievent_create(strVersion.c_str(), 
                                       strEventName.c_str());

    // Checking for null pointer of structure IgniteEvent
    // Do not proceed if it is nullptr
    ASSERT_TRUE(nullptr != pstIE);

    // Releasing the memory consumed
    ievent_delete(pstIE);
}

TEST_F(CIgniteEventCWrapperTest, Test_ievent_getVersion) 
{
    // Passing sample values for verion and event name
    const std::string strVersion = "1.0";
    const std::string strEventName = "IgniteEvent";

    // Converting json payload into an event
    IgniteEvent* pstIE = ievent_create(strVersion.c_str(), 
                                       strEventName.c_str());

    // Checking for null pointer of structure IgniteEvent
    // Do not proceed if it is nullptr
    ASSERT_TRUE(nullptr != pstIE);

    char* pchResponse = ievent_get_version(pstIE);

    /*
     * Using if-else condition instead of ASSERT to check for nullptr as if-else
     * allows us to delete both the IgniteEvent pointer(pstIE) and pchResponse 
     * in if block or delete just pstIE in the else block if pchResponse is a 
     * nullptr but using ASSERT for nullcheck of pchResponse will not make
     * this possible.
     */
    if (nullptr != pchResponse)
    {
        // Expecting same sample version as set with the event
        EXPECT_STREQ(pchResponse, "1.0");

       // Releasing the memory consumed
       delete pchResponse;
       ievent_delete(pstIE);
    }
    else
    {
        //delete the object
        ievent_delete(pstIE);
        
        //forcing the test to fail as the pointer is null        
        EXPECT_TRUE(0);
    }    
}

TEST_F(CIgniteEventCWrapperTest, Test_ievent_getEventID) 
{
    // Passing sample values for verion and event name
    const std::string strVersion = "1.0";
    const std::string strEventName = "IgniteEvent";

    // Converting json payload into an event
    IgniteEvent* pstIE = ievent_create(strVersion.c_str(), 
                                       strEventName.c_str());

    // Checking for null pointer of structure IgniteEvent
    // Do not proceed if it is nullptr
    ASSERT_TRUE(nullptr != pstIE);

    char* pchResponse = ievent_get_eventid(pstIE);

    /*
     * Using if-else condition instead of ASSERT to check for nullptr as if-else
     * allows us to delete both the IgniteEvent pointer(pstIE) and pchResponse 
     * in if block or delete just pstIE in the else block if pchResponse is a 
     * nullptr but using ASSERT for nullcheck of pchResponse will not make
     * this possible.
     */
    if (nullptr != pchResponse)
    {
        // Expecting same sample version as set with the event
        EXPECT_STREQ(pchResponse, "IgniteEvent");

       // Releasing the memory consumed
       delete pchResponse;
       ievent_delete(pstIE);
    }
    else
    {
        //delete the object
        ievent_delete(pstIE);
        
        //forcing the test to fail as the pointer is null        
        EXPECT_TRUE(0);
    }    
}

TEST_F(CIgniteEventCWrapperTest, Test_ievent_get_timestamp) 
{
    // Creating the json payload in string format
    const char chJsonEvent[] = "{\"Data\":{\"accuracy\":"
    "3.7245287895202637,\"altitude\":250.27734375,\"antennaState\":"
    "1,\"bearing\":0,\"distance\":0,\"gpsLatitude\":"
    "42.466876310000004,\"gpsLongitude\":-83.412123539999996,\"latitude\":"
    "42.466876310000004,\"longitude\":-83.412123539999996,\"satsUsed\":"
    "5,\"speed\":0},\"EventID\":\"Location\",\"Timestamp\":"
    "1648137071335,\"Timezone\":0,\"Version\":\"1.0\"}";
    
    // Converting json payload into an event
    IgniteEvent* pstIE = ievent_jsontoevent(chJsonEvent);

    // Checking for null pointer of structure IgniteEvent
    // Do not proceed if it is nullptr
    ASSERT_TRUE(nullptr != pstIE);
    
    double dblRetValue = ievent_get_timestamp(pstIE);

    // Expecting the same timestamp as set with the event
    EXPECT_EQ(dblRetValue, 1648137071335);
    
    // Releasing the memory consumed
    ievent_delete(pstIE);
}

TEST_F(CIgniteEventCWrapperTest, Test_ievent_addfield_getfield_String)
{
    // Passing sample values for verion and event name
    const std::string strVersion = "1.0";
    const std::string strEventName = "IgniteEvent";

    // Converting json payload into an event
    IgniteEvent* pstIE = ievent_create(strVersion.c_str(), 
                                       strEventName.c_str());

    // Checking for null pointer of structure IgniteEvent
    // Do not proceed if it is nullptr
    ASSERT_TRUE(nullptr != pstIE);

    // Adding and getting sample value for string field
    ievent_addfield_char(pstIE, "accuracy", "3.2160000801086426");
    char* pchResponse = ievent_get_string(pstIE, "accuracy");

    /*
     * Using if-else condition instead of ASSERT to check for nullptr as if-else
     * allows us to delete both the IgniteEvent pointer(pstIE) and pchResponse 
     * in if block or delete just pstIE in the else block if pchResponse is a 
     * nullptr but using ASSERT for nullcheck of pchResponse will not make
     * this possible.
     */
    if (nullptr != pchResponse)
    {
        // Expecting the same string value that is added
        EXPECT_STREQ(pchResponse, "3.2160000801086426");

       // Releasing the memory consumed
       delete pchResponse;
       ievent_delete(pstIE);
    }
    else
    {
        //delete the object
        ievent_delete(pstIE);
        
        //forcing the test to fail as the pointer is null        
        EXPECT_TRUE(0);
    }
}

TEST_F(CIgniteEventCWrapperTest, Test_ievent_addfield_getfield_Int)
{
    // Passing sample values for verion and event name
    const std::string strVersion = "1.0";
    const std::string strEventName = "IgniteEvent";

    // Converting json payload into an event
    IgniteEvent* pstIE = ievent_create(strVersion.c_str(), 
                                       strEventName.c_str());

    // Checking for null pointer of structure IgniteEvent
    // Do not proceed if it is nullptr
    ASSERT_TRUE(nullptr != pstIE);

    // Adding and getting sample value for int field
    ievent_addfield_int(pstIE, "antennaState", 1);
    int nResponse = ievent_get_int(pstIE, "antennaState");

    // Expecting the same sample value added
    EXPECT_EQ(nResponse, 1);

    // Releasing the memory consumed
    ievent_delete(pstIE);
}

TEST_F(CIgniteEventCWrapperTest, Test_ievent_addfield_getfield_double)
{
    // Passing sample values for verion and event name
    const std::string strVersion = "1.0";
    const std::string strEventName = "IgniteEvent";

    // Converting json payload into an event
    IgniteEvent* pstIE = ievent_create(strVersion.c_str(), 
                                       strEventName.c_str());

    // Checking for null pointer of structure IgniteEvent
    // Do not proceed if it is nullptr
    ASSERT_TRUE(nullptr != pstIE);

    // Adding and getting sample value for double field
    ievent_addfield_double(pstIE, "altitude", 250.27734375);
    double dblResponse = ievent_get_double(pstIE, "altitude");

    // Expecting the same sample value added
    EXPECT_EQ(dblResponse, 250.27734375);
    
    // Releasing the memory consumed
    ievent_delete(pstIE);
}

TEST_F(CIgniteEventCWrapperTest, Test_ievent_addfield_getfield_long)
{
    // Passing sample values for verion and event name
    const std::string strVersion = "1.0";
    const std::string strEventName = "IgniteEvent";

    // Converting json payload into an event
    IgniteEvent* pstIE = ievent_create(strVersion.c_str(), 
                                       strEventName.c_str());

    // Checking for null pointer of structure IgniteEvent
    // Do not proceed if it is nullptr
    ASSERT_TRUE(nullptr != pstIE);

    // Adding and getting sample value for long field
    ievent_addfield_long(pstIE, "altitude", 25027734375);
    long lResponse = ievent_get_long(pstIE, "altitude");

    // Expecting the same sample value added
    EXPECT_EQ(lResponse, 25027734375);
    
    // Releasing the memory consumed
    ievent_delete(pstIE);
}

TEST_F(CIgniteEventCWrapperTest, Test_ievent_addfield_getfield_boolTrue)
{
    // Passing sample values for verion and event name
    const std::string strVersion = "1.0";
    const std::string strEventName = "IgniteEvent";

    // Converting json payload into an event
    IgniteEvent* pstIE = ievent_create(strVersion.c_str(), 
                                       strEventName.c_str());

    // Checking for null pointer of structure IgniteEvent
    // Do not proceed if it is nullptr
    ASSERT_TRUE(nullptr != pstIE);

    // Adding and getting sample value for bool field
    ievent_addfield_bool(pstIE, "providerGPS", true);
    bool bResponse = ievent_get_bool(pstIE, "providerGPS");

    // Expecting bool value to be true
    EXPECT_TRUE(bResponse);

    // Releasing the memory consumed
    ievent_delete(pstIE);
}

TEST_F(CIgniteEventCWrapperTest, Test_ievent_addfield_getfield_boolFalse)
{
    // Passing sample values for verion and event name
    const std::string strVersion = "1.0";
    const std::string strEventName = "IgniteEvent";

    // Converting json payload into an event
    IgniteEvent* pstIE = ievent_create(strVersion.c_str(), 
                                       strEventName.c_str());

    // Checking for null pointer of structure IgniteEvent
    // Do not proceed if it is nullptr
    ASSERT_TRUE(nullptr != pstIE);

    // Adding and getting sample value for bool field
    ievent_addfield_bool(pstIE, "providerGPS", false);
    bool bResponse = ievent_get_bool(pstIE, "providerGPS");

    // Expecting bool value to be false
    EXPECT_FALSE(bResponse);

    // Releasing the memory consumed
    ievent_delete(pstIE);
}

TEST_F(CIgniteEventCWrapperTest, Test_ievent_addfield_getfield_BizTransactionID)
{
    // Passing sample values for verion and event name
    const std::string strVersion = "1.0";
    const std::string strEventName = "IgniteEvent";

    // Converting json payload into an event
    IgniteEvent* pstIE = ievent_create(strVersion.c_str(), 
                                       strEventName.c_str());

    // Checking for null pointer of structure IgniteEvent
    // Do not proceed if it is nullptr
    ASSERT_TRUE(nullptr != pstIE);

    // Adding and getting sample value for BizTransactionID field
    ievent_add_biz_transactionid(pstIE, "7aec2481-5c09-11ec-8c45-9bdfa73044b8");
    char* pchResponse = ievent_get_biz_transactionid(pstIE);

    /*
     * Using if-else condition instead of ASSERT to check for nullptr as if-else
     * allows us to delete both the IgniteEvent pointer(pstIE) and pchResponse 
     * in if block or delete just pstIE in the else block if pchResponse is a 
     * nullptr but using ASSERT for nullcheck of pchResponse will not make
     * this possible.
     */
    if (nullptr != pchResponse)
    {
        // Expecting the same BizTransactionID that is added
        EXPECT_STREQ(pchResponse, "7aec2481-5c09-11ec-8c45-9bdfa73044b8");

       // Releasing the memory consumed
       delete pchResponse;
       ievent_delete(pstIE);
    }
    else
    {
        //delete the object
        ievent_delete(pstIE);
        
        //forcing the test to fail as the pointer is null        
        EXPECT_TRUE(0);
    }
}

TEST_F(CIgniteEventCWrapperTest, Test_ievent_addfield_getfield_messageID)
{
    // Passing sample values for verion and event name
    const std::string strVersion = "1.0";
    const std::string strEventName = "IgniteEvent";

    // Converting json payload into an event
    IgniteEvent* pstIE = ievent_create(strVersion.c_str(), 
                                       strEventName.c_str());

    // Checking for null pointer of structure IgniteEvent
    // Do not proceed if it is nullptr
    ASSERT_TRUE(nullptr != pstIE);

    // Adding and getting sample value for messageID field
    ievent_add_messageid(pstIE, "36002");
    char* pchResponse = ievent_get_messageid(pstIE);

    /*
     * Using if-else condition instead of ASSERT to check for nullptr as if-else
     * allows us to delete both the IgniteEvent pointer(pstIE) and pchResponse 
     * in if block or delete just pstIE in the else block if pchResponse is a 
     * nullptr but using ASSERT for nullcheck of pchResponse will not make
     * this possible.
     */
    if (nullptr != pchResponse)
    {
        // Expecting the same messageID that is added
        EXPECT_STREQ(pchResponse, "36002");

       // Releasing the memory consumed
       delete pchResponse;
       ievent_delete(pstIE);
    }
    else
    {
        //delete the object
        ievent_delete(pstIE);
        
        //forcing the test to fail as the pointer is null        
        EXPECT_TRUE(0);
    }
}

TEST_F(CIgniteEventCWrapperTest, Test_ievent_addfield_getfield_correlationID)
{
    // Passing sample values for verion and event name
    const std::string strVersion = "1.0";
    const std::string strEventName = "IgniteEvent";

    // Converting json payload into an event
    IgniteEvent* pstIE = ievent_create(strVersion.c_str(), 
                                       strEventName.c_str());

    // Checking for null pointer of structure IgniteEvent
    // Do not proceed if it is nullptr
    ASSERT_TRUE(nullptr != pstIE);

    // Adding and getting sample value for correlationID field
    ievent_add_correlationid(pstIE, "22222");
    char* pchResponse = ievent_get_correlationid(pstIE);

    /*
     * Using if-else condition instead of ASSERT to check for nullptr as if-else
     * allows us to delete both the IgniteEvent pointer(pstIE) and pchResponse 
     * in if block or delete just pstIE in the else block if pchResponse is a 
     * nullptr but using ASSERT for nullcheck of pchResponse will not make
     * this possible.
     */
    if (nullptr != pchResponse)
    {
        // Expecting the same correlationID that is added
        EXPECT_STREQ(pchResponse, "22222");

       // Releasing the memory consumed
       delete pchResponse;
       ievent_delete(pstIE);
    }
    else
    {
        //delete the object
        ievent_delete(pstIE);
        
        //forcing the test to fail as the pointer is null        
        EXPECT_TRUE(0);
    }
}

TEST_F(CIgniteEventCWrapperTest, Test_ievent_eventtojsonstring) 
{
    // Creating the json payload in string format
    const char chJsonEvent[] = "{\"Data\":{\"accuracy\":"
    "3.2160000801086426,\"altitude\":550,\"antennaState\":1,\"bearing\":"
    "270,\"distance\":679,\"gpsLatitude\":18.511010630000001,\"gpsLongitude\":"
    "73.777324039999996,\"latitude\":18.511010630000001,\"longitude\":"
    "73.777324039999996,\"providerGPS\":true,\"satsUsed\":20,\"speed\":"
    "14.508000370979289},\"EventID\":\"Location\",\"Timestamp\":"
    "1554349116702,\"Timezone\":330,\"Version\":\"1.0\"}\n";
    
    // Converting json payload into an event
    IgniteEvent* pstIE = ievent_jsontoevent(chJsonEvent);

    // Checking for null pointer of structure IgniteEvent
    // Do not proceed if it is nullptr
    ASSERT_TRUE(nullptr != pstIE);
    
    char* pchResponse = ievent_eventtojsonstring(pstIE);

    /*
     * Using if-else condition instead of ASSERT to check for nullptr as if-else
     * allows us to delete both the IgniteEvent pointer(pstIE) and pchResponse 
     * in if block or delete just pstIE in the else block if pchResponse is a 
     * nullptr but using ASSERT for nullcheck of pchResponse will not make
     * this possible.
     */
    if (nullptr != pchResponse)
    {
        // Expecting the same payload set
        EXPECT_STREQ(pchResponse, chJsonEvent);

       // Releasing the memory consumed
       delete pchResponse;
       ievent_delete(pstIE);
    }
    else
    {
        //delete the object
        ievent_delete(pstIE);

        //forcing the test to fail as the pointer is null        
        EXPECT_TRUE(0);
    }
}

TEST_F(CIgniteEventCWrapperTest, Test_ievent_send) 
{
    // Creating the json payload in string format
    const char chJsonEvent[] = "{\"EventID\":"
    "\"VehicleMessagePublish\",\"Version\":\"1.0\",\"Timestamp\":"
    "1528977275388,\"Data\":{\"vehicleMessageID\":36002,\"vin\":"
    "\"HU8NRXZFXS8628\",\"messageTemplate\":\"CVCI_Notification\",\"message\":"
    "[{\"language\":\"en-US\",\"messageText\":"
    "\"AnymessagetobedisplayedtoUI\",\"title\":"
    "\"CVCINotification\"}],\"messageType\":\"SERVICE_NOTICE\",\"priority\":"
    "0,\"additionalData\":{\"vin\":\"HCPDO3SOIFBX15684\"},\"notificationId\":"
    "\"CVCI_Notification\"},\"Timezone\":0,\"MessageId\":"
    "\"36002\",\"BizTransactionId\":\"7aec2481-5c09-11ec-8c45-9bdfa73044b8\"}";
    
    // Converting json payload into an event
    IgniteEvent* pstIE = ievent_jsontoevent(chJsonEvent);

    // Checking for null pointer of structure IgniteEvent
    // Do not proceed if it is nullptr
    ASSERT_TRUE(nullptr != pstIE);
        
    int nRetVal = ievent_send(pstIE);

    // Expecting 0 as sending event is successful
    EXPECT_EQ(nRetVal, 0);

    // Releasing the memory consumed
    ievent_delete(pstIE);
}
}
