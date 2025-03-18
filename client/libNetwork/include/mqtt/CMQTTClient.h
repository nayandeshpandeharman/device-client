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
* \file CMQTTClient.h                                                           *
*
* \brief This class abstracts mosquitto MQTT client library and                *
*        enables to establish & communicate with the given mqtt broker         *
*        end-point                                                             *
*******************************************************************************
*/

#ifndef CMQTTCLIENT_H
#define CMQTTCLIENT_H

#include <string>
#include <map>
#include <mutex>
#include <condition_variable>
#include "CIgniteMutex.h"
#include "CIgniteThread.h"
#include "mosquittopp.h"

namespace ic_network
{

/**
 * CMQTTClient class abstracts mosquitto MQTT client library.
 */
class CMQTTClient :  public mosqpp::mosquittopp, public ic_utils::CIgniteThread 
{
public:
/*********** OVERRIDABLE FUNCTIONS***********/

    /**
     * Method to stop the client's connection with mqtt broker.
     * @param void
     * @return 0 on success
     */
    virtual int StopClient();

    /**
     * Method to trigger client's connection establishment with mqtt broke.
     * @param void
     * @return 0 on success
     */
    virtual int StartClient();

    /**
     * Method to be called before going thru power-cycle.Upon called, 
     * this method will initiate graceful tear down of the connection.
     * @param void
     * @return void
     */
    virtual void NotifyShutdown();


/*********** NONOVERRIDABLE FUNCTIONS***********/

    /**
     * Method to read the error string according to given int error code.
     * @param[in] error Int value of the error-code
     * @return Error string
     */
    std::string GetErrorString(int error);

    /**
     * Method which will return the current mqtt connection status.
     * @param void
     * @return True-if connected; false otherwise
     */
    bool IsConnected()
    {
        return m_bConnected;
    }

protected:
    /**
     * SSLAttributes structure to hold the SSL realted setting details
     */
    struct SSLAttributes 
    {
        std::string m_strCaFile; ///< Certificate Authority file name
        std::string m_strCaPath; ///< Certificate Authority file path
        std::string m_strCertFile; ///< Certificate file path
        std::string m_strKeyPath;  ///< Key file path
        bool m_bInsecure;          ///< To omit certificate name matching
        bool m_bUseTLSEngine; ///< TLS engine to be used or not
        std::string m_strTLSEngineID; ///< TLS Engine ID

        /**
         * Prameterized constructor
         * @param[in] rstrCafile Certificate Authority file name
         * @param[in] rstrCapath Certificate Authority file path
         * @param[in] rstrCertfile Certificate file path
         * @param[in] rstrKeypath Key file path
         * @param[in] rbInsecure To omit certificate name matching
         * @param[in] bUseTLSEngine TLS engine to be used or not
         * @param[in] strTLSEngineID TLS Engine ID
         */
        SSLAttributes(std::string &rstrCafile, std::string &rstrCapath,
                      std::string &rstrCertfile, std::string &rstrKeypath,
                      bool &rbInsecure, bool bUseTLSEngine = false,
                      std::string strTLSEngineID = "") :
                       m_strCaFile(rstrCafile), m_strCaPath(rstrCapath), 
                       m_strCertFile(rstrCertfile),m_strKeyPath(rstrKeypath),
                       m_bInsecure(rbInsecure),m_bUseTLSEngine(bUseTLSEngine),
                       m_strTLSEngineID(strTLSEngineID)
        {}
    };

    /**
     * Config structure to hold the MQTT related configration details
     */
    struct Config {
        SSLAttributes *m_pstSSL; ///< SSL attributes for the MQTT connection
        int m_nKeepAlive; ///< MQTT keep alive time in seconds
        std::string m_strHost; ///< MQTT host address (URL)
        int m_nPort; ///< MQTT host port number
    };

    /**
     * Default no-argument constructor
     */
    CMQTTClient();

    /**
     * Destructor
     */
    virtual ~CMQTTClient();

/***********PURE VIRTUAL FUNCTIONS TO BE IMPLEMENTED***********/

    /**
     * A pure virtual member.
     * Method to be implemented to have any initialization logics right before 
     * the thread starts running.
     * @param void
     * @return void
     */
    virtual void InitRun() = 0;

    /**
     * A pure virtual member.
     * Method to be implemented to have any deInitialization logics right before
     * the thread exits.
     * @param void
     * @return void
     */
    virtual void ExitRun() = 0;

    /**
     * A pure virtual member.
     * Method to be implemented to have any topic initialization logics.
     * @param void
     * @return void
     */
    virtual void ReloadTopics() = 0 ;

