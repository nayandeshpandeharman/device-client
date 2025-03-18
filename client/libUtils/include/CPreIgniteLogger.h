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
* \file CPreIgniteLogger.h
*
* \brief This is responsible for printing logs before ignite logger 
* initialization
*******************************************************************************
*/

#ifndef CPREIGNITE_LOGGER_H
#define CPREIGNITE_LOGGER_H

#if defined (__ANDROID__)
#include <android/log.h>
    #define LOG_NATIVE_TAG "ACP_CLIENT_NATIVE"
    #define HCP_PRELOG_W(msg) __android_log_write(ANDROID_LOG_WARN, \
        LOG_NATIVE_TAG, msg)
#else
    #define HCP_PRELOG_W(msg) std::cout << __PRETTY_FUNCTION__ << ":" \
        << __LINE__ << " "<<msg<<std::endl
#endif

#endif /* CPREIGNITE_LOGGER_H */
