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

#include <signal.h>
#include <unistd.h>
#include "CIgniteThread.h"
#include "CIgniteLog.h"

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CIgniteThread"

namespace ic_utils
{
// const std::string VERSION =   "v3.0.0";

#if defined(__ANDROID__)
    void Exit_Handler(int sig)
    {
        HCPLOG_E << "Exiting Thread:" << pthread_self();
        if (sig != SIGALRM)
        {
            HCPLOG_E << "Recieved signal to stop process:" << sig;
            exit(0);
        }
        pthread_exit(0);
    }
#endif

CIgniteThread::~CIgniteThread()
{
    Stop();
    sleep(1);
}

int CIgniteThread::Start()
{
    HCPLOG_D << "CIgniteThread to be started";

    pthread_attr_t attrs;
    pthread_attr_init(&attrs);
    if (m_bIsDetached)
    {
        pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);
    }

    HCPLOG_D << "signal to be register";
#if defined(__ANDROID__)
    sigset_t mask;

    struct sigaction stActions;
    memset(&stActions, 0, sizeof(stActions));
    sigemptyset(&stActions.sa_mask);
    stActions.sa_flags = 0;
    stActions.sa_handler = Exit_Handler;
    int nRc = sigaction(SIGALRM, &stActions, NULL);
    // WARNING: cppcheck-suppress redundantAssignment
    nRc = sigaction(SIGHUP, &stActions, NULL);
    // WARNING: cppcheck-suppress unreadVariable
    nRc = sigaction(SIGTERM, &stActions, NULL);

#endif
    HCPLOG_D << "signal registered";
    int nRet = pthread_create(&m_pthreadId, &attrs, &(this->StartThread), this);
    HCPLOG_D << "ThreadId:" << m_pthreadId << ",nRet:" << nRet;
    if (nRet == 0)
    {
        m_bIsRunning = true;
    }
    else
    {
        HCPLOG_E << "Error creating the thread. Error code=" << nRet;
    }
    return nRet;
}

int CIgniteThread::Stop()
{
    HCPLOG_D << "ThreadId:" << m_pthreadId;
    int nRet = -1;

    if (m_bIsRunning)
    {
        if (!m_bIsDetached)
        {
            Detach();
        }
#if defined(__ANDROID__)
        nRet = pthread_kill(m_pthreadId, SIGALRM);
#else
        nRet = pthread_cancel(m_pthreadId);
#endif
        if (nRet == 0)
        {
            m_bIsRunning = false;
        }
        else
        {
            HCPLOG_E << "Error cancelling thread.code:" << nRet;
        }
    }
    return nRet;
}

void CIgniteThread::Exit(void *pData)
{
    HCPLOG_C << "ThreadId:" << m_pthreadId;
    pthread_exit(pData);
}

int CIgniteThread::Join()
{
    HCPLOG_D << "ThreadId:" << m_pthreadId;
    int nRet = -1;

    if (m_bIsRunning && !m_bIsDetached)
    {
        nRet = pthread_join(m_pthreadId, NULL);
        if (nRet == 0)
        {
            m_bIsRunning = false;
            m_bIsDetached = true;
        }
        else
        {
            HCPLOG_E << "Error joining the thread. Error code=" << nRet;
        }
    }
    else
    {
        HCPLOG_E << "Requested thread is not running or is already detached!";
    }

    return nRet;
}

int CIgniteThread::Detach()
{
    HCPLOG_D << "ThreadId:" << m_pthreadId;

    int nRet = -1;
    if (m_bIsRunning && !m_bIsDetached)
    {
        // Detach a thread that's already running.
        nRet = pthread_detach(m_pthreadId);
        if (nRet != 0)
        {
            HCPLOG_E << "Error detaching thread.code:" << nRet;
        }
    }
    else
    {
        /* Otherwise we'll just set the flag and set the attribute
            when we Start() */
        nRet = 0;
    }

    if (nRet == 0)
    {
        m_bIsDetached = true;
    }

    return nRet;
}

int CIgniteThread::SetCurrentThreadPriority(const int nPrio)
{
    HCPLOG_D << "Priority:" << nPrio;
    // cppcheck-suppress redundantAssignment
    int nRet = -1;
    int nPolicy;
    struct sched_param stParam;
    // cppcheck-suppress redundantAssignment
    nRet = pthread_getschedparam(pthread_self(), &nPolicy, &stParam);
    if (nRet == 0)
    {
        stParam.sched_priority = nPrio;
        nRet = pthread_setschedparam(pthread_self(), nPolicy, &stParam);
        if (nRet != 0)
        {
            HCPLOG_E << "setSchedParam fail nRet:" << nRet << ",prio:" << nPrio;
        }
    }
    else
    {
        HCPLOG_E << "getSchedParam fail nRet:" << nRet << ",prio:" << nPrio;
    }
    return nRet;
}

int CIgniteThread::SetCurrentThreadName(const std::string &rstrName)
{
    HCPLOG_C << "Name:" << rstrName;
    int nRet = 0;
#if defined(__QNX__) || defined(__gnu_linux__)
    nRet = pthread_setname_np(pthread_self(), rstrName.c_str());
#elif defined(__MACH__)
    // No way to set name of a different thread on OSX/BSD.
    nRet = pthread_setname_np(rstrName.c_str());
#endif
    if (nRet != 0) // cppcheck-suppress knownConditionTrueFalse
    {
        HCPLOG_E << "Error setting thread rstrName.nRet:" << nRet;
    }
    return nRet;
}

void *CIgniteThread::StartThread(void *p)
{
    CIgniteThread *pT = (CIgniteThread *)p;
    pT->Run();
    // Once run method exits, set to not running.
    pT->m_bIsRunning = false;
    return NULL;
}

}
