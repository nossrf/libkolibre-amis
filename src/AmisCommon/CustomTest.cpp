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

//INCLUDES
#include "CustomTest.h"
#include <iostream>

using namespace std;

//--------------------------------------------------
//--------------------------------------------------
amis::CustomTest::CustomTest()
{
    mId = "";
    mbOverride = true;
    mbDefaultState = false;
    mBookStruct = "";
    mbCurrentState = false;
}

//--------------------------------------------------
//--------------------------------------------------
amis::CustomTest::~CustomTest()
{
}

//--------------------------------------------------
//--------------------------------------------------
void amis::CustomTest::setId(const std::string id)
{
    mId = id;
}

//--------------------------------------------------
//--------------------------------------------------
void amis::CustomTest::setOverride(bool override)
{
    mbOverride = override;
}

//--------------------------------------------------
//--------------------------------------------------
void amis::CustomTest::setDefaultState(bool defaultState)
{
    mbDefaultState = defaultState;
}

//--------------------------------------------------
//--------------------------------------------------
void amis::CustomTest::setBookStruct(const std::string bookStruct)
{
    mBookStruct = bookStruct;
}

//--------------------------------------------------
//--------------------------------------------------
void amis::CustomTest::setCurrentState(bool currentState)
{
    mbCurrentState = currentState;
}

//--------------------------------------------------
//--------------------------------------------------
const string amis::CustomTest::getId()
{
    return mId;
}

//--------------------------------------------------
//--------------------------------------------------
bool amis::CustomTest::getOverride()
{
    return mbOverride;
}

//--------------------------------------------------
//--------------------------------------------------
bool amis::CustomTest::getDefaultState()
{
    return mbDefaultState;
}

//--------------------------------------------------
//--------------------------------------------------
const string amis::CustomTest::getBookStruct()
{
    return mBookStruct;
}

//--------------------------------------------------
//--------------------------------------------------
bool amis::CustomTest::getCurrentState()
{
    return mbCurrentState;
}
