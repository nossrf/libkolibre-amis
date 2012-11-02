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

//tree structure 
#ifndef NAVMAP_H
#define NAVMAP_H

#include <vector>
#include <map>
#include <cstring>

#include "NavPoint.h"
#include "NavContainer.h"

namespace amis {

class NAVPARSE_API NavMap: public amis::NavContainer
{
public:
    NavMap();
    ~NavMap();

    amis::NavPoint* getRoot();

    //test print function
    void print();
    void printNode(amis::NavPoint*, int);

    bool isEmpty();

    //overrides
    amis::NavNode* next();
    amis::NavNode* first();
    amis::NavNode* previous();
    amis::NavNode* last();
    void updateCurrent(amis::NavNode*);
    amis::NavNode* syncPlayOrder(int);
    amis::NavNode* goToContentRef(std::string);
    amis::NavNode* goToId(std::string);
    int getNumberOfSubsections();

    int getMaxDepth();
    void createCache();
    void deleteCache();

    void recordNewDepth(int);
    void setRoot(amis::NavPoint*);

private:

    amis::NavPoint* mpRoot;
    int mMaxDepth;

    struct ltstr
    {
        bool operator()(const char* s1, const char* s2) const
        {
            return strcmp(s1, s2) < 0;
        }
    };

    std::map<const char *, amis::NavPoint *, ltstr> mpNavMapCache;
};

}

#endif
