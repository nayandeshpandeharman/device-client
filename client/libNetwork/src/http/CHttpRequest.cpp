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

#include "CHttpRequest.h"
#include "CIgniteLog.h"
#include "CHttpSessionManager.h"
#include "IHttpSession.h"

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CHttpRequest"

namespace ic_network
{

CHttpRequest::CHttpRequest()
{
        /* Reset all the member variables(settings)
         * which will be used later to construct the request payload
         */
        ResetAll();
}

CHttpRequest::~CHttpRequest()
{
}

void CHttpRequest::SetUrl(const string strUrl)
{
    m_strUrl = strUrl;
}

string CHttpRequest::GetUrl()
{
    return m_strUrl;
}

void CHttpRequest::SetTimeout(const int nTimeout)
{
    m_nTimeout = nTimeout;
}

const int CHttpRequest::GetTimeout()
{
    return m_nTimeout;
}

void CHttpRequest::SetPostFields(const string strPostFields)
{
    m_strPostFields = strPostFields;
}

string CHttpRequest::GetPostFields()
{
    return m_strPostFields;
}

void CHttpRequest::SetJsonType()
{
    AddHeader("Accept: application/json");
    AddHeader("Content-Type: application/json");
    AddHeader("charsets: utf-8");
}

void CHttpRequest::AddHeader(const string strHeader)
{
    m_strHeaders.push_back(strHeader);

    HCPLOG_I << "Printing headers...";
    std::list<string>::iterator iter;
    for (iter = m_strHeaders.begin(); iter != m_strHeaders.end(); iter++)
    {
        HCPLOG_I << *iter ;
    }
}

void CHttpRequest::SetHeaders(const std::list<string> strlistHeaders)
{
    m_strHeaders = strlistHeaders;
}

std::list<string> CHttpRequest::GetHeaders()
{
    return m_strHeaders;
}

void CHttpRequest::SetProxy(const string& rstrHost, const unsigned int unPort,
                            const string& rstrUser, const string& rstrPassword)
{
    m_stProxy.m_strHost = rstrHost;
    m_stProxy.m_nPort = unPort;
    m_stProxy.m_strUser = rstrUser;
    m_stProxy.m_strPassword = rstrPassword;
}

ProxySetting CHttpRequest::GetProxy()
{
    return m_stProxy;
}

void CHttpRequest::SetLocalPortRange(const unsigned int unStart, 
                                     const unsigned int unEnd)
{
    m_stPortRange.m_nStart = unStart;
    m_stPortRange.m_nEnd = unEnd;
}

LocalPortRange CHttpRequest::GetLocalPortRange()
{
    return m_stPortRange;
}

void CHttpRequest::AddFormFromBuffer(const string strFormName, 
                                    const string strContentType, 
                                    const void* pvoidBufferPtr,
                                    const long lBufferSize, bool bUploadAsFile)
{
    FormBuffer stBuff;
    stBuff.m_strFormName = strFormName;
    stBuff.m_strContentType = strContentType;
    stBuff.m_pvoidBufferPtr = (void *)pvoidBufferPtr;
    stBuff.m_lBufferSize = lBufferSize;
    stBuff.m_bUploadAsFile = bUploadAsFile;

    m_stFormBuffs.push_back(stBuff);
}

std::list<FormBuffer> CHttpRequest::GetFormBuffers()
{
    return m_stFormBuffs;
}

void CHttpRequest::AddFormFromFile(const string strFormName, 
                                   const string strContentType, 
                                   const string strFilePath,
                                   const string strUploadfilePath)
{
    FormFile stFrmFiles;
    stFrmFiles.m_strFormName = strFormName;
    stFrmFiles.m_strContentType = strContentType;
    stFrmFiles.m_strFilePath = strFilePath;
    stFrmFiles.m_strAttachmentUploadName = strUploadfilePath;

    m_stFormFiles.push_back(stFrmFiles);
}

std::list<FormFile> CHttpRequest::GetFormFiles()
{
    return m_stFormFiles;
}

void CHttpRequest::ClearHeaders()
{
    m_strHeaders.clear();
}

void CHttpRequest::ClearFormBuffers()
{
    m_stFormBuffs.clear();
}

void CHttpRequest::ClearFormFiles()
{
    m_stFormFiles.clear();
}

void CHttpRequest::ResetAll()
{
    m_strUrl = "";
    m_nTimeout = -1;
    m_strPostFields = "";
    m_strHeaders.clear();
    m_stProxy.Reset();
    m_stPortRange.Reset();
    m_stFormBuffs.clear();
    m_stFormFiles.clear();
}

HttpErrorCode CHttpRequest::ExecuteGET(CHttpResponse &rResp)
{
    HCPLOG_METHOD();

    HttpErrorCode eRet = HttpErrorCode::eERR_UNKNOWN;

    IHttpSession *pHTTPsession = 
                 CHttpSessionManager::GetInstance()->AcquireSession();
    if (NULL == pHTTPsession)
    {
        HCPLOG_E << "Error: could not get a http session!";
        return eRet;
    }

    //url
    pHTTPsession->SetUrl(m_strUrl);
    HCPLOG_I << "URL=" << m_strUrl;

    //timeout
    int nTimeout = m_nTimeout;
    if (nTimeout > 0)
    {
        pHTTPsession->SetTimeout(nTimeout);
        HCPLOG_I << "Timeout=" << nTimeout;
    }

    //postfield
    string strPfield = GetPostFields();
    if ("" != strPfield)
    {
        pHTTPsession->SetPostFields(strPfield);
        HCPLOG_I << "PostField=" << strPfield;
    }

    //headers
    std::list<string>::iterator iter;
    std::list<string> strlistHdrs = GetHeaders();
    for (iter = strlistHdrs.begin(); iter != strlistHdrs.end(); iter++)
    {
        string strHdr = *iter;
        pHTTPsession->AddHeader(strHdr);
        HCPLOG_I << "Header=" << strHdr;
    }

    //proxy
    ProxySetting stProxy = GetProxy();
    if (("" != stProxy.m_strHost) && (stProxy.m_nPort > 0))
    {
        pHTTPsession->SetProxy(stProxy.m_strHost, stProxy.m_nPort, 
                               stProxy.m_strUser, stProxy.m_strPassword);
        HCPLOG_I << "Host=" << stProxy.m_strHost << "; Port=" << stProxy.m_nPort
                 << "; User=" << stProxy.m_strUser << "; Passwd=" <<
                 stProxy.m_strPassword;
    }

    //portrange
    LocalPortRange stPortRange = GetLocalPortRange();
    if ((stPortRange.m_nStart > 0) && (stPortRange.m_nEnd > 0))
    {
        pHTTPsession->SetLocalPortRange(stPortRange.m_nStart, 
                                        stPortRange.m_nEnd);
        HCPLOG_I << "Portrange: start=" << stPortRange.m_nStart << "; end=" <<
                 stPortRange.m_nEnd;
    }

    HCPLOG_I << "sending request...";

    if ((eRet = pHTTPsession->PerformGETRequest()) == HttpErrorCode::eERR_OK)
    {
        HCPLOG_I << "Http request send SUCCESSFUL!";
        HCPLOG_I << "Http code=" << pHTTPsession->GetHttpCode();
        HCPLOG_I << "Http Data=" << pHTTPsession->GetData();
        HCPLOG_I << "Http LastError=" << pHTTPsession->GetLastError();
    }
    else
    {
        HCPLOG_I << "Http request send FAILED!";
    }

    //send the response
    rResp.SetHttpCode(pHTTPsession->GetHttpCode());
    rResp.SetHttpResponseHeader(pHTTPsession->GetHttpResponseHeader());
    rResp.SetRespData(pHTTPsession->GetData());
    rResp.SetLastError(pHTTPsession->GetLastError());

    CHttpSessionManager::GetInstance()->ReleaseSession(pHTTPsession);

    return eRet;
}

HttpErrorCode CHttpRequest::Execute(CHttpResponse &rResp)
{
    HCPLOG_METHOD();

    HttpErrorCode eRet = HttpErrorCode::eERR_UNKNOWN;

    IHttpSession *pHTTPsession = 
                  CHttpSessionManager::GetInstance()->AcquireSession();
    if (NULL == pHTTPsession)
    {
        HCPLOG_E << "Error: could not get a http session!";
        return eRet;
    }

    //url
    pHTTPsession->SetUrl(m_strUrl);
    HCPLOG_I << "URL=" << m_strUrl;

    //timeout
    int nTimeout = m_nTimeout;
    if (nTimeout > 0)
    {
        pHTTPsession->SetTimeout(nTimeout);
        HCPLOG_I << "Timeout=" << nTimeout;
    }

    //postfield
    string strPfield = GetPostFields();
    if ("" != strPfield)
    {
        pHTTPsession->SetPostFields(strPfield);
        HCPLOG_I << "PostField=" << strPfield;
    }

    //headers
    std::list<string>::iterator iter;
    std::list<string> strlistHdrs = GetHeaders();
    for (iter = strlistHdrs.begin(); iter != strlistHdrs.end(); iter++)
    {
        string strHdr = *iter;
        pHTTPsession->AddHeader(strHdr);
        HCPLOG_I << "Header=" << strHdr;
    }

    //proxy
    ProxySetting stProxy = GetProxy();
    if (IsProxySettingsValid(stProxy))
    {
        pHTTPsession->SetProxy(stProxy.m_strHost, stProxy.m_nPort, 
                               stProxy.m_strUser, stProxy.m_strPassword);
        HCPLOG_I << "Host=" << stProxy.m_strHost << "; Port=" << 
                stProxy.m_nPort << "; User=" << stProxy.m_strUser << 
                "; Passwd=" << stProxy.m_strPassword;
    }

    //portrange
    LocalPortRange stPortRange = GetLocalPortRange();
    if (IsPortRangeValid(stPortRange))
    {
        pHTTPsession->SetLocalPortRange(stPortRange.m_nStart, 
                                        stPortRange.m_nEnd);
        HCPLOG_I << "Portrange: start=" << stPortRange.m_nStart << 
                 "; end=" << stPortRange.m_nEnd;
    }

    //form buffers
    std::list<FormBuffer>::iterator bIter;
    std::list<FormBuffer> stlistFbuffers = GetFormBuffers();
    for (bIter = stlistFbuffers.begin(); bIter != stlistFbuffers.end(); bIter++)
    {
        FormBuffer stFbuff = *bIter;
        pHTTPsession->AddFormFromBuffer(stFbuff.m_strFormName, 
                                        stFbuff.m_strContentType, 
                                        stFbuff.m_pvoidBufferPtr, 
                                        stFbuff.m_lBufferSize, 
                                        stFbuff.m_bUploadAsFile);
        HCPLOG_I << "Formname=" << stFbuff.m_strFormName << "; ContentType=" <<
                 stFbuff.m_strContentType << "; BufferSize=" << 
                 stFbuff.m_lBufferSize << "uploadAsFile=" << 
                 stFbuff.m_bUploadAsFile;
    }

    //form files
    std::list<FormFile>::iterator fIter;
    std::list<FormFile> stlistFfiles = GetFormFiles();
    for (fIter = stlistFfiles.begin(); fIter != stlistFfiles.end(); fIter++)
    {
        FormFile stFfile = *fIter;
        pHTTPsession->AddFormFromFile(stFfile.m_strFormName, 
                                      stFfile.m_strContentType, 
                                      stFfile.m_strFilePath, 
                                      stFfile.m_strAttachmentUploadName);
        HCPLOG_I << "FormName=" << stFfile.m_strFormName << "; ContentType=" <<
                 stFfile.m_strContentType << "Filepath=" <<stFfile.m_strFilePath
                  << " Uploadpath:" << stFfile.m_strAttachmentUploadName;
    }

    HCPLOG_I << "sending request...";

    if ((eRet = pHTTPsession->PerformRequest()) == HttpErrorCode::eERR_OK)
    {
        HCPLOG_I << "Http request send SUCCESSFUL!";
        HCPLOG_I << "Http code=" << pHTTPsession->GetHttpCode();
        HCPLOG_I << "Http Data=" << pHTTPsession->GetData();
        HCPLOG_I << "Http LastError=" << pHTTPsession->GetLastError();
    }
    else
    {
        HCPLOG_I << "Http request send FAILED!";
    }

    //send the response
    rResp.SetHttpCode(pHTTPsession->GetHttpCode());
    rResp.SetHttpResponseHeader(pHTTPsession->GetHttpResponseHeader());
    rResp.SetRespData(pHTTPsession->GetData());
    rResp.SetLastError(pHTTPsession->GetLastError());

    CHttpSessionManager::GetInstance()->ReleaseSession(pHTTPsession);

    return eRet;
}

bool CHttpRequest::IsProxySettingsValid(const ProxySetting &rstProxy)
{
    return (("" != rstProxy.m_strHost) && (rstProxy.m_nPort > 0));
}

bool CHttpRequest::IsPortRangeValid(const LocalPortRange &rstPortRange)
{
    return ((rstPortRange.m_nStart > 0) && (rstPortRange.m_nEnd > 0));
}
}
