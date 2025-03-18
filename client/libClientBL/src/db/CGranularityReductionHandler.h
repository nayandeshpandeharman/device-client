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
********************************************************************************
* \file CGranularityReductionHandler.h
*
* \brief This class performing granularity reduction in the database upon 
* request. The operation is based on the policies read from the config file.
********************************************************************************
*/

#ifndef CGRANULARITY_REDUCTION_HANDLER_H
#define CGRANULARITY_REDUCTION_HANDLER_H

#include <string>
#include <vector>
#include <set>
#include "jsoncpp/json.h"
#include "upload/CUploadUtils.h"

namespace ic_bl 
{
/**
 * Class CGranularityReductionHandler performing granularity reduction in the 
 * database upon request.
 */
class CGranularityReductionHandler 
{
public:
    /**
     * Default no-argument constructor.
     */
    CGranularityReductionHandler();

    /**
     * Destructor
     */
    ~CGranularityReductionHandler();

    /**
     * Method to perform granularity reduction operation
     * @param void
     * @return void
     */
    void PerformGranularityReduction();

    /**
     * Structure containing trigger event details.
     * (Trigger ideally contains pre and post trigger files).
     */
    struct TrigEventDetails 
    {
        long long llPreTrigId; ///< pre trigger event id
        std::string strPreTrigEvents; ///< pre trigger event data string
        long long llPostTrigId; ///< post trigger event id
        std::string strPostTrigEvents; ///< post trigger event data string
        
        /**
         * Method to initialize structure variables
         * @param void
         * @return void
         */
        void Init() 
        {
            llPreTrigId = -1;
            strPreTrigEvents = "";
            llPostTrigId = -1;
            strPostTrigEvents = "";
        }
    };

    /**
     * Structure containing event details.
     */
    struct EventList 
    {
        long long llEventId; ///< event id
        std::string strEventId; ///< event id in string
        std::string strEvents; ///< event data
        int nHasAttach; ///< event had attached int value
        long long llTimestamp; ///< event timestamp

        /**
         * Method is used to order elements by ST
         * @param[in] rstELObj structure 'EventList' object 
         * @return true if 'rstELObj.llEventId' greater than 'this->llEventId', 
         * false otherwise
         */
        bool operator<(const EventList &rstELObj) const
        {
            return (this->llEventId < rstELObj.llEventId);
        }
    };

    /**
     * Structure containing details about default policy.
     */
    struct DefaultGRPolicy 
    {
        std::string strName; ///< GR policy name
        int nFifoEventsRemovePercent; ///< FIFO events remove percent
    };

    /**
     * Structure containing policy details loaded from the config file.
     */
    struct GRPolicyConfigDetails 
    {
        std::string strName; ///< GR policy name
        std::set<std::string> setAddlExemptedEvnts; ///< set to add exempted events
        std::set<std::string> setRemoveFromExemptedEvnts; ///< set to remove exempted events
        bool bClearDefExemptedEvents; ///< flag to clear default exempted events
    };

    /**
     * Structure to hold individual policy details after they are read from 
     * config and ready to use.
     */
    struct GRPolicy 
    {
        std::string strName; ///< GR policy name
        std::string strExemptedEvents; ///< exempted events
        int nGranLevel; ///< Granularity level
    };

    /**
     * Structure to hold timestamp & id of Last GR performed record.
     */
    struct LastGRPerformedRecord
    {
        long long llTimeStamp; ///< GR performed record timestamp
        long long llLastGRPerRecId; ///< GR performed record id

        /**
         * Method is reset 'LastGRPerformedRecord' structure data
         * @param void
         * @return void
         */
        void Reset() 
        {
            llTimeStamp = 0;
            llLastGRPerRecId = 0;
        }
    };

#ifdef IC_UNIT_TEST
public:
    friend class CGranularityReductionHandlerTest;

    /**
     * Method to read the value of m_bIsPolicyReadFromConfig
     * @param void
     * @return true if policy is already read from config; false otherwise
     */
    bool IsPolicyReadFromConfig();

    /**
     * Method to read the value of m_stLastGrRecord
     * @param void
     * @return the db record detail for which GR policy was performed last time
     */
    LastGRPerformedRecord GetLastGRRecord();

    /**
     * Method to read the value of m_strPreTrgrEventIdSuffix
     * @param void
     * @return the pre-trigger eventID suffix
     */
    std::string GetPreTrgrEventIdSuffix();

    /**
     * Method to read the value of m_strPostTrgrEventIdSuffix
     * @param void
     * @return the post-trigger eventID suffix
     */
    std::string GetPostTrgrEventIdSuffix();

    /**
     * Method to read the value of m_nFreeStorageGainPercent
     * @param void
     * @return the free storage gain percent
     */
    int GetFreeStorageGainPercent();

