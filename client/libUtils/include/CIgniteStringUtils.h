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

/*!
*******************************************************************************
* \file CIgniteStringUtils.h
*
* \brief CIgniteStringUtils provides the utility functions for string operations.
*******************************************************************************
*/

#ifndef CIGNITE_STRINGUTILS_H
#define CIGNITE_STRINGUTILS_H

#include <sstream>
#include <algorithm>
#include <iomanip>

namespace ic_utils 
{
/**
 * class CIgniteStringUtils provides string utilities.
 */
class CIgniteStringUtils 
{
public:
    /**
     * Template method to convert a numeric value to its string representation.
     * @tparam T The type of the numeric value.
     * @param[in] tNumber The numeric value to be converted to a string.
     * @return String representation of the numeric value.
     */
    template <typename T> static std::string NumberToString(T tNumber)
    {
        std::ostringstream ss;
        ss << tNumber;
        return ss.str();
    }

    /**
     * Template method to convert a string to a numeric value of a specified 
     * type.
     * @tparam T The type of the numeric value to be obtained.
     * @param[in] rstrText The input string to be converted to a numeric value.
     * @return The numeric value obtained from the input string;
     *         default value if conversion fails.
     */
    template <typename T> static T StringToNumber(const std::string& rstrText)
    {
        std::istringstream ss(rstrText);
        T tResult;
        return ss >> tResult ? tResult : 0;
    }

    /**
     * Method to check whether a given string represents a valid numeric value
     * @param[in] strInput Input string 
     * @return true, if given string represents a valid numeric value;
     *         false, otherwise.
     */
    static bool IsNumeric(std::string strInput)
    {
        // First check for valid characters only
        if (strInput.find_first_not_of("0123456789.") == std::string::npos)
        {
            // Next make sure we only have one decimal point:
            if ( std::count(strInput.begin(), strInput.end(), '.') <= 1 )
            {
                return true;
            }
        }
        return false;
    }

    /**
     * Method to convert a given input string into its hexadecimal 
     * representation as a string.
     * @param[in] rstrInputKey The input string 
     * @return The hexadecimal representation of the input string
     */
    static const std::string ToHexString(const std::string& rstrInputKey)
    {
        std::ostringstream outputKey;
        for (std::string::size_type i = 0; i < rstrInputKey.length(); ++i)
        {
            outputKey << std::hex << std::setfill('0') << std::setw(2) 
            << std::uppercase << (int)rstrInputKey[i];
        }
        return outputKey.str();
    }

    /**
     * Method to erase a substring from the main string.
     * @param[in] rstrMainStr The main string 
     * @param[in] rstrToErase The substring to be erased from rstrMainStr
     * @return void
     */
    static void EraseSubStr(std::string & rstrMainStr, 
                            const std::string & rstrToErase)
    {
        // Search for the substring in string
        size_t nPos = rstrMainStr.find(rstrToErase);

        if (nPos != std::string::npos)
        {
            // If found then erase it from string
            rstrMainStr.erase(nPos, rstrToErase.length());
        }
    }

    /**
     * Template method to split a given string into tokens using a specified 
     * delimiter and populates a container.
     * @tparam T The type of container to store the tokens.
     * @param[in] rstrInput The input string to be split.
     * @param[in] chDelim The delimiter character used for splitting.
     * @param[out] tTokenlist The container to store the resulting tokens.
     */
    template<typename T> static void Split(const std::string &rstrInput, 
                                           const char chDelim, 
                                           T& tTokenlist)
    {
        std::stringstream ss;
        ss.str(rstrInput);
        std::string strItem;
        while (std::getline(ss, strItem, chDelim)) 
        {
            tTokenlist.insert(tTokenlist.end(), strItem);
        }
    }

    /**
     * Method to return random alphanumeric string of requested length
     * @param[in] nLength The length of required random string
     * @return alphanumeric String of requested length
     */
    static std::string RandomString(const int nLength)
    {
        // Use current time as seed for random generator 
        srand(time(0));
        auto randchar = []() -> char
        {
            const char charset[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
            const size_t max_index = (sizeof(charset) - 1);
            return charset[rand() % max_index];
        };
        std::string strResult(nLength,0);
        std::generate_n(strResult.begin(), nLength, randchar);
        return strResult;
    }
};

} /* namespace ic_utils */

#endif /* CIGNITE_STRINGUTILS_H */
