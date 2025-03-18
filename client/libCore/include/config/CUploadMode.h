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
* \file CUploadMode.h
*
* \brief This class provide methods to handle different modes of event upload
*******************************************************************************
*/

#ifndef CUPLOAD_MODE_H
#define CUPLOAD_MODE_H

#include <bitset>
#include <exception>
#include <map>
#include <set>
#include <vector>
#include "CIgniteMutex.h"
#include "dam/CEventWrapper.h"
#include "db/CContentValues.h"

namespace ic_core 
{
/**
 * class CUploadMode handles different event upload modes over cloud
 */
class CUploadMode
{
public:
    /**
     * Destructor
     */
    ~CUploadMode();

    /**
     * Method to reload instance's resources
     * @param void
     * @return void
     */
    void ReloadInstance();

    /**
     * Method to get object of CUploadMode
     * @param void
     * @return Instance of CUploadMode
     */
    static CUploadMode* GetInstance();

    /**
     * Method to check if event is supported for batch mode
     * @param[in] rstrEventId eventID to check batch mode
     * @return true if event supported for batch mode, false otherwise
     */
    bool IsEventSupportedForBatch(const std::string& rstrEventId) const;

    /**
     * Method to check if event is supported for stream mode
     * @param[in] rstrEventId eventID to check stream mode
     * @return true if event supported for stream mode, false otherwise
     */
    bool IsEventSupportedForStream(const std::string& rstrEventId) const;

    /**
     * Method to check if batch mode is supported
     * @param void
     * @return true if batch mode supported, false otherwise
     */
    bool IsBatchModeSupported() const;

    /**
     * Method to check if stream mode is supported
     * @param void
     * @return true if stream mode supported, false otherwise
     */
    bool IsStreamModeSupported() const;

    /**
     * Method to check if anonymous mode is supported
     * @param void
     * @return true if anonymous mode supported, false otherwise
     */
    bool IsAnonymousUploadSupported() const;

    /**
     * Method to check by default batch mode is supported
     * @param void
     * @return true if batch mode supported, false otherwise
     */
    bool IsBatchModeSupportedAsDefault() const;

    /**
     * Method to check by default stream mode is supported
     * @param void
     * @return true if stream mode supported, false otherwise
     */
    bool IsStreamModeSupportedAsDefault() const;

    /**
     * Method to check if store and forward mode is supported
     * @param void
     * @return true if store and forward mode supported, false otherwise
     */
    bool IsStoreAndForwardSupported() const;

    /**
     * Method to get batch mode supported events list
     * @param void
     * @return batch mode supported events list 
     */
    std::set<std::string> GetBatchModeEventList() const;

    /**
     * Method to get stream mode supported events list
     * @param void
     * @return stream mode supported events list
     */
    std::set<std::string> GetStreamModeEventList() const;

    #ifdef IC_UNIT_TEST
        friend class AcpModeTest;
    #endif

private:
    /**
     * Enum of different event upload modes
     */
    enum SupportedModes
    {
        eSTREAM = 0, //! Stream upload mode
        eBATCH = 1 //! Batch upload mode
    };

    /**
     * Default no-argument constructor.
     */
    CUploadMode();

    /**
     * Method to update map of events and its upload mode based on configured 
     * event list and mode
     * @param[in] rjsonEventList configured event list,
     * @param[in] rstrMode configured upload mode
     * @return void
     */
    void FillEventModeMap(const ic_utils::Json::Value &rjsonEventList, 
                          const std::string &rstrMode);

    /**
     * Method to return bitset value for configured upload mode, if config is 
     * empty use defaultBitset as default value
     * @param[in] rstrMode configured upload mode
     * @param[in] defaultBitset default bitset value
     * @return 2 bit space boolean value for upload mode
     */
    std::bitset<2> GetModeBitSet(const std::string &rstrMode, 
                               std::bitset<2> defaultBitset = std::bitset<2>());

    /**
     * Method to read and initialize configured events upload mode
     * @param void
     * @return void
     */
    void Init();

    /**
     * Method to read supported modes based on input parameter
     * @param[in] rjsonSupported JSON object containing supported mode list
     * @param[in] rjsonEvents JSON object containing event supported list
     * @param[in] rTempConfiguredModes Bitset value of temp configured modes
     * @return void
     */
    void ReadSupportedModes(const ic_utils::Json::Value &rjsonSupported,
                            const ic_utils::Json::Value &rjsonEvents,
                            std::bitset<2> &rTempConfiguredModes);

    /**
     * Method to check if anonymous upload mode is supported or not based in 
     * input parameter
     * @param[in] rjsonUploadConfig JSON object containing upload config
     * @return true if mode is supported, false otherwise
     */
    bool IsAnonymousUploadModeSupported(const ic_utils::Json::Value &rjsonUploadConfig);

    //! Member variable stores configured event upload modes
    std::bitset<2> m_ConfiguredModes;

    //! Member variable stores default event upload mode
    std::bitset<2> m_DefaultMode;

    //! Member variable stores eventId as key and upload mode as value
    std::map<std::string, std::bitset<2>> m_mapEventModeMap;

    //! Member variable stores configured anonymous upload mode status
    bool m_bAnonymousUploadMode;

    //! Member variable stores configured store and forward enablement status
    bool m_bStoreAndForwardEnabled;

    //! Mutex variable
    mutable ic_utils::CIgniteMutex m_DataLock;
};
} /* namespace ic_core */

#endif //CUPLOAD_MODE_H
