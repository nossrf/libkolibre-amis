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

#ifndef AMISBRAIN_H
#define AMISBRAIN_H

#include "SmilEngine.h"
#include "Media.h"
#include "Bookmarks.h"
#include "AmisError.h"

class DaisyTest
{
protected:
    DaisyTest();

public:

    static DaisyTest* Instance();
    void DestroyInstance();
    ~DaisyTest();

    void exit();

    bool openFile(std::string, std::string uid = "", bool bStartAtLastmark = true,
            bool bIsStartBook = false);

    bool nextPhrase(bool rewindWhenEndOfBook = false);
    bool prevPhrase();
    bool escape();
    bool loadSmilContent(std::string, bool flagNoSync = false);

    bool nextInNavList(int);
    bool prevInNavList(int);
    bool nextPage();
    bool prevPage();
    bool goToPage(std::string);

    void continuePlayingMediaGroup();
    void playMediaGroup(amis::SmilMediaGroup* pMedia, bool flagNoSync = false);

    amis::SmilMediaGroup* getCurrentMediaGroup();
    std::string getBookFilePath();

    void printMediaGroup(amis::SmilMediaGroup*);

    void reportGeneralError(amis::AmisError err);

    void printNavLists();
    void printPageList();

private:
    amis::BookmarkFile* mpBmk;
    amis::SmilMediaGroup* mpCurrentMedia;

    int mCurrentVolume;
    bool mbFlagNoSync;
    int mFontSize;
    bool mbCanEscapeCurrent;
    bool mbMenusNeedUpdateOnViewSwitch;

    std::string mFilePath;
    std::string mBmkFilePath;
    std::string mLastmarkUri;
    std::string mAppPath;

private:
    static DaisyTest* pinstance;
};

#endif
