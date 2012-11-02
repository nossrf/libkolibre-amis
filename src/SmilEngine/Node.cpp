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
#include "SmilEngineConstants.h"
#include "TimeContainerNode.h"
#include "Node.h"

using namespace std;

//--------------------------------------------------
/*!
 the constructor initializes member variables
 */
//--------------------------------------------------
Node::Node()
{
    mElementId = "";
    mpSibling = NULL;
    mpParent = NULL;
    mpSmilTree = NULL;
}

//--------------------------------------------------
/*!
 the destructor does nothing special
 */
//--------------------------------------------------
Node::~Node()
{
    //empty function
}

//--------------------------------------------------
//	Save a pointer to a SmilTree object.
/*!
 @pre
 This node object is part of that SmilTree
 */
//--------------------------------------------------
void Node::setSmilTreePtr(SmilTree* pTree)
{
    mpSmilTree = pTree;
}

//--------------------------------------------------
/*
 return a pointer to the smil tree
 */
//--------------------------------------------------
SmilTree* Node::getSmilTreePtr()
{
    return mpSmilTree;
}

//--------------------------------------------------
// Add a sibling to this node
/*!
 This is a recursive function that will add the new node at the 
 end of the sibling chain
 */
//--------------------------------------------------
void Node::addSibling(Node* pNode)
{
    //if-else block to see if this node's sibling pointer is null
    if (mpSibling == NULL)
    {
        //add the new node pointer as its sibling
        mpSibling = pNode;

        //set the new node's parent to be the same as this node's parent
        mpSibling->setParent(this->getParent());
    }
    else
    {
        //recursive call to try and add the new node to the sibling of this node
        mpSibling->addSibling(pNode);
    }
    //end if-else block to see if this node's sibling pointer is null

}

//--------------------------------------------------
// Set a pointer to this node's parent
//--------------------------------------------------
void Node::setParent(TimeContainerNode* pParent)
{
    mpParent = pParent;
}

//--------------------------------------------------
// Get a pointer to this node's parent
//--------------------------------------------------
TimeContainerNode* Node::getParent()
{
    return mpParent;
}

//--------------------------------------------------
//Geturn a pointer to this node's sibling
/*!
 each node has one sibling
 */
//--------------------------------------------------
Node* Node::getFirstSibling()
{
    return mpSibling;
}

//--------------------------------------------------
//Get this node's ID, as given in the Smil File
//--------------------------------------------------
string Node::getElementId()
{
    return mElementId;
}

//--------------------------------------------------
//	Set this node's Id.
/*!
 @pre
 Id exists for this node in the Smil File
 */
//--------------------------------------------------
void Node::setElementId(string newElementId)
{
    mElementId = newElementId;
}

//--------------------------------------------------
/*! 
 Print information about this node to the screen.
 @param[in] level
 the number of tabs to indent each line
 */
//--------------------------------------------------
void Node::print(int level)
{
    // loop counter
    int i;

    //Node type
    NodeType node_type;

    node_type = this->getTypeOfNode();

    // print out the correct number of tabs
    for (i = 0; i < level; i++)
    {
        cerr << "\t";
    }

    // print out this node's Id
    cerr << "ID: " << mElementId << endl;

    // print out the correct number of tabs
    for (i = 0; i < level; i++)
    {
        cerr << "\t";
    }

    // long "if, else if, else" block to check node_type and print the right statement
    if (node_type == A)
    {
        cerr << "Type: A" << endl;
    }
    else if (node_type == SEQ)
    {
        cerr << "Type: SEQ" << endl;
    }

    else if (node_type == PAR)
    {
        cerr << "Type: PAR" << endl;
    }
    else if (node_type == TXT)
    {
        cerr << "Type: TXT" << endl;
    }

    else if (node_type == AUD)
    {
        cerr << "Type: AUD" << endl;
    }

    else if (node_type == IMG)
    {
        cerr << "Type: IMG" << endl;
    }

    else
    {
        cerr << "Type: NONE" << endl;
    }
    //end of long "if, else if, else" block to check node_type and print accordingly

    //print out an empty line
    cerr << endl;
}
