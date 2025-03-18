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

#include "CHttpSessionManager.h"
#include "CIgniteLog.h"

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CHttpSessionManager"

#include "curl-based/CCurlHttpSession.h"
#include <unistd.h>
#include "jsoncpp/json.h"

namespace ic_network
{

namespace 
{
pthread_mutex_t g_SessionMutex = PTHREAD_MUTEX_INITIALIZER;
} /* namespace */

CSSLAttributes* CHttpSessionManager::m_pSslAttributes = NULL;

void CHttpSessionManager::SetSSLAttributes(CSSLAttributes *pSslAttributes) {
    m_pSslAttributes = pSslAttributes;

    HCPLOG_C << "cafile:" << pSslAttributes->GetCAfile()
             << ",capath:" << pSslAttributes->GetCApath()
             << ",clientcert:" << pSslAttributes->GetCertFile()
             << ",privkey:" << pSslAttributes->GetKeyFile()
             << ",verifyPeer:" << pSslAttributes->GetVerifyPeer()
             << ",verifyHost:" << pSslAttributes->GetVerifyHost()
             << ",useTLSEngine:" << pSslAttributes->UseTLSengine()
             << ",engineID:" << pSslAttributes->GetTLSengineID();
}

IHttpSession* CHttpSessionManager::AcquireSession()
{

    IHttpSession* pAcq = NULL;

    HCPLOG_T;
    pthread_mutex_lock(&g_SessionMutex);
    int nI;
    for (nI = 0; nI < MAX_SESSIONS; nI++)
    {
        // check if session for number 'nI' is already acquired or not
        if (m_bSessionAcquired[nI] == false)
        {
            // check and create session
            CreateSession(nI);

            //if creation of session in above if loop fails return from function
            if (m_pSession[nI] == NULL)
            {
                HCPLOG_F << "FATAL: No Http Sessions available." <<
                            "  Do you have a leak?";
                return NULL;
            }

            // clear all the previous settings of the session
            m_pSession[nI]->Reset();
            // apply TLS settings
            m_pSession[nI]->ApplyTLSSettings();
            m_bSessionAcquired[nI] = true;
            m_unNumSessionsAcquired++;
            pAcq = m_pSession[nI];
            if (!m_strProxy.empty() && (m_unProxyPort != 0))
            {
                pAcq->SetProxy(m_strProxy, m_unProxyPort, m_strProxyUser, 
                               m_strProxyPwd);
            }
            if (m_unPortRangeStart != 0 && m_unPortRangeEnd != 0 )
            {
                pAcq->SetLocalPortRange(m_unPortRangeStart, m_unPortRangeEnd);
            }
            break;
        }
    }
    pthread_mutex_unlock(&g_SessionMutex);
    HCPLOG_METHOD() << "Returning session " << nI << "=" << pAcq;

    // check if number of threshold is crossed.
    CheckForSessionThresholdWarning();

    return pAcq;
}

void CHttpSessionManager::CreateSession(const int nIndex)
{
    if (m_pSession[nIndex] == NULL)
    {

        if (nullptr != m_pExtHttpSessionHandler)
        {
            std::cout << "EXTERNAL SESSION IS BEING CREATED" << std::endl;
            m_pSession[nIndex] = 
               m_pExtHttpSessionHandler->CreateSession(m_pSslAttributes);
        }
        else
        {
            m_pSession[nIndex] = new CCurlHttpSession(m_pSslAttributes);
        }
    }
}

void CHttpSessionManager::CheckForSessionThresholdWarning(void)
{
    /* Our number of simultaneous sessions is set to several higher than we
     * think we will ever reach.  If we get close, log an event so we know. But
     * only send the event once for each count.
     */
    if ((m_unNumSessionsAcquired >= (MAX_SESSIONS - 2)) && 
        (m_unNumSessionsAcquired > m_unSessionWarningSentAt))
    {
        /* TBD: considering adding Event support using compile time flag to
         *      decide if AcpEvent has to be reported
         * acp::Event nearThresholdEvent("0.1", "HttpSessionThresholdWarning");
         * nearThresholdEvent.addField("count", (int)m_unNumSessionsAcquired);
         * nearThresholdEvent.send();
         */
        m_unSessionWarningSentAt = m_unNumSessionsAcquired;
    }
}

void CHttpSessionManager::ReleaseSession(const IHttpSession *pSession)
{

    HCPLOG_T << "session=" << pSession;
    pthread_mutex_lock(&g_SessionMutex);
    for (int nI = 0; nI < MAX_SESSIONS; nI++)
    {
        if (pSession == m_pSession[nI])
        {
            if (m_pSession[nI]->GetLastError() != HttpErrorCode::eERR_OK)
            {
                /* Delete old session and create a new one next time,
                 * to resolve the "No Route to Host" problem.
                 */
                delete m_pSession[nI];
                HCPLOG_D << "Existing Session Deleted";
                m_pSession[nI] = NULL;
            }
            m_bSessionAcquired[nI] = false;
            m_unNumSessionsAcquired--;
        }
    }
    pthread_mutex_unlock(&g_SessionMutex);

}

void CHttpSessionManager::SetProxy(const std::string& rstrHost, 
                                  const unsigned int unPort,
                                  const std::string& rstrUser, 
                                  const std::string& rstrPassword)
{
    m_strProxy = rstrHost;
    m_unProxyPort = unPort;
    m_strProxyUser = rstrUser;
    m_strProxyPwd = rstrPassword;
}

void CHttpSessionManager::SetLocalPortRange(const unsigned int unStart,
        const unsigned int unEnd)
{
    m_unPortRangeStart = unStart;
    m_unPortRangeEnd = unEnd;

    pthread_mutex_lock(&g_SessionMutex);
    for (int nI = 0; nI < MAX_SESSIONS; nI++)
    {
        //set port range if the seesion is acquierd for session number 'i'
        if (m_bSessionAcquired[nI] == true)
        {
            m_pSession[nI]->SetLocalPortRange(m_unPortRangeStart,
                                              m_unPortRangeEnd);
        }
    }
    pthread_mutex_unlock(&g_SessionMutex);

}

CHttpSessionManager *CHttpSessionManager::GetInstance(void)
{
    HCPLOG_METHOD();
    static CHttpSessionManager hsmSingleton;
    return &hsmSingleton;
}

void CHttpSessionManager::ReleaseInstance()
{
    GetInstance()->ReleaseResources();
}

void CHttpSessionManager::ReleaseResources()
{
    for (int nI = 0; nI < MAX_SESSIONS; nI++)
    {
        if (m_pSession[nI] != NULL)
        {
            delete m_pSession[nI];
            m_pSession[nI] = NULL;
        }
    }
}

CHttpSessionManager::CHttpSessionManager()
{
    m_unPortRangeStart = 0;
    m_unPortRangeEnd = 0;

    m_unNumSessionsAcquired = 0;
    m_unSessionWarningSentAt = 0;
    // Assigning the default values for session array
    for (int nI = 0; nI < MAX_SESSIONS; nI++)
    {
        m_pSession[nI] = NULL;
        m_bSessionAcquired[nI] = false;
    }
}

CHttpSessionManager::~CHttpSessionManager()
{
}

void CHttpSessionManager::SetExternalHttpSessionHandler(
                            IExternalHttpSessionHandler *pHandler)
{
    if (nullptr != pHandler)
    {
        m_pExtHttpSessionHandler = pHandler;
    }
}
}
