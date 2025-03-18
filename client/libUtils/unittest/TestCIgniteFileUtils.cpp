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

#include <zlib.h>
#include <iostream>
#include <fstream>
#include "gtest/gtest.h"
#include "CIgniteFileUtils.h"
#include "CIgniteLog.h"

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "test_CIgniteFileUtils"

namespace ic_utils 
{
//! Global variable to store instance of CIgniteFileUtils
CIgniteFileUtils *g_pFileUtilsObj = nullptr;

//! Define a test fixture for CIgniteFileUtils
class CIgniteFileUtilsTest : public ::testing::Test
{
public:
    //! string to store file name
    std::string m_strFileName = "./TestFile.txt";

    /**
     * Wrapper method for Copy of CIgniteFileUtils class
     * @see CIgniteFileUtils::Copy()
     */
    static int Copy(const std::string &rstrFrom, const std::string &rstrTo);

    /**
     * Wrapper method for Remove of CIgniteFileUtils class
     * @see CIgniteFileUtils::Remove()
     */
    static int Remove(const std::string &rstrFilename);

    /**
     * Wrapper method for Exists of CIgniteFileUtils class
     * @see CIgniteFileUtils::Exists()
     */
    static bool Exists(const std::string &rstrFilename);

    /**
     * Wrapper method for GetSize of CIgniteFileUtils class
     * @see CIgniteFileUtils::GetSize()
     */
    static int GetSize(const std::string &rstrFilename);

    /**
     * Wrapper method for ListFiles of CIgniteFileUtils class
     * @see CIgniteFileUtils::ListFiles()
     */
    static int ListFiles(const std::string &rstrDirName, 
                         std::vector<std::string> &rvectorFileList);

    /**
     * Wrapper method for Base64Decode of CIgniteFileUtils class
     * @see CIgniteFileUtils::Base64Decode()
     */
    static std::string Base64Decode(std::string const &rstrEncodedString);

    /**
     * Wrapper method for Base64Encode of CIgniteFileUtils class
     * @see CIgniteFileUtils::Base64Encode()
     */
    static std::string Base64Encode(std::string const strBytesToEncode, 
                                    unsigned int unInputlength);

    /**
     * Wrapper method for Compress of CIgniteFileUtils class
     * @see CIgniteFileUtils::Compress()
     */
    static int Compress(const std::string &rstrSrcPath);

    /**
     * Wrapper method for Compress of CIgniteFileUtils class
     * @see CIgniteFileUtils::Compress()
     */
    static int Compress(const std::string &rstrSrcPath, 
                        const std::string &rstrDestPath);

    /**
     * Wrapper method for MakeDirectory of CIgniteFileUtils class
     * @see CIgniteFileUtils::MakeDirectory()
     */
    static bool MakeDirectory(std::string &rstrDirPath);

    /**
     * Wrapper method for Move of CIgniteFileUtils class
     * @see CIgniteFileUtils::Move()
     */
    static bool Move(std::string &rstrSrc, std::string &rstrDest);

    /**
     * Wrapper method for RemoveDirectory of CIgniteFileUtils class
     * @see CIgniteFileUtils::RemoveDirectory()
     */
    static bool RemoveDirectory(std::string &rstrDirPath);

    /**
     * Wrapper method for ReadFile of CIgniteFileUtils class
     * @see CIgniteFileUtils::ReadFile()
     */
    static void ReadFile(std::string &rstrFilename, std::string &rstrData);

    /**
     * Wrapper method for StringEndsWith of CIgniteFileUtils class
     * @see CIgniteFileUtils::StringEndsWith()
     */
    static bool StringEndsWith(std::string const &rstrvalue, 
                               std::string const &rstrEnding);

    /**
     * Wrapper method for ListFiles of CIgniteFileUtils class
     * @see CIgniteFileUtils::ListFiles()
     */
    static void ListFiles(const std::string &rstrdir,  
                          const std::string strSuffix,
                          std::list<std::string> &rlistFileList);
    
