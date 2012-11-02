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

#include <string>
#include <iostream>
#include <algorithm>
#include "FilePathTools.h"
#include "MetadataSet.h"
#include "md5.h"

#include <XmlError.h>

#include <cstring>
#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.amis
log4cxx::LoggerPtr amisMetadataSetLog(
        log4cxx::Logger::getLogger("kolibre.amis.metadataset"));

using namespace std;

amis::MetaItem::MetaItem()
{
}
amis::MetaItem::~MetaItem()
{
    this->mContent.empty();
    this->mName.empty();
}

//--------------------------------------------------
//--------------------------------------------------
amis::MetadataSet::MetadataSet()
{
    clearVector();
    b_getChars = false;
    mError.setSourceModuleName(amis::module_AmisCommon);
}

//--------------------------------------------------
//--------------------------------------------------
amis::MetadataSet::~MetadataSet()
{
    clearVector();
}

//--------------------------------------------------
//--------------------------------------------------
void amis::MetadataSet::clearVector()
{
    unsigned int sz = mMetaList.size();
    MetaItem* tmp_ptr;

    for (int i = sz - 1; i >= 0; i--)
    {
        tmp_ptr = mMetaList[i];
        mMetaList.pop_back();
        delete tmp_ptr;
    }

    mMetaList.clear();
}

//--------------------------------------------------
//--------------------------------------------------
string amis::MetadataSet::getMetadata(string metaname)
{
    unsigned int i;
    string null_str;
    null_str.erase();

    LOG4CXX_DEBUG( amisMetadataSetLog,
            "MetadataSet: Looking for '" << metaname << "'");

    for (i = 0; i < mMetaList.size(); i++)
    {
        if (mMetaList[i]->mName.compare(metaname) == 0)
        {
            LOG4CXX_DEBUG( amisMetadataSetLog,
                    "MetadataSet: Got '" << mMetaList[i]->mContent << "'");
            return mMetaList[i]->mContent;

        }
    }

    return null_str;
}

string amis::MetadataSet::getChecksum()
{
    unsigned int i;
    string content;

    for (i = 0; i < mMetaList.size(); i++)
    {
        content.append(mMetaList[i]->mName);
        content.append("=");
        content.append(mMetaList[i]->mContent);
        content.append("\n");
    }

    // Create an md5 checksum digest
    string hash = md5(content);

    LOG4CXX_INFO(amisMetadataSetLog,
            "Generated md5 of metadata " << hash);

    return hash;
}

//--------------------------------------------------
/*!
 @param[in] filepath
 this is an NCC or OPF file
 */
//--------------------------------------------------
amis::AmisError amis::MetadataSet::openBookFile(string filepath)
{
    XmlReader parser;

    mError.setCode(amis::OK);
    mError.setMessage("");
    mError.setFilename(filepath);

    mFilepath = amis::FilePathTools::getAsLocalFilePath(filepath);

    string file_name = amis::FilePathTools::getFileName(filepath);
    string file_ext = amis::FilePathTools::getExtension(filepath);

    //convert the string to lower case before doing a comparison
    std::transform(file_name.begin(), file_name.end(), file_name.begin(),
            (int (*)(int))tolower);

            //convert the string to lower case before doing a comparison
std    ::transform(file_ext.begin(), file_ext.end(), file_ext.begin(),
            (int (*)(int))tolower);

if(    file_name.compare(FILENAME_NCC) == 0)
    {
        mFiletype = NCC;
    }

    if (file_ext.compare(FILE_EXT_OPF) == 0)
    {
        mFiletype = NCX;
    }

    if (file_ext.compare(FILE_EXT_NCX) == 0)
    {
        mFiletype = NCX;
    }

    parser.setContentHandler(this);
    parser.setErrorHandler(this);

    bool ret = false;
    switch (mFiletype)
    {
    case NCC:
        ret = parser.parseHtml(filepath.c_str());
        break;
    default:
        ret = parser.parseXml(filepath.c_str());
        break;
    }

    if (!ret)
    {
        const XmlError *e = parser.getLastError();
        if (e)
        {
            LOG4CXX_ERROR(amisMetadataSetLog,
                    "Error in MetadataSet: " << e->getMessage());
            mError.loadXmlError(*e);
        }
        else
        {
            LOG4CXX_ERROR(amisMetadataSetLog, "Unknown error in MetadataSet");
            mError.setCode(UNDEFINED_ERROR);
        }
    }

    return mError;
}

