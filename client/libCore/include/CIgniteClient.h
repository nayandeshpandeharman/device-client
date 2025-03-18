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
* \file CIgniteClient.h
*
* \brief This class/module implements client related utilities.     
*******************************************************************************
*/

#ifndef CIGNITE_CLIENT_H
#define CIGNITE_CLIENT_H

#include <string>
#include "IClientMessageDispatcher.h"
#include "IMessageHandler.h"
#include "IOnOff.h"
#include "IProduct.h"
#include "IExternalHttpSessionHandler.h"

namespace ic_core 
{
/**
 * Enum of application exit codes to be used by modules if they call 
 * RestartAnalytics() API.
 */
enum ExitCode
{
    eDISASSOCIATION_EXIT=100,             ///< Disassociation exit code
    eOPTOUT_ACTIVATION_TO_ANONYMOUS_EXIT, ///< Optout activation to anonymous exitCode
    eOPTOUT_ANONYMOUS_TO_ACTIVATION_EXIT, ///< Optout anonymous to activation exitCode
    eWIPE_DATA_EXIT                       ///< Wipe data exit code
};

/**
 * Enum of ignite client running state
 */
enum IC_STATE
{
    eSTARTED, ///< Ignite client started state
    eEXITING, ///< Ignite client exiting state
    eRUNNING, ///< Ignite client running state
    ePAUSED,  ///< Ignite client paused state
    eUNKNOWN  ///< Ignite client unknown state
};

/**
 * Enum of queryId used to persist and query data in/from DB
 */
enum IC_QUERY_ID
{
    eACTIVATION_STATUS,           ///< Activation status query
    eIC_CURRENT_STATE,            ///< Ignite client current state query
    eDB_SIZE,                     ///< DB size query
    eMQTT_CONN_STATUS,            ///< MQTT connection status query
    eDEVICE_ID,                   ///< Device id query
    eLAST_SUCCESSFUL_BATCH_UPLOAD ///< Last successful batch upload query
};

/**
 * Enum of device activation status
 */
enum IC_ACTIVATION_STATUS
{
    eNOT_ACTIVATED, ///< Ignite client not activated
    eACTIVATED      ///< Ignite client activated
};

/**
 * Enum of client application exit status
 */
enum IC_EXIT_TYPE
{
    eQUICK_EXIT = 1, ///< Ignite client quick exit
    eNORMAL_EXIT     ///< Ignite client normal exit
};

/**
 * Enum of MQTT connection state
 */
typedef enum
{
    eSTATE_NOT_CONNECTED, ///< MQTT not connected
    eSTATE_CONNECTION_COMPLETE, ///< MQTT connection completed
    eSTATE_CONNECTED_BUT_TOPICS_SUBS_PENDING, ///< MQTT connected but topics subs pending
    eSTATE_CONNECTION_TEARING_DOWN ///< MQTT connection tearing down
} MQTT_CONNECTION_STATE;

/**
 * Structure to maintain the library name and version
 */
typedef struct
{
    std::string m_strLibName; ///< Library Name
    std::string m_strLibVersion; ///< Version corresponding the m_strLibName

}LibVersionDetails;

/**
 * Class CIgniteClient handles ignite client related utilities
 */
class CIgniteClient
{
public:
    /**
     * Method to initializes ignite analytics process
     * @param[in] strDbEncryptSeed string containing database encryption key
     * @param[in] bSeedChanged status to delete existing events from database
     * @return 0 on successfully initialize ignite analytics, non zero otherwise
     */
    static int InitAnalytics(std::string strDbEncryptSeed = "",
                             bool bSeedChanged = false );

    /**
     * Method to initializes Ignite Client (creation of config, db).
     * @param[in] strConfigPath string containing config file path
     * @param[in] bOutputInfoAndExit OutputInfoAndExit status
     * @param[in] bTriggerUploadAndExit TriggerUploadAndExit status
     * @return 0 on successfully initialize Ignite Client, non zero otherwise
     */
    static int InitClient( std::string strConfigPath = "", 
                           bool bOutputInfoAndExit= false,
                           bool bTriggerUploadAndExit = false );

    /**
     * Method to set instance of IProduct class
     * @param[in] pImpl instance of IProduct
     * @return 0 on successfully set class instance, non zero otherwise
     */
    static int SetProductImpl(IProduct* pImpl);

    /**
     * Method to get instance of IProduct class
     * @param void
     * @return instance of IProduct class
     */
    static IProduct* GetProductImpl();

    /**
     * Method to set instance of IClientMessageDispatcher class
     * @param[in] pDispatcher instance of IClientMessageDispatcher
     * @return 0 on successfully set class instance, non zero otherwise
     */
    static int SetClientMessageDispatcher(IClientMessageDispatcher* pDispatcher);

    /**
     * Method to get instance of IClientMessageDispatcher class
     * @param void
     * @return instance of IClientMessageDispatcher class
     */
    static IClientMessageDispatcher* GetClientMessageDispatcher();

