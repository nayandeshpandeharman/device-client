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

#include <fstream>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <stdio.h>
#include <zlib.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include "CIgniteFileUtils.h"
#include "CIgniteLog.h"
#include "CIgniteStringUtils.h"
#include "CIgniteFileUtils.h"
#include "CIgniteLog.h"
#include "CIgniteStringUtils.h"

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX "CIgniteFileUtils"

namespace ic_utils 
{
const std::string g_strBase64Chars =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";

int CIgniteFileUtils::Copy(const std::string &rstrFrom, 
    const std::string &rstrTo)
{
    HCPLOG_METHOD() << " from=" << rstrFrom << ", to=" << rstrTo;

    int nReturnValue = -1;
    std::ifstream in(rstrFrom.c_str(), std::ios::binary);
    std::ofstream out(rstrTo.c_str(), std::ios::binary);
    
    if(in.is_open() && out.is_open())
    {
        out << in.rdbuf();
        nReturnValue = 0;
    }
    out.close();
    in.close();
    return nReturnValue;
}

int CIgniteFileUtils::Remove(const std::string &rstrFilename)
{
    HCPLOG_METHOD() << " fileName=" << rstrFilename;

    if (std::remove(rstrFilename.c_str()))
    {
        HCPLOG_METHOD() << " file=" << rstrFilename << ", ReturnValue=-1";
        return -1;
    }
    HCPLOG_METHOD() << " file=" << rstrFilename << ", ReturnValue=0";
    return 0;
}

bool CIgniteFileUtils::Exists(const std::string &rstrFilename)
{
    HCPLOG_METHOD() << " fileName=" << rstrFilename;
    std::ifstream iFile(rstrFilename.c_str());
    return iFile.good();
}

int CIgniteFileUtils::GetSize(const std::string &rstrFilename)
{
    HCPLOG_METHOD() << " fileName=" << rstrFilename;
    std::ifstream iFile(rstrFilename.c_str(), std::ios::binary | std::ios::ate);
    
    if (!iFile.good())
    {
        return -1;
    }
    return iFile.tellg();
}

int CIgniteFileUtils::ListFiles(const std::string &rstrDirName, 
    std::vector<std::string> &rvectorFileList)
{
    HCPLOG_METHOD() << " dirName=" << rstrDirName;

    DIR *dirp;
    int nReturnValue = -1;

    dirp = opendir(rstrDirName.c_str());
    
    if (dirp != NULL)
    {
        struct dirent *pstDirentp;
        
        while ((pstDirentp = readdir(dirp)) != NULL)
        {
            if (std::strncmp(pstDirentp->d_name, ".", 1) && 
                std::strncmp(pstDirentp->d_name, "..", 2))
            {
                // Push any files that are not "." or ".."
                rvectorFileList.push_back(std::string(pstDirentp->d_name));
            }
        }
        closedir( dirp );
        nReturnValue = 0;
    }

    return nReturnValue;
}

/**
 * Global function to report a zlib error.
 * @param[in] nZErrCode Zlib error code.
 * @return void.
 */
void z_err(const int nZErrCode)
{    
    switch (nZErrCode)
    {
        case Z_ERRNO:
            HCPLOG_E << "[" << nZErrCode << "] Error reading/writng a file";
            break;
        case Z_STREAM_ERROR:
            HCPLOG_E << "[" << nZErrCode << "] Invalid compression level";
            break;
        case Z_DATA_ERROR:
            HCPLOG_E << "[" << nZErrCode << "] Invalid or incomplete deflate \
                        data";
            break;
        case Z_MEM_ERROR:
            HCPLOG_E << "[" << nZErrCode << "] Out of memory.";
            break;
        case Z_VERSION_ERROR:
            HCPLOG_E << "[" << nZErrCode << "] Zlib version mismatch.";
            break;
        default: 
            HCPLOG_E << "[" << nZErrCode << "] Error: other.";
    }
}

int CIgniteFileUtils::Compress(const std::string &rstrSrcPath)
{
    return Compress(rstrSrcPath, rstrSrcPath + ".gz");
}

int CIgniteFileUtils::Compress(const std::string &rstrSrcPath, 
                               const std::string &rstrDestPath)
{
    HCPLOG_METHOD() << " fileName=" << rstrSrcPath;
    int nReturnValue;
    FILE *pstSrcFile, *pstDestFile;
    pstSrcFile = fopen (rstrSrcPath.c_str(), "rb" );
    
    if (pstSrcFile == NULL)
    {
        HCPLOG_METHOD() << " compress source file could not be opened" 
            << rstrSrcPath;
        nReturnValue = Z_ERRNO;
        z_err(nReturnValue);
        return nReturnValue;
    }
    pstDestFile = fopen ( rstrDestPath.c_str() , "wb+" );
    
    if (pstDestFile == NULL)
    {
        HCPLOG_METHOD() << " compress destination path can not be accesed"
            << rstrDestPath;
        // clean up and return
        nReturnValue = Z_ERRNO;
        fclose(pstSrcFile);
        z_err(nReturnValue);
        return nReturnValue;
    }
    z_stream stStrm;
    // allocate deflate state
    stStrm.zalloc = Z_NULL;
    stStrm.zfree = Z_NULL;
    stStrm.opaque = Z_NULL;

    nReturnValue = deflateInit2(&stStrm, Z_DEFAULT_COMPRESSION, 
                                Z_DEFLATED, MAX_WBITS + 16, 
                                MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY);

    if (nReturnValue != Z_OK)
    {
        fclose(pstSrcFile);
        fclose(pstDestFile);
        z_err(nReturnValue);
        return nReturnValue;
    }
    nReturnValue = CompressFile(stStrm, pstSrcFile, pstDestFile);

    if (nReturnValue == Z_STREAM_END || nReturnValue == Z_OK )
    {
        HCPLOG_T << "File Compressed to :" << 
            CIgniteFileUtils::GetSize(rstrDestPath)
            << "from :" << CIgniteFileUtils::GetSize(rstrSrcPath);
        nReturnValue = Z_OK;
    }
    else
    {
        HCPLOG_E << "File Compression Unsuccessful!";
        CIgniteFileUtils::Remove(rstrDestPath);
    }

    // clean up and return
    (void)deflateEnd(&stStrm);
    fclose (pstSrcFile);
    fclose (pstDestFile);

    z_err(nReturnValue);
    return nReturnValue;
}

int CIgniteFileUtils::CompressFile(z_stream &rstStrm, FILE *pstSrcFile, 
                                   FILE *pstDestFile)
{
    int nFlush;
    unsigned char *puchIn = new unsigned char[CHUNK];
    int nReturnValue;

    //  compress until end of file
    do
    {
        rstStrm.avail_in = fread(puchIn, 1, CHUNK, pstSrcFile);

        if (ferror(pstSrcFile))
        {
            nReturnValue = Z_ERRNO;
            break;
        }

        nFlush = feof(pstSrcFile) ? Z_FINISH : Z_NO_FLUSH;
        rstStrm.next_in = puchIn;
        nReturnValue = CompressChunk(rstStrm, pstDestFile, nFlush);
        
        if (rstStrm.avail_in != 0)  // all input will be used
        {
            nReturnValue = Z_DATA_ERROR;
            break;
        }
        // done when last data in file processed
    } 
    while (nFlush != Z_FINISH);

    if (puchIn)
    {
        delete[] puchIn;
    }

    return nReturnValue;
}

int CIgniteFileUtils::CompressChunk(z_stream &rstStrm, FILE *pstDestFile, 
                                    const int nFlush)
{
    unsigned int nReqSize = deflateBound(&rstStrm, rstStrm.avail_in);
    unsigned char *puchOut = new unsigned char[nReqSize];
    int nReturnValue;

    /* run deflate() on input until output buffer not full, finish
     * compression if all of source has been read in 
     */
    do
    {
        rstStrm.avail_out = nReqSize;
        rstStrm.next_out = puchOut;
        nReturnValue = deflate(&rstStrm, nFlush);

        if (nReturnValue == Z_STREAM_ERROR)
        {
            break;
        }

        unsigned uHave = nReqSize - rstStrm.avail_out;

        if ((fwrite(puchOut, 1, uHave, pstDestFile) != uHave) || 
            ferror(pstDestFile))
        {
            nReturnValue = Z_ERRNO;
            break;
        }
    }
    while (rstStrm.avail_out == 0 );
        
    if (puchOut)
    {
        delete [] puchOut;
    }
    return nReturnValue;
}

bool CIgniteFileUtils::MakeDirectory(std::string &rstrDirPath)
{
    HCPLOG_METHOD() << "path: " << rstrDirPath;
    std::string strCommand = "mkdir -p " + rstrDirPath;
    return EXIT_SUCCESS == system(strCommand.c_str());
}

bool CIgniteFileUtils::RemoveDirectory(std::string &rstrDirPath)
{
    HCPLOG_METHOD() << "path: " << rstrDirPath;
    std::string strCommand = "rm -r " + rstrDirPath;
    return EXIT_SUCCESS == system(strCommand.c_str());
}

bool CIgniteFileUtils::Move(std::string &rstrSrc, std::string &rstrDest)
{
    HCPLOG_METHOD() << "src: " << rstrSrc << ", dest: " << rstrDest;
    std::string strCommand = "mv " + rstrSrc + " " + rstrDest;
    return EXIT_SUCCESS == system(strCommand.c_str());
}

bool CIgniteFileUtils::StringEndsWith(std::string const &rstrvalue, 
    std::string const &rstrEnding)
{
    if (rstrEnding.size() > rstrvalue.size())
    {
        return false;
    }
    return std::equal(rstrEnding.rbegin(), rstrEnding.rend(), 
        rstrvalue.rbegin());
}

void CIgniteFileUtils::ListFiles(const std::string &rstrdir,  
                                 const std::string strSuffix, 
                                 std::list<std::string> &rlistFileList)
{
    DIR *pstDp = NULL;
    struct dirent *pstEp;

    pstDp = opendir(rstrdir.c_str());

    if (pstDp != NULL)
    {
        while ((pstEp = readdir(pstDp)) != NULL)
        {
            //printf("Filename: %s\n",epdf->d_name);
            std::string strFileName = pstEp->d_name;

            if (strFileName[0] == '.')
            {
                continue;
            }

            if(StringEndsWith(strFileName, strSuffix))
            {
                HCPLOG_T << "listFiles~file~" << strFileName;
                rlistFileList.push_back(strFileName);
            }
        }
        closedir(pstDp);
    }

    rlistFileList.sort();
    HCPLOG_T << "Listed Files==>";

    for(std::list<std::string>::iterator listIter = rlistFileList.begin();
        listIter != rlistFileList.end();++listIter)
    {
        HCPLOG_T << *listIter;
    }

}

inline bool is_base64(unsigned char uchBaseChar) 
{
    return (isalnum(uchBaseChar) || (uchBaseChar == '+') || 
        (uchBaseChar == '/'));
}

std::string CIgniteFileUtils::Base64Encode(
    unsigned char const *puchBytesToEncode, 
    unsigned int unInputlength)
{
    std::string strReturnValue;
    int nEncodeStrIndex = 0;
    unsigned char ucstrBytesArray[BYTES_GRP];   
    unsigned char ucstrCharArray[CHAR_GRP];

    while (unInputlength--) 
    {
        ucstrBytesArray[nEncodeStrIndex++] = *(puchBytesToEncode++);
        if (BYTES_GRP == nEncodeStrIndex) 
        {
            ucstrCharArray[0] = (ucstrBytesArray[0] & 0xfc) >> 2;
            ucstrCharArray[1] = ((ucstrBytesArray[0] & 0x03) << 4) + 
                                ((ucstrBytesArray[1] & 0xf0) >> 4);
            ucstrCharArray[2] = ((ucstrBytesArray[1] & 0x0f) << 2) + 
                                ((ucstrBytesArray[2] & 0xc0) >> 6);
            ucstrCharArray[3] = ucstrBytesArray[2] & 0x3f;
        
            /* Convert each character using g_strBase64Chars and append
             * to strReturnValue
             */ 
            for(int nIndex = 0; (nIndex <CHAR_GRP) ; nIndex++)
            {
                strReturnValue += g_strBase64Chars[ucstrCharArray[nIndex]];
            }
            nEncodeStrIndex = 0;
        }
    }

    if (nEncodeStrIndex)
    {
        // Pad the remaining characters in ucstrBytesArray with zeros
        for(int nPadIndex = nEncodeStrIndex; nPadIndex < BYTES_GRP; nPadIndex++)
        {
            ucstrBytesArray[nPadIndex] = '\0';
        }

        ucstrCharArray[0] = (ucstrBytesArray[0] & 0xfc) >> 2;
        ucstrCharArray[1] = ((ucstrBytesArray[0] & 0x03) << 4) + 
                            ((ucstrBytesArray[1] & 0xf0) >> 4);
        ucstrCharArray[2] = ((ucstrBytesArray[1] & 0x0f) << 2) + 
                            ((ucstrBytesArray[2] & 0xc0) >> 6);
        ucstrCharArray[3] = ucstrBytesArray[2] & 0x3f;

        for (int nIndex = 0; (nIndex < nEncodeStrIndex + 1); nIndex++)
        {
            strReturnValue += g_strBase64Chars[ucstrCharArray[nIndex]];
        }

        while((nEncodeStrIndex++ < BYTES_GRP))
        {
            strReturnValue += '=';
        }

    }
    return strReturnValue;
}

std::string CIgniteFileUtils::Base64Decode(std::string const &rstrEncodedString)
{
    int nInputlength = rstrEncodedString.size();
    int nDecodeStrIndex = 0;
    int nIndex = 0;
    unsigned char ucstrCharArray[CHAR_GRP];
    unsigned char ucstrBytesArray[BYTES_GRP];
    std::string strReturnValue;

    while (nInputlength-- && ( rstrEncodedString[nIndex] != '=') && 
        is_base64(rstrEncodedString[nIndex])) 
    {
        ucstrCharArray[nDecodeStrIndex++] = rstrEncodedString[nIndex]; 
        nIndex++;

        if (CHAR_GRP == nDecodeStrIndex) 
        {
            Decode(ucstrBytesArray, ucstrCharArray);

            // Append the decoded bytes to the result string
            for (int nIndex = 0; (nIndex < BYTES_GRP); nIndex++)
            {
                strReturnValue += ucstrBytesArray[nIndex];
            }
            nDecodeStrIndex = 0; // Reset the index for the next group of 4 char
        }
    }

    if (nDecodeStrIndex) 
    {
        // Pad the remaining characters in ucstrCharArray with zeros
        for (int nIndex = nDecodeStrIndex; nIndex <CHAR_GRP; nIndex++)
        {
            ucstrCharArray[nIndex] = 0;
        }

        Decode(ucstrBytesArray, ucstrCharArray);

        // Append the decoded bytes (except the last one) to the result string
        for (int nIndex = 0; (nIndex < nDecodeStrIndex - 1); nIndex++) 
        {
            strReturnValue += ucstrBytesArray[nIndex];
        }
    }
    return strReturnValue;
}

void CIgniteFileUtils::Decode(unsigned char *pucstrBytesArray,
                              unsigned char *pucstrCharArray)
{
    // Convert each character in using the Base64 encoding table and store
    for (int nIndex = 0; nIndex < CHAR_GRP; nIndex++)
    {
        pucstrCharArray[nIndex] = 
            g_strBase64Chars.find(pucstrCharArray[nIndex]);
    }

    // Decode chars to bytes

    /* Shift the bits of the first Base64 character left by 2 positions to 
     * align with the higher-order bits of the first decoded byte.
     * Extract and shift the relevant bits from the second Base64 character 
     * to align with the lower-order bits of the first decoded byte.
     */
    pucstrBytesArray[0] = (pucstrCharArray[0] << 2) + 
                          ((pucstrCharArray[1] & 0x30) >> 4);

    /* Extract and shifts the relevant bits from the second Base64 character 
     * to occupy the higher-order bits of the second decoded byte.
     * Extract and shift the relevant bits from the third Base64 character 
     * to align with the lower-order bits of the second decoded byte.
     */    
    pucstrBytesArray[1] = ((pucstrCharArray[1] & 0xf) << 4) + 
                          ((pucstrCharArray[2] & 0x3c) >> 2);

    /* Extract and shift the relevant bits from the third Base64 character 
     * to occupy the higher-order bits of the third decoded byte.
     * Forth char directly contributes its bits to occupy the lower-order bits  
     * of the third decoded byte.
     */    
    pucstrBytesArray[2] = ((pucstrCharArray[2] & 0x3) << 6) + 
                            pucstrCharArray[3];     
}       

void CIgniteFileUtils::ReadFile(std::string &rstrFilename, 
                                std::string &rstrData)
{
    static const int nBUF_SIZE = 100;
    HCPLOG_T << rstrFilename;
    std::string strFname = rstrFilename;
    int nFileDes = open(strFname.c_str(),O_RDONLY);
    HCPLOG_T << "Reading File data :"<<strFname;
    char cstrBuffer[nBUF_SIZE];      /* Character buffer */
    ssize_t nRetVal;

    struct stat stStatBuf;
    fstat(nFileDes,&stStatBuf);
    HCPLOG_T << "FileSize :"<<stStatBuf.st_size;

    memset(cstrBuffer, 0, sizeof(char)*nBUF_SIZE);
    // Copy process
    while((nRetVal = read (nFileDes, &cstrBuffer, nBUF_SIZE-1)) > 0)
    {
        rstrData += cstrBuffer;
        memset(cstrBuffer, 0, sizeof(char)*nBUF_SIZE);
    }
    close(nFileDes);
    //data += cstrBuffer;
    //delete cstrBuffer;
    HCPLOG_T <<"DATA==>"<<rstrData;
}

std::string CIgniteFileUtils::GetFileName(const std::string &rstrPath, 
                                          bool bWithoutExtension) 
{
    char chSeparator;

    #ifdef _WIN32
    chSeparator = '\\';
    #else
    chSeparator = '/';
    #endif

    size_t iterI = rstrPath.rfind(chSeparator, rstrPath.length());
    if (iterI != std::string::npos) 
    {
        std::string strFileName = 
            (rstrPath.substr(iterI+1, rstrPath.length() - iterI));
        if(bWithoutExtension)
        {
            iterI = strFileName.find_last_of(".");
            if (iterI != std::string::npos) 
            {
                return (strFileName.substr(0,iterI));
            }
        }
        return strFileName;
    }

    return("");
}

bool CIgniteFileUtils::IsFile(const std::string &rstrFilePath) 
{
    struct stat stBuffer;
    stat(rstrFilePath.c_str(), &stBuffer);
    return S_ISREG(stBuffer.st_mode);
}

bool CIgniteFileUtils::IsDirectory(const std::string &rstrDirPath) 
{
    struct stat stBuffer;
    stat(rstrDirPath.c_str(), &stBuffer);
    return S_ISDIR(stBuffer.st_mode);
}

bool CIgniteFileUtils::Split(const std::string &rstrFile, size_t nChunkSize, 
    std::vector<std::string> &rvectorFileList)
{
    std::ifstream fileInStream(rstrFile, std::ifstream::binary);

    if(not fileInStream)
    {
        return false;
    }
    long long int nTotalFileSize = GetSize(rstrFile);
    int nNoOfChunks = nTotalFileSize / nChunkSize;
    size_t nSizeOfLastChunk = nTotalFileSize % nChunkSize;

    if(nSizeOfLastChunk != 0)
    {
        nNoOfChunks++;
    }
    else
    {
        nSizeOfLastChunk = nChunkSize;
    }
    
    for(int nChunk = 0; nChunk < nNoOfChunks; nChunk++)
    {
        size_t nCurrentChunkSize = 
            (nChunk == nNoOfChunks - 1) ? nSizeOfLastChunk : nChunkSize;

        std::vector<char> vectorChunkData(nCurrentChunkSize);
        fileInStream.read(&vectorChunkData[0], nCurrentChunkSize);

        std::string strOutFileName(rstrFile);
        strOutFileName += ".";
        strOutFileName += 
            ic_utils::CIgniteStringUtils::NumberToString(nChunk+1);

        std::ofstream fileOutStream(strOutFileName, std::ifstream::binary);
        fileOutStream.write(&vectorChunkData[0], nCurrentChunkSize);
        fileOutStream.close();

        rvectorFileList.push_back(strOutFileName);
    }

    fileInStream.close();
    return true;
}
} /* namespace ic_utils */
