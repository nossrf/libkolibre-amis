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

#ifndef BOOKMARKSWRITER_H
#define BOOKMARKSWRITER_H

#include "AmisCommon.h"
#include "Bookmarks.h"
#include "Media.h"

#include <string>

#include <libxml/xmlwriter.h>

namespace amis
{

class AMISCOMMON_API BookmarksWriter
{

public:
    BookmarksWriter();
    ~BookmarksWriter();

    bool saveFile(std::string, BookmarkFile*);

private:
    int writeTitle(amis::MediaGroup*);
    int writeUid(std::string);
    int writeLastmark(amis::PositionData*);
    int writeHilite(amis::Hilite*);
    int writeBookmark(amis::Bookmark*);
    int writePositionData(amis::PositionData*);
    int writeMediaGroup(amis::MediaGroup*);
    int writeNote(amis::MediaGroup*);

    xmlTextWriterPtr xmlwriter;
    BookmarkFile* mpFile;
};

}

#endif
