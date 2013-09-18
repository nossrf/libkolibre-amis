/*
 SMIL Engine: linear and skippable navigation of Daisy 2.02 and Daisy 3 SMIL contents

 Copyright (C) 2004  DAISY for All Project

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

#ifndef SMILTREEBUILDER_H
#define SMILTREEBUILDER_H

//SYSTEM INCLUDES
#include <string>
#include <pthread.h>
//#include <xmlreaderc/sax2/DefaultHandler.hpp>

//PROJECT INCLUDES
#include "AmisError.h"
#include "NodeBuilder.h"
#include "MetadataSet.h"

#include <XmlDefaultHandler.h>

class SmilTree;

//! The Smil Tree Builder parses a SMIL file and builds a tree
/*!
 The input files must be valid SMIL documents. Xmlreader SAX 2 is the parsing engine. 
 The Smil Tree Builder acts as a Xmlreader Content and Error handler object.
 */
class SmilTreeBuilder: public XmlDefaultHandler
{

public:

    //LIFECYCLE
    //!default constructor
    SmilTreeBuilder();
    //!destructor
    ~SmilTreeBuilder();

    //ACCESS
    //!set the daisy version
    void setDaisyVersion(int);

    //!get daisy version
    int getDaisyVersion();

    //INQUIRY
    //!see if the node name is interesting to us
    bool isSupportedNodeType(const char*);

    //METHODS
    //!main method to create a smil tree from a filepath
    amis::AmisError createSmilTree(SmilTree*, std::string);

    //!find metadata in smil file
    std::string getMetadata(std::string);

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
    //!process a node in the smil body
    void processNode(const xmlChar* const, const XmlAttributes&);
    //!process a layout region element
    void processRegion(const xmlChar* const, const XmlAttributes&);

    //MEMBER VARIABLES

    amis::AmisError mError;

    //!node builder
    NodeBuilder mNodeBuilder;
    //!smil tree pointer
    SmilTree* mpSmilTree;

    //!list of open nodes in the tree (nodes to which children can be added)
    pthread_mutex_t dataMutex;
    std::vector<Node*> mOpenNodes;

    //!type of file
    int mFiletype;
    //!daisy version
    int mDaisyVersion;

    //!list of metaitems
    std::vector<amis::MetaItem*> mMetaList;

    //!is there a link element open?
    bool mbLinkOpen;
    //!the link href
    std::string mLinkHref;

    //!the source file path
    std::string mSmilSourceFile;

    //!build error
    amis::ErrorCode mBuildErrorFlag;
    //!build error message
    std::string mBuildErrorMsg;
};

#endif

