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
* \file: CIgniteEvent.h
*
* \brief: This class is used to capture all the necessary attributes 
          required by the Ignite Event.
*******************************************************************************
*/

#ifndef CIGNITE_EVENT_H
#define CIGNITE_EVENT_H

#include <vector>
#include <string>
#include <memory>
#include <ctime>
#include "jsoncpp/json.h"

#define ATTACHMENT_OK                0 /*File Accepted for Upload*/
#define ERROR_ATTACHLIMIT           -1 /*AttachementCount limit reached =100*/
#define ERROR_NOCONFIG              -2 /*Attachement Path not Configured*/
#define ERROR_NODEVICEID            -3 /*deviceId is empty*/
#define ERROR_FILETOOLARGE          -4 /*Individual FileSizeLimit*/
#define ERROR_TOTALOVERLIMIT        -5 /*TotalSizeLimit of Attachments*/
#define ERROR_UNDERSCOREREJECTED    -6 /*File Type Error,Underscore present in FileType*/
#define ERROR_NOFILENAME            -7 /*FileName is empty*/
#define ERROR_NOFILE                -8 /*File Does not Exists*/
#define ERROR_FILELENGTHLIMIT       -9 /*FileName exceeds lenght limit, (MAX_FILENAME_LENGTH(90)-3)*/

namespace ic_event 
{

//! Static member variable to define the eventID tag
static const char* EVENT_ID_TAG = "EventID";

//! Static member variable to define the mode tag
static const char* MODE_TAG = "BenchMode";

//! Static member variable to define the timestamp tag
static const char* TIMESTAMP_TAG = "Timestamp";

//! Static member variable to define timezone tag
static const char* TIMEZONE_TAG = "Timezone";

//! Static member variable to define the value tag
static const char* VALUE_TAG = "Data";

//! Static member variable to define the version tag
static const char* VERSION_TAG = "Version";

//! Static member variable to define the file attachment tag
static const char* FILE_ATTACHMENT_TAG = "UploadId";

//! Static member variable to define PII tag
static const char* PII_TAG = "pii";

//! Static member variable to define the BizTransactionId tag
static const char* BIZTRANCID = "BizTransactionId";

//! Static member variable to define the messageID tag
static const char* MSGID = "MessageId";

//! Static member variable to define the CorrelationId tag
static const char* CORID = "CorrelationId";

/**
 * This interface is used by the Event to send data.
 */
class IEventSender 
{
public:
    /**
    * Destructor
    */
    virtual ~IEventSender() {}

    /**
     * Method to send event to queue
     * @param[in] serializedEvent Event getting added to queue
     * @return 0 if event is sent successfully else -1
     */
    virtual int Send(const std::string& serializedEvent) = 0;
};

/**
 * Event class is used to create events by capturing all the necessary attributes.
 */
class CIgniteEvent 
{
public:
    /**
     * Event default constructor
     */
    CIgniteEvent ();

    /**
     * Parameterized constructor with three parameters to create a new Event object
     * @param[in] rstrVersion Version of the event
     * @param[in] rstrEventId EventID of the event
     * @param[in] dblTimestamp Timestamp of the event.
     *                  It is specified as number of milliseconds from Jan 1, 1970
     */
    CIgniteEvent(const std::string& rstrVersion, const std::string& rstrEventId, const double dblTimestamp);

    /**
     * Parameterized constructor with two parameters to create a new Event object
     * @param[in] rstrVersion Version of the event
     * @param[in] rstrEventId EventID of the event
     */
    CIgniteEvent(const std::string& rstrVersion, const std::string& rstrEventId);

    /**
    * Destructor
    */
    virtual ~CIgniteEvent();

    /**
     * Overloaded method to add given key-value pair to the event
     * @param[in] rstrName key name
     * @param[in] nVal Value of type int
     * @return void
     */
    virtual void AddField(const std::string& rstrName, const int nVal);

    /**
     * Overloaded method to add given key-value pair to the event
     * @param[in] rstrName key name
     * @param[in] llVal Value of type long long
     * @return void
     */
    virtual void AddField(const std::string& rstrName, const long long llVal);

    /**
     * Overloaded method to add given key-value pair to the event
     * @param[in] rstrName key name
     * @param[in] dblVal Value of type double
     * @return void
     */
    virtual void AddField(const std::string& rstrName, const double dblVal);

