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
* \file HttpErrorCodes.h
*
* \brief This class provides the necessary interfaces to communicate with the *
*        cloud through HTTPS channel.                                         *
*******************************************************************************
*/

#ifndef HTTPERRORCODES_H_
#define HTTPERRORCODES_H_

namespace ic_network
{

/**
 * Enum of internal error codes to indicate the type of
 *    error occurred for the https communication.
 */
enum HttpErrorCode
{
    eERR_OK,                   /**< eERR_OK */
    eERR_ACCESS = -1,          /**< eERR_ACCESS */
    eERR_TOKEN = -2,           /**< eERR_TOKEN */
    eERR_NETWORK = -3,         /**< eERR_NETWORK */
    eERR_RESPONSE_FORMAT = -4, /**< eERR_RESPONSE_FORMAT */
    eERR_RESPONSE_DATA = -5,   /**< eERR_RESPONSE_DATA */
    eERR_TIMEOUT = -6,         /**< eERR_TIMEOUT */
    eERR_SERVER = -7,          /**< eERR_SERVER */
    eERR_BACKOFF = -8,         /**< eERR_BACKOFF */
    eERR_INV_INPUT = -9,       /**< eERR_INV_INPUT */
    eERR_OTHER = -999,         /**< eERR_OTHER */
    eERR_UNKNOWN = -1099       /**< eERR_UNKNOWN */
};
}

#endif //HTTPERRORCODES_H_
