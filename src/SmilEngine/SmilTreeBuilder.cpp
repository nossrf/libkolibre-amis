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

//SYSTEM INCLUDES
#include <string>
#include <iostream>
#include <cstring>
#include <algorithm>


//PROJECT INCLUDES
#include "FilePathTools.h"
#include "AmisError.h"

#include "SmilEngineConstants.h"
#include "SmilTree.h"
#include "Node.h"
#include "TimeContainerNode.h"
#include "ContentNode.h"
#include "ParNode.h"
#include "SeqNode.h"
#include "SmilTreeBuilder.h"

#include <XmlReader.h>
#include <XmlAttributes.h>
#include <XmlError.h>

#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.amis
log4cxx::LoggerPtr amisSmilTreeBuildLog(
        log4cxx::Logger::getLogger("kolibre.amis.smiltreebuilder"));

using namespace std;

//--------------------------------------------------
//Default constructor
//--------------------------------------------------
SmilTreeBuilder::SmilTreeBuilder():
        mOpenNodes(), mMetaList(), dataMutex()
{
    pthread_mutex_init (&dataMutex, NULL);
    mSmilSourceFile = "";
    mFiletype = 0;
    mDaisyVersion = 0;
    mbLinkOpen = false;
    mLinkHref = "";
    mError.setSourceModuleName(amis::module_SmilEngine);
}

//--------------------------------------------------
//Destructor
//--------------------------------------------------
SmilTreeBuilder::~SmilTreeBuilder()
{
    //empty function
}

//--------------------------------------------------
//set the Daisy version: 
/*!
 version is DAISY202 or DAISY3
 */
//--------------------------------------------------
void SmilTreeBuilder::setDaisyVersion(int daisyVersion)
{
    mDaisyVersion = daisyVersion;
}

//--------------------------------------------------
//get the Daisy version: 
/*!
 version is DAISY202 or DAISY3
 */
//--------------------------------------------------
int SmilTreeBuilder::getDaisyVersion()
{
    return mDaisyVersion;
}

//--------------------------------------------------
/*!
 Load a SMIL file and start the Xmlreader SAX2 parser.
 @param[in] pSmilTree
 points to an allocated but empty tree object
 */
//--------------------------------------------------
amis::AmisError SmilTreeBuilder::createSmilTree(SmilTree *pSmilTree,
        string filePath)
{
    //local variables
    int i;
    string tmp_string;

    //reset all variables
    mLinkHref = "";
    mSmilSourceFile = "";

    mError.setCode(amis::OK);
    mError.setMessage("");
    mError.setFilename(filePath);

    //save the file path
    mSmilSourceFile = filePath;

    mSmilSourceFile = amis::FilePathTools::clearTarget(mSmilSourceFile);

    //save a pointer to the smil tree that we will populate
    mpSmilTree = pSmilTree;

    //save the full path to this smil file
    mpSmilTree->setSmilFilePath(filePath);

    //set the default value for couldescape
    mpSmilTree->setCouldEscape(false);

    //remove anything from mOpenNodes NodeList
    pthread_mutex_lock(&dataMutex);
    while (mOpenNodes.size()>0)
    {
        delete mOpenNodes.back();
        mOpenNodes.back() = NULL;
        mOpenNodes.pop_back();
    }
    pthread_mutex_unlock(&dataMutex);
    //remove any stored metadata
    amis::MetaItem *tmp_ptr;

    while (mMetaList.size()>0)
    {
        delete mMetaList.back();
        mMetaList.pop_back();
    }
    mMetaList.clear();

    //get the local file path version of the SMIL file path
    tmp_string = amis::FilePathTools::getAsLocalFilePath(filePath);

    XmlReader parser;

    parser.setContentHandler(this);
    parser.setErrorHandler(this);

    LOG4CXX_DEBUG(amisSmilTreeBuildLog, "opening: " << tmp_string);
    if (!parser.parseXml(tmp_string.c_str()))
    {
        const XmlError *e = parser.getLastError();
        if (e)
        {
            LOG4CXX_ERROR(amisSmilTreeBuildLog,
                    "Error in SmilTreeBuilder: " << e->getMessage());
            mError.loadXmlError(*e);
        }
        else
        {
            LOG4CXX_ERROR(amisSmilTreeBuildLog,
                    "Unknown error in SmilTreeBuilder");
            mError.setCode(amis::UNDEFINED_ERROR);
        }
    }

    return mError;

} //end SmilTreeBuilder::createSmilTree function

