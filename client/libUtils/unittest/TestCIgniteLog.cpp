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
#include <fstream>
#include "gtest/gtest.h"
#include "CIgniteFileUtils.h"
#include "CIgniteLog.h"

//! Constant key for new directory
static const std::string NEWDIR = "NewDir";

//! Constant key for log file name
static const std::string LOGNAME = "acp.log";

//! Constant key for log file with .bak extension
static const std::string LOGWITHBAKEXTN = ".bak";

//! Constant key for new log file name
static const std::string NEWLOGNAME = "acpNew.log";

namespace ic_utils
{
//! Define a test fixture for CIgniteLogTest
class CIgniteLogTest : public ::testing::Test 
{
protected:
   /**
    * Constructor
    */
   CIgniteLogTest() 
   {
      // do nothing
   }

   /**
    * Destructor
    */ 
   ~CIgniteLogTest() override 
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

public:
    /**
     * Wrapper method to access the GetTruncateLogValue of CIgniteLog
     * @param void
     * @return True if flag is set true else False
     */
   bool GetTruncateLogStatus();
};

bool CIgniteLogTest::GetTruncateLogStatus()
{
   CIgniteLog igniteLogObj;
   return igniteLogObj.GetTruncateLogStatus();
}

//Tests

TEST_F(CIgniteLogTest, Test_GetFileOutputLevel) 
{
   //Setting the log level to Trace
   CIgniteLog::SetFileOutputLevel(eHCP_LOG_TRACE);

   //Expect Fetched File Output Level equal to the value set to eHCP_LOG_TRACE
   EXPECT_EQ(eHCP_LOG_TRACE, CIgniteLog::GetFileOutputLevel());
}

TEST_F(CIgniteLogTest, Test_SetTruncateFileSize) 
{
   /* Expect SetTruncateFileSize return true as the value set 
    * to TrucateFileSize > 0
    */
   EXPECT_EQ(true, CIgniteLog::SetTruncateFileSize(5000000));
}

TEST_F(CIgniteLogTest, Test_GetTruncateFileSize) 
{
   // Setting the Truncate File Size to 5000000
   CIgniteLog::SetTruncateFileSize(5000000);

   /* Expect GetTruncateFileSize to return 5000000 as TrucateFileSize  is 
    * set to 5000000
    */ 
   EXPECT_EQ(5000000, CIgniteLog::GetTruncateFileSize());
}

TEST_F(CIgniteLogTest, Test_SetTruncateToSize) 
{
   /* Expect SetTruncateToSize return true as the value set 
    * to TrucateFileSize > 0
    */
   EXPECT_EQ(true, CIgniteLog::SetTruncateToSize(3000000));
}

TEST_F(CIgniteLogTest, Test_GetTruncateToSize) 
{
   // Setting the Truncate To Size to 3000000
   CIgniteLog::SetTruncateToSize(3000000);

   // Expect GetTruncateToSize return 3000000 as TrucateFileSize set to 3000000
   EXPECT_EQ(3000000, CIgniteLog::GetTruncateToSize());
}

// Test Case to update log file to new directory Path and new filename
TEST_F(CIgniteLogTest, Test_UpdateFilePath_NewDirectoryAndFilename)
{
   // Fetching the existing File Path
   std::string strFilePath = CIgniteLog::GetFileOutputPath();

   // Fetching strParentDir from strFilePath
   std::string strParentDir = strFilePath.substr(0, 
                                                 strFilePath.find_last_of("/"));

   // New Directory Path from Parent Directory
   std::string strNewDir = strParentDir + "/" + NEWDIR;

   // New file Log path
   std::string strNewPath = strNewDir + "/" + LOGNAME;

   /* Expect true as we are moving file contents from existing file to
    * new file in different location
    */ 
   EXPECT_TRUE(CIgniteLog::UpdateFilePath(strNewPath));

   //Expect false as log file is moved with name LOGNAME in strNewDir directory
   EXPECT_FALSE(CIgniteFileUtils::Exists(strFilePath));

   /* Expect true as we are moving new log file back to previous strFilePath 
    * just to keep log in existing directory
    */ 
   EXPECT_TRUE(CIgniteLog::UpdateFilePath(strFilePath));

   // Removing newly created directory
   CIgniteFileUtils::RemoveDirectory(strNewDir);
}

//Test Case to update log file to existing directory Path and existing filename
TEST_F(CIgniteLogTest, Test_UpdateFilePath_ExistingDirectoryAndFilename)
{
   // Fetching the existing File Path
   std::string strFilePath = CIgniteLog::GetFileOutputPath();

   // Fetching strParentDir from strFilePath
   std::string strParentDir = strFilePath.substr(0, 
                                                 strFilePath.find_last_of("/"));

   // New Directory Path from Parent Directory
   std::string strNewDir = strParentDir + "/" + NEWDIR;

   // New file Log path
   std::string strNewPath = strNewDir + "/" + LOGNAME;

   // Log file with new path with .bak extension
   std::string strBakFileInNewPath = strNewPath + LOGWITHBAKEXTN;

   // Creating the new Directory
   CIgniteFileUtils::MakeDirectory(strNewDir);

   // New file ptr to create the file in new directory
   std::ofstream of;

   // Opening a new file with newly created path
   of.open(strNewPath.c_str(), std::ios::app);

   // Continue other tests if of is in open mode only
   ASSERT_TRUE(of.is_open());

   // Closing the opened file
   of.close();

   /* Expect true as we are moving file contents from existing file to new file
    * in different location
    */ 
   EXPECT_TRUE(CIgniteLog::UpdateFilePath(strNewPath));

   /* Expect true as file already present in strNewDir will be moved with
    * .bak extension
    */ 
   EXPECT_TRUE(CIgniteFileUtils::Exists(strBakFileInNewPath));

   // Expect false as log file is moved with name LOGNAME in strNewDir directory
   EXPECT_FALSE(CIgniteFileUtils::Exists(strFilePath));

   /* Expect true as we are moving new log file back to previous file path 
    * just to keep log in existing directory
    */
   EXPECT_TRUE(CIgniteLog::UpdateFilePath(strFilePath));

   // Removing newly created directory
   CIgniteFileUtils::RemoveDirectory(strNewDir);
}

//Test Case to update log file to existing file path
TEST_F(CIgniteLogTest, Test_UpdateFilePath_ExistingPath)
{
   // Fetching the existing File Path
   std::string strFilePath = CIgniteLog::GetFileOutputPath();

   // Expect false as new path and existing log file path are same
   EXPECT_FALSE(CIgniteLog::UpdateFilePath(strFilePath));
}

//Test Case to update log file to empty file path
TEST_F(CIgniteLogTest, Test_UpdateFilePath_EmptyPath)
{
   // Initializing file path with empty string
   std::string strFilePath = "";

   // Expect false as new path is empty
   EXPECT_FALSE(CIgniteLog::UpdateFilePath(strFilePath));
}

//Test Case to update log file to same directory Path and new filename
TEST_F(CIgniteLogTest, Test_UpdateFilePath_SameDirectoryAndNewFilename)
{
   // Fetching the existing File Path
   std::string strFilePath = CIgniteLog::GetFileOutputPath();

   // Fetching strParentDir from strFilePath
   std::string strParentDir = strFilePath.substr(0, 
                                                 strFilePath.find_last_of("/"));

   // New file Log path
   std::string strNewPath = strParentDir + "/" + NEWLOGNAME;

   /* Expect true as we are moving file contents from existing file to new file 
    * in the same location
    */
   EXPECT_TRUE(CIgniteLog::UpdateFilePath(strNewPath));

   // Expect false as log file is moved with new filename in same directory
   EXPECT_FALSE(CIgniteFileUtils::Exists(strFilePath));

   /* Expect true as we are moving new log file back to previous strFilePath  
    * just to keep log in existing directory
    */
   EXPECT_TRUE(CIgniteLog::UpdateFilePath(strFilePath));
}

// Test Case to check the initialization of diagheader with Empty string
TEST_F(CIgniteLogTest,Test_InitDiagHeader_EmptyDiagHeaderInitCheck)
{
    std::vector<std::string> vectInitArray;

   // When empty string is passed, InitDiagHeader is supoosed to return false
   bool bOutput = CIgniteLog::InitDiagHeader(vectInitArray);

   EXPECT_FALSE(bOutput);
}

// Test Case to check the initialization of diagheader with valid strings
TEST_F(CIgniteLogTest,Test_InitDiagHeader_DiagHeaderInitializationCheck)
{
   std::vector<std::string> vectInitArray;
   vectInitArray.push_back("test1");
   vectInitArray.push_back("test2");
   vectInitArray.push_back("test3");

   // when valid strings are passed, InitDiagHeader is supoosed to return true
   bool bOutput = CIgniteLog::InitDiagHeader(vectInitArray);

   EXPECT_TRUE(bOutput);
}

/* Test Case to check the initialization of diagheader with entries more that
 * MAX_DIAG_ENTRIES
 */  
TEST_F(CIgniteLogTest, Test_InitDiagHeader_ExactMaxEntryInitCheck)
{
   std::vector<std::string> vectInitArray;
   std::string strTestValue = "test";

   // Exact max entries are used to init the diag header . 
   for (int i = 0; i < (CIgniteLog::GetMaxDiagEntryValue()); i++)
   {
     std::string strPushVal = strTestValue + std::to_string(i);
     vectInitArray.push_back(strPushVal);
   }

   /* When the entries sent for the InitDiagHeader with exact MAX_DIAG_ENTRIES, 
    * function will return true
    */ 
   bool bOutput = CIgniteLog::InitDiagHeader(vectInitArray);

   // Printing on console to verify the information
   CIgniteLog::PrintDiagHeader();

   EXPECT_TRUE(bOutput);
}

/* Test Case to check the initialization of diagheader with entries more that 
 * MAX_DIAG_ENTRIES
 */
TEST_F(CIgniteLogTest,Test_InitDiagHeader_MoreThanMaxEntryInitCheck)
{
   std::vector<std::string> vectInitArray;
   std::string strTestValue = "test";

   /* Max entries + 5 is used to create a vector which has more than 
    * MAX_DIAG_ENTRIES entries, 5 is used as random number, 
    * other values also can be used
    */
   for (int i = 0; i < (CIgniteLog::GetMaxDiagEntryValue() + 5); i++)
   {
      std::string strPushVal = strTestValue + std::to_string(i);
      vectInitArray.push_back(strPushVal);
   }

   /* When the entries sent for the InitDiagHeader are more than ,
    * MAX_DIAG_ENTRIES. Init finction will use last MAX_DIAG_ENTRIES entries 
    * from iniput vector to initialize , and then function will return true
    */ 
   bool bOutput = CIgniteLog::InitDiagHeader(vectInitArray);

   // Printing on console to verify the information
   CIgniteLog::PrintDiagHeader();

   EXPECT_TRUE(bOutput);
}

// Test Case to check the updation of diagheader with empty string
TEST_F(CIgniteLogTest,Test_UpdateDiagHeader_updateWithEmptyString)
{
   std::string strTestValue;

   // When empty string is passed updation function will return false
   bool bRetVal = CIgniteLog::UpdateDiagHeader(strTestValue);

   EXPECT_FALSE(bRetVal);
}

// Test Case to check the updation of diagheader with valid string
TEST_F(CIgniteLogTest, Test_UpdateDiagHeader_updateWithString)
{
   std::string strTestValue = "123456789, MQTT, FATAL, \
                           divairbvwjrwiayrvbjwhrbfvlwhybvlawb";

   // When valid string is passed updation function will return true
   bool bRetVal = CIgniteLog::UpdateDiagHeader(strTestValue);

   EXPECT_TRUE(bRetVal);
}

// Test case to update diag header with strings more than MAX_DIAG_ENTRIES
TEST_F(CIgniteLogTest, Test_UpdateDiagHeader_updateWithMultipleString)
{
   std::string strTestValue = "123456789, MQTT, FATAL, \
                           divairbvwjrwiayrvbjwhrbfvlwhybvlawb";
   bool bRetVal;

   /* Max entries + 4 extra entries are used to check the behavior when header 
    * is updated with entries which  are beyond max allowed entries, 4 is
    * randomly used to see the updation , any other nnumber also can be used
    */
   for (int i = 0; i < ( CIgniteLog::GetMaxDiagEntryValue() + 4); i++)
   {
      std::string strUpdateString = strTestValue + std::to_string(i);
      // When valid string is passed updation function will return true
      bRetVal = CIgniteLog::UpdateDiagHeader(strUpdateString);
   }

   EXPECT_TRUE(bRetVal);
}

// Test Case to validate the printing of diagheader
TEST_F(CIgniteLogTest, Test_PrintDiagHeader_DiagHeaderPrintCheck)
{
   //On succesful printing print function will return true
   bool bRetVal = CIgniteLog::PrintDiagHeader();

   EXPECT_TRUE(bRetVal);
}

// Test Case to validate the get max diag entries
TEST_F(CIgniteLogTest, Test_GetMaxDiagEntryValue_validateDefaultMaxDiagEnties)
{
   unsigned int nRetVal = CIgniteLog::GetMaxDiagEntryValue();

   /* since default value of diag entries is 20 , it is expected to return 20
    * when called before setting a value
    */ 
   EXPECT_EQ(20,nRetVal);
}

// Test Case to set the get max diag entries
TEST_F(CIgniteLogTest, Test_SetMaxDiagEntryValue_settingValidValue)
{
   bool bRetVal = CIgniteLog::SetMaxDiagEntryValue(25);

   /* On succesfully updating the diag entries value , get is expected to return 
    * the number which is set
    */ 
   EXPECT_EQ(25, CIgniteLog::GetMaxDiagEntryValue());
}

/* Test Case to validate the SetMaxDiagEntryValue when value is set beyond  
 * upper limit
 */ 
TEST_F(CIgniteLogTest, Test_SetMaxDiagEntryValue_settingValueBeyondUpperLimit)
{
   bool bRetVal = CIgniteLog::SetMaxDiagEntryValue(65);

   /* To not overrun the header diag entries , upper limit has been defined with 
    * value as 30, any value being tried to be set above upper limit will be 
    * rejected, and set func will return false
    */ 
   EXPECT_FALSE(bRetVal);
}

/* Test Case to validate the SetMaxDiagEntryValue when value is set 
 * beyond upper limit
 */ 
TEST_F(CIgniteLogTest, Test_SetMaxDiagEntryValue_settingValueBelowLowerLimit)
{
   bool bRetVal = CIgniteLog::SetMaxDiagEntryValue(0);

   /* To not underrun the header diag entries , lower limit has been defined 
    * with value as 5, any value being tried to be set below lower limit
    * will be rejected , and set func will return false
    */ 
   EXPECT_FALSE(bRetVal);
}

// Test Case to validate the printing of diagheader
TEST_F(CIgniteLogTest, Test_ResetDiagHeader_DiagHeaderResetCheck)
{
   // On succesful printing print function will return true
   CIgniteLog::ResetDiagHeader();

   // Printing on console to verify the information
   bool bRetVal = CIgniteLog::PrintDiagHeader();

   EXPECT_TRUE(bRetVal);
}

// Test Case to set and get the Log level
TEST_F(CIgniteLogTest, Test_settingAndGettingLogLevel)
{
   // Setting log level as Info
   LogLevel logLevel = LogLevel::eHCP_LOG_NONE;
   CIgniteLog::SetReportingLevel(logLevel);

   // Expecting log level returned to be same as the one that is set
   EXPECT_EQ(logLevel, CIgniteLog::GetReportingLevel());
}

// Test Case to disable the truncate log flag
TEST_F(CIgniteLogTest, Test_TruncateLogStatus_Disable)
{
   // Disabling truncate log
   CIgniteLog::DisableTruncate();

   // Expecting truncate log flag to return false
   EXPECT_FALSE(GetTruncateLogStatus());
}

// Test Case to enable the truncate log flag
TEST_F(CIgniteLogTest, Test_TruncateLogStatus_Enabled)
{
   // Enabling truncate log
   CIgniteLog::EnableTruncate();

   // Expecting truncate log flag to return true
   EXPECT_TRUE(GetTruncateLogStatus());
}

// Test case to move file to upload with empty file path
TEST_F(CIgniteLogTest, Test_MoveToUpload_emptyLogPath_compressedTrue)
{
   // Setting values for log file as empty, upload path and compress value true
   std::string strLogPath = "";
   std::string strUploadPath = "./TestUT";
   bool bCompress = true;

   // Expecting false as log file is not moved to upload as log path is empty 
   EXPECT_FALSE(CIgniteLog::MoveToUpload(strLogPath, strUploadPath, bCompress));
}

// Test case to move file to upload with valid file path
TEST_F(CIgniteLogTest, Test_MoveToUpload_validLogPath_compressedFalse)
{
   // Create a files and writing data into it to execute tests on it
   std::string strLogPath = "./testFileSrc.log";
   std::ofstream file(strLogPath);
   file << "Adding data into this testSrc file for testing purpose";
   std::string strUploadPath = "./testFileDestination.log";
   file.close();

   /* Setting compress flag as false so that the log file does not get removed
    * after successful compression
    */
   bool bCompress = false;

   // Expecting true as log file is moved to upload path
   EXPECT_TRUE(CIgniteLog::MoveToUpload(strLogPath, strUploadPath, bCompress));

   // Removing newly created files
   CIgniteFileUtils::Remove(strLogPath);
   CIgniteFileUtils::Remove(strUploadPath);
}

// Test case to move file to upload with valid file path
TEST_F(CIgniteLogTest, Test_MoveToUpload_validLogPath_compressedTrue)
{
   // Create a files and writing data into it to execute tests on it
   std::string strLogPath = "./testFileSrc.log";
   std::ofstream file(strLogPath);
   file << "Adding data into this testSrc file for testing purpose";
   std::string strUploadPath = "./testFileDestination.log";
   file.close();
   bool bCompress = true;

   // Expecting true as log file is moved to upload path
   EXPECT_TRUE(CIgniteLog::MoveToUpload(strLogPath, strUploadPath, bCompress));

   // Removing newly created files
   CIgniteFileUtils::Remove(strUploadPath);
}

}