    /**
     * A pure virtual member.
     * Method to be implemented to load username, passcode and the Client ID.
     * @param[out] rstrUsername The username to establish the connection
     * @param[out] rstrPassword The password to establish the connection
     * @param[out] rstrClientID The clientID to establish the connection
     * @return True if the credentials are loaded successfully, false otherwise
     */
    virtual bool GetCredentials(std::string& rstrUsername, 
                                std::string& rstrPassword,
                                std::string& rstrClientID) = 0 ;

    /**
     * A pure virtual member.
     * Method to be implemented to read the maximum-inflight-msg-count if it has
     * to be set explicitly than the default value 20.A higher number here
     * results in greater message throughput , however may add additional cache
     * and load in the system
     * @param[out] runMaxMsgCnt Max-inflight-msg-count value.
     *                          if set to 0, default 20 will be used.
     * @return True if runMaxMsgCnt variable is set with a value.
     *         False if default value to be used. When false is returned, the
     *         value of runMaxMsgCnt variable will not be used.
     */
    virtual bool GetMaxInflightMsgCnt(unsigned int& runMaxMsgCnt) = 0;

    /**
     * A pure virtual member.
     * Method to be implemented to get the push notifications.
     * @param[in] rstrTopic The topic on which the notification is received
     * @param[in] rstrPayload The json notification payload in string format
     * @return True if the payload received is valid json , false otherwise
     */
    virtual bool OnMessageReceived(const std::string& rstrTopic, 
                                   const std::string& rstrPayload)=0;

/*********** OVERRIDABLE FUNCTIONS ***********/

    /**
     *  virtual method of mosqpp::mosquittopp 
     */
    virtual void on_connect(int rc);

    /**
     *  virtual method of mosqpp::mosquittopp
     */
    virtual void on_subscribe(int mid, int qos_count, const int* granted_qos);

    /**
     *  virtual method of mosqpp::mosquittopp
     */
    virtual void on_disconnect(int rc);

    /**
     *  virtual method of mosqpp::mosquittopp
     */
    virtual void on_publish(int mid);

    /**
     *  virtual method of mosqpp::mosquittopp
     */
    virtual void on_unsubscribe(int mid);

    /**
     *  virtual method of mosqpp::mosquittopp
     */
    virtual void on_log(int level, const char* str);

    /**
     *  virtual method of mosqpp::mosquittopp
     */
    virtual void on_error();

/*********** NONOVERRIDABLE FUNCTIONS ***********/

    //callback method of mosqpp::mosquittopp
    void on_message(const struct mosquitto_message* pMessage);

    /**
     * Method to be implemented to get the push notifications.
     * @param[out] pnMid A pointer to an int. If not NULL, the function will 
     *             set this to the message id of the particular message.
     *             This can be then used with the subscribe callback to 
     *             determine when the message has been sent.
     * @param[in] rstrTopic Topic name for which given topic to be subscribed.
     * @return MOSQ_ERR_SUCCESS(0) if success  OR
     *         MOSQ_ERR_INVAL(3) - if the input parameters were invalid OR
     *         MOSQ_ERR_NOMEM(1) - if an out of memory condition occurred OR
     *         MOSQ_ERR_NO_CONN(4) - if the client isn't connected to a broker.
     */
    int Subscribe(int* pnMid,const std::string& rstrTopic);

    /**
     * Method to be called to publish the message payload on the given topic.
     * @param[out] mid The message-id acquired post publishing the message
     * @param[in] payload The message payload
     * @param[in] size The size of the payload in bytes
     * @param[in] topic The topic name on which to publish the payload
     * @param[in] qos The qos level to use while publishing the message
     * @return 0 upon success; non-zero otherwise.
     */
    int PublishMessage(int* mid, const void* payload, const int size,
                                 const std::string& topic, const int& qos);

    /**
     *  virtual method of ic_utils::CIgniteThread 
     */
    virtual void Run();

    /**
     * Member variable to hold SSL and MQTT specific attributes
     */
    Config m_stConfig;

    /**
     * Member variable to maintain current connection status
     */
    bool m_bConnected;

    /**
     * Member variable to indicate if the connection is requested to be 
     * stopped/disconnected
     */
    bool m_bStopClientReq;

    /**
     * Member variable to indicate if there is a power-cycle request
     */
    bool m_bIsShutDownInitiated;

private:
    /**
     * Method to initialize the Client and establish connection with the broker.
     * @param void
     * @return 0 on success; non-zero on error.
     */
    int InitClient();

    /**
     * Method to set TLS values using m_stConfig.
     * @param void
     * @return void
     */
    void SetTLSValues(void);

    /**
     * Method to establish the mqtt connection and loop untill the connection 
     * is alive.
     * @param void
     * @return void
     */
    void Initialize(void);
};

} //namespace

#endif // CMQTTCLIENT_H