    /**
     * Wrapper method for GetFileName of CIgniteFileUtils class
     * @see CIgniteFileUtils::GetFileName()
     */
    static std::string GetFileName(const std::string &rstrPath, 
                                   bool bWithoutExtension);

    /**
     * Wrapper method for IsFile of CIgniteFileUtils class
     * @see CIgniteFileUtils::IsFile()
     */
    static bool IsFile(const std::string &rstrFilePath);

    /**
     * Wrapper method for IsDirectory of CIgniteFileUtils class
     * @see CIgniteFileUtils::IsDirectory()
     */
    static bool IsDirectory(const std::string &rstrDirPath);

protected:
    /**
     * Constructor
     */
    CIgniteFileUtilsTest()
    {
        // do nothing         
    }

    /**
     * Destructor
     */ 
    ~CIgniteFileUtilsTest() override
    {
        // do nothing  
    }

    /**
     * Overriding Method of testing::Test class
     * @see testing::Test::SetUp()
     */
    void SetUp() override
    {
        /* Expect true in case of successful memory allocation, 
         * will fail in case of bad allocation
         */
        ASSERT_TRUE(InitializeObject());

        // Create a file to execute tests on it
        std::ofstream MyFile(m_strFileName);
        if(!MyFile) 
        { 
            HCPLOG_E<<"Error in creating file for testing CIgniteFileUtils!!!"; 
            GTEST_SKIP(); 
        }
        else
        {
            MyFile << "File to test the UTs for CIgniteFileUtils!";
        }
    }

    /**
     * Overriding Method of testing::Test class
     * @see testing::Test::TearDown()
     */
    void TearDown() override
    {
        // Remove the test file
        if(Exists(m_strFileName))
        {
            Remove(m_strFileName);
        }
        else
        {
            // do nothing
        }

        // Deallocate memory
        if(nullptr != g_pFileUtilsObj)
        {
            delete g_pFileUtilsObj;
            g_pFileUtilsObj = nullptr;
        } 
    }

