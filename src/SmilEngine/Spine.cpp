/*
 SMIL Engine: linear and skippable navigation of Daisy 2.02 and Daisy 3 SMIL contents

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
#include "FilePathTools.h"
#include "SmilEngineConstants.h"
#include "Spine.h"

//SYSTEM INCLUDES
#include <string>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.amis
log4cxx::LoggerPtr amisSpineLog(
        log4cxx::Logger::getLogger("kolibre.amis.spine"));

using namespace std;

//--------------------------------------------------
//Default constructor
//--------------------------------------------------
Spine::Spine() :
        mSpineList()
{
    //initialize variables
    mListIndex = 0;
    mStatus = amis::OK;
}

//--------------------------------------------------
//Destructor
//--------------------------------------------------
Spine::~Spine()
{
    freeSpineList();
}

//--------------------------------------------------
//return the next file in the spine list
//--------------------------------------------------
string Spine::getNextFile()
{
    //if we will not pass the end of the list
    if (mListIndex + 1 < mSpineList.size() && mListIndex + 1 >= 0)
    {
        //increment list index and record the status as OK
        mListIndex++;
        mStatus = amis::OK;
    }

    //else, we have passed the end of the list
    else
    {
        //record status as LIST_END and do not increment mListIndex
        mStatus = amis::AT_END;
    }

    //return the filepath at mListIndex.  
    return mSpineList[mListIndex];

}

//--------------------------------------------------
//return the previous file in the spine list
//--------------------------------------------------
string Spine::getPreviousFile()
{
    //if we will not pass the beginning of the list
    if (mListIndex - 1 >= 0 && mListIndex - 1 < mSpineList.size())
    {
        //decrement list index and record the status as OK
        mListIndex--;
        mStatus = amis::OK;
    }

    //else, we have passed the beginning of the list
    else
    {
        //record the status as LIST_BEGIN and do not decrement mListIndex
        mStatus = amis::AT_BEGINNING;
    }

    //return the filepath at mListIndex
    return mSpineList[mListIndex];
}

//--------------------------------------------------
//return the first file in the spine list
//--------------------------------------------------
string Spine::getFirstFile()
{
    mStatus = amis::OK;
    mListIndex = 0;
    return mSpineList[mListIndex];
}

//--------------------------------------------------
//return the last file in the spine list
//--------------------------------------------------
string Spine::getLastFile()
{
    mStatus = amis::OK;
    mListIndex = mSpineList.size() - 1;
    return mSpineList[mListIndex];
}

//--------------------------------------------------
/*!
 status is OK, LIST_END, LIST_BEGIN
 */
//--------------------------------------------------
amis::ErrorCode Spine::getStatus()
{
    return mStatus;
}

//--------------------------------------------------
//check to see if a file is present in the spine
//--------------------------------------------------
bool Spine::isFilePresent(const string filePath)
{
    // Special case if mSpineList is empty
    if (mSpineList.size() == 0)
        return false;

    //local variables
    bool b_exists = false;
    const char *file_to_compare = amis::FilePathTools::getAsLocalFilePath(
            filePath).c_str();
    string file_in_list;

    //for-loop through the spine list from beginning to end
    for (int i = mSpineList.size() - 1; i >= 0; i--)
    {
        if (strstr(file_to_compare, mSpineList[i]) != NULL)
        {
            b_exists = true;
            break;
        }
    } //! end for-loop

    return b_exists;
}

//--------------------------------------------------
//add a file to the spine
//--------------------------------------------------
void Spine::addFile(string filePath)
{

    //LOG4CXX_DEBUG(amisSpineLog, "Spine::addFile() " << filePath);

    //if the file doesn't already exist, add it to the spine list
    if (isFilePresent(filePath) == false)
    {
        filePath = amis::FilePathTools::clearTarget(filePath);
        char *path = strdup(filePath.c_str());
        mSpineList.push_back(path);
    }
}

//--------------------------------------------------
//print the spine
//--------------------------------------------------
void Spine::printList()
{
    unsigned int i;

    for (i = 0; i < mSpineList.size(); i++)
    {
        cerr << i << ". " << mSpineList[i] << endl;
    }
}

//--------------------------------------------------
//does the spine contain files?
//--------------------------------------------------
bool Spine::isEmpty()
{
    if (mSpineList.size() == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}
//--------------------------------------------------
//free the spinelist
//--------------------------------------------------
void Spine::freeSpineList()
{
    //remove anything from mSpineList char* list
    char* tmp = NULL;
    //cout << "Starting freeing spinelist with " << mSpineList.size() << " items" << endl;

    while (mSpineList.size() > 0)
    {
        tmp = mSpineList[mSpineList.size() - 1];
        //cout << "Freeing item " << mSpineList.size() - 1 << " '" << tmp << "'" <<endl;
        if (tmp != NULL)
            free((void *) tmp);
        mSpineList.pop_back();

    }
}

//--------------------------------------------------
//set the spine at this particular file, if exists
//--------------------------------------------------
bool Spine::goToFile(string filePath)
{
    string file_to_compare = amis::FilePathTools::getAsLocalFilePath(filePath);
    string file_in_list;
    bool b_found = false;

    //convert to lower case
    std::transform(file_to_compare.begin(), file_to_compare.end(),
            file_to_compare.begin(), (int (*)(int))tolower);

            //for-loop through the spine list from beginning to end
for(    unsigned int i=0; i<mSpineList.size(); i++)
    {
        //get the file in the list in its local file path form
        file_in_list = amis::FilePathTools::getAsLocalFilePath(mSpineList[i]);

        //convert to lower case
        std::transform(file_in_list.begin(), file_in_list.end(),
        file_in_list.begin(), (int(*)(int))tolower);

        //compare the filepaths and see if they are equal
        if (file_in_list.compare(file_to_compare) == 0)
        {
            b_found = true;
            mListIndex = i;
            mStatus = amis::OK;
            break;
        }
    } //! end for-loop

    return b_found;
}

//--------------------------------------------------
//get the number of smil files in the publication
//--------------------------------------------------
int Spine::getNumberOfSmilFiles()
{
    return mSpineList.size();
}

//--------------------------------------------------
//get the source path for a smil file (by index)
//--------------------------------------------------
string Spine::getSmilFilePath(unsigned int idx)
{
    if (idx >= 0 && idx < mSpineList.size())
    {
        return mSpineList[idx];
    }
    else
    {
        return "";
    }
}

