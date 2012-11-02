/*
 NavParse: Navigation model for DAISY NCC and NCX

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

/**
 * @class amis::NavNode
 *
 * @brief Base node for building a tree of navigation nodes
 *
 * @author Kolibre (www.kolibre.org)
 *
 * Contact: info@kolibre.org
 *
 */

#include <iostream> 

#include "NavNode.h"

using namespace std;
using namespace amis;

NavNode::NavNode()
{
    mContent = "";
    mId = "";
    mPlayOrder = -1;
    mClass = "";
    mpLabel = NULL;
}

NavNode::~NavNode()
{
//	cerr<<"Deleting: "<<mId<<endl;
    if (mpLabel != NULL)
    {
        mpLabel->destroyContents();
        delete mpLabel;
    }
}

void NavNode::setLabel(amis::MediaGroup* pLabel)
{
    mpLabel = pLabel;
}

void NavNode::setContent(std::string content)
{
    mContent = content;
}

void NavNode::setId(std::string id)
{
    mId = id;
}

void NavNode::setPlayOrder(int playOrder)
{
    mPlayOrder = playOrder;
}

void NavNode::setClass(std::string classname)
{
    mClass = classname;
}

amis::MediaGroup* NavNode::getLabel()
{
    return mpLabel;
}

//this function returns a content URL and denotes the node as 'selected'
//the play order in the nav model is changed.
std::string NavNode::getContent()
{
    return mContent;
}

std::string NavNode::getId()
{
    return mId;
}

int NavNode::getPlayOrder()
{
    return mPlayOrder;
}

std::string NavNode::getClass()
{
    return mClass;
}

NavNode::TypeOfNode NavNode::getTypeOfNode()
{
    return mTypeOfNode;
}

//--------------------------------------------------
/*! 
 Print information about this node to the screen.
 @param[in] level
 the number of tabs to indent each line
 */
//--------------------------------------------------
void NavNode::print(int level)
{
    // loop counter
    int i, j;

    // print out the correct number of tabs
    for (i = 0; i < level; i++)
    {
        cerr << "\t";
    }
    // print out this node's Id
    cerr << "ID: " << mId << endl;

    // print out the correct number of tabs
    for (i = 0; i < level; i++)
    {
        cerr << "\t";
    }
    cerr << "CONTENT: " << mContent << endl;

    if (this->mpLabel != NULL)
    {

        if (this->mpLabel->getText() != NULL)
        {
            for (i = 0; i < level; i++)
            {
                cerr << "\t";
            }

            cerr << "TEXT: " << this->mpLabel->getText()->getTextString()
                    << endl;
        }

        for (j = 0; j < (int) this->mpLabel->getNumberOfAudioClips(); j++)
        {
            for (i = 0; i < level; i++)
            {
                cerr << "\t";
            }
            cerr << "AUDIO: " << this->mpLabel->getAudio(j)->getSrc()
                    << " from " << this->mpLabel->getAudio(j)->getClipBegin()
                    << " to " << this->mpLabel->getAudio(j)->getClipEnd()
                    << endl;
        }
    }

    for (i = 0; i < level; i++)
    {
        cerr << "\t";
    }
    cerr << "ORDER: " << this->getPlayOrder() << endl;

    for (i = 0; i < level; i++)
    {
        cerr << "\t";
    }
    cerr << "CLASS: " << this->getClass() << endl;

    cerr << endl;
}
