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

//INCLUDES
#include "AmisError.h"
#include <XmlError.h>

#include <iostream>
#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.amis
log4cxx::LoggerPtr amisAmisErrorLog(
        log4cxx::Logger::getLogger("kolibre.amis.amiserror"));

using namespace std;

//--------------------------------------------------
//--------------------------------------------------
amis::AmisError::AmisError()
{
    mCode = OK;
    mMessage = "";
    mSourceModule = "";
}

//--------------------------------------------------
//--------------------------------------------------
amis::AmisError::~AmisError()
{
}

//--------------------------------------------------
//--------------------------------------------------
void amis::AmisError::setCode(amis::ErrorCode code)
{
    mCode = code;
}

//--------------------------------------------------
//--------------------------------------------------
void amis::AmisError::setMessage(const std::string &message)
{
    mMessage = message;
}

//--------------------------------------------------
//--------------------------------------------------
void amis::AmisError::setFilename(const std::string &filename)
{
    mFilename = filename;
}

//--------------------------------------------------
//--------------------------------------------------
void amis::AmisError::setSourceModuleName(const std::string &sourceModuleName)
{
    mSourceModule = sourceModuleName;
}

//--------------------------------------------------
//--------------------------------------------------
void amis::AmisError::loadXmlError(const XmlError &e)
{
    LOG4CXX_ERROR(amisAmisErrorLog, " Got exception: " << e.message());

    setMessage(e.message());

    switch (e.domain())
    {
    case XML_FROM_PARSER:
        LOG4CXX_ERROR(amisAmisErrorLog, "PARSE_ERROR");
        setCode(PARSE_ERROR);
        break;
    case XML_FROM_IO:
        switch (e.code())
        {
        case XML_IO_EACCES:
            LOG4CXX_ERROR(amisAmisErrorLog, "PERMISSION_ERROR");
            setCode(PERMISSION_ERROR);
            break;

        case XML_IO_ENOENT:
        case XML_IO_EIO:
        default:
            LOG4CXX_ERROR(amisAmisErrorLog, "NOT_FOUND");
            setCode(NOT_FOUND);
            break;
        }
        break;
    case XML_FROM_HTTP:
        LOG4CXX_ERROR(amisAmisErrorLog, "NOT_FOUND");
        setCode(NOT_FOUND);
        break;
    default:
        LOG4CXX_ERROR(amisAmisErrorLog, "UNDEFINED_ERROR");
        setCode(UNDEFINED_ERROR);
        break;
    }

}

//--------------------------------------------------
//--------------------------------------------------
amis::ErrorCode amis::AmisError::getCode() const
{
    return mCode;
}

//--------------------------------------------------
//--------------------------------------------------
std::string amis::AmisError::getMessage() const
{
    return mMessage;
}

//--------------------------------------------------
//--------------------------------------------------
std::string amis::AmisError::getFilename() const
{
    return mFilename;
}

//--------------------------------------------------
//--------------------------------------------------
std::string amis::AmisError::getSourceModuleName() const
{
    return mSourceModule;
}
