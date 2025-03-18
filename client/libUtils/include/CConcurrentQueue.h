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
* \file CConcurrentQueue.h
*
* \brief CConcurrentQueue provides a thread-safe queue implementation suitable 
* for concurrent operations.
*******************************************************************************
*/

#ifndef CCONCURRENTQUEUE_H
#define CCONCURRENTQUEUE_H
#include <unistd.h>

namespace ic_utils 
{
/**
 * A abstracted template class representing a generic queue.
 * @tparam Data The type of data to be stored in the queue.
 */
template <class Data>
class CQueue
{
public:
    /**
     * Default no-argument constructor.
     */
    CQueue()
    {
        //do nothing
    };

    /**
     * Virtual destructor.
     */
    virtual ~CQueue()
    {
        //do nothing
    };

    /**
     * Method to put data into the queue.
     * @param data The data to be inserted into the queue.
     * @param unDataSize The size of the data (optional, default is 1).
     * @return True if successful, false otherwise.
     */
    virtual bool Put(Data data, unsigned int unDataSize = 1) = 0;

    /**
     * Method to take data from the queue.
     * @param pData Pointer to the variable where the retrieved data will be
     * stored.
     * @return True if successful, false if the queue is empty.
     */
    virtual bool Take(Data *pData) = 0;

    /**
     * Method to get the current size of the queue.
     * @param void
     * @return The size of the queue.
     */
    virtual unsigned int Size() = 0;
};
} // namespace ic_utils 

/*
 * Both of the conditional compile flags below must be set manually in
 * the Makefile as needed
 *
 * SHORT_POINTER - Must be set when long long integer type is longer than
 *      pointer so that m_ullnNodePtr have enough room to hold both node pointer
 *      as well as size counter. This flag can still be set if long long and
 *      pointer are of equla length, the queue will be functional although size
 *      of the queue will not be tracked and all Size() call attempts will
 *      result in program termination.
 * CPP11_SUPPORT - Is set when C++11 standard is fully supported and
 *      <atomic> utilities can be used.
 */
#if defined SHORT_POINTER && defined CPP11_SUPPORT

#include <atomic>
#include "CIgniteLog.h"

namespace ic_utils 
{
/**
 * A template class representing a concurrent queue with thread-safe operations.
 * @tparam Data The type of data stored in the queue.
 */
template <class Data>
class CConcurrentQueue : CQueue<Data>
{
private:
    //! Number of bits to shift for size tracking in m_ullnNodePtr.
    static const int OFFSET = sizeof(unsigned int) * 8;

    //! Type definition for node pointer.
    typedef unsigned long long int m_ullnNodePtr;

    //! Type definition for atomic node pointer.
    typedef std::atomic<m_ullnNodePtr> m_atomicNodePtr;

    //! Node structure representing elements in the queue.
    struct Node
    {
        Data value;
        m_atomicNodePtr atomicNext;
    };

    //! Atomic pointer to the head of the queue.
    m_atomicNodePtr m_atomicHead;

    //! Atomic pointer to the tail of the queue.
    m_atomicNodePtr m_atomicTail;

    //! Flag indicating whether to track the size of the queue.
    bool m_bTrackSize;

    /**
     * Method to compare and swaps node pointers atomically.
     * @param[in,out] atomicDest Destination node pointer.
     * @param[in,out] ullnExpected Expected node pointer.
     * @param[in] ullnDesired Desired node pointer.
     * @return True if the operation is successful, false otherwise.
     */
    bool ComAndSwap(m_atomicNodePtr *atomicDest, m_ullnNodePtr *ullnExpected, 
                    m_ullnNodePtr ullnDesired);

    /**
     * Method to generate a new node pointer with optional size
     * tracking.
     * @param[in] pstPtr Pointer to the node (optional).
     * @param[in] unSize Size of the node (optional).
     * @return The new node pointer.
     */
    m_ullnNodePtr NewNodePtr(Node *pstPtr, unsigned int unSize);

    /**
     * Method to retrieve the pointer from a node pointer.
     * @param[in] ullnNodePtr The node pointer.
     * @return Pointer to the node.
     */
    Node *GetPtr(const m_ullnNodePtr ullnNodePtr);

    /**
     * Method to retrieve the size from a node pointer.
     * @param[in] ullnNodePtr The node pointer.
     * @return The size retrieved from the node pointer.
     */
    unsigned int GetSize(const m_ullnNodePtr ullnNodePtr);

