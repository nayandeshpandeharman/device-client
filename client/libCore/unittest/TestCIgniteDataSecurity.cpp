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

#include <string.h>
#include "gtest/gtest.h"
#include "crypto/CIgniteDataSecurity.h"

namespace ic_core
{
/**
 * Class CIgniteDataSecurityTest defines a test feature for 
 * CIgniteDataSecurity class
 */
class CIgniteDataSecurityTest : public ::testing::Test
{
protected:
    /**
     * Constructor
     */
   CIgniteDataSecurityTest()
   {
      // Do nothing
   }

   /**
    * Destructor
    */ 
   ~CIgniteDataSecurityTest() override
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

};

// Tests

TEST_F(CIgniteDataSecurityTest, Test_ZeroTest)
{
   // Initializing a key and a IV values
   std::string strKey = "VIN+SLNO+RANDOMKey";
   std::string strIV = "VIN+SLNO+RANDOMIV";

   // Initializing a object of CIgniteDataSecurity with key and IV
   CIgniteDataSecurity dataSecurity(strKey, strIV);

   // Expect the key and IV to be set
   EXPECT_EQ(strKey, dataSecurity.GetKey());
   EXPECT_EQ(strIV, dataSecurity.GetIV());
}

TEST_F(CIgniteDataSecurityTest, Test_EncryptEmptyMulti)
{
   // Assigning test key and IV values
   std::string strKey = "VIN+SLNO+RANDOMKey";
   std::string strIV = "VIN+SLNO+RANDOMIV";
   CIgniteDataSecurity dataSecurity(strKey, strIV);

   std::string strPlaintext = "";
   // Encrpyt the strPlaintext and get the result
   std::string strEncrypted = dataSecurity.Encrypt(strPlaintext);
   std::string strEncryptedAgain = dataSecurity.Encrypt(strPlaintext);

   // Result of encrypting an empty plaintext is empty string
   EXPECT_EQ(strEncryptedAgain, strEncrypted);
}

TEST_F(CIgniteDataSecurityTest, Test_EncryptEmptyString)
{
   // Test to encrypt empty string
   CIgniteDataSecurity dataSecurity("VIN+SLNO+RANDOM", "VIN+SLNO+RANDOM");
   
   std::string strPlaintext = "";
   // Encrpyt the empty strPlaintext
   std::string strEncrypted = dataSecurity.Encrypt(strPlaintext);
   
   // Expect the strEncrypted also to be empty in case of empty strPlaintext
   EXPECT_EQ("", strEncrypted);
}

TEST_F(CIgniteDataSecurityTest, Test_ManyEncryptPlaintext)
{
   // Assigning test key and IV values
   std::string strKey = "VIN+SLNO+RANDOMKey";
   std::string strIV = "VIN+SLNO+RANDOMIV";
   CIgniteDataSecurity dataSecurity(strKey, strIV);

   // Plaintext to be encrypted
   std::string strPlaintext = "this is a random text to be encrypted";

   // Encrpyt the strPlaintext
   std::string strEncrypted = dataSecurity.Encrypt(strPlaintext);

   // Decrpyt the strEncrypted text
   std::string strDecrypted = dataSecurity.Decrypt(strEncrypted);

   // Expect the strDecrypted to be same as strPlaintext
   EXPECT_EQ(strDecrypted, strPlaintext);

   // Again encrypt the previously decrypted text
   std::string strEncryptedAgain = dataSecurity.Encrypt(strDecrypted);

   // Decrypt back the strEncryptedAgain text
   std::string strDecryptedBack = dataSecurity.Decrypt(strEncryptedAgain);

   // Result of encrypting an empty plaintext is empty string
   EXPECT_EQ(strDecrypted, strDecryptedBack);
}

TEST_F(CIgniteDataSecurityTest, Test_EncryptPlaintext)
{
   //Test to encrypt plaintext
   CIgniteDataSecurity dataSecurity("VIN+SLNO+RANDOM", "VIN+SLNO+RANDOM");

   std::string strPlaintext = "this is a random text to be encrypted";
   // Encrpyt the strPlaintext and get the result
   std::string strEncrypted = dataSecurity.Encrypt(strPlaintext);

   // Expect the strEncrypted to be different from the original strPlaintext
   EXPECT_NE(strPlaintext, strEncrypted);
   EXPECT_NE("", strEncrypted);
}

TEST_F(CIgniteDataSecurityTest, Test_Decrypt_EmptyString)
{
   //Test to decrypt empty string
   CIgniteDataSecurity dataSecurity("VIN+SLNO+RANDOM", "VIN+SLNO+RANDOM");

   std::string strBase64encText = "";
   // Decrpyt the strBase64encText and get the result
   std::string strDecrypted = dataSecurity.Decrypt(strBase64encText);
   
   // Expect the strDecrypted also to be empty in case of empty strBase64encText
   EXPECT_EQ("", strDecrypted);
}

TEST_F(CIgniteDataSecurityTest, Test_Encrypt_WithTagAppend)
{
   /* Test to verify combination bIsTagAppendEnabled=false & strAADKey 
    * is empty and encryption works without AAD.
    */
   CIgniteDataSecurity dataSecurity("VIN+SLNO+RANDOM", "VIN+SLNO+RANDOM");
   std::string strPlaintext = "this is a random text to be encrypted";
   
   // Encrpyt the strPlaintext and get the result
   std::string strEncrypted = dataSecurity.Encrypt(strPlaintext, true, 
                                                   "123AADKEY321");

   // Expect the strEncrypted to be different from the original strPlaintext
   EXPECT_NE(strPlaintext, strEncrypted);
   EXPECT_NE("", strEncrypted);
}

TEST_F(CIgniteDataSecurityTest, Test_Encrypt_WithoutTagAppend_WithoutAAD)
{
   /* Test to verify combination bIsTagAppendEnabled=false & strAADKey 
    * is empty and encryption works without AAD.
    */
   CIgniteDataSecurity dataSecurity("VIN+SLNO+RANDOM", "VIN+SLNO+RANDOM");
   std::string strPlaintext = "this is a random text to be encrypted";
   
   // Encrpyt the strPlaintext and get the result
   std::string strEncrypted = dataSecurity.Encrypt(strPlaintext,false,"");

   // Expect the strEncrypted to be different from the original strPlaintext
   EXPECT_NE(strPlaintext, strEncrypted);
   EXPECT_NE("", strEncrypted);
}

TEST_F(CIgniteDataSecurityTest, Test_Encrypt_WithoutTagAppend)
{
   /* Test to verify combination bIsTagAppendEnabled=false & strAADKey
    * is sent and encryption works without AAD.
    */  
   CIgniteDataSecurity dataSecurity("VIN+SLNO+RANDOM", "VIN+SLNO+RANDOM");
   std::string strPlaintext = "this is a random text to be encrypted";
   
   // Encrpyt the strPlaintext and get the result
   std::string strEncrypted = dataSecurity.Encrypt(strPlaintext, false, 
                                                "123456AADstr123456");

   
   // Expect the strEncrypted to be different from the original strPlaintext
   EXPECT_NE(strPlaintext, strEncrypted);
   EXPECT_NE("", strEncrypted);
}

}