string SmilTreeBuilder::getMetadata(string metaname)
{
    unsigned int i;
    string null_str;
    null_str.erase();

    LOG4CXX_DEBUG(amisSmilTreeBuildLog,
            "getMetadata: Looking for '" << metaname << "'");

    for (i = 0; i < mMetaList.size(); i++)
    {
        if (mMetaList[i]->mName.compare(metaname) == 0)
        {
            LOG4CXX_DEBUG(amisSmilTreeBuildLog,
                    "getMetadata: Got '" << mMetaList[i]->mContent << "'");
            return mMetaList[i]->mContent;

        }
    }

    return null_str;
}

//--------------------------------------------------
/*!
 analyze the element type and collect data to build a node
 */
//--------------------------------------------------
bool SmilTreeBuilder::startElement(const xmlChar * uri,
        const xmlChar * localname, const xmlChar * qname,
        const XmlAttributes& attributes)
{
    //local variables
    const char* element_name = NULL;
    //const char* attribute_name = NULL;
    //const char* attribute_value = NULL;
    //unsigned int i;
    string tmp_string;

    //get the element name as a string
    element_name = XmlReader::transcode(qname);

    //convert the node name to lowercase letters
    //std::transform(element_name.begin(), element_name.end(),
    //     element_name.begin(), (int(*)(int))tolower);

    //cout << "SmilTreeBuilder::startElement: '" << element_name << "'" << endl; usleep(100);

    //large "if, else if, else" statement to match the element name
    if (strcmp(element_name, TAG_REGION) == 0)
    {
        processRegion(qname, attributes);
    }

    //if this is a link node, get the href and flag linkOpen
    else if (strcmp(element_name, TAG_A) == 0)
    {
        mbLinkOpen = true;

        const xmlChar * txt_HREF = XmlReader::transcode("href");

        const char* href = NULL;
        href = XmlReader::transcode(attributes.getValue(txt_HREF));

        XmlReader::release(txt_HREF);

        if (href != NULL)
        {
            mLinkHref.assign(href);
            XmlReader::release(href);
        }

    }

    //else if this is a meta node
    else if (strcmp(element_name, TAG_META) == 0)
    {
        amis::MetaItem* meta_item = new amis::MetaItem;
        mMetaList.push_back(meta_item);

        //get attributes
        const char* current_attribute_name;

        //save the attributes list length
        int len = attributes.getLength();

        //for-loop through the attributes list until we find a match
        for (int i = 0; i < len; i++)
        {
            current_attribute_name = XmlReader::transcode(
                    attributes.getQName(i));

            //comparison if statement
            if (strcmp(current_attribute_name, ATTR_NAME) == 0)
            {
                //a match has been found, so save it and break from the loop
                const char* attribute_value;
                attribute_value = XmlReader::transcode(attributes.getValue(i));

                //convert the string to lower case
                //Damn book producers keep using uppercase and lowercase characters
                //together
                tmp_string.assign(attribute_value);
                std::transform(tmp_string.begin(), tmp_string.end(),
                        tmp_string.begin(), (int (*)(int))tolower);

                mMetaList[mMetaList.size() - 1]->mName = tmp_string.c_str();

                LOG4CXX_DEBUG( amisSmilTreeBuildLog,
                        "Metadata: mName '" << tmp_string << "'");

                XmlReader::release(attribute_value);
            }
            else if (strcmp(current_attribute_name, ATTR_CONTENT) == 0)
            {
                //a match has been found, so save it and break from the loop
                //#ifdef WIN32

                const char* attribute_value;
                attribute_value = XmlReader::transcode(attributes.getValue(i));
                mMetaList[mMetaList.size() - 1]->mContent = attribute_value;

                LOG4CXX_DEBUG( amisSmilTreeBuildLog,
                        "Metadata: mValue '" << attribute_value << "'");

                XmlReader::release(attribute_value);

            }
            XmlReader::release(current_attribute_name);
        } //end for-loop

    }

    //else if this is a seq, par, aud, txt, img
    else if (isSupportedNodeType(element_name) == true)
    {
        //call makeNode and maybe it will be able to do something with this data
        processNode(qname, attributes);
    }
    else
    {
        //empty
    }

    XmlReader::release(element_name);

    return true;

} //end SmilTreeBuilder::startElement function

