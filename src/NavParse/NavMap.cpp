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
 * @class amis::NavMap
 *
 * @brief Class for holding the root of navigation tree
 *
 * @author Kolibre (www.kolibre.org)
 *
 * Contact: info@kolibre.org
 *
 */

#include <iostream>
#include "FilePathTools.h"
#include "NavContainer.h"
#include "NavMap.h"
#include <cstdlib>
#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.amis
log4cxx::LoggerPtr amisNavMapLog(
        log4cxx::Logger::getLogger("kolibre.amis.navmap"));

using namespace std;
using namespace amis;

NavMap::NavMap()
{
    mpRoot = NULL;
    mMaxDepth = -1;
}

NavMap::~NavMap()
{
    deleteCache();
    delete mpRoot;
}

//--------------------------------------------------
//print the tree
//--------------------------------------------------
void NavMap::print()
{
    LOG4CXX_INFO(amisNavMapLog, "***TREE***");
    printNode(mpRoot, 0);
}

//--------------------------------------------------
/*!
 recursive function to print a node at a certain indentation level
 */
//--------------------------------------------------
int counter = 0;
void NavMap::printNode(amis::NavPoint* pNode, int level)
{
    stringstream nodeout;
    int cnt;
    int i;

    // print out the correct number of tabs
    for (i = 0; i < level; i++)
    {
        nodeout << "\t";
    }
    nodeout << "COUNTER = " << counter++ << endl;

    for (i = 0; i < level; i++)
    {
        nodeout << "\t";
    }
    nodeout << "LEVEL = " << pNode->getLevel() << endl;

    for (i = 0; i < level; i++)
    {
        nodeout << "\t";
    }
    nodeout << "CHILDREN = " << pNode->getNumChildren() << endl;

    for (i = 0; i < level; i++)
    {
        nodeout << "\t";
    }
    nodeout << "ORDER = " << pNode->getPlayOrder() << endl;

    amis::NavPoint* p_tmp = pNode->getParent();
    if (p_tmp != NULL)
    {
        // print out the correct number of tabs
        for (i = 0; i < level; i++)
        {
            nodeout << "\t";
        }
        nodeout << "PARENT = " << p_tmp->getId() << endl;
    }
    else
    {
        nodeout << "I AM ROOT" << endl;
    }
    LOG4CXX_INFO(amisNavMapLog, nodeout);
    //print this node
    if (level > 0)
    {
        pNode->print(level);
    }

    //print the children
    for (cnt = 0; cnt < pNode->getNumChildren(); cnt++)
    {
        printNode(pNode->getChild(cnt), level + 1);
    }
}

amis::NavPoint* NavMap::getRoot()
{
    return mpRoot;
}

void NavMap::setRoot(amis::NavPoint* pNode)
{
    mpRoot = pNode;

    recordNewDepth(pNode->getLevel());
}

void NavMap::recordNewDepth(int new_depth)
{
    if (new_depth > mMaxDepth)
    {
        mMaxDepth = new_depth;
    }
}

int NavMap::getMaxDepth()
{
    return mMaxDepth;
}

//overrides
void NavMap::updateCurrent(amis::NavNode* pNode)
{
    mpCurrent = pNode;
}

//find the node with this play order
amis::NavNode* NavMap::syncPlayOrder(int playOrder)
{
    amis::NavPoint* p_node = (amis::NavPoint*) first();

    LOG4CXX_DEBUG(amisNavMapLog, "NavMap::syncPlayOrder(): searching for " << playOrder );

    bool b_found = false;

    while (b_found == false && p_node != NULL)
    {
        if (p_node->getPlayOrder() == playOrder)
        {
            b_found = true;
        }
        else
        {
            p_node = p_node->next();
        }
    }

    if (b_found == true)
    {
        LOG4CXX_DEBUG(amisNavMapLog, "Found it!! " << p_node->getPlayOrder());
        mpCurrent = p_node;
        return mpCurrent;
    }
    else
    {
        LOG4CXX_DEBUG(amisNavMapLog, "Could not find " << playOrder);
        return NULL;
    }
}

