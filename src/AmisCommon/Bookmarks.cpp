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

#include "Bookmarks.h"
#include "FilePathTools.h"
#include <iostream>
#include <log4cxx/logger.h>
using namespace std;

// create logger which will become a child to logger kolibre.amis
log4cxx::LoggerPtr amisBookmarksLog(
        log4cxx::Logger::getLogger("kolibre.amis.bookmarks"));

bool amis::PositionData::compare(const PositionData *second)
{
    LOG4CXX_DEBUG( amisBookmarksLog,
            "comparing " << mUri << " to " << second->mUri);
    if (mUri != second->mUri)
        return false;
    if (mNcxRef != second->mNcxRef)
        return false;
    if (mTextRef != second->mTextRef)
        return false;
    if (mAudioRef != second->mAudioRef)
        return false;
    if (mPlayOrder != second->mPlayOrder)
        return false;
    if (mTimeOffset != second->mTimeOffset)
        return false;
    if (mCharOffset != second->mCharOffset)
        return false;

    return true;
}

amis::BookmarkFile::BookmarkFile()
{
    mpLastmark = NULL;
    mpTitle = NULL;
}

amis::BookmarkFile::~BookmarkFile()
{
    LOG4CXX_TRACE( amisBookmarksLog, "BookmarksFile..: Deleting...");

    if (mpTitle != NULL)
    {
        mpTitle->destroyContents();
        delete mpTitle;

        LOG4CXX_DEBUG( amisBookmarksLog, "BookmarksFile..: \ttitle");
    }

    if (mpLastmark != NULL)
    {
        delete mpLastmark;
        LOG4CXX_DEBUG( amisBookmarksLog, "BookmarksFile..: \tlastmark");
    }

    int i;
    int sz;
    amis::Bookmark* p_temp_bmk;
    amis::Hilite* p_temp_hilite;
    amis::MediaGroup* p_note;
    amis::PositionData* p_pos;

    sz = mItems.size() - 1;

    for (i = sz; i >= 0; i--)
    {
        if (mItems[i]->mType == amis::PositionMark::BOOKMARK)
        {

            p_temp_bmk = (amis::Bookmark*) mItems[i];

            mItems.pop_back();

            p_pos = p_temp_bmk->mpStart;

            delete p_pos;

            if (p_temp_bmk->mbHasNote == true)
            {
                p_note = p_temp_bmk->mpNote;
                p_note->destroyContents();
                delete p_note;
            }

            delete p_temp_bmk;

            LOG4CXX_DEBUG( amisBookmarksLog, "BookmarksFile..: \tbookmark");
        }

        else
        {
            p_temp_hilite = (amis::Hilite*) mItems[i];

            mItems.pop_back();

            p_pos = p_temp_hilite->mpStart;

            delete p_pos;

            p_pos = p_temp_hilite->mpEnd;

            delete p_pos;

            if (p_temp_hilite->mbHasNote == true)
            {
                p_note = p_temp_hilite->mpNote;
                p_note->destroyContents();
                delete p_note;
            }

            delete p_temp_hilite;

            LOG4CXX_DEBUG( amisBookmarksLog, "BookmarksFile..: \thilite");
        }
    }

}

int amis::BookmarkFile::getNumberOfItems()
{
    return (int) mItems.size();
}

int amis::BookmarkFile::getMaxId()
{
    int i, id;
    int sz = mItems.size();
    int maxId = 0;

    amis::Bookmark* p_bmk = NULL;

    for (i = 0; i < sz; i++)
    {
        if (mItems[i]->mType == amis::PositionMark::BOOKMARK)
        {
            p_bmk = (amis::Bookmark*) mItems[i];
            id = p_bmk->mId;

            if (id > maxId)
                maxId = id;
        }
    }

    return maxId;
}

amis::PositionMark* amis::BookmarkFile::getItem(int idx)
{
    if (idx >= 0 && idx < (int) mItems.size())
    {
        return mItems[idx];
    }
    else
    {
        return NULL;
    }
}

amis::PositionData* amis::BookmarkFile::getLastmark()
{
    return mpLastmark;
}

string amis::BookmarkFile::getUid()
{
    LOG4CXX_INFO( amisBookmarksLog,
            "BookmarkFile::getUid() Returning uid : " << mUid);
    return mUid;
}

amis::MediaGroup* amis::BookmarkFile::getTitle()
{
    return mpTitle;
}

void amis::BookmarkFile::addHilite(Hilite* pHilite)
{
    mItems.push_back(pHilite);
}

