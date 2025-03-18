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
#include "crypto/CAes.h"
#include "CIgniteLog.h"

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "test_CAes"

namespace ic_core 
{
/**
 * Class CAesTest defines a test feature for CAes class
 */
class CAesTest : public ::testing::Test
{
protected:
    /**
     * Constructor
     */
    CAesTest()
    {
        // Do nothing
    }

    /**
     * Destructor
     */ 
    ~CAesTest() override
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

// Tests

TEST_F(CAesTest, Test_Encrypt_EncryptDecrypt_EmptyPlaintext)
{
    CAes aesObj("VIN+SLNO+RANDOM", "VIN+SLNO+RANDOM");

    const std::string strPlaintext = "";
    // Encrypt the strPlaintext(empty string)
    std::string strEncrypted = aesObj.Encrypt(strPlaintext);

    // Expect Encrypt to return empty string "" if the input is empty
    EXPECT_EQ("", strEncrypted);

    // Decrypt empty cipher text
    std::string strDecrypted = aesObj.Decrypt(strEncrypted);

    // Expect strDecrypted to be empty
    EXPECT_EQ("", strDecrypted);
}

TEST_F(CAesTest, Test_Encrypt_EncryptDecryptPlaintext)
{
    CAes aesObj("VIN+SLNO+RANDOM", "VIN+SLNO+RANDOM");

    const std::string strPlaintext = "this is a random text to encrypt";
    // Encrypt the strPlaintext(non-empty string)
    std::string strEncrypted = aesObj.Encrypt(strPlaintext);
    
    // Expect the strEncrypted to be different from the original strPlaintext
    EXPECT_NE(strPlaintext, strEncrypted);

    std::string strDecrypted = aesObj.Decrypt(strEncrypted);

    // Expect strDecrypted to be equal to the strPlaintext
    EXPECT_EQ(strPlaintext, strDecrypted);
}
} // namespace ic_core 
