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
#include <string.h>
#include "crypto/CAesGcm.h"
#include "crypto/CAes.h"
#include "db/CLocalConfig.h"
#include "core/CKeyGenerator.h"
#include "CIgniteConfig.h"
#include "CIgniteLog.h"

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "test_AesGcm"

namespace ic_core 
{
/**
 * Class CAesGcmTest defines a test feature for CAesGcm class
 */
class CAesGcmTest : public ::testing::Test
{
protected:
   /**
    * Constructor
    */
   CAesGcmTest()
   {
      // Do nothing
   }

   /**
    * Destructor
    */ 
   ~CAesGcmTest() override
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

TEST_F(CAesGcmTest, Test_ZeroTest)
{
   // Initializing a key and a IV values
   std::string strKey = "VIN+SLNO+RANDOMKey";
   std::string strIV = "VIN+SLNO+RANDOMIV";  

   CAesGcm aesGcmObj(strKey, strIV);

   EXPECT_EQ(strKey, aesGcmObj.GetKey());
   EXPECT_EQ(strIV, aesGcmObj.GetIV());
}

TEST_F(CAesGcmTest, Test_GcmEncrypt_EmptyPlaintextMultiple)
{
   // Check the One test case as per TDD approach
   CAesGcm aesGcmObj("VIN+SLNO+RANDOM", "VIN+SLNO+RANDOM");
   
   std::string strPlaintext = "";

   // Encrpyt the strPlaintext and get the strEncrypted string
   std::string strEncrypted;
   aesGcmObj.GcmEncrypt(strPlaintext, strEncrypted);

   std::string strEncryptedAgain;
   // Encrpyt the strEncrypted and get the strEncryptedAgain
   aesGcmObj.GcmEncrypt(strEncrypted, strEncryptedAgain);

   // Result of encrypting an empty plaintext is empty string
   EXPECT_EQ(strPlaintext, strEncrypted);
   EXPECT_EQ(strEncrypted, strEncryptedAgain);
}

TEST_F(CAesGcmTest, Test_GcmEncrypt_EncryptEmptyPlaintext)
{
   CAesGcm aesGcmObj("VIN+SLNO+RANDOMJ", "VIN+SLNO+RANDOMJ");

   // Initializing a empty strPlaintext
   const std::string strPlaintext = "";
   std::string strEncryptedText = "";

   // Encrpyt the strPlaintext and get the result
   int nRet = aesGcmObj.GcmEncrypt(strPlaintext, strEncryptedText);

   // Expect GcmEncrypt to fail when input is empty
   EXPECT_EQ(eGCM_FAILURE, nRet);
   EXPECT_EQ("", strEncryptedText);
}

TEST_F(CAesGcmTest, Test_GcmEncrypt_EncryptPlaintext)
{
   // Positive case for GcmEncrypt with default value of tag=false
   CAesGcm aesGcmObj("VIN+SLNO+RANDOMJ", "VIN+SLNO+RANDOMJ");

   // Initializing a strPlaintext
   const std::string strPlaintext = "this is a random text to be encrypted";
   std::string strEncryptedText = "";

   // Encrpyt the strPlaintext and get the result
   int nRet = aesGcmObj.GcmEncrypt(strPlaintext, strEncryptedText);

   // Expect GcmEncrypt to succeed
   EXPECT_EQ(eGCM_SUCCESS, nRet);
}

TEST_F(CAesGcmTest, Test_GcmDecrypt_DecryptEmptyCiphertext)
{
   CAesGcm aesGcmObj("VIN+SLNO+RANDOMJ", "VIN+SLNO+RANDOMJ");

   // Initializing a empty ciphertext
   const std::string strCiphertext = "";
   std::string strDecrypted;

   // Decrypt the strCiphertext and get the result
   int nDecryptStatus = aesGcmObj.GcmDecrypt(strCiphertext, strDecrypted);

   // Expect GcmDecrypt to fail when input is empty
   EXPECT_EQ(eGCM_FAILURE, nDecryptStatus);
}

TEST_F(CAesGcmTest, Test_GcmDecrypt_DecryptInvalidCiphertext)
{
   CAesGcm aesGcmObj("VIN+SLNO+RANDOMJ", "VIN+SLNO+RANDOMJ");

   // Initializing a invalid ciphertext
   const std::string strCiphertext = "invalid_ciphertext";

   std::string strDecrypted;
   // Decrypt the strCiphertext and get the result
   int nDecryptStatus = aesGcmObj.GcmDecrypt(strCiphertext, strDecrypted);

   // Expect GcmDecrypt to fail when input is invalid
   EXPECT_EQ(eGCM_FAILURE, nDecryptStatus);
}

TEST_F(CAesGcmTest, Test_GcmEncryptAndGcmDecrypt_withoutTag)
{
   //Test GcmEncrypt and GcmDecrypt without Tag
   CAesGcm aesGcmObj("VIN+SLNO+RANDOMJ", "VIN+SLNO+RANDOMJ");

   // Initializing a strPlaintext
   std::string strPlaintext = "Hello, World! What's in your mind now? \
                            I'm good at reading minds:)";

   std::string strEncrypted;
   // Encrpyt the strPlaintext and get the result
   int nEncryptStatus = aesGcmObj.GcmEncrypt(strPlaintext, strEncrypted, false);

   // Proceed further only when GcmEncrypt is successful
   ASSERT_EQ(eGCM_SUCCESS, nEncryptStatus);

   std::string strDecrypted;
   // Decrypt the strEncrypted and get the result
   int nDecryptStatus = aesGcmObj.GcmDecrypt(strEncrypted, strDecrypted);

   // Proceed further only when GcmDecrypt is successful
   ASSERT_EQ(eGCM_SUCCESS, nDecryptStatus);

   // Expect the decrypted text to be equal to the original plaintext
   EXPECT_EQ(strPlaintext, strDecrypted);
}

TEST_F(CAesGcmTest, Test_GcmEncryptAndGcmDecrypt_shortPlaintext)
{
   //Test GcmEncrypt and GcmDecrypt with shorter inputs
   CAesGcm aesGcmObj("VIN+SLNO+RANDOMJ", "VIN+SLNO+RANDOMJ");
   std::string strPlaintext = "H";

   std::string strEncrypted;
   // Encrpyt the strPlaintext and get the result
   int nEncryptStatus = aesGcmObj.GcmEncrypt(strPlaintext, strEncrypted);

   // Proceed further only when GcmEncrypt is successful
   ASSERT_EQ(eGCM_SUCCESS, nEncryptStatus);

   std::string strDecrypted;
   // Decrypt the strEncrypted and get the result
   int nDecryptStatus = aesGcmObj.GcmDecrypt(strEncrypted, strDecrypted);

   // Proceed further only when GcmDecrypt is successful
   ASSERT_EQ(eGCM_SUCCESS, nDecryptStatus);

   // Expect the decrypted text to be equal to the original plaintext
   EXPECT_EQ(strPlaintext, strDecrypted);
}

TEST_F(CAesGcmTest, Test_GcmEncryptAndGcmDecrypt_longPlaintext)
{
   //Test GcmEncrypt and GcmDecrypt with longer input
   CAesGcm aesGcmObj("VIN+SLNO+RANDOMJ", "VIN+SLNO+RANDOMJ");
   std::string strPlaintext = 
               "A string of five rowing boats set out from the opposite bank."
               "A string of five rowing boats set out from the opposite bank."
               "A string of five rowing boats set out from the opposite bank."
               "A string of five rowing boats set out from the opposite bank."
               "A string of five rowing boats set out from the opposite bank."
               "A string of five rowing boats set out from the opposite bank."
               "A string of five rowing boats set out from the opposite bank."
               "A string of five rowing boats set out from the opposite bank."
               "A string of five rowing boats set out from the opposite bank."
               "A string of five rowing boats set out from the opposite bank."
               "A string of five rowing boats set out from the opposite bank."
               "A string of five rowing boats set out from the opposite bank."
               "A string of five rowing boats set out from the opposite bank."
               "A string of five rowing boats set out from the opposite bank."
               "A string of five rowing boats set out from the opposite bank."
               "A string of five rowing boats set out from the opposite bank."
               "A string of five rowing boats set out from the opposite bank."
               "A string of five rowing boats set out from the opposite bank."
               "A string of five rowing boats set out from the opposite bank.";

   std::string strEncrypted;
   // Encrpyt the strPlaintext and get the result
   int nEncryptStatus = aesGcmObj.GcmEncrypt(strPlaintext, strEncrypted);

   // Proceed further only when GcmEncrypt is successful
   ASSERT_EQ(eGCM_SUCCESS, nEncryptStatus);

   std::string strDecrypted;
   // Decrypt the strEncrypted and get the result
   int nDecryptStatus = aesGcmObj.GcmDecrypt(strEncrypted, strDecrypted);
   EXPECT_EQ(eGCM_SUCCESS, nDecryptStatus);

   // Expect the decrypted text to be equal to original plaintext
   EXPECT_EQ(strPlaintext, strDecrypted);
}

TEST_F(CAesGcmTest, Test_EncryptWithTag_DecryptWithoutTag)
{
   // Test Encryption with Tag Enabled but Decryption without Tag Enabled
   CAesGcm aesGcmObj("VIN+SLNO+RANDOMJ", "VIN+SLNO+RANDOMJ");
   std::string strPlaintext = "Hello, World! What's in your mind now?"
                              "I'm good at reading minds:)"
                              "This is a Encrypt_WithTagDecrypt Test";

   std::string strEncrypted;
   // Encrpyt the strPlaintext and get the result
   int nEncryptStatus = aesGcmObj.GcmEncrypt(strPlaintext, strEncrypted, true, 
                                         "123aadstr32112");

   // Proceed further only when GcmEncrypt is successful
   ASSERT_EQ(eGCM_SUCCESS, nEncryptStatus);

   std::string strDecrypted;
   // Decrypt the strEncrypted and get the result
   int nDecryptStatus = aesGcmObj.GcmDecrypt(strEncrypted, strDecrypted);
   EXPECT_EQ(eGCM_SUCCESS, nDecryptStatus);

   /* Expect the strDecrypted not equal to strPlaintext, 
    * as no tag was provided while decryption
    */
   EXPECT_NE(strPlaintext, strDecrypted);
}

TEST_F(CAesGcmTest, Test_GcmEncrypt_EncryptWithTagEnabled)
{
   // Test Encryption with Tag Enabled, third parameter of GcmEncrypt()sent True
   CAesGcm aesGcmObj("VIN+SLNO+RANDOMJ", "VIN+SLNO+RANDOMJ");
   std::string strPlaintext = "Here is a random text";

   std::string strEncrypted;
   // Encrpyt the strPlaintext and get the result
   int nEncryptStatus = aesGcmObj.GcmEncrypt(strPlaintext, strEncrypted, true);

   // Expect encrypt status as eGCM_AADKEY_EMPTY as no AADkey is provided
   EXPECT_EQ(eGCM_AADKEY_EMPTY, nEncryptStatus);
}

TEST_F(CAesGcmTest, Test_GcmEncrypt_WithTagAppend_WithEmptyAADKey)
{
   // Verify Combination TagAppend Enabled, strAADKey is sent Empty
   CAesGcm aesGcmObj("VIN+SLNO+RANDOMJ", "VIN+SLNO+RANDOMJ");
   std::string strPlaintext = "Here is a random text";

   std::string strEncrypted;
   // Encrpyt the strPlaintext and get the result
   int nEncryptStatus = aesGcmObj.GcmEncrypt(strPlaintext, strEncrypted, true, 
                                             "");

   // Expect encrypt status as eGCM_AADKEY_EMPTY as AADkey is empty 
   EXPECT_EQ(eGCM_AADKEY_EMPTY, nEncryptStatus);
}

TEST_F(CAesGcmTest, Test_GcmEncrypt_AADKeyGreaterThanFive)
{
   // Test Encryption with Tag Enabled, strAADKey is sent with AADKeyLength > 5
   CAesGcm aesGcmObj("VIN+SLNO+RANDOMJ", "VIN+SLNO+RANDOMJ");
   std::string strPlaintext = "Here is a random text";

   std::string strEncrypted;
   // Encrpyt the strPlaintext and get the result
   int nEncryptStatus = aesGcmObj.GcmEncrypt(strPlaintext, strEncrypted,
                                          true, "123456789");

   // Expect GcmEncrypt to succeed
   EXPECT_EQ(eGCM_SUCCESS, nEncryptStatus);
}

TEST_F(CAesGcmTest, Test_GcmEncrypt_AADKeyLessThanFive)
{
   // Test Encryption with Tag Enabled, strAADKey is sent with AADKeyLength < 5
   CAesGcm aesGcmObj("VIN+SLNO+RANDOMJ", "VIN+SLNO+RANDOMJ");
   std::string strPlaintext = "H";

   std::string strEncrypted;
   // Encrpyt the strPlaintext and get the result
   int nEncryptStatus = aesGcmObj.GcmEncrypt(strPlaintext, strEncrypted,
                                          true, "123");

   // Expect GcmEncrypt to succeed
   EXPECT_EQ(eGCM_SUCCESS, nEncryptStatus);
}
} //namespace
