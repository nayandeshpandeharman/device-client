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
* \file CSSLAttributes.h
*
* \brief This class exposing APIs for setting or fetching SSL related
*        settings.                                                             *
********************************************************************************
*/

#ifndef CSSL_ATTRIBUTES_H
#define CSSL_ATTRIBUTES_H

#include <string>
#include <iostream>
#include "CIgniteLog.h"
#include "jsoncpp/json.h"

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CSSLAttributes"

namespace ic_network
{
/**
 * CSSLAttributes class exposing APIs for setting or fetching SSL related
 * settings.
 */
class CSSLAttributes
{
public:
    /**
     * constructor accepting individual setting values.
     * @param[in] rstrCafile  Certificate Authority file name
     * @param[in] rstrCapath  Certificate Authority file path
     * @param[in] rstrCertfile  Certificate file path
     * @param[in] rstrKeypath  Key file path
     * @param[in] rbUseTLSEngine  paramter suggesting if TLS engine to be
     *                            used or not
     * @param[in] rstrTLSEngineID  TLS Engine ID
     * @param[in] rstrKeytype  Key Type (PEM", "DER" or "ENG")
     * @param[in] rstrCerttype  certificate type
     */
    CSSLAttributes(std::string &rstrCafile, std::string &rstrCapath,
                    std::string &rstrCertfile, std::string &rstrKeypath,
                    bool &rbUseTLSEngine, std::string &rstrTLSEngineID,
                    std::string &rstrKeytype, std::string &rstrCerttype)
        : m_strCaFile(rstrCafile), m_strCaPath(rstrCapath),
            m_strCertFile(rstrCertfile), m_strKeyPath(rstrKeypath),
            m_bUseTLSEngine(rbUseTLSEngine), m_strTLSEngineID(rstrTLSEngineID),
            m_strKeyType(rstrKeytype), m_strCertType(rstrCerttype)
    {
    }

    /**
     * constructor accepting setting values in the form of string(JSON ).
     * @param[in] sslSettings The JSON String containing the ssl settings.
     * Sample :
     *  {
     *   "verifyPeer": true,
     *   "verifyHost": true,
     *   "cafile": "web-url.net",
     *   "capath": ".",
     *   "certfile": "./client.crt",
     *   "keypath": "./client.key",
     *   "useTLSEngine": true,
     *   "TLSEngineID": "ValidTLSEngineName"
     *  }
     */
    CSSLAttributes(const std::string sslSettings)
    {
        ic_utils::Json::Value jsonRoot;
        ic_utils::Json::Reader jsonReader;

        // parse json and update the member variables
        if (jsonReader.parse(sslSettings, jsonRoot))
        {
            try
            {
                m_bVerifyPeer = jsonRoot["verifyPeer"].asBool();
                m_bVerifyHost = jsonRoot["verifyHost"].asBool();
                m_strCaFile = jsonRoot["cafile"].asString();
                m_strCaPath = jsonRoot["capath"].asString();
                m_strCertFile = jsonRoot["certfile"].asString();
                m_strKeyPath = jsonRoot["keypath"].asString();
                m_strTLSEngineID = jsonRoot["TLSEngineID"].asString();
                m_bUseTLSEngine = jsonRoot["useTLSEngine"].asBool();
                if (jsonRoot.isMember("keytype") && jsonRoot["keytype"].isString())
                {
                    m_strKeyType = jsonRoot["keytype"].asString();
                }
                if (jsonRoot.isMember("certtype") && jsonRoot["certtype"].isString())
                {
                    m_strCertType = jsonRoot["certtype"].asString();
                }
            }
            catch (const std::exception &e)
            {
                HCPLOG_E << "failed to set SSL attributes";
            }
        }
        else
        {
            HCPLOG_E << "Invalid format - SSL settings";
        }
    }

    /**
     * Method to get the Certificate Authority file name.
     * @param void
     * @return CA File name
     */
    std::string GetCAfile()
    {
        return m_strCaFile;
    }

    /**
     * Method to get the path to Certificate Authority file
     * @param void
     * @return CA file path
     */
    std::string GetCApath()
    {
        return m_strCaPath;
    }

    /**
     * Method to get the path to Certificate file
     * @param void
     * @return Cert file path
     */
    std::string GetCertFile()
    {
        return m_strCertFile;
    }

    /**
     * Method to get the path to key file
     * @param void
     * @return Key file path
     */
    std::string GetKeyFile()
    {
        return m_strKeyPath;
    }

    /**
     * Method to know if TLS engine to be used or not
     * @param void
     * @return TRUE if TLS engine is to be used
     */
    bool UseTLSengine()
    {
        return m_bUseTLSEngine;
    }

    /**
     * Method to know if Peer needs to be verified.
     * @param void
     * @return TRUE if Peer needs to be verified
     */
    bool GetVerifyPeer()
    {
        return m_bVerifyPeer;
    }

    /**
     * Method to know if host to be verified.
     * @param void
     * @return TRUE if Host needs to be verified
     */
    bool GetVerifyHost()
    {
        return m_bVerifyHost;
    }

    /**
     * Method to get TLS Engine ID.
     * @param void
     * @return TLS Engine ID
     */
    std::string GetTLSengineID()
    {
        return m_strTLSEngineID;
    }

    /**
     * Method to get keyType .
     * @param void
     * @return Key Type (PEM", "DER" or "ENG")
     */
    std::string GetKeyType()
    {
        return m_strKeyType;
    }

    /**
     * Method to get certificate type.
     * @param void
     * @return Certificate type "PEM" or "DER"(CURLOPT_SSLCERTTYPE).
     */
    std::string GetCertType()
    {
        return m_strCertType;
    }

private:
    /**
     * member variable to hold the CA file name
     */
    std::string m_strCaFile = "";

    /**
     * member variable to hold the CA file path
     */
    std::string m_strCaPath = "";

    /**
     * member variable to hold certificate file path
     */
    std::string m_strCertFile = "";

    /**
     * member variable to hold key path
     */
    std::string m_strKeyPath = "";

    /**
     * member variable to hold theTLS engine ID value
     */
    std::string m_strTLSEngineID = "";

    /**
     * member variable to hold the key type
     */
    std::string m_strKeyType = "";

    /**
     * member variable to hold certificate type "PEM" or 
     * "DER"(CURLOPT_SSLCERTTYPE)
     */
    std::string m_strCertType = "";

    /**
     * member variable to indicate if TLS engine needs to be used
     */
    bool m_bUseTLSEngine = false;

    /**
     * member variable to indicate if peer needs to be verified
     */
    bool m_bVerifyPeer = true;

    /**
     * member variable to indicate if host neds to be verified
     */
    bool m_bVerifyHost = true;
};
}
#endif // CSSL_ATTRIBUTES_H