//--------------------------------------------------
//process a "region" element
//--------------------------------------------------

void SmilTreeBuilder::processRegion(const xmlChar * qname,
        const XmlAttributes& attributes)
{
//if this is a region tag then add its data to the Smil Tree's content regions list
//also collect all its data as a string: the Smil Engine doesn't need to 
//manage regions except by Id, but an interface might want to have all the 
//data for a node's Id (positioning etc)

    //local variables
    int i;
    string xml_string;
    const char* attribute_name = NULL;
    const char* attribute_value = NULL;
    ContentRegionData region;
    const char* element_name = XmlReader::transcode(qname);

    //cout << "SmilTreeBuilder::processRegion: '" << element_name << "'" << endl;

    //build the XML string for this element containing all data 
    xml_string.assign("<");
    xml_string.append(element_name);

    XmlReader::release(element_name);

    //for-loop through all attributes
    for (i = 0; i < attributes.getLength(); i++)
    {
        attribute_name = XmlReader::transcode(attributes.getQName(i));
        attribute_value = XmlReader::transcode(attributes.getValue(i));

        xml_string.append(" ");
        xml_string.append(attribute_name);
        xml_string.append("=\"");
        xml_string.append(attribute_value);
        xml_string.append("\"");

        //save the ID
        if (strcmp(attribute_name, ATTR_ID) == 0)
        {
            region.mId.assign(attribute_value);
        }

        XmlReader::release(attribute_value);
        XmlReader::release(attribute_name);

    } //end for-loop through attributes

    xml_string.append("/>");

    region.mXmlString = xml_string;

    //add to region list
    mpSmilTree->addContentRegion(region);

}

//--------------------------------------------------
/*!
 This function uses the NodeBuilder to create a node, and then 
 adds the node to the right spot on the tree
 */
//--------------------------------------------------
void SmilTreeBuilder::processNode(const xmlChar * qname,
        const XmlAttributes& attributes)
{
    //local variables
    Node* p_node_data = NULL;
    SeqNode* p_root = NULL;
    string empty_str = "";
    Node* p_parent = NULL;
    int len;
    string skip_option_name;

    //try to create a node
    p_node_data = mNodeBuilder.createNode(qname, attributes);

    mNodeBuilder.setSourceSmilPath(mSmilSourceFile);
    //if we successfully created the node
    if (p_node_data != NULL)
    {
        //give the node a pointer to the Smil tree
        p_node_data->setSmilTreePtr(mpSmilTree);

        //if the tree is empty, then we need to add a root
        if (mpSmilTree->isEmpty() == true)
        {
            //if our new node is the first node in the tree and it is not of type SEQ
            if (p_node_data->getTypeOfNode() != SEQ)
            {
                //make a seq node for the root
                p_root = new SeqNode();
                p_root->setSmilTreePtr(mpSmilTree);
                p_root->setParent(NULL);
                p_root->setSkipOption(empty_str);

                //set the root of the tree equal to this root
                mpSmilTree->setRoot(p_root);

                //add the root to the open nodes list
                pthread_mutex_lock(&dataMutex);
                mOpenNodes.push_back((Node* const ) p_root);
                pthread_mutex_unlock(&dataMutex);

                //add pNodeData as a child of the root
                p_root->addChild(p_node_data);

                //if this node data is a par, add it to the open nodes list
                //seqs also go on this list, but we have already determined this is not a seq
                if (p_node_data->getTypeOfNode() == PAR)
                {
                    pthread_mutex_lock(&dataMutex);
                    mOpenNodes.push_back((Node* const ) p_node_data);
                    pthread_mutex_unlock(&dataMutex);
                }

            } //end if new node is not a seq

            //else, it is the first node in the tree but it is of type SEQ
            else
            {
                //add our new node as the root of the tree
                mpSmilTree->setRoot((SeqNode*) p_node_data);

                //store the total duration of smil file if found
                //get and save dur
                const char* dur = NULL;
                const xmlChar* txt_DUR = XmlReader::transcode("dur");
                dur = XmlReader::transcode(attributes.getValue(txt_DUR));
                if (dur)
                {
                    mpSmilTree->setSmilDuration(string(dur));
                }

                //put the new node on the open nodes list
                pthread_mutex_lock(&dataMutex);
                mOpenNodes.push_back((Node* const ) p_node_data);
                pthread_mutex_unlock(&dataMutex);
            }

        } //end if tree is empty

        //else, the tree is not empty
        else
        {
            //get the last item from the open nodes list and set it as the parent for this node
            pthread_mutex_lock(&dataMutex);
            len = mOpenNodes.size();
            p_parent = mOpenNodes[len - 1];
            pthread_mutex_unlock(&dataMutex);

            //record link data for this node
            //only content nodes may be nested beneath a link node
            if (p_node_data->getCategoryOfNode() == CONTENT
                    && mbLinkOpen == true)
            {
                amis::MediaNode* pMediaNode =
                        ((ContentNode*) p_node_data)->getMediaNode();
                pMediaNode->setHref(mLinkHref);
            }

            //add as the next child to the last item of the open nodes list	
            ((TimeContainerNode*) p_parent)->addChild(p_node_data);

            //add to the open nodes list if it's a time container (par or seq)
            if (p_node_data->getCategoryOfNode() == TIME_CONTAINER)
            {
                pthread_mutex_lock(&dataMutex);
                mOpenNodes.push_back((Node* const ) p_node_data);
                pthread_mutex_unlock(&dataMutex);
            }

        } //end 

    } //closing brace for if pNodeData != NULL
}

