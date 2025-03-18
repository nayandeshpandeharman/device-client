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
* \file CHttpSessionManager.h
*
* \brief This class manages the HttpRequest sessions in terms of initializing, *
*          execution and cleanup.                                              *
********************************************************************************
*/

#ifndef CHTTP_SESSION_MANAGER_H
#define CHTTP_SESSION_MANAGER_H

#include "IHttpSession.h"
#include "CSSLAttributes.h"
#include "IExternalHttpSessionHandler.h"

namespace ic_network
{

/**
 * CHttpSessionManager class exposing APIs to create a HTTP session.
 */
class CHttpSessionManager
{
public:
    /**
     * Method to get Instance of CHttpSessionManager
     * @param  void
     * @return A Pointer to a CHttpSessionManager object
     */
    static CHttpSessionManager *GetInstance();

    /**
     * Method to release the instance.
     * @param  void
     * @return void
     */
    static void ReleaseInstance();

    /**
     * Method to release the internal resources
     * @param  void
     * @return void
     */
    void ReleaseResources();

    /**
     * Method to acquire a HTTP session.
     * @param void
     * @return A Pointer to the acquired HTTP session
     */
    IHttpSession *AcquireSession();

    /**
     * Method to release the session
     * @param[in] pSession A pointer to the curlHttpSession(IHttpSession),
     *            that needs to be released
     * @return void
     */
    void ReleaseSession(const IHttpSession *pSession);

    /**
     * Method to set the proxy details.
     * @param[in] rstrHost  the host name
     * @param[in] unPort  the port number
     * @param[in] rstrUser  user name (default is empty)
     * @param[in] rstrPassword Password (default is empty)
     * @return void
     */
    void SetProxy(const std::string &rstrHost, 
                    const unsigned int unPort,
                    const std::string &rstrUser = "", 
                    const std::string &rstrPassword = "");

    /**
     * Method to set Local Port Range
     * (CURLOPT_LOCALPORTRANGE-number of additional local ports to try)
     * @param[in] unStart Range Start
     * @param[in] unEnd Range End
     * @return void
     */
    void SetLocalPortRange(const unsigned int unStart,
                            const unsigned int unEnd);

    /**
     * Method to set the SSL attributes.
     * @param[in] pSslAttributes A pointer to the sslAttributes class object,
     *                             which contains the values to set
     * @return void
     */
    void SetSSLAttributes(CSSLAttributes *pSslAttributes);

    /**
     * Method to set instance of External HTTP Session Handler interface
     * @param[in] pHandler instance of IExternalHttpSessionHandler
     * @return void
     */
    void SetExternalHttpSessionHandler(IExternalHttpSessionHandler *pHandler);

#ifdef IC_UNIT_TEST
    /**
     * Method to get the local port range set.
     * @param[out] unStartValueSet port range start
     * @param[out] unEndValueSet port range end
     * @return void
     */
    void GetLocalPortRange(unsigned int &unStartValueSet,
                           unsigned int &unEndValueSet)
    {
        unStartValueSet = m_unPortRangeStart;
        unEndValueSet = m_unPortRangeEnd;
    }

    /**
     * Method to get the proxy values set.
     * @param[out] rstrHostSet host value 
     * @param[out] runPortSet port value
     * @param[out] rstrUserSet user name
     * @param[out] rstrPasswordSet password
     * @return void
     */
    void GetProxyValues(std::string &rstrHostSet,
                           unsigned int &runPortSet,
                           std::string &rstrUserSet,
                           std::string &rstrPasswordSet)
    {
        rstrHostSet = m_strProxy ;
        runPortSet = m_unProxyPort;
        rstrUserSet=  m_strProxyUser ;
        rstrPasswordSet = m_strProxyPwd ;
    }

#endif

private:
    /**
     * Default no-argument constructor.
     */
    CHttpSessionManager();

    /**
     *  Destructor.
     */
    ~CHttpSessionManager();

    /**
     * member variable to value of maximum sessions allowed
     */
    static const int MAX_SESSIONS = 8;

    /**
     * member variable IHttpSession array of pointers to holder pointers of
     * each session created
     */
    IHttpSession *m_pSession[MAX_SESSIONS];

    /**
     * member variable boolean array indicating if session acquired or not
     */
    bool m_bSessionAcquired[MAX_SESSIONS];

    /**
     * member variable to hold the number of sessions acquired
     */
    unsigned int m_unNumSessionsAcquired;

    /**
     * member variable to details of last session number when threshold
     * alert event is generated
     */
    unsigned int m_unSessionWarningSentAt;

    /**
     * member variable to hold the proxy host name
     */
    std::string m_strProxy;

    /**
     * member variable to hold the proxy port number
     */
    unsigned int m_unProxyPort;

    /**
     * member variable to hold the proxy user name
     */
    std::string m_strProxyUser;

    /**
     * member variable to hold the proxy password
     */
    std::string m_strProxyPwd;

    /**
     * member variable to hold the value of local porst range start
     */
    unsigned int m_unPortRangeStart;

    /**
     * member variable to hold the  value of local porst range end
     */
    unsigned int m_unPortRangeEnd;

    IExternalHttpSessionHandler *m_pExtHttpSessionHandler = nullptr;

    /**
     * member variable to hold the SSL Attribute details
     */
    static CSSLAttributes *m_pSslAttributes;

    /**
     * Method to check if the number of sessions is exceeding the threshold
     * of (MAX_SESSIONs -2) and previous warning alert is not sent for this
     * number of sessions.
     * @param void
     * @return void
     */
    void CheckForSessionThresholdWarning(void);

    /**
     * Method to create a http session for the index 'nIndex', if not 
     * created already.
     * @param[in] nIndex index in the sessions array for which session needs
     *          to be created.
     * @return void
     */
    void CreateSession(const int nIndex);
};

} // namespace ic_network
#endif /* CHTTP_SESSION_MANAGER_H */
