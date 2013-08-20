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

#include "BookmarksReader.h"
#include "FilePathTools.h"

#include <iostream>

#include <sstream>
#include <XmlError.h>

#include <cstring>
#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.amis
log4cxx::LoggerPtr amisBmkReaderLog(
        log4cxx::Logger::getLogger("kolibre.amis.bookmarksreader"));

using namespace std;

// Handy templates to convert string to int
template<typename T> T stringTo(const std::string& s)
{
    std::istringstream iss(s);
    T x;
    iss >> x;
    return x;
}

using namespace std;

amis::BookmarksReader::BookmarksReader()
{
    mError.setSourceModuleName(amis::module_AmisCommon);
}

amis::BookmarksReader::~BookmarksReader()
{
}

amis::AmisError amis::BookmarksReader::openFile(string filepath, BookmarkFile* pFile)
{
    mError.setCode(amis::OK);
    mError.setMessage("");
    mError.setFilename(filepath);

    if (pFile == NULL)
    {
        mError.setCode(amis::UNDEFINED_ERROR);
        mError.setMessage("BookmarkFile pointer not set");
        return mError;
    }

    mpFile = pFile;

    //local variables
    const char* cp_file;
    string tmp_string;
    XmlReader parser;

    mb_flagGetChars = false;
    // For idcounter
    maxId = 0;

    mpCurrentPosMark = NULL;
    mpCurrentNote = NULL;
    mElementStack.clear();

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
            LOG4CXX_ERROR(amisBmkReaderLog,
                    "Error in BookmarksReader: " << e->getMessage());
            mError.loadXmlError(*e);
        }
        else
        {
            LOG4CXX_ERROR(amisBmkReaderLog, "Unknown error in BookmarksReader");
            mError.setCode(UNDEFINED_ERROR);
        }
    }

    pFile = mpFile;

    return mError;
}

//--------------------------------------------------
//xerces start element event
//--------------------------------------------------
bool amis::BookmarksReader::startElement(const xmlChar* const uri,
        const xmlChar* const localname, const xmlChar* const qname,
        const XmlAttributes& attributes)
{
    const char* element_name;

    //get the element name as a string
    element_name = XmlReader::transcode(qname);

    //std::cout << "BookmarksReader..: At element: '" << element_name << "'" << endl;

    string tmpstr;
    tmpstr.assign(element_name);

    mpAttributes = &attributes;

    if (strcmp(element_name, "title") == 0)
    {
        mpTitle = new amis::MediaGroup();
    }
    else if (strcmp(element_name, "text") == 0)
    {
        //if we are processing a title or note
        if (mElementStack.back().compare("title") == 0)
        {
            mb_flagGetChars = true;

            amis::TextNode* p_text = new amis::TextNode();
            mpTitle->setText(p_text);
        }

        else if (mElementStack.back().compare("note") == 0)
        {
            mb_flagGetChars = true;

            amis::TextNode* p_text = new amis::TextNode();
            mpCurrentNote->setText(p_text);
        }

        else
        {
            //empty
        }
    }
    else if (strcmp(element_name, "audio") == 0)
    {
        //if we are processing a title or a note
        if (mElementStack.back().compare("title") == 0)
        {
            amis::AudioNode* p_audio = new amis::AudioNode();

            string tmpstr = getAttributeValue("src");
            p_audio->setSrc(tmpstr);
            tmpstr = getAttributeValue("clipBegin");
            p_audio->setClipBegin(tmpstr);
            tmpstr = getAttributeValue("clipEnd");
            p_audio->setClipEnd(tmpstr);

            mpTitle->addAudioClip(p_audio);

        }
        else if (mElementStack.back().compare("note") == 0)
        {
            amis::AudioNode* p_audio = new amis::AudioNode();
            string tmpstr = getAttributeValue("src");
            p_audio->setSrc(tmpstr);
            tmpstr = getAttributeValue("clipBegin");
            p_audio->setClipBegin(tmpstr);
            tmpstr = getAttributeValue("clipEnd");
            p_audio->setClipEnd(tmpstr);

            //std::cout << "BookmarksReader..: Adding note audio clip" << endl;
            mpCurrentNote->addAudioClip(p_audio);
        }
        else
        {
            //empty
        }
    }
    else if (strcmp(element_name, "uid") == 0)
    {
        mb_flagGetChars = true;
    }
    else if (strcmp(element_name, "lastmark") == 0)
    {
        //std::cout << "BookmarksReader..: Found lastmark in bookmark file" << endl;
        mpCurrentPosData = new amis::PositionData;
        mpCurrentPosData->mbHasCharOffset = false;
        mpCurrentPosData->mbHasTimeOffset = false;
        mpCurrentPosData->mPlayOrder = -1;
    }

    else if (strcmp(element_name, "uri") == 0)
    {
        mb_flagGetChars = true;
    }
    else if (strcmp(element_name, "ncxRef") == 0)
    {
        mb_flagGetChars = true;
    }
    else if (strcmp(element_name, "textRef") == 0)
    {
        mb_flagGetChars = true;
    }
    else if (strcmp(element_name, "audioRef") == 0)
    {
        mb_flagGetChars = true;
    }

    else if (strcmp(element_name, "playOrder") == 0)
    {
        mb_flagGetChars = true;
    }
    else if (strcmp(element_name, "timeOffset") == 0)
    {
        mb_flagGetChars = true;
    }

    else if (strcmp(element_name, "charOffset") == 0)
    {
        mb_flagGetChars = true;
    }

    else if (strcmp(element_name, "note") == 0)
    {
        mpCurrentNote = new amis::MediaGroup();
    }
    else if (strcmp(element_name, "bookmark") == 0)
    {
        mpCurrentPosMark = new amis::Bookmark;
        mpCurrentPosMark->mbHasNote = false;
        mpCurrentPosData = new amis::PositionData;
        mpCurrentPosData->mbHasCharOffset = false;
        mpCurrentPosData->mbHasTimeOffset = false;
        mpCurrentPosData->mPlayOrder = -1;

        amis::Bookmark *p_bmk = (amis::Bookmark*) mpCurrentPosMark;
        int id = stringTo<int>(getAttributeValue("id"));

        // Crude way to autoincrement id's in bookmarkfiles that don't have id's
        if (id < 0 || id > 30000)
            id = 0;
        if (id == 0)
            id = maxId + 1;
        if (id > maxId)
            maxId = id;
        p_bmk->mId = id;

    }
    else if (strcmp(element_name, "hilite") == 0)
    {
        mpCurrentPosMark = new amis::Hilite;
        mpCurrentPosMark->mbHasNote = false;
    }
    else if (strcmp(element_name, "hiliteStart") == 0)
    {
        mpCurrentPosData = new amis::PositionData;
        mpCurrentPosData->mbHasCharOffset = false;
        mpCurrentPosData->mbHasTimeOffset = false;
        mpCurrentPosData->mPlayOrder = -1;
    }
    else if (strcmp(element_name, "hiliteEnd") == 0)
    {
        mpCurrentPosData = new amis::PositionData;
        mpCurrentPosData->mbHasCharOffset = false;
        mpCurrentPosData->mbHasTimeOffset = false;
        mpCurrentPosData->mPlayOrder = -1;
    }
    else
    {
        //empty
    }

    mElementStack.push_back(tmpstr);

    XmlReader::release(element_name);
    return true;
}