    /**
     * Method to read the value of m_strMandatoryExemptedEvents
     * @param void
     * @return the mandatory exempted events string
     */
    std::string GetMandatoryExemptedEvents();

    /**
     * Method to read the value of m_eGReductionEventType
     * @param void
     * @return the type of events to delete during GR process
     */
    CUploadUtils::UploadDataMode GetGReductionEventType();

    /**
     * Method to read the value of DEF_FREE_STORAGE_GAIN_PERCENT
     *    defined in the source file
     * @param void
     * @return the default free storage gain percent
     */
    int GetDefaultFreeStorageGainPercent();

    /**
     * Method to read the value of DEF_POST_TRIGGER_EVENT_ID_SUFFIX
     *    defined in the source file
     * @param void
     * @return the default post trigger event ID suffix
     */
    std::string GetDefaultPostTriggerEventIDSuffix();

    /**
     * Method to read the value of DEF_PRE_TRIGGER_EVENT_ID_SUFFIX
     *    defined in the source file
     * @param void
     * @return the default pre trigger event ID suffix
     */
    std::string GetDefaultPreTriggerEventIDSuffix();
#endif

private:
    /**
     * Method to load the mandatory exempted events (i.e. streaming events) from
     * config file (streaming events are not supposed to be granularized).
     * @param void
     * @return void
     */
    void LoadMandatoryExemptedEvents();

    /**
     * Method to load the policy configurations from config file
     * @param void
     * @return void
     */
    void LoadPolicies();

    /**
     * Method to populate exempted events list for the given policy
     * @param[in] stPolicy GR policy details as read in the config file
     * @param[out] rstrEventsExempted exempted events list
     * @return void
     */
    void PopulateExemptionList(GRPolicyConfigDetails stPolicy,
                               std::string &rstrEventsExempted);

    /**
     * Method to handle the policy 'RemoveAlternateSimilarEvent'
     * @param[in] llStartTime startTime in the database to apply this policy
     * @param[in] llEndTime endTime in the database to apply this policy
     * @param[in] stPolicy policy details
     * @param[in] llLastGRedId last GR performed recordId to consider for query
     * @return void
     */
    void RemoveAlternateSimilarEventPolicyHandler(
                                     long long llStartTime, long long llEndTime,
                                     GRPolicy stPolicy, long long llLastGRedId);

    /**
     * Method to handle the policy 'RemoveRepeatedTriggerEvents_LeaveFirstAndLast'
     * @param[in] llStartTime startTime in the database to apply this policy
     * @param[in] llEndTime endTime in the database to apply this policy
     * @param[in] stPolicy policy details
     * @return void
     */
    void RemoveRepeatedTriggerEventsLeaveFirstAndLastPolicyHandler(
                                     long long llStartTime, long long llEndTime,
                                     GRPolicy stPolicy);

    /**
     * Method to handle the policy 'RemoveTriggerEventPostFiles'
     * @param[in] llStartTime startTime in the database to apply this policy
     * @param[in] llEndTime endTime in the database to apply this policy
     * @param[in] stPolicy policy details
     * @return void
     */
    void RemoveTriggerEventPostFilesPolicyHandler(
                       long long startTime, long long endTime, GRPolicy policy);

    /**
     * Method to handle the policy 'Default'
     * @param[in] llStartTime startTime in the database to apply this policy
     * @param[in] llEndTime endTime in the database to apply this policy
     * @param[in] stPolicy policy details
     * @return void
     */
    void DefaultPolicyHandler();

    /**
     * Method to apply granularity level in the database according to the given
     * policy
     * @param[in] llStartTime startTime in the database to apply this policy
     * @param[in] llEndTime endTime in the database to apply this policy
     * @param[in] stPolicy policy details that contains the granularity level 
     * to be applied
     * @return void
     */
    void UpdateGRLevel(long long llStartTime, long long llEndTime,
                       GRPolicy stPolicy);

    /**
     * Method to query and get LastGranularilyApplied timestamp
     * @param[in] llStartTime startTime in the database to apply this policy
     * @param[in] llEndTime endTime in the database to apply this policy
     * @param[in] rstLastGrRecordDetails timestamp and id of last record for which GR applied
     * @return true if record available, false if it is empty.
     */
    bool GetLastGranularityPerformedRecord(
                                 long long llStartTime, long long llEndTime,
                                 LastGRPerformedRecord &rstLastGrRecordDetails);

    /**
     * Method to retrieve unique steady state event ids from the database
     * @param[in] llStartTime retrieval start time
     * @param[in] llEndTime retrieval end time
     * @param[in] strExempEventList exempted events lis
     * @return void
     */
    std::set<std::string> PopulateUniqueSteadyStateEventIds(
                                     long long llStartTime, long long llEndTime,
                                     std::string strExempEventList);

