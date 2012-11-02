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
 * \class amis::PageList
 *
 * \brief This class maintains a page list and provides different methods for navigating and jumping between pages
 *
 * \author Kolibre (www.kolibre.org)
 *
 * Contact: info@kolibre.org
 *
 */

#include "PageList.h"
#include "NavModel.h"
#include "FilePathTools.h"

#include <iostream>
#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.amis
log4cxx::LoggerPtr amisPageListLog(
        log4cxx::Logger::getLogger("kolibre.amis.pagelist"));

using namespace std;
using namespace amis;

/**
 * Constructor
 */
PageList::PageList()
{
    mCurrentIndex = 0;
    mpCurrent = NULL;
}

/**
 * Destructor
 */
PageList::~PageList()
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

/**
 * Add node to list
 *
 * @param pNode the node to add
 */
void PageList::addNode(PageTarget* pNode)
{
    pNode->setIndex(mpNodes.size());
    mpNodes.push_back(pNode);
}

/**
 * Get the first node in the list
 */
NavNode* PageList::first()
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

/**
 * Get the last node in the list
 */
NavNode* PageList::last()
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
/**
 * Get the next node in the list
 */
NavNode* PageList::next()
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

/**
 * Get the previous node in the list
 */
NavNode* PageList::previous()
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

/**
 * Set the given node as current node
 *
 * @param pNewCurrent node
 */
void PageList::updateCurrent(NavNode* pNewCurrent)
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

/**
 * Get the list length
 *
 * @return the length of page node list
 */
int PageList::getLength()
{
    return mpNodes.size();
}

/**
 * Count the number of pages in the given range
 *
 * @param start index of range
 * @param end index of range
 * @return the total number of pages in range
 */
int PageList::countPagesInRange(int start, int end)
{
    int page_count = 0;

    for (unsigned int i = 0; i < mpNodes.size(); i++)
    {
        if (mpNodes[i]->getPlayOrder() >= start
                && mpNodes[i]->getPlayOrder() < end)
        {
            page_count++;
        }
    }

    return page_count;
}

/**
 * Go to a page with the given label
 *
 * @param pageLabel to look for
 * @return the page node, or NULL if the page can not be found
 */

//--------------------------------------------------
PageTarget* PageList::findPage(std::string pageLabel)
{
    amis::MediaGroup* p_label;
    bool b_found = false;
    unsigned int i = 0;

    for (i = 0; i < mpNodes.size(); i++)
    {
        p_label = mpNodes[i]->getLabel();

        if (p_label->getText()->getTextString().compare(pageLabel) == 0)
        {
            b_found = true;
            break;
        }
    }

    if (b_found == true)
    {
        mCurrentIndex = i;
        mpCurrent = mpNodes[mCurrentIndex];
        return static_cast<PageTarget*>(mpNodes[mCurrentIndex]);
    }
    else
    {
        return NULL;
    }
}

/**
 * Go to a node with the given index, based on play order
 *
 * @param playOrder The index of the node in play order
 * @return The new current node, or NULL if not found
 */
NavNode* PageList::syncPlayOrder(int playOrder)
{
    bool b_found = false;

    for (unsigned int i = 0; i < mpNodes.size(); i++)
    {
        if (mpNodes[i]->getPlayOrder() == playOrder + 1)
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

/**
 * Go to a certain node based on href
 *
 * @param contentHref The wanted href
 * @return The new current node, or NULL if not found
 */
NavNode* PageList::goToContentRef(const std::string contentHref)
{

    if (mpPageListCache.size() == 0)
        createCache();

    map<const char *, int>::const_iterator iter = mpPageListCache.find(
            contentHref.c_str());
    if (iter != mpPageListCache.end())
    {
        mCurrentIndex = iter->second;
        mpCurrent = mpNodes[mCurrentIndex];
        return mpNodes[mCurrentIndex];
    }

    return NULL;

}

/**
 * Go to a certain node with the given id
 *
 * @param id The wanted id
 * @return The new current node, or NULL if not found
 */
NavNode* PageList::goToId(std::string id)
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

/**
 * Create the node cache
 */
void PageList::createCache()
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
        mpPageListCache.insert(make_pair(tmp, i));
    }
}

/**
 * Delete the node cache
 */
void PageList::deleteCache()
{
    LOG4CXX_DEBUG(amisPageListLog, "deleting mpPageListCache");

    map<const char *, int>::const_iterator iter;
    for (iter = mpPageListCache.begin(); iter != mpPageListCache.end(); ++iter)
        free((void *) iter->first);
}

/**
 * Print the page list to console
 */
void PageList::print()
{
    cerr << "Page List" << endl;
    for (unsigned int i = 0; i < mpNodes.size(); i++)
    {
        mpNodes[i]->print(0);
    }
}
