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
#include "OpfItemExtract.h"

#include <XmlError.h>

#include <cstring>
#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.amis
log4cxx::LoggerPtr amisOpfItemExtractLog(
        log4cxx::Logger::getLogger("kolibre.amis.opfitemxxtract"));

using namespace std;

//--------------------------------------------------
//constructor
//--------------------------------------------------
amis::OpfItemExtract::OpfItemExtract()
{
    mError.setSourceModuleName(amis::module_AmisCommon);
}

//--------------------------------------------------
//destructor
//--------------------------------------------------
amis::OpfItemExtract::~OpfItemExtract()
{
}

//--------------------------------------------------
/*!
 @param[in] filepath 
 the full path to a smil file with an id target on the end
 */
//--------------------------------------------------
string amis::OpfItemExtract::getItemHref(string filepath, string id)
{
    mResults.clear();

    mSearchFor = id;

    mMode = GET_BY_ID;
    openFile(filepath);

    if (mHref != "")
        mResults.push_back(mHref);

    return mHref;

}

unsigned int amis::OpfItemExtract::getByMediaType(string filepath,
        string mediaType)
{
    mMode = GET_BY_MEDIATYPE;

    mSearchFor = mediaType;
    mResults.clear();

    openFile(filepath);

    return mResults.size();
}

string amis::OpfItemExtract::getItem(unsigned int idx)
{
    if (idx >= 0 && idx < mResults.size())
    {
        return mResults[idx];
    }
    else
    {
        return "";
    }
}

amis::AmisError amis::OpfItemExtract::openFile(string filepath)
{
    //local variables
    const char* cp_file;
    string tmp_string;
    XmlReader parser;

    mb_flagFinished = false;
    mHref = "";

    mFilepath = amis::FilePathTools::getAsLocalFilePath(filepath);
    mFilepath = amis::FilePathTools::clearTarget(filepath);

    mError.setCode(amis::OK);
    mError.setMessage("");
    mError.setFilename(filepath);

    //do a SAX parse of this new file
    cp_file = mFilepath.c_str();

    parser.setContentHandler(this);
    parser.setErrorHandler(this);

    if (!parser.parseXml(cp_file))
    {
        const XmlError *e = parser.getLastError();
        if (e)
        {
            LOG4CXX_ERROR(amisOpfItemExtractLog,
                    "Error in OpfItemExtract: " << e->getMessage());
            mError.loadXmlError(*e);
        }
        else
        {
            LOG4CXX_ERROR(amisOpfItemExtractLog,
                    "Unknown error in OpfItemExtract");
            mError.setCode(UNDEFINED_ERROR);
        }
    }

    return mError;
}

//SAX METHODS
//--------------------------------------------------
//! (SAX Event) analyze the element type and collect data to build a node
//--------------------------------------------------
bool amis::OpfItemExtract::startElement(const xmlChar* const uri,
        const xmlChar* const localname, const xmlChar* const qname,
        const XmlAttributes&attributes)
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

    if (strcmp(element_name, TAG_ITEM) == 0)
    {
        if (mMode == GET_BY_ID)
        {
            if (getAttributeValue(ATTR_ID).compare(mSearchFor) == 0)
            {
                mHref = getAttributeValue(ATTR_HREF);
                mHref = amis::FilePathTools::goRelativePath(mFilepath, mHref);
                mb_flagFinished = true;
            }
        }
        else if (mMode == GET_BY_MEDIATYPE)
        {
            if (getAttributeValue("media-type").compare(mSearchFor) == 0)
            {
                string filepath = getAttributeValue(ATTR_HREF);
                filepath = amis::FilePathTools::goRelativePath(mFilepath,
                        filepath);
                mResults.push_back(filepath);
            }
        }
        else
        {
            //empty
        }
    }

    XmlReader::release(element_name);
    return true;
} //end SmilTreeBuilder::startElement function

//--------------------------------------------------
//! (SAX Event) error
//--------------------------------------------------
bool amis::OpfItemExtract::error(const XmlError& e)
{
    // Not fatal
    return true;
}

//--------------------------------------------------
//! (SAX Event) fatal error
//--------------------------------------------------
bool amis::OpfItemExtract::fatalError(const XmlError& e)
{
    mError.loadXmlError(e);
    return false;
}

//--------------------------------------------------
//! (SAX Event) warning
//--------------------------------------------------
bool amis::OpfItemExtract::warning(const XmlError& e)
{
    //cerr<<"warning"<<endl;
    return true;
}

//---------------------------------
//utility function
//---------------------------------
string amis::OpfItemExtract::getAttributeValue(string attributeName)
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