//--------------------------------------------------
//! (SAX Event) analyze the element type and collect metadata
//--------------------------------------------------
bool amis::MetadataSet::startElement(const xmlChar* const uri,
        const xmlChar* const localname, const xmlChar* const qname,
        const XmlAttributes& attributes)
{

    //local variables
    const char* element_name = NULL;
    string w_attribute_value;
    const char* current_attribute_name = NULL;
    string tmp_string;
    string sub_string;
    MetaItem* meta_item = NULL;

    //get the element name as a string
    element_name = XmlReader::transcode(qname);

    tmp_string.assign(element_name);

    //convert the string to lower case
    std::transform(tmp_string.begin(), tmp_string.end(), tmp_string.begin(),
            (int (*)(int))tolower);

if(    tmp_string.length() >= 3)
    {
        sub_string = tmp_string.substr(0, 3);
    }
    else
    {
        sub_string = "";
    }

    b_getChars = false;

    if (sub_string.compare(TAG_DC) == 0)
    {
        meta_item = new MetaItem;
        mMetaList.push_back(meta_item);
        //get the characters
        mMetaList[mMetaList.size() - 1]->mName = tmp_string;
        b_getChars = true;
    }

    else if (tmp_string.compare(TAG_META) == 0)
    {
        meta_item = new MetaItem;
        mMetaList.push_back(meta_item);

        //get attributes
        //initialize local strings

        b_getChars = false;

        //save the attributes list length
        int len = attributes.getLength();

        //for-loop through the attributes list until we find a match
        for (int i = 0; i < len; i++)
        {
            current_attribute_name = XmlReader::transcode(
                    attributes.getQName(i));

            //comparison if statement
            if (strcmp(current_attribute_name, ATTR_NAME) == 0)
            {
                //a match has been found, so save it and break from the loop
                const char* attribute_value;
                attribute_value = XmlReader::transcode(attributes.getValue(i));

                //convert the string to lower case
                //Damn book producers keep using uppercase and lowercase characters
                //together
                tmp_string.assign(attribute_value);
                std::transform(tmp_string.begin(), tmp_string.end(),
                        tmp_string.begin(), (int (*)(int))tolower);

mMetaList                [mMetaList.size() - 1]->mName = tmp_string.c_str();

                LOG4CXX_DEBUG( amisMetadataSetLog,
                        "MetadataSet: mName '" << tmp_string << "'");

                XmlReader::release(attribute_value);
            }
            else if (strcmp(current_attribute_name, ATTR_CONTENT) == 0)
            {
                //a match has been found, so save it and break from the loop

                const char* attribute_value;
                attribute_value = XmlReader::transcode(attributes.getValue(i));
                mMetaList[mMetaList.size() - 1]->mContent = attribute_value;

                LOG4CXX_DEBUG( amisMetadataSetLog,
                        "MetadataSet: mValue '" << attribute_value << "'");

                XmlReader::release(attribute_value);
            }
            XmlReader::release(current_attribute_name);
        } //end for-loop

    }

    else
    {
        //ignore this element
    }

    XmlReader::release(element_name);

    mTempChars.erase();
    return true;
} //end startElement function

//--------------------------------------------------
//--------------------------------------------------
bool amis::MetadataSet::characters(const xmlChar * const chars,
        const unsigned int length)
{
    if (b_getChars == true)
    {
        //mTempChars.append((wchar_t*)chars);
        const char *tmpchars = XmlReader::transcode(chars);
        mTempChars.append(tmpchars, length);
        XmlReader::release(tmpchars);

        if (mMetaList[mMetaList.size() - 1]->mContent.size() == 0)
        {
            mMetaList[mMetaList.size() - 1]->mContent = mTempChars;
        }
    }
    return true;
}
//--------------------------------------------------
//! (SAX Event) error
//--------------------------------------------------
bool amis::MetadataSet::error(const XmlError& e)
{
    // Not fatal
    LOG4CXX_ERROR( amisMetadataSetLog, " Exception " << e.getMessage());
    return true;
}

//--------------------------------------------------
//! (SAX Event) Output a parse error with Xmlreader' message
//--------------------------------------------------
bool amis::MetadataSet::fatalError(const XmlError& e)
{
    mError.loadXmlError(e);
    return false;
}

//--------------------------------------------------
//! (SAX Event) warning
//--------------------------------------------------
bool amis::MetadataSet::warning(const XmlError& e)
{
    LOG4CXX_WARN( amisMetadataSetLog, "warning: " << e.getMessage());
    return true;
}
