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

#ifndef NAVMODEL_H
#define NAVMODEL_H

#ifdef _MSC_VER
#pragma warning(disable : 4251)
#endif

#include "NavList.h"
#include "PageList.h"
#include "NavPoint.h"

#include "Media.h"
#include "NavNode.h"
#include "CustomTest.h"
#include "AmisError.h"
#include "NavMap.h"

namespace amis {

class NAVPARSE_API NavModel
{
public:
    NavModel();
    ~NavModel();

    int getNumberOfPagesInCurrentSection();

    //other navigation
    amis::AmisError goToSection(std::string, amis::NavPoint*&);
    amis::AmisError goToId(std::string, amis::NavPoint*&);
    amis::NavNode* goToHref(std::string);

    amis::MediaGroup* getDocAuthor();
    amis::MediaGroup* getDocTitle();
    void setDocAuthor(amis::MediaGroup*);
    void setDocTitle(amis::MediaGroup*);

    std::vector<amis::NavList*>::size_type getNumberOfNavLists();
    amis::NavList* getNavList(unsigned int);
    amis::NavList* getNavList(std::string);
    int addNavList(std::string);

    //sync nav and page lists based on play order
    void syncLists(int);

    bool hasPages();

    amis::PageList* getPageList();

    NavMap* getNavMap();

    //needs function: get metadata

    unsigned int getNumberOfCustomTests();

    amis::CustomTest* getCustomTest(unsigned int);
    void addCustomTest(amis::CustomTest*);

    void updatePlayOrder(int);
    int getPlayOrder();

private:
    NavMap* mpNavMap;
    amis::PageList* mpPageList;
    std::vector<amis::NavList*> mpNavLists;

    amis::MediaGroup* mpDocTitle;
    amis::MediaGroup* mpDocAuthor;

    //needs variable: vector <metadata> all_meta //metadata object should go here

    std::vector<amis::CustomTest*> mpSmilCustomTest;

    int mGlobalPlayOrder;
};

}
#endif
