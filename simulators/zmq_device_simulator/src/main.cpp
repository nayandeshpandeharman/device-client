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
* \file main.cpp
*
* \brief This file handles multiple command line options to run as a ZMQClient *
*        or as a ZMQServer.                                                    *
********************************************************************************
*/

#include <iostream>
#include <algorithm>
#include <regex>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <zmq.h>
#include "jsoncpp/json.h"

//! Simulator version
#define VERSION "3.1.2"

//! Usage type argument position
#define USAGE_TYPE_ARG_POS 1

//! Message argument position for the client usage
#define MSG_ARG_POS_FOR_CLIENT_USAGE 2

//! Argument count for the client usage
#define ARG_COUNT_FOR_CLIENT_USAGE 3

//! RO response argument position for the server usage
#define RO_RESP_ARG_POS 2

//! Server usage option
#define SERVER_USAGE_OPTION "S"

//! Client usage option
#define CLIENT_USAGE_OPTION "C"

//! Argument for auto sending the success response to ro commands
#define RO_SUCCESS_ARG "ROS"

//! Argument for auto sending the fail response to ro commands
#define RO_FAIL_ARG "ROF"

//! server url
#define SERVER_URL "ipc:///tmp/ipcd_remote.ipc"

//! client url
#define CLIENT_URL "ipc:///tmp/ipcd_notif.ipc"

//! String for success response
#define SUCCESS_RESPONSE "SUCCESS"

//! String for fail response
#define FAIL_RESPONSE "FAIL"

//! String for custom response
#define CUSTOM_RESPONSE "CUSTOM"

//! String for invalid response
#define INVALID_RESPONSE "INVALID"

//! Unique number used to generate the MessageID for RemoteOperationResponse event
unsigned int UNIQUE_NUMBER = 876345;

//! Enum for RO responses
typedef enum
{
    eRO_RESPONSE_SUCCESS, ///< RO Response SUCCESS
    eRO_RESPONSE_FAIL, ///< RO Response FAIL
    eRO_RESPONSE_CUSTOM, ///< RO Response CUSTOM
    eRO_RESPONSE_INVALID ///< RO Response Invalid
} ROResponseType;

//! Static global variable to store user preference for RO response
static ROResponseType g_eROResponseToSend = eRO_RESPONSE_INVALID;

/**
 * Method to get current time in miliseconds
 * @param void
 * @return Numeric value indicates the time in milliseconds which
 * corresponds to system time.
 */
signed long long get_currenttime_ms()
{
    long long lCurrentTime = 0;

    struct timespec stTimeBuf;
    if (clock_gettime(CLOCK_REALTIME, &stTimeBuf) == -1)
    {
        return 0l;
    }
    lCurrentTime = (((unsigned long long)stTimeBuf.tv_sec) * 1000) +
                   (((unsigned long long)stTimeBuf.tv_nsec) / 1000000);

    return lCurrentTime;
}

/**
 * Method to get timezone offset in minutes
 * @param void
 * @return Numeric value indicates the time in minutes which
 * corresponds to the timezone offset.
 */
int get_timezone_offset_minutes()
{
    time_t t = time(0);
    struct tm *pstTime = (struct tm *)malloc(sizeof(struct tm));
    int nTimeZoneOffsetMinutes = 0;
    if (NULL != pstTime)
    {
        localtime_r(&t, pstTime);
        nTimeZoneOffsetMinutes = (pstTime->tm_gmtoff) / 60;
        free(pstTime);
    }
    return nTimeZoneOffsetMinutes;
}

/**
 * Method to connect to given URL and send the given message.
 * @param[in] rstrUrlToConnect URL to connect.
 * @param[in] rstrMsgToSend Message to send
 * @return void
 */
