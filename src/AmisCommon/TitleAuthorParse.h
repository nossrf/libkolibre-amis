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

//title author parse object
#ifndef TITLEAUTHORPARSE_H
#define TITLEAUTHORPARSE_H

//SYSTEM INCLUDES
#include <string>

//PROJECT INCLUDES
#include "AmisCommon.h"
#include "AmisError.h"
#include "Media.h"

#include <XmlDefaultHandler.h>
#include <XmlAttributes.h>

//!local defines
#define TAG_H1			"h1"
#define ATTR_CLASS		"class"
#define ATTRVAL_TITLE	"title"
#define ID_NCX			"ncx"
#define FILENAME_NCC	"ncc.html"
#define FILE_EXT_OPF	"opf"
#define FILE_EXT_NCX	"ncx"
#define TAG_DOCAUTHOR	"docAuthor"
#define TAG_DOCTITLE	"docTitle"
#define TAG_TEXT		"text"
#define TAG_AUDIO		"audio"
#define ATTR_CLIPBEGIN	"clipBegin"
#define ATTR_CLIPEND	"clipEnd"
#define ATTR_SRC		"src"
#define TAG_A			"a"

class XmlDefaultHandler;

namespace amis
{
//!Title Author Parse object: gets multimedia data for title and author
/*!
 This class uses SmilAudioExtract and OpfItemExtract to help it gather data.
 Calling procedure is responsible for object destruction (mediagroup->destroyContents
 and delete mediagroup)

 Calling procedure must also link to Xmlreader to resolve DefaultHandler definitions 
 (this object implements xmlreader SAX default handler interface)

 */

class AMISCOMMON_API TitleAuthorParse: public XmlDefaultHandler
{

public:
    //!default constructor
    TitleAuthorParse();
    //!destructor
    ~TitleAuthorParse();

    //!open a file - NCC, NCX, or OPF
    amis::AmisError openFile(std::string filepath);

    //!get the author info - DAISY 3.0 only
    amis::MediaGroup* getAuthorInfo();

    //!get the title info
    amis::MediaGroup* getTitleInfo();

    //!get the file path
    std::string getFilePath();

    //SAX METHODS
    //!xmlreader start element event
    bool startElement(const xmlChar* const, const xmlChar* const,
            const xmlChar* const, const XmlAttributes &);
    bool endElement(const xmlChar* const, const xmlChar* const,
            const xmlChar* const);

    //!xmlreader error event
    bool error(const XmlError&);
    //!xmlreader fatal error event
    bool fatalError(const XmlError&);
    //!xmlreader warning event
    bool warning(const XmlError&);
    //!xmlreader character data event
    bool characters(const xmlChar* const, const unsigned int);

private:
    //!utility function
    const char *getAttributeValue(const char *);

    //!pointer to attributes collection for node being currently processed
    const XmlAttributes* mpAttributes;

    AmisError mError;

    //!the source filepath
    std::string mFilePath;

    //!the author info
    amis::MediaGroup* mpAuthorInfo;

    //!the title info
    amis::MediaGroup* mpTitleInfo;

    //!data has been extracted
    bool mb_flagFinished;

    //!retrieve character data
    bool mb_flagGetChars;

    //!name of element to get character data for
    std::string mChardataElm;

    //!retrieve link href
    bool mb_flagGetHref;

    //!processing doc author
    bool mb_flagDocAuthor;

    //!processing doc title
    bool mb_flagDocTitle;

    //!character data
    std::string mTempChars;

    //!link href
    std::string mHref;

    //!did someone else take over mpTitleInfo?
    bool mb_flagTheyHaveTitleInfo;

    //!did someone else take over mpAuthorInfo?
    bool mb_flagTheyHaveAuthorInfo;

    //!int filetype
    enum Filetype
    {
        NCC = 1, NCX = 2, OPF = 3,
    } mFiletype;
};
}

#endif
