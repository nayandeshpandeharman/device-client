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

#ifndef ICONN_NOTIFICATION_H
#define ICONN_NOTIFICATION_H

namespace acp {

class IConnNotification {
public:
    //enum of direct (http & mqtt) and external connection notification types
    enum ConnNotifyType {
        eUL_CONN_NOTIF_DIRECT_HTTP_CONNECTED, 
        eUL_CONN_NOTIF_DIRECT_HTTP_DISCONNECTED,
        eUL_CONN_NOTIF_DIRECT_STREAM_CONNECTED,
        eUL_CONN_NOTIF_DIRECT_STREAM_DISCONNECTED,
        eUL_CONN_NOTIF_EXTERNAL_CONNECTED,
        eUL_CONN_NOTIF_EXTERNAL_DISCONNECTED
    };

    /**
     * Interface method to notify the connection status
     * @param[in] nType enum of upload connection notification
     * @return  void
     */
    virtual void ConnNotificationUpdate(
                                IConnNotification::ConnNotifyType nType) = 0;
};

}

#endif //ICONN_NOTIFICATION_H