void client(const std::string &rstrUrlToConnect, const std::string &rstrMsgToSend)
{

    // zmq context and socket init
    void *pvoidContext = zmq_ctx_new();
    void *pvoidSocket = zmq_socket(pvoidContext, ZMQ_PUSH);

    // set zmq socket options
    int nVal = 0;
    zmq_setsockopt(pvoidSocket, ZMQ_LINGER, &nVal, sizeof(nVal));

    // connecting to the given url
    std::cout << "Connecting to " << rstrUrlToConnect << "..." << std::endl;
    int nRet = zmq_connect(pvoidSocket, rstrUrlToConnect.c_str());
    if (nRet == -1)
    {
        std::cout << "Error connecting to url..." << rstrUrlToConnect << std::endl;
        return;
    }

    // a slight breathing time for zmq connection to complete
    sleep(1);

    zmq_msg_t msg;

    // zmq msg init
    int nRc = zmq_msg_init_size(&msg, rstrMsgToSend.size() + 1);
    if (nRc) // nRc shoudld be zero
    {
        std::cout << "zmq msg init failed!" << std::endl;
    }
    else
    {
        // convert the given string type message into a zmq msg packet
        memcpy(zmq_msg_data(&msg), rstrMsgToSend.c_str(), rstrMsgToSend.size() + 1);

        // send the message
        if (-1 != zmq_msg_send(&msg, pvoidSocket, 0))
        {
            std::cout << "Msg is sent." << std::endl;
        }
        else
        {
            std::cout << "Msg send failed!" << std::endl;
        }

        /// close the msg object
        zmq_msg_close(&msg);
    }

    // disconnect and close the zmq socket & context
    zmq_disconnect(pvoidSocket, rstrUrlToConnect.c_str());
    zmq_close(pvoidSocket);
    zmq_term(pvoidContext);
}


/**
 * Method converts the ROResponseType enum to corresponding string.
 * @param[in] eType RO Response type.
 * @return String corresponding to the given ROResponseType
 */
std::string get_string_for_response_type(ROResponseType eType)
{
    switch(eType)
    {
        case eRO_RESPONSE_SUCCESS : return SUCCESS_RESPONSE;
        case eRO_RESPONSE_FAIL    : return FAIL_RESPONSE;
        case eRO_RESPONSE_CUSTOM  : return CUSTOM_RESPONSE;
        default : return INVALID_RESPONSE;
    }
}

/**
 * Method to construct the ro response
 * @param[in] jsonROResponse RO Response Json.
 * @param[in] jsonROResponseData Data section of RO Response Json.
 * @param[in] strResponseToSend RO response status string.
 * @return Ro Response string 
 */
std::string construct_ro_response(const ic_utils::Json::Value jsonROResponse,
                                  const ic_utils::Json::Value jsonROResponseData,
                                  std::string strResponseToSend)
{
    ic_utils::Json::FastWriter jsonFastwriter;
    ic_utils::Json::Value jsonROResponsePayload = jsonROResponse;
    ic_utils::Json::Value jsonROResponseDataPayload = jsonROResponseData;
    jsonROResponseDataPayload["response"] = strResponseToSend;
    jsonROResponsePayload["Data"] = jsonROResponseDataPayload;
    std::string strROResponse = jsonFastwriter.write(jsonROResponsePayload);
    // FastWriter introduces newline at the end, that needs to be truncated
    strROResponse.erase(std::remove(strROResponse.begin(), strROResponse.end(), 
                                    '\n'), strROResponse.end());
    return strROResponse;                                
}

/**
 * Method to print the ro response on to console
 * @param[in] strResp RO Response string to be printed.
 * @param[in] eType RO Response type(success/fail etc).
 * @param[in] bPrintCommand flag indicating if the complete command to send 
 * the response needs to be printed
 * @return void 
 */
void print_ro_response(const std::string strResp,
                       const ROResponseType eType, bool bPrintCommand)
{
    std::cout << " ======= RESPONSE FOR ";
    std::string strRespType = get_string_for_response_type(eType);
    std::cout << strRespType << " =======\n" << std::endl;
    std::string strRegexedString = std::regex_replace(strResp, std::regex("\""), "\\\"");
    if(bPrintCommand)
    {
        std::cout <<"./zmq_device_simulator C ";
    }
    std::cout << "\"" << strRegexedString << "\"";
    std::cout << "\n\n" << std::endl;
}

/**
 * Method to send the RO Response over zmq channel
 * @param[in] strResp RO Response string to be sent. 
 * the response needs to be printed
 * @return void 
 */
void send_ro_response(const std::string strResp)
{
    std::cout << "Sending RO response " << std::endl;
    client(CLIENT_URL,strResp);
}

/**
 * This function will prepare sample responses for both SUCCESS
 * and FAILURE scenarios so that the SUCCESS and FAILURE use-cases
 * can be simulated
 * @param[in] rjsonPayload RO command payload
 * @return void
 */
