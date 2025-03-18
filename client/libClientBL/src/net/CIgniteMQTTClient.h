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
* \file CIgniteMQTTClient.h
*
* \brief This class provides the interface functions for connecting using
* MQTT protocol.
********************************************************************************
*/

#ifndef CIGNITE_MQTT_CLIENT_H
#define CIGNITE_MQTT_CLIENT_H

#include "CMQTTClient.h"
#include "CIgniteClient.h"
#include "IConnNotification.h"
#include "IOnOffNotificationReceiver.h"

namespace ic_bl
{

/**
 * Class implements methods which would be used for managing the MQTT topic
 * information for corrsponding service, as configured in config file.
 */
class TopicInfo
{
public:

    //! member variable to hold the publish topic string
    std::string m_strPublishTopic;

    //!  member variable to hold the subscribe topic string
    std::string m_strSubscribeTopic;

    //!  member variable to hold QoS value
    int m_nQos;

public:
    /**
     * Parameterized Constructor
     * @param[in] strPubtopic Publish topic
     * @param[in] strSubTopic Subscribe topic
     * @param[in] nQos Quality Of Service value
     * @return No return
     */
    TopicInfo(std::string strPubtopic, std::string strSubTopic, int nQos) : 
                    m_strPublishTopic(strPubtopic), 
                    m_strSubscribeTopic(strSubTopic), 
                    m_nQos(nQos){};

    /**
     * Default Constructor
     */
    TopicInfo(){};

    /**
     * Method to get the publish topic
     * @param void
     * @return Publish topic
     */
    std::string GetPubTopic() { return m_strPublishTopic; }

    /**
     * Method to get the subscribe topic
     * @param void
     * @return Publish topic
     */
    std::string GetSubTopic() { return m_strSubscribeTopic; }