    /**
     * Overloaded method to add given key-value pair to the event
     * @param[in] rstrName key name
     * @param[in] rstrVal Value of type string
     * @return void
     */
    virtual void AddField(const std::string& rstrName, const std::string& rstrVal);

    /**
     * Overloaded method to add given key-value pair to the event
     * @param[in] rstrName key name
     * @param[in] pchVal Value of type char*
     * @return void
     */
    virtual void AddField(const std::string& rstrName, const char* pchVal);

    /**
     * Overloaded method to add given key-value pair as json string to the event
     * @param[in] rstrName key name
     * @param[in] rstrVal json value in string format
     * @return void
     */
    virtual void AddFieldAsRawJsonString(const std::string& rstrName, const std::string& rstrVal);

    /**
     * Overloaded method to add given key-value pair as json string to the event
     * @param[in] rstrName key name
     * @param[in] pchVal json value in char* format
     * @return void
     */
    virtual void AddFieldAsRawJsonString(const std::string& rstrName, const char* pchVal);

    /**
     * Overloaded method to add given key-value pair to the event
     * @param[in] rstrName key name
     * @param[in] bVal Value of type bool
     * @return void
     */
    virtual void AddField(const std::string& rstrName, const bool bVal);

    /**
     * Overloaded method to add given key-value pair to the event
     * @param[in] rstrName key name
     * @param[in] jsonVal Value of type ic_utils::Json::Value
     * @return void
     */
    virtual void AddField(const std::string &rstrName, ic_utils::Json::Value jsonVal);

    /**
     * Method to remove the given key (and associated value) from the event
     * @param[in] key key name
     * @return void
     */
    virtual void RemoveField(const std::string& rstrKey);

    /**
     * Method to add biz transaction ID to the event
     * @param[in] rstrVal Biztransactionid
     * @return void
     */
    virtual void AddBizTransaction(const std::string& rstrVal);

    /**
     * Method to add message-id to the event
     * @param[in] rstrVal Message-id
     * @return void
     */
    virtual void AddMessageId(const std::string& rstrVal);

    /**
     * Method to add correlation-id to the event
     * @param[in] rstrVal correlation-id
     * @return void
     */
    virtual void AddCorrelationId(const std::string& rstrVal);

    /**
     * Method to attach a file to the event
     * @param[in] rstrFileType Type of the file
     * @param[in] rstrPath Absolute path of the given file
     * @param[in] bCompressFile True to compress the given file
     * @param[in] bDeleteAttachment True to delete the given file after copied the file under Ignite folder
     * @return void
     */
    virtual void AttachFile(const std::string& rstrFileType, const std::string& rstrPath,
                            const bool bCompressFile = true, const bool bDeleteAttachment=true);

    /**
     * Method to get the event-id from the event
     * @param void
     * @return EventId of the event 
     */
    virtual std::string GetEventId();

    /**
     * Method to get the timestamp from the event
     * @param void
     * @return Event timestamp 
     */
    virtual double GetTimestamp();

    /**
     * Method to get the biztransactionId from the event
     * @param void
     * @return BiztransactionId 
     */
    virtual std::string GetBizTransactionId();

    /**
     * Method to get the message-id from the event
     * @param void
     * @return Message-id
     */
    virtual std::string GetMessageId();

    /**
     * Method to get the CorrelationId from the event
     * @param void
     * @return CorrelationId
     */
    virtual std::string GetCorrelationId();

    /**
     * Method to get the value of the given key from the event
     * @param[in] rstrKey Key name
     * @param[in] nDefaultValue Value of type int has default value 0
     * @return Integer value if key is member else default value
     */
    virtual int GetInt(const std::string& rstrKey, int nDefaultValue = 0);

    /**
     * Method to get the value of the given key from the event
     * @param[in] rstrKey Key name
     * @param[in] bDefaultValue Value of type bool has default value false
     * @return True if key is a member else false
     */
    virtual bool GetBool(const std::string& rstrKey, bool bDefaultValue = false);

    /**
     * Method to get the value of the given key from the event
     * @param[in] rstrKey Key name
     * @param[in] strDefaultValue Value of type string has empty string as default value
     * @return String if key is member else default value
     */
    virtual std::string GetString(const std::string& rstrKey, std::string strDefaultValue = "");

