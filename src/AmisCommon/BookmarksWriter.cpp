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

//PROJECT INCLUDES
#include <iostream>
#include <sstream>

#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>

#include <cstring>
#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.amis
log4cxx::LoggerPtr amisBmkWriterLog(
        log4cxx::Logger::getLogger("kolibre.amis.bookmarkswriter"));

using namespace std;

#define ENCODING "utf-8"

template<typename T> std::string toString(const T& x)
{
    std::ostringstream oss;
    oss << x;
    return oss.str();
}

#include "FilePathTools.h"
#include "BookmarksWriter.h"

class Converter
{
public:
    Converter(const char* const toTranscode)
    {

#ifdef WIN32
        const char *inptr = toTranscode;
#else
        char *inptr = (char*)toTranscode;
#endif

        outbuf = strdup(inptr);
    }

    ~Converter()
    {
        free(outbuf);
    }

    const xmlChar* xmlchar() const
    {
        return (xmlChar*) outbuf;
    }

private:
    char * outbuf;
};

#define X(str) Converter(str).xmlchar()

//end of borrowed Xerces Code.

amis::BookmarksWriter::BookmarksWriter() :
        xmlwriter(0)
{
}

amis::BookmarksWriter::~BookmarksWriter()
{
}

bool amis::BookmarksWriter::saveFile(string filepath, BookmarkFile* pFile)
{
    mpFile = pFile;
    unsigned int i;

    //cout << "BookmarksWriter::saveFile() Writing bookmarks to " << filepath << endl;

    xmlDocPtr doc;

    // returncode
    int rc;

    // Create xmlwriter
    xmlwriter = xmlNewTextWriterDoc(&doc, 0);
    if (xmlwriter == NULL)
    {
        LOG4CXX_ERROR(amisBmkWriterLog, "Error creating the xml writer");
        xmlFreeDoc(doc);
        return false;
    }

    rc = xmlTextWriterStartDocument(xmlwriter, NULL, ENCODING, NULL);
    if (rc < 0)
    {
        LOG4CXX_ERROR(amisBmkWriterLog, "Error at xmlTextWriterStartDocument");
        xmlFreeTextWriter(xmlwriter);
        xmlFreeDoc(doc);
        return false;
    }

    rc = xmlTextWriterStartElement(xmlwriter, X("bookmarkSet"));
    if (rc < 0)
        return rc;

    //start adding data
    rc = writeTitle(mpFile->getTitle());
    if (rc < 0)
    {
        LOG4CXX_ERROR(amisBmkWriterLog, "Error at writeTitle");
        xmlFreeTextWriter(xmlwriter);
        xmlFreeDoc(doc);
        return false;
    }

    rc = writeUid(mpFile->getUid());
    if (rc < 0)
    {
        LOG4CXX_ERROR(amisBmkWriterLog, "Error at writeUid");
        xmlFreeTextWriter(xmlwriter);
        xmlFreeDoc(doc);
        return false;
    }

    rc = writeLastmark(mpFile->getLastmark());
    if (rc < 0)
    {
        LOG4CXX_ERROR(amisBmkWriterLog, "Error at writeLastmark");
        xmlFreeTextWriter(xmlwriter);
        xmlFreeDoc(doc);
        return false;
    }

    amis::PositionMark* p_pos;
    amis::Bookmark* p_bmk;
    amis::Hilite* p_hilite;

    for (i = 0; i < mpFile->getNumberOfItems(); i++)
    {
        p_pos = mpFile->getItem(i);

        if (p_pos->mType == amis::PositionMark::BOOKMARK)
        {
            p_bmk = (amis::Bookmark*) p_pos;
            rc = writeBookmark(p_bmk);
            if (rc < 0)
            {
                LOG4CXX_ERROR(amisBmkWriterLog, "Error at writeBookmark");
                xmlFreeTextWriter(xmlwriter);
                xmlFreeDoc(doc);
                return false;
            }
        }
        else
        {
            p_hilite = (amis::Hilite*) p_pos;
            rc = writeHilite(p_hilite);
            if (rc < 0)
            {
                LOG4CXX_ERROR(amisBmkWriterLog, "Error at writeHilite");
                xmlFreeTextWriter(xmlwriter);
                xmlFreeDoc(doc);
                return false;
            }
        }
    }

    rc = xmlTextWriterEndElement(xmlwriter);
    if (rc < 0)
    {
        LOG4CXX_ERROR(amisBmkWriterLog, "Error at xmlTextWriterEndElement");
        xmlFreeTextWriter(xmlwriter);
        xmlFreeDoc(doc);
        return false;
    }

    //cout << "BookmarksWriter::savefile(): Ending document" << endl;
    rc = xmlTextWriterEndDocument(xmlwriter);
    if (rc < 0)
    {
        LOG4CXX_ERROR(amisBmkWriterLog, "Error at xmlTextWriterEndDocument");
        xmlFreeTextWriter(xmlwriter);
        xmlFreeDoc(doc);
        return false;
    }

    xmlFreeTextWriter(xmlwriter);

    //------------------
    // save the document to a file
    //------------------

    // make sure the path to the file exists
    string dir = amis::FilePathTools::getParentDirectory(filepath);
    if (not amis::FilePathTools::isDirectory(dir))
    {
        amis::FilePathTools::createDirectory(dir);
    }

    // write the file
    //LOG4CXX_DEBUG(amisBmkWriterLog, "writing file to " + filepath);
    rc = xmlSaveFormatFileEnc(filepath.c_str(), doc, ENCODING, 1);
    if (rc < 0)
    {
        LOG4CXX_ERROR(amisBmkWriterLog, "Error at xmlSaveFormatFileEnc");
        xmlFreeDoc(doc);
        return false;
    }

    xmlFreeDoc(doc);

    return true;
}

