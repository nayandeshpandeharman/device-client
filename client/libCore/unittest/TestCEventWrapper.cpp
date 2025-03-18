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
#include "dam/CEventWrapper.h"
#include "CIgniteLog.h"

//! Macro for CEventWrapper test class
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "test_CEventWrapper"

namespace ic_core
{
/**
 * Class CEventWrapperTest defines a test feature for CEventWrapper class
 */
class CEventWrapperTest : public ::testing::Test
{
public:
    /**
     * Method to set and get the "EventID"
     * @param[in] strId String containing EventID
     * @return String set as EventID
     */
    std::string SetAndGetEventId(const std::string strId);

    /**
     * Method to set and get the PII string
     * @param[in] rstrName String containing PII field key
     * @param[in] rstrVal String containing PII field value
     * @return PII string value
     */
    std::string SetAndGetStrPII(const std::string &rstrName, 
                                const std::string &rstrVal);

    /**
     * Method to set and get the PII JSon
     * @param[in] rstrName String containing PII field key
     * @param[in] rstrVal JSON object containing PII field value
     * @return PII JSON object value
     */
    ic_utils::Json::Value SetAndGetJsonPII(const std::string &rstrName, 
                                        const ic_utils::Json::Value &rjsonVal);

    /**
     * Method to check is Attachments are cleared.
     * @param void
     * @return true, if Attachments are cleared. 
     *         false, otherwise.
     */
    bool RemoveAttachments();
protected:
    /**
     * Constructor
     */
    CEventWrapperTest()
    {
        // Do nothing
    }

    /**
     * Destructor
     */ 
    ~CEventWrapperTest() override
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
     * @see testing::Test::TearDown()
     */
    void TearDown() override
    {
        // Do nothing
    }
};

std::string CEventWrapperTest::SetAndGetEventId(const std::string strId)
{
    // Creating an object for CEventWrapper class
    CEventWrapper eventWrapperObj;

    // Set an EventId
    eventWrapperObj.SetEventId(strId);

    // Fetch the desired value
    std::string strFetchedId = 
        eventWrapperObj.m_jsonEventFields[ic_event::EVENT_ID_TAG].asString();
    
    return strFetchedId;
}

std::string CEventWrapperTest::SetAndGetStrPII(const std::string &rstrName, 
                                           const std::string &rstrVal)
{
    // Creating an object for CEventWrapper class
    CEventWrapper eventWrapperObj;

    // Add a PII field
    eventWrapperObj.AddPiiField(rstrName, rstrVal);

    // Fetch the desired value
    std::string strFetchedVal = 
        eventWrapperObj.m_jsonPiiFields[rstrName].asString();
    
    return strFetchedVal;
}

ic_utils::Json::Value CEventWrapperTest::SetAndGetJsonPII
                                    (const std::string &rstrName, 
                                     const ic_utils::Json::Value &rjsonVal)
{
    // Creating an object for CEventWrapper class
    CEventWrapper eventWrapperObj;

    // Add a PII field
    eventWrapperObj.AddPiiField(rstrName, rjsonVal);

    // Fetch the desired value
    ic_utils::Json::Value jsonFetchedVal = 
        eventWrapperObj.m_jsonPiiFields[rstrName];
    
    return jsonFetchedVal;
}

bool CEventWrapperTest::RemoveAttachments()
{
    // Creating an object for CEventWrapper class
    CEventWrapper eventWrapperObj;

    // Push few Attachments and then try to remove it
    eventWrapperObj.m_vectAttachment.push_back("somethingToFillTheVect");
    eventWrapperObj.m_vectAttachment.push_back("somethingMoveToFillTheVect");

    // RemoveAttachments is expected to clear m_vectAttachment
    eventWrapperObj.RemoveAttachments();

    return (eventWrapperObj.m_vectAttachment.empty());
}

TEST_F(CEventWrapperTest, Test_GetInt_defaultCase)
{
    // Creating an object for CEventWrapper class
    CEventWrapper eventWrapperObj;

    /* Expecting the API to return default value (0) as key("volumeChange") 
     * is not added
     */
    EXPECT_EQ(eventWrapperObj.GetInt("volumeChange"), 0);
}

TEST_F(CEventWrapperTest, Test_GetBool_defaultCase)
{
    // Creating an object for CEventWrapper class
    CEventWrapper eventWrapperObj;

    /* Expecting the API to return false value as key("presetAdvance") 
     * is not added
     */
    EXPECT_FALSE(eventWrapperObj.GetBool("presetAdvance"));
}

TEST_F(CEventWrapperTest, Test_GetString_defaultCase)
{
    // Creating an object for CEventWrapper class
    CEventWrapper eventWrapperObj;

    /* Expecting the API to return empty value as key("presetSeekUp") 
     * is not added
     */
    EXPECT_EQ("", eventWrapperObj.GetString("presetSeekUp"));
}

TEST_F(CEventWrapperTest, Test_GetLong_defaultCase)
{
    // Creating an object for CEventWrapper class
    CEventWrapper eventWrapperObj;

    /* Expecting the API to return default value (0) as key("modeAdvance") 
     * is not added
     */
    EXPECT_EQ(eventWrapperObj.GetLong("modeAdvance"), 0);
}

TEST_F(CEventWrapperTest, Test_Hasfield)
{
    // Creating an object for CEventWrapper class
    CEventWrapper eventWrapperObj;

    /* Expecting the API to return default value (0) as key("modeAdvance") 
     * is not added
     */
    EXPECT_EQ(eventWrapperObj.Hasfield("modeAdvance"), 0);
}

TEST_F(CEventWrapperTest, Test_SetEventId)
{
    // Define a test event string
    std::string strId = "TestEventId";

    // Expect provided EventID value and fetched value to be equal  
    EXPECT_EQ(strId, SetAndGetEventId(strId));
}

TEST_F(CEventWrapperTest, Test_AddPiiField_strVal)
{
    // Create a test PII field
    std::string strname = "SomeConfidentialName";
    std::string strVal = "SomeConfidentialVal";

    // Expect provided PII value and fetched value to be equal 
    EXPECT_EQ(strVal, SetAndGetStrPII(strname, strVal));
}

TEST_F(CEventWrapperTest, Test_AddPiiField_JsonVal)
{
    // Create a test PII field
    std::string strname = "SomeConfidentialName";
    ic_utils::Json::Value jsonVal = ic_utils::Json::objectValue;
    jsonVal["SomeConfidentialVal"] = 10000;

    // Expect provided PII value and fetched value to be equal 
    EXPECT_EQ(jsonVal, SetAndGetJsonPII(strname, jsonVal));
}

TEST_F(CEventWrapperTest, Test_RemoveAttachments)
{
    /* Expect RemoveAttachments to return true,
     * in case of successful removal.
     */
    EXPECT_TRUE(RemoveAttachments());
}

} // namespace ic_core