/*
 DaisyHandler: common system objects and utility routines

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

#ifndef HISTORYRECORDER_H
#define HISTORYRECORDER_H

#include "AmisCommon.h"
#include "Media.h"
#include "Bookmarks.h"
#include <string>
#include <vector>

// HistoryRecorder class is responsible for recording navigational history within a book

// The history class records all navigation (auto/user), back is always available,
// forward is available if the user previously only within MERGE_TIMEOUT seconds
// MERGE_TIMEOUT is necessary so that we don't branch too quickly before user has decided 
// stop browsing and start listening again

//!HistoryRecorder class
class AMISCOMMON_API HistoryRecorder
{

public:
    HistoryRecorder();
    ~HistoryRecorder();

    unsigned int getNumberOfItems();

    bool getPrevious(amis::PositionData**);
    bool getNext(amis::PositionData**);
    bool getLast(amis::PositionData**);

    void addItem(amis::PositionData *);

    void printItems();
    void mergeBranches();

private:
    // Timeout until history is merged with mTemp
    time_t mergeTime;
    bool bMerged;

    //history vector
    std::vector<amis::PositionData*> mItems;
    unsigned int mCurrentPos;

    //temporary branching vector
    std::vector<amis::PositionData*> mTemp;

    //functions for temp
    void clearTemp();
};

#endif

