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
#include "dam/CCacheTransport.h"

namespace ic_bl 
{
/**
 * Class CCacheTransportTest defines a test feature
 * for CCacheTransport class
 */
class CCacheTransportTest : public ::testing::Test
{
public:
    /**
     * Wrapper method for GetInstance of CCacheTransport class
     * @see CCacheTransport::GetInstance()
     */
    static CCacheTransport *GetInstance();

    /**
     * Wrapper method for LogCriticalEvent of CCacheTransport class
     * @see CCacheTransport::LogCriticalEvent()
     */
    static bool LogCriticalEvent(const std::string &rstrEID, 
                                 const std::string &rstrEPayload);

    /**
     * Wrapper method for UpdateLogConfigMap of CCacheTransport class
     * @see CCacheTransport::UpdateLogConfigMap()
     */
    static bool UpdateLogConfigMap(const ic_utils::Json::Value &rjsonEvntList);

    /**
     * Wrapper method for UpdateLogConfigMap of CCacheTransport class
     * @see CCacheTransport::InitConfigBasedLogging()
     */
    static bool InitConfigBasedLogging();

    /**
     * Wrapper method for PrintMap of CCacheTransport class
     * @see CCacheTransport::PrintMap()
     */
    static bool PrintMap(const std::map<std::string, int> &rMap);

    /**
     * Wrapper method for UpdateLogConfigMap of CCacheTransport class
     * @see CCacheTransport::ResetInflowEventLogCounter()
     */
    static bool ResetInflowEventLogCounter();

    /**
     * Wrapper method for ResetInflowEventLogCounterNonEmptyList of 
     * CCacheTransport class
     * @see CCacheTransport::ResetInflowEventLogCounter()
     */
    static bool ResetInflowEventLogCounterNonEmptyList();

    /**
     * Wrapper method for IsIgniteWhiteListedEventsValid of 
     * CCacheTransport class
     * @see CCacheTransport::IsIgniteWhiteListedEventsValid()
     */
    static bool IsIgniteWhiteListedEventsValid(const ic_utils::Json::Value 
                                               &rjsonWhitelistedEvents);

    /**
     * Wrapper method for IsDomainEventMapValid of CCacheTransport class
     * @see CCacheTransport::IsDomainEventMapValid()
     */
    static bool IsDomainEventMapValid(const ic_utils::Json::Value 
                                      &rjsonDomainEventMap);

    /**
     * Method to check FlushCache for CCacheTransport class
     * @param void
     * @return true is flushed cache else returned false
     */
    static bool FlushCache();

    /**
     * Method to return Session status of client
     * @param void
     * @return true if session is in progress else false
     */
    static bool FetchSessionStatus();

protected:
    /**
     * Constructor
     */
    CCacheTransportTest()
    {
        // do nothing
    }

