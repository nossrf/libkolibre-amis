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

//SYSTEM INCLUDES
#include <iostream>

//PROJECT INCLUDES
#include "ContentNode.h"
#include "SmilTree.h"

using namespace std;

//--------------------------------------------------
/*!
 constructor sets pointers to null
 */
//--------------------------------------------------
ContentNode::ContentNode()
{
    mpMediaData = NULL;
    mNodeType = NONE;
}

//--------------------------------------------------
/*!
 destructor deallocates mpMediaData
 */
//--------------------------------------------------
ContentNode::~ContentNode()
{
    if (mNodeType != NONE)
    {
        delete mpMediaData;

    }
}

//--------------------------------------------------
/*!
 category of this node is CONTENT
 */
//--------------------------------------------------
NodeType ContentNode::getCategoryOfNode()
{
    return CONTENT;
}

//--------------------------------------------------
/*!
 type of media node  is TXT, AUD, IMG, or NONE
 */
//--------------------------------------------------
NodeType ContentNode::getTypeOfNode()
{
    return mNodeType;
}

//--------------------------------------------------
/*
 create mpMediaData as a new audio node
 */
//--------------------------------------------------
void ContentNode::createNewAudio()
{
    mpMediaData = new amis::AudioNode();
    mNodeType = AUD;

}

//--------------------------------------------------
/*
 create mpMediaData as a new image node
 */
//--------------------------------------------------
void ContentNode::createNewImage()
{
    mpMediaData = new amis::ImageNode();
    mNodeType = IMG;

}

//--------------------------------------------------
/*
 create mpMediaData as a new text node
 */
//--------------------------------------------------
void ContentNode::createNewText()
{
    mpMediaData = new amis::TextNode();
    mNodeType = TXT;

}

//--------------------------------------------------
/*
 return mpMediaData
 */
//--------------------------------------------------
amis::MediaNode* ContentNode::getMediaNode()
{
    return mpMediaData;
}

//--------------------------------------------------
/*!
 play the media node by adding it to the SmilMediaGroup which is initialized
 outside the SmilEngine.  The SmilEngine sends the media group pointer around
 the tree and each node which is asked to play adds itself to the media group
 */
//--------------------------------------------------
void ContentNode::play(amis::SmilMediaGroup* pPlayGroup)
{
    if (mNodeType == AUD)
    {
        pPlayGroup->addAudioClip((amis::AudioNode*) this->getMediaNode());

    }

    else if (mNodeType == IMG)
    {
        pPlayGroup->setImage((amis::ImageNode*) this->getMediaNode());
    }
    else if (mNodeType == TXT)
    {
        pPlayGroup->setText((amis::TextNode*) this->getMediaNode());
    }
    else
    {
        //nothing
    }

    //if the current id is blank, set it to the current media node
    if (this->getSmilTreePtr()->getCurrentId().compare("") == 0)
    {
        this->getSmilTreePtr()->setCurrentId(this->getElementId());
    }
}

