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

#ifndef SEQNODE_H
#define SEQNODE_H

//PROJECT INCLUDES
#include "Node.h"
#include "TimeContainerNode.h"
#include "SmilEngineConstants.h"

//!represents a Seq element in a SMIL file
/*!
 seq (sequence) nodes play one child node at a time.
 */
class SeqNode: public TimeContainerNode
{
public:
    //LIFECYCLE
    //!default constructor
    SeqNode();
    //!destructor
    ~SeqNode();

    //ACCESS
    //!return the type of node
    NodeType getTypeOfNode();

    //METHODS
    //!go to the next node
    amis::ErrorCode setNext();
    //!go to the previous node
    amis::ErrorCode setPrevious();
    //!go to the first node
    amis::ErrorCode setFirst();
    //!go to the last node
    amis::ErrorCode setLast();

    //!play this node
    void play(amis::SmilMediaGroup*);

    //!set this node's child index
    void setChildIndex(int);

private:
    //!index of the active child in this node's child node collection
    int mActiveChildIndex;
};

#endif