    /**
     * Destructor
     */
    ~CCacheTransportTest() override
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

CCacheTransport* CCacheTransportTest::GetInstance()
{
    return CCacheTransport::GetInstance();
}

bool CCacheTransportTest::LogCriticalEvent(const std::string &rstrEID,
                                           const std::string &rstrPayload)
{
    //Fetching instance of CCacheTransport
    CCacheTransport *pCacheTransport = CCacheTransport::GetInstance();

    return pCacheTransport->LogCriticalEvent(rstrEID, rstrPayload);
}

bool CCacheTransportTest::UpdateLogConfigMap(const ic_utils::Json::Value 
                                             &rjsonEvntList)
{
    //Fetching instance of CCacheTransport
    CCacheTransport *pCacheTransport = CCacheTransport::GetInstance();

    return pCacheTransport->UpdateLogConfigMap(rjsonEvntList);
}

bool CCacheTransportTest::InitConfigBasedLogging()
{
    //Fetching instance of CCacheTransport
    CCacheTransport *pCacheTransport = CCacheTransport::GetInstance();

    return pCacheTransport->InitConfigBasedLogging();
}

bool CCacheTransportTest::PrintMap(const std::map<std::string, int> &rMap)
{
    //Fetching instance of CCacheTransport
    CCacheTransport *pCacheTransport = CCacheTransport::GetInstance();

    return pCacheTransport->PrintMap(rMap);
}

bool CCacheTransportTest::ResetInflowEventLogCounter()
{
    //Fetching instance of CCacheTransport
    CCacheTransport *pCacheTransport = CCacheTransport::GetInstance();

    return pCacheTransport->ResetInflowEventLogCounter();
}

bool CCacheTransportTest::ResetInflowEventLogCounterNonEmptyList()
{
    //Creating the speed event in string
    std::string strEvent = "{\"Data\":{\"value\":70},\"EventID\":"
    "\"Speed\",\"Timestamp\":1641643200000,\"Timezone\":330,\"Version\":"
    "\"1.0\"}";

    //Fetching instance of CCacheTransport
    CCacheTransport *pCacheTransport = CCacheTransport::GetInstance();

    //Sending the strEvent to CacheTransport
    pCacheTransport->Send(strEvent);

    //returns the value returned by ResetInflowEventLogCounter method
    return pCacheTransport->ResetInflowEventLogCounter();
}

bool CCacheTransportTest::IsIgniteWhiteListedEventsValid(
                    const ic_utils::Json::Value &rjsonWhitelistedEvents)
{
    //Fetching instance of CCacheTransport
    CCacheTransport *pCacheTransport = CCacheTransport::GetInstance();

    //returns the value returned by IsIgniteWhiteListedEventsValid method
    return pCacheTransport->IsIgniteWhiteListedEventsValid(
                                                       rjsonWhitelistedEvents);
}

bool CCacheTransportTest::IsDomainEventMapValid(const ic_utils::Json::Value 
                                                &rjsonDomainEventMap)
{
    //Fetching instance of CCacheTransport
    CCacheTransport *pCacheTransport = CCacheTransport::GetInstance();

    //Clearing values so that new values will be added
    pCacheTransport->m_jsonConfigDomainEventMap.clear();

    //returns the value returned by IsDomainEventMapValid method
    return pCacheTransport->IsDomainEventMapValid(rjsonDomainEventMap);
}

bool CCacheTransportTest::FlushCache()
{
    //Creating the speed event in string
    std::string strEvent =  "{\"Data\":{\"value\":70},\"EventID\":"
    "\"Speed\",\"Timestamp\":1641643200000,\"Timezone\":"
    "330,\"Version\":\"1.0\"}";

    //Fetching instance of CCacheTransport
    CCacheTransport *pCacheTransport = CCacheTransport::GetInstance();

    //Sending event to CCacheTransport returns fail then Flush is not possible
    if(!pCacheTransport->Send(strEvent))
    {
        return false;
    }

    //Calling fush Cache method
    pCacheTransport->FlushCache();

    //Checking size of queue if it 0 it has flushed the event
    if(0 == pCacheTransport->m_eventQueue.Size())
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool CCacheTransportTest::FetchSessionStatus()
{
    //Fetching instance of CCacheTransport
    CCacheTransport *pCacheTransport = CCacheTransport::GetInstance();

    //return session status of session status handler
    return pCacheTransport->m_pSessionStatusHandler->GetSessionStatus();
}

//Tests

TEST_F(CCacheTransportTest, Test_GetInstance) 
{
    //Fetching instance of CCacheTransport
    CCacheTransport *pCacheTransport = CCacheTransport::GetInstance();

    //Expect the both the below objects are equal
    EXPECT_EQ(pCacheTransport,CCacheTransportTest::GetInstance());
}

TEST_F(CCacheTransportTest, Test_getInstanceManyCaseValidation) 
{
    //Fetching multiple instance of CCacheTransport
    CCacheTransport *mCCacheTransportObj1 = CCacheTransport::GetInstance();
    CCacheTransport *mCCacheTransportObj2 = CCacheTransport::GetInstance();
    CCacheTransport *mCCacheTransportObj3 = CCacheTransport::GetInstance();
    CCacheTransport *mCCacheTransportObj4 = CCacheTransport::GetInstance();
    CCacheTransport *pCacheTransport = CCacheTransport::GetInstance();

    //Expect the all the below objects are equal
    EXPECT_EQ(mCCacheTransportObj1,pCacheTransport);
    EXPECT_EQ(mCCacheTransportObj2,pCacheTransport);
    EXPECT_EQ(mCCacheTransportObj3,pCacheTransport);
    EXPECT_EQ(mCCacheTransportObj4,pCacheTransport);
}

TEST_F(CCacheTransportTest, Test_Send_Event)
{
    //Creating speed Event in string format
    std::string strEvent = "{\"Data\":{\"value\":70},\"EventID\":"
    "\"Speed\",\"Timestamp\":1641643200000,\"Timezone\":"
    "330,\"Version\":\"1.0\"}";

    //Fetching instance of CCacheTransport
    CCacheTransport *pCacheTransport = CCacheTransport::GetInstance();

    //Expect true as CacheTrsnaport send event will return true
    EXPECT_TRUE(pCacheTransport->Send(strEvent));
}

TEST_F(CCacheTransportTest, Test_FlushCache_ForCacheTransport)
{
    //Expect true as events will be flushed
    EXPECT_TRUE(CCacheTransportTest::FlushCache());
}

TEST_F(CCacheTransportTest, Test_Send_WithNotifyShutdown)
{
    //Creating speed Event in string format
    std::string strEvent = "{\"Data\":{\"value\":70},\"EventID\":"
    "\"Speed\",\"Timestamp\":1641643200000,\"Timezone\":"
    "330,\"Version\":\"1.0\"}";

    //Fetching instance of CCacheTransport
    CCacheTransport *pCacheTransport = CCacheTransport::GetInstance();

    //calling notifyShutdown Event
    pCacheTransport->NotifyShutdown();

    //Expect false as CacheTrsnaport send event will return false 
    EXPECT_FALSE(pCacheTransport->Send(strEvent));
}

TEST_F(CCacheTransportTest, Test_IsSessionInProgress_SessionProgress) 
{
    //Fetching instance of CCacheTransport
    CCacheTransport *pCacheTransport = CCacheTransport::GetInstance();

    bool bSessionStatus = CCacheTransportTest::FetchSessionStatus();

    //Expect session status of FetchSessionStatus and IsSessionInProgress equal
    EXPECT_EQ(bSessionStatus, pCacheTransport->IsSessionInProgress());
}

TEST_F(CCacheTransportTest, Test_LogCriticalEvent_ForAlertsList)
{
    //Creating PreHibernate Event in string format
    std::string strEvent = "{\"Data\":"
    "{\"battVolt\":9.5},\"Timezone\":-300,\"EventID\":"
    "\"PreHibernate\",\"Version\":\"1.0\",\"Timestamp\":1654255953000}";
    
    // Expecting this event to be logged as critical
    EXPECT_TRUE(CCacheTransportTest::LogCriticalEvent("PreHibernate", strEvent));
}

TEST_F(CCacheTransportTest, Test_LogCriticalEvent_ForInfiniteMaxCount)
{
    //Creating IgnStatus Event in string format
    std::string strEvent = "{\"Data\":{\"state\":\"run\"},\"EventID\":"
    "\"IgnStatus\",\"Timestamp\":1554349111215,\"Timezone\":"
    "330,\"Version\":\"1.0\"}";

    // Expecting this event to be logged as critical
    EXPECT_TRUE(CCacheTransportTest::LogCriticalEvent("IgnStatus", strEvent));
}

TEST_F(CCacheTransportTest, Test_LogCriticalEvent_ForFirstTime)
{
    //Creating Speed Event in string format
    std::string strEvent = "{\"Data\":{\"state\":\"run\"},\"EventID\":"
    "\"IgnStatus\",\"Timestamp\":1554349111215,\"Timezone\":"
    "330,\"Version\":\"1.0\"}";

    // Expecting this event to be logged as critical
    EXPECT_TRUE(CCacheTransportTest::LogCriticalEvent("Speed", strEvent));
}

TEST_F(CCacheTransportTest, Test_LogCriticalEvent_ForExceedMaxCount)
{
    //MaxCount for Speed event is 5 as configured in the config file
    std::string strEvent = "{\"Data\":{\"value\":5558},\"EventID\":"
    "\"RPM\",\"Timestamp\":1641643200000,\"Timezone\":"
    "330,\"Version\":\"1.0\"}";

    /* Expecting these events to be logged as critical log as its logging count 
     * value configured as 5
     */ 
    EXPECT_TRUE(CCacheTransportTest::LogCriticalEvent("RPM", strEvent));
    EXPECT_TRUE(CCacheTransportTest::LogCriticalEvent("RPM", strEvent));
    EXPECT_TRUE(CCacheTransportTest::LogCriticalEvent("RPM", strEvent));
    EXPECT_TRUE(CCacheTransportTest::LogCriticalEvent("RPM", strEvent));
    EXPECT_TRUE(CCacheTransportTest::LogCriticalEvent("RPM", strEvent));

    /* Expecting same event to be logged as non-critical as its logging max 
     * count is exceeded
     */
    EXPECT_FALSE(CCacheTransportTest::LogCriticalEvent("RPM", strEvent));
}

TEST_F(CCacheTransportTest, Test_logEvent_ForDefaultMaxCount)
{
    //Creating ServiceRecordId Event in string format
    std::string strEvent = "{\"Data\":{\"maintenanceId\":\"NA\"},\"EventID\":"
    "\"ServiceRecordId\",\"Timestamp\":1554349119086,\"Timezone\":"
    "330,\"Version\":\"1.0\"}";

    /* Expecting this event to be logged as critical based on default logging 
     * count as its not configured as ignite event
     */ 
    EXPECT_TRUE(CCacheTransportTest::LogCriticalEvent("ServiceRecordId",
                                                       strEvent));
    EXPECT_TRUE(CCacheTransportTest::LogCriticalEvent("ServiceRecordId",
                                                       strEvent));

    /* Expecting this event to be logged as non-critical as its not configured
     * as ignite event
     */ 
    EXPECT_FALSE(CCacheTransportTest::LogCriticalEvent("ServiceRecordId", 
                                                        strEvent));
}

TEST_F(CCacheTransportTest, Test_logEvent_ForExceedDefaultMaxCount)
{
    //Creating Typeoffuel Event in string format
    std::string strEvent = "{\"Data\":{\"value\":\"6\"}, \"EventID\":"
    "\"Typeoffuel\",\"Timestamp\":1554349119001,\"Timezone\":"
    "330,\"Version\":\"1.0\"}";

    /* Expecting this event to be logged as critical based on default logging 
     * count as its not configured as ignite event
     */ 
    EXPECT_TRUE(CCacheTransportTest::LogCriticalEvent("Typeoffuel", strEvent));
    EXPECT_TRUE(CCacheTransportTest::LogCriticalEvent("Typeoffuel", strEvent));

    /* Expecting this event to be logged as non-critical as its not configured
     * as ignite event
     */
    EXPECT_FALSE(CCacheTransportTest::LogCriticalEvent("Typeoffuel", strEvent));
}

TEST_F(CCacheTransportTest, Test_UpdateLogConfigMap_Empty)
{
    //Creating empty string
    std::string strEvent = "[]";
    ic_utils::Json::Value jsonEvent;
    ic_utils::Json::Reader jsonReader;

    //Expect true as parsing of json will be successfull
    EXPECT_TRUE(jsonReader.parse(strEvent, jsonEvent));

    //Expect false as above event will not updated in config map
    EXPECT_FALSE(CCacheTransportTest::UpdateLogConfigMap(jsonEvent));
}

TEST_F(CCacheTransportTest, Test_UpdateLogConfigMap_UpdateTheMap)
{
    //Initializing json to be sent to cacheTransport for logging
    std::string strJson = "[{\"RPM\":5},{\"Speed\":5},{\"Odometer\":"
    "5},{\"Location\":20},{\"VIN\":-1},{\"SessionStatus\":"
    "-1},{\"DeviceRemoval\":-1},{\"IgnStatus\":"
    "-1},{\"DeviceClientStateChanged\":-1},{\"ModuleState\":"
    "-1},{\"IgniteClientLaunched\":-1},{\"FirmwareVersion\":"
    "-1},{\"ActivationStateRequest\":-1},{\"DeviceConfigSync\":"
    "-1},{\"DeviceNotificationResponse\":5},{\"RequestConfigUpdate\":"
    "5},{\"ReloadMapperConfigUpdate\":5},{\"FuelLevel\":5},{\"VINResponse\":"
    "-1},{\"TokenRequest\":5},{\"TokenChanged\":5},{\"LogConfig\":"
    "-1},{\"ActivationEvent\":-1},{\"ConfigUpdateRequest\":"
    "-1},{\"ConfigUpdateResponse\":-1}]";
    ic_utils::Json::Value jsonEvent;
    ic_utils::Json::Reader jsonReader;

    //Expect true as parsing of Json is success
    EXPECT_TRUE(jsonReader.parse(strJson, jsonEvent));

    //Expect true as UpdateLogConfigMap will update with above json
    EXPECT_TRUE(CCacheTransportTest::UpdateLogConfigMap(jsonEvent));
}

TEST_F(CCacheTransportTest, Test_InitConfigBasedLogging_initializeconfig)
{
    //Expect true as InitConfigBasedLogging will be initialized based on config
    EXPECT_TRUE(CCacheTransportTest::InitConfigBasedLogging());
}

TEST_F(CCacheTransportTest, Test_PrintMap_EmptyMap)
{
    //declaring Map
    std::map<std::string, int> mapStr;

    //Expect false as empty map will not be printed in log
    EXPECT_FALSE(CCacheTransportTest::PrintMap(mapStr));
}

TEST_F(CCacheTransportTest, Test_PrintMap_GivenMap)
{
    //declaring Map
    std::map<std::string, int> mapStr;

    //Inserting Map with IgnStatus string
    mapStr.insert(std::pair<std::string, int>("IgnStatus",1));

    //Expect true as above map will be printed
    EXPECT_TRUE(CCacheTransportTest::PrintMap(mapStr));
}

TEST_F(CCacheTransportTest, Test_ResetInflowEventLogCounter_EmptyEventList) 
{
    //Expect true as inflow event log counter will be resetted as list is empty
    EXPECT_TRUE(CCacheTransportTest::ResetInflowEventLogCounter());
}

TEST_F(CCacheTransportTest, Test_ResetInflowEventLogCounter_withEventList) 
{
    //Expect true as inflow event log counter will clear the list
    EXPECT_TRUE(CCacheTransportTest::ResetInflowEventLogCounterNonEmptyList());
}

TEST_F(CCacheTransportTest, Test_IsIgniteWhiteListedEventsValid_EmptyValue) 
{
    //Initializing json with null ref
    ic_utils::Json::Value jsonIgniteEvent = ic_utils::Json::Value::nullRef;

    //Expect false as IsIgniteWhiteListedEventsValid will return false
    EXPECT_FALSE(CCacheTransportTest::IsIgniteWhiteListedEventsValid(
                                                            jsonIgniteEvent));
}

TEST_F(CCacheTransportTest, Test_IsIgniteWhiteListedEventsValid_WithConfigValue) 
{
    //Initializing json with values present from config for Ignite Events
    ic_utils::Json::Value jsonIgniteEvent = ic_core::CIgniteConfig::
                    GetInstance()->GetJsonValue("DAM.IgnitewhiteListedEvents");

    //Expect false as IsIgniteWhiteListedEventsValid will return false
    EXPECT_TRUE(CCacheTransportTest::IsIgniteWhiteListedEventsValid(
                                                            jsonIgniteEvent));
}

TEST_F(CCacheTransportTest, Test_IsDomainEventMapValid_EmptyValue) 
{
    //Initializing json with null ref
    ic_utils::Json::Value jsonIgniteEvent = ic_utils::Json::Value::nullRef;

    //Expect false as IsDomainEventMapValid will return false
    EXPECT_FALSE(CCacheTransportTest::IsDomainEventMapValid(jsonIgniteEvent));
}

TEST_F(CCacheTransportTest, Test_IsDomainEventMapValid_WithConfigValue) 
{
    //Initializing json with values present from config for domain map
    ic_utils::Json::Value jsonIgniteEvent = ic_core::CIgniteConfig::
                    GetInstance()->GetJsonValue("MQTT.domainEventMap");

    //Expect false as IsDomainEventMapValid will return false
    EXPECT_TRUE(CCacheTransportTest::IsDomainEventMapValid(jsonIgniteEvent));
}
} /* namespace ic_bl*/
