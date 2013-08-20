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

#ifndef BOOKMARKSREADER_H
#define BOOKMARKSREADER_H

#include "Bookmarks.h"
#include "AmisError.h"

#include <string>
#include <vector>

#include <XmlDefaultHandler.h>
#include <XmlAttributes.h>

namespace amis
{

class AMISCOMMON_API BookmarksReader: public XmlDefaultHandler
{
public:
    BookmarksReader();
    ~BookmarksReader();

    AmisError openFile(std::string, BookmarkFile*);

    //SAX METHODS
    bool startElement(const xmlChar* const, const xmlChar* const,
            const xmlChar* const, const XmlAttributes&);
    bool endElement(const xmlChar*, const xmlChar*, const xmlChar*);
    bool characters(const xmlChar * const, const unsigned int);
    bool error(const XmlError&);
    bool fatalError(const XmlError&);
    bool warning(const XmlError&);
    /*end of sax methods*/

private:
    //!get an attribute value from the member variable mpAttributes
    std::string getAttributeValue(std::string attributeName);

    std::string mFilePath;

    AmisError mError;

    //!character data
    std::string mTempWChars;
    std::string mTempChars;

    int maxId;

    //!pointer to attributes collection for node being currently processed
    const XmlAttributes* mpAttributes;

    //!retrieve character data
    bool mb_flagGetChars;

    std::vector<std::string> mElementStack;

    //the title element
    amis::MediaGroup* mpTitle;
    //the current bookmark or hilite element
    amis::PositionMark* mpCurrentPosMark;
    //the current note element
    amis::MediaGroup* mpCurrentNote;
    //the current position data
    amis::PositionData* mpCurrentPosData;

    //the bookmarks file object model
    BookmarkFile* mpFile;
};
}

#endif
