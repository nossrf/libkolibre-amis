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
#ifndef SPINE_H
#define SPINE_H

//SYSTEM INCLUDES
#include <string>
#include <vector>

//PROJECT INCLUDES

//! represents an in-order list of all SMIL files that make up a Daisy book

/*!
 The Spine keeps track of the in-order list of all SMIL files that 
 make up a Daisy book.
 */
class Spine
{

public:
    //LIFECYCLE
    //!default constructor
    Spine();
    //!destructor
    ~Spine();

    //ACCESS
    //!add a file to the spine
    void addFile(std::string);

    //INQUIRY
    //!see if a file exists in the spine
    bool isFilePresent(std::string);
    //!get the status of the spine
    amis::ErrorCode getStatus();
    //!is the spine empty?
    bool isEmpty();
    //!free memory of spinelist
    void freeSpineList();

    //METHODS
    //!get the next filepath in the spine
    std::string getNextFile();
    //!get the previous filepath in the spine
    std::string getPreviousFile();
    //!get the first file in the spine
    std::string getFirstFile();
    //!get the last file in the spine
    std::string getLastFile();
    //!go to a particular file in the spine
    bool goToFile(std::string);
    //!print a list of all files in the spine
    void printList();
    //!get the number of SMIL files
    int getNumberOfSmilFiles();
    //!get the filepath of a SMIL file
    std::string getSmilFilePath(unsigned int);

private:
    //MEMBER VARIABLES
    //!the file list
    std::vector<char *> mSpineList;
    //!the current list index
    unsigned int mListIndex;
    //!the status
    amis::ErrorCode mStatus;
};

#endif
