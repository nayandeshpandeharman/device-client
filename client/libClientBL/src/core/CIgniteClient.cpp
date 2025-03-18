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

#include <sys/prctl.h>
#include <curl/curl.h>
#include <string>
#include <signal.h>
#include "CIgniteLog.h"
#include "CIgniteClient.h"
#include "CIgniteConfig.h"
#include "CIgniteDateTime.h"
#include "CMessageQueue.h"
#include "ClientBLLibVersion.h"
#include "EventLibVersion.h"
#include "CoreLibVersion.h"
#include "NetworkLibVersion.h"
#include "UtilsLibVersion.h"
#include "config/CConfigRequestHandler.h"
#include "config/CUploadMode.h"
#include "dam/CInsightEngine.h"
#include "dam/CCacheTransport.h"
#include "dam/CEventReceiver.h"
#include "db/CLocalConfig.h"
#include "notif/CNotificationListener.h"
#include "upload/CUploadController.h"
#include "core/CAesSeed.h"
#include "notif/CMQTTNotificationHandler.h"
#include "CDeviceDetails.h"
#include "dam/CMessageController.h"
#include "CHttpSessionManager.h"
#include "CClientConnectorImpl.h"
#include "core/CPersistancyAndStateHandler.h"
#include "net/CIgniteMQTTClient.h"
#include "CSSLAttributes.h"
#include "dam/CDBTransport.h"
#include "CPreIgniteLogger.h"
#include "CClientOnOff.h"

//! macro for log tag
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CIgniteClient"

#ifdef INCLUDE_IGNITE_AUTO
#include "AutoLibVersion.h"
#endif

