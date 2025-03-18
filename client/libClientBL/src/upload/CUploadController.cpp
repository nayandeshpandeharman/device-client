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

#include "CUploadController.h"
#include "config/CUploadMode.h"
#include "CIgniteLog.h"
#include "CIgniteConfig.h"
#include "CIgniteDateTime.h"
#include "net/CIgniteMQTTClient.h"

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CUploadController"

namespace ic_bl 
{
CUploadController* CUploadController::GetInstance()
{
    HCPLOG_METHOD();
    static CUploadController sSelf;
    return &sSelf;
}

void CUploadController::ReleaseInstance()
{
    GetInstance()->ReleaseResources();
}

void CUploadController::ReleaseResources()
{

}

CUploadController::CUploadController() : m_pMqttUploader(NULL)
{
    m_bIsUploadersSuspended = false;
}

CUploadController::~CUploadController()
{

}

void CUploadController::StartUploadController()
{
    HCPLOG_METHOD();

    ic_core::CUploadMode *pMode = ic_core::CUploadMode::GetInstance();

    if (pMode->IsStreamModeSupported() &&
            (ic_core::CIgniteConfig::GetInstance()->
            GetJsonValue("MQTT")).isObject()) 
    {
        HCPLOG_T << " - Starting MQTT Uploader...";
        m_pMqttUploader = CMQTTUploader::GetInstance();
        if (m_pMqttUploader)
        {
            m_pMqttUploader->Start();
        }
        else
        {
            HCPLOG_E << " - MQTTUploader is NULL!";
        }
    }

    Start();
}

void CUploadController::StopUploadController()
{
    HCPLOG_METHOD();

    if (NULL != m_pMqttUploader)
    {
        m_pMqttUploader->Stop();
    }
}

void CUploadController::SuspendUploadController()
{
    HCPLOG_METHOD();

    m_bIsUploadersSuspended = true;

    SuspendDirectUploaders();
}

void CUploadController::ResumeUploadController()
{
    HCPLOG_METHOD();

    m_bIsUploadersSuspended = false;

    ResumeDirectUploaders();
}

void CUploadController::Wait()
{
    m_WaitMutex.TryLock();
    m_WaitCondition.ConditionWait(m_WaitMutex);
}

void CUploadController::Notify()
{
    m_WaitCondition.ConditionBroadcast();
    m_WaitMutex.Unlock();
}

void CUploadController::Run()
{
    HCPLOG_METHOD();

    // Register to get the Shutdown Notification
    ic_core::CIgniteClient::GetOnOffMonitor()->
        RegisterForShutdownNotification(this,
                                        ic_core::IOnOff::eR_UPLOAD_CONTROLLER);

    while (!m_bIsShutdownInitiated)
    {
        Wait();
    }

    HCPLOG_I << "Exiting from UploadController";
    ic_core::CIgniteClient::GetOnOffMonitor()->
        ReadyForShutdown(ic_core::IOnOff::eR_UPLOAD_CONTROLLER);
    ic_core::CIgniteClient::GetOnOffMonitor()->
        UnregisterForShutdownNotification(ic_core::IOnOff::eR_UPLOAD_CONTROLLER);
    Detach();
}


void CUploadController::SuspendDirectUploaders()
{
    HCPLOG_METHOD();

    if (NULL != m_pMqttUploader) 
    {
        HCPLOG_T << "Suspending MQTT uploader service...";
        m_pMqttUploader->SuspendUpload();
    }
}

void CUploadController::ResumeDirectUploaders()
{
    HCPLOG_METHOD();
    if (!m_bIsUploadersSuspended)
    {
        if (NULL != m_pMqttUploader) {
            HCPLOG_T << "Resuming MQTT uploader service...";
            m_pMqttUploader->ResumeUpload();
        }
    }
}


int CUploadController::ForceStreamUpload(bool bExitWhenDone)
{
    int nUploadVal = -1;
    if(m_pMqttUploader)
    {
        nUploadVal = m_pMqttUploader->ForceUpload(bExitWhenDone);
    }

    return nUploadVal;
}

bool CUploadController::IsEventToBeForceUplaoded(const std::string& rstrEventID)
{
    bool bReturnValue = false;
    if(m_pMqttUploader)
    {
        bReturnValue = m_pMqttUploader->IsForceUploadEvent(rstrEventID);
    }

    return bReturnValue;
}

int CUploadController::TriggerAlertsUpload(const std::string& rstrAlert)
{
    int nReturnValue = -1;
    if(m_pMqttUploader)
    {
        nReturnValue = m_pMqttUploader->AlertUpload(rstrAlert);
    }

    return nReturnValue;
}

bool CUploadController::SuspendStreamUpload()
{
    bool bReturnValue = false;
    if(m_pMqttUploader)
    {
        bReturnValue = m_pMqttUploader->SuspendUpload();
        
    }

    return bReturnValue;
}

bool CUploadController::ReloadStreamPeriodicity(int nValue)
{
    bool bReturnValue = false;
    if(NULL != m_pMqttUploader)
    {
        bReturnValue = m_pMqttUploader->ReloadPeriodicity();
    }
    return bReturnValue;
}

void CUploadController::NotifyShutdown()
{
    HCPLOG_D << "Shutdown Request Recieved for UploadController";
    m_bIsShutdownInitiated = true;
    Notify();
}

} // namespace acp
