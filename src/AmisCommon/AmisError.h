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

//Amis error object
#ifndef AMISERROR_H
#define AMISERROR_H

//SYSTEM INCLUDES
#include <string>

//PROJECT INCLUDES
#include "AmisCommon.h"

#include <XmlError.h>

namespace amis
{
//!Amis Error object: encapsulates a code, message, and source module name
class AMISCOMMON_API AmisError //amis::AmisError
{

public:
    //!default constructor
    AmisError();
    //!destructor
    ~AmisError();

    //!set the error code
    void setCode(amis::ErrorCode code);
    //!set the error message
    void setMessage(const std::string &);
    //!set the source module name
    void setSourceModuleName(const std::string &);

    void loadXmlError(const XmlError &);

    void setFilename(const std::string &);

    //!get the error code
    amis::ErrorCode getCode() const;
    //!get the error message
    std::string getMessage() const;
    //!get the source module name
    std::string getSourceModuleName() const;
    //!get the filename where the error occurred
    std::string getFilename() const;

private:
    //!the error code
    amis::ErrorCode mCode;
    //!the error message
    std::string mMessage;
    //!the source file name
    std::string mFilename;
    //!the source module name (module in which the error originated)
    std::string mSourceModule;

};
}
#endif