    /**
     * Method to get the value of the given key from the event
     * @param[in] rstrKey Key name
     * @param[in] llDefaultValue Value of type long long has default value 0
     * @return long long if key is member else default value 0
     */
    virtual long long GetLong(const std::string& rstrKey, long long llDefaultValue = 0);

    /**
     * Method to get the value of the given key from the event
     * @param[in] rstrKey Key name
     * @param[in] dblDefaultValue Value of type double has default value 0.0
     * @return double if key is member else default value
     */
    virtual double GetDouble(const std::string& rstrKey, double dblDefaultValue = 0.0);
    
    /**
     * Method to get the value of the given key from the event
     * @param[in] rstrKey Key name
     * @param[in] jsonDefaultValue Value of type ic_utils::Json::Value has default value null 
     * @return  Value of type ic_utils::Json::Value if key is member else default value
     */
    virtual ic_utils::Json::Value GetJson(const std::string& rstrKey, ic_utils::Json::Value jsonDefaultValue = ic_utils::Json::Value::nullRef);

    /**
     * Method to get the complete data section of the event
     * @param void
     * @return value of type ic_utils::Json::Value
     */
    virtual ic_utils::Json::Value GetData();

    /**
     * Method to set the timestamp of the event
     * @param[in] dblTimestamp Timestamp specified as number of milliseconds from Jan 1, 1970
     * @return void
     */
    virtual void SetTimestamp(const double dblTimestamp);

    /**
     * Method to set the timezone of the event
     * @param[in] nTzoffset Timezone value
     * @return void
     */
    virtual void SetTimezone (const int nTzoffset);

    /**
     * Method to set the version of the event
     * @param[in] rstrVersion Event version
     * @return void
     */
    virtual void SetVersion(const std::string& rstrVersion);

    /**
     * Method to get the version of the event
     * @param[in] void
     * @return Event version 
     */
    virtual std::string GetVersion();

    /**
     * Method for == overloaded
     * @param[in] event Input event
     * @return true if eventId is same else false
     */
    virtual bool operator == (CIgniteEvent& event);

    /* != overloaded */
    /**
     * Method for != overloaded
     * @param[in] event Input event
     * @return true if event is found else false
     */
    virtual bool operator != (CIgniteEvent& event);

    /**
     * Method to convert the event to a json value
     * @param[out] string The event data converted as json in string format
     * @return void
     */
    virtual void EventToJson(std::string&);

    /**
     * Method to extract the filename from given file path
     * @param[in] string File path
     * @return void
     */
    virtual void ExtractName(std::string&);

    /**
     * Method to send the event to the receiver
     * @param void
     * @return 0 if sending event is successful else -1
     */
    virtual int Send();

    /**
     * IEventSender reference to hold the sender object
     */
    static IEventSender* eventSender;

    /**
     * Static method to set the event sender
     * @param[in] pSender Event sender reference
     * @return void
     */
    static void SetSender(IEventSender* pSender);

    /**
     * Method to convert the given event which is in json format, and load it into the event
     * @param[in] jsonEvent Event of string type in json format
     * @return void
     */
    void JsonToEvent(const std::string& rstrJsonEvent);

    /**
     * flag to track the bench mode 
     */
    static bool bBenchMode;

    /**
     * Method to return the status of the attachment updated in m_attach_reqs
     * @param[in] rstrPath File path 
     * @return 0 if given file attachment is success; negative value otherwise
     */
    virtual int GetAttachFileStatus(const std::string& rstrPath);

    /**
     * Method to clear m_attach_reqs data structure,to be called once getAttachStatus is done
     * @param void
     * @return void
     */
    virtual void ClearAttachMentDetails();

    /**
     * Static method to clear the event sender
     * @param void
     * @return void
     */
    static void ClearSender();

protected:

    //! Member variable to store the json object values
    ic_utils::Json::Value m_jsonValue;

    //! Member variable to store the event fields
    ic_utils::Json::Value m_jsonEventFields;

    //! Member variable to store the PII value
    ic_utils::Json::Value m_jsonPiiFields;

    //! Member variable to store the attachment count
    int m_nAttachmentCount;

    //! Member variable to store all the attachments
    std::vector<std::string> m_vectAttachment;

