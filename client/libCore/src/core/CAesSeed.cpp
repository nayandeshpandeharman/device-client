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

#include "CIgniteConfig.h"
#include "db/CLocalConfig.h"
#include "CIgniteLog.h"
#include "core/CAesSeed.h"
#include "db/CDataBaseFacade.h"

//! Macro for CAesSeed string
#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CAesSeed"

namespace ic_core 
{
//! Constant key for 'android' string
const std::string KEYSTORE_ANDROID = "android";

CAesSeed::CAesSeed() 
{

}

std::string CAesSeed::GetIvRandom() 
{
    //Local config generates a seed if not available.
    if (m_bUseKeystoreAndroid)
    {
        return m_strSeedContent;
    }
    else
    {
        return CLocalConfig::GetInstance()->GetIvRandomNumber(m_strSeedContent);
    }
}

void CAesSeed::Init(std::string strContent, bool bIsChanged) 
{
    HCPLOG_T << "Init. Content" << strContent;
    m_strSeedContent = strContent;

    std::string strKeystore = CIgniteConfig::GetInstance()->GetString(
                                                       "DAM.Database.keystore");
    if (strKeystore == KEYSTORE_ANDROID && !m_strSeedContent.empty()) 
    {
        //pad it with 'X' for length 16. Key length must be minimum 16
        if (m_strSeedContent.length() < 16)
        {
            m_strSeedContent.insert(m_strSeedContent.end(), 16 - 
                                    m_strSeedContent.length(), 'X');
        }

        if (bIsChanged)
        {
            //delete existing events in db
            CDataBaseFacade::GetInstance()->Remove(
                                            CDataBaseConst::TABLE_EVENT_STORE);
        }

        m_bUseKeystoreAndroid = true;
        HCPLOG_C << "Using android keystore";
    }
    else
    {
        //Use local keystore.
        m_bUseKeystoreAndroid = false;
        HCPLOG_C << "Using local keystore";
    }
}

CAesSeed *CAesSeed::GetInstance() 
{
    static CAesSeed instance;
    return &instance;
}
}