void amis::BookmarkFile::addBookmark(Bookmark* pBookmark)
{
    mItems.push_back(pBookmark);
}

void amis::BookmarkFile::deleteItem(int idx)
{
    LOG4CXX_INFO( amisBookmarksLog, "Erasing bookmark: " << idx);
    PositionMark* posmark = mItems[idx];
    mItems.erase(mItems.begin() + idx);
    delete posmark;
}

void amis::BookmarkFile::setTitle(amis::MediaGroup* pData)
{
    mpTitle = pData;
}

void amis::BookmarkFile::setUid(string uid)
{
    LOG4CXX_INFO( amisBookmarksLog, "BookmarkFile::setUid() Got uid : "<< uid);
    mUid = uid;
}

void amis::BookmarkFile::setLastmark(amis::PositionData* pData)
{
    if (mpLastmark != NULL)
    {
        delete mpLastmark;
    }
    mpLastmark = pData;
}

void amis::BookmarkFile::print()
{
    stringstream ssbookmark;
    ssbookmark << " TITLE: " << endl;
    printMediaGroup(mpTitle);
    ssbookmark << endl;

    ssbookmark << " UID: " << mUid << endl << endl;

    if (mpLastmark != NULL)
    {
        ssbookmark << " LASTMARK: " << endl;
        printPositionData(mpLastmark);
        ssbookmark << endl;
    }

    int i;
    amis::Bookmark* p_bmk;
    amis::Hilite* p_hilite;

    for (i = 0; i < (int) mItems.size(); i++)
    {
        if (mItems[i]->mType == amis::PositionMark::BOOKMARK)
        {
            p_bmk = (amis::Bookmark*) mItems[i];

            ssbookmark << " BOOKMARK: " << p_bmk->mId << endl;

            printPositionData(p_bmk->mpStart);
            if (p_bmk->mbHasNote == true)
            {
                printMediaGroup(p_bmk->mpNote);
            }
            ssbookmark << endl;
        }
        else
        {
            p_hilite = (amis::Hilite*) mItems[i];

            ssbookmark << " HILITE: " << endl;
            ssbookmark << " start - " << endl;
            printPositionData(p_hilite->mpStart);
            ssbookmark << " end - " << endl;
            printPositionData(p_hilite->mpEnd);

            if (p_hilite->mbHasNote == true)
            {
                printMediaGroup(p_hilite->mpNote);
            }
            ssbookmark << endl;
        }
    }
    LOG4CXX_INFO( amisBookmarksLog, ssbookmark);
}

void amis::BookmarkFile::printPositionData(amis::PositionData* pData)
{

    if (pData == NULL)
    {
        LOG4CXX_WARN( amisBookmarksLog, " \tBAD POSITION DATA");
        return;
    }
    stringstream ssposdata;

    ssposdata << " \tUri: " << pData->mUri << endl;
    ssposdata << " \tNcxRef: " << pData->mNcxRef << endl;
    ssposdata << " \tAudioRef: " << pData->mAudioRef << endl;
    ssposdata << " \tTextRef: " << pData->mTextRef << endl;
    ssposdata << " \tPlayOrder: " << pData->mPlayOrder << endl;

    if (pData->mbHasCharOffset == true)
    {
        ssposdata << " \tCharOffset: " << pData->mCharOffset << endl;
    }
    if (mpLastmark->mbHasTimeOffset == true)
    {
        ssposdata << " \tTimeOffset: " << pData->mTimeOffset << endl;
    }
    LOG4CXX_INFO( amisBookmarksLog, ssposdata);
}

void amis::BookmarkFile::printMediaGroup(amis::MediaGroup* pData)
{
    if (pData == NULL)
    {
        LOG4CXX_WARN( amisBookmarksLog, " \tBAD MEDIA DATA");
        return;
    }
    stringstream ssmediagroup;

    if (pData->hasText() == true)
    {
        amis::TextNode* p_text = pData->getText();

        if (p_text != NULL)
        {
            string text_content = p_text->getTextString();

            ssmediagroup << "\tText: \"" << text_content << "\"" << endl;
        }
    }
    if (pData->hasAudio() == true)
    {
        ssmediagroup << "\tAudio: " << pData->getAudio(0)->getSrc();
        ssmediagroup << "\t" << pData->getAudio(0)->getClipBegin() << " "
                << pData->getAudio(0)->getClipEnd() << endl;
    }
    LOG4CXX_INFO( amisBookmarksLog, ssmediagroup);
}

