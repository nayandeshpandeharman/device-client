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
* \file IOnOffNotificationReceiver.h
*
* \brief This class provides the necessary interfaces to be implemented by 
* individual components in order to receive shutdown notification
********************************************************************************
*/

#ifndef ION_OFF_NOTIFICATION_RECEIVER_H
#define ION_OFF_NOTIFICATION_RECEIVER_H

namespace ic_core 
{
/**
 * Interface class expose APIs necessary for receiving shutdown notification.
 */
class IOnOffNotificationReceiver
{
public:
    /**
     * Method to notify the receiver about shutdown.
     * @param void
     * @return void
     */
    virtual void NotifyShutdown() = 0;
};
} /* namespace ic_core */
#endif //ION_OFF_NOTIFICATION_RECEIVER_H