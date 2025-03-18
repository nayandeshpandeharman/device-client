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
* \file CMQTTUploader.h
*
* \brief This class provides methods for handling the upload of 
* events/alerts/data over MQTT
********************************************************************************
*/

#ifndef CMQTT_UPLOADER_H
#define CMQTT_UPLOADER_H

#include "CIgniteThread.h"
#include "CConcurrentQueue.h"
#include <string.h>
#include <CIgniteMutex.h>
#include <set>
#include <map>
#include <mutex>
#include <condition_variable>
#include "IOnOffNotificationReceiver.h"
#include "CMidHandler.h"
#include "CIgniteConfig.h"
#include "net/CIgniteMQTTClient.h"

#define TRIGGER_ALERT_UPLOAD ""

using ic_core::IOnOffNotificationReceiver;

namespace ic_bl
{

/**
 * Class which provides methods for handling the upload of 
 * events/alerts/data over MQTT
 */
class CMQTTUploader : public ic_utils::CIgniteThread,
                      public ic_core::IOnOffNotificationReceiver, 
                      public ic_core::IConfigUpdateNotification
{
public:
    /**
     * Method to get an instance of CMQTTUploader
     * @param void
     * @return Pointer to an instance of CMQTTUploader
     */
    static CMQTTUploader* GetInstance(void);

    /**
     * Method to upload alerts
     * @param[in] rstrAlert Alert string that needs to be uploaded
     * if empty string then the alerts from database are uploaded.
     * @return  0 if success, else returns -1
     */
    int AlertUpload(const std::string &rstrAlert = TRIGGER_ALERT_UPLOAD );

    /**
     * Overriding ic_utils::CIgniteThread::Run
     * @see ic_utils::CIgniteThread::Run()
     */
    virtual void Run();
   
    /**
     * Method to force upload the events
     * @param[in] bExitWhenDone Indicates if the stream upload should exit
     * after this force upload
     * @return  0 if success, else returns -1
     */
    int ForceUpload(bool bExitWhenDone = false);

    /**
     * Method to check wither event is to be uploaded immediately
     * @param[in] rstrEventID Event ID
     * @return True if event is to be force uploaded, false otherwise
     */
    bool IsForceUploadEvent(const std::string &rstrEventID);

    /**
     * Method to suspend upload temporarily
     * @param void
     * @return True on success and false on failure
     */
    bool SuspendUpload();

    /**
     * Method to indicate periodicity needs to be updated from config
     * @param void
     * @return True on success and false on failure
     */
    bool ReloadPeriodicity();

    /**
     * Method to resume suspended upload
     * @param void
     * @return void
     */
    void ResumeUpload();

    /**
     * Overriding IOnOffNotificationReceiver::NotifyShutdown
     * @see ic_core::IOnOffNotificationReceiver::NotifyShutdown()
     */
    void NotifyShutdown() override;

    /**
     * Method to notify that configuration has been updated
     * @param void
     * @return void
     */
    void NotifyConfigUpdate();

    /**
     * Method to reset the uploader log counter
     * @param void
     * @return True on success and false on failure
     */
    bool ResetEventUploaderLogCounter();

    /**
     * Method to get the component related status
     * @param void
     * @return Component related status string
     */
    std::string GetComponentStatus();

    //! Enum to indicate timer type
    enum TimerType
    {
        eTT_EVENT = 0, ///< Time type for events
        eTT_ALERT, ///< Timer type for alerts
    };

    //! Enum for Upload errors
    enum UploadError
    {
        eUE_NO_CONNECTION = -1, ///< Upload error due to no connection
        eUE_UPLOAD_SUCCESS = 0, ///< Upload is successful
        eUE_DATA_NOT_AVAILABLE = 1, ///< Data is not available to upload
        eUE_PUBLISH_ERROR = 2   ///< Upload error as publish to MQTT failed
    };
    
    #ifdef IC_UNIT_TEST
        friend class CMQTTUploaderTest;
    #endif

private:
    /**
     * Class which provides the methods to manage the delays/timings of upload
     */
    class CIgniteTimer : public ic_utils::CIgniteThread 
    {
    public:
        /**
         * Method to get the component related status
         * @param[in] pUploader Pointer to CMQTTUploader of which the
         * RequestUpload will be called post the time expiry
         * @param[in] delay Delay time in seconds
         * @param[in] timerType Timer type(eTT_EVENT,eTT_ALERT)
         */
        CIgniteTimer(CMQTTUploader* pUploader, time_t delay,
                     TimerType timerType = TimerType::eTT_EVENT) ;

        /**
         * Overriding ic_utils::CIgniteThread::Start
         * @see ic_utils::CIgniteThread::Start()
         */
        virtual int Start();

    private:
        //! Member variable to hold the delay time in secs 
        time_t m_Delay;

        //! Pointer Variable for CMQTTUploader object
        /*!
         * Member variable to hold the pointer to CMQTTUploader of which the
         * RequestUpload will be called post the m_Delay time
         */
        CMQTTUploader* m_pUploader;

        //! Member variable to time type
        TimerType m_Timetype;

        /**
         * Overriding ic_utils::CIgniteThread::Run
         * @see ic_utils::CIgniteThread::Run()
         */
        virtual void Run();
    };

    /**
     * Default constructor
     */
    CMQTTUploader();

    /**
     * Destructor
     */
    ~CMQTTUploader();

    /**
     * Method to start MQTT upload
     * @param void
     * @return 0 on successful upload , -1 otherwise
     */
    int StartMQTTUpload();

    /**
     * Method to start Events upload
     * @param void
     * @return 0 on graceful exit
     */
    int StartEventsUpload(void);

    /**
     * Method to start Alerts upload
     * @param void
     * @return void
     */
    void StartAlertsUpload(void);

    /**
     * Method to upload the events over MQTT
     * @param void
     * @return 0 on success , non-zero otherwise
     */
    int UploadEvents(void);

    /**
     * Method to upload the alerts over MQTT
     * @param void
     * @return 0(eUE_UPLOAD_SUCCESS) on success , non-zero otherwise
     */
    UploadError UploadAlerts();

    /**
     * Method to start the events upload over MQTT topic
     * @param void
     * @return 0(eUE_UPLOAD_SUCCESS) on success , non-zero otherwise
     */
    int UploadTopicedEvents();

    /**
     * Method to upload events over specified MQTT topic
     * @param void
     * @return 0(eUE_UPLOAD_SUCCESS) on success , non-zero otherwise
     */
    int UploadAllTopicedEvent(
        std::map<long long, std::pair<std::string, std::string>> &rmapTopicedEvent);

    /**
     * Method to log upload event and its summary as per the count configured
     * in config file
     * @param[in] rstrLogEvntStr String containing the events
     * @param[in] nEvntCnt Number of events in the string
     * @return True of logging is successful , false otherwise
     */
    bool LogUploadEvntPayload(std::string &rstrLogEvntStr, int nEvntCnt);

    /**
     * Method to initialize the logging based on the config
     * @param void
     * @return True of initializing is successful , false otherwise
     */
    bool InitConfigBasedLogging();

    /**
     * Method to wait based on the timer type
     * @param[in] timerType Timer type (eTT_EVENT,eTT_ALERT)
     * @return void
     */
    void Wait(TimerType timerType);

    /**
     * Method to notify post usage of critical section based on time type
     * @param[in] timerType Timer type (eTT_EVENT,eTT_ALERT)
     * @return void
     */
    void Notify(TimerType timerType);

    /**
     * Method to wait based on the timer type
     * @param[in] unTimeOut Wait time in seconds 
     * @return void
     */
    void WaitWhileSuspended(unsigned int unTimeOut);

    /**
     * Method to notify to release wait mutex (locked with WaitWhileSuspended)
     * @param void
     * @return void
     */
    void NotifySuspended();

    /**
     * Method to request upload based on the timer type and delay
     * @param[in] timerType Timer type (eTT_EVENT,eTT_ALERT)
     * @param[in] delay Delay time in secs post which upload will be triggered
     * @return  0 if success, else returns -1
     */
    int RequestUpload(TimerType timerType, time_t delay = 0);

    /**
     * Method to request upload of alerts
     * @param void
     * @return void
     */
    void RequestAlertsUpload();

    /**
     * Method to initialize m_unMaxEventUploadCnt based on configuration
     * @param void
     * @return void
     */
    void InitEventsUploadCnt();

    /**
     * Method to set upload event log count value
     * @param[in] rnCnt upload event log count value
     * @return true if valid counter value is set, false otherwise
     */
    bool SetUploadEvntLogCount(const int &rnCnt);

    /**
     * Method to set upload summary logging iter count value
     * @param[in] rnCnt summary logging iter count value
     * @return true if valid counter value is set, false otherwise
     */
    bool SetSummaryLogIterCount(const int &rnCnt);

    /**
     * Method to prepare events data to publish on topic
     * @param[in]/[out] rnMid mid value
     * @param[in] rstrEventData string containing event data
     * @param[in]/[out] rnErr Event upload status
     * @param[in] rstrTopic topic name
     * @return returns true if data is prepared successfully and publish is 
     * initiated, otherwise return false if data could not be prepared
     */
    bool PrepareDataAndPublish(int &rnMid, const std::string &rstrEventData, 
                             int &rnErr, const std::string &rstrTopic);

    /**
     * Method to process published Alerts
     * @param[in] rnMid mid of the published alerts
     * @param[in] rvectRowIDs number of rows retrived retrived from db to
     *                    publish alerts
     * @return UploadError::eUE_UPLOAD_SUCCESS if processing is success else
     *         return UploadError::eUE_DATA_NOT_AVAILABLE
     */
    CMQTTUploader::UploadError ProcessPublishedAlerts(
                                const int &rnMid,
                                const std::vector <long long> &rvectRowIDs);
    
    /**
     * Method to verify data post publishing events to mqtt topic
     * @param[in] rnMid mid value returned after publishing events.
     * @param[in/out] rnErr error returned after publishing events, sets the
     *                value as -1 if verification fails.
     * @param[in] rvectRowIDs Number of rows retrived from db
     * @return true if data verification is success else return false
     */
    bool VerifyPostPublish(const int &rnMid, int &rnErr,
                        const std::vector <long long> rvectRowIDs);

    /**
     * Method to publish received events over mqtt topic
     * @param[out] rnMid mid value returned after publishing events.
     * @param[out] rnErr sets value to 0 if publish method is success else
     *                    sets the error code returned after publishing events.
     * @param[in] rstrEvents events payload received to publish to cloud. .
     * @return true if publishing events over events topic is success
     *         else return false
     */
    bool PublishEvents(int &rnErr, int &_nMid, std::string &rstrEvents);

    /**
     * Method to wait for next upload request
     * @param void
     * @return void
     */
    void WaitForNextUploadRequest();

    /**
     * Method to trigger event uploads
     * @param void
     * @return void
     */
    void TriggerEventUploads();

    /**
     * Method to process alert uploads
     * @param void
     * @return void
     */
    void ProcessAlertUploads();

    //! Member variable to hold CIgniteMQTTClient pointer
    CIgniteMQTTClient *m_pMqClient;

    //! Member variable to indicate if the compression of MQTT payload is enabled
    const bool m_bCompression;

    //! Member variable for event wait condition
    ic_utils::CThreadCondition m_EventWaitCondition;

    //! Member variable for alert wait condition
    ic_utils::CThreadCondition m_AlertWaitCondition;

    //! Member variable for event wait mutex
    ic_utils::CIgniteMutex m_EventWaitMutex;

    //! Member variable for alert wait mutex
    ic_utils::CIgniteMutex m_AlertWaitMutex;

    //! Member variable for event timer
    CIgniteTimer *m_EventTimer;

    //! Member variable for alert timer
    CIgniteTimer *m_AlertTimer;

    //! Member variable to indicate if upload is requested
    bool m_bUploadRequested;

    //! Member variable to indicate if the shutdown is requested
    bool m_bShutdownRequested;

    //! Member variable to indicate if upload is paused
    bool m_bPauseUploader;

    //! Member variable to indicate if alert is available for upload
    bool m_bAlertsAvailable;

    //! Member variable to indicate is if upload is suspended
    bool m_bUploadSuspended;

    //! Member variable to hold event periodicty value
    uint m_unEventPeriodicity;

    //! Member variable to indicate if the periodicity has changed
    bool m_bIsPeriodicityChanged;

    //! Mutex variables to lock with basicLockable requirement
    std::mutex m_WaitMutex;

    //! Wait condtion variable
    /*!
     * Condition variables to block a thread until another thread modifies the
     * condition or till timeout
     */
    std::condition_variable m_WaitCondition;

    //! Event Upload log count
    /*!
     * Variable is counter for uploadEventLogging set as per configuration,
     * used to print the uploaded events details
     */
    int m_nEvntUploadLogIterCount;

    //! Event upload summary count
    /*!
     * Variable is counter for summaryLogIterCount, used to print the summary
     * of uploaded events
     */
    int m_nEvntUploadSummaryIterCount;

    //! Current event upload count
    /*!
     * Variable to print the uploaded events details upto uploadEventLogging
     * counter
     */
    unsigned int m_unCurrEvntUploadCnt;

    //! Current event upload summary count
    /*!
     * Variable to print the summary of no of uploaded events on each
     * summaryLogIterCount interval
     */
    unsigned int m_unCurrEvntUploadSummaryCnt;

    //! Current Uploaded Event COunt
    /*!
     * Variable to print the no of uploaded events on each summaryLogIterCount
     * interval
     */
    unsigned int m_unCurrUploadedEvntCnt;

    //! Variable to store maximum number of events to be uploaded at a time
    unsigned int m_unMaxEventUploadCnt;

    //! Member variable to hold force upload events
    std::set<std::string> m_setForceUploadEvents;
};

} // namespace ic_bl

#endif // CMQTT_UPLOADER_H
