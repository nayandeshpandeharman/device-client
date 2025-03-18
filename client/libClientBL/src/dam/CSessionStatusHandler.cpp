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
#include "CSessionStatusHandler.h"
#include "CIgniteDateTime.h"
#include "CIgniteLog.h"
#include "CIgniteClient.h"
#include "upload/CUploadUtils.h"
#include "CIgniteConfig.h"
#include "CIgniteStringUtils.h"
#include "CMessageController.h"
#include "config/CUploadMode.h"

//! Macro for CSessionStatusHandler string
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CSessionStatusHandler"

namespace ic_bl 
{
namespace {

    //! Constant key for integer 60000
    static const int MAX_WAIT_SINCE_BOOT_MILI_SECS = 60000;
    
    //! Constant key for string "IgnStatus"
    static const std::string IGNITION_EV_ID = "IgnStatus";

    //! Constant key for string "SessionStatus"
    static const std::string SESSION_EV_ID = "SessionStatus";
    
    //! Constant key for string "EngineRPM"
    static const std::string ERPM_EV_ID = "EngineRPM";

    //! Constant key for string "RPM"
    static const std::string RPM_EV_ID = "RPM";

    //! Constant key for string "state"
    static const std::string FIELD_STATE = "state";

    //! Constant key for string "run"
    static const std::string STATE_RUN = "run";

    //! Constant key for string "off"
    static const std::string STATE_OFF = "off";

    //! Constant key for string "IgniteClientLaunched"
    static const std::string IGNITECLIENTLAUNCHED_EV_ID = "IgniteClientLaunched";

    //! Constant key for string "starting"
    static const std::string STATE_STARTING = "starting";

    //! Constant key for string "shuttingdown"
    static const std::string STATE_SHUTTINGDOWN = "shuttingdown";
    
    //! Constant key for string "DAM.UseRpmForIgnition"
    static const std::string RPM_FOR_IGN_KEY_ID = "DAM.UseRpmForIgnition";

    //! Constant key for string "IgnON"
    static const std::string RPM_IGNON_KEY_ID = "IgnON";

    //! Constant key for string "IgnOFF"
    static const std::string RPM_IGNOFF_KEY_ID = "IgnOFF";

    //! Constant key for string "startupType"
    static const std::string STARTUP_TYPE = "startupType";

    //! Constant key for string "external"
    static const std::string STARTUP_EXTERNAL = "external";

    //! Constant key for string "self"
    static const std::string STARTUP_SELF = "self";

    //! Constant key for string "detached"
    static const std::string VALUE_DETACHED = "detached";
    
