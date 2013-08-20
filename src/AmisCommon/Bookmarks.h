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

#ifndef BOOKMARKS_H
#define BOOKMARKS_H

#include "AmisCommon.h"
#include "Media.h"
#include <string>
#include <vector>
#include <iostream>

namespace amis
{

//!PositionMark class
class AMISCOMMON_API PositionData
{
public:
    PositionData()
    {
        mPlayOrder = -1;
        mUri = "";
        mNcxRef = "";
        mTextRef = "";
        mTextString = ""; // Used only for currentpos
        mAudioRef = "";
        mTimeOffset = "";
        mCharOffset = "";
        mbHasTimeOffset = false;
        mbHasCharOffset = false;
    }
    ~PositionData()
    {
        //std::cout << "PositionData::~PositionData()" << endl;
        mNcxRef = "";
        mUri = "";
        mTimeOffset = "";
        mCharOffset = "";
    }

    bool compare(const PositionData *second);

    std::string mUri;
    std::string mNcxRef;
    std::string mTextRef;
    std::string mTextString;
    std::string mAudioRef;
    std::string mTimeOffset;
    std::string mCharOffset;
    bool mbHasTimeOffset;
    bool mbHasCharOffset;
    int mPlayOrder;
};

//!Generic mark class
class AMISCOMMON_API PositionMark
{
public:
    PositionData* mpStart;
    amis::MediaGroup* mpNote;
    bool mbHasNote;

    enum MarkType
    {
        BOOKMARK = 0, HILITE = 1, HISTORY = 2
    };

    MarkType mType;
};

//!Bookmark class
class AMISCOMMON_API Bookmark: public amis::PositionMark
{
public:
    Bookmark()
    {
        mType = BOOKMARK;
        mId = 0;
    }

    int mId;
};

//!Hilite class
class AMISCOMMON_API Hilite: public amis::PositionMark
{
public:
    Hilite()
    {
        mType = HILITE;
    }

    PositionData* mpEnd;
};

//!History class
class AMISCOMMON_API HistoryItem: public amis::PositionMark
{
public:
    HistoryItem()
    {
        mType = HISTORY;
    }
};

//!BookmarkFile class
class AMISCOMMON_API BookmarkFile
{

public:
    BookmarkFile();
    ~BookmarkFile();

    int getNumberOfItems();
    int getMaxId();

    amis::PositionMark* getItem(int);

    amis::PositionData* getLastmark();
    std::string getUid();
    amis::MediaGroup* getTitle();

    void addHilite(Hilite*);
    void addBookmark(Bookmark*);

    void deleteItem(int);

    void print();
    void printPositionData(amis::PositionData*);
    void printMediaGroup(amis::MediaGroup*);

    void setTitle(amis::MediaGroup*);
    void setUid(std::string);
    void setLastmark(amis::PositionData*);

private:
    amis::MediaGroup* mpTitle;
    std::string mUid;
    PositionData* mpLastmark;

    //bookmarks and hilites
    std::vector<amis::PositionMark*> mItems;

};

}
#endif

