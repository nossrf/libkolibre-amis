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

#include "HistoryRecorder.h"
#include <iostream>

#define MERGE_TIMEOUT 5

using namespace amis;

HistoryRecorder::HistoryRecorder()
{
    mCurrentPos = 0;
}

HistoryRecorder::~HistoryRecorder()
{
    //cout << "HistoryRecorder::~HistoryRecorder: Deleting browsing history" << endl;

    int sz, i;

    amis::PositionData *p_pos;

    // Delete contents of main branch
    sz = mItems.size() - 1;

    for (i = sz; i >= 0; i--)
    {
        p_pos = mItems[i];
        mItems.pop_back();
        delete p_pos;
    }

    // Delete contents of temporary branch
    sz = mTemp.size() - 1;
    for (i = sz; i >= 0; i--)
    {
        p_pos = mTemp[i];
        mTemp.pop_back();
        delete p_pos;
    }
}

void HistoryRecorder::clearTemp()
{
    if (mTemp.size() == 0)
        return;

    //cout << "HistoryRecorder::clearTemp() clearing temporary items" << endl;

    amis::PositionData *p_pos;

    int sz, i;
    // Delete contents of temporary branch
    sz = mTemp.size() - 1;
    for (i = sz; i >= 0; i--)
    {
        p_pos = mTemp[i];
        mTemp.pop_back();
        delete p_pos;
    }
}

unsigned int HistoryRecorder::getNumberOfItems()
{
    return mItems.size();
}

bool HistoryRecorder::getPrevious(PositionData **p_pos)
{
    if (time(NULL) > mergeTime)
    {
        // merge the branches if too much time has elapsed
        if (mTemp.size() > 0 || mCurrentPos != mItems.size())
        {
            mergeBranches();
        }
    }

    mergeTime = time(NULL) + MERGE_TIMEOUT;

    if (mCurrentPos > 0)
    {
        mCurrentPos--;
        clearTemp();
        //cout << "HistoryReader::getPrevious(): returning item " << mCurrentPos << endl;
        *p_pos = mItems[mCurrentPos];

        return true;
    }
    else
    {
        //cout << "HistoryReader::getPrevious(): no previous history item, returning first item" << endl;
        *p_pos = mItems[0];

    }

    return false;
}

bool HistoryRecorder::getNext(PositionData **p_pos)
{
    if (time(NULL) > mergeTime)
    {
        // merge the branches if too much time has elapsed
        if (mTemp.size() > 0 || mCurrentPos != mItems.size())
        {
            mergeBranches();
        }
    }

    mergeTime = time(NULL) + MERGE_TIMEOUT;

    if (mCurrentPos < mItems.size() - 1)
    {
        mCurrentPos++;
        clearTemp();
        *p_pos = mItems[mCurrentPos];
        return true;
    }
    else
    {
        //cout << "HistoryReader::getPrevious(): no next history item, returning last item" << endl;
        *p_pos = mItems[mItems.size() - 1];
    }

    return false;
}

bool HistoryRecorder::getLast(PositionData **p_pos)
{
    if (time(NULL) > mergeTime)
    {
        // merge the branches if too much time has elapsed
        if (mTemp.size() > 0 || mCurrentPos != mItems.size())
        {
            mergeBranches();
        }
    }

    mergeTime = time(NULL) + MERGE_TIMEOUT;

    if (mCurrentPos < mItems.size())
    {
        clearTemp();
        *p_pos = mItems[mCurrentPos];
        return true;
    }
    else
    {
        *p_pos = mItems[mItems.size() - 1];
        return true;
    }

    return false;
}

void HistoryRecorder::mergeBranches()
{
    PositionData *p_pos;

    unsigned int i;
    //cout << "HistoryRecorder::mergeBranches() merging mItems of size " << mItems.size() <<
    //" with mTemp of size " << mTemp.size() << " at mCurrentPos " << mCurrentPos << endl;

    // Clear items after current position
    for (i = mItems.size() - 1; i > mCurrentPos; i--)
    {
        p_pos = mItems[i];
        mItems.pop_back();
        //cout << "deleting mItems["<<i<<"]: "<<p_pos->mUri<< endl;
        delete p_pos;
    }

    //cout << "finished deleting items" << endl;

    // Move all remaining items from mTemp to mItems
    for (i = 0; i < mTemp.size(); i++)
    {
        p_pos = mTemp[i];
        if (i == 0)
        {
            delete p_pos;
        }
        else
        {
            //cout << "addming mTemp["<<i<<"]: "<<p_pos->mUri<< endl;
            mItems.push_back(p_pos);
        }
    }

    mTemp.clear();
    mCurrentPos = mItems.size();
}

void HistoryRecorder::addItem(PositionData *pPositionData)
{
    // If we haven't browsed the history for a while
    if (time(NULL) > mergeTime)
    {

        // If we haven't merged yet, merge the braches..
        if (mTemp.size() > 0 || mCurrentPos != mItems.size())
        {
            mergeBranches();
        }

        // ..and push the new item back on the mItems 
        //cout << "HistoryRecorder::addItem(): pushed item onto stack (size:" << mItems.size() <<")"<< endl;

        //if this is the first item
        if (mItems.size() == 0)
            mItems.push_back(pPositionData);

        //or if it's different from the previous one
        else if (mItems[mItems.size() - 1]->compare(pPositionData) == false)
            mItems.push_back(pPositionData);

        //delete the item if we dont want it
        else
        {
            //cout << "HistoryRecorder::addItem(): Item same as previous one, ignoring it" << endl;
            delete pPositionData;
        }

        mCurrentPos = mItems.size();

    }
    else
    {
        // if we are at the moment browsing the history,
        // push the item to the temporary stack if it's different from the previous one
        //cout << "HistoryRecorder::addItem(): pushing item onto temporary stack (size:" << mTemp.size() <<")"<< endl;

        if (mTemp.size() == 0)
            mTemp.push_back(pPositionData);
        else if (mTemp[mTemp.size() - 1]->compare(pPositionData) == false)
            mTemp.push_back(pPositionData);
        else
        {
            //cout << "HistoryRecorder::addItem(): Item same as previous one, ignoring it" << endl;
            delete pPositionData;
        }

    }
}

void HistoryRecorder::printItems()
{
    std::cout << "HistoryRecorder::printItems() mCurrentPos: " << mCurrentPos
            << std::endl;

    unsigned int i;
    std::cout << "HistoryRecorder::mItems()" << std::endl;
    for (i = 0; i < mItems.size(); i++)
        std::cout << "Item " << i << ": " << mItems[i]->mUri << " AudioRef: "
                << mItems[i]->mAudioRef << std::endl;

    std::cout << "HistoryRecorder::mTemp()" << std::endl;
    for (i = 0; i < mTemp.size(); i++)
        std::cout << "TempItem " << i << ": " << mTemp[i]->mUri << " AudioRef: "
                << mTemp[i]->mAudioRef << std::endl;

}
