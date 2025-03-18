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

#include "dam/CSigThread.h"

namespace ic_core
{
CSigThread::CSigThread(const int &rnSignum):m_nSignum(rnSignum),
                                            m_bWaiting(false)
{
    sigset_t signals_to_block;
    sigemptyset(&signals_to_block);
    sigaddset(&signals_to_block, m_nSignum);
    pthread_sigmask(SIG_BLOCK, &signals_to_block, NULL);
}

bool CSigThread::Notify(const pthread_t &rTid)
{
    int nRes=-1;
    if (!pthread_equal(rTid, pthread_self()))
    {
        if (rTid && m_bWaiting)
        {
            nRes = pthread_kill(rTid, m_nSignum);
        }
    }

    return (nRes==0)?true:false;
}

bool CSigThread::Wait(bool bTimout)
{
    int nRes;
    sigset_t set;

    nRes = sigemptyset(&set);
    if (nRes!=0)
    {
        return false;
    }

    nRes = sigaddset(&set, m_nSignum);
    if (nRes!=0)
    {
        return false;
    }


#if defined(__ANDROID__)
    int nRetsig;
    m_bWaiting = true;
    nRes = sigwait(&set,&nRetsig);
    m_bWaiting = false;

    if (nRetsig != m_nSignum)
    {
        nRes=-1;
    }
#else
    if (bTimout)
    {
        struct timespec stTimeout;
        stTimeout.tv_sec = 1;
        stTimeout.tv_nsec = 0;
        m_bWaiting = true;
        nRes = sigtimedwait(&set, NULL, &stTimeout);
        m_bWaiting = false;
    }
    else
    {
        int nRetsig;
        m_bWaiting = true;
        nRes = sigwait(&set,&nRetsig);
        m_bWaiting = false;

        if (nRetsig != m_nSignum)
        {
            nRes=-1;
        }
    }
#endif

    return (nRes==0)?true:false;
}
} /* namespace ic_core */