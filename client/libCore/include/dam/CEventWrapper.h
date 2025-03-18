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
* \file CEventWrapper.h
*
* \brief This class/module provides wrapper for event class used to capture all 
* the necessary attributes required by the Ignite Event
*******************************************************************************
*/

#ifndef CEVENT_WRAPPER_H
#define CEVENT_WRAPPER_H

#include "jsoncpp/json.h"
#include "CIgniteEvent.h"

namespace ic_core 
{
/**
 * CEventWrapper class is a wrapper for event class used to create events by 
 * capturing all the necessary attributes.
 */
class CEventWrapper : public ic_event::CIgniteEvent 
{
public:
    /**
     * Method to set the eventId to the ignite event
     * @param[in] strId String containing EventID
     * @return void
     */
    virtual void SetEventId(std::string strId);
    
    /**
     * Overriding Method of CIgniteEvent class
     * @see CIgniteEvent::GetInt()
     */
    int GetInt(const std::string &rstrKey, int nDefaultValue = 0) override;

    /**
     * Overriding Method of CIgniteEvent class
     * @see CIgniteEvent::GetBool()
     */
    bool GetBool(const std::string &rstrKey, bool bDefaultValue= false) override;

    /**
     * Overriding Method of CIgniteEvent class
     * @see CIgniteEvent::GetString()
     */
    std::string GetString(const std::string &rstrKey, 
                          std::string strDefaultValue = "") override;

    /**
     * Overriding Method of CIgniteEvent class
     * @see CIgniteEvent::GetLong()
     */
    long long GetLong(const std::string &rstrKey,
                      long long llDefaultValue = 0) override;

    /**
     * Method to add PII filed to the ignite event based on input parameter
     * @param[in] rstrName String containing PII field key
     * @param[in] rstrVal String containing PII field value
     * @return void
     */
    virtual void AddPiiField(const std::string &rstrName, 
                             const std::string &rstrVal);

    /**
     * Overloading Method to add PII filed based on input parameter
     * @param[in] rstrName String containing PII field key
     * @param[in] rjsonVal JSON object containing PII field value
     * @return void
     */
    virtual void AddPiiField(const std::string &rstrName, 
                             const ic_utils::Json::Value &rjsonVal);
    
    /**
     * Method to get attachments
     * @param void
     * @return vector containing attachments
     */
    virtual std::vector<std::string> GetAttachments();

    /**
     * Method to remove attachments
     * @param void
     * @return void
     */
    virtual void RemoveAttachments();

    /**
     * Overriding Method of Event class
     * @see Event::GetData()
     */
    ic_utils::Json::Value GetData() override;

    /**
     * Method to check if the given field key is a member of data or not
     * @param[in] rstrKey String containing key value
     * @return true if key is present, false otherwise
     */
    virtual bool Hasfield(const std::string &rstrKey);

    /**
     * Method to get timezone 
     * @param void
     * @return Timezone integer value 
     */
    virtual int GetTimezone();

    #ifdef IC_UNIT_TEST
    friend class CEventWrapperTest;
    #endif
};
} /* namespace ic_core */
#endif /* CEVENT_WRAPPER_H */