    //! Member variable to store the file attachment path
    std::string m_strFileAttachmentPath  = "";

    //! Member variable to store the device ID
    std::string m_strDeviceId = "";

    //! Member variable to store the state of deleting the file attachment
    bool m_bDeleteAttachmentAfterAccept = false ;
    
    /**
     * Method to validate the file attachment and accept if validation succeeded
     * @param[in] rstrFileType Type of the file
     * @param[in] rstrPath File path
     * @param[in] bCompressFile True to compress the given file
     * @param[in] bDeleteAttachment True to delete the given file after copied the file under Ignite folder
     * @return 0 if success else negative value 
     */
    virtual int AcceptAttach(const std::string& rstrFileType, const std::string& rstrPath,
            const bool bCompressFile, bool bDeleteAttachment);

    /**
     * Method to trigger the processing of the file attachment
     * @param void
     * @return void
     */
    void ProcessAttachment();

private:
    /**
     * Method to read the file name and extension from given file path
     * @param[in] rstrPath file path
     * @param[out] rstrExtn file extension
     * @param[out] rstrFilename file name
     * @return void
     */
    void GetFileNameAndExtension(const std::string &rstrPath,
            std::string &rstrExtn, std::string &rstrFilename);

    /**
     * Method to validate the file attachment path and
     *   create it if it doesn't exist.
     * @param[in] rstrAttachTempPath file attachment path
     * @return true if setup successfully, false otherse.
     */
    bool SetupFileAttachmentPath(std::string &rstrAttachTempPath);

    /**
     * Method to prepare the compressed file path based on
     *   given file attributes.
     * @param[in] rstrAttachTempPath file attachment path
     * @param[in] rstrFileName file name
     * @param[in] rstrFileExtension file extension
     * @return string compressed file path
     */
    std::string SetupCompressedFilePath(const std::string &rstrAttachTempPath,
                                                        const std::string &rstrFileName,
                                                        const std::string &rstrFileExtension);

    /**
     * Method to compress the given file
     * @param[in] rbCompressFile flag indicating to compress or not
     * @param[in] rstrPath file path
     * @param[in] strCompressedPath compressed file path
     * @param[out] rnNewAttachSize new attachment file size
     * @return true if successfully compressed, false otherwise
     */
    bool CompressFile(const bool &rbCompressFile, const std::string &rstrPath,
                        const std::string &strCompressedPath, int &rnNewAttachSize);

    /**
     * Method to read total file size of all the files present
     *   in the given path.
     * @param[in] rnstrFileAttachmentPath file attachments' path
     * @return int total size of all the file attachments
     */
    int GetExistingAttachmentSize(const std::string &rnstrFileAttachmentPath);

    /**
     * Method to validate given file attributes and accordingly
     *   construct the out file name.
     * @param[in] rnNewAttachSize new file attachment size
     * @param[in] rnExistingAttachSize existing file attachment size
     * @param[in] rnFileSizeLimit file size limit
     * @param[in] rnTotalFileSizeLimit total file size limit
     * @param[in] rstrFileType file type
     * @param[in] rstrEventId event id
     * @param[out] rstrOutFileName filename post validation
     * @return int error code indicating the validation result
     */
    int ValidateOutFileName(const int &rnNewAttachSize,
                            const int &rnExistingAttachSize,
                            const int &rnFileSizeLimit,
                            const int &rnTotalFileSizeLimit,
                            const std::string &rstrFileType,
                            const std::string &rstrEventId,
                            std::string &rstrOutFileName);

    /**
     * Method to add given extension to the given filename
     *   construct the out file name.
     * @param[in] rstrFileExtension file extension
     * @param[out] rstrOutputFileName filename after adding extn
     * @return void
     */
    void AddExtnToOutFileName(const std::string &rstrFileExtension,
                              std::string &rstrOutputFileName);

    /**
     * Method to truncate the file based on given file extension
     * @param[out] rstrOutputFileName filename after truncated by extension
     * @param[in] rstrFileExtension file extension
     * @return void
     */
    void TruncateFileByExtn(std::string &rstrOutputFileName,
                           std::string &rstrFileExtension);

