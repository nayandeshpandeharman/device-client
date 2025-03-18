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

#include <stdlib.h>
#include "dam/CEventTimestampValidationHandler.h"
#include "dam/CTransportHandlerBase.h"
#include "CIgniteLog.h"
#include "CIgniteDateTime.h"
#include "CIgniteConfig.h"
#include "db/CInvalidTimestampEventStore.h"

//! Macro for CEventTimestampValidationHandler string
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CEventTimestampValidationHandler"

using ic_event::CIgniteEvent;
namespace ic_bl
{
CEventTimestampValidationHandler::CEventTimestampValidationHandler(
                                          CTransportHandlerBase *pNextHandler) :
                                          CTransportHandlerBase(pNextHandler),
                                          m_bInvalidEventsAvailableInDb(false)
{
    /* below initializations are used to correct invalid timestamps,
     * and also to determine timestamp for startup event.
     * Value 1704067200000 (01/01/2024) is used as a cutoff timestamp.
     */
    m_dblTimestampCutoff = 1704067200000.0;
    m_dblLastInvalidTimestamp = 0.0;
    m_dblFrstValidTimestamp = 0.0;
    m_nInitialEventQueueLimit = 100;

    ic_utils::Json::Value jsonRoot = 
                     ic_core::CIgniteConfig::GetInstance()->GetJsonValue("DAM");
    ic_utils::Json::Value jsonEvents = 
                              jsonRoot["TimestampValidatorExceptions"].isArray()
                              ? jsonRoot["TimestampValidatorExceptions"] : 
                              ic_utils::Json::Value::nullRef;
    LoadTSExceptionEvents(jsonEvents);
}

CEventTimestampValidationHandler::~CEventTimestampValidationHandler()
{
}

/*
"DAM": {
    "TimestampValidatorExceptions": [
        "AppRegistration",
        "RequestActivationId",
        "AppMsgAck",
        "IgniteClientLaunched",
        "ForceUpload"
    ]
}
*/
void CEventTimestampValidationHandler::LoadTSExceptionEvents(
                                  const ic_utils::Json::Value &rjsonTsExpEvents)
{
    if (rjsonTsExpEvents != ic_utils::Json::Value::nullRef)
    {
        for (int index = 0; index < rjsonTsExpEvents.size(); index++)
        {
            m_setExceptionEvents.insert(rjsonTsExpEvents[index].asString());
            HCPLOG_I << "exception event:" << rjsonTsExpEvents[index].asString();
        }
    }
}

void CEventTimestampValidationHandler::HandleEvent(
                                                 ic_core::CEventWrapper *pEvent)
{
    if (IsEventHasValidTS(pEvent))
    {
        ValidateTsOfEvent(pEvent);
    }
}

bool CEventTimestampValidationHandler::IsEventHasValidTS(
                                                 ic_core::CEventWrapper *pEvent)
{
    bool bIsTsPresent = false;
    try
    {
        double dblEventTimestamp = pEvent->GetTimestamp();
        bIsTsPresent = true;
    }
    catch (ic_utils::Json::LogicError ex)
    {
        // can't do anything if timestamp is not valid - FAILURE case
        HCPLOG_E << "Error in validating timestamp~" << pEvent->GetEventId();
        delete pEvent;
    }
    return bIsTsPresent;
}

void CEventTimestampValidationHandler::ValidateTsOfEvent(
                                                 ic_core::CEventWrapper *pEvent)
{
    std::string strEventID = pEvent->GetEventId();
    HCPLOG_D << "validating timestamp of event=" << strEventID;
    double dblEventTimestamp = pEvent->GetTimestamp();

    //let us find the first valid timestamp received
    if (m_dblFrstValidTimestamp == 0.0)
    {
        //Let exception events pass without validation.
        if(m_setExceptionEvents.find(strEventID) != m_setExceptionEvents.end())
        {
            HCPLOG_C << "exception event:"<<pEvent->GetEventId();
            HCPLOG_D << "process it without timestamp validation";
            m_pNextHandler->HandleEvent(pEvent);
            return;
        }

        ValidateEventTSBasedOnCutOff(pEvent);
    }
    else
    {
        HCPLOG_METHOD() << "Valid timestamp is seen already; let us proceed...";
        /* We have previously seen at least one valid timestamp.
         * Handle the m_listInitialEvents that are being sent back through after
         * finding the first real timestamp,
         * along with other events with invalid timestamps that might come in 
         * later.
         */
        FixAndSend(strEventID, dblEventTimestamp, pEvent);
    }
}

void CEventTimestampValidationHandler::ValidateEventTSBasedOnCutOff(
                                                 ic_core::CEventWrapper *pEvent)
{
    std::string strEventID = pEvent->GetEventId();
    double dblEventTimestamp = pEvent->GetTimestamp();

    /* Assume a timestamp is valid only if it is more than the cutoff.*/
    if (dblEventTimestamp < m_dblTimestampCutoff)
    {
        if (dblEventTimestamp > m_dblLastInvalidTimestamp)
        {
            m_dblLastInvalidTimestamp = dblEventTimestamp;
        }

        HCPLOG_W << "pushing the event " << strEventID 
                 << " into m_listInitialEvents queue";
        //let us push this event (with invalid timestamp) into the vector
        m_listInitialEvents.push_back(pEvent);

        HandleInvalidEventTS();
    }
    else /* Timestamp is greater than cutoff.*/
    {
        m_dblFrstValidTimestamp = dblEventTimestamp;
        HCPLOG_C << "Got valid timestamp" << m_dblFrstValidTimestamp;

        HandleFirstValidTs();

        /* This is the first valid timestamped event. 
         * Send the initial events back through the CacheTransport
         * So that they can get their timestamps fixed.
         */
        ValidateAndSendEvent(strEventID, pEvent);
    }
}

void CEventTimestampValidationHandler::HandleInvalidEventTS()
{
    /* we can't keep increasing the m_listInitialEvents 
     * (with invalid timestamp);
     * if it exceeds the limit, let them through.
     */
    if ((int)m_listInitialEvents.size() > m_nInitialEventQueueLimit)
    {
        ic_core::CIgniteConfig* pConfig = ic_core::CIgniteConfig::GetInstance();
        bool bStoreInvalidEvents = pConfig->GetBool(
                                        "DAM.Database.storeInvalidEvents",true);
        if(bStoreInvalidEvents)
        {
            HCPLOG_C << "InitialEvens Queue exceeded; pushing to database";
            CInvalidTimestampEventStore *pInvEvnStoreObj =
                                     CInvalidTimestampEventStore::GetInstance();
            m_bInvalidEventsAvailableInDb = true;
            pInvEvnStoreObj->InsertEvents(m_listInitialEvents);

        }
        else
        {
            HCPLOG_C << "InitialEvents queue exceeded~flush it out";
            while (!m_listInitialEvents.empty())
            {
                ic_core::CEventWrapper *pEv = m_listInitialEvents.front();
                ValidateAndSendEvent(pEv->GetEventId(), pEv);
                m_listInitialEvents.pop_front();
            }
        }
    }
}

void CEventTimestampValidationHandler::HandleFirstValidTs()
{
    if (!m_listInitialEvents.empty() || m_bInvalidEventsAvailableInDb)
    {
        if(m_bInvalidEventsAvailableInDb)
        {
            HCPLOG_D << "First clear the events from CInvalidTimestampEventStore";
            CInvalidTimestampEventStore *pInvEvnStoreObj = 
                                     CInvalidTimestampEventStore::GetInstance();
            int nRowsToprocess = 100;
            int nTotalRows = pInvEvnStoreObj->GetEventRowCount();
            while(nTotalRows > 0)
            {
                std::list<int> listRowIDs;
                std::list<std::string> listEventData;
                pInvEvnStoreObj->RetriveEvents(nRowsToprocess,
                                                  listRowIDs,listEventData);

                while (!listEventData.empty())
                {
                    std::string strEvent = listEventData.front();
                    HCPLOG_D << "Process to Fix >>" << strEvent;
                    ic_core::CEventWrapper *pInvalidevent = 
                                                   new ic_core::CEventWrapper();
                    pInvalidevent->JsonToEvent(strEvent);
                    FixAndSend(pInvalidevent->GetEventId(), 
                                  pInvalidevent->GetTimestamp(), pInvalidevent);

                    listEventData.pop_front();
                }

                pInvEvnStoreObj->DeleteEvents(listRowIDs);
                nTotalRows -= nRowsToprocess;
            }
        }

        HCPLOG_D << "Fix and clear the initialEvents queue";
        while (!m_listInitialEvents.empty())
        {
            ic_core::CEventWrapper *pEvt = m_listInitialEvents.front();
            FixAndSend(pEvt->GetEventId(), pEvt->GetTimestamp(), pEvt);
            m_listInitialEvents.pop_front();
        }
        m_bInvalidEventsAvailableInDb = false;
    }
    else
    {
        /* There were no events with invalid timestamps. 
         * We need to create the startup event here.
         */
        HCPLOG_D << "No initial events to fix timestamp~";
    }
}

void CEventTimestampValidationHandler::FixAndSend(const std::string &rstrEventId,
                                               const double &rdblEventTimestamp, 
                                               ic_core::CEventWrapper *pEvent)
{
    /* We have previously seen at least one valid timestamp.*/
    /* Handle the initialEvents that are being sent back through after finding 
     * the first real timestamp,
     * along with other events with invalid timestamps that might come in later.
     */
    if (rdblEventTimestamp < m_dblTimestampCutoff)
    {
        HCPLOG_D << "timestamp(" << rdblEventTimestamp 
                 << ") less than cutoff (" << rstrEventId << ")";

        /* If m_dblLastInvalidTimestamp still equals 0, then there were no 
         * initial events. If there were no initial events, then we don't know
         * what to do with this invalid timestamp.
         * We'll have to throw the event out. 
         */
        if (m_dblLastInvalidTimestamp != 0.0)
        {
            HCPLOG_D << "m_dblLastInvalidTimestamp(" 
                     << m_dblLastInvalidTimestamp 
                     << "); m_dblFrstValidTimestamp(" 
                     << m_dblFrstValidTimestamp << ")";

            int nDelta = m_dblLastInvalidTimestamp - rdblEventTimestamp;
            double dblNewTimestamp = m_dblFrstValidTimestamp - nDelta;
            pEvent->AddField("OriginalTimestamp", rdblEventTimestamp);
            pEvent->SetTimestamp(dblNewTimestamp);

            HCPLOG_C << "Event~" << rstrEventId << "-Timestamp after fix:" 
                     << dblNewTimestamp;
            ValidateAndSendEvent(rstrEventId, pEvent);
            return;
        }
        else
        {
            HCPLOG_E << "Cannot fix event received with invalid timestamp: " 
                     << rdblEventTimestamp;
            
            //delete event with invalid timestamp which cannot be fixed
            if (pEvent)
            {
                delete pEvent;
            }
            return;
        }
    }
    else
    {
        HCPLOG_D << "rdblEventTimestamp is valid; proceeding with the event...";
        ValidateAndSendEvent(rstrEventId, pEvent);
    }
}

void CEventTimestampValidationHandler::ValidateAndSendEvent(const std::string 
                                   &rstrEventId, ic_core::CEventWrapper *pEvent)
{

    HCPLOG_D << "event validation is complete... pass it to next handler." 
             << rstrEventId;
    if (rstrEventId.empty())
    {
        HCPLOG_E << "ignore event with empty ID: " << rstrEventId;
        //delete object here. other wise it will cause memory.
        delete pEvent;
        return;
    }

    //finally we've got the valid event; let it go thru next phase (ActiveDelay)
    m_pNextHandler->HandleEvent(pEvent);

    return;
}

bool CEventTimestampValidationHandler::IsEventTimestampValid(double &rdblTimestamp)
{
    return (rdblTimestamp >= m_dblTimestampCutoff) ? true : false;
}

}/* namespace ic_bl */