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
#include <fstream>
#include <string>
#include <iostream>
#include <cctype>
#include <algorithm>

//PROJECT INCLUDES
#include "FilePathTools.h"
#include "TitleAuthorParse.h"
#include "OpfItemExtract.h"
#include "SmilAudioExtract.h"
#include "trim.h"

#include <XmlError.h>

#include <cstring>
#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.amis
log4cxx::LoggerPtr amisTitleAutorParseLog(
        log4cxx::Logger::getLogger("kolibre.amis.titleauthorparse"));

using namespace std;

//--------------------------------------------------
//constructor
//--------------------------------------------------
amis::TitleAuthorParse::TitleAuthorParse()
{
    mb_flagTheyHaveTitleInfo = false;
    mb_flagTheyHaveAuthorInfo = false;
    mb_flagGetHref = false;

    mpTitleInfo = NULL;
    mpAuthorInfo = NULL;

    mError.setSourceModuleName(amis::module_AmisCommon);
    mFiletype = NCC;
}

//--------------------------------------------------
//destructor
//!destroy the media group objects if no one else has taken ownership of them
//--------------------------------------------------
amis::TitleAuthorParse::~TitleAuthorParse()
{
    if (mb_flagTheyHaveTitleInfo == false && mpTitleInfo != NULL)
    {
        mpTitleInfo->destroyContents();

        delete mpTitleInfo;
    }

    if (mb_flagTheyHaveAuthorInfo == false && mpAuthorInfo != NULL)
    {
        mpAuthorInfo->destroyContents();

        delete mpAuthorInfo;
    }
}

//--------------------------------------------------
//return the file path
//--------------------------------------------------
string amis::TitleAuthorParse::getFilePath()
{
    return this->mFilePath;
}

//!open a file (ncc, ncx, or opf)
amis::AmisError amis::TitleAuthorParse::openFile(string filepath)
{
    LOG4CXX_DEBUG(amisTitleAutorParseLog, "Opening filepath: " << filepath);
    //local variables
    string tmp_string;

    mb_flagFinished = false;
    mb_flagGetChars = false;
    mb_flagDocAuthor = false;
    mb_flagDocTitle = false;

    mError.setCode(amis::OK);
    mError.setMessage("");
    mError.setFilename(filepath);

    mFilePath = amis::FilePathTools::getAsLocalFilePath(filepath);
    mFilePath = amis::FilePathTools::clearTarget(filepath);

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
        OpfItemExtract opf_parse;
        mFilePath = opf_parse.getItemHref(mFilePath, ID_NCX);

        mFiletype = NCX;

    }

    if (file_ext.compare(FILE_EXT_NCX) == 0)
    {
        mFiletype = NCX;
    }

    XmlReader parser;

    parser.setContentHandler(this);
    parser.setErrorHandler(this);

    bool ret = false;
    switch (mFiletype)
    {
    case NCC:
        ret = parser.parseHtml(mFilePath.c_str());
        break;
    default:
        ret = parser.parseXml(mFilePath.c_str());
        break;
    }

    if (!ret)
    {
        const XmlError *e = parser.getLastError();
        if (e)
        {
            LOG4CXX_ERROR(amisTitleAutorParseLog,
                    "Error in titleauthorparse: " << e->getMessage());
            mError.loadXmlError(*e);
        }
        else
        {
            LOG4CXX_ERROR(amisTitleAutorParseLog,
                    "Unknown error in titleauthorparse");
            mError.setCode(UNDEFINED_ERROR);
        }
    }

    //if this was an NCC file, resolve the smil href to get the title audio data
    if (mError.getCode() == amis::OK && mFiletype == NCC)
    {
        if (mHref != "")
        {
            SmilAudioExtract* smil_extract = new SmilAudioExtract();
            mHref = amis::FilePathTools::goRelativePath(mFilePath, mHref);
            amis::AudioNode* p_audio = NULL;

            mError = smil_extract->getAudioAtId(mHref, &p_audio);

            if (p_audio != NULL)
            {
                mpTitleInfo->addAudioClip(p_audio);
            }
            else
            {
                LOG4CXX_WARN(amisTitleAutorParseLog,
                        "Could not extract audio from " << filepath);
            }
            delete smil_extract;
        }
    }

    return mError;
}

