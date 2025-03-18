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

#include <sys/time.h>
#include "CIgniteMutex.h"

namespace ic_utils 
{
CIgniteMutex::CIgniteMutex()
{
    m_stMutex = PTHREAD_MUTEX_INITIALIZER;
}

CIgniteMutex::~CIgniteMutex()
{
    pthread_mutex_destroy(&m_stMutex);
}

void CIgniteMutex::Lock()
{
    pthread_mutex_lock(&m_stMutex);
}

void CIgniteMutex::Unlock()
{
    pthread_mutex_unlock(&m_stMutex);
}

bool CIgniteMutex::TryLock()
{
    return pthread_mutex_trylock(&m_stMutex);
}

CThreadCondition::CThreadCondition()
{
    m_conditionVar = PTHREAD_COND_INITIALIZER;
}

CThreadCondition::~CThreadCondition()
{
    pthread_cond_destroy(&m_conditionVar);
}

int CThreadCondition::ConditionSignal()
{
    return pthread_cond_signal(&m_conditionVar);
}

int CThreadCondition::ConditionBroadcast()
{
    return pthread_cond_broadcast(&m_conditionVar);
}

int CThreadCondition::ConditionWait(CIgniteMutex& rMutex)
{
    return pthread_cond_wait(&m_conditionVar, rMutex.GetMutexHandle());
}

int CThreadCondition::ConditionTimedwait(CIgniteMutex& rMutex , 
                                         unsigned int unTimeInMs)
{
    struct timespec stTimeToWait;
    struct timeval now;
    gettimeofday(&now,NULL);
    stTimeToWait.tv_sec =  now.tv_sec + (unTimeInMs / 1000);
    stTimeToWait.tv_nsec = (now.tv_usec+1000UL*unTimeInMs)*1000UL;
    return pthread_cond_timedwait(&m_conditionVar, rMutex.GetMutexHandle(), 
                                  &stTimeToWait);
}

CScopeLock::CScopeLock(CIgniteMutex& rMutex): m_Mutex(rMutex) 
{
    m_Mutex.Lock();
}

CScopeLock::~CScopeLock() 
{
    m_Mutex.Unlock();
}

} /* namespace ic_utils */