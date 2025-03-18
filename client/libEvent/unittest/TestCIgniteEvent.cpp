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
#include "CIgniteEvent.h"
#include "CIgniteEventSender.h"
#include "CIgniteLog.h"
#include "CIgniteDateTime.h"
#include "CClientInfo.h"

// Macro for "test_CIgniteEvent" string
const std::string PREFIX = "test_CIgniteEvent";

namespace ic_event 
{

/**
 * Enum of various types to test ValidateOutFileName() API
 */
typedef enum
{
    eATTACHMENT_LIMIT_EXCEED,   ///< attachment file count crossing the limit
    eATTACHMENT_PATH_EMPTY,     ///< attachment path is empty
    eDEVICE_ID_EMPTY,           ///< device id is empty
    eLAREG_FILE,                ///< attachment file is large
    eTOTAL_OVERLIMIT,           ///< total size of attachments crosses limit
    eUNDERSCORE                 ///< underscore is rejected in filetype/eventid
}ValidateOutFileNameTypes;

// Class CIgniteEventTest defines a test feature for CIgniteEvent class
class CIgniteEventTest : public ::testing::Test 
{
public:
    /**
     * Constructor
     */
    CIgniteEventTest()
    {
        // Do nothing
    }
 
    /**
     * Destructor
     */
    ~CIgniteEventTest() override
    {
        // Do nothing
    }

protected:
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

    void TestBody() override {
    }

public:
    /**
     * Wrapper method for CIgniteEvent::GetFileNameAndExtension()
     * @see CIgniteEvent::CGranularityReductionHandler()
     */
    void GetFileNameAndExtension(const std::string &rstrPath,
            std::string &rstrExtn, std::string &rstrFilename);

    /**
     * Wrapper method for CIgniteEvent::GetExistingAttachmentSize()
     * @see CIgniteEvent::GetExistingAttachmentSize()
     */
    int GetExistingAttachmentSize(const std::string &rnstrFileAttachmentPath);

    /**
     * Wrapper method for CIgniteEvent::ValidateOutFileName()
     * @param [in] eType type of validation to perform
     * @param [out] rstrOutFileName output filename
     * @return error code indicating if any failure in validation
     * @see CIgniteEvent::ValidateOutFileName()
     */
    int ValidateOutFileName(ValidateOutFileNameTypes eType,
                                std::string &rstrOutFileName);

