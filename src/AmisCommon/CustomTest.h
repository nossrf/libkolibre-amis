/*
 AmisCommon: common system objects and utility routines

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

#ifndef CUSTOMTEST_H
#define CUSTOMTEST_H

//SYSTEM INCLUDES
#include <string>

//PROJECT INCLUDES
#include "AmisCommon.h"

namespace amis
{
//!Custom test object represents a custom test structure in a DTB
class AMISCOMMON_API CustomTest
{
public:
    //!default constructor
    CustomTest();
    //!destructor
    ~CustomTest();

    //!set the id
    void setId(const std::string);
    //!set the user override 
    void setOverride(bool);
    //!set the default state
    void setDefaultState(bool);
    //!set the book struct name
    void setBookStruct(const std::string);
    //!set the current state
    void setCurrentState(bool);

    //!get the id
    const std::string getId();
    //!get the user override
    bool getOverride();
    //!get the default state
    bool getDefaultState();
    //!get the book struct name
    const std::string getBookStruct();
    //!get the current state
    bool getCurrentState();

private:
    //!custom test id
    std::string mId;
    //!can the user override the default state? (true = encourage, false = discourage)
    bool mbOverride;
    //!default state (true = render, false = skip)
    bool mbDefaultState;
    //!book structure name
    std::string mBookStruct;
    //!current state (true = render, false = skip)
    bool mbCurrentState;

};
}
#endif
