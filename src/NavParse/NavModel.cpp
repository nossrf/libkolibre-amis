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
 * @class amis::NavModel
 *
 * @brief Class holding the nav map and page list
 *
 * @author Kolibre (www.kolibre.org)
 *
 * Contact: info@kolibre.org
 *
 */

#include "NavModel.h"
#include <iostream>

using namespace std;
using namespace amis;

NavModel::NavModel()
{
    mpDocTitle = NULL;
    mpDocAuthor = NULL;

    //make a new nav map.
    //all models will have a nav map and page list
    mpNavMap = new NavMap();
    //mpNavMap->setNavModel(this);

    mpPageList = new PageList();
    //mpPageList->setNavModel(this);

    NavPoint* p_root = new NavPoint();

    //give the map a 'fake' root
    p_root->setClass("ROOT");
    p_root->setLevel(0);
    p_root->setPlayOrder(-1);
    p_root->setId("ROOT");
    p_root->setParent(NULL);

    //p_root->setNavModel(this);
    p_root->setLabel(NULL);

    mpNavMap->setRoot(p_root);

    mGlobalPlayOrder = 0;
}

NavModel::~NavModel()
{
    int i;
    int sz = mpNavLists.size();
    NavList* p_tmp_navlist;

    if (mpNavMap != NULL)
    {
        delete mpNavMap;
    }

    //delete nav lists and page list
    for (i = sz - 1; i >= 0; i--)
    {
        p_tmp_navlist = NULL;
        p_tmp_navlist = mpNavLists[i];
        mpNavLists.pop_back();
        if (p_tmp_navlist != NULL)
        {
            delete p_tmp_navlist;
        }
    }

    delete mpPageList;

    sz = mpSmilCustomTest.size();

    //delete smil custom tests
    for (i = sz - 1; i >= 0; i--)
    {
        amis::CustomTest *p_custom_test = NULL;
        p_custom_test = mpSmilCustomTest[i];
        mpNavLists.pop_back();
        if (p_custom_test != NULL)
        {
            delete p_custom_test;
        }
    }

    if (mpDocTitle != NULL)
        delete mpDocTitle;

    if (mpDocAuthor != NULL)
        delete mpDocAuthor;
}

void NavModel::syncLists(int playOrder)
{
    if (this->hasPages() == true)
    {
        this->mpPageList->syncPlayOrder(playOrder);
    }

    for (unsigned int i = 0; i < this->getNumberOfNavLists(); i++)
    {
        this->mpNavLists[i]->syncPlayOrder(playOrder);
    }
}

void NavModel::updatePlayOrder(int playOrder)
{
    mGlobalPlayOrder = playOrder;
}

int NavModel::getPlayOrder()
{
    return mGlobalPlayOrder;
}

//@bug
//last section at any level returns no pages
int NavModel::getNumberOfPagesInCurrentSection()
{
    NavPoint* p_curr = (NavPoint*) mpNavMap->current();

    if (p_curr == NULL)
    {
        return 0;
    }

    if (this->hasPages() == false)
    {
        return 0;
    }

//	NavPoint* p_next_section = p_curr->getFirstSibling();
    NavPoint* p_next_section = (NavPoint*) p_curr->next();

    int start_count = p_curr->getPlayOrder();
    int end_count = -99;
    if (p_next_section != NULL)
    {
        end_count = p_next_section->getPlayOrder();
    }

    int num_pages = mpPageList->countPagesInRange(start_count, end_count);

    return num_pages;
}

//other navigation
//put the requested node in pNode
amis::AmisError NavModel::goToSection(std::string section, NavPoint*& pNode)
{
    amis::AmisError err;
    err.setCode(amis::OK);

    vector<int> paths;
    string section_path = section;
    string tmpstr;
    int tmpint;
    string::size_type pos;

    while (section_path.size() > 0)
    {
        pos = section_path.find(".");

        if (pos != string::npos)
        {
            tmpstr = section_path.substr(0, pos);
            section_path = section_path.substr(pos + 1);
        }
        else
        {
            tmpstr = section_path;
            section_path = "";
        }

        if (tmpstr.size() > 0)
        {
            tmpint = atoi((char*) tmpstr.c_str());
            if (tmpint > 0)
            {
                //the input is 1-based but the tree is 0-based
                //so decrease the path values by one
                paths.push_back(tmpint - 1);
            }
        }
    }

    NavPoint* p_node;
    p_node = mpNavMap->getRoot();

    for (unsigned int i = 0; i < paths.size(); i++)
    {
        if (p_node->getNumChildren() > paths[i])
        {
            p_node = p_node->getChild(paths[i]);
        }
        else
        {
            p_node = NULL;
            err.setCode(amis::NOT_FOUND);
            break;
        }
    }

    pNode = p_node;

    return err;
}

