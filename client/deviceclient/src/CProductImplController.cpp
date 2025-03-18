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

#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <cstring>
#include "CProductImplController.h"
#include "CIgniteLog.h"
#include "CIgniteClient.h"
#include "CIgniteConfig.h"
#include "CIgniteEvent.h"
#include "CIgniteFileUtils.h"
#include "CIgniteLog.h"
#include "CCpuLoad.h"
#include "CClientMessageDispatcherImpl.h"
#include "CPreIgniteLogger.h"
#include "CDefaultProductImpl.h"
#include "DeviceLibVersion.h"

#if IC_UNIT_TEST == 1
#include <auth/CActivationState.h>
#include "gtest/gtest.h"

int g_nTestReturn = 0;
#endif

using std::string;

//! macro for log tag
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CProductImplController"

namespace ic_app
{

namespace
{
//! configuration file path
std::string g_strConfigPath = "";

//! Log level
int g_nLogLevel = -1;

//! control flag if Client is started just to print it's details
bool g_bOutputInfo = false;

//! reason why the Client is started
std::string g_strStartupReason = "";

//! Client restart count
int g_nRestartCount = 0;

#if defined(IC_UNIT_TEST)
static const std::string IGNITION_EV_ID = "IgnStatus";
static const std::string IGNITION_EV_VER = "1.0";
static const std::string FIELD_STATE = "state";
static const std::string STATE_RUN = "run";
#endif

/**
 * Method to print the Client application usage formats.
 * @param none
 * @return void
 */
void print_usage()
{
    std::cerr << "Usage: deviceclient [-c <config file path>]" << std::endl;
    std::cerr << "\nOptional arguments:" << std::endl;
    std::cerr << "-re <restart reason> <restart count> " <<
            "- to set the client restart reason and restart count" << std::endl;
    std::cerr << "-d <debug level {1..7}> - to set debug level" << std::endl;
    std::cerr << "-i - to print client details and then exit\n" << std::endl;
}

/**
 * Method to handle the config file command-line argument.
 * @param[in] rnIter current iterator position of the command-line arguments
 * @param[in] rnArgC count of command-line arguments
 * @param[in] pchArgV array of command-line arguments
 * @return true if config file is successfully read, false otherwise
 */
bool handle_config_file_option(int &rnIter, const int &rnArgC, char *pchArgV[])
{
    bool bResult = false;

    //increment to read the config file path
    rnIter++;

    if (rnIter < rnArgC)
    {
        char *pchConfigFilePath = pchArgV[rnIter];

        // Make sure this isn't another switch
        if (pchConfigFilePath[0] != '-')
        {
            std::ifstream file(pchConfigFilePath, std::ifstream::binary);
            if (!file.good())
            {
                std::cerr << "Config file " << pchConfigFilePath 
                          << " not valid!" << std::endl;
            }
            else
            {
                g_strConfigPath = pchConfigFilePath;
                HCPLOG_T << "Config path=" << g_strConfigPath;

                bResult = true;
            }
        }
    }
    else
    {
        std::cerr << "No config file found!" << std::endl;
    }
    return bResult;
}

/**
 * Method to handle the logging level command-line argument.
 * @param[in] rnIter current iterator position of the command-line arguments
 * @param[in] rnArgC count of command-line arguments
 * @param[in] pchArgV array of command-line arguments
 * @return true if logging level is successfully read, false otherwise
 */
bool handle_debug_option(int &rnIter, const int &rnArgC, char *pchArgV[])
{
    bool bResult = false;

    //increment to read the debug level
    rnIter++;

    if (rnIter < rnArgC)
    {
        const char *pchDebugValue = pchArgV[rnIter];

        // Make sure this isn't another switch
        if (pchDebugValue[0] != '-')
        {
            const int iLevel = atoi(pchDebugValue);
            if ((iLevel >= 0) && (iLevel <= HCPLOG_MAX_LEVEL))
            {
                g_nLogLevel = iLevel;
                bResult = true;
            }
        }
    }
    else
    {
        std::cerr << "Debug level is not found!" << std::endl;
    }
    return bResult;
}

/**
 * Method to handle the command-line arguments for client restart details.
 * @param[in] rnIter current iterator position of the command-line arguments
 * @param[in] rnArgC count of command-line arguments
 * @param[in] pchArgV array of command-line arguments
 * @return true if restart details are read successfully, false otherwise
 */
bool handle_restart_count_option(int &rnIter, const int &rnArgC, char *pchArgV[])
{
    bool bResult = false;

    if ((rnIter+2) < rnArgC)
    {
        const char *pchRestartReason = pchArgV[rnIter+1];
        const char *pchRestartCntValue = pchArgV[rnIter+2];

        if ((pchRestartReason[0] != '-') && (pchRestartCntValue[0] != '-'))
        {
            g_strStartupReason = pchRestartReason;
            g_nRestartCount = atoi(pchRestartCntValue);
            bResult = true;
        }
    }
    else
    {
        std::cerr << "Restart reason and count are not found!" << std::endl;
    }

    //have the iterator incremented for next processing
    rnIter+=2;

    return bResult;
}

/**
 * Method to parse the command-line arguments
 * @param[in] nArgC count of command-line arguments
 * @param[in] pchArgV array of command-line arguments
 * @return true if all the given arguments are parsed successfully and valid,
 *         false otherwise.
 */
bool parse_args(int nArgC, char *pchArgV[])
{
    bool bParseResult = false;
    bool bIsConfigValid = false;

    int nIter = 1;
    while (nIter < nArgC)
    {
        if (0 == strcmp(pchArgV[nIter], "-c"))
        {
            bIsConfigValid = bParseResult = handle_config_file_option(nIter,
                                                                nArgC, pchArgV);
        }
        else if (0 == strcmp(pchArgV[nIter], "-d"))
        {
            bParseResult = handle_debug_option(nIter, nArgC, pchArgV);
        }
        else if (0 == strcmp(pchArgV[nIter], "-re"))
        {
            bParseResult = handle_restart_count_option(nIter, nArgC, pchArgV);
        }
        else if (0 == strcmp(pchArgV[nIter], "-i"))
        {
            bParseResult = g_bOutputInfo = true;
        }
        else
        {
            HCPLOG_T << "Error on argument: " << pchArgV[nIter];
            bParseResult = false;
            break;
        }

        nIter++;
    }

    if ((!bIsConfigValid) || (!bParseResult))
    {
        bParseResult = false;
    }

    return bParseResult;
}

} //unnamed namespace

CProductImplController *CProductImplController::GetInstance()
{
    static CProductImplController piCntrlr;
    return &piCntrlr;
}

void CProductImplController::ReleaseInstance()
{
    HCPLOG_METHOD();
}

CProductImplController::CProductImplController() : m_bIsShutdownNotified(false),
                                                 m_pProd(nullptr),
                                                 m_pMsgDispatcher(nullptr),
                                                 m_pClientConnector(nullptr),
                                                 m_pDeviceCommandObject(nullptr)
{
}

CProductImplController::~CProductImplController()
{
    ReleaseProdImplResoruces();
}

void CProductImplController::Run()
{
    HCPLOG_METHOD();
    ic_core::CIgniteClient::GetOnOffMonitor()->RegisterForShutdownNotification(
                             this, ic_core::IOnOff::eR_PRODUCT_IMPL_CONTROLLER);
    while (!m_bIsShutdownNotified)
    {
        Wait(1);
    }
    ic_core::CIgniteClient::GetOnOffMonitor()->ReadyForShutdown(
                                   ic_core::IOnOff::eR_PRODUCT_IMPL_CONTROLLER);
    ic_core::CIgniteClient::GetOnOffMonitor()->UnregisterForShutdownNotification
                                  (ic_core::IOnOff::eR_PRODUCT_IMPL_CONTROLLER);
    Detach();
}

void CProductImplController::NotifyShutdown()
{
    m_bIsShutdownNotified = true;
    Notify();
}

void CProductImplController::ReleaseProdImplResoruces()
{
    if (nullptr != m_pProd)
    {
        HCPLOG_I << "Deleting CProduct object...";
        delete m_pProd;
    }
    else
    {
        HCPLOG_I << "CProduct object is not valid.";
    }

    if (nullptr != m_pMsgDispatcher)
    {
        HCPLOG_I << "Deleting IClientMessageDispatcher object...";
        delete m_pMsgDispatcher;
    }
    else
    {
        HCPLOG_I << "IClientMessageDispatcher object is not valid.";
    }

    if (nullptr != m_pDeviceCommandObject)
    {
        m_pDeviceCommandObject->StopListening();
        HCPLOG_I << "Deleting DeviceCommandObject...";
        delete m_pDeviceCommandObject;
    }
    else
    {
        HCPLOG_I << "DeviceCommandObject is not valid.";
    }

    ic_core::CIgniteConfig::ClearInstance();
}

void CProductImplController::StartController(int &rnArgC, char *pchArgV[])
{
    if (!parse_args(rnArgC, pchArgV))
    {
        print_usage();

        exit(-2);
    }

    if (-1 != g_nLogLevel)
    {
        ic_utils::CIgniteLog::SetReportingLevel(
                                  static_cast<ic_utils::LogLevel>(g_nLogLevel));
        HCPLOG_T << "Setting Log Level =" << g_nLogLevel;
    }

#if defined(DEVICE_CLIENT_APP_VERSION)
    ic_core::CIgniteClient::SetClientApplicationVersion(DEVICE_CLIENT_APP_VERSION);
#endif

    /* libDevice is a custom library supporting device level methods/APIs.
     * This library version need to be printed on console/log file .So the value
     * will be set using ic_core::CIgniteClient::SetAdditionalLibraryVersions 
     * method.
     */
    ic_core::LibVersionDetails stLibDeviceDetails;
    stLibDeviceDetails.m_strLibName = "libDevice";
    stLibDeviceDetails.m_strLibVersion = DEVICE_LIB_VERSION_TS;
    std::vector<ic_core::LibVersionDetails> vectAdditionalLibs;
    vectAdditionalLibs.push_back(stLibDeviceDetails);

    ic_core::CIgniteClient::SetAdditionalLibraryVersions(vectAdditionalLibs);

    // Initialize the client
    ic_core::CIgniteClient::InitClient(g_strConfigPath, g_bOutputInfo, false);

    HCPLOG_I << " - Registering Product Impl...";
    m_pProd = new ic_device::CDefaultProductImpl();

    /* first four parameters are initialized with empty string values.
     * based on deployed device, additional logics can be added here
     * to read them from the actual device and set accordingly.
     */
    m_pProd->StartupInit("","","","","", g_strStartupReason, g_nRestartCount);

    ic_core::CIgniteClient::SetProductImpl(m_pProd);

    HCPLOG_I << " - Initializing analytics client";
    ic_core::CIgniteClient::InitAnalytics("IvRandomString", false);

    m_pMsgDispatcher = new ic_device::CClientMessageDispatcherImpl();
    ic_core::CIgniteClient::SetClientMessageDispatcher(m_pMsgDispatcher);

    HCPLOG_I << " - Detecting and setting Bench Mode to differentiate real cars and test HUs.";
    std::string benchModeFilePath = ic_core::CIgniteConfig::GetInstance()->
                                               GetAsString("BenchModeFilePath");

    if (ic_utils::CIgniteFileUtils::Exists(benchModeFilePath))
    {
        HCPLOG_I << " - BENCH MODE detected.";
        ic_event::CIgniteEvent::bBenchMode = true;
    }
    else
    {
        HCPLOG_I << " - VEHICLE MODE detected.";
        ic_event::CIgniteEvent::bBenchMode = false;
    }

#ifdef ENABLE_ZMQ
    HCPLOG_I << " - Start listening to ZMQ Pull Channel...";
    m_pDeviceCommandObject = new ic_device::CDeviceCommandHandlerImpl();
    if (m_pDeviceCommandObject != nullptr)
    {
        m_pDeviceCommandObject->StartListening();
    }
    else
    {
        HCPLOG_E << "Could not start ZMQ listener";
    }
#endif
    HCPLOG_I << " - StartAnalytics analytics client";
    ic_core::CIgniteClient::StartAnalytics();

    HCPLOG_I << " - Starting notification thread";
    ic_core::CIgniteClient::StartNotifications();

#if defined(IC_UNIT_TEST)
    // Send ignition on event to start analytics session
    ic_event::CIgniteEvent ignEvent(IGNITION_EV_VER, IGNITION_EV_ID);
    ignEvent.AddField(FIELD_STATE, STATE_RUN);
    ignEvent.Send();
    HCPLOG_C << "Send ignition on event to start analytics session";
#endif

#if IC_UNIT_TEST == 1
    ic_bl::CActivationState::GetInstance();
    ::testing::InitGoogleTest(&rnArgC, pchArgV);
    HCPLOG_I << " - Starting Unit Tests.";
    /* Sleep is added to ensure MQTT connection is SUCCESS before Test gets
     * executed.
     */
    sleep(10);

    g_nTestReturn = RUN_ALL_TESTS();
    HCPLOG_I << "UnitTest:AcpClient exiting normally";

    /* call shutdown-simulate API; if DeviceCommandObject is null,
     * call shutdown API directly
     */
    if(nullptr != m_pDeviceCommandObject)
    {
        m_pDeviceCommandObject->SimulateShutdownNotif();
    }
    else
    {
        ic_core::CIgniteClient::PrepareForShutdown(30, true,
                                           ic_core::IC_EXIT_TYPE::eNORMAL_EXIT);
    }
#endif

    // start the controller thread
    Start();
}

void CProductImplController::Wait(unsigned int timeInMs)
{
    std::unique_lock<std::mutex> lk(m_WaitMutex);
    std::chrono::milliseconds ms;
    m_WaitCondition.wait_for(lk, std::chrono::milliseconds(timeInMs * 1000));
}

void CProductImplController::Notify()
{
    m_WaitCondition.notify_all();
}

ic_core::IClientConnector *CProductImplController::GetClientConnector()
{
    return m_pClientConnector;
}


#ifdef IC_UNIT_TEST
bool CProductImplController::
           HandleConfilefileArg(int &rnIter, const int &rnArgC, char *pchArgV[])
{
    if (handle_config_file_option(rnIter, rnArgC, pchArgV))
    {
        return true;
    }
    else
    {
        print_usage();
        return false;
    }
}

bool CProductImplController::
           HandleDebugLevelArg(int &rnIter, const int &rnArgC, char *pchArgV[])
{
    if (handle_debug_option(rnIter, rnArgC, pchArgV))
    {
        return true;
    }
    else
    {
        print_usage();
        return false;
    }
}

bool CProductImplController::
           HandleRestartCountArg(int &rnIter, const int &rnArgC, char *pchArgV[])
{
    if (handle_restart_count_option(rnIter, rnArgC, pchArgV))
    {
        return true;
    }
    else
    {
        print_usage();
        return false;
    }
}
#endif
} // namespace ic_app