//--------------------------------------------------
//(SAX Event) close this element
//--------------------------------------------------
bool amis::BookmarksReader::endElement(const xmlChar* const uri,
        const xmlChar* const localname, const xmlChar* const qname)
{
    //local variable
    const char* element_name = XmlReader::transcode(qname);

    if (strcmp(element_name, "note") == 0)
    {
        mpCurrentPosMark->mpNote = mpCurrentNote;
        mpCurrentPosMark->mbHasNote = true;
        mpCurrentNote = NULL;
    }
    else if (strcmp(element_name, "uid") == 0)
    {
        mpFile->setUid(mTempChars);
    }
    else if (strcmp(element_name, "uri") == 0)
    {
        std::cout << "BookmarksReader..: got uri: '" << mTempChars << "'"
                << endl;
        mpCurrentPosData->mUri = mTempChars;
    }
    else if (strcmp(element_name, "ncxRef") == 0)
    {
        //std::cout << "BookmarksReader..: got ncxRef: '" << mTempChars << "'" << endl;
        mpCurrentPosData->mNcxRef = mTempChars;
    }
    else if (strcmp(element_name, "textRef") == 0)
    {
        //std::cout << "BookmarksReader..: got textRef: '" << mTempChars << "'" << endl;
        mpCurrentPosData->mTextRef = mTempChars;
    }
    else if (strcmp(element_name, "audioRef") == 0)
    {
        //std::cout << "BookmarksReader..: got audioRef: '" << mTempChars << "'" << endl;
        mpCurrentPosData->mAudioRef = mTempChars;
    }
    else if (strcmp(element_name, "playOrder") == 0)
    {
        //std::cout << "BookmarksReader..: got mPlayOrder: '" << mTempChars << "' or " << stringTo<int>(mTempChars) << endl;
        mpCurrentPosData->mPlayOrder = stringTo<int>(mTempChars);
    }
    else if (strcmp(element_name, "timeOffset") == 0)
    {
        //std::cout << "BookmarksReader..: got timeoffset: '" << mTempChars << "'" << endl;
        mpCurrentPosData->mTimeOffset = mTempChars;
        mpCurrentPosData->mbHasTimeOffset = true;
    }
    else if (strcmp(element_name, "charOffset") == 0)
    {
        std::cout << "BookmarksReader..: got charoffset: '" << mTempChars << "'"
                << endl;
        mpCurrentPosData->mCharOffset = mTempChars;
        mpCurrentPosData->mbHasCharOffset = true;
    }
    else if (strcmp(element_name, "lastmark") == 0)
    {
        std::cout << "BookmarksReader..: got lastmark..." << endl;
        std::cout << "       uri:\t'" << mpCurrentPosData->mUri << "'" << endl;
        std::cout << "    ncxref:\t'" << mpCurrentPosData->mNcxRef << "'"
                << endl;
        std::cout << "   textref:\t'" << mpCurrentPosData->mTextRef << "'"
                << endl;
        std::cout << "  audioref:\t'" << mpCurrentPosData->mAudioRef << "'"
                << endl;
        std::cout << "timeoffset:\t'" << mpCurrentPosData->mTimeOffset << "'"
                << endl;
        std::cout << "charoffset:\t'" << mpCurrentPosData->mCharOffset << "'"
                << endl;
        mpFile->setLastmark(mpCurrentPosData);
    }
    else if (strcmp(element_name, "text") == 0)
    {
        //look back to see if we are in an open "note" or "title" tag
        string elmname = mElementStack[mElementStack.size() - 2];
        if (elmname.compare("note") == 0)
        {
            mpCurrentNote->getText()->setTextString(mTempWChars);
        }
        else if (elmname.compare("title") == 0)
        {
            //std::cout << "BookmarksReader..: Got title: '" << mTempWChars << "'" << endl;
            mpTitle->getText()->setTextString(mTempWChars);
        }
        else
        {
            //empty
        }
    }
    else if (strcmp(element_name, "bookmark") == 0)
    {
        mpCurrentPosMark->mpStart = mpCurrentPosData;
        amis::Bookmark* p_bookmark = (amis::Bookmark*) mpCurrentPosMark;
        this->mpFile->addBookmark(p_bookmark);
    }
    else if (strcmp(element_name, "hilite") == 0)
    {
        amis::Hilite* p_hilite;
        p_hilite = (amis::Hilite*) mpCurrentPosMark;
        this->mpFile->addHilite(p_hilite);
    }
    else if (strcmp(element_name, "hiliteStart") == 0)
    {
        amis::Hilite* p_hilite;
        p_hilite = (amis::Hilite*) mpCurrentPosMark;
        p_hilite->mpStart = mpCurrentPosData;
    }
    else if (strcmp(element_name, "hiliteEnd") == 0)
    {
        amis::Hilite* p_hilite;
        p_hilite = (amis::Hilite*) mpCurrentPosMark;
        p_hilite->mpEnd = mpCurrentPosData;
    }
    else if (strcmp(element_name, "title") == 0)
    {
        this->mpFile->setTitle(mpTitle);
    }
    else
    {
        //empty
    }

    mb_flagGetChars = false;
    mTempChars.erase();
    mTempWChars.erase();

    XmlReader::release(element_name);

    mElementStack.pop_back();
    return true;
}

