/*
 NavParse: Navigation model for DAISY NCC and NCX

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

#ifndef NAVFILEREADER_H
#define NAVFILEREADER_H

#include <string>

#include "AmisError.h"
#include "NavModel.h"
#include "PageTarget.h"
#include <XmlDefaultHandler.h>


class NavFileReader: public XmlDefaultHandler
{

public:
    NavFileReader();
    virtual ~NavFileReader() = 0;

    virtual amis::AmisError open(std::string, amis::NavModel*);

    //SAX METHODS

    //virtual sax methods
    //!xmlreader start element event
    virtual bool startElement(const xmlChar* const, const xmlChar* const,
            const xmlChar* const, const XmlAttributes&) = 0;
    //!xmlreader end element event
    virtual bool endElement(const xmlChar* const, const xmlChar* const,
            const xmlChar* const) = 0;

    //!xmlreader character data event
    virtual bool characters(const xmlChar* const, const unsigned int) = 0;

    //not virtual sax methods

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

protected:
    void addCustomTest(std::string, bool, bool, std::string);

    amis::NavPoint* mpCurrentNavPoint;
    amis::NavTarget* mpCurrentNavTarget;
    amis::PageTarget* mpCurrentPageTarget;

    std::vector<amis::NavPoint*> mOpenNodes;

    amis::NavModel* mpNavModel;

    std::string mFilePath;

    amis::AmisError mError;

};

#endif