namespace ic_core 
{
namespace 
{
//! Global variable to stores shutdown completed status
bool g_bIsShutdownCompleted = false;

//! Global variable to stores shutdown started status
bool g_bIsShutdownStarted = false;

//! Global variable to stores threadId for shutdown completed thread
pthread_t g_shutdownCompleteThread;

//! Global variable to stores 'IProduct' pointer object
IProduct *g_pProdImpl = NULL;

//! Global variable to stores 'IClientMessageDispatcher' pointer object
IClientMessageDispatcher* g_pClientMsgDispatcher = NULL;

//! Global variable to stores 'CMessageQueue' pointer object
CMessageQueue *g_pProcessor = NULL;

//! Constant key for 'RETRY_VALIDATION_SECS' integer value
static const time_t RETRY_VALIDATION_SECS = 5;

//! Static global variable to store client application version
static std::string g_strClientAppVersion = "";

//! Static global variable to store additional/external library version details
static std::vector<LibVersionDetails> g_vectAdditionalLibVersionDetails;

//! Global variable to stores 'CInsightEngine' pointer object
ic_bl::CInsightEngine *g_pEngineInstance = NULL;

//! Global variable to stores 'CMQTTNotificationHandler' pointer object
ic_bl::CMQTTNotificationHandler *g_pMqNotifReq = NULL;

//! Global variable to stores 'CUploadController' pointer object
ic_bl::CUploadController *g_pUploadController = NULL;

//! Global variable to stores 'CSSLAttributes' pointer object
ic_network::CSSLAttributes *g_pSslAttributes = NULL;

//! Global variable to stores 'CConfigRequestHandler' pointer object
ic_bl::CConfigRequestHandler *g_pCfgHandler = NULL;

//! Constant key for 'deviceDisassociated' string
static const std::string KEY_DISASSOCIATION = "deviceDisassociated";

//! Constant key for 'android' string
const std::string KEYSTORE_ANDROID = "android";

//! Constant key for 'ICRunningStatus' string
static const std::string KEY_IC_RUNNING_STATUS = "ICRunningStatus";

} /* end unnamed namespace */

int CIgniteClient::m_nClientStatus = CIgniteClient::eUNKNOWN;
IExternalHttpSessionHandler *CIgniteClient::m_pExtSessionHandler = nullptr;

/**
 * Method to invoke when a new thread is crated and start execution
 * @param[in] pvoidParams thread parameter
 * @return void pointer
 */
void* shutdown_complete_monitor(void *pvoidParams)
{
    pthread_detach(pthread_self());
    int nExitType = eQUICK_EXIT; // Default value

    if (pvoidParams)
    {
        nExitType = *((int *)pvoidParams);
        free(pvoidParams);
    }

    HCPLOG_C << "Waiting for shutdown completion.. exitType:" << nExitType;
    while (!g_bIsShutdownCompleted)
    {
        sleep(1);
    }
 
    switch(nExitType)
    {
        case eNORMAL_EXIT: 
        {
            HCPLOG_C <<"Shutdown completed.exiting normally...";
            ic_event::CIgniteEvent::ClearSender();
            ic_utils::CIgniteLog::CloseLogFile();
            exit(0);
        }
        break;
        case eQUICK_EXIT:
        default: 
        {
            HCPLOG_C <<"Shutdown completed.exiting quickly...";
            quick_exit(0);
        }
        break;
    }

    return NULL;
}

/**
 * Method to print additional library versions . Upper layer can set the
 * additional/external library versions using SetAdditionalLibraryVersions
 * method. which can be printed on console/log file.
 * @param[in] bPrintToConsole True if to be printed on console, otherwise 
 * versions will be printed in log file.
 * @return void
 */
void print_additional_library_versions(bool bPrintToConsole)
{
    std::vector<LibVersionDetails>::iterator iterStart =
        g_vectAdditionalLibVersionDetails.begin();
    for (; iterStart != g_vectAdditionalLibVersionDetails.end();
         iterStart++)
    {
        if (bPrintToConsole)
        {
            std::cout << iterStart->m_strLibName << " Version = "
                      << iterStart->m_strLibVersion << std::endl;
        }
        else
        {
            HCPLOG_C << iterStart->m_strLibName << " = "
                     << iterStart->m_strLibVersion;
        }
    }
}

/**
 * Method to set status in log for additional library versions . 
 * @param void
 * @return void
 */
void set_additional_lib_ver_status()
{
    std::vector<LibVersionDetails>::iterator iterStart =
        g_vectAdditionalLibVersionDetails.begin();
    for (; iterStart != g_vectAdditionalLibVersionDetails.end();
         iterStart++)
    {
        ic_utils::CIgniteLog::SetStatus(iterStart->m_strLibName,
                                        iterStart->m_strLibVersion);
    }
}

IProduct* CIgniteClient::GetProductImpl()
{
    if (g_pProdImpl == NULL)
    {
        HCPLOG_F << "Product Implementation has not been registered!";
    }
    return g_pProdImpl;

}

int CIgniteClient::SetProductImpl(IProduct *pProdImpl)
{

    if (pProdImpl)
    {
        g_pProdImpl = pProdImpl;
        return 0;
    }
    return -1;
}

bool CIgniteClient::SetClientApplicationVersion(const std::string &rstrClientVersion)
{
    g_strClientAppVersion = rstrClientVersion;
    return true;
}

bool CIgniteClient::SetAdditionalLibraryVersions(
    const std::vector<LibVersionDetails> &rvectLibraryVersionDetails)
{
    g_vectAdditionalLibVersionDetails = rvectLibraryVersionDetails;
    return true;
}

std::string CIgniteClient::GetClientApplicationVersion()
{
    return g_strClientAppVersion;
}

IClientMessageDispatcher* CIgniteClient::GetClientMessageDispatcher()
{
    if (g_pClientMsgDispatcher == NULL)
    {
        HCPLOG_W << "Message Dispatcher not implemented";
    }
    return g_pClientMsgDispatcher;

}

int CIgniteClient::SetClientMessageDispatcher(IClientMessageDispatcher *pDispatcher)
{
    if (pDispatcher)
    {
        g_pClientMsgDispatcher = pDispatcher;
        return 0;
    }
    return -1;
}

/**
 * Signal handler method which will be called for child native process
 * whenever parent(Client Application) process dies. As this function registered
 * to receive parent termination, it will stop itself.
 * @param[in] nSig signal
 * @return void pointer
 */
void client_signal_handler(int nSig)
{
    HCPLOG_W << "Received Acp Client stop signal";
    exit(0);
}

int CIgniteClient::InitAnalytics(std::string strDBEncryptSeed, bool bSeedChanged)
{
    HCP_PRELOG_W("Initializing Analytics...");
    HCPLOG_C << "***Launching Ignite Client " << g_strClientAppVersion << "***";
    HCPLOG_C << "Lib Ver(Event/Client/Core/Network/AcpUtils)~" <<
                    EVENT_LIB_VERSION << "/" <<
                    CLIENTBL_LIB_VERSION << "/" <<
                    CORE_LIB_VERSION << "/" <<
                    NETWORK_LIB_VERSION << "/" <<
                    UTILS_LIB_VERSION;

#ifdef INCLUDE_IGNITE_AUTO
    HCPLOG_C << "Lib Ver(Auto)~" << AUTO_LIB_VERSION;
#endif

    print_additional_library_versions(false);

    HCPLOG_C << "Firmware=" 
             << CIgniteClient::GetProductImpl()->GetAttribute(IProduct::eSWVersion)
             << ";" << "IMEI=" 
             << CIgniteClient::GetProductImpl()->GetAttribute(IProduct::eIMEI) 
             << ";" << "Serial=" 
             << CIgniteClient::GetProductImpl()->GetAttribute(IProduct::eSerialNumber)
             << ";" << "Time:" 
             << ic_utils::CIgniteDateTime::GetCurrentFormattedDateTime();

    std::string strSerialNumber = CIgniteClient::GetProductImpl()->
                                          GetAttribute(IProduct::eSerialNumber);
    std::string strVIN = CIgniteClient::GetProductImpl()->
                                                   GetAttribute(IProduct::eVIN);
    if (!g_strClientAppVersion.empty())
    {
        ic_utils::CIgniteLog::SetStatus("Application", g_strClientAppVersion);
    }
    ic_utils::CIgniteLog::SetStatus("libEvent",EVENT_LIB_VERSION);
    ic_utils::CIgniteLog::SetStatus("libClientBL",CLIENTBL_LIB_VERSION);
    ic_utils::CIgniteLog::SetStatus("libCore",CORE_LIB_VERSION);
    ic_utils::CIgniteLog::SetStatus("libNetwork",NETWORK_LIB_VERSION);
#ifdef INCLUDE_IGNITE_AUTO
    ic_utils::CIgniteLog::SetStatus("libAuto",AUTO_LIB_VERSION);
#endif
    ic_utils::CIgniteLog::SetStatus("libUtils",UTILS_LIB_VERSION);

    ic_utils::CIgniteLog::SetStatus("Firmware",
           CIgniteClient::GetProductImpl()->GetAttribute(IProduct::eSWVersion));
    ic_utils::CIgniteLog::SetStatus("IMEI",
                CIgniteClient::GetProductImpl()->GetAttribute(IProduct::eIMEI));
    ic_utils::CIgniteLog::SetStatus("SerialNumber",strSerialNumber);

    set_additional_lib_ver_status();

    /* Reduce priority below typical apps & services.  Do this before spawning
     * off new threads.
     */
    int nPrio = CIgniteConfig::GetInstance()->GetInt("ThreadPriority", -1);
    if (nPrio != -1)
    {
        ic_utils::CIgniteThread::SetCurrentThreadPriority(nPrio);
    }

    CDeviceDetails *pDeviceDetails = CDeviceDetails::GetInstance();
    pDeviceDetails->SetDeviceSpecificIdentifier(strVIN);
    pDeviceDetails->SetSerialNumber(strSerialNumber);
    
    /* If strKeystore is android, strDBEncryptSeed is random key generated
     * Sending serialNumber as strDBEncryptSeed from here in case of linux.
     * Init singleton, check keyStore not empty
     */
    std::string strKeystore = CIgniteConfig::GetInstance()->
                                             GetString("DAM.Database.keystore");

    if (strKeystore != KEYSTORE_ANDROID || strDBEncryptSeed.empty())
    {
        strDBEncryptSeed = strSerialNumber;
    }
    CAesSeed::GetInstance()->Init(strDBEncryptSeed, bSeedChanged);
    
    // Create main message queue and insert handlers for messages.
    HCPLOG_D << "Creating Message Processor...";
    g_pProcessor = new CMessageQueue(HCP_MSGQUEUE_NAME);

    HCPLOG_D << " - Registering Config Handler with MessageQueue...";
    g_pCfgHandler = new ic_bl::CConfigRequestHandler(g_pProcessor);

    ic_utils::Json::Value jsonHttpConfig = 
                     CIgniteConfig::GetInstance()->GetJsonValue("NET.HTTP.SSL");
    ic_utils::Json::FastWriter jsonFastWriter;
    std::string strHttpConfig = jsonFastWriter.write(jsonHttpConfig);
    g_pSslAttributes = new ic_network::CSSLAttributes(strHttpConfig);

    ic_network::CHttpSessionManager::GetInstance()->
                                             SetSSLAttributes(g_pSslAttributes);

    return 0;
}

int CIgniteClient::InitClient(std::string strConfigPath,
                              bool bOutputInfoAndExit,
                              bool bTriggerUploadAndExit)
{
    HCP_PRELOG_W("Initializing Client...");
    signal(SIGHUP,client_signal_handler);
    prctl(PR_SET_PDEATHSIG,SIGHUP);

    /* Initialize this early per documentation:
     * This function is not thread safe. You must not call it when any other 
     * thread in the program (i.e. a thread sharing the same memory) is running.
     * This doesn't just mean no other thread that is using libcurl. Because 
     * curl_global_init() calls functions of other libraries that are similarly
     * thread unsafe, it could conflict with any other thread that uses these
     * other libraries.
     */ 
    curl_global_init(CURL_GLOBAL_SSL );

    HCPLOG_C << "Init Config";
    CIgniteConfig::CreateSingleton(strConfigPath);

    /* if USER-DISASSOCIATION is set as TRUE in the previous cycle, clearing the
     * tables before reloading config and config used by other components.
     */
    if (NULL != CLocalConfig::GetInstance()) 
    {
        if (CLocalConfig::GetInstance()->Get(KEY_DISASSOCIATION) == "true")
        {
            HCPLOG_C << "Disassociation was requested. Clear db";
            CDataBaseFacade::GetInstance()->ClearTables();
            
            //cleanup the attachment files after restart
            CleanupAttachments();
        }
    }
    else 
    {
        HCPLOG_E << " LocalConfig is NULL";
    }

    CIgniteConfig::GetInstance()->ReloadConfigFromDB();
    CIgniteConfig::GetInstance()->ConfigureFileLogging();

    if (bOutputInfoAndExit)
    {
        std::cout << "Device ID = " << CLocalConfig::GetInstance()->Get("login")
                  << std::endl;
        if (!g_strClientAppVersion.empty())
        {
            std::cout << "Application version = " << g_strClientAppVersion
                      << std::endl;
        }
        std::cout << "libClientBL Version = "
                  << CLIENTBL_LIB_VERSION_TS << std::endl;
        std::cout << "libEvent Version = "
                  << EVENT_LIB_VERSION_TS << std::endl;
        std::cout << "libCore Version = "
                  << CORE_LIB_VERSION_TS << std::endl;
        std::cout << "libNetwork Version = "
                  << NETWORK_LIB_VERSION_TS << std::endl;
        std::cout << "libUtils Version = "
                  << UTILS_LIB_VERSION_TS << std::endl;

#ifdef INCLUDE_IGNITE_AUTO
        std::cout << "libAuto Version = "
                  << AUTO_LIB_VERSION_TS << std::endl;
#endif

        print_additional_library_versions(true);
           
        std::cout << "Attributes = "
                  << CLocalConfig::GetInstance()->Get("decodedFields") 
                  << std::endl;
        quick_exit(0);
    }


    return 0;
}

int CIgniteClient::StartAnalytics()
{
    if (g_pProcessor == NULL)
    {
        return -1;
    }
    g_pProcessor->Start();

    bool bDataAggregationStatus = true;

#ifndef DIRECT_UPLOAD
    HCPLOG_D << " - Creating Insight Engine...";
    g_pEngineInstance = new ic_bl::CInsightEngine(g_pProcessor, 
            bDataAggregationStatus);

    HCPLOG_D << " Init CUploadController";
    g_pUploadController = ic_bl::CUploadController::GetInstance();
    if (g_pUploadController)
    {
        g_pUploadController->StartUploadController();
    }
    else
    {
        HCPLOG_E << " - CUploadController is NULL!";
    }

#endif

    if (bDataAggregationStatus)
    {
        SendClientLaunchedEvent();
    }
    m_nClientStatus = eRUNNING;

    PersistAndBroadcastICStatus(eSTARTED);
    return 0;
}

void CIgniteClient::SuspendAnalytics()
{
    if (eRUNNING == m_nClientStatus)
    {
        if (g_pEngineInstance)
        {
            HCPLOG_C << " - suspending Insight Engine...";
            g_pEngineInstance->SuspendInsightEngine();
        }

        if (g_pUploadController)
        {
            HCPLOG_C << " - suspending Uploaders...";
            g_pUploadController->SuspendUploadController();
        }

        m_nClientStatus = eSUSPENDED;
    }
}

void CIgniteClient::ResumeAnalytics()
{
    if (eSUSPENDED == m_nClientStatus)
    {
        if (g_pEngineInstance)
        {
            HCPLOG_C << " - resuming Insight Engine...";
            g_pEngineInstance->ResumeInsightEngine();
        }

        if (g_pUploadController) 
        {
            HCPLOG_C << " - resuming Uploaders...";
            g_pUploadController->ResumeUploadController();
        }

        m_nClientStatus = eRUNNING;
    }
}

void CIgniteClient::StopAnalytics()
{
    HCPLOG_METHOD();

    //just to make sure
    SuspendAnalytics();

    ic_event::CIgniteEvent sessionEndEvnt("1.0", "SessionStatus");
    sessionEndEvnt.AddField("status", "shutdown");
    sessionEndEvnt.AddField("reason", "normalShutdown");

    std::string strEvnt;
    sessionEndEvnt.EventToJson(strEvnt);
    ic_bl::CDBTransport::InsertEvent(strEvnt);
    
    if (NULL != g_pEngineInstance)
    {
        delete g_pEngineInstance;
        g_pEngineInstance = NULL;
    }
    HCPLOG_T << "engineInstance closed...";

    if (NULL != g_pUploadController)
    {
        g_pUploadController->StopUploadController();
        HCPLOG_T << "UploadController is stopped.";
        g_pUploadController->ReleaseInstance();
        HCPLOG_T << "UploadController is closed.";
    }

    if (NULL != g_pCfgHandler)
    {
        delete g_pCfgHandler;
    }
    HCPLOG_T << "ConfigHandler is closed.";

    ic_network::CHttpSessionManager::ReleaseInstance();
    HCPLOG_T << "HttpSessionManager is released.";

    if (NULL != g_pSslAttributes)
    {
        delete g_pSslAttributes;
    }
    HCPLOG_T << "SslAttributes object is closed.";

    if (NULL != g_pProcessor)
    {
        delete g_pProcessor;
    }
    HCPLOG_D << "MessageQueue is closed.";

    CDataBaseFacade::GetInstance()->CloseConnection();
    HCPLOG_C << "DB Connection is closed.";
}

void CIgniteClient::RestartAnalytics(int nRestartCode, bool bClearDB)
{
    HCPLOG_METHOD();
    PersistAndBroadcastICStatus(eEXITING);
    if (bClearDB)
    {
        HCPLOG_C << "Delete db file";
        CDataBaseFacade::GetInstance()->RemoveDatabase();

        //cleanup the attachment files
        CleanupAttachments();
    }
    else
    {
        HCPLOG_C << "Close db Connection";
        CDataBaseFacade::GetInstance()->CloseConnection();
    }

    HCPLOG_C << "Restart analytics";
    /* Calling close log File in order to print Closing log File before
     * stopping the IC
     */
    ic_utils::CIgniteLog::CloseLogFile();
    
#ifdef __ANDROID__
    abort();
#else
    quick_exit(nRestartCode);
#endif
}

void CIgniteClient::DisconnectMQTT()
{
    ic_bl::CIgniteMQTTClient::GetInstance()->DisconnectConn();
}

void CIgniteClient::PrepareForShutdown(int nGraceTime, bool bExitOnComplete,
                                       IC_EXIT_TYPE eExitType)
{
    HCPLOG_METHOD() << "GraceTime..." << nGraceTime;

    //just to avoid handling multiple shutdown requests
    if (g_bIsShutdownStarted)
    {
        HCPLOG_E << "Already shutdown is in progress...";
        return;
    }

    g_bIsShutdownStarted = true;
    //need to start the CClientOnOff thread
    ((ic_bl::CClientOnOff*)GetOnOffMonitor())->Start();

    if (bExitOnComplete)
    {
        HCPLOG_D << "ShutdownCompleteMonitor is started... exitType is "
                 << eExitType;
        int *pnExitTypePtr = (int *)malloc(sizeof(int));
        if (pnExitTypePtr != NULL)
        {
            *pnExitTypePtr = eExitType;
            pthread_create(&g_shutdownCompleteThread, NULL,
                           &shutdown_complete_monitor, pnExitTypePtr);
        }
        else
        {
            HCPLOG_E << " Memory allocation failed , starting shutdown_complete_monitor with default exit ";
            pthread_create(&g_shutdownCompleteThread, NULL,
                           &shutdown_complete_monitor, NULL);
        }
    }

    //send ACK notification to device
    ic_utils::Json::Value jsonAck = ic_utils::Json::Value::nullRef;
    jsonAck["state"] = ic_bl::ShutdownNotifAckEnum::eSHUTDOWN_INTIATED;
    GetClientMessageDispatcher()->DeliverShutdownNotifAckToDevice(jsonAck);
}

void CIgniteClient::ShutdownInitiated()
{
    HCPLOG_METHOD();
    //device initiated the shutdown; Client may not get any more time to do any additional shutdown process
}

void CIgniteClient::CompleteShutdown()
{
    HCPLOG_METHOD();

    StopNotifications();
    StopAnalytics();

    //send ACK notification to device
    ic_utils::Json::Value jsonAck = ic_utils::Json::Value::nullRef;
    jsonAck["state"] = ic_bl::ShutdownNotifAckEnum::eREADY_FOR_SHUTDOWN;
    GetClientMessageDispatcher()->DeliverShutdownNotifAckToDevice(jsonAck);
    
    g_bIsShutdownCompleted = true;
    HCPLOG_C << "Shutdown preparation completed.waiting to be terminated...";
}

void CIgniteClient::SendClientLaunchedEvent()
{
    if (ic_bl::CCacheTransport::GetInstance() 
        && (!ic_bl::CCacheTransport::GetInstance()->IsSessionInProgress()))
    {
        ic_event::CIgniteEvent clientLaunchedEvent("1.0", "IgniteClientLaunched");
        clientLaunchedEvent.AddField("state","starting");
        clientLaunchedEvent.Send();

        HCPLOG_D << "IgniteClientLaunched event is sent!";
    }
    else
    {
        HCPLOG_D << "Session already in progress!";
    }
}

void CIgniteClient::CleanupAttachments()
{
    HCPLOG_C << "Deleting attachment files...";
    std::string strAttachPath = 
                       CIgniteConfig::GetInstance()->GetString(ic_bl::KEY_ATTACH_PATH);
    if (!strAttachPath.empty())
    {
        bool bRetVal = false;

        //instead of deleting file by file, just removing the directory
        bRetVal = ic_utils::CIgniteFileUtils::RemoveDirectory(strAttachPath);
        HCPLOG_C << "rmdir status:"<< bRetVal;

        if (bRetVal)
        {
            //recreate the directory
            bRetVal = ic_utils::CIgniteFileUtils::MakeDirectory(strAttachPath);
            HCPLOG_C << "MakeDirectory status:"<< bRetVal;
        }
    }
}

int CIgniteClient::StartNotifications()
{

    HCPLOG_D << " - Creating NotificationListener...";
    ic_bl::CNotificationListener *pNotifListner = 
                            ic_bl::CNotificationListener::GetNotificationListener();
    pNotifListner->Start();

    CUploadMode *pMode = CUploadMode::GetInstance();
    if (pMode->IsStreamModeSupported())
    {
        if ((CIgniteConfig::GetInstance()->GetJsonValue("MQTT")).isObject())
        {
            HCPLOG_I << " - Creating CMQTTNotificationHandler...";
            g_pMqNotifReq = new ic_bl::CMQTTNotificationHandler(pNotifListner);
        }
    }

    return 0;
}

int CIgniteClient::StopNotifications()
{
    if (NULL != g_pMqNotifReq)
    {
        delete g_pMqNotifReq;
        g_pMqNotifReq = NULL;
    }

    return 0;
}

IClientConnector* CIgniteClient::AddAnalyticsHandler(IMessageHandler *pHandler)
{
    HCPLOG_METHOD();

    ic_bl::CMessageController::AddHandler(pHandler);
    return (ic_bl::CClientConnectorImpl::GetInstance());
}

IOnOff* CIgniteClient::GetOnOffMonitor()
{
    return ic_bl::CClientOnOff::GetInstance();
}

ic_utils::Json::Value CIgniteClient::QueryParameter(const enum IC_QUERY_ID eId)
{
    HCPLOG_METHOD();
    return ic_bl::CPersistancyAndStateHandler::GetInstance()->GetICParam(eId);
}

void CIgniteClient::PersistAndBroadcastICStatus(enum IC_STATE eState)
{
    HCPLOG_METHOD();
    ic_bl::CPersistancyAndStateHandler::GetInstance()->PersistICstate(eState);
    ic_utils::Json::Value valueJson = 
                ic_bl::CPersistancyAndStateHandler::GetInstance()->GetICParam(
                IC_QUERY_ID::eIC_CURRENT_STATE);
    GetClientMessageDispatcher()->DeliverICStatusToDevice(valueJson);
}

void CIgniteClient::SetExternalHttpSessionHandler(IExternalHttpSessionHandler 
                                                  *pHandler)
{
   if (nullptr != pHandler)
   {
       m_pExtSessionHandler = pHandler;
   }
}
} /* namespace ic_core */
