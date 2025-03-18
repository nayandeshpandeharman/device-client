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
* \file CExternalAppMsgTypes.h
*
* \brief This class/module defines message type constants used by external app
********************************************************************************
*/

#ifndef CEXTERNAL_APP_MSG_TYPES_H
#define CEXTERNAL_APP_MSG_TYPES_H

namespace ic_core 
{
/**
 * Macro for connection message type 
 */
#define EXT_APP_MESSAGE_TYPE_CONNECTION  1

/**
 * Macro for configuration message type 
 */
#define EXT_APP_MESSAGE_TYPE_CONFIGURATION 2

/**
 * Macro for ignite status message type 
 */
#define EXT_APP_MESSAGE_TYPE_IGNITE_STATUS 3

/**
 * Macro for activation message type 
 */
#define EXT_APP_MESSAGE_TYPE_ACTIVATION 4

/**
 * Macro for firmware upgrade message type 
 */
#define EXT_APP_MESSAGE_FIRMWARE_UPGRADE 5

/**
 * Macro for notification message type
 */
#define EXT_APP_MESSAGE_TYPE_NOTIFICATION 6

/**
 * Constant key for 'message' string
 */
static const std::string EXT_APP_KEY_MESSAGE = "message";

/**
 * Constant key for 'IsEncoded' string
 */
static const std::string EXT_APP_KEY_IS_ENCODED = "IsEncoded";

} /* namespace ic_core */
#endif //CEXTERNAL_APP_MSG_TYPES_H