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
* \file CDBTransportWrapper.h
*
* \brief This class provides methods to access CDBTransport
********************************************************************************
*/

#ifndef CDB_TRANSPORT_WRAPPER_H
#define CDB_TRANSPORT_WRAPPER_H

#include "dam/CDBTransport.h"
#include "CIgniteMutex.h"
namespace ic_bl
{
/**
 * Class CDBTransportWrapper is wrapper for CDBTransport
 */
class CDBTransportWrapper 
{
public:
    /**
     * Method to get Instance of CDBTransport
     * @param void
     * @return Pointer to Singleton Object of CDBTransport
     */
    static CDBTransport* GetDBTransportInstance();

private:
    //! Member variable holding db mutex
    static ic_utils::CIgniteMutex m_DbMutex;
};
} /* namespace ic_bl*/
#endif /* CDB_TRANSPORT_WRAPPER_H */