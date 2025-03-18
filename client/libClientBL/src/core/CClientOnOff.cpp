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

#include <unistd.h>
#include "CClientOnOff.h"
#include "CIgniteLog.h"
#include "CIgniteClient.h"

//! Macro for 'CClientOnOff' string
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CClientOnOff"

namespace ic_bl
{

CClientOnOff* CClientOnOff::GetInstance()
{
    static CClientOnOff onoff;
    return &onoff;
}

CClientOnOff::CClientOnOff()
{
    int nOrder = 0;
    std::list <NotifReceiverCode> listNotifReceiver;

    listNotifReceiver.push_back(eR_MESSAGE_CONTROLLER);
    listNotifReceiver.push_back(eR_MQTT_CLIENT);
    listNotifReceiver.push_back(eR_MQTT_UPLOADER);
    listNotifReceiver.push_back(eR_NOTIFICATION_LISTENER);
    listNotifReceiver.push_back(eR_UPLOAD_CONTROLLER);
    #ifdef IC_UNIT_TEST
        listNotifReceiver.push_back(eR_UT_THREAD);
    #endif

    m_mapNotifOrder.insert(std::make_pair(++nOrder, listNotifReceiver));

    listNotifReceiver.clear();
    listNotifReceiver.push_back(eR_MESSAGE_QUEUE);
    listNotifReceiver.push_back(eR_MID_HANDLER);
    m_mapNotifOrder.insert(std::make_pair(++nOrder, listNotifReceiver));

    listNotifReceiver.clear();
    listNotifReceiver.push_back(eR_CACHE_TRANSPORT);
    m_mapNotifOrder.insert(std::make_pair(++nOrder, listNotifReceiver));

    listNotifReceiver.clear();
    listNotifReceiver.push_back(eR_DB_TRANSPORT);
    listNotifReceiver.push_back(eR_ZMQ_RECEIVE_MESSAGE);
    listNotifReceiver.push_back(eR_PRODUCT_IMPL_CONTROLLER);
    m_mapNotifOrder.insert(std::make_pair(++nOrder, listNotifReceiver));

    //START: test code to print the notification order
    for (std::map<int, std::list <NotifReceiverCode>>::iterator iterNotif = 
        m_mapNotifOrder.begin(); iterNotif != m_mapNotifOrder.end(); 
        iterNotif++)
    {
        for (NotifReceiverCode eNotifCode : iterNotif->second)
        {
            HCPLOG_I << "Order #" << iterNotif->first << " - " << 
                                                    GetReceiverName(eNotifCode);
        }
        HCPLOG_I << "\n";
    }
    //END: test code to print the notification order
}

CClientOnOff::~CClientOnOff()
{

}

bool CClientOnOff::RegisterForShutdownNotification(
    ic_core::IOnOffNotificationReceiver* pRcvr, NotifReceiverCode eRcvrCode, 
                                                    std::string strRcvrName)
{
    m_onoffMutex.Lock();
    bool bIsRegistered = false;
    if (NULL == pRcvr)
    {
        HCPLOG_E << "Code: " << eRcvrCode << " - NULL receiver reference!";
        std::cout << "CClientOnOff::registerForShutdownNotification-"<< "Code: "
                    << eRcvrCode << " - NULL receiver reference!" << std::endl;
    }
    else
    {
        if (eRcvrCode == eR_OTHER)
        {
            if (m_mapNonDefinedReceivers.end() != 
                                    m_mapNonDefinedReceivers.find(strRcvrName))
            {
                HCPLOG_E << "NonDefined Receiver: " << eRcvrCode << "-" << 
                        strRcvrName << " registered already! Replacing it...";

                std::cout << "CClientOnOff::registerForShutdownNotification-"<< 
                    "NonDefined Receiver: " << eRcvrCode << "-" << strRcvrName
                    << " registered already! Replacing it..." << std::endl;
            }

            ReceiverDetail recvrDetail;

            recvrDetail.pRcvrRef = pRcvr;
            recvrDetail.eStatus = SubscribeStatus::eS_SUBSCRIBED;
            m_mapNonDefinedReceivers.insert(std::make_pair(strRcvrName, 
                                                            recvrDetail));

            HCPLOG_I << "Non-Defined Receiver Subscribed..." << strRcvrName <<
                "-" << pRcvr << ". NDR-Cnt~" << m_mapNonDefinedReceivers.size();
            std::cout << "CClientOnOff::registerForShutdownNotification-"<< 
                    "Non-Defined Receiver Subscribed..." << strRcvrName << "-"
                    << pRcvr << ". NDR-Cnt~" << m_mapNonDefinedReceivers.size()
                    << std::endl;

        }
        else
        {
            std::string strRecvrName = GetReceiverName(eRcvrCode);
            if (m_mapReceivers.end() != m_mapReceivers.find(eRcvrCode))
            {
                HCPLOG_E << "Receiver " << eRcvrCode << "-" << strRecvrName << 
                            " registered already! Replacing it...";

                std::cout << "CClientOnOff::registerForShutdownNotification-"<< 
                "Receiver " << eRcvrCode << "-" << strRecvrName << 
                " registered already! Replacing it..." << std::endl;
            }

            ReceiverDetail recvrDetail;
            recvrDetail.pRcvrRef = pRcvr;
            recvrDetail.eStatus = SubscribeStatus::eS_SUBSCRIBED;
            m_mapReceivers.insert(std::make_pair(eRcvrCode, recvrDetail));
            HCPLOG_I << "Subscribed..." << eRcvrCode << "-" << strRecvrName << 
                "-" << pRcvr << ". Cnt~" << m_mapReceivers.size();
            std::cout << "CClientOnOff::registerForShutdownNotification-"<< 
                    "Subscribed..." << eRcvrCode << "-" << strRecvrName << "-"
                    << pRcvr << ". Cnt~" << m_mapReceivers.size() << std::endl;

        }
        bIsRegistered = true;
    }
    m_onoffMutex.Unlock();
    return bIsRegistered;
}

bool CClientOnOff::UnregisterForShutdownNotification(
                                                    NotifReceiverCode eRcvrCode,
                                                        std::string strRcvrName)
{
    HCPLOG_METHOD() << eRcvrCode;
    if (eRcvrCode == eR_OTHER)
    {
        std::map<std::string, ReceiverDetail>::iterator iterMap = 
                                    m_mapNonDefinedReceivers.find(strRcvrName);

        if (iterMap != m_mapNonDefinedReceivers.end())
        {
            HCPLOG_D << "Updating status to UNSUB " << eRcvrCode << "-" 
                                                                << strRcvrName;
            std::cout << "CClientOnOff::unregisterForShutdownNotification-"<<
            "Updating status to UNSUB " << eRcvrCode << "-" << strRcvrName 
                                                                << std::endl;

            ReceiverDetail recvrDetail = iterMap->second;
            recvrDetail.eStatus = SubscribeStatus::eS_UNSUBSCRIBED;
            iterMap->second = recvrDetail;
            return true;
        }
        else
        {
            HCPLOG_E << "Non-Defined Receiver: " << eRcvrCode << "-" << 
                                                strRcvrName << " not found!";
            std::cout << "CClientOnOff::unregisterForShutdownNotification-"<< 
            "Non-Defined Receiver: " << eRcvrCode << "-" << strRcvrName << 
            " not found!" << std::endl;

            return false;
        }
    }
    else
    {
        std::string strRecvrName = GetReceiverName(eRcvrCode);
        std::map<NotifReceiverCode, ReceiverDetail>::iterator iterMap = 
                                                m_mapReceivers.find(eRcvrCode);
        if (iterMap != m_mapReceivers.end())
        {
            HCPLOG_D << "Updating status to UNSUB " << eRcvrCode << "-" << 
                                                    GetReceiverName(eRcvrCode);
            std::cout << "CClientOnOff::unregisterForShutdownNotification-"<< 
            "Updating status to UNSUB " << eRcvrCode << "-" <<
            GetReceiverName(eRcvrCode) << std::endl;

            ReceiverDetail recvrDetail = iterMap->second;
            recvrDetail.eStatus = SubscribeStatus::eS_UNSUBSCRIBED;
            iterMap->second = recvrDetail;
            return true;
        }
        else
        {
            HCPLOG_E << "Receiver " << eRcvrCode << "-" << strRecvrName 
                                                            << " not found!";
            std::cout << "CClientOnOff::unregisterForShutdownNotification-"<< 
            "Receiver " << eRcvrCode << "-" << strRecvrName << " not found!"
            << std::endl;

            return false;
        }
    }
}

bool CClientOnOff::ReadyForShutdown(NotifReceiverCode eRcvrCode, 
                                                    std::string strRcvrName)
{
    HCPLOG_METHOD() << eRcvrCode;
    if (eRcvrCode == eR_OTHER)
    {
        std::map<std::string, ReceiverDetail>::iterator iterMap = 
                                    m_mapNonDefinedReceivers.find(strRcvrName);

        if (iterMap != m_mapNonDefinedReceivers.end())
        {
            HCPLOG_I << "Updating Non-Defined Receiver status.." << eRcvrCode 
                                                        << "-" << strRcvrName;
            std::cout << "CClientOnOff::readyForShutdown-"<< 
            "Updating Non-Defined Receiver status.." << eRcvrCode << "-" 
            << strRcvrName << std::endl;

            ReceiverDetail recvrDetail = iterMap->second;
            recvrDetail.eStatus = SubscribeStatus::eS_SHUTDOWN_COMPLETED;
            iterMap->second = recvrDetail;
            return true;
        }
        else
        {
            HCPLOG_E << "Non-Defined Receiver: " << eRcvrCode << "-"
            << strRcvrName << " not found!";
            std::cout << "CClientOnOff::readyForShutdown-"
            << "Non-Defined Receiver: " << eRcvrCode << "-"
            << strRcvrName << " not found!" << std::endl;
            return false;
        }
    }
    else
    {
        std::string strRecvrName = GetReceiverName(eRcvrCode);

        std::map<NotifReceiverCode, ReceiverDetail>::iterator iterMap =
                                                m_mapReceivers.find(eRcvrCode);

        if (iterMap != m_mapReceivers.end())
        {
            ReceiverDetail recvrDetail = iterMap->second;
            recvrDetail.eStatus = SubscribeStatus::eS_SHUTDOWN_COMPLETED;
            iterMap->second = recvrDetail;
            HCPLOG_I << "Status updated.." << eRcvrCode << "-" << strRecvrName;
            std::cout << "CClientOnOff::readyForShutdown-"<< "Status updated.."
            << eRcvrCode << "-" << strRecvrName << std::endl;
            return true;
        }
        else
        {
            HCPLOG_E << "Receiver " << eRcvrCode << "-" << strRecvrName 
            << " not found!";
            std::cout << "CClientOnOff::readyForShutdown-"<< "Receiver " 
            << eRcvrCode << "-" << strRecvrName << " not found!" << std::endl;
            return false;
        }
    }
}

std::string CClientOnOff::GetReceiverName(NotifReceiverCode eRcvrCode)
{
    switch(eRcvrCode)
    {
        case eR_CACHE_TRANSPORT:
            return "CacheTransport";

        case eR_DB_TRANSPORT:
            return "DBTransport";

        case eR_MESSAGE_QUEUE:
            return "MessageQueue";

        case eR_MESSAGE_CONTROLLER:
            return "MessageController";

        case eR_MQTT_CLIENT:
            return "MQTTClient";

        case eR_MQTT_UPLOADER:
            return "MQTTUploader";

        case eR_MID_HANDLER:
            return "MidHandler";

        case eR_NOTIFICATION_LISTENER:
            return "NotificationListener";

        case eR_PRODUCT_IMPL_CONTROLLER:
            return "ProductImplController";

        case eR_UPLOAD_CONTROLLER:
            return "UploadController";

        case eR_ZMQ_RECEIVE_MESSAGE:
            return "ZMQReceiveMessage";

        #ifdef IC_UNIT_TEST
        case eR_UT_THREAD:
            return "UnitTestThread";
        #endif 

        default:
            return "NameNotFound!";
    }//switch
}

void CClientOnOff::Run()
{
    HCPLOG_METHOD() ;

    /* If at all shutdown is initiated immediately after Client is started,
     * some time is required for threads to complete the initialization
     * of subscribing with CClientOnOff hence starting this thread with
     * some initial delay.
     */
    sleep(5);

    while (true)
    {
        //if any non-defined receivers exist, notify them first.
        NotifyNonDefinedOnOffRcvrs();

        //notify pre-defined receivers
        NotifyPreDefinedOnOffRcvrs();

        HCPLOG_I << "CHECKING SHUTDOWN COMPLETE STATUS...";
        std::cout << "CClientOnOff::run-CHECKING SHUTDOWN COMPLETE STATUS..." 
                  << std::endl;
        
        // Check if shutdown is completed by all non-defined subscribers.
        bool bIsNDShutdownComplete = CheckStatusOfNonDefinedRcvrs();

        // Check if shutdown is completed by all pre-defined subscribers.
        bool bIsPDShutdownComplete = CheckStatusOfPreDefinedRcvrs();

#if 0
        /* Below section is just a debug code which will help to
         * to print the current status of all the OnOff subscribers
         */
        HCPLOG_I << "OnOff-Status-BEGIN";`
        std::cout << "OnOff-Status-BEGIN" << std::endl;

        //print receiver status based on notification order
        for (std::map<int, std::list <NotifReceiverCode>>::iterator iterNotif =
                     m_mapNotifOrder.begin(); 
                     iterNotif != m_mapNotifOrder.end(); iterNotif++)
        {
            for (NotifReceiverCode eNotifCode : iterNotif->second)
            {
                std::map<NotifReceiverCode, ReceiverDetail>::iterator
                            iterRecvrDetail = m_mapReceivers.find(eNotifCode);
                if (iterRecvrDetail != m_mapReceivers.end())
                {
                    ReceiverDetail stRcvrsDetail = iterRecvrDetail->second;
                    HCPLOG_T << iterRecvrDetail->first << "-" <<
                                GetReceiverName(iterRecvrDetail->first) 
                             << " -> " << stRcvrsDetail.eStatus;
                    std::cout << "CClientOnOff::run-" << iterRecvrDetail->first 
                    << "-" \ << GetReceiverName(iterRecvrDetail->first) 
                    << " -> " << stRcvrsDetail.eStatus << std::endl;
                }
            }
        }

       //print all receiver status
       for (std::map<NotifReceiverCode, 
            ReceiverDetail>::iterator iterNotif = m_mapReceivers.begin(); 
            iterNotif != m_mapReceivers.end(); iterNotif++)
       {
           ReceiverDetail stRcvrsDetail = iterNotif->second;
           HCPLOG_T << iterNotif->first << "-" << 
           GetReceiverName(iterNotif->first) << " -> " << stRcvrsDetail.eStatus;
       }

        HCPLOG_I << "OnOff-Status-END";
        std::cout << "OnOff-Status-END" << std::endl;
        // Debug code end
#endif

        if (bIsNDShutdownComplete && bIsPDShutdownComplete)
        {
            HCPLOG_I << "Notif complete... breaking...";
            std::cout << "CClientOnOff::run-Notif complete... breaking..." 
                      << std::endl;
            break;
        }
        sleep(1);
    }
    Detach();
    ic_core::CIgniteClient::CompleteShutdown();
}

void CClientOnOff::NotifyNonDefinedOnOffRcvrs()
{
    for (std::map<std::string, ReceiverDetail>::iterator iterMap = 
         m_mapNonDefinedReceivers.begin(); iterMap != 
         m_mapNonDefinedReceivers.end();
         iterMap++)
    {
        ReceiverDetail recvrDetail = iterMap->second;
        if (eS_SUBSCRIBED == recvrDetail.eStatus)
        {
            if (recvrDetail.pRcvrRef) 
            {
                HCPLOG_I << "Notifying non-defined receiver: " << 
                                                                iterMap->first;
                std::cout << "CClientOnOff::run-Notifying non-defined receiver: " 
                          << iterMap->first << std::endl;
                recvrDetail.eStatus = SubscribeStatus::eS_NOTIFIED;
                iterMap->second = recvrDetail;

                recvrDetail.pRcvrRef->NotifyShutdown();
            }
            else 
            {
                HCPLOG_E << "Null receiver ref! " << iterMap->first;
                std::cout << "CClientOnOff::run-Null receiver ref! " 
                          << iterMap->first << std::endl;
            }
        }
        else
        {
            //do nothing
        }
    }//end of for loop
}

void CClientOnOff::NotifyPreDefinedOnOffRcvrs()
{
    for (std::map<int, std::list <NotifReceiverCode>>::iterator iterNotif = 
         m_mapNotifOrder.begin(); iterNotif != m_mapNotifOrder.end(); 
         iterNotif++)
    {
        bool bCanNotifyNext = true;
        for (NotifReceiverCode eNotifCode : iterNotif->second)
        {
            std::map<NotifReceiverCode , ReceiverDetail>::iterator iterReceiver= 
                                                m_mapReceivers.find(eNotifCode);

            if (iterReceiver != m_mapReceivers.end())
            {
                ReceiverDetail recvrDetail = iterReceiver->second;
                if (eS_SUBSCRIBED == recvrDetail.eStatus)
                {
                    HCPLOG_I << "Notifying " << iterReceiver->first 
                             << "-" << GetReceiverName(iterReceiver->first);
                    
                    std::cout << "CClientOnOff::run-Notifying " 
                              << iterReceiver->first << "-" 
                              << GetReceiverName(iterReceiver->first) 
                              << std::endl;

                    recvrDetail.eStatus = SubscribeStatus::eS_NOTIFIED;
                    iterReceiver->second = recvrDetail;
                    recvrDetail.pRcvrRef->NotifyShutdown();
                
                    bCanNotifyNext = false;
                }
                else if (eS_NOTIFIED == recvrDetail.eStatus)
                {
                    bCanNotifyNext = false;
                }
                else
                {
                    HCPLOG_I << GetReceiverName(iterReceiver->first) 
                             << " - already in state " << recvrDetail.eStatus;
                    
                    std::cout << "CClientOnOff::run-" 
                              << GetReceiverName(iterReceiver->first) 
                              << " - already in state "
                              << recvrDetail.eStatus << std::endl;
                }
            }
        }
        if (!bCanNotifyNext) 
        {
            break;
        }
    } //end of for loop
}

bool CClientOnOff::CheckStatusOfNonDefinedRcvrs()
{
    bool bIsShutdownComplete = true;

    for (std::map<std::string, ReceiverDetail>::iterator iterMap = 
    m_mapNonDefinedReceivers.begin(); iterMap != m_mapNonDefinedReceivers.end();
        iterMap++)
    {
        ReceiverDetail recvrDetail = iterMap->second;
        if ((recvrDetail.eStatus == eS_SHUTDOWN_COMPLETED) 
             || (recvrDetail.eStatus == eS_UNSUBSCRIBED))
        {
            HCPLOG_I << "Shutdown completed - " << iterMap->first 
                     << "-" << recvrDetail.eStatus;
            std::cout << "CClientOnOff::run-Shutdown completed - " 
                      << iterMap->first << "-" << recvrDetail.eStatus 
                      << std::endl;
        }
        else
        {
            HCPLOG_I << "Waiting for shutdown complete from non-defined \
                    receiver: " << iterMap->first << "-" << recvrDetail.eStatus;

            std::cout << "CClientOnOff::run-Waiting for shutdown complete \
                       from non-defined receiver: " << iterMap->first 
                       << "-" << recvrDetail.eStatus << std::endl;

            bIsShutdownComplete = false;
        }
    }//End of for loop

    return bIsShutdownComplete;
}

bool CClientOnOff::CheckStatusOfPreDefinedRcvrs()
{
    bool bIsShutdownComplete = true;

    for (std::map<int, std::list <NotifReceiverCode>>::iterator iterMapNotList 
        = m_mapNotifOrder.begin(); iterMapNotList != m_mapNotifOrder.end(); 
        iterMapNotList++)
    {
        HCPLOG_I << "Order: " << iterMapNotList->first;
        for (NotifReceiverCode eNotifCode : iterMapNotList->second)
        {
            std::map<NotifReceiverCode, ReceiverDetail>::iterator iterMap 
                                            = m_mapReceivers.find(eNotifCode);
            if (iterMap != m_mapReceivers.end())
            {
                ReceiverDetail recvrDetail = iterMap->second;
                if ((recvrDetail.eStatus == eS_SHUTDOWN_COMPLETED) 
                   || (recvrDetail.eStatus == eS_UNSUBSCRIBED)) 
                {
                    HCPLOG_I << "Shutdown completed - " << iterMap->first 
                             << "-" << recvrDetail.eStatus;
                    std::cout << "CClientOnOff::run-Shutdown completed - " 
                              << iterMap->first << "-" << recvrDetail.eStatus 
                              << std::endl;
                }
                else 
                {
                    HCPLOG_I << "Waiting for shutdown complete from " 
                             << iterMap->first << "-" << recvrDetail.eStatus;
                    std::cout << "CClientOnOff::run-Waiting for shutdown \
                    complete from " << iterMap->first << "-" << 
                    recvrDetail.eStatus << std::endl;

                    bIsShutdownComplete = false;
                }
            }
        }
        HCPLOG_I << "-----------------------------------";
        std::cout << "CClientOnOff::run------------------------------------" 
                  << std::endl;
    }//End of for loop

    return bIsShutdownComplete;
}
}
