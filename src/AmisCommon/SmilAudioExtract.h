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

#ifndef SMILAUDIOEXTRACT_H
#define SMILAUDIOEXTRACT_H

//SYSTEM INCLUDES
#include <string>

//PROJECT INCLUDES
#include "AmisCommon.h"
#include "AmisError.h"
#include "Media.h"

#include <XmlDefaultHandler.h>
#include <XmlAttributes.h>

//!local defines
#define SMIL_ATTR_CLIPBEGIN	"clip-begin"
#define SMIL_ATTR_CLIPEND	"clip-end"
#define SMIL_ATTR_SRC		"src"
#define SMIL_ATTR_ID		"id"
#define SMIL_TAG_TEXT		"text"
#define SMIL_TAG_PAR		"par"
#define SMIL_TAG_AUDIO		"audio"

namespace amis
{
//! The SmilAudioExtract parses a SMIL file to get audio data for a single element
/*!
 this class is based on the SmilAudioRetrieve Nav utility class
 but differs because that class builds a list of all audio references, 
 under the assumption that it may have to pull several references from the same 
 file.  in that scenario, this saves parsing time.  but here we just want one
 reference so building that list is unneccesary*/
class AMISCOMMON_API SmilAudioExtract: public XmlDefaultHandler
{

public:

    //LIFECYCLE
    SmilAudioExtract();
    ~SmilAudioExtract();

    //!extract a single audio element from a par or text id
    AmisError getAudioAtId(std::string filepath, amis::AudioNode**);

    //SAX METHODS
    bool startElement(const xmlChar* const, const xmlChar* const,
            const xmlChar* const, const XmlAttributes& attributes);
    bool endElement(const xmlChar* const, const xmlChar* const,
            const xmlChar* const);
    bool error(const XmlError&);
    bool fatalError(const XmlError&);
    bool warning(const XmlError&);
    /*end of sax methods*/

private:

    //!get an attribute value from the member variable mpAttributes
    std::string getAttributeValue(std::string attributeName);

    //!smil source path
    std::string mFilePath;
    //!audio information
    amis::AudioNode* mpAudioInfo;
    //!target id
    std::string mId;
    //!pointer to attributes collection for node being currently processed
    const XmlAttributes* mpAttributes;

    AmisError mError;

    //!flag if we are in a par element
    bool mb_flagInPar;
    //!if we found the id (on a text or par)
    bool mb_flagFoundId;
    //!if the search is done and the data has been collected
    bool mb_flagFinished;
};
}

#endif

