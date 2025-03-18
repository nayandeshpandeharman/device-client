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

#include <time.h>
#include <cstdlib>
#include "CIgniteLog.h"
#include "crypto/CRNG.h"

namespace ic_core 
{
int CRNG::Get(std::string strSeedKey, int nMin, int nMax)
{
    HCPLOG_METHOD();
    /*
     * Apply strSeedKey bytes by XORing them with time-stamp based seed to 
     * ensure it's unique
     */ 
    std::string strLocalSeedKey = strSeedKey;

    unsigned int unSeed = time(NULL);
    int nShift = 0;
    for (int i = 0; i < strLocalSeedKey.length(); i++)
    {
        char ch = strLocalSeedKey[i];
        unSeed = unSeed ^ (ch << (nShift * 8));
        if (++nShift >= sizeof(unSeed))
        {
            nShift = 0;
        }
    }
    std::srand(unSeed);
    return nMin + std::rand() % (nMax - nMin + 1);
}

int CRNG::Get(std::string strSeedKey, int nMax) 
{
    return Get(strSeedKey, 0, nMax);
}

std::string CRNG::GetString(std::string strSeedKey, int nMin, int nMax)
{
    std::stringstream sstm;
    sstm << CRNG::Get(strSeedKey, nMin, nMax);
    return sstm.str();
}

std::string CRNG::GetString(std::string strSeedKey, int nMax) {
    return GetString(strSeedKey, 0, nMax);
}
} /* namespace ic_core */
