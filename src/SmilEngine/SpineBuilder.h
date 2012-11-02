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

#ifndef SPINEBUILDER_H
#define SPINEBUILDER_H

//SYSTEM INCLUDES
#include <vector>
#include <string>

//PROJECT INCLUDES
#include "AmisCommon.h"
#include "AmisError.h"

#include "Spine.h"

#include <XmlDefaultHandler.h>

//! local struct for storing a filepath and id together
struct ManifestItem
{
	std::string mFileHref;
	std::string mId;
};

//! SpineBuilder parses a ncc, opf, or master.smil and creates an in-order SMIL spine

/*!
 The Spine Builder calls upon Xmlreader-SAX2 to parse a *.OPF, NCC.HTML, or MASTER.SMIL
 file.  (case insensitive file names are handled okay).The spine list is built as 
 parser events are fired.  The Spine Builder invokes Xmlreader and acts as a 
 Xmlreader Content and Error handler
 */
class SpineBuilder: public XmlDefaultHandler
{

public:
    //LIFECYCLE
    //!default constructor
    SpineBuilder();
    //!destructor
    ~SpineBuilder();

    //METHODS
    //!main method to create a spine
    amis::AmisError createSpine(Spine*, std::string);

    //SAX METHODS
    //!xmlreader start element event
    bool startElement(const xmlChar*, const xmlChar*, const xmlChar*,
            const XmlAttributes&);
    //!xmlreader end element event
    bool endElement(const xmlChar*, const xmlChar*, const xmlChar*);
    //!xmlreader start document event
    bool startDocument();
    //!xmlreader end document event
    bool endDocument();
    //!xmlreader error event
    bool error(const XmlError&);
    //!xmlreader fatal error event
    bool fatalError(const XmlError&);
    //!xmlreader warning event
    bool warning(const XmlError&);

private:
    //METHODS
    //!check the file name before starting the parse
    amis::ErrorCode doPreBuildCheck(std::string);
    //!sort the spine of an OPF
    void sortOpfSpine();

    //MEMBER VARIABLES
    //!pointer to spine object
    Spine* mpSpine;

    amis::AmisError mError;

    //!type of file being processed
    int mFiletype;

    //!are we processing an opf manifest element?
    bool mbProcessingOpfManifest;
    //!are we processing an opf spine element?
    bool mbProcessingOpfSpine;

    //!list of manifest items
    std::vector<ManifestItem> mOpfManifest;
    //!list of spine items
    std::vector<std::string> mOpfSpine;

    //!spine source file
    std::string mSpineSourceFile;
};

#endif
