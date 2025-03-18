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

#ifndef DEVICE_CLIENT_APP_VERSION_H
#define DEVICE_CLIENT_APP_VERSION_H

/**
 * Macro DEVICE_CLIENT_APP_VERSION should be defined in the CMakeLists.txt.
 * If not defined, define here as "0.0.0".
 */
#if !defined(DEVICE_CLIENT_APP_VERSION)
#define DEVICE_CLIENT_APP_VERSION "0.0.0"
#endif /* DEVICE_CLIENT_APP_VERSION */

/**
 * Macro for IgniteClientApp version with date and time info
 */
#define DEVICE_CLIENT_APP_VERSION_TS DEVICE_CLIENT_APP_VERSION " " \
    __DATE__ " " __TIME__

#endif /* DEVICE_CLIENT_APP_VERSION_H */