//make the current nav point the one with this smil ref if exists
amis::NavNode* NavMap::goToContentRef(const std::string contentHref)
{
    if (mpNavMapCache.size() == 0)
        createCache();

    map<const char *, NavPoint *>::const_iterator iter = mpNavMapCache.find(
            contentHref.c_str());
    if (iter != mpNavMapCache.end())
    {
        LOG4CXX_DEBUG(amisNavMapLog, "Got node from map " << iter->first);
        mpCurrent = iter->second;
        return mpCurrent;
    }

    return NULL;
}

void NavMap::createCache()
{
    NavPoint* p_node = (NavPoint*) first();

    string content_href;
    string content_target;

    while (p_node != NULL)
    {
        content_href = p_node->getContent();
        content_target = amis::FilePathTools::getTarget(content_href);
        content_href = amis::FilePathTools::getFileName(content_href);
        content_href += "#";
        content_href += content_target;

        const char *tmp = strdup(content_href.c_str());
        mpNavMapCache.insert(make_pair(tmp, p_node));

        p_node = p_node->next();
    }
}

void NavMap::deleteCache()
{
    LOG4CXX_DEBUG(amisNavMapLog, "deleting mpNavMapCache");

    map<const char *, NavPoint *>::const_iterator iter;
    for (iter = mpNavMapCache.begin(); iter != mpNavMapCache.end(); ++iter)
        free((void *) iter->first);
}

NavNode* NavMap::first()
{
    //the calling function will have to upcast the return value to a NavPoint
    if (mpRoot != NULL)
    {
        if (mpRoot->getNumChildren() > 0)
        {
            //reset child counter otherwise "next" function doesn't work
            mpRoot->resetChildCount();
            mpCurrent = mpRoot->getChild(0);

            NavPoint* p_tmp = (NavPoint*) mpCurrent;
            p_tmp->resetChildCount();

            return mpCurrent;
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        return NULL;
    }
}

NavNode* NavMap::next()
{
    NavPoint* p_navp;

    //the calling function will have to upcast the return value to a NavPoint
    if (mpCurrent != NULL)
    {
        p_navp = (NavPoint*) mpCurrent;
        mpCurrent = p_navp->next();
        return mpCurrent;

    }
    else
    {
        return NULL;
    }
}

NavNode* NavMap::previous()
{
    NavPoint* p_navp;

    //the calling function will have to upcast the return value to a NavPoint
    if (mpCurrent != NULL)
    {
        p_navp = (NavPoint*) mpCurrent;
        return p_navp->previous();

    }
    else
    {
        return NULL;
    }
}

NavNode* NavMap::last()
{
    int num_children;
    NavPoint* p_navp;

    if (mpRoot != NULL)
    {
        p_navp = mpRoot;
        do
        {
            num_children = p_navp->getNumChildren();
            if (num_children > 0)
            {
                p_navp = p_navp->getChild(num_children - 1);
            }
        } while (num_children > 0);

        return p_navp;

    }
    else
    {
        return NULL;
    }
}

//--------------------------------------------------
//returns whether this tree is empty or not
//--------------------------------------------------
bool NavMap::isEmpty()
{
    //test the root for NULL
    if (mpRoot == NULL)
    {
        return true;
    }
    else
    {
        return false;
    }
}

int NavMap::getNumberOfSubsections()
{
    if (mpCurrent == NULL)
    {
        mpCurrent = mpRoot;
    }

    if (mpCurrent != NULL)
    {
        return ((NavPoint*) mpCurrent)->getNumChildren();
    }
    else
    {
        return 0;
    }
}

NavNode* NavMap::goToId(std::string id)
{
    NavPoint* p_node = (NavPoint*) first();

    bool b_found = false;

    LOG4CXX_DEBUG(amisNavMapLog, "NavMap::goToId(): searching for " << id);

    while (b_found == false && p_node != NULL)
    {
        LOG4CXX_DEBUG(amisNavMapLog, "Comparing '" << p_node->getId() << " to " << id);

        if (p_node->getId().compare(id) == 0)
        {
            b_found = true;
        }
        else
        {
            p_node = p_node->next();
        }
    }

    if (b_found == true)
    {
        LOG4CXX_DEBUG(amisNavMapLog, "Found it!! '" << p_node->getId() << "'");
        mpCurrent = p_node;
        return mpCurrent;
    }
    else
    {
        LOG4CXX_DEBUG(amisNavMapLog, "Could not find '" << id << "'");
        return NULL;
    }
}
