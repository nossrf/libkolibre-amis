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

#ifndef SMILTREE_H
#define SMILTREE_H

//SYSTEM INCLUDES
#include <string>

//PROJECT INCLUDES
#include "AmisError.h"
#include "CustomTest.h"

#include "SmilMediaGroup.h"
#include "SmilEngineConstants.h"
#include "Node.h"
#include "SeqNode.h"

//! The Smil Tree is a tree structure representing the contents of one SMIL file.
/*!
 The tree's root node is a seq node and the root's children are par, seq, audio,
 text, and image nodes.

 Playback of a Smil Tree object can begin at any node in the tree. Next and 
 Previous commands traverse items contained within the most local sequence, 
 relative to the current playback position.
 */
class SmilTree
{

public:

    //LIFECYCLE
    //!default constructor
    SmilTree();
    //!destructor
    ~SmilTree();

    //ACCESS
    //!set the root node
    void setRoot(SeqNode*);
    //!get the root node
    Node* getRoot();

    //!set the smil file path
    void setSmilFilePath(std::string);
    //!get the smil file path
    std::string getSmilFilePath();

    //!get the current id
    std::string getCurrentId();
    //!set the current id
    void setCurrentId(std::string);

    //!get the recent text id
    std::string getRecentTextId();
    //!set the recent text id
    void setRecentTextId(std::string);

    //!get escape status
    bool getCouldEscape();
    //!set esape status
    void setCouldEscape(bool);

    //!set the skip option list
    void setSkipOptionList(std::vector<amis::CustomTest*>*);

    //!identify a node as being escapable
    void setPotentialEscapeNode(Node*);

    //!get smil duration
    unsigned int getSmilDuration();

    //!set smil duration
    void setSmilDuration(std::string durationStr);
    void setSmilDuration(unsigned int duration);

    //!add content region data
    void addContentRegion(ContentRegionData);
    //!get a list of content region data
    std::vector<ContentRegionData> getContentRegionList();

    //METHODS
    //!print the tree
    void print();

    //!go to the next group of parallel elements
    amis::AmisError goNext(amis::SmilMediaGroup*);
    //!go to the previous group of parallel elements
    amis::AmisError goPrevious(amis::SmilMediaGroup*);
    //!go to the first group of parallel elements
    amis::AmisError goFirst(amis::SmilMediaGroup*);
    //!go to the last group of parallel elements
    amis::AmisError goLast(amis::SmilMediaGroup*);
    //!go to a specific id
    amis::AmisError goToId(std::string, amis::SmilMediaGroup*);
    //!escape current structure
    amis::AmisError escapeStructure(amis::SmilMediaGroup*);

    //INQUIRY
    //!is the tree empty?
    bool isEmpty();
    //!is this node to be skipped or escaped?
    bool mustSkipOrEscapeNode(Node*);

private:
    //METHODS
    //!print a node
    void printNode(Node*, int);

    //!convert a duration string to seconds
    unsigned int stringToSeconds(std::string timeString);

    //MEMBER VARIABLES
    //!root of the tree
    SeqNode* mpRoot;
    //!current escape node if exists
    TimeContainerNode* mpEscapeNode;

    //!content region list
    std::vector<ContentRegionData> mRegions;

    //!skippable options list
    std::vector<amis::CustomTest*>* mpSkipOptions;

    //!path to smil file
    std::string mSmilFilePath;

    //!did the user request an escape?
    bool mbEscapeRequested;
    //!is the current set of parallel nodes escapable?
    bool mbCouldEscape;
    //!status of the tree
    amis::ErrorCode mTreeStatus;

    //!current node id
    std::string mCurrentId;
    //!recent text id
    std::string mRecentTextId;

    //!duration of smil tree
    unsigned int mDuration;
};

#endif
