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

#include <gmock/gmock.h>
#include "gtest/gtest.h"
#include "CHttpRequest.h"
#include "../include/http/HttpErrorCodes.h"

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "Test_CHttpRequestTest"

using ::testing::MatchesRegex;

namespace ic_network
{

/**
 * Class CHttpRequestTest defines a test methods for CHttpRequest
 */
class CHttpRequestTest : public ::testing::Test
{
protected:
    /**
     * Constructor
     */
    CHttpRequestTest()
    {
        // do nothing
    }

    /**
     * Destructor
     */
    ~CHttpRequestTest() override
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
     * @see testing::Test::SetUp()
     */
    void TearDown() override
    {
        // do nothing
    }

};

// Tests

TEST_F(CHttpRequestTest, Test_CHttpRequestTest_ZeroUsecase)
{
    CHttpRequest httpReq;

    // check the url , it should be empty when there is no url set
    EXPECT_EQ(httpReq.GetUrl(),"");
    // check the timeout, it should be -1 ,as initialized in the constructor
    EXPECT_EQ(httpReq.GetTimeout(), -1);

    // check the ProxySetting , it should have default values
    ProxySetting stProxy;
    stProxy = httpReq.GetProxy();
    EXPECT_EQ(stProxy.m_strHost,"");
    EXPECT_EQ(stProxy.m_nPort,-1);
    EXPECT_EQ(stProxy.m_strUser,"");
    EXPECT_EQ(stProxy.m_strPassword,"");

    // check the size of heaaders list, it should be empty
    std::list<std::string> listHeader;
    listHeader = httpReq.GetHeaders();
    EXPECT_EQ(listHeader.size(), 0);

    // check the local port start and end , it should be default -1
    LocalPortRange stLclRange;
    stLclRange = httpReq.GetLocalPortRange();
    EXPECT_EQ(stLclRange.m_nStart, -1);
    EXPECT_EQ(stLclRange.m_nEnd, -1);

    // check the form files list , it should be empty
    std::list<FormFile> listFrmFiles;
    listFrmFiles = httpReq.GetFormFiles();
    EXPECT_EQ(listFrmFiles.size(), 0);

    // check the form buffers list , it should be empty
    std::list<FormBuffer> listFormBuffer;
    listFormBuffer = httpReq.GetFormBuffers();
    EXPECT_EQ(listFormBuffer.size(), 0);
}

TEST_F(CHttpRequestTest, Test_CHttpRequestTest_GetUrl)
{
    CHttpRequest httpReq;
    //set url
    httpReq.SetUrl("https://www.google.com");

    // check the url using get method, it should be same as set before
    EXPECT_EQ("https://www.google.com", httpReq.GetUrl());
}

TEST_F(CHttpRequestTest, Test_CHttpRequestTest_GeTimeOut)
{
    CHttpRequest httpReq;
    // set the dummy time out
    httpReq.SetTimeout(130);

    // check the tiemout  using get method, it should be same as set before
    EXPECT_EQ(130, httpReq.GetTimeout());
}

TEST_F(CHttpRequestTest, Test_CHttpRequestTest_SetProxy_GetProxy)
{
    CHttpRequest httpReq;
    ProxySetting stProxy;

    // set dummy proxy values
    httpReq.SetProxy("host",8010,"user","password");

    // check the proxy settings using get method, it shuold be as set before
    stProxy = httpReq.GetProxy();
    
    EXPECT_EQ("host", stProxy.m_strHost);
    EXPECT_EQ(8010, stProxy.m_nPort);
    EXPECT_EQ("user", stProxy.m_strUser);
    EXPECT_EQ("password", stProxy.m_strPassword);
}

TEST_F(CHttpRequestTest, Test_CHttpRequestTest_SetHeaders)
{
    CHttpRequest httpReq;

    std::list<std::string> listHeader{"Authorization: Basic", 
                            "Cache-Control: no-cache", 
                            "Content-Type: application/x-www-form-urlencoded"};

    // set dummy header values
    httpReq.SetHeaders(listHeader);

    std::list<std::string> listRcvdHeader;

    /* check the headers list using get method, size of the list should be
     * more than 0
     */
    listRcvdHeader = httpReq.GetHeaders();

    EXPECT_EQ(listRcvdHeader.size(), 3);

    // clear the headers and check if the list is empty
    httpReq.ClearHeaders();
    std::list<std::string> listRcvdHeader1;
    listRcvdHeader1 = httpReq.GetHeaders();
    EXPECT_EQ(listRcvdHeader1.size(), 0);
}

TEST_F(CHttpRequestTest, Test_CHttpRequestTest_SetLocalRange)
{
    CHttpRequest httpReq;
    LocalPortRange stLclRange;

    // set dummy port range values
    httpReq.SetLocalPortRange(1000,2000);

    // check the port ranges using get method, it shuold be as set before
    stLclRange = httpReq.GetLocalPortRange();

    EXPECT_EQ(stLclRange.m_nStart, 1000);
    EXPECT_EQ(stLclRange.m_nEnd, 2000);
}

TEST_F(CHttpRequestTest, Test_CHttpRequestTest_FormFromFile)
{
    CHttpRequest httpReq;
    std::list<FormFile> stFrmFiles;

    // add dummy form file values
    httpReq.AddFormFromFile("FormName", "ContentType", "FilePath", "UploadFilePath");

    // check the form file values using get method, it shuold be as set before
    stFrmFiles = httpReq.GetFormFiles();

    FormFile stFfile = stFrmFiles.front();
    EXPECT_EQ(stFfile.m_strFormName, "FormName");
    EXPECT_EQ(stFfile.m_strContentType, "ContentType");
    EXPECT_EQ(stFfile.m_strFilePath, "FilePath");
    EXPECT_EQ(stFfile.m_strAttachmentUploadName, "UploadFilePath");

    // clear the form files and check if the list is empty
    httpReq.ClearFormFiles();

    std::list<FormFile> stFrmFiles1;
    stFrmFiles1 = httpReq.GetFormFiles();
    EXPECT_EQ(stFrmFiles1.size(), 0);
}

TEST_F(CHttpRequestTest, Test_CHttpRequestTest_FormFromBuffers)
{
    CHttpRequest httpReq;
    std::list<FormBuffer> stFormBuffers;

    // add dummy form buffer values
    httpReq.AddFormFromBuffer("FormName", "ContentType", nullptr,
                                10, true);

    // check the form buffer values using get method, it shuold be as set before
    stFormBuffers = httpReq.GetFormBuffers();

    FormBuffer stFBuf = stFormBuffers.front();
    EXPECT_EQ(stFBuf.m_strFormName, "FormName");
    EXPECT_EQ(stFBuf.m_strContentType, "ContentType");
    EXPECT_EQ(stFBuf.m_pvoidBufferPtr, nullptr);
    EXPECT_EQ(stFBuf.m_bUploadAsFile, true);

    // clear the form buffers and check if the list is empty
    httpReq.ClearFormBuffers();

    std::list<FormBuffer> stFormBuffers1;
    stFormBuffers1 = httpReq.GetFormBuffers();
    EXPECT_EQ(stFormBuffers1.size(), 0);
}

TEST_F(CHttpRequestTest, Test_CHttpRequestTest_SetJsonType)
{
    CHttpRequest httpReq;
    std::list<FormBuffer> stFormBuffers;

    httpReq.ClearHeaders();

    httpReq.SetJsonType();

    std::list<std::string> listRcvdHeader;

    /* check the headers list using get method, size of the list should be
     * more than 0
     */
    listRcvdHeader = httpReq.GetHeaders();

    EXPECT_EQ(listRcvdHeader.size(), 3);
}

}