void handle_remote_operations_command(const ic_utils::Json::Value &rjsonPayload)
{
    std::cout << " ************************************************ \n" << std::endl;
    std::cout << " *** RO COMMAND RECEIVED *** : " << rjsonPayload["EventID"].asString() << "\n" << std::endl;

    // fetch the required data i.e BizTransactionID , MsgID, Topic etc
    std::string strBizID = rjsonPayload["BizTransactionId"].asString();
    std::string strMsgID = rjsonPayload["MessageId"].asString();
    ic_utils::Json::Value jsonData = rjsonPayload["Data"];
    std::string strTopic = jsonData["topic"].asString();
    std::string strRoReqID = jsonData["roRequestId"].asString();

    // Construct the RO response event
    ic_utils::Json::Value jsonROResponse;
    ic_utils::Json::Value jsonROResponseData;

    // Construct part of Data section of the RO response event
    jsonROResponseData["roRequestId"] = strRoReqID;
    jsonROResponseData["topic"] = strTopic;

    // Construct the RO response event
    jsonROResponse["BizTransactionId"] = strBizID;
    jsonROResponse["CorrelationId"] = strMsgID;
    jsonROResponse["Data"] = jsonROResponseData;
    jsonROResponse["EventID"] = "RemoteOperationResponse";
    jsonROResponse["MessageId"] = UNIQUE_NUMBER++;
    jsonROResponse["Timestamp"] = (ic_utils::Json::Value::Int64)get_currenttime_ms();
    jsonROResponse["Timezone"] = get_timezone_offset_minutes();
    jsonROResponse["Version"] = "1.1";

    switch (g_eROResponseToSend)
    {
        case eRO_RESPONSE_SUCCESS:
            {          
                std::cout << "Auto sending RO SUCCESS response \n" << std::endl;
                //construct the SUCCESS response
                std::string strROSuccessResp = construct_ro_response(jsonROResponse,
                                                                jsonROResponseData,
                                                                SUCCESS_RESPONSE);
                print_ro_response(strROSuccessResp,eRO_RESPONSE_SUCCESS ,false);
                send_ro_response(strROSuccessResp);   
            }                                              
            break;

        case eRO_RESPONSE_FAIL:
            {
                std::cout << "Auto sending RO FAIL response \n" << std::endl;
                //construct the FAIL response
                std::string strROFailResp = construct_ro_response(jsonROResponse,
                                                                jsonROResponseData,
                                                                FAIL_RESPONSE);
                print_ro_response(strROFailResp,eRO_RESPONSE_FAIL,false);
                send_ro_response(strROFailResp); 
            }                                                
            break;
        
        default:
            {
                std::string strROSuccessResp = construct_ro_response(jsonROResponse,
                                                                    jsonROResponseData,
                                                                    SUCCESS_RESPONSE);
                print_ro_response(strROSuccessResp,eRO_RESPONSE_SUCCESS,true);

                std::string strROFailResp = construct_ro_response(jsonROResponse,
                                                                    jsonROResponseData,
                                                                    FAIL_RESPONSE);
                print_ro_response(strROFailResp,eRO_RESPONSE_FAIL,true);

                std::string strROCustomResp = construct_ro_response(jsonROResponse,
                                                                    jsonROResponseData,
                                                                    CUSTOM_RESPONSE);
                print_ro_response(strROCustomResp,eRO_RESPONSE_CUSTOM,true);
            }
            break;
    }
    std::cout << "\n ************************************************ \n" << std::endl;

}

/**
 * Method to connect to given URL and print the incoming messages
 *   until receiving the message 'quit'.
 * @param[in] rstrUrlToConnect URL to connect
 * @return void
 */
void server(const std::string &rstrUrlToConnect)
{
    // zmq context and socket init
    void *pvoidContext = zmq_ctx_new();
    void *pvoidSocket = zmq_socket(pvoidContext, ZMQ_PULL);

    // binding with the given url
    std::cout << "Binding with the url " << rstrUrlToConnect << "..." << std::endl;
    if (0 != zmq_bind(pvoidSocket, rstrUrlToConnect.c_str()))
    {
        std::cout << "ERR: could not bind given url!" << std::endl;
        return;
    }

    std::cout << "Start listening...(to quit, send \"quit\" message)" << std::endl;

    zmq_msg_t msg;

    // until receiving a message 'quit', this loop will continue running
    while (true)
    {
        // zmq msg init
        int nRc = zmq_msg_init(&msg);
        if (nRc) // nRc should be zero
        {
            std::cout << "zmq msg init failed!" << std::endl;
            break;
        }

        // blocker call until receiving a message
        zmq_msg_recv(&msg, pvoidSocket, 0);

        // convert the message into a char*
        char *msg_content = (char *)zmq_msg_data(&msg);
        std::cout << "Received: " << msg_content << std::endl;

        std::string strCmdPayLoad(msg_content);

        ic_utils::Json::Reader jsonReader;
        ic_utils::Json::Value jsonPayload = ic_utils::Json::Value::nullRef;

        if (!jsonReader.parse(strCmdPayLoad, jsonPayload))
        {
            std::cout << "DeviceCmd parse error..." << strCmdPayLoad;
        }
        else
        {
            std::string strEvntID = jsonPayload["EventID"].asString();
            if (strEvntID.find("RemoteOperation") != std::string::npos)
            {
                handle_remote_operations_command(jsonPayload);
            }
        }    

        // close the zmq msg object
        zmq_msg_close(&msg);

        // if the received msg is 'quit', exit the loop
        if (0 == strcmp(msg_content, "quit"))
        {
            break;
        }
    }

    // close the zmq socket and context
    zmq_close(pvoidSocket);
    zmq_term(pvoidContext);
}