    /**
     * This method will initialize the object.
     * @param void
     * @return true, in case of successful memory allocation
     *         false, in case of bad allocation.
     */
    bool InitializeObject()
    {
        try
        {
            g_pFileUtilsObj = new ic_utils::CIgniteFileUtils();
        }
        catch(std::bad_alloc& e)
        {
           HCPLOG_E << "bad_alloc caught: " << e.what();
           return false;
        }
        return true;
    }
};

int CIgniteFileUtilsTest::Copy(const std::string &rstrFrom, 
                               const std::string &rstrTo)
{
    return g_pFileUtilsObj->Copy(rstrFrom, rstrTo);
}

int CIgniteFileUtilsTest::Remove(const std::string &rstrFilename)
{
    return g_pFileUtilsObj->Remove(rstrFilename);
}

bool CIgniteFileUtilsTest::Exists(const std::string &rstrFilename)
{
    return g_pFileUtilsObj->Exists(rstrFilename);
}

int CIgniteFileUtilsTest::GetSize(const std::string &rstrFilename)
{
    return g_pFileUtilsObj->GetSize(rstrFilename);
}

int CIgniteFileUtilsTest::ListFiles(const std::string &rstrDirName, 
                                    std::vector<std::string> &rvectorFileList)
{
    return g_pFileUtilsObj->ListFiles(rstrDirName, rvectorFileList);
}

std::string CIgniteFileUtilsTest::Base64Decode(
                                  std::string const &rstrEncodedString)
{
    return g_pFileUtilsObj->Base64Decode(rstrEncodedString);
}

std::string CIgniteFileUtilsTest::Base64Encode(std::string strBytesToEncode, 
                                               unsigned int unInputlength)
{
    const unsigned char* puchStrToEncode = 
        reinterpret_cast<const unsigned char*>(strBytesToEncode.c_str());

    return g_pFileUtilsObj->Base64Encode(puchStrToEncode, unInputlength);
}

bool CIgniteFileUtilsTest::MakeDirectory(std::string &rstrDirPath)
{
    return g_pFileUtilsObj->MakeDirectory(rstrDirPath);
}

bool CIgniteFileUtilsTest::Move(std::string &rstrSrc, std::string &rstrDest)
{
    return g_pFileUtilsObj->Move(rstrSrc, rstrDest);
}

bool CIgniteFileUtilsTest::RemoveDirectory(std::string &rstrDirPath)
{
    return g_pFileUtilsObj->RemoveDirectory(rstrDirPath);
}

void CIgniteFileUtilsTest::ReadFile(std::string &rstrFilename, 
                                    std::string &rstrData)
{
    g_pFileUtilsObj->ReadFile(rstrFilename, rstrData);
}

bool CIgniteFileUtilsTest::StringEndsWith(std::string const &rstrvalue, 
                                          std::string const &rstrEnding)
{
    return g_pFileUtilsObj->StringEndsWith(rstrvalue, rstrEnding);
}

void CIgniteFileUtilsTest::ListFiles(const std::string &rstrdir, 
                                     const std::string strSuffix, 
                                     std::list<std::string> &rlistFileList)
{
    return g_pFileUtilsObj->ListFiles(rstrdir, strSuffix, rlistFileList);
}

std::string CIgniteFileUtilsTest::GetFileName(const std::string &rstrPath, 
                                              bool bWithoutExtension)
{
    return g_pFileUtilsObj->GetFileName(rstrPath, bWithoutExtension);
}

bool CIgniteFileUtilsTest::IsFile(const std::string &rstrFilePath)
{
    return g_pFileUtilsObj->IsFile(rstrFilePath);
}

bool CIgniteFileUtilsTest::IsDirectory(const std::string &rstrDirPath)
{
    return g_pFileUtilsObj->IsDirectory(rstrDirPath);
}

int CIgniteFileUtilsTest::Compress(const std::string &rstrSrcPath)
{
    return g_pFileUtilsObj->Compress(rstrSrcPath);
}

int CIgniteFileUtilsTest::Compress(const std::string &rstrSrcPath, 
                                   const std::string &rstrDestPath)
{
    return g_pFileUtilsObj->Compress(rstrSrcPath, rstrDestPath);
}

// Tests

TEST_F(CIgniteFileUtilsTest, Test_CopyAndRemove) 
{
    // Test Copy method with existing file
    EXPECT_EQ(0, Copy(m_strFileName, "copied.txt"));

    // Test Remove method with existing file
    EXPECT_EQ(0, Remove("copied.txt"));

    // Test Copy method with non existing file
    EXPECT_EQ(-1, Copy("non_existing_file.txt", "output.txt"));
    Remove("output.txt");

    // Test Remove method with non-existing file
    EXPECT_EQ(-1, Remove("non_existing_file.txt"));
}

TEST_F(CIgniteFileUtilsTest, Test_Exists) 
{
    // Test Exists method with existing file
    EXPECT_TRUE(Exists(m_strFileName));

    // Test Exists method with non-existing file
    EXPECT_FALSE(Exists("non_existing_file.txt"));
}

TEST_F(CIgniteFileUtilsTest, Test_GetSize) 
{
    // Test GetSize method with existing file
    EXPECT_GT(GetSize(m_strFileName), 0);

    // Test GetSize method with non-existing file
    EXPECT_EQ(-1, GetSize("non_existing_file.txt"));
}

TEST_F(CIgniteFileUtilsTest, Test_ListFiles) 
{
    // Test ListFiles method with existing directory
    std::vector<std::string> vectFileList;

    ASSERT_EQ(0, ListFiles("./", vectFileList));
    ASSERT_GT(vectFileList.size(), 0);

    // Test ListFiles method with non-existing directory
    vectFileList.clear();

    ASSERT_EQ(-1, ListFiles("non_existing_directory", vectFileList));
    ASSERT_EQ(0, vectFileList.size());
}

TEST_F(CIgniteFileUtilsTest, Test_MakeCheckMoveRemoveDirectory)
{
    std::string strSrcDirPath = "./TestDirectory";
    std::string strDestDirPath = "./TestUT";

    // Test the MakeDirectory method: create a dir
    EXPECT_TRUE(MakeDirectory(strSrcDirPath));

    // Test IsDirectory
    ASSERT_TRUE(IsDirectory(strSrcDirPath));

    // Test Move
    ASSERT_TRUE(Move(strSrcDirPath, strDestDirPath));

    // Test the RemoveDirectory: remove dir if exists
    ASSERT_TRUE(Exists(strDestDirPath));
    EXPECT_TRUE(RemoveDirectory(strDestDirPath));
}

TEST_F(CIgniteFileUtilsTest, Test_CheckAndReadFile) 
{
    // Test IsFile 
    ASSERT_TRUE(IsFile(m_strFileName));

    // ReadFile
    std::string strExpectedData = "File to test the UTs for CIgniteFileUtils!";
    std::string strActualData;
    ReadFile(m_strFileName, strActualData);

    EXPECT_EQ(strActualData, strExpectedData);
}

TEST_F(CIgniteFileUtilsTest, Test_GetFileName_NoSeparator) 
{
    // Test GetFileName with no separator in the input
    std::string strFileName = "TestFile.txt";
    std::string strActualName = GetFileName(strFileName, true);

    EXPECT_EQ("", strActualName);
}

TEST_F(CIgniteFileUtilsTest, Test_GetFileName_WithSeparator) 
{
    // Test GetFileName with separator in the input
    std::string strExpectedName = "TestFile";
    std::string strActualName = GetFileName(m_strFileName, true);

    EXPECT_EQ(strActualName, strExpectedName);
}

TEST_F(CIgniteFileUtilsTest, Test_Base64Decode)
{
    const std::string strSampleText = 
        "This is a sample text to test base64 encode/decode";
    
    std::string strEncodedResult;
    strEncodedResult = Base64Encode(strSampleText, strSampleText.length());
    
    std::string strDecodedResult;
    strDecodedResult = Base64Decode(strEncodedResult);

    // Expect original sample text and decoded text to be equal
    EXPECT_EQ(strSampleText, strDecodedResult);
}

TEST_F(CIgniteFileUtilsTest, Test_CompressSrcDest)
{
    std::string strSrc = m_strFileName;
    std::string strDest = strSrc + ".gz";
    int nCompressResult = Compress(strSrc, strDest);

    // Happy senario, src and dest paths are valid
    EXPECT_EQ(Z_OK, nCompressResult);
    Remove(strDest);
}

TEST_F(CIgniteFileUtilsTest, Test_CompressSrc)
{
    std::string strSrc = m_strFileName;
    int nCompressResult = Compress(strSrc);

    // Happy senario, only src path as parameter
    EXPECT_EQ(Z_OK, nCompressResult);
    Remove(strSrc + ".gz");
}

TEST_F(CIgniteFileUtilsTest, Test_CompressSrcFileError)
{
    std::string strSrc = "./SomeRandomNotExistingFile.txt";
    int nCompressResult = Compress(strSrc);

    // Expect error since src file could not be opened.
    EXPECT_EQ(Z_ERRNO, nCompressResult);
}

TEST_F(CIgniteFileUtilsTest, Test_CompressDestFileError)
{
    std::string strSrc = m_strFileName;
    std::string strDest = "./DirNotExist/";
    int nCompressResult = Compress(strSrc, strDest);

    // Expect error since destination path can not be accesed
    EXPECT_EQ(Z_ERRNO, nCompressResult);
}

TEST_F(CIgniteFileUtilsTest, Test_StringEndsWith)
{
    std::string strEnd = "I am a developer.";
    std::string strInputTrue = "Here there! I am a developer.";
    std::string strInputFalse = "I am a developer and also a Fitness trainer";

    // Test Positive senario for StringEndsWith
    EXPECT_TRUE(StringEndsWith(strInputTrue, strEnd));

    // Test Negative senario for StringEndsWith
    EXPECT_FALSE(StringEndsWith(strInputFalse, strEnd));
}

} /* namespace ic_utils */