    /**
     * Method to truncate the file based on given filename
     * @param[out] rstrOutputFileName filename updated by rstrTruncFileName
     * @param[out] rstrTruncFileName truncated filename
     * @param[in] rstrFileExtension file extension
     * @param[in] rstrDeviceId device id
     * @param[in] rstrFileType file type
     * @param[in] rstrEventId event id
     * @param[in] rstrTimeConvert time string
     * @param[in] rstrstreamAttachCountConvert attachment count
     * @return void
     */
    void TruncateFileByFileName(std::string &rstrOutputFileName,
            std::string &rstrTruncFileName,
            const std::string &rstrFileExtension,
            const std::string &rstrDeviceId,
            const std::string &rstrFileType,
            const std::string &rstrEventId,
            const std::string &rstrTimeConvert,
            const std::ostringstream &rstrstreamAttachCountConvert);

    /**
     * Method to truncate the file based on given file type
     * @param[out] rstrOutputFileName filename updated by rstrTruncFileType
     * @param[out] rstrTruncFileType truncated file type
     * @param[in] rstrTruncFileName truncated file name
     * @param[in] rstrFileExtension file extension
     * @param[in] rstrDeviceId device id
     * @param[in] rstrTimeConvert time string
     * @param[in] rstrstreamAttachCountConvert attachment count
     * @return void
     */
    void TruncateFileByFileType(std::string &rstrOutputFileName,
            std::string &rstrTruncFileType,
            const std::string &rstrTruncFileName,
            const std::string &rstrFileExtension,
            const std::string &rstrDeviceId,
            const std::string &rstrTimeConvert,
            const std::ostringstream &rstrstreamAttachCountConvert);

    /**
     * Method to truncate the file based on given event name
     * @param[out] rstrOutputFileName filename updated by rstrTruncFileType
     * @param[out] rstrTruncEventName truncated event name
     * @param[in] rstrTruncFileName truncated file name
     * @param[in] rstrFileExtension file extension
     * @param[in] rstrTruncFileType truncated file type
     * @param[in] rstrTimeConvert time string
     * @param[in] rstrstreamAttachCountConvert attachment count
     * @return void
     */
    void TruncateFileByEventName(std::string &rstrOutputFileName,
            std::string &rstrTruncEventName,
            const std::string &rstrTruncFileName,
            const std::string &rstrFileExtension,
            const std::string &rstrTruncfileType,
            const std::string &rstrTimeConvert,
            const std::ostringstream &rstrstreamAttachCountConvert);

    /**
     * Method to finalize the given out file name
     * @param[out] rstrOutputFileName filename post all validations
     * @param[out] rstrFinalOutFileName final filename
     * @param[out] rnErrorCode finalization error code (if any)
     * @param[in] rbIsCompressed flag indicating if the file is compressed
     * @param[in] rstrCfgDeleteAttachment config flag if to delete the file
     * @param[in] rstrCompressedPath compressed file path
     * @param[in] rstrFileAttachmentPath file attachment path
     * @param[in] rbDeleteAttachment bool flag to delete the file
     * @param[in] rstrPath actual file path
     * @return void
     */
    void FinalizeOutFileName(std::string &rstrOutputFileName,
            std::string &rstrFinalOutFileName,
            int &rnErrorCode,
            const bool &rbIsCompressed,
            const std::string &rstrCfgDeleteAttachment,
            const std::string &rstrCompressedPath,
            const std::string &rstrFileAttachmentPath,
            const bool &rbDeleteAttachment,
            const std::string &rstrPath);

    /**
     * Data structure to process the file attachments
     */
    typedef struct 
    {
        std::string strType;   ///< Type of the attachment
        std::string strPath;   ///< Attachment path
        bool bCompress;      ///< True if attachment is to be compressed else false
        bool bDeleteAttachment;  ///< True if attachment is to be deleted else false
        int  nStatusCode;    ///< The status of the file attachment
    } AttachRequest;

    //! Vector of all file attachments
    std::vector<AttachRequest> m_vectorAttachReqs;

public:
#ifdef IC_UNIT_TEST
public:
    friend class CIgniteEventTest;
#endif

    /**
     * Method to add device specific id to the event
     * @param[in] rstrName Device id name
     * @param[in] rstrVal Value of the id
     * @return void
     */
    virtual void AddDeviceId(const std::string& rstrName, const std::string& rstrVal);
};

} /* namespace ic_event */

#endif /* CIGNITE_EVENT_H */
