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
* \file IOnOff.h
*
* \brief This class provides the necessary interfaces to be implemented by 
* individual components to register/unregister with OnOff to receive shutdown 
* notification.
********************************************************************************
*/

#ifndef ION_OFF_H
#define ION_OFF_H

#include <string>
#include "IOnOffNotificationReceiver.h"

namespace ic_core 
{
/**
 * Interface class expose APIs necessary to register/unregister with OnOff for 
 * receiving shutdown notification.
 */
class IOnOff
{
public:
    
    /**
     * Enum for shutdown notification receivers
     */
    enum NotifReceiverCode
    {
        eR_CACHE_TRANSPORT, ///< CacheTransport receiver
        eR_DB_TRANSPORT, ///< DBTransport receiver
        eR_MESSAGE_QUEUE, ///< MessageQueue receiver
        eR_MESSAGE_CONTROLLER, ///< MessageController receiver
        eR_MQTT_CLIENT, ///< IgniteMQTTClient receiver
        eR_MQTT_UPLOADER, ///< MQTTUploader receiver
        eR_MID_HANDLER, ///< MidHandler receiver
        eR_NOTIFICATION_LISTENER, ///< NotificationListener receiver
        eR_PRODUCT_IMPL_CONTROLLER, ///< ProductImplController receiver
        eR_SENSOR_DATA_PROCESSOR, ///< SendorDataProcessor receiver
        eR_UPLOAD_CONTROLLER, ///< UploadController receiver
        eR_ZMQ_RECEIVE_MESSAGE, ///< ZMQReceiveMessage receiver
        eR_OTHER ///< Other receiver
        #ifdef IC_UNIT_TEST
            ,eR_UT_THREAD ///< UT_Thread receiver
        #endif 
    };

    /**
     * Method to register for shutdown notification.
     * @param[in] pRcvr notification receiver
     * @param[in] eRcvrCode notification receiver code
     * @param[in] strRcvrName notification receiver name
     * @return true if successfully registered, false otherwise
     */
    virtual bool RegisterForShutdownNotification(
                                              IOnOffNotificationReceiver* pRcvr,
                                              NotifReceiverCode eRcvrCode,
                                              std::string strRcvrName="") = 0;

    /**
     * Method to unregister for shutdown notification.
     * @param[in] eRcvrCode notification receiver code
     * @param[in] strRcvrName notification receiver name
     * @return true if successfully unregistered, false otherwise
     */
    virtual bool UnregisterForShutdownNotification(NotifReceiverCode eRcvrCode,
                                                std::string strRcvrName="") = 0;

    /**
     * Method to notify that the receiver is ready for shutdown.
     * @param[in] eRcvrCode receiver enum value
     * @param[in] strRcvrName receiver name
     * @return false if receiver code not found, true otherwise
     */
    virtual bool ReadyForShutdown(NotifReceiverCode eRcvrCode,
                                  std::string strRcvrName="") = 0;
};
} /* namespace ic_core */
#endif //ION_OFF_H
