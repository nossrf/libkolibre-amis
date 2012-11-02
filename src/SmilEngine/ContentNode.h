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

#ifndef CONTENTNODE_H
#define CONTENTNODE_H

//PROJECT INCLUDES
#include "Media.h"
#include "Node.h"
#include "SmilMediaGroup.h"

//! ContentNode represents any media reference in a SMIL file
/*!
 the media data itself is represented by an amis::MediaNode member variable.
 at runtime, this variable created as a text, audio, or image node
 */
class ContentNode: public Node
{
public:

    //LIFECYCLE
    //!default constructor
    ContentNode();
    //!destructor
    ~ContentNode();

    //INQUIRY
    //!return the category of node
    NodeType getCategoryOfNode();
    //!return the type of node
    NodeType getTypeOfNode();

    //ACCESS
    //!create the media node as a new audio node
    void createNewAudio();
    //!create the media node as a new image node
    void createNewImage();
    //!create the media node as a new text node
    void createNewText();
    //!play this node
    void play(amis::SmilMediaGroup*);

    //!return the media node, which contains all data required for rendering
    amis::MediaNode* getMediaNode();

private:
    //!media node data
    amis::MediaNode* mpMediaData;
    //!have we allocated mpMediaData as a specific type of node?
    bool mbAllocatedMediaNode;
    //!node type (content node in this case)
    NodeType mNodeType;

};

#endif