    /**
     * Method to get the QoS
     * @param void
     * @return QoS value corresponding to the service
     */
    int GetQOs() { return m_nQos; }
};

/**
 * Class implements methods which would be used for connecting to MQTT host and 
 * uploading the data.
 */
class CIgniteMQTTClient :  public ic_network::CMQTTClient, 
                          public ic_core::IOnOffNotificationReceiver 
{
public:
    /**
     * Method to get instance of CIgniteMQTTClient class
     * @param void
     * @return instance of CIgniteMQTTClient class
     */
    static CIgniteMQTTClient* GetInstance();

    /**
     * Method to Upload / publish alerts
     * @param[in] pnMid  Message id of data massage to be published
     * @param[in] pvoidAlertPayload Buffer to be uploaded
     * @param[in] nSize Size of buffer
     * @param[in] rstrVendor  If it is not empty then buffer will be uploaded 
     * on its corresponding topic
     * @return 0 on success.
     */
    int PublishAlerts(int* pnMid, const void* pvoidAlertPayload, 
                      const int nSize, const std::string& rstrVendor="");

    /**
     * Method to Upload / publish event
     * @param[in] pnMid  Message id of data massage to be published
     * @param[in] pvoidEventPayload Buffer to be uploaded
     * @param[in] nSize Size of buffer
     * @param[in] rstrVendor  If it is not empty then buffer will be uploaded
     * on its corresponding topic
     * @return 0 on success.
     */
    int PublishEvents(int *pnMid, const void *pvoidEventPayload,
                      const int nSize, const std::string &rstrVendor = "");

    /**
     * Method to Upload / publish events on a particular topic
     * @param[in] pnMid  Message id of data massage to be published
     * @param[in] pvoidEventPayload Buffer to be uploaded
     * @param[in] nSize Size of buffer
     * @param[in] rstrtopic Topic on which the events needs to be uploaded
     * @return 0 on success.
     */
    int PublishEventsOnTopic(int *pnMid, const void *pvoidEventPayload,
                             const int nSize, const std::string &rstrtopic);

    /**
     * Method to populate the services topic from config file
     * @param[in] strDeviceId Device Id
     * @param[in] strTopicPrefix topic prefix to be used
     * @return void
     */
    void PopulateServicesTopic(std::string strDeviceId, 
                               std::string strTopicPrefix);

    /**
     * overridding MQTTClient::stopClient
     * @see ic_network::CMQTTClient::stopClient
     */
    virtual int StopClient();

    /**
     *  virtual method of mosqpp::mosquittopp
     */
    virtual void on_connect(int nRc);

    /**
     *  virtual method of mosqpp::mosquittopp
     */
    virtual void on_disconnect(int nRc);

    /**
     *  virtual method of mosqpp::mosquittopp
     */
    virtual void on_publish(int nMid);

    /**
     *  virtual method of mosqpp::mosquittopp
     */
    virtual void on_log(int nLevel, const char* cstrStr);

    /**
     *  virtual method of mosqpp::mosquittopp
     */
    virtual void on_subscribe(int nMid, int nQosCount, const int *pnGrantedQos);

    /**
     * Method to get the device id
     * @param void
     * @return Device Id
     */
    std::string GetDeviceId() 
    {
        return m_strDeviceID;
    }

    /**
     * Method to read config and constructs event and alert topics.
     * @param void
     * @return void
     */
    void ReloadPublishTopic();

    /**
     * Method to enable the subscriber to get connection notifications.
     * @param[in] pCNotif Class implementing IConnNotification pointer object
     * @return void
     */
    void RegisterForConnNotif(acp::IConnNotification *pCNotif);

    /**
     * Method to Explicitly disconnect the MQTT connection
     * @param void
     * @return Returns value of mosquitto::disconnect()
     */
    int DisconnectConn();

    /**
     * overridding IOnOffNotificationReceiver::NotifyShutdown
     * @see ic_core::IOnOffNotificationReceiver::NotifyShutdown
     */
    void NotifyShutdown() override;

    /**
     * Method to reset m_nPUBACKCurrLogCnt
     * @param void
     * @return void
     */
    void ResetPUBACKLogCounter();

    /**
     * Method to set m_nPUBACKLogMaxCnt
     * @param[in] nMaxcnt Value to be set for PUBACKLogMaxCnt
     * @return void
     */
    void SetPUBACKLogStatus(int nMaxcnt);

    /**
     * Method to get current state of mqttConnection in Json format
     * @param void
     * @return Returns mqtt connection status payload
     */
    ic_utils::Json::Value GetCurrentConnectionState();

    /**
     * Method to get status of the mqtt component
     * @param void
     * @return Returns component related info string
     */
    std::string GetComponentStatus();
    
    //! Internal enum list to trace mqtt connection steps
    typedef enum
    {
        eSTEP_DISCONNECTED, ///< Connection is disconnected
        eSTEP_CONNECTED_BUT_SUBS_PENDING, ///< Connected SUBACK pending
        eSTEP_CONNECTED_AND_SUBS_ACK_RCVD, ///< Connected all SUBACKs received
        eSTEP_CONNECTION_TEARING_DOWN ///< Connection tearing down
    } MQTT_CONNECTION_STEPS;

#ifdef IC_UNIT_TEST
    friend class CIgniteMQTTClientTest;

    /**
     * Method to get username prefix used for the mqtt communication
     * @param void
     * @return Username prefix string
     */
    std::string GetUnameprefix(){return m_strUnameprefix;}

    /**
     * Method to test OnMessageReceived
     * @param[in] rstrTopic MQTT topic
     * @param[in] rstrPayload The json notification payload in string format
     * @return True if the payload received is valid json , false otherwise
     */
    bool TestOnMessageReceived(const std::string &rstrTopic,
                               const std::string &rstrPayload) 
    {
       return OnMessageReceived(rstrTopic, rstrPayload);
    };
#endif

private:
    /**
     * Constructor 
     */
    CIgniteMQTTClient();

    /**
     * Destructor
     */
    virtual ~CIgniteMQTTClient();

    /**
     * Overriding CMQTTClient::IntiRun
     * @see ic_network::CMQTTClient::InitRun 
     */
    virtual void InitRun() override;

    /**
     * Overriding CMQTTClient::ExitRun
     * @see ic_network::CMQTTClient::ExitRun
     */
    virtual void ExitRun() override;

    /**
     * Overriding CMQTTClient::ReloadTopics
     * @see ic_network::CMQTTClient::ReloadTopics
     */
    virtual void ReloadTopics() override;

    /**
     * Overriding CMQTTClient::GetMaxInflightMsgCnt
     * @see ic_network::CMQTTClient::GetMaxInflightMsgCnt
     */
    virtual bool GetMaxInflightMsgCnt(unsigned int &rnMaxMsgCnt) override;

    /**
     * Overriding CMQTTClient::GetCredentials
     * @see ic_network::CMQTTClient::GetCredentials
     */
    virtual bool GetCredentials(std::string &rstrDeviceID,
                                std::string &rstrAuthToken,
                                std::string &rstrClientid) override;

    /**
     * Overriding CMQTTClient::OnMessageReceived
     * @see ic_network::CMQTTClient::OnMessageReceived
     */
    virtual bool OnMessageReceived(const std::string &rstrTopic,
                                    const std::string &rstrPayload) override;

    /**
     * Method to send MQTT Connection status to device
     * @param[in] eConnState MQTT connection Status
     * @param[in] bForced flag if the status to be broadcasted
     *               forcefully; default is false
     * @return true if status to device is sent successfuly else false
     */
    bool BroadcastConnStatus(MQTT_CONNECTION_STEPS eConnState,
                                                bool bForced=false);

    //! condition variable 
    std::condition_variable m_WaitCondition;

    //! mutex variable
    std::mutex m_WaitMutex;

    //! member variable used to keep track of how many topics are subscribed
    static int m_nNoOfTopicsToSubscribe;

    //! Count of topic SUBACKs received 
    /*!
     * Member variable used to keep track of how many subscribed topics 
     * received acknowledgement
     */
    static int m_nNoOfTopicSubAckRcvd;

    /**
     * Method to wait for specified time in milliseconds
     * @param[in] unTimeInMs Wait time in milliseconds
     * @return void
     */
    void Wait(unsigned int unTimeInMs);

    /**
     * Method to notify post usage of critical section
     * @param void
     * @return void
     */
    void Notify();

    //! member variable for storing device id
    std::string m_strDeviceID;

    //! Username Prefix
    /*!
     * member variable for storing username prefix, to be used while 
     * connecting to mqtt
     */ 
    std::string m_strUnameprefix;

    //! Topic prefix
    /*!
     * member variable for storing topic prefix, to be used while
     * sending data on to particular mqtt topic
     */
    std::string m_strTopicprefix;

    //! member variable for storing config topic
    std::string m_strConftopic;

    //! member variable for storing alert topic
    std::string m_strAlerttopic;

    //! member variable for storing event topic
    std::string m_strEventtopic;

    //! member variable for storing service/feature to topic map
    std::map<std::string, TopicInfo> m_mapFeaturesTopicMap;

    //! member variable for storing publish topic mutex to be used for scopelock
    ic_utils::CIgniteMutex m_PubTopicMutex;

    //! member variable to store the notification subscriber object
    acp::IConnNotification *m_pConnNotif;

    //! member variable to store last mqtt status broadcasted
    ic_core::MQTT_CONNECTION_STATE m_eLastBroadcasted;

    //! used to restrict the logs to print mid on_publish as critical logs
    int m_nPUBACKLogMaxCnt;

    //! used to print the mid on_publish upto PUBACKLogMaxCnt
    int m_nPUBACKCurrLogCnt;

    /**
     * Method to broadcast MQTT disconnect status to device
     * @param[in] bForced flag if the status to be broadcasted
     *               forcefully; default is false
     * @return true if disconnect status is successfully broadcasted,
     * false otherwise
     */
    bool BroadcastDisconnectStatus(bool bForced=false);

    /**
     * Method to broadcast MQTT connect but topic subscription is pending status
     * to device
     * @param[in] bForced flag if the status to be broadcasted
     *               forcefully; default is false
     * @return true if connect but topic subscription is pending status is 
     * successfully broadcasted, false otherwise
     */
    bool BroadcastConnSubsPendingStatus(bool bForced=false);

    /**
     * Method to broadcast MQTT connect with topic subscription ACK received
     * status to device
     * @param[in] bForced flag if the status to be broadcasted
     *               forcefully; default is false
     * @return true if connect with topic subscription ACK received status 
     * is successfully broadcasted, false otherwise
     */
    bool BroadcastConnSubsAckRcvdStatus(bool bForced=false);

    /**
     * Method to broadcast MQTT connection Tearing down status to device
     * @param void
     * @return true if connection Tearing down status is successfully 
     * broadcasted, false otherwise
     */
    bool BroadcastConnTearingDownStatus(bool bForced=false);

    /**
     * Method to get Cafile based on input parameter
     * @param[in] rjsonSsl ssl json config
     * @return string containing cafile, otherwise return empty string
     */
    std::string GetCAFile(const ic_utils::Json::Value &rjsonSsl);

    /**
     * Method to get Capath based on input parameter
     * @param[in] rjsonSsl ssl json config
     * @return string containing capath, otherwise return empty string
     */
    std::string GetCAPath(const ic_utils::Json::Value &rjsonSsl);

    /**
     * Method to get Certfile based on input parameter
     * @param[in] rjsonSsl ssl json config
     * @return string containing certfile, otherwise return empty string
     */
    std::string GetCertFile(const ic_utils::Json::Value &rjsonSsl);

    /**
     * Method to get key path based on input parameter
     * @param[in] rjsonSsl ssl json config
     * @return string containing key path, otherwise return empty string
     */
    std::string GetKeyPath(const ic_utils::Json::Value &rjsonSsl);

    /**
     * Method to get insecure status based on input parameter
     * @param[in] rjsonSsl ssl json config
     * @return insecure status
     */
    bool GetInsecureStatus(const ic_utils::Json::Value &rjsonSsl);

    /**
     * Method to get use TLS engine status based on input parameter
     * @param[in] rjsonSsl ssl json config
     * @return use TLS engine status
     */
    bool GetUseTLSEngineStatus(const ic_utils::Json::Value &rjsonSsl);

    /**
     * Method to get TLS engine id based on input parameter
     * @param[in] rjsonSsl ssl json config
     * @return string containing TLS engine id, otherwise return empty string
     */
    std::string GetTLSEngineID(const ic_utils::Json::Value &rjsonSsl);

    /**
     * Method to send firmware version event based on input parameters
     * @param[in] rstrDbfrmVersion DB firmware version
     * @param[in] rstrFrmVersion firmware version
     * @return void
     */
    void SendFirmwareVersionEvent(const std::string &rstrDbfrmVersion,
                                  const std::string &rstrFrmVersion);

    /**
     * Method to get token as password
     * @param void
     * @return password string
     */
    std::string GetTokenAsPassword();
};

} // namespace ic_bl
#endif // CIGNITE_MQTT_CLIENT_H
