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
 * @class amis::NavContainer
 *
 * @brief Base container for page and nav list
 *
 * @author Kolibre (www.kolibre.org)
 *
 * Contact: info@kolibre.org
 *
 */

#include "NavContainer.h"

using namespace amis;

NavContainer::NavContainer()
{
    mpLabel = NULL;
    mpCurrent = NULL;
}

NavContainer::~NavContainer()
{
    if (mpLabel != NULL)
    {
        delete mpLabel;
    }
}

void NavContainer::setLabel(amis::MediaGroup* pLabel)
{
    this->mpLabel = pLabel;
}

amis::MediaGroup* NavContainer::getLabel()
{
    return mpLabel;
}

void NavContainer::setNavInfo(amis::MediaGroup* pNavInfo)
{
    mpNavInfo = pNavInfo;
}

amis::MediaGroup* NavContainer::getNavInfo()
{
    return mpNavInfo;
}

void NavContainer::setId(std::string id)
{
    mId = id;
}

std::string NavContainer::getId()
{
    return mId;
}

amis::NavNode* NavContainer::current()
{
    return mpCurrent;
}

/**
 * Get the next node in the list, relative to the given play order
 *
 * @return next node, returns NULL if none is found
 */
amis::NavNode* NavContainer::nextBasedOnPlayOrder(int playOrder)
{
    int closest = 0;
    int nodeOrder;
    int nodeIdx = 0;
    bool found = false;

    for (unsigned int i = 0; i < mpNodes.size(); i++)
    {
        nodeOrder = mpNodes[i]->getPlayOrder();

        //if this node comes after and it's closer than our current closest value
        if (nodeOrder > playOrder && (!found || nodeOrder < closest))
        {
            found = true;
            nodeIdx = i;
            closest = nodeOrder;
        }
    }

    if(found){
        return mpNodes[nodeIdx];
    }


    //LOG4CXX_ERROR(amisPageListLog, "Page node with larger play order not found, returning last page in list");
    return mpNodes.back();
}

/**
 * Get the previous node in the list, relative to the given play order
 *
 * @return the previous node, returns NULL if none is found
 */
amis::NavNode* NavContainer::previousBasedOnPlayOrder(int playOrder)
{
    int closest = 0;
    int nodeOrder;
    int nodeIdx = 0;
    bool found = false;

    for (unsigned int i = 0; i < mpNodes.size(); i++)
    {
        nodeOrder = mpNodes[i]->getPlayOrder();

        //if this node comes before and it's closer than our current closest value
        if (nodeOrder < playOrder && (!found || nodeOrder > closest))
        {
            found = true;
            nodeIdx = i;
            closest = nodeOrder;
        }
    }

    if(found){
        return mpNodes[nodeIdx];
    }

    //LOG4CXX_ERROR(amisPageListLog, "Page node with smaller play order not found, returning first page in list");
    return mpNodes.front();
}

