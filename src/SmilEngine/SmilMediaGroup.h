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

#ifndef SMILMEDIAGROUP_H
#define SMILMEDIAGROUP_H

//SYSTEM INCLUDES
#include <string>

//PROJECT INCLUDES
#include "Media.h"
#include "AmisCommon.h"

//! SmilMediaGroup represents a group of nodes to be played simultaneously
/*!
 audio nodes, if more than one exists, are to be played in sequence.  this is a special
 case for situations where additional audio data is prepended to a media group, such as
 ["chapter"] + " 4.3 how to write a daisy player"

 the SmilMediaGroup is derived from the more generic amis::MediaGroup class
 */

namespace amis {

class SMILENGINE_API SmilMediaGroup: public amis::MediaGroup
{
public:

    //LIFECYCLE
    //!default constructor
    SmilMediaGroup();
    //!destructor
    ~SmilMediaGroup();

    //ACCESS
    //!set escapability info
    void setEscape(bool);
    //!set a pause
    void setPause(bool);
    //!set the id of the element after which we are going to pause
    void setPauseId(std::string);
    //!set the length of the pause
    void setPauseLength(std::string);

    //!get escapability info
    bool getEscape();
    //!get pause info
    bool getPause();
    //!get the id of the element after which we are going to pause
    std::string getPauseId();
    //!get the length of the pause
    std::string getPauseLength();

private:
    //!escapability on or off (true or false)
    bool mbEscape;
    //!does this media group have a pause in it?
    bool mbPause;

    //!the id of the element after which we shall pause
    std::string mPauseId;
    //!the length of the pause
    std::string mPauseLength;

};

}  // namespace amis
#endif

