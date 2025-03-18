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
* \file CEventIntervalValidator.h
*
* \brief This class provides methods which decides on validity of event based on
* interval of 2 same events that can be configured in configuration file
********************************************************************************
*/

#ifndef CEVENT_INTERVAL_VALIDATOR_H
#define CEVENT_INTERVAL_VALIDATOR_H

#include <map>
#include "CIgniteMutex.h"

namespace ic_bl
{
/**
 * Class CEventIntervalValidator provides methods which decides on validity of
 * event based on interval of 2 same events that can be configured in
 * configuration file
 */
class CEventIntervalValidator
{
public:
    /**
     * Method to get instance of CEventIntervalValidator
     * @param void
     * @return Pointer to singleton object of CEventIntervalValidator
     */
    static CEventIntervalValidator* GetInstance();

    /**
     * Method to check if the interval between the given event's timestamp and
     * the corresponding timestamp in the configuration is within the configured
     * interval or not
     * @param[in] strEventId whose timestamp should be updated
     * @param[in] llTimestamp timestamp value to be updated to eventId
     * @return true if valid interval or interval validation is disabled,
     *         false otherwise.
     */
    bool IsValidInterval(std::string strEventId, long long llTimestamp);

    /**
     * Destructor.
     */
    ~CEventIntervalValidator();

    #ifdef IC_UNIT_TEST
        friend class CEventIntervalValidatorTest;
    #endif 

private:
    /**
     * Default no-argument constructor.
     */
    CEventIntervalValidator();

    /**
     * Method to populate event Interval based on configuration
     * @param void
     * @return void 
     * 
     */
    void PopulateEventIntervalFromConfig();

    /**
     * Method to validate the difference between two given timestamp is greater
     * than or equal to given interval.
     * @param[in] llCurTimestamp the current timestamp of the event
     * @param[in] llPrevTimestamp the timestamp of the event stored prevously.
     * @param[in] nInterval the interval value with which the difference between
     *                      two timestamps should be compared
     * @return true if difference between two timestamp is greater than or equal
     *         to nInterval else return false
     */
    bool ValidateInterval(long long llCurTimestamp, long long llPrevTimestamp,
                                                              int nInterval);

    //! Member variable to store list of eventId and interval based on config
    std::map<std::string, std::pair<int, long long int> >m_mapEventInterval;

    //! Member variable to check whether interval validation to be done or not
    bool m_bValidateInterval;

    //! Member variable to provide mutex lock
    ic_utils::CIgniteMutex m_IntervalMutex;
};
}
#endif //CEVENT_INTERVAL_VALIDATOR_H
