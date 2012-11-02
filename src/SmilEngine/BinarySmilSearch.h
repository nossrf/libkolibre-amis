/*
 Copyright (C) 2012 Kolibre
 
 This file is part of Kolibre-amis.
 
 Kolibre-amis is free software: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation, either version 2.1 of the License, or
 (at your option) any later version.
 
 Kolibre-amis is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public License
 along with Kolibre-amis.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef _BINARYSMILSEARCH_H_
#define _BINARYSMILSEARCH_H_

#include <string>

// "00:00:00" -> unsigned int
unsigned int stringToSeconds(std::string timeString);

#include "SmilTreeBuilder.h"
#include "SmilTree.h"

class BinarySmilSearch
{
public:
    enum searchDirection
    {
        DOWN, UP
    };

    SmilTreeBuilder* begin();
    SmilTreeBuilder* next(searchDirection);

    bool currentSmilIsBeyond(unsigned int seconds);
    bool currentSmilContains(unsigned int seconds);

    std::string getCurrentSmilPath();
    SmilTree* getCurrentSmilTree();
private:
    SmilTreeBuilder* buildTree(int id);

    int upperSmilIdx;
    int currentSmilIdx;
    int lowerSmilIdx;

    SmilTreeBuilder currentTreeBuilder;
    SmilTree currentSmilTree;
    std::string currentSmilPath; // Extract from here if necessary.
};

#endif
