/*

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

#ifndef SMILAUDIORETRIEVE_H
#define SMILAUDIORETRIEVE_H

//SYSTEM INCLUDES
#include <string>
#include "Media.h"
#include "AmisError.h"

#include <XmlDefaultHandler.h>

class AudioElement
{
    //we're not sure how an NCC file will refer to an element
    //could be text or par 
    //so get two possible IDs for each audio clip
public:
    AudioElement();
    ~AudioElement();
    std::string mParId;
    std::string mTextId;
    bool mbIsOwned;
    amis::AudioNode* mpAudioData;
};

//! The SmilAudioRetrieve calls upon Xmlreader-SAX2 to parse a SMIL file and deliver the audio reference for a given element
class SmilAudioRetrieve: public XmlDefaultHandler
{

public:

    //LIFECYCLE
    SmilAudioRetrieve();
    ~SmilAudioRetrieve();

    //return a reference to the audio source from a smil file
    amis::AmisError getAudioReference(std::string, amis::AudioNode*);

private:
    //SAX METHODS
    bool startElement(const xmlChar* const, const xmlChar* const,
            const xmlChar* const, const XmlAttributes&);
    bool endElement(const xmlChar* const, const xmlChar* const,
            const xmlChar* const);
    bool error(const XmlError&);
    bool fatalError(const XmlError&);
    bool warning(const XmlError&);
    /*end of sax methods*/
    //amis::AudioNode* buildAudioNode(const Attributes*);
    amis::AudioNode* findId(std::string);
    void cleanVector();

    //MEMBER VARIABLES
    std::string mSmilSourceFile;
    std::vector<AudioElement*> mAudioList;
    std::string mCurrentTextId;
    std::string mCurrentParId;
    amis::AudioNode* mpCurrentAudioNode;
    bool mbFoundAudio;
    int audionodecnt;
    bool mbUsingCurrentAudioNode;

    amis::AmisError mError;
};

#endif

