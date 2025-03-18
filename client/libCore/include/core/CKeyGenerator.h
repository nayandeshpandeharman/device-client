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
* \file CKeyGenerator.h
*
* \brief This class provide methods to generate passcode and activation key
********************************************************************************
*/

#ifndef CKEY_GENERATOR_H
#define CKEY_GENERATOR_H

#include <string>

namespace ic_core 
{
/**
 * Class CKeyGenerator provide methods to generate passcode and activation key
 */
class CKeyGenerator 
{
public:
      /**
       * Method to get passcode key
       * @param[in] rstrDeviceID String containing deviceId
       * @return passcode key string
       */
      static std::string GetPasscodeKey(const std::string &rstrDeviceID);

      /**
       * Method to get activation key
       * @param void
       * @return activation key string
       */
      static std::string GetActivationKey();
      
      /**
       * Method to get activation qualifier key
       * @param[in] strVin String containing vin value
       * @param[in] strSerial String containing serial number
       * @return activation qualifier key string
       */
      static std::string GetActivationQualifierKey(std::string strVin,
                                                   std::string strSerial);

private:
      /**
       * Default no-argument constructor.
       */
      CKeyGenerator();

      /**
       * Destructor
       */
      ~CKeyGenerator();
};
} /* namespace ic_core */
#endif /* CKEY_GENERATOR_H */
