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
#include "CIgniteHTTPConnector.h"
#include "CIgniteLog.h"
#include "core/CKeyGenerator.h"
#include "crypto/CIgniteDataSecurity.h"
#include "db/CLocalConfig.h"
#include "../include/http/HttpErrorCodes.h"
#include "CIgniteConfig.h"
#include "CIgniteClient.h"


#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "Test_IgniteHTTPConnector"
using ::testing::MatchesRegex;

namespace ic_network
{

/**
 * Class for unit testing CIgniteHTTPConnector
 */
class CIgniteHTTPConnectorTest : public ::testing::Test 
{
public:
    /**
     * Constructor
     */
    CIgniteHTTPConnectorTest() 
    {
    }

    /**
     * Destructor
     */
    ~CIgniteHTTPConnectorTest() override 
    {
    }

    /**
     * Overriding Method of testing::Test class
     * @see testing::Test::SetUp()
     */
    void SetUp() override 
    {

    }

    /**
     * Overriding Method of testing::Test class
     * @see testing::Test::SetUp()
     */
    void TearDown() override {

    }

    /**
     * Wrapper method to get the activation url from class under test
     * @param[in] pConn pointer to the object of class CIgniteHTTPConnector
     * @return Activation URL
     */
    std::string GetActivationURL(CIgniteHTTPConnector *pConn){
        return pConn->m_strActivationUrl;
    }

