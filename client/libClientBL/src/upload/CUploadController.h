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
* \file CUploadController.h
*
* \brief As DeviceClient may be extended to support different uploaders in 
* future, this module helps in excution of various such uploaders
********************************************************************************
*/

#ifndef CUPLOAD_CONTROLLER_H
#define CUPLOAD_CONTROLLER_H

#include "CMQTTUploader.h"
#include "CIgniteMutex.h"
#include "CUploadUtils.h"
#include "CConcurrentQueue.h"
#include <map>
#include "net/IConnNotification.h"
#include "IOnOffNotificationReceiver.h"

static std::string START_ALERT_UPLOAD = "";

namespace ic_bl 
{
/**
 * Class which provides methods for controlling the execution of various
 * uploaders 
 */
class CUploadController : public ic_utils::CIgniteThread, 
                          public ic_core::IOnOffNotificationReceiver
{
public:
    /** 
     * Method to get a singleton class object reference.
     * @param void
     * @return Pointer to an instance of CUploadController
     */
    static CUploadController* GetInstance();

    /**
     * Method to release an instance of CUploadController.
     * @param void
     * @return void
     */
    static void ReleaseInstance();

    /**
     * Method to release object's resources
     * @param void
     * @return void
     */
    void ReleaseResources();

    /**
     * Startup method to start the upload controller.
     * @param void
     * @return void
     */
    void StartUploadController();

    /**
     * Method to stop the upload controller.
     * @param void
     * @return void
     */
    void StopUploadController();

    /**
     * Method to suspend the upload controller.
     * @param void
     * @return void
     */
    void SuspendUploadController();

    /**
     * Method to resume the upload controller.
     * @param void
     * @return void
     */
    void ResumeUploadController();

    /**
     * Method to trigger forceUpload of stored events to cloud.
     * @param[in] bExitWhenDone To indicate if the stream upload should exit 
     *                          after this force upload
     * @return 0 if success, else returns -1
     */
    int ForceStreamUpload(bool bExitWhenDone = false);

    /**
     * Method to trigger isForceUploadEvent of stored events to cloud.
     * @param[in] rstrEventID Event ID of the event
     * @return true if event needs to be force Uploaded, 
     *         false if not needed to be forceUplaoded.
     */
    bool IsEventToBeForceUplaoded(const std::string& rstrEventID);

    /**
     * Method to trigger AlertUpload of stored events to cloud.
     * @param[in] rstrAlert Event ID of the Alert
     * @return 0 if alert is empty or parsing is successful, 
     *         else returns -1
     */
    int TriggerAlertsUpload(const std::string& rstrAlert);

    /**
     * Method to suspend stream uploader.
     * @param void
     * @return true on success, false on failure
     */
    bool SuspendStreamUpload();

    /**
     * Method to reload the periodicty since it has changed .
     * @param[in] nValue If nValue is > 0 , then periodicty can be set with the 
     *                  value provided.
     * @return true on success, false on failure
     */
    bool ReloadStreamPeriodicity(int nValue=0);

    /**
     * Overriding Method of IOnOffNotificationReceiver class
     * @see IOnOffNotificationReceiver::NotifyShutdown()
     */
    void NotifyShutdown() override;

private:
    /**
     * Default constructor
     */
    CUploadController();

    /**
     * Destructor
     */
    ~CUploadController();

    /**
     * Method to trigger mutex wait API.
     * @param void
     * @return void
     */
    void Wait();

    /**
     * Method to trigger mutex notify API.
     * @param void
     * @return void
     */
    void Notify();

    /**
     * Method for thread execution
     * @param void
     * @return void
     */
    virtual void Run();

    /**
     * Method to suspend direct uploaders.
     * @param void
     * @return void
     */
    void SuspendDirectUploaders();

    /**
     * Method to resume direct uploaders.
     * @param void
     * @return void
     */
    void ResumeDirectUploaders();

    //! thread variable for synchronization purpose
    ic_utils::CThreadCondition m_WaitCondition;

    //! mutex variable for synchronization purpose
    ic_utils::CIgniteMutex m_WaitMutex;

    //! stream uploader object
    CMQTTUploader *m_pMqttUploader;

    //! flag to indicate if uploaders are currently suspended
    bool m_bIsUploadersSuspended;

    //! flag to indicate shutdown is requested
    bool m_bIsShutdownInitiated = false;
};
} // namespace ic_bl
#endif /* CUPLOAD_CONTROLLER_H */

