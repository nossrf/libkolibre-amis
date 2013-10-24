/*
 AmisCommon: common system objects and utility routines

 Copyright (C) 2004  DAISY for All Project
 
 Copyright (C) 2012 Kolibre

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef FILEPATHTOOLS_H
#define FILEPATHTOOLS_H

//SYSTEM INCLUDES 
#include <string>

//PROJECT INCLUDES
#include "AmisCommon.h"

namespace amis
{
//!File path tools: utility functions for working with file paths
class AMISCOMMON_API FilePathTools
{
public:
    //!get the file name (file.ext) from a string
    static std::string getFileName(std::string);
    //!get the target from a string (#target)
    static std::string getTarget(std::string);
    //!get the file extension
    static std::string getExtension(std::string);
    //!get as a local file path
    static std::string getAsLocalFilePath(std::string);
    //!clear the target from a string
    static std::string clearTarget(std::string);
    //!calculate a new path based on a starting path and a relative path
    static std::string goRelativePath(std::string, std::string);
    //!convert the slashes to go forward
    static std::string convertSlashesFwd(std::string);
    //!convert the slashes to go backward
    static std::string convertSlashesBkw(std::string);
    //!get the parent directory.  input can be a file or directory path.
    static std::string getParentDirectory(std::string);
    //!check if path is a directory
    static bool isDirectory(std::string);
    //!create a directory
    static bool createDirectory(std::string);
    //!check if a file exists and is readable
    static bool fileIsReadable(std::string);
    //!check if a file exists and is writable
    static bool fileIsWriteable(std::string);
    //!rename file
    static bool renameFile(std::string, std::string);
};
}
#endif

