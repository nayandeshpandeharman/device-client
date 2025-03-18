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
#include "core/CClientConnectorImpl.h"
#include "CIgniteClient.h"
#include "db/CLocalConfig.h"

namespace ic_bl
{
/**
 * Class CClientConnectorImplTest defines a test feature
 * for CClientConnectorImpl class
 */
class CClientConnectorImplTest : public ::testing::Test
{
 protected:
   /**
    * Constructor
    */
   CClientConnectorImplTest()
   {
      // do nothing
   }

   /**
    * Destructor
    */
   ~CClientConnectorImplTest() override
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

/**
 * We cannot create the object of CClientConnectorImpl as CClientConnectorImpl
 * object is created, since CClientConnectorImpl is singleton instance
 * 
 */
TEST_F(CClientConnectorImplTest, Test_GetConfig_FetchConfigFromKey)
{
   //fetching instance of CClientConnectorImpl class
   CClientConnectorImpl *pinstance = CClientConnectorImpl::GetInstance();

   //Persisting config with key "TestgetConfigKey" and velue "TestValue"
   pinstance->PersistConfig("TestgetConfigKey","TestValue");

   //Expect value equal to "TestValue" for key "TestgetConfigKey"
   EXPECT_EQ("TestValue", pinstance->GetConfig("TestgetConfigKey"));
}

TEST_F(CClientConnectorImplTest, Test_PersistConfig_StoreKeyValue)
{
   //fetching instance of CClientConnectorImpl class
   CClientConnectorImpl *pinstance = CClientConnectorImpl::GetInstance();

   //Expect true as given key value stored in config
   EXPECT_EQ(true,pinstance->PersistConfig("TestpersistConfigKey","TestValue"));
}

TEST_F(CClientConnectorImplTest, Test_GetAttribute_FetchVIN)
{
   //fetching instance of CClientConnectorImpl class
   CClientConnectorImpl *pinstance = CClientConnectorImpl::GetInstance();

   //Fetch Vin of the device
   std::string strVIN = ic_core::CIgniteClient::GetProductImpl()->GetAttribute(
                                                       ic_core::IProduct::eVIN);

   //Fetch Vin from CClientConnectorImpl GetAttribute
   std::string strCConnectorVIN = pinstance->GetAttribute(
                                             ic_core::IClientConnector::eVIN);

   //Expect true as both above VINs are same
   EXPECT_STREQ(strVIN.c_str(), strCConnectorVIN.c_str());
}

TEST_F(CClientConnectorImplTest, Test_GetAttribute_FetchSerialNumber)
{
   //fetching instance of CClientConnectorImpl class
   CClientConnectorImpl *pinstance = CClientConnectorImpl::GetInstance();

   //Fetch Serial Number of the device
   std::string strSerialNum = ic_core::CIgniteClient::GetProductImpl()->
                                 GetAttribute(ic_core::IProduct::eSerialNumber);

   //Fetch Serial Number from CClientConnectorImpl GetAttribute
   std::string strCConnectorSerialNum = pinstance->GetAttribute(
                                     ic_core::IClientConnector::eSERIAL_NUMBER);

   //Expect true as both above Serial Number are same
   EXPECT_STREQ(strSerialNum.c_str(), strCConnectorSerialNum.c_str());
}

TEST_F(CClientConnectorImplTest, Test_GetAttribute_FetchImei)
{
   //fetching instance of CClientConnectorImpl class
   CClientConnectorImpl *pinstance = CClientConnectorImpl::GetInstance();

   //Fetch Imei of the device
   std::string strImei = ic_core::CIgniteClient::GetProductImpl()->
                                       GetAttribute(ic_core::IProduct::eIMEI);

   //Fetch Imei from CClientConnectorImpl GetAttribute
   std::string strCConnectorImei = pinstance->GetAttribute(
                                             ic_core::IClientConnector::eIMEI);

   //Expect true as both above Imei are same
   EXPECT_STREQ(strImei.c_str(), strCConnectorImei.c_str());
}

TEST_F(CClientConnectorImplTest, Test_GetAttribute_FetchDeviceId)
{
   //fetching instance of CClientConnectorImpl class
   CClientConnectorImpl *pinstance = CClientConnectorImpl::GetInstance();

   //Fetch device Id stored in local config
   std::string strDeviceId = ic_core::CLocalConfig::GetInstance()->Get("login");

   //Fetch device Id from CClientConnectorImpl GetAttribute
   std::string strCConnectorDeviceId = pinstance->GetAttribute(
                                         ic_core::IClientConnector::eDEVICE_ID);

   //Expect true as both above device IDs are same
   EXPECT_STREQ(strDeviceId.c_str(), strCConnectorDeviceId.c_str());
}
}/* namespace ic_bl */

