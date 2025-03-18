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
* \file CSessionStatusHandler.h
*
* \brief This class handles Session Status of Ignite client.
********************************************************************************
*/

#ifndef CSESSION_STATUS_HANDLER_H
#define CSESSION_STATUS_HANDLER_H

#include <set>
#include "CTransportHandlerBase.h"

namespace ic_bl 
{

/**
 * Class CSessionStatusHandler handles session status of ignite client.
 * This class generates ignition status event, if non zero RPM is received before
 * IgnStatus event
 */
class CSessionStatusHandler : public CTransportHandlerBase 
{

public:
    /**
     * Parameterized constructor
     * @param[in] pNextHandler Pointer to the CTransportHandlerBase class
     */
    CSessionStatusHandler(CTransportHandlerBase* nextHandler);

    /**
     * Method to handle session status related events
     * @param[in] pEvent Event data
     * @return void
     */
    virtual void HandleEvent(ic_core::CEventWrapper* pEvent);

    /**
     * Destructor
     */
    ~CSessionStatusHandler();

    /**
     * Method to get session status
     * @param void
     * @return true if session is in progress, false otherwise
     */
    bool GetSessionStatus();

    /**
     * Method to captures this component/class related Diagnostcs information and report to caller.
     * @param void
     * @return component related diag info string
     */
    static std::string GetComponentStatus();

    #ifdef IC_UNIT_TEST
        friend class CSessionStatusHandlerTest;
    #endif
private:
    enum IgnitionState
    {
        eIGNITION_OFF,     ///< Ignition Status Off
        eIGNITION_ON,      ///< Ignition Status On
        eIGNITION_UNKNOWN  ///< Ignition Status not known
    };
    
    /**
     * Method to check if received event is just an event or alert
     * @param[in] pEvent Event object
     * @return true if alert is received, false otherwise
     */
    bool IsAlert(ic_core::CEventWrapper* pEvent);

    /**
     * Method to start the session
     * @param void
     * @return Returns an error code. Error code 0 means success (no errors)
     */
    int StartSession();

    /**
     * Method to end the session
     * @param void
     * @return Returns an error code. Error code 0 means success (no errors)
     */
    int EndSession();

    /**
     * Method to generate Ign status(On/Off),and DongalRemoval based on RPM.
     * If RPM value > 0, then will raise the IgnON event
     * If RPM value <= 0, then will raise the IgnOFF event
     * Similarly, If previous device status is detached,then will raise the dongal removal status as "attached".
     * With the below configurable payload, Ign ON/Off, and DeviceRemoval can be enabled/disabled.
     * "UseRpmForIgnition": {  "IgnON": true, "IgnOFF": false, "DeviceRemoval": true },
     * Note: true means enabled, false means disabled
     * If UseRpmForIgnition is set as below,
     * "UseRpmForIgnition" : true, then it will generate all above events. 
     * Setting it to false will disable generation of these events.
     * @param[in] pEvent Event object
     * @return If ignition status On/Off is sent then returns 0, otherwise non-zero value.
     */
    int HandleRPMEvent(ic_core::CEventWrapper* pEvent);
    
    /**
     * Method to check if event has exeception to upload before session starts
     * @param rStrEventID Event ID
     * @return true if event has exeception, false otherwise
     */
    bool IsExceptionEvent(const std::string& rStrEventID);

    /**
     * Method to initialize session exception events list
     * @param void
     * @return void
     */
    void InitSessionExceptionEvent();

    /**
     * Method to get rpm value
     * @param[in] pEvent rpm event
     * @return rpm value
     */
    int GetRpmValue(ic_core::CEventWrapper *pEvent);

    /**
     * Method to send ignition status event
     * @param[in] rstrState ign state
     * @return if event is sent successfully returns 0, otherwise non-zero value.
     */
    int SendIgnStatusEvent(const std::string &rstrState);

    /**
     * Method to check if event id is RPM
     * @param[in] rstrStartupType startup type
     * @param[in] rstrEventID event id
     * @return true if event id is RPM when startup type is self, false otherwise
     */
    bool IsRpmEvent(const std::string &rstrStartupType,
                    const std::string &rstrEventID);

    /**
     * Method to check if RPM is used for IgnStatus
     * @param void
     * @return true if RPM is used for IgnStatus, false otherwise
     */
    bool IsRpmUsedForIgnStatus();

    /**
     * Method to check if event id is 'IgniteClientLaunched'
     * @param[in] rstrStartupType startup type
     * @param[in] rstrEventID event id
     * @return true if event id is IgniteClientLaunched when startup type is 
     * external, false otherwise
     */
    bool IsIgniteClientLaunchedEvent(const std::string &rstrStartupType,
                                     const std::string &rstrEventID);
    
    /**
     * Method to handle 'IgniteClientLaunched' event
     * @param[in] rstrState 'IgniteClientLaunched' event state
     * @return void
     */
    void HandleIgniteClientLaunchedEvent(const std::string &rstrState);

    /**
     * Method to check if event id is IgnStatus
     * @param[in] rstrStartupType startup type
     * @param[in] rstrEventID event id
     * @return true if event id is IgnStatus when startup type is self, 
     * false otherwise
     */
    bool IsIgnStatusEvent(const std::string &rstrStartupType,
                          const std::string &rstrEventID);

    /**
     * Method to check if 'IgnStatus' event state is repeated
     * @param[in] rstrState 'IgnStatus' event state
     * @return true if current ign state and previous ign state is same,
     * false otherwise
     */
    bool IsRepeatedIgnStatusEvnt(const std::string &rstrState);

    /**
     * Method to handle 'IgnStatus' event
     * @param[in] rstrState IgnStatus event state
     * @return void
     */
    void HandleIgnStatusEvent(const std::string &rstrState);

    /**
     * Method to check if 'SessionStatus' event state is repeated
     * @param[in] rbInProgress SessionStatus state
     * @return true if current session status and in progress session status 
     * is same, false otherwise
     */
    bool IsRepeatedSessionStatusEvnt(const bool &rbInProgress);

    //! Member variable flag storing session status
    static bool m_bSessionInProgress;

    //! Member variable flag storing ignition status
    IgnitionState m_eIgnState;

    //! Member variable object holds set of exceptional events to SessionStatus
    std::set<std::string> *m_pSetExceptionEvents;

    //! Member variable storing last ignition status
    IgnitionState m_eLastIgnStatus;

    //! Member variable flag holds value of generating ignition event based on RPM
    bool m_bGenIgnON;

    //! Member variable flag holds value of generating ignition event based on RPM
    bool m_bGenIgnOFF;
};
} /* namespace ic_bl*/
#endif // CSESSION_STATUS_HANDLER_H
