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

//SYSTEM INCLUDES
#include <string>
#include <iostream>
#include <vector>

//PROJECT INCLUDES
#include "FilePathTools.h"
#include "SmilAudioExtract.h"

#include <XmlError.h>

#include <cstring>
#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.amis
log4cxx::LoggerPtr amisSmilAudioExtractLog(
        log4cxx::Logger::getLogger("kolibre.amis.smilaudioextract"));

using namespace std;

//--------------------------------------------------
//constructor
//--------------------------------------------------
amis::SmilAudioExtract::SmilAudioExtract()
{
    mError.setSourceModuleName(amis::module_AmisCommon);
}

//--------------------------------------------------
//destructor
//--------------------------------------------------
amis::SmilAudioExtract::~SmilAudioExtract()
{
}

//--------------------------------------------------
/*!
 @param[in] filepath 
 the full path to a smil file with an id target on the end
 */
//--------------------------------------------------
amis::AmisError amis::SmilAudioExtract::getAudioAtId(string filepath,
        amis::AudioNode** pAudioInfo)
{
    //local variables
    const char* cp_file;
    string tmp_string;
    XmlReader parser;

    mb_flagInPar = false;
    mb_flagFoundId = false;
    mb_flagFinished = false;

    mpAudioInfo = NULL;

    mError.setCode(amis::OK);
    mError.setMessage("");
    mError.setFilename(filepath);

    mId = amis::FilePathTools::getTarget(filepath);

    mFilePath = amis::FilePathTools::getAsLocalFilePath(filepath);
    mFilePath = amis::FilePathTools::clearTarget(filepath);

    //do a SAX parse of this new file
    cp_file = mFilePath.c_str();

    parser.setContentHandler(this);
    parser.setErrorHandler(this);

    if (!parser.parseXml(cp_file))
    {
        const XmlError *e = parser.getLastError();
        if (e)
        {
            LOG4CXX_ERROR(amisSmilAudioExtractLog,
                    "Error in SmilAudioExtract: " << e->getMessage());
            mError.loadXmlError(*e);
        }
        else
        {
            LOG4CXX_ERROR(amisSmilAudioExtractLog,
                    "Unknown error in SmilAudioExtract");
            mError.setCode(UNDEFINED_ERROR);
        }
    }

    if (mpAudioInfo != NULL)
        *pAudioInfo = mpAudioInfo;

    return mError;
}

//SAX METHODS
//--------------------------------------------------
//! (SAX Event) analyze the element type and collect data to build a node
//--------------------------------------------------
bool amis::SmilAudioExtract::startElement(const xmlChar* const uri,
        const xmlChar* const localname, const xmlChar* const qname,
        const XmlAttributes& attributes)
{
    if (mb_flagFinished == true)
    {
        return false;
    }

    //local variables
    const char* element_name;
    string tmp_string;

    //int len = attributes.getLength();

    //get the element name as a string
    element_name = XmlReader::transcode(qname);

    mpAttributes = &attributes;

    //if the id has not been found yet and this is a par tag
    if (mb_flagFoundId == false && strcmp(element_name, SMIL_TAG_PAR) == 0)
    {
        mb_flagInPar = true;

        tmp_string = getAttributeValue(SMIL_ATTR_ID);
        //when we find the ID, flag the system to stop looking. 
        //it will stop processing elements past the close of this par.
        if (tmp_string.compare(mId) == 0)
        {
            mb_flagFoundId = true;
        }
    }

    //if we are in a par and this is an audio tag
    else if (mb_flagInPar == true && strcmp(element_name, SMIL_TAG_AUDIO) == 0)
    {
        if (mpAudioInfo == NULL)
        {
            mpAudioInfo = new amis::AudioNode();

            //record the audio data, it might be useful if this is the par we're looking for
            mpAudioInfo->setClipBegin(getAttributeValue(SMIL_ATTR_CLIPBEGIN));
            mpAudioInfo->setClipEnd(getAttributeValue(SMIL_ATTR_CLIPEND));

            tmp_string = getAttributeValue(SMIL_ATTR_SRC);
            tmp_string = amis::FilePathTools::goRelativePath(mFilePath,
                    tmp_string);
            mpAudioInfo->setSrc(tmp_string);
        }
    }

    //if we are in a par and this is a text tag
    else if (mb_flagInPar == true && strcmp(element_name, SMIL_TAG_TEXT) == 0)
    {
        //when we find the ID (could be on the text although not usually),
        //flag the system to stop searching after the close of this text node's
        //parent par.
        tmp_string = getAttributeValue(SMIL_ATTR_ID);
        if (tmp_string.compare(mId) == 0)
        {
            mb_flagFoundId = true;
        }
    }

    else
    {
        //empty
    }

    XmlReader::release(element_name);
    return true;
} //end SmilTreeBuilder::startElement function

//--------------------------------------------------
//! (SAX Event) close this element so that no additional child nodes are added to it in the Smil Tree
//--------------------------------------------------
bool amis::SmilAudioExtract::endElement(const xmlChar* const uri,
        const xmlChar* const localname, const xmlChar* const qname)
{
    //local variable
    const char* element_name = XmlReader::transcode(qname);

    //if this element is a par, then create all data collected since the open-par tag
    //and add it to the audio list
    if (strcmp(element_name, SMIL_TAG_PAR) == 0)
    {
        mb_flagInPar = false;

        //if we found the id, set a flag to not process any more data
        if (mb_flagFoundId == true)
        {
            mb_flagFinished = true;
        }
    }

    XmlReader::release(element_name);
    return true;
}

//--------------------------------------------------
//! (SAX Event) error
//--------------------------------------------------
bool amis::SmilAudioExtract::error(const XmlError& e)
{
    //mError.setException(e);
    return true;
}

//--------------------------------------------------
//! (SAX Event) fatal error
//--------------------------------------------------
bool amis::SmilAudioExtract::fatalError(const XmlError& e)
{
    mError.loadXmlError(e);
    return false;
}

//--------------------------------------------------
//! (SAX Event) warning
//--------------------------------------------------
bool amis::SmilAudioExtract::warning(const XmlError& e)
{
    //ignore, it's non-fatal
    return true;
}

//---------------------------------
//utility function
//---------------------------------
string amis::SmilAudioExtract::getAttributeValue(string attributeName)
{
    //initialize local strings
    const char* current_attribute_name;
    const char* attribute_value;
    string return_value = "";

    //save the attributes list length
    int len = mpAttributes->getLength();

    //for-loop through the attributes list until we find a match
    for (int i = 0; i < len; i++)
    {
        current_attribute_name = XmlReader::transcode(
                mpAttributes->getQName(i));

        //comparison if statement
        if (strcmp(current_attribute_name, attributeName.c_str()) == 0)
        {
            //a match has been found, so save it and break from the loop
            attribute_value = XmlReader::transcode(mpAttributes->getValue(i));

            return_value.assign(attribute_value);
            XmlReader::release(attribute_value);
            XmlReader::release(current_attribute_name);

            break;
        }
        XmlReader::release(current_attribute_name);
    } //end for-loop

    //return the value of the requested attribute
    //if the attribute does not exist, this function returns an empty string
    return return_value;
}
