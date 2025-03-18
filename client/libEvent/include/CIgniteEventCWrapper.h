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

#ifdef __cplusplus
extern "C" 
{
#endif

typedef struct IgniteEvent IgniteEvent;

//! Event wrapper functions
/**
 * Method to create Ignite Event based on version and event name
 * @param[in] pchVersion Version of the event
 * @param[in] pchEventName Name of the event
 * @return Pointer to struct Ignite Event
 */
IgniteEvent* ievent_create(const char* pchVersion, const char* pchEventName);

/**
 * Method to add given key-value pair to the event
 * @param[in] pstIE Pointer to struct IgniteEvent
 * @param[in] pchFieldName Key name
 * @param[in] pchFieldVal Value of type char
 * @return void
 */
void ievent_addfield_char(IgniteEvent* pstIE, const char* pchFieldName, const char* pchFieldVal);

/**
 * Method to add given key-value pair to the event
 * @param[in] pstIE Pointer to struct IgniteEvent
 * @param[in] pchFieldName Key name 
 * @param[in] nFieldVal Value of type int
 * @return void
 */
void ievent_addfield_int(IgniteEvent* pstIE, const char* pchFieldName, const int nFieldVal);

/**
 * Method to add given key-value pair to the event
 * @param[in] pstIE Pointer to struct IgniteEvent
 * @param[in] pchFieldName Key name 
 * @param[in] dblFieldVal Value of type double
 * @return void
 */
void ievent_addfield_double(IgniteEvent* pstIE, const char* pchFieldName, const double dblFieldVal);

/**
 * Method to add given key-value pair to the event
 * @param[in] pstIE Pointer to struct IgniteEvent
 * @param[in] pchFieldName Key name 
 * @param[in] llFieldVal Value of type long long 
 * @return void
 */
void ievent_addfield_long(IgniteEvent* pstIE, const char* pchFieldName, const long long llFieldVal);

/**
 * Method to add given key-value pair to the event
 * @param[in] pstIE Pointer to struct IgniteEvent
 * @param[in] pchFieldName Key name 
 * @param[in] bFieldVal Value of type bool 
 * @return void
 */
void ievent_addfield_bool(IgniteEvent* pstIE, const char* pchFieldName, const bool bFieldVal);

/**
 * Method to add given key-value pair as json string to the event
 * @param[in] pstIE Pointer to struct IgniteEvent
 * @param[in] pchFieldName Key name 
 * @param[in] pchFieldVal Value of type json string 
 * @return void
 */
void ievent_addfield_rawjsonstring(IgniteEvent* pstIE, const char* pchFieldName, const char* pchFieldVal);

/**
 * Method to add biz transaction ID to the event
 * @param[in] pstIE Pointer to struct IgniteEvent
 * @param[in] pchFieldVal Field value is BizTransaction-ID 
 * @return void
 */
void ievent_add_biz_transactionid(IgniteEvent* pstIE, const char* pchFieldVal);

/**
 * Method to add message-id to the event
 * @param[in] pstIE Pointer to struct IgniteEvent 
 * @param[in] pchFieldVal Field value is Message-ID 
 * @return void
 */
void ievent_add_messageid(IgniteEvent* pstIE, const char* pchFieldVal);

/**
 * Method to add correlation-id to the event
 * @param[in] pstIE Pointer to struct IgniteEvent 
 * @param[in] pchFieldVal Field value is Correclation-ID  
 * @return void
 */
void ievent_add_correlationid(IgniteEvent* pstIE, const char* pchFieldVal);

/**
 * Method to attach an file to the event
 * @param[in] pstIE Pointer to struct IgniteEvent 
 * @param[in] pchFileType Attachment file name
 * @param[in] pchFilePath Attachment file path
 * @return void
 */
void ievent_attachfile(IgniteEvent* pstIE, const char* pchFileType, const char* pchFilePath);

/**
 * Method to send the event to the receiver
 * @param[in] pstIE Pointer to struct IgniteEvent
 * @return 0 if sending event is successful else -1 
 */
int ievent_send(IgniteEvent* pstIE);

/**
 * Method to delete Ignite Event pointer
 * @param[in] pstIE Pointer to struct IgniteEvent 
 * @return void
 */
void ievent_delete(IgniteEvent* pstIE);

/**
 * Method to get the biz transaction ID from the event
 * @param[in] pstIE Pointer to struct IgniteEvent 
 * @return Biztransaction-ID if filed in event is present else empty string
 * @note Pointer returned by following Get funtions needs to be freed by calling application, Also Do NULL check in case if malloc fails
 */
char* ievent_get_biz_transactionid(IgniteEvent* pstIE);

/**
 * Method to get the message-id from the event
 * @param[in] pstIE Pointer to struct IgniteEvent 
 * @return Message-ID if field in event is present else empty string
 * @note Pointer returned by following Get funtions needs to be freed by calling application, Also Do NULL check in case if malloc fails
 */
char* ievent_get_messageid(IgniteEvent* pstIE);

/**
 * Method to get the correlation-id from the event
 * @param[in] pstIE Pointer to struct IgniteEvent 
 * @return Correlation-ID if field in event is present else empty string
 * @note Pointer returned by following Get funtions needs to be freed by calling application, Also Do NULL check in case if malloc fails
 */
char* ievent_get_correlationid(IgniteEvent* pstIE);

/**
 * Method to get the value of the given key from the event
 * @param[in] pstIE Pointer to struct IgniteEvent 
 * @param[in] pchFieldName Key name 
 * @return Pointer to string if field name is present else empty string
 * @note Pointer returned by following Get funtions needs to be freed by calling application, Also Do NULL check in case if malloc fails
 */
char* ievent_get_string(IgniteEvent* pstIE,const char* pchFieldName);

/**
 * Method to get the value of the given key from the event
 * @param[in] pstIE Pointer to struct IgniteEvent 
 * @param[in] pchFieldName Key name 
 * @return True if field name exists else false
 */
bool ievent_get_bool(IgniteEvent* pstIE , const char* pchFieldName);

/**
 * Method to get the value of the given key from the event
 * @param[in] pstIE Pointer to struct IgniteEvent 
 * @param[in] pchFieldName Key name 
 * @return Integer value if key is member else 0
 */
int ievent_get_int(IgniteEvent* pstIE,const char* pchFieldName);

/**
 * Method to get the value of the given key from the event
 * @param[in] pstIE Pointer to struct IgniteEvent 
 * @param[in] pchFieldName Key name 
 * @return long long if key is member else 0
 */
long long ievent_get_long(IgniteEvent* pstIE,const char* pchFieldName);

/**
 * Method to get the value of the given key from the event
 * @param[in] pstIE Pointer to struct IgniteEvent 
 * @param[in] pchFieldName Key name 
 * @return double if key is member else 0.0
 */
double ievent_get_double(IgniteEvent* pstIE,const char* pchFieldName);

/**
 * Method to convert the given event to json format and load it into the event
 * @param[in] pchJsonEvent Event of type char*
 * @return  Pointer of struct IgniteEvent if parsing successful else null
 */
IgniteEvent* ievent_jsontoevent(const char* pchJsonEvent);

/**
 * Method to convert the event to a json value
 * @param[in] pstIE Pointer to struct IgniteEvent
 * @return Pointer if event is found else null
 * @note Pointer returned by following Get funtions needs to be freed by calling application, Also Do NULL check in case if malloc fails
 */
char* ievent_eventtojsonstring(IgniteEvent* pstIE);

/**
 * Method to get the event-id from the event
 * @param[in] pstIE Pointer to struct IgniteEvent
 * @return EventId of the event
 * @note Pointer returned by following Get funtions needs to be freed by calling application, Also Do NULL check in case if malloc fails
 */
char* ievent_get_eventid(IgniteEvent* pstIE);

/**
 * Method to get timestamp from the event
 * @param[in] pstIE Pointer to struct IgniteEvent 
 * @return Timestamp of the event
 */
double ievent_get_timestamp(IgniteEvent* pstIE);

/**
 * Method to get the timestamp from the event
 * @param[in] pstIE Pointer to struct IgniteEvent 
 * @return Version of the event
 * @note Pointer returned by following Get funtions needs to be freed by calling application, Also Do NULL check in case if malloc fails
 */
char* ievent_get_version(IgniteEvent* pstIE);

#ifdef __cplusplus
}
#endif