amis::AmisError NavModel::goToId(std::string id, NavPoint*& pNode)
{
    amis::AmisError err;
    err.setCode(amis::OK);

    NavNode* p_temp;
    NavList* p_list;

    p_temp = NULL;

    p_temp = mpNavMap->goToId(id);

    if (p_temp == NULL)
    {
        if (this->hasPages() == true)
        {
            p_temp = this->mpPageList->goToId(id);
        }

    }

    if (p_temp == NULL)
    {
        if (this->getNumberOfNavLists() > 0)
        {
            for (unsigned int i = 0; i < this->getNumberOfNavLists(); i++)
            {
                p_list = this->getNavList(i);

                p_temp = p_list->goToId(id);

                if (p_temp != NULL)
                {
                    break;

                }
            }
        }
    }

    if (p_temp != NULL)
    {
        this->mGlobalPlayOrder = p_temp->getPlayOrder();
        pNode = (NavPoint*) p_temp;
        return err;
    }

    pNode = NULL;
    err.setCode(amis::NOT_FOUND);

    return err;
}

NavNode* NavModel::goToHref(std::string href)
{
    amis::AmisError err;
    err.setCode(amis::OK);

    NavNode* p_temp;
    NavList* p_list;

    p_temp = NULL;

    p_temp = mpNavMap->goToContentRef(href);

    if (p_temp == NULL)
    {
        if (this->hasPages() == true)
        {
            p_temp = this->mpPageList->goToContentRef(href);
        }
    }

    if (p_temp == NULL)
    {
        if (this->getNumberOfNavLists() > 0)
        {
            for (unsigned int i = 0; i < this->getNumberOfNavLists(); i++)
            {
                p_list = this->getNavList(i);

                p_temp = p_list->goToContentRef(href);

                if (p_temp != NULL)
                {
                    break;
                }
            }
        }
    }

    //return err;
    if (p_temp != NULL)
    {
        this->mGlobalPlayOrder = p_temp->getPlayOrder();
    }

    return p_temp;
}

amis::MediaGroup* NavModel::getDocAuthor()
{
    return mpDocAuthor;
}

amis::MediaGroup* NavModel::getDocTitle()
{
    return mpDocTitle;
}

void NavModel::setDocTitle(amis::MediaGroup* media)
{
    mpDocTitle = media;
}

void NavModel::setDocAuthor(amis::MediaGroup* media)
{
    mpDocAuthor = media;
}

vector<NavList*>::size_type NavModel::getNumberOfNavLists()
{
    return mpNavLists.size();
}

NavList* NavModel::getNavList(unsigned int index)
{
    if (index < mpNavLists.size())
    {
        return mpNavLists[index];
    }
    else
    {
        return NULL;
    }
}

NavList* NavModel::getNavList(std::string listname)
{
    bool b_found = false;
    unsigned int i;

    for (i = 0; i < mpNavLists.size(); i++)
    {
        if (mpNavLists[i]->getId().compare(listname) == 0)
        {
            b_found = true;
            break;
        }
    }

    if (b_found == true)
    {
        return mpNavLists[i];
    }
    else
    {
        return NULL;
    }
}

int NavModel::addNavList(std::string listname)
{
    NavList* p_temp;
    bool b_found = false;

    for (unsigned int i = 0; i < mpNavLists.size(); i++)
    {
        p_temp = mpNavLists[i];
        if (listname.compare(p_temp->getId()) == 0)
        {
            b_found = true;
            break;
        }
    }

    if (b_found == false)
    {
        NavList* p_list = new NavList();
        p_list->setId(listname);
        mpNavLists.push_back(p_list);

        return mpNavLists.size() - 1;
    }
    else
    {
        return -1;
    }
}

PageList* NavModel::getPageList()
{
    return mpPageList;
}

bool NavModel::hasPages()
{
    if (mpPageList->getLength() > 0)
    {
        return true;
    }
    else
    {
        return false;
    }

}

NavMap* NavModel::getNavMap()
{
    return mpNavMap;
}

unsigned int NavModel::getNumberOfCustomTests()
{
    return mpSmilCustomTest.size();
}

amis::CustomTest* NavModel::getCustomTest(unsigned int index)
{
    unsigned int sz = mpSmilCustomTest.size();

    if (index >= 0 && index < sz)
    {
        return mpSmilCustomTest[index];
    }
    else
    {
        return NULL;
    }
}

void NavModel::addCustomTest(amis::CustomTest* pCustTest)
{
    mpSmilCustomTest.push_back(pCustTest);
}
