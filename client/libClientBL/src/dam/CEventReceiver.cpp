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
#include "dam/CEventReceiver.h"
#include "dam/CCacheTransport.h"
#include "CIgniteMessage.h"
#include "CMessageQueue.h"
#include "CIgniteConfig.h"
#include "CIgniteFileUtils.h"
#include "CIgniteLog.h"

namespace ic_bl
{
CEventReceiver::CEventReceiver(ic_core::CMessageQueue* pPublisher, bool bReceiverSuspended)
{
    HCPLOG_METHOD();
    m_pPublisher = pPublisher;
    m_bReceiverSuspended = bReceiverSuspended;
    
    if (pPublisher)
    {
        pPublisher->Subscribe(ic_event::CMessageTypes::eEVENT, this);
    }
    
    CCacheTransport::GetInstance();
}

CEventReceiver::~CEventReceiver()
{
    if (m_pPublisher)
    {
        m_pPublisher->Unsubscribe(ic_event::CMessageTypes::eEVENT, this);
    }
}

void CEventReceiver::SuspendReceiver()
{
    m_bReceiverSuspended = true;
}

void CEventReceiver::ResumeReceiver()
{
    m_bReceiverSuspended = false;
}

bool CEventReceiver::Handle(const ic_event::CIgniteMessage& rMsg)
{
    bool bEvntProcessed = false;
    static std::string strAttachmentPath = 
                               ic_core::CIgniteConfig::GetInstance()->GetString(
                               "DAM.Upload.CurlSender.fileAttachmentPath");

    HCPLOG_T << "CEventReceiver::send() received message: " 
             << rMsg.GetMessageAsString();
    if (!m_bReceiverSuspended)
    {
        CCacheTransport::GetInstance()->Send(rMsg.GetMessageAsString());
        bEvntProcessed = true;
    }
    else
    {
        ic_core::CEventWrapper* pEvent = new ic_core::CEventWrapper();
        pEvent->JsonToEvent(rMsg.GetMessageAsString());

        HCPLOG_T << "CEventReceiver not active: ignoring message: " 
                 << rMsg.GetMessageAsString();

        //while ignoring, if the event has any attachments, delete them
        std::vector<std::string> vectorAttachments = pEvent->GetAttachments();
        for(unsigned int i = 0; i < vectorAttachments.size(); i++)
        {
            //remove the attachment files from the local drive.
            std::string strFileToDelete = strAttachmentPath + "/" +
                                          vectorAttachments[i];
            ic_utils::CIgniteFileUtils::Remove(strFileToDelete);
        }

        delete pEvent;
    }

    return bEvntProcessed;
}
}/* namespace ic_bl */