//--------------------------------------------------
//get the author info
//--------------------------------------------------
amis::MediaGroup* amis::TitleAuthorParse::getAuthorInfo()
{
    mb_flagTheyHaveAuthorInfo = true;
    return mpAuthorInfo;
}

//--------------------------------------------------
//get the title info
//--------------------------------------------------
amis::MediaGroup* amis::TitleAuthorParse::getTitleInfo()
{
    mb_flagTheyHaveTitleInfo = true;
    return mpTitleInfo;
}

//--------------------------------------------------
//SAX METHODS
//--------------------------------------------------

//--------------------------------------------------
//xmlreader start element event
//--------------------------------------------------
bool amis::TitleAuthorParse::startElement(const xmlChar* const namespaceURI,
        const xmlChar* const localName, const xmlChar* const qName,
        const XmlAttributes &attributes)
{
    mpAttributes = &attributes;
    const char* element_name = NULL;
    element_name = XmlReader::transcode(qName);

    LOG4CXX_TRACE(amisTitleAutorParseLog, "In startelement, name " << element_name );

    if (mb_flagFinished == false)
    {
        //for ncc's we are looking for <h1 class="title"...>
        if (mFiletype == NCC)
        {
            //if this is a heading one element
            if (strcmp(element_name, TAG_H1) == 0)
            {
                string classval = getAttributeValue(ATTR_CLASS);

                // Convert the class value to lower case for comparison
                std::transform(classval.begin(), classval.end(),
                        classval.begin(), (int (*)(int))tolower);

                        LOG4CXX_TRACE(amisTitleAutorParseLog, "classval is " << classval );
                        //if the class="title"
if(                classval.compare(ATTRVAL_TITLE) == 0)
                {
                    mpTitleInfo = new amis::MediaGroup();

                    mb_flagGetChars = true;
                    mTempChars.erase();

                    mb_flagGetHref = true;
                    mChardataElm = TAG_H1;
                }
            }

            //if this is a link element and we are looking to get an href value
            else if(mb_flagGetHref == true &&
                    strcmp(element_name, TAG_A) == 0)
            {
                mHref = getAttributeValue(ATTR_HREF);
                mb_flagGetHref = false;
            }

            else
            {
                //empty
            }
        }

        //for ncx's we are looking for docTitle and docAuthor elements
        else if(mFiletype == NCX)
        {
            if (strcmp(element_name, TAG_DOCAUTHOR) == 0)
            {
                mpAuthorInfo = new amis::MediaGroup();
                mb_flagDocAuthor = true;
            }

            else if(strcmp(element_name, TAG_DOCTITLE) == 0)
            {
                mpTitleInfo = new amis::MediaGroup();
                mb_flagDocTitle = true;
            }

            else if((mb_flagDocTitle == true || mb_flagDocAuthor == true)
                    && strcmp(element_name, TAG_TEXT) == 0)
            {
                mb_flagGetChars = true;
                mTempChars.erase();
                mChardataElm = TAG_TEXT;
            }

            else if((mb_flagDocTitle == true || mb_flagDocAuthor == true)
                    && strcmp(element_name, TAG_AUDIO) == 0)
            {
                string src = getAttributeValue(ATTR_SRC);
                string clip_begin = getAttributeValue(ATTR_CLIPBEGIN);
                string clip_end = getAttributeValue(ATTR_CLIPEND);

                src = amis::FilePathTools::goRelativePath(mFilePath, src);
                amis::AudioNode* p_audio;
                p_audio = new amis::AudioNode();
                p_audio->setSrc(src);
                p_audio->setClipBegin(clip_begin);
                p_audio->setClipEnd(clip_end);

                amis::MediaGroup* p_current = NULL;

                if (mb_flagDocTitle == true)
                {
                    p_current = mpTitleInfo;
                }
                else if(mb_flagDocAuthor == true)
                {
                    p_current = mpAuthorInfo;
                }
                else
                {
                    //empty
                }

                if(p_current)
                p_current->addAudioClip(p_audio);

            }

            else
            {
                //empty
            }
        }

    }

    return false;
}

