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

#ifndef NAVNODE_H
#define NAVNODE_H

// DLL Stuff
#ifdef WIN32
#ifdef AMIS_DLL
#define NAVPARSE_API __declspec(dllexport)
#else
#define NAVPARSE_API __declspec(dllimport)
#endif
#else 
#define NAVPARSE_API
#endif

#include "Media.h"

namespace amis {

class NAVPARSE_API NavNode
{
public:
    enum TypeOfNode
    {
        NAV_POINT = 0, NAV_TARGET = 1, PAGE_TARGET = 2
    };

    NavNode();
    virtual ~NavNode() = 0;

    TypeOfNode getTypeOfNode();

    MediaGroup* getLabel();
    std::string getContent();
    std::string getId();
    int getPlayOrder();
    std::string getClass();
    void print(int);
    void setPlayOrder(int);
    void setLabel(amis::MediaGroup*);
    void setContent(std::string);
    void setId(std::string);
    void setClass(std::string);

protected:
    TypeOfNode mTypeOfNode;

private:
    amis::MediaGroup* mpLabel;
    std::string mContent;
    std::string mId;
    int mPlayOrder;
    std::string mClass;
};

}  // namespace amis

#endif
