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

#ifndef METADATASET_H
#define METADATASET_H

//SYSTEM INCLUDES
#include <string>
#include <vector>

//PROJECT INCLUDES
#include "AmisCommon.h"
#include "AmisError.h"

#include <XmlDefaultHandler.h>
#include <XmlAttributes.h>

//!element and attribute name defines
#define ATTR_NAME		"name"
#define ATTR_CONTENT	"content"
#define TAG_DC			"dc:"
#define TAG_META		"meta"

#define FILENAME_NCC	"ncc.html"
#define FILE_EXT_OPF	"opf"
#define FILE_EXT_NCX	"ncx"

namespace amis
{
//!meta item struct to group name and content for a metadata item
class MetaItem
{
public:
    MetaItem();
    ~MetaItem();
    std::string mName;
    std::string mContent;
};

//!MetadataSet handles the parsing of a file and building of a set of metadata
class MetadataSet: public XmlDefaultHandler
{
public:
    //!default constructor
    MetadataSet();
    //!destructor
    ~MetadataSet();

    //!open a book file
    amis::AmisError openBookFile(std::string);
    //!retrieve metadata specified by name in the parameter
    std::string getMetadata(std::string);

    //!retrieve an md5 checksum of all the metadata content
    std::string getChecksum();

//SAX METHODS
    //!xmlreader start element event
    bool startElement(const xmlChar* const, const xmlChar* const,
            const xmlChar* const, const XmlAttributes&);
    //!xmlreader error event
    bool error(const XmlError&);
    //!xmlreader fatal error event
    bool fatalError(const XmlError&);
    //!xmlreader warning event
    bool warning(const XmlError&);
    //!xmlreader character data event
    bool characters(const xmlChar * const, const unsigned int);

private:
    //!the source filepath
    std::string mFilepath;
    //!clear the metaitem vector
    void clearVector();
    //!list of metaitems
    std::vector<amis::MetaItem*> mMetaList;
    //!flag if we are getting the chardata or not
    bool b_getChars;
    //!chardata accumulator
    std::string mTempChars;

    AmisError mError;

    //!int filetype
    enum Filetype
    {
        NCC = 1, NCX = 2, OPF = 3,
    } mFiletype;

};
}
#endif

