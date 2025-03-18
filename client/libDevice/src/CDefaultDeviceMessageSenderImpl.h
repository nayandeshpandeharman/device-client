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
* \file CDefaultDeviceMessageDispatcher.h
*
* \brief CDefaultDeviceMessageDispatcher for handling messages that needs to be
* sent to the device
*******************************************************************************
*/

#ifndef CDEFAULT_DEVICE_MESSAGE_SENDER_IMPL_H
#define CDEFAULT_DEVICE_MESSAGE_SENDER_IMPL_H

#include <string>
#include "IDeviceMessageSender.h"

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CDefaultDeviceMessageSenderImpl"

namespace ic_device
{

/**
 * CDefaultDeviceMessageDispatcher is default DeviceMessageDispatcher class
 */
class CDefaultDeviceMessageSenderImpl : public IDeviceMessageSender
{
public:
    /**
     * Default constructor
     */
    CDefaultDeviceMessageSenderImpl()
    {
    }

    /**
     * Virtual destructor
     */
    virtual ~CDefaultDeviceMessageSenderImpl()
    {
    }

    /**
     * This function has no functionality implemented.
     * @see CDeviceMessageDispatcher::DispatchMessage()
     * @param[in] eMsgId Message ID
     * @param[in] strMsg Message string
     * @return Always true
     */
    bool DispatchMessage(MESSAGE_ID eMsgId, const std::string &rstrMsg) override
    {
        HCPLOG_D << "Not implemented! " << eMsgId << "-" << rstrMsg;
        return true;
    }
};
}
#endif //CDEFAULT_DEVICE_MESSAGE_SENDER_IMPL_H
