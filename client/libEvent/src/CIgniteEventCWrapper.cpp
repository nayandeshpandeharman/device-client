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

#include <string.h>
#include <stdio.h>
#include "CIgniteEvent.h"
#include "CIgniteEventCWrapper.h"

using ic_event::CIgniteEvent;

IgniteEvent* ievent_create(const char* pchVersion, const char* pchEventName)
{
    return reinterpret_cast<IgniteEvent*>(new CIgniteEvent(std::string(pchVersion), std::string(pchEventName)));
}

void ievent_addfield_char(IgniteEvent* pstIE, const char* pchFieldName, const char* pchFieldVal)
{
    (reinterpret_cast<CIgniteEvent*>(pstIE))->AddField((std::string(pchFieldName)), (std::string(pchFieldVal)));
}

void ievent_addfield_int(IgniteEvent* pstIE, const char* pchFieldName, const int nFieldVal)
{
    (reinterpret_cast<CIgniteEvent*>(pstIE))->AddField((std::string(pchFieldName)), ((const int)(nFieldVal)));
}

void ievent_addfield_double(IgniteEvent* pstIE, const char* pchFieldName, const double dblFieldVal)
{
    (reinterpret_cast<CIgniteEvent*>(pstIE))->AddField((std::string(pchFieldName)), ((const double)(dblFieldVal)));
}

void ievent_addfield_long(IgniteEvent* pstIE, const char* pchFieldName, const long long llFieldVal)
{
    (reinterpret_cast<CIgniteEvent*>(pstIE))->AddField((std::string(pchFieldName)), ((const long long)(llFieldVal)));
}

void ievent_addfield_bool(IgniteEvent* pstIE, const char* pchFieldName, const bool bFieldVal)
{
    (reinterpret_cast<CIgniteEvent*>(pstIE))->AddField((std::string(pchFieldName)), ((bFieldVal == true) ? true : false));
}

void ievent_addfield_rawjsonstring(IgniteEvent* pstIE, const char* pchFieldName, const char* pchFieldVal)
{
    (reinterpret_cast<CIgniteEvent*>(pstIE))->AddFieldAsRawJsonString((std::string(pchFieldName)), (std::string(pchFieldVal)));
}

void ievent_add_biz_transactionid(IgniteEvent* pstIE, const char* pchFieldVal)
{
    (reinterpret_cast<CIgniteEvent*>(pstIE))->AddBizTransaction((std::string(pchFieldVal)));
}

void ievent_add_messageid(IgniteEvent* pstIE, const char* pchFieldVal)
{
    (reinterpret_cast<CIgniteEvent*>(pstIE))->AddMessageId((std::string(pchFieldVal)));
}

void ievent_add_correlationid(IgniteEvent* pstIE, const char* pchFieldVal)
{
    (reinterpret_cast<CIgniteEvent*>(pstIE))->AddCorrelationId((std::string(pchFieldVal)));
}

void ievent_attachfile(IgniteEvent* pstIE, const char* pchFileType, const char* pchFilePath)
{
    (reinterpret_cast<CIgniteEvent*>(pstIE))->AddField(std::string(pchFileType), std::string(pchFilePath));
}

int ievent_send(IgniteEvent* pstIE)
{
    return (reinterpret_cast<CIgniteEvent*>(pstIE))->Send();
}

void ievent_delete(IgniteEvent* pstIE)
{
    delete (reinterpret_cast<CIgniteEvent*>(pstIE));
}

char* ievent_get_biz_transactionid(IgniteEvent* pstIE)
{
    char* pchResponse;
    std::string strRetVal = reinterpret_cast<CIgniteEvent*>(pstIE)->GetBizTransactionId();
    int nLen = strRetVal.size()+1;
    pchResponse = (char *) malloc ( nLen);
    if(pchResponse != NULL)
    {
        strncpy(pchResponse,strRetVal.c_str(),nLen-1);
        pchResponse[nLen-1] = '\0';
    }
    return pchResponse;
}

char* ievent_get_messageid(IgniteEvent* pstIE)
{
    char* pchResponse;
    std::string strRetVal = reinterpret_cast<CIgniteEvent*>(pstIE)->GetMessageId();
    int nLen = strRetVal.size()+1;
    pchResponse = (char *) malloc ( nLen);
    if(pchResponse != NULL)
    {
        strncpy(pchResponse,strRetVal.c_str(),nLen-1);
        pchResponse[nLen-1] = '\0';
    }
    return pchResponse;
}