    /**
     * Wrapper method to get the auth url from class under test
     * @param[in] pConn pointer to the object of class CIgniteHTTPConnector
     * @return Auth URL
     */
    std::string GetAuthURL(CIgniteHTTPConnector *pConn){
        return pConn->m_strAuthUrl;
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

//Tests

TEST_F(CIgniteHTTPConnectorTest, Test_getInstance_CIgniteHTTPConnectorClassCreation)
{
    CIgniteHTTPConnector *pIHC = CIgniteHTTPConnector::GetInstance();
    /* check the existance of IgniteHTTPConnector class as Zero test case as 
     * per TDD approach
     */
    EXPECT_NE(pIHC,nullptr);
}

TEST_F(CIgniteHTTPConnectorTest, Test_getInstance_ManyCaseValidation)
{
    CIgniteHTTPConnector *pIHC = CIgniteHTTPConnector::GetInstance();
    CIgniteHTTPConnector *pIHC2 = CIgniteHTTPConnector::GetInstance();

    EXPECT_EQ(pIHC,pIHC2);
}

TEST_F(CIgniteHTTPConnectorTest, 
       Test_CIgniteHTTPConnector_checkEmptySerialNoInActivationReq)
{
    CIgniteHTTPConnector::CActivationRequest actReq;
    EXPECT_EQ(actReq.m_strSerialNumber,"");
}

TEST_F(CIgniteHTTPConnectorTest, 
       Test_CIgniteHTTPConnector_checkActivationReqPayload)
{
    CIgniteHTTPConnector::CActivationRequest actReq;
    actReq.m_strSerialNumber="TestSerial";
    actReq.m_strImei="TestImei";
    actReq.m_strQualifierID="TestQualifier";

    EXPECT_EQ(actReq.m_strSerialNumber,"TestSerial");
    EXPECT_EQ(actReq.m_strImei,"TestImei");
    EXPECT_EQ(actReq.m_strQualifierID,"TestQualifier");
}

TEST_F(CIgniteHTTPConnectorTest, Test_CIgniteHTTPConnector_checkEmptyAuthReq)
{
    CIgniteHTTPConnector::CAuthRequest authReq;

    EXPECT_EQ(authReq.m_strLoginStr,"");
}

TEST_F(CIgniteHTTPConnectorTest, 
       Test_CIgniteHTTPConnector_checkAuthReqPostFieldEKey)
{
    CIgniteHTTPConnector::CAuthRequest authReq;
    authReq.m_strLoginStr="enxry1234";

    EXPECT_EQ(authReq.m_strLoginStr,"enxry1234");

}

TEST_F(CIgniteHTTPConnectorTest, 
       Test_setActivationURL_checkValidUrlWhichisSetIsReturned)
{
    CIgniteHTTPConnector *pIHC = CIgniteHTTPConnector::GetInstance();
    pIHC->SetActivationURL("https://example");

    CIgniteHTTPConnectorTest objIgniteHTTPConnectorTest;
    std::string strActUrl = objIgniteHTTPConnectorTest.GetActivationURL(pIHC);
    EXPECT_EQ(strActUrl,"https://example");
}

TEST_F(CIgniteHTTPConnectorTest, Test_setAuthURL_checkValidUrlWhichisSetIsReturned)
{
    CIgniteHTTPConnector *pIHC = CIgniteHTTPConnector::GetInstance();
    pIHC->SetAuthURL("https://example_auth");

    CIgniteHTTPConnectorTest objIgniteHTTPConnectorTest;
    std::string strAuthUrl = objIgniteHTTPConnectorTest.GetAuthURL(pIHC);
    EXPECT_EQ(strAuthUrl,"https://example_auth");
}

TEST_F(CIgniteHTTPConnectorTest, Test_activate_checkactivateAPIwithNoData)
{
    CIgniteHTTPConnector::CActivationRequest aReq;
    CIgniteHTTPConnector::CActivationResponse aResp;

    CIgniteHTTPConnector::GetInstance()->Activate(aReq,aResp,"");

    // Expecting not eERR_OK as activation request payload is empty
    EXPECT_NE(aResp.m_eHttpSessionErrCode,ic_network::HttpErrorCode::eERR_OK);
}

TEST_F(CIgniteHTTPConnectorTest, Test_GetAuthToken_withEmptyURL)
{
    CIgniteHTTPConnector *pIHC = CIgniteHTTPConnector::GetInstance();

    // auth token
    CIgniteHTTPConnector::CAuthRequest wReq;
    CIgniteHTTPConnector::CAuthResponse wResp;
    pIHC->SetAuthURL("");
    wReq.m_strLoginStr = "SAMPLEHCPID";
    wReq.m_strPasscodeStr = "ENCODEDPASSCODE";
    wReq.m_bUseDevType = 
        ic_core::CIgniteConfig::GetInstance()->GetBool("useDeviceType");
    wReq.m_strProductType = 
        ic_core::CIgniteConfig::GetInstance()->GetString("ProductType");
    pIHC->GetAuthToken(wReq, wResp);

    EXPECT_EQ(wResp.m_eHttpSessionErrCode, 
                    ic_network::HttpErrorCode::eERR_INV_INPUT);
    EXPECT_EQ(wResp.m_strHttpRespData, "Auth URL not set");
}

TEST_F(CIgniteHTTPConnectorTest, Test_Activate_withEmptyURL)
{
    CIgniteHTTPConnector *pIHC = CIgniteHTTPConnector::GetInstance();

    // Activation
    CIgniteHTTPConnector::CActivationRequest aReq;
    CIgniteHTTPConnector::CActivationResponse aResp;

    // fill dynamic parameters for CActivationRequest by reading from config file
    aReq.m_strQualifierID = 
        ic_core::CIgniteClient::GetProductImpl()->GetActivationQualifierID(); 
    aReq.m_strVin = 
        ic_core::CIgniteConfig::GetInstance()->GetString("Default.Vin");  
    aReq.m_strSerialNumber = 
        ic_core::CIgniteConfig::GetInstance()->GetString("Default.Serial");
    aReq.m_strImei = 
        ic_core::CIgniteConfig::GetInstance()->GetString("Default.IMEI"); 
    aReq.m_strSwVersion = 
        ic_core::CIgniteConfig::GetInstance()->GetString("Default.SwVersion"); 
    aReq.m_strHwVersion = 
        ic_core::CIgniteConfig::GetInstance()->GetString("Default.HwVersion"); 
    aReq.m_strProductType = 
        ic_core::CIgniteConfig::GetInstance()->GetString("ProductType");
    aReq.m_strDeviceType = 
        ic_core::CIgniteConfig::GetInstance()->GetBool("useDeviceType");

    // set activation url to empty string
    pIHC->SetActivationURL("");
    pIHC->Activate(aReq, aResp);

    EXPECT_EQ(aResp.m_eHttpSessionErrCode, 
                    ic_network::HttpErrorCode::eERR_INV_INPUT);
    EXPECT_EQ(aResp.m_strRespString, "Activation URL not set");
}

TEST_F(CIgniteHTTPConnectorTest, Test_GetConnectionHealthCheckStatus_withEmptyURL)
{
    CIgniteHTTPConnector *pIHC = CIgniteHTTPConnector::GetInstance();
    CIgniteHTTPConnector::CIgniteConnHealthCheckResponse hcResp;
    // set health check url to empty string
    pIHC->SetHealthCheckURL("");
    pIHC->GetConnectionHealthCheckStatus(hcResp);

    EXPECT_EQ(hcResp.m_eHttpSessionErrCode,
              ic_network::HttpErrorCode::eERR_INV_INPUT);
}

TEST_F(CIgniteHTTPConnectorTest, Test_GetConnectionHealthCheckStatus)
{
    CIgniteHTTPConnector *pIHC = CIgniteHTTPConnector::GetInstance();
    CIgniteHTTPConnector::CIgniteConnHealthCheckResponse hcResp;

    std::string strHealthCheckURL = "";
    strHealthCheckURL = 
    ic_core::CIgniteConfig::GetInstance()->GetString("HCPAuth.healthcheck_url");
    // set health check url to empty string
    pIHC->SetHealthCheckURL(strHealthCheckURL);
    pIHC->GetConnectionHealthCheckStatus(hcResp);

    EXPECT_EQ(hcResp.m_eHttpSessionErrCode,
              ic_network::HttpErrorCode::eERR_INV_INPUT);
}

} // namespace
