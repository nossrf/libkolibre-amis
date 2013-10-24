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
 * @class amis::NavPoint
 *
 * @brief Navigation pointer for navigating a tree
 *
 * @author Kolibre (www.kolibre.org)
 *
 * Contact: info@kolibre.org
 *
 */

#include <iostream>
#include "NavPoint.h"
#include "NavMap.h"
#include "NavModel.h"

#include <log4cxx/logger.h>
using namespace amis;

// create logger which will become a child to logger kolibre.amis
log4cxx::LoggerPtr amisNavPointLog(
        log4cxx::Logger::getLogger("kolibre.amis.navpoint"));

NavPoint::NavPoint()
{
    mNumChildren = 0;
    mpSibling = NULL;
    mpParent = NULL;
    mpFirstChild = NULL;
    mChildCount = -1;

    mTypeOfNode = amis::NavNode::NAV_POINT;
}

/**
 * Destructor
 *
 * Delete all children recursively
 */
NavPoint::~NavPoint()
{
    NavPoint* p_tmp_node;

    //if this node has no children, perform any internal cleanup and return
    if (mpFirstChild == NULL)
    {
        return;
    }

    //else, this node has one or more children
    else
    {
        //loop through the children and delete one by one
        while (mNumChildren > 0)
        {
            p_tmp_node = getChild(mNumChildren - 1);
            delete p_tmp_node;

            //each time a child is deleted, decrement the number of children
            mNumChildren--;
        }
    }
}
/**
 * Reset the number of child counter
 */
void NavPoint::resetChildCount()
{
    LOG4CXX_TRACE( amisNavPointLog,
            "node " << getPlayOrder() << ": Resetting childcount");
    mChildCount = -1;
}

/**
 * Navigate to the next child
 *
 * @return Returns the next child
 * @return NULL if there is no next child
 */
NavPoint* NavPoint::next()
{
    LOG4CXX_TRACE( amisNavPointLog,
            "node " << getPlayOrder() << ": NavPoint* NavPoint::next() childcount: " << mChildCount);

    if (mChildCount < mNumChildren - 1)
    {
        mChildCount++;
        LOG4CXX_TRACE( amisNavPointLog,
                "node " << getPlayOrder() << ": Returning child: " << mChildCount+1 << "(" << mNumChildren << ")");
        NavPoint* ptmp = getChild(mChildCount);
        ptmp->resetChildCount();
        return ptmp;
    }
    else
    {
        // we are leaving this branch so reset the child count
        this->resetChildCount();
        if (mpParent != NULL)
        {
            LOG4CXX_TRACE( amisNavPointLog,
                    "node " << getPlayOrder() << ": Returning parents next node");
            return mpParent->next();
        }
        else
        {
            LOG4CXX_TRACE( amisNavPointLog,
                    "node " << getPlayOrder() << ": Returning NULL");
            return NULL;
        }
    }

}

/**
 * Navigate to the previous child
 *
 * @return Returns the previous child
 * @return NULL if there is no previous child
 */
NavPoint* NavPoint::previous()
{
    LOG4CXX_TRACE( amisNavPointLog,
            "node " << getPlayOrder() << ": NavPoint* NavPoint::previous() childcount: " << mChildCount);

    if (mChildCount > 0)
    {
        mChildCount--;

        LOG4CXX_TRACE( amisNavPointLog,
                "node " << getPlayOrder() << ": Returning child: " << mChildCount+1 << "(" << mNumChildren << ")");
        NavPoint* ptmp = getChild(mChildCount);
        ptmp->resetChildCount();
        return ptmp;

    }
    else if (mChildCount == 0)
    {
        LOG4CXX_TRACE( amisNavPointLog,
                "node " << getPlayOrder() << ": No previous child, returning NULL");
        this->resetChildCount();
        return NULL;
    }
    else
    {
        // we are leaving this branch so reset the child count
        this->resetChildCount();
        if (mpParent != NULL)
        {
            NavPoint* ptmp = mpParent->previous();
            if (ptmp != NULL)
            {
                if (ptmp->getNumChildren() > 0)
                {
                    LOG4CXX_TRACE( amisNavPointLog,
                            "node " << getPlayOrder() << ": Returning parents previous last child");
                    return ptmp->getChild(ptmp->getNumChildren() - 1);
                }
                else
                {
                    LOG4CXX_TRACE( amisNavPointLog,
                            "node " << getPlayOrder() << ": Returning parents previous");
                    return ptmp;
                }
            }

            LOG4CXX_DEBUG( amisNavPointLog,
                    "node " << getPlayOrder() << ": No previous child, returning parent");
            return mpParent;
        }
        else
        {
            LOG4CXX_DEBUG( amisNavPointLog,
                    "node " << getPlayOrder() << ": Returning NULL");
            return NULL;
        }
    }
}

/**
 * Get the immediate sibling
 *
 * @return Returns the first sibling known
 */
NavPoint* NavPoint::getFirstSibling()
{
    return mpSibling;
}

/**
 * Get child at index
 *
 * @param index The index of the child wanted
 * @return Returns the child at index
 * @return NULL if there is no such child
 */
NavPoint* NavPoint::getChild(int index)
{
    //local variables
    NavPoint* p_tmp_node;

    //check the bounds of the requested index
    if (index < mNumChildren && index >= 0)
    {
        //initialize the temp variable to the first child pointer
        //we can assume it exists because of the if-statement conditions
        p_tmp_node = mpFirstChild;

        //for-loop to iterate through the sibling list until we get to the node at index-1
        for (int i = 0; i < index; i++)
        {
            p_tmp_node = p_tmp_node->getFirstSibling();
        }

        mChildCount = index;

        //return a pointer to the requested child
        return p_tmp_node;
    }

    else
    {
        //index out of bounds or this node has no children
        return NULL;
    }

}

/**
 * Get the number of children
 *
 * @return Return the number of children that this node has
 */
//
//--------------------------------------------------
int NavPoint::getNumChildren()
{
    return mNumChildren;
}

/**
 * Get the level of this node
 *
 * @return Returns the level of this node
 */
int NavPoint::getLevel()
{
    return mLevel;
}

/**
 * Add a sibling to this node
 *
 * This is a recursive function that will add the new node at the
 * end of the sibling chain
 *
 * @param pNode The node we wish to add as sibling
 */
void NavPoint::addSibling(NavPoint* pNode)
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


/**
 * Add a child to this node
 *
 * This function will add a child to this specific node
 *
 * @param pNode The new node to set as child
 */
void NavPoint::addChild(NavPoint* pNode)
{
    //check if first child does not exist
    if (mNumChildren == 0 || mpFirstChild == NULL)
    {
        //set new node pointer as first child
        mpFirstChild = pNode;

        //set the new child's parent to be this node
        mpFirstChild->setParent(this);
    }

    //if exists, add new node pointer as a sibling
    else
    {
        mpFirstChild->addSibling(pNode);
    }

    mNumChildren++;
}

/**
 * Set a the level of this node
 *
 * @param level Set the lelvel for this node
 */
void NavPoint::setLevel(int level)
{
    mLevel = level;
}

/**
 * Set a pointer to this node's parent
 *
 * @param pNode The node we want to set as parent
 */
void NavPoint::setParent(NavPoint* pNode)
{
    mpParent = pNode;
}

/**
 * Get a pointer to this node's parent
 *
 * @return Returns the parent node
 */
NavPoint* NavPoint::getParent()
{
    return mpParent;
}

