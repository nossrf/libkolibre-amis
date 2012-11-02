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

#ifndef NCXFILEREADER_H
#define NCXFILEREADER_H

#include <string>

#include "AmisError.h"

#include "NavFileReader.h"
#include "SmilAudioRetrieve.h"

class NcxFileReader: public NavFileReader
{
public:
    NcxFileReader();
    ~NcxFileReader();

    void init();

    //overrides
    amis::AmisError open(std::string, amis::NavModel*);

    //!xmlreader start element event
    bool startElement(const xmlChar* const, const xmlChar* const,
            const xmlChar* const, const XmlAttributes&);
    //!xmlreader end element event
    bool endElement(const xmlChar* const, const xmlChar* const,
            const xmlChar* const);
    bool characters(const xmlChar* const, const unsigned int);

private:

    bool mbFlag_GetChars;

    //0 = nav map; 1 = nav list; 2 = page list
    int mListType;

    SmilAudioRetrieve* mpSmilAudio;

    std::string mTempChars;

    amis::NavNode* mpCurrentNode;
    amis::NavContainer* mpCurrentNavContainer;

    amis::MediaGroup* mpCurrentMediaGroup;
};

#endif