    /**
     * Method to start ignite analytics process
     * @return 0 on successfully started ignite analytics, non zero otherwise
     */
    static int StartAnalytics();

    /**
     * Method to suspend ignite analytics process
     * @param void
     * @return void
     */
    static void SuspendAnalytics();

    /**
     * Method to resume ignite analytics process
     * @param void
     * @return void
     */
    static void ResumeAnalytics();

    /**
     * Method to stop ignite analytics process
     * @param void
     * @return void
     */
    static void StopAnalytics();

    /**
     * Method to get the client application version
     * @param void
     * @return client application version string
     */
    static std::string GetClientApplicationVersion();

    /**
     * Method to set the application version
     * @param[in] rStrClientVersion client application version
     * @return true if successfully set version, false otherwise
     */
    static bool SetClientApplicationVersion(const std::string &rStrClientVersion);

    /**
     * Method to set versions of additional/external libraries which are being 
     * used.These library versions will be printed on console/log file. 
     * @param[in] rvectLibraryVersionDetails Vector of library details
     * @return true if successfully version is updated, false otherwise
     */
    static bool SetAdditionalLibraryVersions(
              const std::vector<LibVersionDetails> &rvectLibraryVersionDetails);

    /**
     * Method to be invoked when device wants to send the pre-shutdown 
     * notification
     * @param[in] nGraceTime grace time in seconds
     * @param[in] bExitOnComplete flag value to indicate if Client has to exit 
     * upon completing the shutdown preparation
     * @param[in] eExitType If bExitOnComplete is specified, exitType can also
     * be specified as an optional parameter. If not specified, eQUICK_EXIT will
     * be considered as default option.
     * @return void
     */
    static void PrepareForShutdown(int nGraceTime, bool bExitOnComplete, 
                                   IC_EXIT_TYPE eExitType = eQUICK_EXIT);

    /**
     * Method to be invoked when device wants to send the shutdown-initiated 
     * notification
     * @param void
     * @return void
     */
    static void ShutdownInitiated();

    /**
     * Method to complete the shutdown process
     * @param void
     * @return void
     */
    static void CompleteShutdown();

     /**
     * Method to restart analytics application
     * @param[in] nRestartCode application exit code enum
     * @param[in] bClearDB flag to delete database before exiting
     * @return void
     */
    static void RestartAnalytics(int nRestartCode, bool bClearDB = false);

    /**
     * Method to disconnects MQTT connection
     * @param void
     * @return void
     */
    static void DisconnectMQTT();

    /**
     * Method to start notification
     * @param void
     * @return 0 on successfully started notification, non zero otherwise
     */
    static int StartNotifications();

    /**
     * Method to stop notification
     * @param void
     * @return 0 on successfully stopped notification, non zero otherwise
     */
    static int StopNotifications();

    /**
     * Method to add additional analytics handler
     * @param[in] instance of IMessageHandler class
     * @return 0 on successfully add analytics handler, non zero otherwise
     */
    static IClientConnector* AddAnalyticsHandler(IMessageHandler* pHandler);

    /**
     * Method to get instance of IOnOff class
     * @param void
     * @return instance of IOnOff class
     */
    static IOnOff* GetOnOffMonitor();

    /**
     * Method to get parameter based on input parameter
     * @param[in] eId query id
     * @return JSON query parameter
     */
    static ic_utils::Json::Value QueryParameter(const enum IC_QUERY_ID eId);

    /**
     * Method to set an external HTTP session handler that implements the  
     * IExternalHttpSessionHandler interface
     * @param[in] pHandler instance of IExternalHttpSessionHandler
     * @return void
     */
    static void SetExternalHttpSessionHandler(
                                         IExternalHttpSessionHandler *pHandler);

private:
    /**
     * Enum of ignite client running state
     */
    enum ClientStatus
    {
        eUNKNOWN, ///< Client unknown status
        eSTARTING, ///< Client starting status
        eRUNNING, ///< Client running status
        eSUSPENDED, ///< Client suspended status
        eSTOPPED ///< Client stopped status
    };

    /**
     * Method to send 'IgniteClientLaunched' event if it is not
                       sent yet in the current ignition cycle
     * @param void
     * @return void
     */
    static void SendClientLaunchedEvent();

    /**
     * Method to clean attachments folder whenever client needs
     *                  to be started new
     * @param void
     * @return void
     */
    static void CleanupAttachments();

    /**
     * Method to persist and send client running status
     * @param[in] eState client running status enum value
     * @return void
     */
    static void PersistAndBroadcastICStatus(enum IC_STATE eState);

    //! Member variable to store client running status
    static int m_nClientStatus;

    //! Member object to store the external http session handler.
    static IExternalHttpSessionHandler *m_pExtSessionHandler;
};
} /* namespace ic_core */
#endif /* CIGNITE_CLIENT_H */