//--------------------------------------------------
//xerces error event
//--------------------------------------------------
bool amis::BookmarksReader::error(const XmlError& e)
{
    // Not fatal
    //mError.loadXmlError(e);
    return true;
}

//--------------------------------------------------
//xerces fatal error event
//--------------------------------------------------
bool amis::BookmarksReader::fatalError(const XmlError& e)
{
    mError.loadXmlError(e);
    return false;
}

//--------------------------------------------------
//xerces warning event
//--------------------------------------------------
bool amis::BookmarksReader::warning(const XmlError& e)
{
    return true;
}

//--------------------------------------------------
//xerces character data event
//--------------------------------------------------
bool amis::BookmarksReader::characters(const xmlChar * const chars,
        const unsigned int length)
{
    if (mb_flagGetChars == true)
    {

        int idx = mElementStack.size() - 2;
        if (mElementStack[idx].compare("note") == 0
                || mElementStack[idx].compare("title") == 0)
        {
            //mTempWChars.append((wchar_t*)chars);
            const char* chardata = XmlReader::transcode(chars, length);
            mTempWChars.append(chardata);
            XmlReader::release(chardata);
        }
        else
        {
            if (mTempChars == "")
            {
                const char* chardata = XmlReader::transcode(chars, length);
                mTempChars.assign(chardata);
                XmlReader::release(chardata);
            }
        }

    } //end if mb_flagGetChars = true

    return true;
}

//---------------------------------
//utility function
//---------------------------------
string amis::BookmarksReader::getAttributeValue(string attributeName)
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
