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
* \file: CClientInfo.h
*
* \brief: Class to provide an interface to Event library locally
          to read specific Client information.
*******************************************************************************
*/

#ifndef CCLIENTINFO_H
#define CCLIENTINFO_H

#include "CIgniteLog.h"
#include "CIgniteMessage.h"

namespace ic_event
{

/*
 * Class to provide an interface to Event library locally
 *   to read specific Client information.
 */
class CClientInfo
{
private:
    //! Client connection ID
    int m_nConnId;

public:
    /**
     * Default constructor
     */
    CClientInfo() : m_nConnId(-1)
    {

    }

    /**
     * Destructor
     */
    ~CClientInfo()
    {
        //for valid connection id, close the connection.
        if (m_nConnId != -1 )
        {
            CIgniteMessage::CloseConnection(m_nConnId);
        }
    }

    /**
     * Method to read the given config value from Client.
     * @param[in] rstrKey config value key
     * @return the string value of the given config key
     */
    std::string GetConfigValue(const std::string &rstrKey)
    {
        HCPLOG_METHOD();

        //if connection id is invalid, open the connection
        if (m_nConnId < 0)
        {
            m_nConnId = CIgniteMessage::OpenConnection();
        }
        HCPLOG_METHOD() << " ConnId=" << m_nConnId;

        //send a request to Client over socket message
        CIgniteMessage configReq(CMessageTypes::eGET_CONFIG, m_nConnId);
        configReq.SetMessage(rstrKey);

        std::string strReply;
        if (configReq.Send(strReply) != -1)
        {
            HCPLOG_METHOD() << ", key=" << rstrKey << ", strReply=" << strReply;
        }
        else
        {
            HCPLOG_METHOD() << " Error on key=" << rstrKey;
        }

        return strReply;
    }

    /**
     * Method to read the DeviceID from Client.
     * @param void
     * @return the string value of the DeviceID
     */
    std::string GetDeviceId()
    {
        //if connection id is invalid, open the connection
        if (m_nConnId < 0)
        {
            m_nConnId = CIgniteMessage::OpenConnection();
        }

        //send a request to Client over socket message
        CIgniteMessage deviceIdReq(CMessageTypes::eGET_DEVICEID, m_nConnId);
        std::string strReply;
        if (deviceIdReq.Send(strReply) != -1)
        {
            HCPLOG_METHOD() << ", deviceId=" << strReply;
        }
        else
        {
            HCPLOG_METHOD() << " Error retreiving DeviceID!";
        }

        return strReply;
    }
};

}

#endif //CCLIENTINFO_H