    /**
     * Method to delete a node pointer, freeing associated
     * resources.
     * @param[in] ullnNodePtr The node pointer to delete.
     * @return void
     */
    void DeleteNodePtr(m_ullnNodePtr ullnNodePtr);

public:
    /**
     * Default no-argument constructor.
     */
    CConcurrentQueue();

    /**
     * Destructor
     */
    ~CConcurrentQueue();

    /**
     * Method to insert data into the queue.
     * @param[in] data The data to be inserted.
     * @param[in] unDataSize The size of the data (optional, default is 1).
     * @return True if the data is successfully inserted, false otherwise.
     */
    bool Put(Data data, unsigned int unDataSize = 1);

    /**
     * Method to retrieve data from the queue.
     * @param[out] pData Pointer to store the retrieved data.
     * @return True if data is successfully retrieved, false if the queue is
     * empty.
     */
    bool Take(Data *pData);

    /**
     * Method to get the current size of the queue.
     * @param void
     * @return The size of the queue.
     */
    unsigned int Size();
};

template <class Data>
typename CConcurrentQueue<Data>::m_ullnNodePtr
CConcurrentQueue<Data>::NewNodePtr(Node *pstPtr, unsigned int unSize)
{
    Node *pstNode = NULL == pstPtr ? new Node() : pstPtr;
    m_ullnNodePtr ullnRet = (unsigned long)pstNode;
    if (m_bTrackSize)
    {
        ullnRet = (ullnRet << OFFSET) | unSize;
    }
    return ullnRet;
}

template <class Data>
typename CConcurrentQueue<Data>::Node *CConcurrentQueue<Data>::
    GetPtr(const m_ullnNodePtr ullnNodePtr)
{
    Node *pstPtr = (Node *)(m_bTrackSize ? ullnNodePtr >> OFFSET : ullnNodePtr);
    return pstPtr;
}

template <class Data>
unsigned int CConcurrentQueue<Data>::GetSize(const m_ullnNodePtr ullnNodePtr)
{
    return (unsigned int)ullnNodePtr;
}

template <class Data>
void CConcurrentQueue<Data>::DeleteNodePtr(m_ullnNodePtr ullnNodePtr)
{
    Node *pstNode = GetPtr(ullnNodePtr);
    if (NULL != pstNode)
    {
        delete pstNode;
    }
}

template <class Data>
bool CConcurrentQueue<Data>::ComAndSwap(m_atomicNodePtr *atomicDest, 
                                        m_ullnNodePtr *ullnExpected, 
                                        m_ullnNodePtr ullnDesired)
{
    return std::atomic_compare_exchange_strong(atomicDest, ullnExpected,
        NewNodePtr(GetPtr(ullnDesired), 
        GetSize(*ullnExpected) + GetSize(ullnDesired)));
}

template <class Data>
CConcurrentQueue<Data>::CConcurrentQueue()
{
    m_bTrackSize = sizeof(m_ullnNodePtr) > sizeof(Node *);
    m_ullnNodePtr ullnNode = NewNodePtr(NULL, 0);
    m_atomicHead = m_atomicTail = ullnNode;
}

template <class Data>
CConcurrentQueue<Data>::~CConcurrentQueue()
{
    Data data;
    while (Take(&data))
    {
    }
}

template <class Data>
bool CConcurrentQueue<Data>::Put(Data data, unsigned int unDataSize)
{
    m_ullnNodePtr ullnNode = NewNodePtr(NULL, unDataSize);
    GetPtr(ullnNode)->value = data;

    m_ullnNodePtr ullnTail;
    while (true)
    {
        ullnTail = m_atomicTail;
        m_ullnNodePtr ullnNext = GetPtr(ullnTail)->ullnNext;

        if (ullnTail == m_atomicTail)
        {
            if (0 == ullnNext)
            {
                if (ComAndSwap(&(GetPtr(ullnTail)->ullnNext), &ullnNext, 
                    ullnNode))
                {
                    break;
                }
            }
            else
            {
                ComAndSwap(&m_atomicTail, &ullnTail, ullnNext);
            }
        }
    }
    return ComAndSwap(&m_atomicTail, &ullnTail, ullnNode);
}

template <class Data>
bool CConcurrentQueue<Data>::Take(Data *pData)
{
    m_ullnNodePtr ullnHead;
    while (true)
    {
        ullnHead = m_atomicHead;
        m_ullnNodePtr ullnTail = m_atomicTail;
        m_ullnNodePtr ullnNext = GetPtr(ullnHead)->ullnNext;

        if (ullnHead == m_atomicHead)
        {
            if (ullnHead == ullnTail)
            {
                if (0 == ullnNext)
                {
                    return false;
                }
                ComAndSwap(&m_atomicTail, &ullnTail, ullnNext);
            }
            else
            {
                *pData = GetPtr(ullnNext)->value;
                if (ComAndSwap(&m_atomicHead, &ullnHead, ullnNext))
                {
                    break;
                }
            }
        }
    }

    DeleteNodePtr(ullnHead);
    return true;
}

template <class Data>
unsigned int CConcurrentQueue<Data>::Size()
{
    if (!m_bTrackSize)
    {
        HCPLOG_E << "Size tracking is not supported in this queue implementation "
            "due to CPU " << " architecture limitations !";
            exit(EXIT_FAILURE);
    }
    return GetSize(m_atomicTail) - GetSize(m_atomicHead);
}
} // namespace ic_utils 
#else

