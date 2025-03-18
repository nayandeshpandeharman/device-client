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
* \file CIgniteMutex.h
*
* \brief This file contains the CIgniteMutex class which implements the OS 
* abstraction for Mutex.
*******************************************************************************
*/

#ifndef CIGNITE_MUTEX_H
#define CIGNITE_MUTEX_H

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>

namespace ic_utils 
{
/**
 * CIgniteMutex class manages the functionalities to have concurrent access to 
 * shared resources and preventing conflicts.
 */
class CIgniteMutex 
{
public:
    /**
     * Default no-argument constructor.
     */
    CIgniteMutex();
    
    /**
     * CIgniteMutex destructor
     */
    ~CIgniteMutex();

    /**
     * Method to wait for a lock on a mutex object.
     * @param void
     * @return void
     */
    void Lock();

    /**
     * Method to try to lock mutex but do not block thread if it does not get 
     * the lock.
     * @param void
     * @return false, if lock was successfully acquired; 
     *         true, otherwise.
     */
    bool TryLock();

    /**
     * Method to unlock the mutex.
     * @param void
     * @return void
     */
    void Unlock();

    /**
     * Method to get the mutex handle.
     * @param void
     * @return pointer to mutex handle of the underlying pthread_mutex_t type.
     */
    pthread_mutex_t* GetMutexHandle()
    {
        return &m_stMutex;
    }
private:
    //! Mutex Handle, instance of pthread_mutex_t type.
    pthread_mutex_t m_stMutex;
};

/**
 * CThreadCondition class is used in conjunction with a mutex to facilitate
 * thread synchronization in a multithreaded environment.
 */
class CThreadCondition 
{
public:
    /**
     * Default no-argument constructor.
     */
    CThreadCondition();

    /**
     * CThreadCondition Destructor
     */
    ~CThreadCondition();

    /**
     * Method to unblock atleast one threads that is blocked on the  
     * specified condition variable.
     * @param void
     * @return 0, if the signaling operation succeeds; -1 otherwise
     */
    int ConditionSignal();

    /**
     * Method to unblock all threads that are blocked on the specified 
     * condition variable.
     * @param void
     * @return 0, if the signaling operation succeeds; -1 otherwise
     */
    int ConditionBroadcast();

    /**
     * Method to wait for a signal on the condition variable and
     * release the associated mutex (Mutex) during the wait.
     * @param void
     * @return 0, if the signaling operation succeeds; -1 otherwise
     */
    int ConditionWait(CIgniteMutex&);

    /**
     * Method to wait a for a signal on the condition variable and
     * release the associated mutex (Mutex) during the wait with a timeout 
     * specified in milliseconds.
     * @param mutex Reference to an instance of the Mutex class.
     * @param timeInMs Specifies the maximum time duration, in milliseconds, 
     * that the thread is willing to wait for the condition variable to be 
     * signaled.
     * @return 0, if the signaling operation succeeds; -1 otherwise
     */
    int ConditionTimedwait(CIgniteMutex& , unsigned int );

private:
    //! condition variable; instance of pthread_cond_t type.
    pthread_cond_t m_conditionVar;
};

/**
 * class ScopLock is a mutex wrapper that provides a convenient mechanism for 
 * locking mutex for the duration of a scoped block.
 * When a ScopLock object is created, it attempts to lock the mutex it is given.
 * When control leaves the scope in which the CScopeLock object was created,
 * the CScopeLock is destructed and the mutex are released/unlocked. 
 */
class CScopeLock
{
private:
    //! Mutex object to hold Mutex to be locked for slope.
    CIgniteMutex& m_Mutex;

public:
    /**
     * CScopeLock Constructor
     * @param[in] Reference of Mutex obj
     */
    CScopeLock(CIgniteMutex& rMutex);

    /**
     * CScopeLock Destructor
     */
    ~CScopeLock();

private:
    /**
     * CScopeLock constructor in private scope to avoid creation of CScopeLock
     * object without ic_utils::CIgniteMutex param.
     */
    CScopeLock();

    /**
     * CScopeLock copy constructor in private scope.
     * The CScopeLock class is non-copyable.
     * @param[in] Reference to CScopeLock obj
     */
    CScopeLock(const CScopeLock&);

    /**
     * Overloaded assignment operator for CScopeLock class.
     * @param CScopeLock& Constant reference to CScopeLock object.
     * @return reference to a CScopeLock object.
     */
    CScopeLock& operator=(const CScopeLock&);
};
}
#endif /* CIGNITE_MUTEX_H */