    /**
     * Wrapper method for CIgniteEvent::AddExtnToOutFileName()
     * @see CIgniteEvent::AddExtnToOutFileName()
     */
    void AddExtnToOutFileName(const std::string &rstrFileExtension,
                                  std::string &rstrOutputFileName);

};

void CIgniteEventTest::AddExtnToOutFileName(
                                const std::string &rstrFileExtension,
                                std::string &rstrOutputFileName)
{
    CIgniteEvent igniteEventObj;

    igniteEventObj.AddExtnToOutFileName(rstrFileExtension,rstrOutputFileName);
}

int CIgniteEventTest::ValidateOutFileName(ValidateOutFileNameTypes eType,
                                        std::string &rstrOutFileName)
{
    CIgniteEvent igniteEventObj;

    //initialize the return value
    int nErrorCode = ATTACHMENT_OK;

    if (eATTACHMENT_LIMIT_EXCEED == eType)
    {
        //PRECONDITIONS-BEGIN
        //assigning a higher value than MAX_ATTACHMENT_LIMIT to simulate
        //   the use-case
        igniteEventObj.m_nAttachmentCount = 1000;
        //PRECONDITIONS-END

        //as scope of testing is to test attachment limit case, other
        //  parameters are of no significance for testing, so using dummy values
        nErrorCode = igniteEventObj.ValidateOutFileName(0,0,0,0,
                        "dummy","TestEventId",rstrOutFileName);
    }
    else if (eATTACHMENT_PATH_EMPTY == eType)
    {
        //PRECONDITIONS-BEGIN
        //setting empty path to simulate the use-case
        igniteEventObj.m_strFileAttachmentPath = "";
        //PRECONDITIONS-END

        //as scope of testing is to test attachment limit case, other
        //  parameters are of no significance for testing, so using dummy values
        nErrorCode = igniteEventObj.ValidateOutFileName(0,0,0,0,
                        "dummy","TestEventId",rstrOutFileName);
    }
    else if (eDEVICE_ID_EMPTY == eType)
    {
        //PRECONDITIONS-BEGIN
        //set a dummy attachment path; if it is empty, it will
        //  affect the intended use-case test
        igniteEventObj.m_strFileAttachmentPath = "/tmp/";

        //setting empty device-id to simulate the use-case
        igniteEventObj.m_strDeviceId = "";
        //PRECONDITIONS-END

        //as scope of testing is to test attachment limit case, other
        //  parameters are of no significance for testing, so using dummy values
        nErrorCode = igniteEventObj.ValidateOutFileName(0,0,0,0,
                        "dummy","TestEventId",rstrOutFileName);
    }
    else if (eLAREG_FILE == eType)
    {
        //PRECONDITIONS-BEGIN
        //read the attachment path; if it is empty, it will
        //  affect the intended use-case test
        igniteEventObj.m_strFileAttachmentPath = "/tmp/";

        //setting dummy device-id so that it's emptiness will not
        //  affect the intended test
        igniteEventObj.m_strDeviceId = "dummyId";
        //PRECONDITIONS-END

        //Using 1000bytes as new attachment size and 500bytes as
        //    so that the use-case will be simulated.
        //As scope of testing is to test attachment limit case, other
        //  parameters are of no significance for testing, so using dummy values
        nErrorCode = igniteEventObj.ValidateOutFileName(1000,0,500,0,
                        "dummy","TestEventId",rstrOutFileName);
    }
    else if (eTOTAL_OVERLIMIT == eType)
    {
        //PRECONDITIONS-BEGIN
        //read the attachment path; if it is empty, it will
        //  affect the intended use-case test
        igniteEventObj.m_strFileAttachmentPath = "/tmp/";

        //setting dummy device-id so that it's emptiness will not
        //  affect the intended test
        igniteEventObj.m_strDeviceId = "dummyId";
        //PRECONDITIONS-END

        //Using 1000bytes as new attachment size, 2000bytes as existing
        //    attachment size and 1250bytes as total limit so that the
        //   use-case will be simulated.
        //Use dummy values for other parameters as they will not impact the
        //  test scope.
        nErrorCode = igniteEventObj.ValidateOutFileName(500,1000,1000,1250,
                        "dummy","TestEventId",rstrOutFileName);
    }
    else if (eUNDERSCORE == eType)
    {
        //PRECONDITIONS-BEGIN
        //read the attachment path; if it is empty, it will
        //  affect the intended use-case test
        igniteEventObj.m_strFileAttachmentPath = "/tmp/";

        //setting dummy device-id so that it's emptiness will not
        //  affect the intended test
        igniteEventObj.m_strDeviceId = "dummyId";
        //PRECONDITIONS-END

        //Using "my_type" as file type and "Test_EventId" as eventId where
        //    underscore character is included which will simulate the use-case
        //Use dummy values for other parameters as they will not impact the
        //  test scope.
        nErrorCode = igniteEventObj.ValidateOutFileName(0,0,0,0,
                        "my_type","Test_EventId",rstrOutFileName);
    }
    else
    {
        //do nothing
    }

    return nErrorCode;
}

int CIgniteEventTest::GetExistingAttachmentSize(const std::string &rnstrFileAttachmentPath)
{
    CIgniteEvent igniteEventObj;

    return igniteEventObj.GetExistingAttachmentSize(rnstrFileAttachmentPath);
}

void CIgniteEventTest::GetFileNameAndExtension(const std::string &rstrPath,
        std::string &rstrExtn, std::string &rstrFilename)
{
    CIgniteEvent igniteEventObj;

    igniteEventObj.GetFileNameAndExtension(rstrPath, rstrExtn, rstrFilename);
}

// Tests
TEST_F(CIgniteEventTest, Test_check_AcpEvent_zero_test_defaultConstructor)
{
    // Creating an object for IgniteEvent class
    CIgniteEvent igniteEventObj;

    // Checking the post conditions for CIgniteEvent object as zero test case as per TDD approach
    // Expecting the default values initialized by the default constructor
    EXPECT_GT(igniteEventObj.GetTimestamp(), 0);
}

TEST_F(CIgniteEventTest, Test_check_Acpevent_one_test_parameterizedConstructor_withTwoParameter)
{
    /* Checking logic of CIgniteEvent object as one test case as per TDD approach
     * Creating an object for IgniteEvent class using a sample value for version to
     * check the functionality of GetVersion API
     */
    CIgniteEvent igniteEventObj("1.0", "Location");

    // Expecting the values assigned by the parametrized constructor
    EXPECT_EQ("Location", igniteEventObj.GetEventId());
    EXPECT_EQ("1.0", igniteEventObj.GetVersion());
    EXPECT_GT(igniteEventObj.GetTimestamp(), 0);
}

TEST_F(CIgniteEventTest, Test_check_AcpEvent_one_test_parameterizedConstructor_withThreeParameter)
{
    // Getting the current time and assigning to the variable
    unsigned long long ullTs = ic_utils::CIgniteDateTime::GetCurrentTimeMs();

    /* Checking logic of CIgniteEvent object as one test case as per TDD approach 
     * Creating an object for IgniteEvent class using random number for version and
     * timestamp to check the functionality of GetVersion and GetTimestamp API
     */
    CIgniteEvent igniteEventObj("1.0", "Location", ullTs);

    // Expecting the values assigned by the parametrized constructor
    EXPECT_EQ("Location", igniteEventObj.GetEventId());
    EXPECT_EQ("1.0", igniteEventObj.GetVersion());
    EXPECT_EQ(ullTs, igniteEventObj.GetTimestamp());
}

TEST_F(CIgniteEventTest, Test_getTimestamp_using_setTimestamp)
{
    // Creating an object for IgniteEvent class
    CIgniteEvent igniteEventObj;

    // Getting the current time and assigning to the variable
    unsigned long long ullTs = ic_utils::CIgniteDateTime::GetCurrentTimeMs();

    // Setting the timestamp value with 'SetTimestamp' member function
    igniteEventObj.SetTimestamp(ullTs);

    // Expecting the API to return same value set with 'SetTimestamp'
    EXPECT_EQ(ullTs, igniteEventObj.GetTimestamp());
}

TEST_F(CIgniteEventTest, Test_getVersion_using_setVersion)
{
    // Creating an object for IgniteEvent class
    CIgniteEvent igniteEventObj;

    // Assigning the random value to the variable
    std::string strVersion = "1.0";

    // Setting the value with 'SetVersion' member function
    igniteEventObj.SetVersion(strVersion); 

    // Expecting the API to return same value set with 'SetVersion'
    EXPECT_EQ(igniteEventObj.GetVersion(), strVersion);
}

TEST_F(CIgniteEventTest, Test_getInt_using_addField_method)
{
    // Creating an object for IgniteEvent class
    CIgniteEvent igniteEventObj;

    // Assigning the random value to the variable
    const std::string strKey = "volumeChange";
    const int KeyValue = 10;

    // Adding the above random values to the event
    igniteEventObj.AddField(strKey,KeyValue);

    // Expecting the API to return same value set with 'AddField'
    EXPECT_EQ(igniteEventObj.GetInt(strKey),KeyValue);
}

TEST_F(CIgniteEventTest, Test_getInt_for_default_case)
{
    // Creating an object for IgniteEvent class
    CIgniteEvent igniteEventObj;

    // Expecting the API to return default value (0) as key("volumeChange") is not added
    EXPECT_EQ(igniteEventObj.GetInt("volumeChange"),0);
}

TEST_F(CIgniteEventTest, Test_getLong_using_addField_method)
{
    // Creating an object for IgniteEvent class
    CIgniteEvent igniteEventObj;

    // Assigning the random value to the variable
    const std::string strKey = "modeAdvance";
    const long long llKeyValue = 1505714535000;

    // Adding the above random values to the event
    igniteEventObj.AddField(strKey, llKeyValue);

    // Expecting the API to return same value set with 'AddField'
    EXPECT_EQ(igniteEventObj.GetLong(strKey), llKeyValue);
}

TEST_F(CIgniteEventTest, Test_getLong_for_default_case)
{
    // Creating an object for IgniteEvent class
    CIgniteEvent igniteEventObj;

    // Expecting the API to return default value (0) as key("modeAdvance") is not added
    EXPECT_EQ(igniteEventObj.GetLong("modeAdvance"),0);
}

TEST_F(CIgniteEventTest, Test_getString_using_addField_method)
{
    // Creating an object for IgniteEvent class
    CIgniteEvent igniteEventObj;

    // Assigning the random value to the variable
    const std::string strKey = "presetSeekUp";
    const std::string strKeyValue = "20";

    // Adding the above random values to the event
    igniteEventObj.AddField(strKey, strKeyValue);

    // Expecting the API to return same value set with 'AddField'
    EXPECT_EQ(igniteEventObj.GetString(strKey), strKeyValue);
}

TEST_F(CIgniteEventTest, Test_getString_for_default_case)
{
    // Creating an object for IgniteEvent class
    CIgniteEvent igniteEventObj;

    // Expecting the API to return empty value as key("presetSeekUp") is not added
    EXPECT_TRUE((igniteEventObj.GetString("presetSeekUp")).empty());
}

TEST_F(CIgniteEventTest, Test_getString_using_extractName_method)
{
    // Creating an object for IgniteEvent class
    CIgniteEvent igniteEventObj;

    // Assigning the random value to the variable
    std::string strName = "presetSeekUp_for_extraction_using_underscore_delimiter";

    // Setting the key value to the member variable with 'ExtractName' member function
    igniteEventObj.ExtractName(strName);

    // Setting the delimiter
    std::string str ("_");

    // Find first occurrence of "_"
    std::size_t nFound = strName.find(str);

    // Extracting the first word as key from the string 'presetSeekUp_for_extraction_using_underscore_delimiter'
    std::string strKey = strName.substr(0, nFound);

    // Assigning the remaining sentence '_for_extraction_using_underscore_delimiter' as value
    std::string strKeyValue = strName.substr(nFound);

    // Expecting the API to return same value assigned by 'ExtractName' with the member variable
    EXPECT_EQ(igniteEventObj.GetString(strKey), strKeyValue);
}

TEST_F(CIgniteEventTest, Test_getBool_using_addField_method)
{
    // Creating an object for IgniteEvent class
    CIgniteEvent igniteEventObj;

    // Assigning the random value to the variable
    const std::string strKey = "presetAdvance";
    const bool bKeyValue = true;

    // Adding the above random values to the event
    igniteEventObj.AddField(strKey, bKeyValue);

    // Expecting the API to return same value set with 'AddField'
    EXPECT_TRUE(igniteEventObj.GetBool(strKey));
}

TEST_F(CIgniteEventTest, Test_getBool_for_default_case)
{
    // Creating an object for IgniteEvent class
    CIgniteEvent igniteEventObj;

    // Expecting the API to return false value as key("presetAdvance") is not added
    EXPECT_FALSE(igniteEventObj.GetBool("presetAdvance"));
}

TEST_F(CIgniteEventTest, Test_getDouble_using_addField_method)
{
    // Creating an object for IgniteEvent class
    CIgniteEvent igniteEventObj;

    // Assigning the random value to the variable
    const std::string strKey = "presetAdvance";
    double dblKeyValue = 125.98;

    // Adding the above random values to the event
    igniteEventObj.AddField(strKey, dblKeyValue);

    // Expecting the API to return same value set with 'AddField'
    EXPECT_EQ(igniteEventObj.GetDouble(strKey), dblKeyValue);
}

TEST_F(CIgniteEventTest, Test_getDouble_for_default_case)
{
    // Creating an object for IgniteEvent class
    CIgniteEvent igniteEventObj;

    // Expecting the API to return default value as key("presetAdvance") is not added
    EXPECT_EQ(igniteEventObj.GetDouble("presetAdvance"),0.0);
}

TEST_F(CIgniteEventTest, Test_getData_using_addField_method)
{
    // Creating an object for IgniteEvent class
    CIgniteEvent igniteEventObj("1.0" ,"Location");

    // Composing the jsonData using AddField function
    igniteEventObj.AddField("address","64:A3:CB:86:45:");
    igniteEventObj.AddField("name","Ignite");
    igniteEventObj.AddField("service","A2DP");

    // Expecting the API to return the payload composed with AddField function
    ic_utils::Json::Value jsonValue = igniteEventObj.GetData();

    // Expecting the same address set with the AddField
    EXPECT_EQ(jsonValue["address"].asString(),"64:A3:CB:86:45:");

    // Expecting the same name set with the AddField
    EXPECT_EQ(jsonValue["name"].asString(),"Ignite");

    // Expecting the same service set with the AddField
    EXPECT_EQ(jsonValue["service"].asString(),"A2DP");
}

TEST_F(CIgniteEventTest, Test_getData_using_addFieldAsRawJsonString_for_string_value)
{
    // Creating an object for IgniteEvent class
    CIgniteEvent igniteEventObj;

    // Assigning the random value to the variable
    const std::string strName = "presetAdvance";

    // Creating the json payload in string format
    std::string strJsonValue = "{\"Data\":{\"value\":\"140\"},\"EventID\":\"Odometer\",\"Version\":\"1.0\",\"Timestamp\":1454674114815,\"Timezone\":60}";

    // Setting the value to the member variable with 'addFieldAsRawJsonString' member function
    igniteEventObj.AddFieldAsRawJsonString(strName, strJsonValue);

    // Expecting the API to return same value set with the API addFieldAsRawJsonString
    ic_utils::Json::Value jsonRoot = igniteEventObj.GetData();

    // Expecting true as "presetAdvance" is a member of set payload
    EXPECT_TRUE(jsonRoot.isMember("presetAdvance"));

    // Reading the payload related with the member "presetAdvance" that was set using addFieldAsRawJsonString method
    ic_utils::Json::Value jsonData = jsonRoot["presetAdvance"];

    // Assigning the value of "Data" member to the variable jsonResult
    ic_utils::Json::Value jsonResult = jsonData["Data"];

    // Assigning the value of "value" member to the variable resString
    std::string strResString = jsonResult["value"].asString();
    
    // Expecting the same value set with the payload
    EXPECT_STREQ("140", strResString.c_str());

    // Expecting the same EventID set with the payload
    EXPECT_EQ(jsonData["EventID"].asString(),"Odometer");

    // Expecting the same Version set with the payload
    EXPECT_EQ(jsonData["Version"].asString(),"1.0");

    // Expecting the same Timezone set with the payload
    EXPECT_EQ(jsonData["Timezone"].asInt(),60);
}

TEST_F(CIgniteEventTest, Test_getData_using_addFieldAsRawJsonString_for_character_value)
{
    // Creating an object for IgniteEvent class
    CIgniteEvent igniteEventObj;

    // Assigning the random value to the variable
    const std::string strName = "presetAdvance";

    // Creating the json payload in char* format
    const char* pchJsonValue =  "{\"Data\":{\"value\":\"150\"},\"EventID\":\"Odometer\",\"Version\":\"1.0\",\"Timestamp\":1454674114815,\"Timezone\":60}";

    // Setting the value to the member variable with 'addFieldAsRawJsonString' member function
    igniteEventObj.AddFieldAsRawJsonString(strName, pchJsonValue);

    // Expecting the API to return same value set with the API addFieldAsRawJsonString
    ic_utils::Json::Value jsonRoot = igniteEventObj.GetData();

    // Expecting true as "presetAdvance" is a member of set payload
    EXPECT_TRUE(jsonRoot.isMember("presetAdvance"));

    // Reading the payload related with the member "presetAdvance" that was set using addFieldAsRawJsonString method
    ic_utils::Json::Value jsonData = jsonRoot["presetAdvance"];

    // Assigning the value of "Data" member to the variable jsonResult
    ic_utils::Json::Value jsonResult = jsonData["Data"];

    // Assigning the value of "value" member to the variable resString
    std::string strResString = jsonResult["value"].asString();

    // Expecting the same value set with the payload
    EXPECT_STREQ("150", strResString.c_str());
}

TEST_F(CIgniteEventTest, Test_getBizTransactionId_using_addBizTransaction_method)
{
    // Creating an object for IgniteEvent class
    CIgniteEvent igniteEventObj;

    // Assigning the random value to the variable
    std::string strBzTxnId = "812e38c3-c507-488c-8bfd-dda85dc16e2d";

    // Setting the value to the member variable with 'addBizTransaction' member function
    igniteEventObj.AddBizTransaction(strBzTxnId);

    // Expecting the same bzTxnId set with 'addBizTransaction'
    EXPECT_EQ(igniteEventObj.GetBizTransactionId(),strBzTxnId);
}

TEST_F(CIgniteEventTest, Test_getMessageId_using_addMessageId)
{
    // Creating an object for IgniteEvent class
    CIgniteEvent igniteEventObj;

    // Assigning the random value to the variable
    const std::string strMsgId = "8b002add-4daf-4ec1-9f6e-4f7ebef993af";

    // Setting the value to the member variable with 'addMessageId' member function
    igniteEventObj.AddMessageId(strMsgId);

    // Expecting the same msgId set with 'addMessageId'
    EXPECT_EQ(igniteEventObj.GetMessageId(), strMsgId);
}

TEST_F(CIgniteEventTest, Test_getCorrelationId_using_addCorrelationId)
{
    // Creating an object for IgniteEvent class
    CIgniteEvent igniteEventObj;

    // Assigning the random value to the variable
    std::string strCorrId = "812e38c3-c507-488c-8bfd-dda85dc16e2c";

    // Setting the value to the member variable with 'addCorrelationId' member function
    igniteEventObj.AddCorrelationId(strCorrId);

    // Expecting the same corrId set with 'addCorrelationId'
    EXPECT_EQ(igniteEventObj.GetCorrelationId(), strCorrId);
}

TEST_F(CIgniteEventTest, Test_getJson_using_addFieldAsRawJsonString)
{
    // Creating an object for IgniteEvent class
    CIgniteEvent igniteEventObj;

    // Assigning the random value to the variable
    std::string strId = "RPM";

    // Assigning the random value to the variable
    const std::string strName = "presetAdvance";

    // Creating the json payload in string format
    std::string strJsonValue = "{\"Data\":{\"value\":1000},\"EventID\":\"RPM\",\"Timestamp\":1649851984309,\"Timezone\":330,\"Version\":\"1.0\"}";

    // Setting the value to the member variable with 'addFieldAsRawJsonString' member function
    igniteEventObj.AddFieldAsRawJsonString(strName, strJsonValue);

    // Getting the json payload send with the the key 'presetAdvance'
    ic_utils::Json::Value jsonPayload = igniteEventObj.GetJson("presetAdvance");

    // Expecting the same event ID(set with the payload) returned by the API 'getJson'
    EXPECT_EQ(jsonPayload["EventID"].asString(),"RPM");

    // Expecting the same Timestamp(set with the payload) returned by the API 'getJson'
    EXPECT_EQ(jsonPayload["Timestamp"].asInt64(),1649851984309);

    // Expecting the same Version(set with the payload) returned by the API 'getJson'
    EXPECT_EQ(jsonPayload["Version"].asString(),"1.0");

    // Expecting the same Timezone(set with the payload) returned by the API 'getJson'
    EXPECT_EQ(jsonPayload["Timezone"].asInt(),330);
}

TEST_F(CIgniteEventTest, Test_getattachFileStatus_using_attachFile_emptyPath)
{
    // Creating an object for IgniteEvent class
    CIgniteEvent igniteEventObj;

    // Assigning the file type to the variable
    const std::string& rstrFileType = "txt";

    // Assigning the value and empty file path to the variable
    const bool bCompressFile = true;
    const bool bDeleteAttachment = false;
    std::string strPath = "";

    // Attaching the file to the event.
    igniteEventObj.AttachFile(rstrFileType,strPath,bCompressFile,bDeleteAttachment);

    // Sending the event to the receiver to process the attachment
    igniteEventObj.Send();

    // Expecting the API to return the 'file does not exists' status as empty file path
    EXPECT_EQ(igniteEventObj.GetAttachFileStatus(strPath),ERROR_NOFILE);
}

TEST_F(CIgniteEventTest, Test_getattachFileStatus_using_clearAttachMentDetails)
{
    // Creating an object for IgniteEvent class
    CIgniteEvent igniteEventObj;

    // Assigning the file type to the variable
    const std::string& rstrFileType = "DVFSLog";

    // Assigning the value and empty file path to the variable
    std::string strPath = "/tmp/dvfs.log";
    const bool bCompressFile = true;
    const bool bDeleteAttachment = false;

    // Attaching the file to the event.
    igniteEventObj.AttachFile(rstrFileType,strPath,bCompressFile,bDeleteAttachment);

    // To clear the file attachment details
    igniteEventObj.ClearAttachMentDetails();

    // Expecting the API to return the 'file does not exists' status as attachment is cleared
    EXPECT_EQ(igniteEventObj.GetAttachFileStatus(strPath),ERROR_NOFILE);
}

TEST_F(CIgniteEventTest, Test_GetFileNameAndExtension_filename_with_extn)
{
    // Creating an test object
    CIgniteEventTest tstObj;

    //for testing, use a dummy file path with an extension
    std::string strPath = "/tmp/mytest.log";

    std::string strExtn = "";
    std::string strFilename = "";


    //call the API to get the filename and extn
    tstObj.GetFileNameAndExtension(strPath, strExtn, strFilename);

    //based on the used file path,
    //  expect 'log' as extension and 'mytest' as filename
    EXPECT_EQ("log",strExtn);
    EXPECT_EQ("mytest",strFilename);
}

TEST_F(CIgniteEventTest, Test_GetFileNameAndExtension_filename_without_extn)
{
    // Creating an test object
    CIgniteEventTest tstObj;

    //for testing, use a dummy file path without an extension
    std::string strPath = "/tmp/mytest2";

    std::string strExtn = "";
    std::string strFilename = "";

    //call the API to get the filename and extn
    tstObj.GetFileNameAndExtension(strPath, strExtn, strFilename);

    //based on the used file path,
    //  expect empty extension and 'mytest2' as filename
    EXPECT_EQ("",strExtn);
    EXPECT_EQ("mytest2",strFilename);
}

TEST_F(CIgniteEventTest, Test_GetExistingAttachmentSize_valid_path)
{
    // Creating an test object
    CIgniteEventTest tstObj;

    CClientInfo info;

    const std::string FILE_ATTACHMENT_PATH =
        "DAM.Upload.CurlSender.fileAttachmentTempPath";

    std::string strAttachTempPath = info.GetConfigValue(FILE_ATTACHMENT_PATH);

    //attachment path should not be empty to proceed
    ASSERT_FALSE((""==strAttachTempPath));

    //expect size 0 or more as either no attachment files will be found or
    //   valid files will be found. In either case, value 0 or greater
    //   should be received
    EXPECT_EQ(0,tstObj.GetExistingAttachmentSize(strAttachTempPath));
}

TEST_F(CIgniteEventTest, Test_ValidateOutFileName_exceed_attachment_limit)
{
    // Creating an test object
    CIgniteEventTest tstObj;

    //call the API for testing attachment limit exceed case
    std::string strOutFileName = "";
    int nErrorCode = tstObj.ValidateOutFileName(eATTACHMENT_LIMIT_EXCEED,
                                                               strOutFileName);

    //expecting the limit exceed related file name and error code
    EXPECT_EQ("_ATTACHLIMIT", strOutFileName);
    EXPECT_EQ(ERROR_ATTACHLIMIT, nErrorCode);
}

TEST_F(CIgniteEventTest, Test_ValidateOutFileName_empty_attachment_path)
{
    // Creating an test object
    CIgniteEventTest tstObj;

    //call the API for testing empty attachment path case
    std::string strOutFileName = "";
    int nErrorCode = tstObj.ValidateOutFileName(eATTACHMENT_PATH_EMPTY,
                                                        strOutFileName);

    //expecting the corresponding error code and filename
    EXPECT_EQ("_NOCONFIG", strOutFileName);
    EXPECT_EQ(ERROR_NOCONFIG, nErrorCode);
}

TEST_F(CIgniteEventTest, Test_ValidateOutFileName_empty_device_id)
{
    // Creating an test object
    CIgniteEventTest tstObj;

    //call the API for testing empty device id case
    std::string strOutFileName = "";
    int nErrorCode = tstObj.ValidateOutFileName(eDEVICE_ID_EMPTY,
                                                        strOutFileName);

    //expecting the corresponding error code and filename
    EXPECT_EQ("_NODEVICEID", strOutFileName);
    EXPECT_EQ(ERROR_NODEVICEID, nErrorCode);
}

TEST_F(CIgniteEventTest, Test_ValidateOutFileName_large_file)
{
    // Creating an test object
    CIgniteEventTest tstObj;

    //call the API for testing large file case
    std::string strOutFileName = "";
    int nErrorCode = tstObj.ValidateOutFileName(eLAREG_FILE,
                                                        strOutFileName);

    //expecting the corresponding error code and filename
    EXPECT_EQ("_FILETOOLARGE", strOutFileName);
    EXPECT_EQ(ERROR_FILETOOLARGE, nErrorCode);
}

TEST_F(CIgniteEventTest, Test_ValidateOutFileName_total_overlimit)
{
    // Creating an test object
    CIgniteEventTest tstObj;

    //call the API for testing total overlimit case
    std::string strOutFileName = "";
    int nErrorCode = tstObj.ValidateOutFileName(eTOTAL_OVERLIMIT,
                                                        strOutFileName);

    //expecting the corresponding error code and filename
    EXPECT_EQ("_TOTALOVERLIMIT", strOutFileName);
    EXPECT_EQ(ERROR_TOTALOVERLIMIT, nErrorCode);
}

TEST_F(CIgniteEventTest, Test_ValidateOutFileName_underscore_rejection)
{
    // Creating an test object
    CIgniteEventTest tstObj;

    //call the API for testing invalid filetype or event id where
    //  underscore will be rejected.
    std::string strOutFileName = "";
    int nErrorCode = tstObj.ValidateOutFileName(eUNDERSCORE,
                                                        strOutFileName);

    //expecting the corresponding error code and filename
    EXPECT_EQ("_UNDERSCOREREJECTED", strOutFileName);
    EXPECT_EQ(ERROR_UNDERSCOREREJECTED, nErrorCode);
}

TEST_F(CIgniteEventTest, Test_AddExtnToOutFileName_valid_extension)
{
    // Creating an test object
    CIgniteEventTest tstObj;

    //call the API for testing invalid filetype or event id where
    //  underscore will be rejected.
    std::string strOutFileName = "MyFile";
    tstObj.AddExtnToOutFileName("abc", strOutFileName);

    //expecting the corresponding error code and filename
    EXPECT_EQ("MyFile.abc", strOutFileName);
}


} //namespace
