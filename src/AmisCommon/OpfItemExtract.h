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

#ifndef OPFITEMEXTRACT_H
#define OPFITEMEXTRACT_H

//SYSTEM INCLUDES
#include <string>

//PROJECT INCLUDES
#include "AmisCommon.h"
#include "AmisError.h"

#include <XmlDefaultHandler.h>
#include <XmlAttributes.h>

//!local defines
#define ATTR_ID "id"
#define TAG_ITEM "item"
#define ATTR_HREF "href"

namespace amis
{
enum ModeOfOperation
{
    GET_BY_ID, GET_BY_MEDIATYPE
};

//! The OpfItemExtract parses an file to get its item href for a particular id
/*!
 this class is based on the SmilAudioRetrieve Nav utility class
 but differs because that class builds a list of all audio references, 
 under the assumption that it may have to pull several references from the same 
 file.  in that scenario, this saves parsing time.  but here we just want one
 reference so building that list is unneccesary*/
class AMISCOMMON_API OpfItemExtract: public XmlDefaultHandler
{

public:

    //LIFECYCLE
    OpfItemExtract();
    ~OpfItemExtract();

    //!extract a single audio element from a par or text id
    std::string getItemHref(std::string filepath, std::string id);

    //!extract all the items with given media type
    unsigned int getByMediaType(std::string filepath, std::string mediaType);
    std::string getItem(unsigned int);

    //SAX METHODS
    bool startElement(const xmlChar* const, const xmlChar* const,
            const xmlChar* const, const XmlAttributes&);
    bool error(const XmlError&);
    bool fatalError(const XmlError&);
    bool warning(const XmlError&);
    /*end of sax methods*/

private:

    AmisError openFile(std::string);

    //!get an attribute value from the member variable mpAttributes
    std::string getAttributeValue(std::string attributeName);

    AmisError mError;

    //!smil source path
    std::string mFilepath;
    //!search for this id/media-type/whatever
    std::string mSearchFor;
    //!pointer to attributes collection for node being currently processed
    const XmlAttributes* mpAttributes;

    //!if the search is done and the data has been collected
    bool mb_flagFinished;

    //!href from search
    std::string mHref;

    //!what mode are we operating in?
    ModeOfOperation mMode;

    std::vector<std::string> mResults;
};

}
#endif

