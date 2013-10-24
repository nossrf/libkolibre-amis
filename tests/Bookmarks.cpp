/*
 Copyright (C) 2012 Kolibre
 
 This file is part of Kolibre-amis.
 
 Kolibre-amis is free software: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation, either version 2.1 of the License, or
 (at your option) any later version.
 
 Kolibre-amis is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public License
 along with Kolibre-amis.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <string>
#include <iostream>
#include <sstream>
#include <assert.h>
#include <unistd.h>

#include "BookmarksWriter.h"
#include "BookmarksReader.h"
#include "setup_logging.h"

using namespace amis;

std::string title = "The titleÄÖÅ of the book";
std::string uid = "The uid of the book";
//std::string title = "䀀öå";

std::string createZeroPaddedString(int value)
{
    if ( value < 0) return "0000000";

    std::ostringstream oss;
    if (value < 10) oss << "000000";
    else if (value < 100) oss << "00000";
    else if (value < 1000) oss << "0000";
    else if (value < 10000) oss << "000";
    else if (value < 100000) oss << "00";
    else if (value < 1000000) oss << "0";

    oss << value;
    return oss.str();
}

void saveBookmark(std::string bookmarkFile, BookmarkFile* pBmk)
{
    BookmarksWriter writer;
    writer.saveFile(bookmarkFile, pBmk);
}

void verifyBoomark(std::string bookmarkFile, int playOrder)
{
    BookmarkFile* pBmk = NULL;
    pBmk = new BookmarkFile();

    BookmarksReader reader;
    reader.openFile(bookmarkFile, pBmk);

    assert(pBmk->getTitle()->getText()->getTextString() == title);
    assert(pBmk->getUid() == uid);
    if (pBmk->getLastmark() != NULL)
    {
        assert(pBmk->getLastmark()->mPlayOrder == playOrder);
        std::string comparer = createZeroPaddedString(playOrder);
        assert(pBmk->getLastmark()->mUri == comparer);
        assert(pBmk->getLastmark()->mNcxRef == comparer);
        assert(pBmk->getLastmark()->mTextRef == comparer);
        assert(pBmk->getLastmark()->mAudioRef == comparer);
        //assert(pBmk->getLastmark()->mTimeOffset == comparer);
        //assert(pBmk->getLastmark()->mCharOffset == comparer);
    }

    delete pBmk;
}

int main(int argc, char *argv[])
{
    setup_logging();

    const char* c_bookmarkFile = "./bookmark.bmk";
    std::string bookmarkFile = c_bookmarkFile;

    // create a bookmark
    BookmarkFile* pBmk = NULL;
    pBmk = new BookmarkFile();

    // add a title
    MediaGroup* pMgTitle = NULL;
    pMgTitle = new MediaGroup();
    TextNode* pTnTitle = NULL;
    pTnTitle = new TextNode();
    pTnTitle->setTextString(title);
    pMgTitle->setText(pTnTitle);
    pBmk->setTitle(pMgTitle);

    // add uid
    pBmk->setUid(uid);

    // save and verify bookmark
    saveBookmark(bookmarkFile, pBmk);
    verifyBoomark(bookmarkFile, -1);

    // add a lastmark
    PositionData* pPdLast = NULL;
    pPdLast = new PositionData();
    pPdLast->mPlayOrder = 1;
    pPdLast->mUri = "0000001";
    pPdLast->mNcxRef = "0000001";
    pPdLast->mTextRef = "0000001";
    pPdLast->mAudioRef = "0000001";
    //pPdLast->mTimeOffset = "0000001";
    //pPdLast->mCharOffset = "0000001";
    pBmk->setLastmark(pPdLast);

    // save and verify bookmark
    saveBookmark(bookmarkFile, pBmk);
    verifyBoomark(bookmarkFile, 1);

    // loop and increment values
    for (int i=0; i<10; i++)
    {
        int playOrder = pPdLast->mPlayOrder;
        playOrder++;
        std::string zeroPadStr = createZeroPaddedString(playOrder);
        pPdLast->mPlayOrder = playOrder;
        pPdLast->mUri = zeroPadStr;
        pPdLast->mNcxRef = zeroPadStr;
        pPdLast->mTextRef = zeroPadStr;
        pPdLast->mAudioRef = zeroPadStr;
        //pPdLast->mTimeOffset = zeroPadStr;
        //pPdLast->mCharOffset = zeroPadStr;

        // save and verify bookmark
        saveBookmark(bookmarkFile, pBmk);
        verifyBoomark(bookmarkFile, playOrder);
    }

    remove(c_bookmarkFile);
    delete pBmk;

    return 0;
}