int amis::BookmarksWriter::writeTitle(amis::MediaGroup* pTitle)
{
    if (pTitle == NULL)
    {
        return 0;
    }

    int rc;

    //create the title
    rc = xmlTextWriterStartElement(xmlwriter, X("title"));
    if (rc < 0)
        return rc;

    rc = writeMediaGroup(pTitle);
    if (rc < 0)
        return rc;

    //cout << "BookmarksWriter::writeTitle(): Ending title element" << endl;
    rc = xmlTextWriterEndElement(xmlwriter);
    return rc;
}

int amis::BookmarksWriter::writeUid(string uid)
{
    //cout << "BookmarksWriter::writeUid(): Writing uid element" << uid << endl;
    return xmlTextWriterWriteFormatElement(xmlwriter, X("uid"), "%s",
            uid.c_str());
}

int amis::BookmarksWriter::writeLastmark(amis::PositionData* pLastmark)
{
    if (pLastmark == NULL)
    {
        return 0;
    }

    int rc;

    //cout << "BookmarksWriter::writeLastmark(): Writing lastmark element" << endl;
    rc = xmlTextWriterStartElement(xmlwriter, X("lastmark"));
    if (rc < 0)
        return rc;

    rc = writePositionData(pLastmark);
    if (rc < 0)
        return rc;

    //cout << "BookmarksWriter::writeLastmark(): Finishing lastmark element" << endl;
    rc = xmlTextWriterEndElement(xmlwriter);

    return rc;
}

int amis::BookmarksWriter::writeHilite(amis::Hilite* pHilite)
{
    //cout << "BookmarksWriter::writeLastmark(): Writing hilite.." << endl;
    int rc;

    rc = xmlTextWriterStartElement(xmlwriter, X("hilite"));
    if (rc < 0)
        return rc;

    rc = xmlTextWriterStartElement(xmlwriter, X("hiliteStart"));
    if (rc < 0)
        return rc;

    amis::PositionData* p_start = pHilite->mpStart;
    rc = writePositionData(p_start);
    if (rc < 0)
        return rc;

    rc = xmlTextWriterEndElement(xmlwriter);
    if (rc < 0)
        return rc;

    rc = xmlTextWriterStartElement(xmlwriter, X("hiliteEnd"));
    if (rc < 0)
        return rc;

    amis::PositionData* p_end = pHilite->mpEnd;
    rc = writePositionData(p_end);
    if (rc < 0)
        return rc;

    rc = xmlTextWriterEndElement(xmlwriter);
    if (rc < 0)
        return rc;

    if (pHilite->mbHasNote == true)
    {
        rc = writeNote(pHilite->mpNote);
        if (rc < 0)
            return rc;
    }

    return xmlTextWriterEndElement(xmlwriter);
}

int amis::BookmarksWriter::writeBookmark(amis::Bookmark* pBookmark)
{
    //cout << "BookmarksWriter::writeLastmark(): Writing bookmark.." << endl;
    int rc;

    rc = xmlTextWriterStartElement(xmlwriter, X("bookmark"));
    if (rc < 0)
        return rc;

    // Add ID attribute to bookmark
    string id = toString<int>(pBookmark->mId);
    rc = xmlTextWriterWriteAttribute(xmlwriter, X("id"), X(id.c_str()));
    if (rc < 0)
        return rc;

    amis::PositionData* p_pos = pBookmark->mpStart;
    rc = writePositionData(p_pos);
    if (rc < 0)
        return rc;

    if (pBookmark->mbHasNote == true)
    {
        rc = writeNote(pBookmark->mpNote);
        if (rc < 0)
            return rc;
    }

    rc = xmlTextWriterEndElement(xmlwriter);
    return rc;
}