char* ievent_get_correlationid(IgniteEvent* pstIE)
{
    char* pchResponse;
    std::string strRetVal = reinterpret_cast<CIgniteEvent*>(pstIE)->GetCorrelationId();
    int nLen = strRetVal.size()+1;
    pchResponse = (char *) malloc ( nLen);
    if(pchResponse != NULL)
    {
        strncpy(pchResponse,strRetVal.c_str(),nLen-1);
        pchResponse[nLen-1] = '\0';
    }
    return pchResponse;
}

bool ievent_get_bool(IgniteEvent* pstIE , const char* pchFieldName)
{
    if(reinterpret_cast<CIgniteEvent*>(pstIE)->GetBool(std::string(pchFieldName)))
    {
        return true;
    } 
    else 
    {
        return false;
    }
}

int ievent_get_int(IgniteEvent* pstIE,const char* pchFieldName)
{
    return reinterpret_cast<CIgniteEvent*>(pstIE)->GetInt(std::string(pchFieldName));
}

char* ievent_get_string(IgniteEvent* pstIE,const char* pchFieldName)
{
    char* pchResponse;
    std::string strRetVal = reinterpret_cast<CIgniteEvent*>(pstIE)->GetString(std::string(pchFieldName));
    int nLen = strRetVal.size()+1;
    pchResponse = (char *) malloc ( nLen);
    if(pchResponse != NULL)
    {
        strncpy(pchResponse,strRetVal.c_str(),nLen-1);
        pchResponse[nLen-1] = '\0';
    }
    return pchResponse;
}

double ievent_get_double(IgniteEvent* pstIE,const char* pchFieldName)
{
    return reinterpret_cast<CIgniteEvent*>(pstIE)->GetDouble(std::string(pchFieldName));
}

long long ievent_get_long(IgniteEvent* pstIE,const char* pchFieldName)
{
    return reinterpret_cast<CIgniteEvent*>(pstIE)->GetLong(std::string(pchFieldName));
}

char* ievent_get_eventid(IgniteEvent* pstIE)
{
    char* pchResponse;
    std::string strRetVal = reinterpret_cast<CIgniteEvent*>(pstIE)->GetEventId();
    int nLen = strRetVal.size()+1;
    pchResponse = (char *) malloc ( nLen);
    if(pchResponse != NULL)
    {
        strncpy(pchResponse,strRetVal.c_str(),nLen-1);
        pchResponse[nLen-1] = '\0';
    }
    return pchResponse;
}

double ievent_get_timestamp(IgniteEvent* pstIE)
{
    return reinterpret_cast<CIgniteEvent*>(pstIE)->GetTimestamp();
}

char* ievent_get_version(IgniteEvent* pstIE)
{
    char* pchResponse;
    std::string strRetVal = reinterpret_cast<CIgniteEvent*>(pstIE)->GetVersion();
    int nLen = strRetVal.size()+1;
    pchResponse = (char *) malloc ( nLen);
    if(pchResponse != NULL)
    {
        strncpy(pchResponse,strRetVal.c_str(),nLen-1);
        pchResponse[nLen-1] = '\0';
    }
    return pchResponse;
}

char* ievent_eventtojsonstring(IgniteEvent* pstIE)
{
    char *jsonStr;
    std::string strRetVal;

    reinterpret_cast<CIgniteEvent*>(pstIE)->EventToJson(strRetVal);
    int nLen = strRetVal.size()+1;
    jsonStr = (char *) malloc ( nLen);
    if(jsonStr != NULL)
    {
        strncpy(jsonStr,strRetVal.c_str(),nLen-1);
        jsonStr[nLen-1] = '\0';
    }
    return jsonStr;
}

IgniteEvent* ievent_jsontoevent(const char* pchJsonEvent)
{
    ic_utils::Json::Value jsonRoot;
    ic_utils::Json::Reader jsonReader;
    if(jsonReader.parse(pchJsonEvent, jsonRoot))
    {
        CIgniteEvent *event = new CIgniteEvent();
        try 
        {
            event->JsonToEvent(pchJsonEvent);
            return reinterpret_cast<IgniteEvent*>(event);
        }
        catch(...)
        {
            // jsonToEvent function will try to access ic_event::CIgniteEvent fields
            // if any of the fields not present then it will throw an exception
            // this will denote that input string is valid json but
            // not a valid json event
            delete event;
        }
    }
    return NULL;
}