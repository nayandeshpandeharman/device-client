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
* \file CIgniteFileUtils.h
*
* \brief CIgniteFileUtils provides the utility functions for file operations
*******************************************************************************
*/

#ifndef CIGNITE_FILEUTILS_H
#define CIGNITE_FILEUTILS_H

#include <string>
#include <vector>
#include <list>
#include <zlib.h>

namespace ic_utils 
{
const int CHUNK = 16384;    ///< standard chunksize to compress.
const int CHAR_GRP = 4;     ///< group of char for encoded data.
const int BYTES_GRP = 3;    ///< group of bytes for decoded data.

/**
 * class CIgniteFileUtils provides the utility functions for 
 * file related operations.
 */
class CIgniteFileUtils 
{
public:
    /**
     * Method to copy a file from one location to another.
     * @param[in] rstrFrom Reference to source file path.
     * @param[out] rstrTo Reference to destination file path.
     * @return 0, if the file copying is successful; -1 otherwise.
     */
    static int Copy(const std::string &rstrFrom, const std::string &rstrTo);
    
    /**
     * Method to remove a file.
     * @param[in] rstrFilename Reference to file to be removed.
     * @return 0, if the file removal is successful; -1 otherwise.
     */
    static int Remove(const std::string &rstrFilename);

    /**
     * Method to check if a file exists or not.
     * @param[in] rstrFilename Reference to file to be checked for existence.
     * @return true, if the file exists; false otherwise.
     */
    static bool Exists(const std::string &rstrFilename);
    
    /**
     * Method to get the size of the file.
     * @param[in] rstrFilename Reference to name/path of the file.
     * @return size of file; -1 otherwise.
     */
    static int GetSize(const std::string &rstrFilename);
    
    /**
     * Method to list files in a directory .
     * @param[in] rstrDirName Reference to the name of the directory.
     * @param[out] rvectorFileList Reference to the list of files.
     * @return 0, if listing is successful; -1 otherwise.
     */
    static int ListFiles(const std::string &rstrDirName, 
                         std::vector<std::string> &rvectorFileList);
    
    //! Member variable holds maximum length of filename allowed.
    static const unsigned int MAX_FILENAME_LENGTH = 90;
    
    /**
     * Method compress[overloaded](only the source file path param provided).
     * @param[in] rstrSrcPath Reference to the source file path.
     * @return Integer that represents the result of the compression operation.
     * Z_OK (0): Operation completed successfully.
     * Z_STREAM_END (1): The compression process reached the end of the input 
     * data.
     * Z_ERRNO (-1): Error reading/writng a file.
     * Z_STREAM_ERROR (-2): Invalid compression level.
     * Z_DATA_ERROR (-3): Invalid or incomplete deflate data.
     * Z_MEM_ERROR (-4): Memory allocation failure occurred during processing.
     * Z_BUF_ERROR (-5): Output buffer not large enough; more output space is 
     * needed.
     * Z_VERSION_ERROR (-6): if the version of zlib.h and the version of the 
     * library linked do not match.
     * Other negative values: Various other error conditions.
     * @see https://www.zlib.net/manual.html
     */
    static int Compress(const std::string &rstrSrcPath);
    
    /**
     * Method to compress a file
     * @param[in] rstrSrcPath Reference to the source file path.
     * @param[out] rstrDestPath Reference to the destination of the file.
     * @return Integer that represents the result of the compression operation.
     * Z_OK (0): Operation completed successfully.
     * Z_STREAM_END (1): The compression process reached the end of the input 
     * data.
     * Z_ERRNO (-1): Error reading/writng a file.
     * Z_STREAM_ERROR (-2): Invalid compression level.
     * Z_DATA_ERROR (-3): Invalid or incomplete deflate data.
     * Z_MEM_ERROR (-4): Memory allocation failure occurred during processing.
     * Z_BUF_ERROR (-5): Output buffer not large enough; more output space is 
     * needed.
     * Z_VERSION_ERROR (-6): if the version of zlib.h and the version of the 
     * library linked do not match.
     * Other negative values: Various other error conditions.
     * @see https://www.zlib.net/manual.html
     */
    static int Compress(const std::string &rstrSrcPath, 
                        const std::string &rstrDestPath);

    /**
     * Method to make a directory.
     * @param[in] rstrDirPath Reference to the directory path to be created.
     * @return true, if the directory is created successfully; false, otherwise.
     */
    static bool MakeDirectory(std::string &rstrDirPath);
    
    /**
     * Method to remove a directory.
     * @param[in] rstrDirPath Reference to the directory path to be removed.
     * @return true, if directory is created successfully; false, otherwise.
     */
    static bool RemoveDirectory(std::string &rstrDirPath);
    
    /**
     * Method to move a directory.
     * @param[in] rstrSrc Reference to the source of file.
     * @param[in] rstrDest Reference to the destination of file.
     * @return true, if directory is moved successfully; false, otherwise.
     */
    static bool Move(std::string &rstrSrc, std::string &rstrDest);
    
