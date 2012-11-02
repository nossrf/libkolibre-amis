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
//NodeBuilder
//node creation happens quickly (no file reading going on) so no need to mess with events
//just return a new node pointer.
#ifndef NODEBUILDER_H
#define NODEBUILDER_H

//PROJECT INCLUDES
#include "SmilEngineConstants.h"
#include "Node.h"

#include <XmlReader.h>
#include <XmlAttributes.h>
#include <XmlError.h>

//! creates a Node based on XML element data
/*!
 The XML element data comes from the SmilEngineBuilder, which uses a Xmlreader SAX
 parse to read in the file.
 The NodeBuilder is called to create an audio, image, text, seq, or par node
 */
class NodeBuilder
{
public:
    //LIFECYCLE
    //!default constructor
    NodeBuilder();
    //!destructor
    ~NodeBuilder();

    //METHODS
    //!create a new node
    Node* createNode(const xmlChar* const qname,
            const XmlAttributes& attributes);

    //ACCESS
    //!set the smil file source path
    void setSourceSmilPath(std::string);

private:

    //METHODS
    //!create an audio node
    Node* createAudioNode();
    //!create a text node
    Node* createTextNode();
    //!create an image node
    Node* createImageNode();
    //!create a par node
    Node* createParNode();
    //!create a seq node
    Node* createSeqNode();

    //MEMBER VARIABLES
    //!node type of node being currently processed
    NodeType currentNodeType;
    //!pointer to attributes collection for node being currently processed
    const XmlAttributes* mpAttributes;
    //!path to the smil file being parsed by SmilTreeBuilder
    std::string mSmilPath;
};

#endif