    /**
     * Method to retrieve all the event details including Session event 
     * (IgniteClientLaunched) that match the given eventid
     * @param[in] strEventId event id
     * @param[in] llStartTime retrieval start time
     * @param[in] llEndTime retrieval end time
     * @param[in] llStartId start event id
     * @param[in] nLimit number of events to limit while querying
     * @return retrieved event id set
     */
    std::set<EventList> PopulateSessionBasedEventList(
                          std::string strEventId, long long llStartTime,
                          long long llEndTime, long long llStartId, int nLimit);

    /**
     * Method to retrieve all the POST trigger event details
     * @param[in] llStartTime retrieval start time
     * @param[in] llEndTime retrieval end time
     * @param[in] strPostTrgrEvntIdSuffix post trigger eventId suffix string
     * @param[in] llStartId start event id
     * @param[in] nLimit number of events to limit while querying
     * @return retrieved event id set
     */
    std::set<EventList> PopulatePostTriggerEventList(
                                     long long llStartTime, long long llEndTime,
                                     std::string strPostTrgrEvntIdSuffix,
                                     long long llStartId, int nLimit);

    /**
     * Method to retrieve all the event details of given eventid - including 
     * Session event (IgniteClientLaunched) and corresponding post trigger 
     * events
     * @param[in] strTrigEventId trigger event id
     * @param[in] strPostTrgrEvntIdSuffix post trigger eventId suffix string
     * @param[in] llStartTime retrieval start time
     * @param[in] llEndTime retrieval end time
     * @param[in] llStartId start event id
     * @param[in] nLimit number of events to limit while querying
     * @return retrieved trigger event id set
     */
    std::set<EventList> PopulateSessionBasedTriggerEventList(
                std::string strTrigEventId, std::string strPostTrgrEvntIdSuffix,
                long long llStartTime, long long llEndTime, long long llStartId,
                int nLimit);

    /**
     * Method to retrieve unique PRE trigger event ids
     * @param[in] llStartTime retrieval start time
     * @param[in] llEndTime retrieval end time
     * @param[in] strPreTrgrEvntIdSuffix pre trigger eventId suffix string
     * @param[in] strExempEventList exempted event list
     * @return retrieved trigger event id set
     */
    std::set<std::string> PopulateUniquePreTriggerEventIds(
                                     long long llStartTime, long long llEndTime,
                                     std::string strPreTrgrEvntIdSuffix,
                                     std::string strExempEventList);

    /**
     * Method to retrieve total events in the database
     * @param void
     * @return total number of events in the database
     */
    int GetEventRowCount();

    /**
     * Method to read from the given configuration the type of events
     * to delete during GR process.
     * @param[in] rjsonGR GR configuration in json format
     * @return bool true if the events' type is read successfully,
     * false otherwise.
     */
    bool ReadEventTypeToDeleteDuringGR(const ic_utils::Json::Value &rjsonGR);

    /**
     * Method to read from the given configuration the free storage
     * gain percent post GR process.
     * @param[in] rjsonGR GR configuration in json format
     * @return int the percent value read
     */
    int ReadGRFreeStorageGainPercent(const ic_utils::Json::Value &rjsonGR);

    /**
     * Method to read from the given configuration the GR exempted events
     * @param[in] rjsonGR GR configuration in json format
     * @return bool true if the exempted events are read from the configuration
     *               successfully, false otherwise.
     */
    bool ReadGRExemptedEvents(const ic_utils::Json::Value &rjsonGR);

    /**
     * Method to read from the given configuration the pre trigger
     * events' suffix.
     * @param[in] rjsonGR GR configuration in json format
     * @return string the suffix value
     */
    std::string ReadGRPreTriggerEventIdSuffix(const ic_utils::Json::Value 
                                              &rjsonGR);

    /**
     * Method to read from the given configuration the post trigger
     * events' suffix.
     * @param[in] rjsonGR GR configuration in json format
     * @return string the suffix value
     */
    std::string ReadGRPostTriggerEventIdSuffix(const ic_utils::Json::Value 
                                               &rjsonGR);

    /**
     * Method to read the default GR policy from the given configuration
     * @param[in] rjsonGR GR configuration in json format
     * @return bool true if the default policy is read, false otherwise.
     */
    bool ReadDefaultGRPolicy(const ic_utils::Json::Value &rjsonGR);

    /**
     * Method to read the GR policies from the given configuration
     * @param[in] rjsonGR GR configuration in json format
     * @return bool true if the GR policies are found and read, false otherwise
     */
    bool ReadGRPolicies(const ic_utils::Json::Value &rjsonGR);

