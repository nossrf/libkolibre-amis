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

#ifndef NAVLIST_H
#define NAVLIST_H

#ifdef _MSC_VER
#pragma warning(disable : 4251)
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4251)
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4251)
#endif

#include <map>
#include <cstring>
#include "NavContainer.h"


namespace amis {

class NAVPARSE_API NavList: public amis::NavContainer
{
public:
    NavList();
    ~NavList();
    void addNode(NavNode*);
    int getLength();

    //overrides
    amis::NavNode* next();
    amis::NavNode* previous();
    amis::NavNode* first();
    amis::NavNode* last();

    amis::NavNode* syncPlayOrder(int);
    amis::NavNode* goToContentRef(std::string);
    amis::NavNode* goToId(std::string);
    void updateCurrent(amis::NavNode*);

    void createCache();
    void deleteCache();

    void print();

private:
    unsigned int mCurrentIndex;

    struct ltstr
    {
        bool operator()(const char* s1, const char* s2) const
        {
            return std::strcmp(s1, s2) < 0;
        }
    };

    std::map<const char *, int, ltstr> mpNavListCache;
};

}
#endif
