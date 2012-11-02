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
 * @class amis::NavList
 *
 * @brief A list of navigation points
 *
 * @author Kolibre (www.kolibre.org)
 *
 * Contact: info@kolibre.org
 *
 */

#include "NavList.h"
#include "NavModel.h"
#include "FilePathTools.h"

#include <iostream>
#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.amis
log4cxx::LoggerPtr amisNavListLog(
        log4cxx::Logger::getLogger("kolibre.amis.navlist"));

using namespace std;
using namespace amis;

//--------------------------------------------------
//constructor
//--------------------------------------------------
NavList::NavList()
{
    mCurrentIndex = 0;
    mpCurrent = NULL;

}

//--------------------------------------------------
//destructor
//--------------------------------------------------
NavList::~NavList()
{
    deleteCache();

    int sz = mpNodes.size();
    NavNode* tmp_node;

    for (int i = sz - 1; i >= 0; i--)
    {
        tmp_node = mpNodes[i];
        mpNodes.pop_back();
        delete tmp_node;
    }

}

//--------------------------------------------------
//add node to list
//--------------------------------------------------
void NavList::addNode(NavNode* pNode)
{

    //pNode->setIndex(mpNodes.size());

    //pNode->setNavList(this);

    mpNodes.push_back(pNode);
}

//--------------------------------------------------
//get the first node in the list
//--------------------------------------------------
NavNode* NavList::first()
{
    mCurrentIndex = 0;
    if (mpNodes.size() > mCurrentIndex)
    {
        mpCurrent = mpNodes[mCurrentIndex];
        return mpNodes[mCurrentIndex];
    }
    else
    {
        return NULL;
    }
}
//--------------------------------------------------
//get the last node in the list
//--------------------------------------------------
NavNode* NavList::last()
{
    if (mpNodes.size() > 0)
    {
        mCurrentIndex = mpNodes.size() - 1;
        mpCurrent = mpNodes[mCurrentIndex];
        return mpNodes[mCurrentIndex];
    }
    else
    {
        return NULL;
    }

}

//--------------------------------------------------
//get the next node in the list
//--------------------------------------------------
NavNode* NavList::next()
{

    if (mpNodes.size() > mCurrentIndex + 1)
    {
        mCurrentIndex++;
        mpCurrent = mpNodes[mCurrentIndex];
        return mpNodes[mCurrentIndex];
    }
    else
    {
        return NULL;
    }
}

//--------------------------------------------------
//get the previous node in the list
//--------------------------------------------------
NavNode* NavList::previous()
{

    if (mCurrentIndex > 0 && mCurrentIndex < mpNodes.size())
    {
        mCurrentIndex--;
        mpCurrent = mpNodes[mCurrentIndex];
        return mpNodes[mCurrentIndex];
    }
    else
    {
        return NULL;
    }
}

void NavList::updateCurrent(NavNode* pNewCurrent)
{
    NavNode* p_temp;

    //find mpCurrent in the list
    for (unsigned int i = 0; i < mpNodes.size(); i++)
    {
        p_temp = mpNodes[i];

        if (p_temp == pNewCurrent)
        {
            mpCurrent = pNewCurrent;
            mCurrentIndex = i;
            break;
        }
    }
}

//--------------------------------------------------
//get the list length
//--------------------------------------------------
int NavList::getLength()
{
    return mpNodes.size();
}

//--------------------------------------------------
//go to a certain node, based on play order
//--------------------------------------------------
NavNode* NavList::syncPlayOrder(int playOrder)
{
    bool b_found = false;

    for (unsigned int i = 0; i < mpNodes.size(); i++)
    {
        if (mpNodes[i]->getPlayOrder() == playOrder)
        {
            mCurrentIndex = i;
            b_found = true;
            break;
        }
    }

    if (b_found == true)
    {
        mpCurrent = mpNodes[mCurrentIndex];
        return mpNodes[mCurrentIndex];
    }
    else
    {
        return NULL;
    }
}

NavNode* NavList::goToContentRef(std::string contentHref)
{
    if (mpNavListCache.size() == 0)
        createCache();

    map<const char *, int>::const_iterator iter = mpNavListCache.find(
            contentHref.c_str());
    if (iter != mpNavListCache.end())
    {
        //cout << "Got node from map " << iter->first << " at index " << iter->second << endl;
        mCurrentIndex = iter->second;
        mpCurrent = mpNodes[mCurrentIndex];
        return mpNodes[mCurrentIndex];
    }

    return NULL;
}

void NavList::createCache()
{
    string content_href;
    string content_target;

    for (unsigned int i = 0; i < mpNodes.size(); i++)
    {
        content_href = mpNodes[i]->getContent();
        content_target = amis::FilePathTools::getTarget(content_href);
        content_href = amis::FilePathTools::getFileName(content_href);
        content_href += "#";
        content_href += content_target;

        const char *tmp = strdup(content_href.c_str());
        mpNavListCache.insert(make_pair(tmp, i));
    }
}

void NavList::deleteCache()
{
    LOG4CXX_DEBUG(amisNavListLog, "deleting mpNavListCache");

    map<const char *, int>::const_iterator iter;
    for (iter = mpNavListCache.begin(); iter != mpNavListCache.end(); ++iter)
        free((void *) iter->first);
}

NavNode* NavList::goToId(std::string id)
{
    bool b_found = false;

    for (unsigned int i = 0; i < mpNodes.size(); i++)
    {
        if (mpNodes[i]->getId().compare(id) == 0)
        {
            mCurrentIndex = i;
            b_found = true;
            break;
        }
    }

    if (b_found == true)
    {
        mpCurrent = mpNodes[mCurrentIndex];
        return mpNodes[mCurrentIndex];
    }
    else
    {
        return NULL;
    }
}

void NavList::print()
{
    cerr << "Nav List: " << this->getId() << endl;
    for (unsigned int i = 0; i < mpNodes.size(); i++)
    {
        mpNodes[i]->print(0);

    }
}