//--------------------------------------------------
/*!
 close this element so that no additional child nodes are added 
 to it in the Smil Tree
 */
//--------------------------------------------------
bool SmilTreeBuilder::endElement(const xmlChar * uri, const xmlChar * localname,
        const xmlChar * qname)
{
    //local variable
    const char* element_name = XmlReader::transcode(qname);

    //if this element is a seq or par, then remove the last item from the openNodes list
    //since the element is being ended, we will not want to add children to it
    if (strcmp(element_name, TAG_SEQ) == 0
            || strcmp(element_name, TAG_PAR) == 0)
    {
        pthread_mutex_lock(&dataMutex);
        mOpenNodes.pop_back();
        pthread_mutex_unlock(&dataMutex);
    }

    else if (strcmp(element_name, TAG_A) == 0)
    {
        mbLinkOpen = false;
        mLinkHref = "";
    }
    else
    {
        //empty
    }

    XmlReader::release(element_name);
    return true;
}

//--------------------------------------------------
/*!
 Signal that the Smil Tree is ready
 */
//--------------------------------------------------
bool SmilTreeBuilder::endDocument()
{
    return true;
}

//--------------------------------------------------
//(SAX Event) error
//--------------------------------------------------
bool SmilTreeBuilder::error(const XmlError& e)
{
    //ignore this, it's non-fatal
    LOG4CXX_ERROR( amisSmilTreeBuildLog, "Error: " << e.getMessage());
    return true;
}

//--------------------------------------------------
/*!
 Record a parse error with Xmlreader' message
 */
//--------------------------------------------------
bool SmilTreeBuilder::fatalError(const XmlError& e)
{
    mError.loadXmlError(e);
    return false;
}

//--------------------------------------------------
//(SAX Event) warning
//--------------------------------------------------
bool SmilTreeBuilder::warning(const XmlError& e)
{
    //ignore warnings
    LOG4CXX_WARN( amisSmilTreeBuildLog, "Warning: " << e.getMessage());
    return true;
}

//--------------------------------------------------
/*!
 tests node name for a match in our supported nodes list
 */
//--------------------------------------------------
bool SmilTreeBuilder::isSupportedNodeType(const char* name)
{
    if (strcmp(name, TAG_SEQ) == 0)
    {
        return true;
    }
    if (strcmp(name, TAG_PAR) == 0)
    {
        return true;
    }
    if (strcmp(name, TAG_AUD) == 0)
    {
        return true;
    }
    if (strcmp(name, TAG_TXT) == 0)
    {
        return true;
    }

    if (strcmp(name, TAG_IMG) == 0)
    {
        return true;
    }

    return false;
}

//--------------------------------------------------
//(SAX Event) start document
//--------------------------------------------------
bool SmilTreeBuilder::startDocument()
{
    //Empty function
    return true;
}
