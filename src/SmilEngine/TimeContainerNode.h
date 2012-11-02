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

#ifndef TIMECONTAINERNODE_H
#define TIMECONTAINERNODE_H

//PROJECT INCLUDES
#include "Node.h"
#include "SmilEngineConstants.h"

//! Represents a seq or a par.
/*!
 A time container node will be a seq or a par.  The purpose of this class is to 
 provide	handling for adding and retrieving child nodes.
 */
class TimeContainerNode: public Node
{
public:
    //LIFECYCLE
    //!default constructor
    TimeContainerNode();
    //!destructor
    ~TimeContainerNode();

    //ACCESS
    //!add a child node
    void addChild(Node*);
    //!get the child at the specified index
    Node* getChild(int);

    //!set a skippability option for this time container
    void setSkipOption(std::string);
    //!get this time container's skippability option
    std::string getSkipOption();

    //INQUIRY
    //!get the number of children
    int NumChildren();
    //!get the category of this node
    NodeType getCategoryOfNode();

    //METHODS
    //!set index at first child
    virtual amis::ErrorCode setFirst() = 0;
    //!set index at last child
    virtual amis::ErrorCode setLast() = 0;
    //!set index at next child
    virtual amis::ErrorCode setNext() = 0;
    //!set index at previous child
    virtual amis::ErrorCode setPrevious() = 0;
    //!set index at the child with this id
    bool setAtId(std::string);

private:
    //MEMBER VARIABLES
    //!pointer to first child
    Node* mpFirstChild;
    //!number of children
    int mNumChildren;
    //!skippability option name if exists
    std::string mSkipOptionName;
};

#endif
