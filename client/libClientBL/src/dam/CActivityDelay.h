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
* \file CActivityDelay.h
*
* \brief This class provides methods to computes defer time for upload and
* handles delayed events.                                                                
********************************************************************************
*/

#ifndef CACTIVITY_DELAY_H
#define CACTIVITY_DELAY_H

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include <map>
#include "dam/CTransportHandlerBase.h"

namespace ic_bl 
{
/**
 * Class CActivityDelay handles delayed event and computes defer time for 
 * uploadmanager
 */
class CActivityDelay : public CTransportHandlerBase 
{
public:
    
    /**
     * Parameterized constructor
     * @param[in] pNextHandler Pointer to the CTransportHandlerBase class
     */
    CActivityDelay(CTransportHandlerBase* pNextHandler);
    
    /**
     * Destructor
     */
    virtual ~CActivityDelay();

    /**
     * Method to compute defer for uploadmanager time based on its specific 
     * timeouts from config
     * @param void
     * @return defer time in seconds
     */
    int ComputeDeferUpload();

    /**
     * Method to handle event for computing defer time
     * @param[in] pEvent Event data
     * @return void
     */
    virtual void HandleEvent(ic_core::CEventWrapper* pEvent);

protected:

    //! Member variable stores eventId as key and its current time as value
    std::map<std::string, long long> m_mapDefervent;
    
    //! Member variable stores eventId as key and timeout in seconds as value
    std::map<std::string, int> m_mapEventConfigData;
};

} /* namespace ic_bl*/

#endif /* CACTIVITY_DELAY_H */