//--------------------------------------------------
//(SAX Event) close this element so that no additional child nodes are added to it in the Smil Tree
//--------------------------------------------------
bool amis::TitleAuthorParse::endElement(const xmlChar* const namespaceURI,
        const xmlChar* const localName, const xmlChar* const qName)
{
    const char* element_name = NULL;
    element_name = XmlReader::transcode(qName);

    //if we are ending the tag we wanted to get character data for
    if (mb_flagGetChars == true
            && strcmp(element_name, mChardataElm.c_str()) == 0)
    {
        mb_flagGetChars = false;
    }

    else if (mb_flagDocAuthor == true
            && strcmp(element_name, TAG_DOCAUTHOR) == 0)
    {
        mb_flagDocAuthor = false;
    }

    else if (mb_flagDocTitle == true && strcmp(element_name, TAG_DOCTITLE) == 0)
    {
        mb_flagDocTitle = false;
    }
    return true;
}

//--------------------------------------------------
//xmlreader error event
//--------------------------------------------------
bool amis::TitleAuthorParse::error(const XmlError& e)
{
    // Not fatal
    return true;
}

//--------------------------------------------------
//xmlreader fatal error event
//--------------------------------------------------
bool amis::TitleAuthorParse::fatalError(const XmlError& e)
{
    mError.loadXmlError(e);
    return false;
}

//--------------------------------------------------
//xmlreader warning event
//--------------------------------------------------
bool amis::TitleAuthorParse::warning(const XmlError& e)
{
    return true;
}

//--------------------------------------------------
//xmlreader character data event
//--------------------------------------------------
bool amis::TitleAuthorParse::characters(const xmlChar* const characters,
        const unsigned int length)
{
    if (mb_flagGetChars == true)
    {
        const char *tmpchars = XmlReader::transcode(characters);
        mTempChars.append(tmpchars, length);

        LOG4CXX_TRACE(amisTitleAutorParseLog, "In characters, chars: " << mTempChars );

        amis::MediaGroup* p_current_media = NULL;

        if (mFiletype == NCX)
        {
            if (mb_flagDocTitle == true)
            {
                p_current_media = mpTitleInfo;
            }
            else if (mb_flagDocAuthor == true)
            {
                p_current_media = mpAuthorInfo;
            }
            else
            {
                //empty
            }

        }
        else // if (mFiletype == NCC)
        {
            p_current_media = mpTitleInfo;
        }

        if (p_current_media != NULL)
        {
            if (p_current_media->hasText() == false)
            {
                amis::TextNode* p_text;
                p_text = new amis::TextNode();
                p_text->setTextString(mTempChars);
                p_current_media->setText(p_text);
            }
            else
            {
                p_current_media->getText()->setTextString(trim(mTempChars));

            }

        }
        //XmlReader::release(tmpchars);

    } //end if mb_flagGetChars = true
    return true;
}

//---------------------------------
//utility function
//---------------------------------
const char *amis::TitleAuthorParse::getAttributeValue(const char *attributeName)
{
    //initialize local strings
    const char *current_attribute_name;

    //save the attributes list length
    int len = mpAttributes->getLength();

    LOG4CXX_TRACE(amisTitleAutorParseLog, "Attributes length: " << len );

    //for-loop through the attributes list until we find a match
    for (int i = 0; i < len; i++)
    {
        current_attribute_name = XmlReader::transcode(mpAttributes->qName(i));

        LOG4CXX_TRACE(amisTitleAutorParseLog, "current attrname: " << current_attribute_name );

        //comparison if statement
        if (strcmp(current_attribute_name, attributeName) == 0)
        {
            LOG4CXX_TRACE(amisTitleAutorParseLog, "got a match for " << current_attribute_name << ":" << XmlReader::transcode(mpAttributes->value(i)) );

            //a match has been found, return its value
            return XmlReader::transcode(mpAttributes->value(i));
        }
    } //end for-loop

    return "";
}