    /**
     * Method to populate all the matching events from database
     *  according to the given conditions.
     * @param[in] rstrEId event ID to match with populated events
     * @param[in] rllStartTime minimum timestamp of the event
     * @param[in] rllEndTime maximum timestamp of the event
     * @param[in] rllLastGRedId value of minimum Column ID
     * @param[out] rsetDeleteEvntIds populated events set
     * @param[out] rsetDeleteFileList populated file attachments set
     * @return void
     */
    void PopulateEventsList(const std::string &rstrEId,
                            const long long &rllStartTime,
                            const long long &rllEndTime,
                            const long long &rllLastGRedId,
                            std::set<long long> &rsetDeleteEvntIds,
                            std::set<std::string> &rsetDeleteFileList);

    /**
     * Method to populate all the trigger events that are matching
     *  with the given evenet id.
     * @param[in] rstrPreTrigEventId event ID to match with the trigger events
     * @param[in] rllStartTime minimum timestamp of the event
     * @param[in] rllEndTime maximum timestamp of the event
     * @param[out] vecTrigEvntsToDelete trigger events to delete from database
     * @param[out] vecTrigEvnts trigger events list
     * @return void
     */
    void PopulateTriggerEventsList(const std::string &rstrPreTrigEventId,
                    const long long &rllStartTime,
                    const long long &rllEndTime,
                    std::vector<TrigEventDetails> &vecTrigEvntsToDelete,
                    std::vector<TrigEventDetails> &vecTrigEvnts);

    /**
     * Method to populate events and files to delete according to
     *  the given trigger events list.
     * @param[in] rvecTrigEvntsToDelete vector of trigger events
     * @param[out] rsetIdsToDelete populated events to delete
     * @param[out] rsetDeleteFileList populated files to delete
     * @return void
     */
    void PopulateEventsAndFilesToDelete(
                    const std::vector<TrigEventDetails> &rvecTrigEvntsToDelete,
                    std::set<long long> &rsetIdsToDelete,
                    std::set<std::string> &rsetFilesToDelete);

    /**
     * Method to find out the associated file attachments with the
     *   given event and add them to the list.
     * @param[in] rstrEventData event payload
     * @param[out] rsetFilesToDelete populated files to delete
     * @return void
     */
    void AddAttachmentsToDeleteList(
                const std::string &rstrEventData,
                std::set<std::string> &rsetFilesToDelete);

    /**
     * Method to add the override events of GR exemption
     *   to the given policy.
     * @param[in] rjsonExemp json confi for overridden events
     * @param[out] rpolicy GR policy object
     * @return void
     */
    void AddExemptedOverrideEvents(
                const ic_utils::Json::Value &rjsonExemp,
                GRPolicyConfigDetails &rpolicy);
    /**
     * Method to add the attachments from the given record
     *      to the given filelist.
     * @param[in] rRecord EventList object pointing to a single record
     * @param[out] rsetDeleteFileList populated files to delete
     * @return void
     */
    void AddAttachmentsFromEventRecord(
                const EventList &rRecord,
                std::set<std::string> &rsetDeleteFileList);


    /**
     * Method to process the given Event list and add
     *      trigger events to the given list
     * @param[in] rsetEvntList set of EventList objects
     * @param[out] rllStartId start event id
     * @param[out] rvecTrigEvntsToDelete trigger events to delete from database
     * @param[out] rvecTrigEvnts trigger events list
     * @return void
     */
    void ProcessEventListAndAddEvents(
            const std::set<EventList> &rsetEvntList,
            long long &rllStartId,
            std::vector<TrigEventDetails> &rvecTrigEvntsToDelete,
            std::vector<TrigEventDetails> &rvecTrigEvnts);

    //! Member variable to stores policy read from config
    bool m_bIsPolicyReadFromConfig;

    //! Member variable to stores DB size limit
    size_t m_unDbSizeLimit;

    //! Member variable to stores attach path
    std::string m_strAttachPath;

    //! Member variable to stores pre trigger eventId suffix
    std::string m_strPreTrgrEventIdSuffix;

    //! Member variable to stores post trigger eventId suffix
    std::string m_strPostTrgrEventIdSuffix;

    //! Member variable to stores free storage gain percent
    int m_nFreeStorageGainPercent;

    //! Member variable to stores exempted events
    std::set<std::string> m_setExemptedEvents;

    //! Member variable to stores GR policies details
    std::vector<GRPolicy> m_vectPolicies;

    //! Member variable to stores mandatory exempted events
    std::string m_strMandatoryExemptedEvents;

    //! Member variable to stores default GR policy details
    DefaultGRPolicy m_stDefPolicy;

    //! Member variable to stores indicate type of events to delete during GR
    CUploadUtils::UploadDataMode m_eGReductionEventType;
    
    //! Member variable to stores last GR performed Record
    LastGRPerformedRecord m_stLastGrRecord;
};
}/* namespace ic_bl */
#endif /* CGRANULARITY_REDUCTION_HANDLER_H */