    //! Constant key for string "Location"
    static const std::string LOCATION_EV_ID = "Location";
}

bool CSessionStatusHandler::m_bSessionInProgress = false;

CSessionStatusHandler::CSessionStatusHandler(CTransportHandlerBase *pNextHandler)
    : CTransportHandlerBase(pNextHandler), m_pSetExceptionEvents(NULL), m_bGenIgnON(false), m_bGenIgnOFF(false)
{
    CSessionStatusHandler::m_bSessionInProgress = false;

    if(ic_core::CIgniteConfig::GetInstance()->
        GetJsonValue(RPM_FOR_IGN_KEY_ID).isObject() && 
        !(ic_core::CIgniteConfig::GetInstance()->
        GetJsonValue(RPM_FOR_IGN_KEY_ID).empty()))
    {
        ic_utils::Json::Value jsonUseRpmForIgn = ic_core::CIgniteConfig::GetInstance()->GetJsonValue(RPM_FOR_IGN_KEY_ID);
        if (jsonUseRpmForIgn.isMember(RPM_IGNON_KEY_ID) && jsonUseRpmForIgn[RPM_IGNON_KEY_ID].isBool())
        {
            m_bGenIgnON = jsonUseRpmForIgn[RPM_IGNON_KEY_ID].asBool();
        }
        if (jsonUseRpmForIgn.isMember(RPM_IGNOFF_KEY_ID) && jsonUseRpmForIgn[RPM_IGNOFF_KEY_ID].isBool())
        {
            m_bGenIgnOFF= jsonUseRpmForIgn[RPM_IGNOFF_KEY_ID].asBool();
        }
    }
    else if(ic_core::CIgniteConfig::GetInstance()->GetJsonValue(RPM_FOR_IGN_KEY_ID).isBool())
    {
        m_bGenIgnON = m_bGenIgnOFF = ic_core::CIgniteConfig::GetInstance()->GetBool(RPM_FOR_IGN_KEY_ID);
    }
    else {
         //do nothing
    }

    HCPLOG_D << "RPM used for IgnOn:" << m_bGenIgnON <<",IgnOff:"<<m_bGenIgnOFF;

    m_eIgnState = eIGNITION_UNKNOWN;
    m_eLastIgnStatus = eIGNITION_UNKNOWN;

    InitSessionExceptionEvent();
};


void CSessionStatusHandler::InitSessionExceptionEvent()
{
    ic_utils::Json::Value jsonEventArray = ic_core::CIgniteConfig::GetInstance()
                              ->GetJsonValue("DAM.SessionStatusExceptionEvent");

    if (jsonEventArray.isArray())
    {
        m_pSetExceptionEvents = new std::set<std::string>();
        HCPLOG_W << "Created exception list";

        for (int nIter = 0; nIter < jsonEventArray.size() ; nIter++)
        {
            m_pSetExceptionEvents->insert(jsonEventArray[nIter].asString());
            HCPLOG_W << jsonEventArray[nIter].asString();
        }
    }
}

int CSessionStatusHandler::HandleRPMEvent(ic_core::CEventWrapper* event)
{
    int nRetValue = -1;
    int nRpmValue = GetRpmValue(event);    
    bool bRaiseEvent = false;
    std::string strState = "";

    if (0 < nRpmValue) 
    {
        if (eIGNITION_ON != m_eIgnState) 
        {
            // set the ignition state as on and raise the ignition state event
            m_eIgnState = eIGNITION_ON;
            strState = STATE_RUN;
            bRaiseEvent = true;
        }
        // If ignition status is already on, no need to do anything
        
    } 
    else 
    {
        // Here means ignition is 0 or maybe < 0. This means ignition status is OFF
        if (eIGNITION_OFF != m_eIgnState) 
        {
            m_eIgnState = eIGNITION_OFF;
            strState = STATE_OFF;
            bRaiseEvent = true;
        }
    }
    
    if (bRaiseEvent) 
    {
        nRetValue = SendIgnStatusEvent(strState);
    }
    return nRetValue;
}

int CSessionStatusHandler::GetRpmValue(ic_core::CEventWrapper *pEvent)
{
    int nRpmValue = 0;

    // if the event is RPM
    if (RPM_EV_ID == pEvent->GetEventId())
    {
        nRpmValue = pEvent->GetInt("value");
    }

    return nRpmValue;
}

int CSessionStatusHandler::SendIgnStatusEvent(const std::string &rstrState)
{
    int nRetValue = -1;
    if ((m_bGenIgnON && (STATE_RUN == rstrState)) 
       || (m_bGenIgnOFF && (STATE_OFF == rstrState)))
    {
        ic_event::CIgniteEvent ignEvent("1.0", IGNITION_EV_ID);
        ignEvent.AddField("state", rstrState);            
        nRetValue = ignEvent.Send();
        HCPLOG_W << "Raising Ignition status with state :  " << rstrState;
    }
    else 
    {
        //do nothing
    }
    
    return nRetValue;
}

bool CSessionStatusHandler::IsExceptionEvent(const std::string& rstrEventID)
{
    if(m_pSetExceptionEvents) {
        return (m_pSetExceptionEvents->find(rstrEventID) != m_pSetExceptionEvents->end());
    }
    return true;
}

void CSessionStatusHandler::HandleEvent(ic_core::CEventWrapper *pEvent)
{
    std::string strEventID = pEvent->GetEventId();
    HCPLOG_METHOD() << strEventID;

    std::string strStartupType = ic_core::CIgniteConfig::GetInstance()->
                                          GetString(STARTUP_TYPE, STARTUP_SELF);

    if (IsRpmEvent(strStartupType, strEventID))
    {
        // If RPM is used for ignition status.
        if (IsRpmUsedForIgnStatus())
        {
            HandleRPMEvent(pEvent);
        }
    }

    if (IsIgniteClientLaunchedEvent(strStartupType, strEventID))
    {
        std::string strState = pEvent->GetString(FIELD_STATE);
        HandleIgniteClientLaunchedEvent(strState);
    }
    else if (IsIgnStatusEvent(strStartupType, strEventID)) 
    {
        std::string strState = pEvent->GetString(FIELD_STATE);
        if (IsRepeatedIgnStatusEvnt(strState))
        {
            HCPLOG_W << "Ignore: repeated IGN state:" << strState;
            delete pEvent;
            return;
        }

        HandleIgnStatusEvent(strState);
    }
    else if (SESSION_EV_ID == strEventID)
    {
        std::string strStatus = pEvent->GetString("status");
        bool bInProgress = ("startup" == strStatus);

        if (IsRepeatedSessionStatusEvnt(bInProgress))
        {
            HCPLOG_D << "Ignore: repeated session status=" << strStatus;
            delete pEvent;
            return;
        }
        CSessionStatusHandler::m_bSessionInProgress = bInProgress;
    }
    else if (!CSessionStatusHandler::m_bSessionInProgress && !IsAlert(pEvent) 
             && !IsExceptionEvent(strEventID))
    {
        /* Ignoring all out of session events unless it's an alert or location. 
         * Location is required for tow.
         */
        HCPLOG_W <<"Ignoring event:" <<strEventID;
        delete pEvent;
        return;
    }
    else
    {
        // Do nothing
    }

    #ifdef IC_UNIT_TEST
        //This check is required because in case of UT, m_pNextHandler is nullptr
        if(nullptr == m_pNextHandler)
        {
            return;
        }
    #endif
    m_pNextHandler->HandleEvent(pEvent);
}

bool CSessionStatusHandler::IsRpmEvent(const std::string &rstrStartupType,
                                      const std::string &rstrEventID)
{
    bool bStatus = false;

    /* if client startup type is SELF and event is either RPM or ENGINERPM,
     * consider it as a RPM event
     */
    if ((STARTUP_SELF == rstrStartupType) 
        && ((ERPM_EV_ID == rstrEventID) || (RPM_EV_ID == rstrEventID)))
    {
        bStatus = true;
    }
    return bStatus;
}

bool CSessionStatusHandler::IsRpmUsedForIgnStatus()
{
    bool bStatus = false;

    // check if RPM is configured to generate IgnOn/IgnOff events
    if (m_bGenIgnON || m_bGenIgnOFF)
    {
        bStatus = true;
    }
    return bStatus;
}

bool CSessionStatusHandler::IsIgniteClientLaunchedEvent(
                                             const std::string &rstrStartupType,
                                             const std::string &rstrEventID)
{
    bool bStatus = false;

    /* if client startup type is EXTERNAL and event is IgniteClientLaunched,
     * consider it as a IgniteClientLaunched event
     */
    if ((STARTUP_EXTERNAL == rstrStartupType) 
        && (IGNITECLIENTLAUNCHED_EV_ID == rstrEventID))
    {
        bStatus = true;
    }
    return bStatus;
}

void CSessionStatusHandler::HandleIgniteClientLaunchedEvent(const std::string 
                                                           &rstrState)
{
    HCPLOG_T << "State:" << rstrState;
    if (STATE_STARTING == rstrState)
    {
        StartSession();
    }
    else if (STATE_SHUTTINGDOWN == rstrState)
    {
        EndSession();
    }
    else
    {
        //do nothing
        HCPLOG_D << "Unknown HCPCLIENTSTATUS state received: " << rstrState;
    }
}

bool CSessionStatusHandler::IsIgnStatusEvent(const std::string &rstrStartupType,
                                            const std::string &rstrEventID)
{
    bool bStatus = false;

    /* if client startup type is SELF and event is IgnStatus, consider it as a 
     * IgnStatus event
     */
    if ((STARTUP_SELF == rstrStartupType) && (IGNITION_EV_ID == rstrEventID))
    {
        bStatus = true;
    }
    return bStatus;
}

bool CSessionStatusHandler::IsRepeatedIgnStatusEvnt(const std::string &rstrState)
{
    bool bStatus = false;
    if (!rstrState.empty()) 
    {
        // get the enum value w.r.t ign state
        IgnitionState eIgnStatus = (STATE_RUN == rstrState) ? eIGNITION_ON 
                                    : eIGNITION_OFF;

        // check if the previous state and current state is same
        if (m_eLastIgnStatus == eIgnStatus) 
        {
            bStatus = true;
        }
    }
    return bStatus;
}

void CSessionStatusHandler::HandleIgnStatusEvent(const std::string &rstrState)
{
    if (!rstrState.empty()) 
    {
        // set variable based on received ign state
        m_eLastIgnStatus = (STATE_RUN == rstrState) ? eIGNITION_ON : eIGNITION_OFF;

        // check if the state is 'run'
        if (STATE_RUN == rstrState) 
        {
            ic_core::IProduct *pProd = ic_core::CIgniteClient::GetProductImpl();
            ic_event::CIgniteEvent frmVersionEvent("1.0", "FirmwareVersion");
            std::string strFrmVersion = pProd->GetAttribute(
                                        ic_core::IProduct::eSWVersion);
            frmVersionEvent.AddField("value", strFrmVersion);
            frmVersionEvent.Send();
            StartSession();
        } 
        else 
        {
            EndSession();
        }
    }
}

bool CSessionStatusHandler::IsRepeatedSessionStatusEvnt(const bool &rbInProgress)
{
    bool bStatus = false;

    // check if the current session status and in progress session status is same
    if (rbInProgress == CSessionStatusHandler::m_bSessionInProgress)
    {
        bStatus = true;
    }
    return bStatus;
}

bool CSessionStatusHandler::IsAlert(ic_core::CEventWrapper* pEvent)
{
    if(ic_core::CUploadMode::GetInstance()->IsStreamModeSupported()) {
        return CMessageController::IsAlert(pEvent->GetEventId());
    }

    return false;
}

int CSessionStatusHandler::StartSession()
{
    double dblTimestamp = ic_utils::CIgniteDateTime::GetCurrentTimeMs(); //should we do timestamp-30000.0 ???;
    HCPLOG_METHOD() << " timestamp=" << dblTimestamp;

    ic_event::CIgniteEvent startupEvent("1.0", "SessionStatus", dblTimestamp);
    startupEvent.AddField("status", "startup");

    double dblBootTime = ic_utils::CIgniteDateTime::GetTimeSinceBootMs();
    unsigned long long ullBootTimeinSecs  =  dblBootTime / 1000; //convert to secs
    HCPLOG_D << "TimeSinceBoot ::" << ullBootTimeinSecs;
    startupEvent.AddField("timeSinceBootInSecs", (double)ullBootTimeinSecs);

    int nRestartCount = ic_core::CIgniteClient::GetProductImpl()->GetRestartCount();

    if (0 == nRestartCount)
    {
        startupEvent.AddField("reason", "normalStartup");
        if (dblBootTime <= MAX_WAIT_SINCE_BOOT_MILI_SECS) //Threshold for adjusting SesssionStatus TS.
        {
            double dblNewTS =  dblTimestamp - dblBootTime;
            startupEvent.SetTimestamp(dblNewTS);
            startupEvent.AddField("timestampAdjusted", true);
        }
        else
        {
            startupEvent.AddField("timestampAdjusted", false);
        }
    }
    else
    {
        std::string startupReason = ic_core::CIgniteClient::GetProductImpl()->
                                    GetStartupReason();

        startupEvent.AddField("reason", startupReason);
        startupEvent.AddField("restartCount", nRestartCount);
    }

    ic_utils::Json::Value jsonProperties = ic_bl::CUploadUtils::GetUploadProperties();
    startupEvent.AddFieldAsRawJsonString("properties", jsonProperties.toStyledString());

    std::string strStartup;
    startupEvent.EventToJson(strStartup);
    return startupEvent.Send();
}

int CSessionStatusHandler::EndSession()
{
    ic_event::CIgniteEvent shutDownEvent("1.0", "SessionStatus");
    shutDownEvent.AddField("status", "shutdown");
    shutDownEvent.AddField("reason", "normalShutdown");
    return shutDownEvent.Send();
}

CSessionStatusHandler::~CSessionStatusHandler()
{
    delete m_pSetExceptionEvents;
}

bool CSessionStatusHandler::GetSessionStatus()
{
    return CSessionStatusHandler::m_bSessionInProgress;
}

std::string CSessionStatusHandler::GetComponentStatus()
{
    ic_utils::Json::Value jsonSessionStatus;

    jsonSessionStatus["SessionInProgress"] = CSessionStatusHandler::m_bSessionInProgress;

    ic_utils::Json::FastWriter fastWriter;
    std::string strDiagInfo(fastWriter.write(jsonSessionStatus));

    // FastWriter introduces newline at the end , that needs to be truncated
    strDiagInfo.erase(std::remove(strDiagInfo.begin(), strDiagInfo.end(), '\n'), strDiagInfo.end());

    return strDiagInfo;
}
} /* namespace ic_bl*/