int amis::BookmarksWriter::writePositionData(amis::PositionData* pData)
{
    int rc;
    //cout << "BookmarksWriter::writeLastmark(): Writing positiondata.." << endl;

    rc = xmlTextWriterWriteFormatElement(xmlwriter, X("uri"), "%s",
            X(pData->mUri.c_str()));
    if (rc < 0)
        return rc;

    rc = xmlTextWriterWriteFormatElement(xmlwriter, X("ncxRef"), "%s",
            X(pData->mNcxRef.c_str()));
    if (rc < 0)
        return rc;

    rc = xmlTextWriterWriteFormatElement(xmlwriter, X("textRef"), "%s",
            X(pData->mTextRef.c_str()));
    if (rc < 0)
        return rc;

    rc = xmlTextWriterWriteFormatElement(xmlwriter, X("audioRef"), "%s",
            X(pData->mAudioRef.c_str()));
    if (rc < 0)
        return rc;

    string playOrder = toString<int>(pData->mPlayOrder);
    rc = xmlTextWriterWriteFormatElement(xmlwriter, X("playOrder"), "%s",
            X(playOrder.c_str()));
    if (rc < 0)
        return rc;

    if (pData->mbHasTimeOffset == true)
    {
        rc = xmlTextWriterWriteFormatElement(xmlwriter, X("timeOffset"), "%s",
                X(pData->mTimeOffset.c_str()));
        if (rc < 0)
            return rc;
    }

    if (pData->mbHasCharOffset == true)
    {
        rc = xmlTextWriterWriteFormatElement(xmlwriter, X("charOffset"), "%s",
                X(pData->mCharOffset.c_str()));
        if (rc < 0)
            return rc;
    }
    return rc;
}

int amis::BookmarksWriter::writeMediaGroup(amis::MediaGroup* pMedia)
{
    if (pMedia == NULL)
    {
        return 0;
    }

    int rc;

    //cout << "BookmarksWriter::writeLastmark(): Writing mediagroup.." << endl;

    if (pMedia->hasText() == true)
    {
        //cout << "BookmarksWriter::writeMediaGroup(): Writing formatted text element" << endl;
        string text_content = pMedia->getText()->getTextString();
        rc = xmlTextWriterWriteFormatElement(xmlwriter, X("text"), "%s",
                X(text_content.c_str()));
        if (rc < 0)
            return rc;
    }

    if (pMedia->hasAudio() == true)
    {
        //cout << "BookmarksWriter::writeMediaGroup(): Starting audio elemenet" << endl;
        rc = xmlTextWriterStartElement(xmlwriter, X("audio"));
        if (rc < 0)
            return rc;

        amis::AudioNode* p_audio_obj = pMedia->getAudio(0);

        //cout << "BookmarksWriter::writeMediaGroup(): Setting attribute src" << endl;
        //add src, clipBegin, and clipEnd attributes to "audio"
        rc = xmlTextWriterWriteFormatAttribute(xmlwriter, X("src"), "%s",
                X(p_audio_obj->getSrc().c_str()));
        if (rc < 0)
            return rc;

        //cout << "BookmarksWriter::writeMediaGroup(): Setting attribute clipbegin" << endl;
        rc = xmlTextWriterWriteFormatAttribute(xmlwriter, X("clipBegin"), "%s",
                X(p_audio_obj->getClipBegin().c_str()));
        if (rc < 0)
            return rc;

        //cout << "BookmarksWriter::writeMediaGroup(): Setting attribute clipend" << endl;
        rc = xmlTextWriterWriteFormatAttribute(xmlwriter, X("clipEnd"), "%s",
                X(p_audio_obj->getClipEnd().c_str()));
        if (rc < 0)
            return rc;

        //cout << "BookmarksWriter::writeMediaGroup(): Ending element audio" << endl;
        rc = xmlTextWriterEndElement(xmlwriter);
        if (rc < 0)
            return rc;
    }
    return rc;
}

int amis::BookmarksWriter::writeNote(amis::MediaGroup* pNote)
{
    int rc;
    //cout << "BookmarksWriter::writeLastmark(): Writing note.." << endl;
    rc = xmlTextWriterStartElement(xmlwriter, X("note"));
    if (rc < 0)
        return rc;

    rc = writeMediaGroup(pNote);
    if (rc < 0)
        return rc;

    rc = xmlTextWriterEndElement(xmlwriter);
    if (rc < 0)
        return rc;
    return rc;
}
