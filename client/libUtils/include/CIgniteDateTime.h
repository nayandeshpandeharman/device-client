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
********************************************************************************
* \file CIgniteDateTime.h
*
* \brief CIgniteDateTime provides the utility functions related to date and time
********************************************************************************
*/

#ifndef CIGNITE_DATETIME_H
#define CIGNITE_DATETIME_H

#include <string>
#define SET_TIMEZONE_FOR_TESTING 0

namespace ic_utils 
{
/**
 * Class CIgniteDateTime to provide utilitiy functions related to date and time.
 */
class CIgniteDateTime 
{
public:
    /**
     * Method to get current time in miliseconds
     * @param void
     * @return Numeric value indicates the time in milliseconds which 
     * corresponds to system time.
     */
    static unsigned long long GetCurrentTimeMs();

    /**
     * Method to get Monotonic time in miliseconds
     * @param void
     * @return Numeric value indicates the time in milliseconds which 
     * corresponds to the current monotonic time. 
     */
    static unsigned long long GetMonotonicTimeMs();

    /**
     * Method to get time since boot in miliseconds
     * @param void
     * @return Numeric value indicates the time in milliseconds which 
     * corresponds to the time since boot. 
     */
    static unsigned long long GetTimeSinceBootMs();

    /**
     * Method to get timezone offset in minutes
     * @param void
     * @return Numeric value indicates the time in minutes which 
     * corresponds to the timezone offset.
     */
    static int GetTimeZoneOffsetMinutes();

    /**
     * Method to get current day
     * @param void
     * @return Numeric value indicates the current day of the month.
     */
    static int GetCurrentDay();

    /**
     * Method to get current month
     * @param void
     * @return Numeric value indicates the current month of the year.
     */
    static int GetCurrentMonth();

    /**
     * Method to convert date string to number
     * @param[in] strDate Date as string type
     * @return Numeric value indicates the time in milliseconds since epoch 
     * which corresponds to given date string.
     */
    static unsigned long long ConvertToNumber(std::string strDate);

    /**
     * Method to get current formatted DateTime
     * @param void
     * @return Obtained current local time, formated as a string.
     */
    static std::string GetCurrentFormattedDateTime();

#if SET_TIMEZONE_FOR_TESTING
    /**
     * Method to set timezone offset
     * @param[in] strTimeZone Timezone offset to be set
     * @return void
     */
    static void SetTimeZoneOffset(std::string strTimeZone);
#endif
};

} /* namespace ic_utils */

#endif /* CIGNITE_DATETIME_H */