    /**
     * Method to decode base64 encoded string.
     * @param[in] rstrEncodedString Reference to the input string.
     * @return resultant base64-decoded string.
     */
    static std::string Base64Decode(std::string const &rstrEncodedString);

    /**
     * Method to perform base64 encoding
     * @param[in] puchBytesToEncode Pointer to bytes that needs to be encoded.
     * @param[in] nInputlength Input length of byte array .
     * @return resultant base64-encoded string.
     */
    static std::string Base64Encode(unsigned char const *puchBytesToEncode, 
                                    unsigned int nInputlength);
    
    /**
     * Method to read the content of a file and append it to given string data.
     * @param[in] rstrFilename Reference to the name/path of the file.
     * @param[out] rstrData Reference to the stored file content.
     * @return void
     */
    static void ReadFile(std::string &rstrFilename, std::string &rstrData);
    
    /**
     * Method to check whether a given string value ends with .
     * another string's ending.
     * @param[in] rstrvalue Reference to the input string value
     * @param[in] rstrEnding Reference to the string ending
     * @return true, if the ending matches the end of the value; 
     *         otherwise, it returns false.
     */
    static bool StringEndsWith(std::string const &rstrvalue, 
                               std::string const &rstrEnding);
    
    /**
     * Method to list files in a directory that have a specific suffix,
     * @param[in] rstrdir Reference to the directory path where the files 
     *             should be listed.
     * @param[in] strSuffix File suffix or extension to filter files.
     * @param[out] rlistFileList Reference to the list of resultant file names.
     * @return void
     */
    static void ListFiles(const std::string &rstrdir,  
                          const std::string strSuffix,
                          std::list<std::string> &rlistFileList);
    
    /**
     * Method to extract the filename from a given path and 
     * optionally remove the file extension.
     * @param[in] rstrPath Reference to the file path
     * @param[in] bWithoutExtension True, if file extension is to be removed;
     *             false, otherwise.
     * @return resultant filename string.
     */
    static std::string GetFileName(const std::string &rstrPath, 
                                   bool bWithoutExtension);
    
    /**
     * Method to check whether a given path corresponds to a regular file.
     * @param[in] rstrFilePath Reference to the path of file.
     * @return true, if it is a regular file; false otherwise.
     */
    static bool IsFile(const std::string &rstrFilePath);
    
    /**
     * Method to check whether the file is a directory.
     * @param[in] rstrDirPath Path of file.
     * @return true, if it is a directory; false, otherwise.
     */
    static bool IsDirectory(const std::string &rstrDirPath);
    
    /**
     * Method to split a file into chunks of files.
     * @param[in] rstrFile Target file.
     * @param[in] nChunkSize Size to split files into.
     * @param[out] rvectorFileList Vector of resultant files.
     * @return true, if splitting is successfull; false otherwise.
     */
    static bool Split(const std::string &rstrFile, size_t nChunkSize, 
                      std::vector<std::string> &rvectorFileList);   
private:
    /**
     * Method to perform base64 decoding.
     * @param[in/out] pucstrBytesArray Array to store decoded bytes.
     * @param[in/out] pucstrCharArray Array to store Base64 characters.
     * @return void.
     */
    static void Decode(unsigned char *pucstrBytesArray,
                       unsigned char *pucstrCharArray);

    /**
     * Method to compress a file specified by srcPath and writes the compressed
     * data to a file specified by destPath
     * @param[in] rstStrm Structure that holds information about the state of a 
     * compression stream.
     * @param[out] pstDestFile A pointer to the destination file (FILE*) where 
     * the compressed data is to be written.
     * @param[in] nFlush Specifies the flush type which allows deflate to decide 
     * how much data to accumulate before producing output, 
     * in order to maximize compression.
     * @return Integer that represents the result of the compression operation.
     * Z_OK (0): Operation completed successfully.
     * Z_STREAM_END (1): The compression process reached the end of the input 
     * data.
     * Z_ERRNO (-1): Error reading/writng a file.
     * Z_STREAM_ERROR (-2): Invalid compression level.
     * Z_DATA_ERROR (-3): Invalid or incomplete deflate data.
     * Z_MEM_ERROR (-4): Memory allocation failure occurred during processing.
     * Z_BUF_ERROR (-5): Output buffer not large enough; more output space is 
     * needed.
     * Z_VERSION_ERROR (-6): if the version of zlib.h and the version of the 
     * library linked do not match.
     * Other negative values: Various other error conditions.
     * @see https://www.zlib.net/manual.html
     */
    static int CompressChunk(z_stream &rstStrm, FILE *pstDestFile, 
                             const int nFlush);

    /**
     * Method for file compress.
     * @param[in] rstStrm Structure that holds information about the state of a 
     * compression stream.
     * @param[in] pstSrcFile A pointer to the source file.
     * @param[in] pstDestFile A pointer to the destination file.
     * @return Integer that represents the result of the compression operation.
     * @see https://www.zlib.net/manual.html
     */
    static int CompressFile(z_stream &rstStrm, FILE *pstSrcFile, 
                            FILE *pstDestFile);
};

} /* namespace ic_utils */

#endif /* CIGNITE_FILEUTILS_H */
