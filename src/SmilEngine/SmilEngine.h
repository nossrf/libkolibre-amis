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

#ifndef SMILENGINE_H
#define SMILENGINE_H

//SYSTEM INCLUDES
#include <string>

//PROJECT INCLUDES
#include "AmisError.h"
#include "SmilMediaGroup.h"
#include <string>

#ifdef WIN32
#include <windows.h>
#endif

//! The Smil Engine is the main object of this library.
/*

 This class handles the SMIL playback for a Daisy book.

 The SmilEngine is initialized with two things:

 1. a spine file (ncc, master.smil, or opf)
 2. list of skippability options and their default states

 The Smil engine renders a single file at a time.  when a file reaches its end,
 the next file is loaded from the spine object.

 Traversal functions (previous, next, loadPosition, first, last, escape) return
 a media group object representing the data to be rendered as a result of this
 traversal request.
 */

class SpineBuilder;
class SmilTreeBuilder;
class SmilTree;
class Spine;
class CustomTest;

namespace amis {

class SMILENGINE_API SmilEngine
{

public:
    static SmilEngine* Instance();
    void DestroyInstance();

    //!destructor
    ~SmilEngine();

protected:
    //!default constructor
    SmilEngine();

public:
    //!print the skippability options
    void printSkipOptions();
    //!open a book
    amis::AmisError openBook(std::string, SmilMediaGroup*);
    //!close the book
    void closeBook();
    //!go to the next element
    amis::AmisError next(SmilMediaGroup*);
    //!go to the previous element
    amis::AmisError previous(SmilMediaGroup*);
    //!go to the first element in a smil file
    amis::AmisError first(SmilMediaGroup*);
    //!go to the last element in a smil file
    amis::AmisError last(SmilMediaGroup*);
    //!add a skippability option
    void addSkipOption(amis::CustomTest*);
    //!escape the current element and move on
    amis::AmisError escapeCurrent(SmilMediaGroup*);
    //!load a specific position
    amis::AmisError loadPosition(std::string, SmilMediaGroup*);
    //!change a skippability option
    bool changeSkipOption(std::string, bool);
    //!get the number of SMIL files
    int getNumberOfSmilFiles();
    //!get the filepath of a SMIL file
    std::string getSmilFilePath(int);

    //!return the source path for the current smil file
    std::string getSmilSourcePath();

    //!return metadata in currently open smil file
    std::string getMetadata(std::string);

    //!return daisy version for open book
    int getDaisyVersion();

    void printTree();

private:

    //METHODS
    //!record the current position
    void recordPosition();
    //!create a new smil tree from a file
    amis::AmisError createTreeFromFile(std::string, SmilMediaGroup*);
    //!clear all skippability options
    void clearAllSkipOptions();

    //MEMBER VARIABLES
    //!the spine builder
    SpineBuilder *mSpineBuilder;
    //!the smil tree builder
    SmilTreeBuilder *mSmilTreeBuilder;

    //!the smil tree
    SmilTree* mpSmilTree;
    //!the last known good tree
    SmilTree *mpOldSmilTree;
    //!the spine
    Spine* mpSpine;

    //!a list of skippability options
    std::vector<amis::CustomTest*> mSkipOptions;

    //!status of the spine build process
    amis::ErrorCode mSpineBuildStatus;
    //!status of the smil tree build process
    amis::ErrorCode mSmilTreeBuildStatus;

    //!end of tree flag
    bool mbEndOfTree;
    //!load id flag
    bool mbLoadId;

    //!path to the book's spine file
    std::string mBookFile;
    //!version of daisy for this book
    int mDaisyVersion;

    //!last position URI
    std::string mLastPosition;
    //!target to seek until
    std::string mIdTarget;

    //singleton instance
    static SmilEngine* pinstance;

};

}  // namespace amis

#endif