#include <queue>
#include "CIgniteMutex.h"

namespace ic_utils 
{
/**
 * A template class derived from CQueue representing a generic queue.
 * @tparam Data The type of data to be stored in the queue.
 */
template <class Data>
class CConcurrentQueue : CQueue<Data>
{
public:
    /**
     * Default no-argument constructor.
     */
    CConcurrentQueue();

    /**
     * Destructor
     */
    ~CConcurrentQueue();

    /**
     * Method to insert data into the queue.
     * @param[in] data The data to be inserted.
     * @param[in] unDataSize The size of the data (optional, default is 1).
     * @return True if the data is successfully inserted, false otherwise.
     */
    bool Put(Data data, unsigned int unDataSize = 1);

    /**
     * Method to retrieve data from the queue.
     * @param[out] pData Pointer to store the retrieved data.
     * @return True if data is successfully retrieved, false if the queue is
     *         empty.
     */
    bool Take(Data *pData);

    /**
     * Method to get the current size of the queue.
     * @param void
     * @return The size of the queue.
     */
    unsigned int Size();

private:
    //! Maximum number of retries for locking
    static const int MAX_LOCK_RETRIES = 10;

    //! Node structure to represent elements in the queue
    struct Node
    {
        Data value;
        unsigned int unSize;
    };

    //! Actual queue
    std::queue<Node *> m_queueQ;

    //! Mutex for controlling access to the queue
    ic_utils::CIgniteMutex m_modifyMutex;

    //! Variable to store the current size of the queue
    unsigned int m_unSize;
};

template <class Data>
CConcurrentQueue<Data>::CConcurrentQueue()
{
    m_unSize = 0;
}

template <class Data>
CConcurrentQueue<Data>::~CConcurrentQueue()
{
    while (!m_queueQ.empty())
    {
        Node *pstNode = m_queueQ.front();
        delete pstNode;
        m_queueQ.pop();
    }
}

template <class Data>
bool CConcurrentQueue<Data>::Put(Data data, unsigned int unDataSize)
{
    Node *pstNode = new Node();
    pstNode->value = data;
    pstNode->unSize = unDataSize;

    int nRetries = 0;
    while (m_modifyMutex.TryLock())
    {
        if (++nRetries >= MAX_LOCK_RETRIES)
        {
            delete pstNode;
            return false;
        }
        usleep(100);
    }

    m_queueQ.push(pstNode);
    m_unSize += unDataSize;

    m_modifyMutex.Unlock();

    return true;
}

template <class Data>
bool CConcurrentQueue<Data>::Take(Data *pData)
{
    m_modifyMutex.Lock();
    if (m_queueQ.empty())
    {
        m_modifyMutex.Unlock();
        return false;
    }
    Node *pstNode = m_queueQ.front();
    m_queueQ.pop();
    m_unSize -= pstNode->unSize;
    m_modifyMutex.Unlock();

    *pData = pstNode->value;
    delete pstNode;
    return true;
}

template <class Data>
unsigned int CConcurrentQueue<Data>::Size()
{
    return m_unSize;
}
} // namespace ic_utils 

#endif

#endif /* CCONCURRENTQUEUE_H */
