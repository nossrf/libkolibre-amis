/*
 SMIL Engine: linear and skippable navigation of Daisy 2.02 and Daisy 3 SMIL contents

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
 * @class amis::SmilMediaGroup
 *
 * @brief SmilMediaGroup represents a group nodes to be played simultaneously
 *
 * @details
 * audio nodes, if more than one exists, are to be played in sequence.  this is a special
 * case for situations where additional audio data is prepended to a media group, such as
 * ["chapter"] + " 4.3 how to write a daisy player"
 *
 * the SmilMediaGroup is derived from the more generic amis::MediaGroup class
 *
 * @author Kolibre (www.kolibre.org)
 *
 * Contact: info@kolibre.org
 *
 */

//PROJECT INCLUDES
#include "SmilMediaGroup.h"

using namespace std;
using namespace amis;

//--------------------------------------------------
//default constructor
//--------------------------------------------------
SmilMediaGroup::SmilMediaGroup()
{
}

//--------------------------------------------------
//destructor
//--------------------------------------------------
SmilMediaGroup::~SmilMediaGroup()
{
}
//--------------------------------------------------
//set escape
//--------------------------------------------------
void SmilMediaGroup::setEscape(bool escape)
{
    mbEscape = escape;
}

//--------------------------------------------------
//set pause
/*!
 param[in] pause
 true = pause after playing the element with id=pauseId
 false = no pause associated with this media group
 */
//--------------------------------------------------
void SmilMediaGroup::setPause(bool pause)
{
    mbPause = pause;
}

//--------------------------------------------------
//set pause element id
//--------------------------------------------------
void SmilMediaGroup::setPauseId(std::string pauseId)
{
    mPauseId = pauseId;
}

//--------------------------------------------------
//set pause length
//--------------------------------------------------
void SmilMediaGroup::setPauseLength(std::string pauseLength)
{
    mPauseLength = pauseLength;
}

//--------------------------------------------------
//get escapability
//--------------------------------------------------
bool SmilMediaGroup::getEscape()
{
    return mbEscape;
}

//--------------------------------------------------
//get pause info
//--------------------------------------------------
bool SmilMediaGroup::getPause()
{
    return mbPause;
}

//--------------------------------------------------
//get pause id
//--------------------------------------------------
std::string SmilMediaGroup::getPauseId()
{
    return mPauseId;
}

//--------------------------------------------------
//get pause length
//--------------------------------------------------
std::string SmilMediaGroup::getPauseLength()
{
    return mPauseLength;
}