/**
 * Method to print the usage format of how this utility application
 *    can be utilized.
 * @param void
 * @return void
 */
void usage()
{
    std::cout << std::endl
              << "How to use:" << std::endl;
    std::cout << "As a Client to send a message, use below format." << std::endl;
    std::cout << "  zmq_device_simulator C <msg-to-send>" << std::endl;
    std::cout << std::endl;
    std::cout << "As a Server to listen to messages, use below format." << std::endl;
    std::cout << "  zmq_device_simulator S " << std::endl;
    std::cout << std::endl;
    std::cout << "As a Server to listen to messages " << 
                 "and respond SUCCESS/FAIL to Remote operation messages." <<
                 " use below format ." <<  std::endl;
    std::cout << "  zmq_device_simulator S ROS => to respond SUCCESS to RO messages" 
              << std::endl;
    std::cout << "  zmq_device_simulator S ROF => to respond FAIL to RO messages" 
              << std::endl;
}

/**
 * Method to parse the arguments for server usage
 * @param[in] pchArgV Command line argument array
 * @return -1 for invalid usecase, 0 otherwise
 */
int parse_arg_as_server(char *pchArgV[])
{
    int nRetValue = 0;
    if(0 == strcmp(pchArgV[RO_RESP_ARG_POS],RO_SUCCESS_ARG))
    {
        g_eROResponseToSend = eRO_RESPONSE_SUCCESS;
    }
    else if(0 == strcmp(pchArgV[RO_RESP_ARG_POS],RO_FAIL_ARG))
    {
        g_eROResponseToSend = eRO_RESPONSE_FAIL;
    }
    else
    {
        std::cout << "Invalid usage!" << std::endl;
        usage();
        nRetValue = -1;
    }
    return nRetValue;
}

/**
 * Method to parse the arguments for client usage
 * @param[in] rnArgC Command line arguments count
 * @param[in] pchArgV Command line argument array
 * @return void
 */
void parse_arg_as_client(const int &rnArgC, char *pchArgV[])
{
    if (rnArgC == ARG_COUNT_FOR_CLIENT_USAGE)
    {
        // send message as client
        client(CLIENT_URL, pchArgV[MSG_ARG_POS_FOR_CLIENT_USAGE]);
    }
    else
    {
        std::cout << "Invalid usage!" << std::endl;
        usage();
    }
}

int main(int argc, char *argv[])
{
    std::cout << "\n zmq_device_simulator : version " << VERSION << std::endl;
    int nRetVal = 0;
    if(argc > 1) // There more arguments along with the executable name
    {
        //Check for second argument if its for server usage proceed next
        if(0 == strcmp(argv[USAGE_TYPE_ARG_POS], SERVER_USAGE_OPTION))
        {
            /*
             * If argument count is 3, which means additional argument is 
             * passed with server option , then check for the 3rd argument , if 
             * it is for RO response.
             */
            if(argc == 3) 
            {
                // Returns -1 for invalid usecase, 0 otherwise
                nRetVal = parse_arg_as_server(argv);
            }

            if(nRetVal != -1)   
            {
                //Start server 
                server(SERVER_URL);
            }
             
        } //Check for second argument if its for client usage start client
        else if(0 == strcmp(argv[USAGE_TYPE_ARG_POS], CLIENT_USAGE_OPTION))
        {
            parse_arg_as_client(argc, argv);
        }
        else
        {
            std::cout << "Invalid usage!" << std::endl;
            usage();
        }
    }
    else
    {
        std::cout << "Invalid usage!" << std::endl;
        usage();
    }

    return 1;
}
