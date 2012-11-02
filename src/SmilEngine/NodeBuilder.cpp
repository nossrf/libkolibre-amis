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

//PROJECT INCLUDES
#include "FilePathTools.h"
#include "Media.h"

#include "SmilEngineConstants.h"
#include "Node.h"
#include "TimeContainerNode.h"
#include "ContentNode.h"
#include "ParNode.h"
#include "SeqNode.h"
#include "NodeBuilder.h"

using namespace std;

//--------------------------------------------------
//constructor does nothing special
//--------------------------------------------------
NodeBuilder::NodeBuilder()
{
    //Empty
}

//--------------------------------------------------
//destructor does nothing special
//--------------------------------------------------
NodeBuilder::~NodeBuilder()
{
    //Empty
}

//--------------------------------------------------
//set the file path for the file which contains the node source
//--------------------------------------------------
void NodeBuilder::setSourceSmilPath(string smilFilePath)
{
    mSmilPath = smilFilePath;
}

//--------------------------------------------------
/*!
 Create a node from XML element data.

 Return NULL if the element is not one of six types, 
 otherwise return a pointer to the newly created node
 */
//--------------------------------------------------
Node* NodeBuilder::createNode(const xmlChar* const qname,
        const XmlAttributes& attributes)
{
    //return pNewNode from this function
    Node* p_new_node = NULL;

    //Smil element name
    const char* element_name;

    //get the element name as a native string type
    element_name = XmlReader::transcode(qname);

    //Save a pointer to the attributes
    mpAttributes = &attributes;

    //long "if, else if, else" block to create nodes based on their element names

    //case: "seq"
    if (strcmp(element_name, TAG_SEQ) == 0)
    {
        p_new_node = createSeqNode();
    }

    //case "par"
    else if (strcmp(element_name, TAG_PAR) == 0)
    {
        p_new_node = createParNode();
    }

    //case "audio"
    else if (strcmp(element_name, TAG_AUD) == 0)
    {
        p_new_node = createAudioNode();
    }

    //case "text"
    else if (strcmp(element_name, TAG_TXT) == 0)
    {
        p_new_node = createTextNode();
    }

    //case "img"
    else if (strcmp(element_name, TAG_IMG) == 0)
    {
        p_new_node = createImageNode();
    }

    //else, we're not interested in this element
    else
    {
        p_new_node = NULL;
    }
    //end of long "if, else if, else" block to determine node type

    XmlReader::release(element_name);

    //return a pointer to the newly created node
    return p_new_node;
}

//--------------------------------------------------
//	create a seq node
//--------------------------------------------------
Node* NodeBuilder::createSeqNode()
{
    //temporarily save the attribute value in this variable
    string attribute_value;

    //pointer to a new seq node
    SeqNode* p_seq = new SeqNode();

    //get and save the Id
    const char* id = NULL;
    const xmlChar* txt_ID = XmlReader::transcode("id");
    id = XmlReader::transcode(mpAttributes->getValue(txt_ID));
    XmlReader::release(txt_ID);

    if (id != NULL)
    {

        string str;
        str.assign(id);

        XmlReader::release(id);
        p_seq->setElementId(str);
    }

    //does this node have a customTest attribute?
    //customTest is how Daisy 3 specifies skippable structures
    const char* custom_test = NULL;

    const xmlChar* txt_CUSTOMTEST = XmlReader::transcode("customTest");

    custom_test = XmlReader::transcode(mpAttributes->getValue(txt_CUSTOMTEST));

    XmlReader::release(txt_CUSTOMTEST);

    //set whatever the customTest value is as the skipOption
    //if the attributeValue came back as an empty string, then we assume this node
    //does not represent a skippable structure
    if (custom_test != NULL)
    {

        string str;
        str.assign(custom_test);

        XmlReader::release(custom_test);
        p_seq->setSkipOption(str);
    }

    //return the newly created Seq node
    return (Node*) p_seq;
}

//--------------------------------------------------
//	create a par node
//--------------------------------------------------
Node* NodeBuilder::createParNode()
{
    //temporarily save the attribute value in this variable
    string attribute_value;

    //pointer to a new par node
    ParNode* p_par = new ParNode();

    //get and save the Id
    const char* id = NULL;

    const xmlChar* txt_ID = XmlReader::transcode("id");
    id = XmlReader::transcode(mpAttributes->getValue(txt_ID));
    XmlReader::release(txt_ID);

    if (id != NULL)
    {
        string str;
        str.assign(id);

        XmlReader::release(id);
        p_par->setElementId(str);
    }

    //does this node have a system-required attribute?
    //system-required is how Daisy 2.02 specifies skippable structures
    const char* system_required = NULL;

    const xmlChar* txt_SYSREQUIRED = XmlReader::transcode("system-required");
    system_required = XmlReader::transcode(
            mpAttributes->getValue(txt_SYSREQUIRED));
    XmlReader::release(txt_SYSREQUIRED);

    if (system_required != NULL)
    {
        string attribute_value;
        attribute_value.assign(system_required);
        XmlReader::release(system_required);

        //change this value a bit so that sidebar-on = sidebar
        //daisy 2.02 values in the NCC will appear as, for ex, "sidebar"
        //in the SMIL, they appear as "sidebar-on"
        //in AMIS, transform this "sidebar-on" statement to a customTest class
        //with id=sidebar, defaultState = true

        //look at the last three characters
        //if they are "-on", remove them from the string
        if (attribute_value.length() > 3
                && attribute_value.substr(attribute_value.length() - 3)
                        == "-on")
        {
            attribute_value = attribute_value.substr(0,
                    attribute_value.length() - 3);
        }

        //set the system-required attribute value as this node's skip option
        p_par->setSkipOption(attribute_value);
    }

    //else did not find system-required attribute
    else
    {
        //does this node have a customTest attribute?
        //customTest is how Daisy 3 specifies skippable structures
        const char* custom_test = NULL;

        const xmlChar* txt_CUSTOMTEST = XmlReader::transcode("customTest");
        custom_test = XmlReader::transcode(
                mpAttributes->getValue(txt_CUSTOMTEST));
        XmlReader::release(txt_CUSTOMTEST);

        if (custom_test != NULL)
        {

            string str;
            str.assign(custom_test);

            XmlReader::release(custom_test);

            //set whatever the customTest value is as the skipOption
            //if the attributeValue came back as an empty string, then we assume this node
            //does not represent a skippable structure
            p_par->setSkipOption(str);
        }
    }

    //return the newly created Par node
    return (Node*) p_par;
}

//--------------------------------------------------
//create an audio node
//--------------------------------------------------
Node* NodeBuilder::createAudioNode()
{
    //attribute value temporarily stored here
    string attribute_value;

    //create a new audio node
    ContentNode* p_audio = new ContentNode();
    p_audio->createNewAudio();

    amis::AudioNode* p_audioMedia = (amis::AudioNode*) p_audio->getMediaNode();

    //get and save the Id attribute
    const char* id = NULL;

    const xmlChar* txt_ID = XmlReader::transcode("id");

    id = XmlReader::transcode(mpAttributes->getValue(txt_ID));

    XmlReader::release(txt_ID);

    if (id != NULL)
    {

        string str;
        str.assign(id);

        XmlReader::release(id);
        p_audioMedia->setId(str);
        p_audio->setElementId(str);
    }

    //get and save the media type attribute

    const xmlChar* txt_TYPE = XmlReader::transcode("type");

    const char* type = XmlReader::transcode(mpAttributes->getValue(txt_TYPE));

    XmlReader::release(txt_TYPE);

    if (type != NULL)
    {

        string str;
        str.assign(type);

        XmlReader::release(type);
        p_audioMedia->setMediaType(str);
    }

    //get and save the content region
    const
    char* region = NULL;

    const xmlChar* txt_REGION = XmlReader::transcode("region");
    region = XmlReader::transcode(mpAttributes->getValue(txt_REGION));
    XmlReader::release(txt_REGION);

    if (region != NULL)
    {
        string str;
        str.assign(region);
        XmlReader::release(region);
        p_audioMedia->setRegionId(str);

    }

    //get and save the content src
    const char* src = NULL;

    const xmlChar* txt_SRC = XmlReader::transcode("src");

    src = XmlReader::transcode(mpAttributes->getValue(txt_SRC));

    XmlReader::release(txt_SRC);

    if (src != NULL)
    {
        string str;
        str.assign(src);
        str = amis::FilePathTools::goRelativePath(mSmilPath, str);

        XmlReader::release(src);

        p_audioMedia->setSrc(str);
    }

    //get the clip begin attribute
    //it could be in two forms: clip-begin or clipBegin
    const char* clipbegin = NULL;

    const xmlChar * txt_CLIPBEGIN = XmlReader::transcode("clipBegin");
    clipbegin = XmlReader::transcode(mpAttributes->getValue(txt_CLIPBEGIN));
    XmlReader::release(txt_CLIPBEGIN);

    if (clipbegin == NULL)
    {
        const xmlChar * txt_CLIP_BEGIN = XmlReader::transcode("clip-begin");
        clipbegin = XmlReader::transcode(
                mpAttributes->getValue(txt_CLIP_BEGIN));
        XmlReader::release(txt_CLIP_BEGIN);
    }

    if (clipbegin != NULL)
    {

        string str;
        str.assign(clipbegin);
        XmlReader::release(clipbegin);

        //assign the media node's clipbegin value
        p_audioMedia->setClipBegin(str);
    }

    //get the clip begin attribute
    //it could be in two forms: clip-begin or clipBegin

    const char* clipend = NULL;
    const xmlChar * txt_CLIPEND = XmlReader::transcode("clipEnd");
    clipend = XmlReader::transcode(mpAttributes->getValue(txt_CLIPEND));
    XmlReader::release(txt_CLIPEND);

    if (clipend == NULL)
    {
        const xmlChar * txt_CLIP_END = XmlReader::transcode("clip-end");
        clipend = XmlReader::transcode(mpAttributes->getValue(txt_CLIP_END));
        XmlReader::release(txt_CLIP_END);
    }

    if (clipend != NULL)
    {
        string str;
        str.assign(clipend);
        XmlReader::release(clipend);

        //assign the media node's clipbegin value
        p_audioMedia->setClipEnd(str);
    }

    //return a pointer to the new audio node
    return (Node*) p_audio;
}

//--------------------------------------------------
//create a text node
//--------------------------------------------------
Node* NodeBuilder::createTextNode()
{
    //attribute value stored here
    string attribute_value;

    //create a new text node
    ContentNode* p_text = new ContentNode();
    p_text->createNewText();

    amis::TextNode* p_textMedia = (amis::TextNode*) p_text->getMediaNode();

    //get and save the Id attribute

    const xmlChar * txt_ID = XmlReader::transcode("id");

    const char* id = NULL;
    id = XmlReader::transcode(mpAttributes->getValue(txt_ID));

    XmlReader::release(txt_ID);

    if (id != NULL)
    {
        string str;
        str.assign(id);

        XmlReader::release(id);

        p_textMedia->setId(str);
        p_text->setElementId(str);
    }

    //get and save the media type attribute

    const xmlChar * txt_TYPE = XmlReader::transcode("type");

    const char* type = XmlReader::transcode(mpAttributes->getValue(txt_TYPE));

    XmlReader::release(txt_TYPE);

    if (type != NULL)
    {
        string str;
        str.assign(type);

        XmlReader::release(type);
        p_textMedia->setMediaType(str);
    }

    //get and save the content region

    const xmlChar * txt_REGION = XmlReader::transcode("region");

    const char* region = NULL;
    region = XmlReader::transcode(mpAttributes->getValue(txt_REGION));

    XmlReader::release(txt_REGION);

    if (region != NULL)
    {
        string str;
        str.assign(region);
        XmlReader::release(region);
        p_textMedia->setRegionId(str);
    }

    //get and save the content src

    const xmlChar * txt_SRC = XmlReader::transcode("src");

    const char* src = NULL;
    src = XmlReader::transcode(mpAttributes->getValue(txt_SRC));

    XmlReader::release(txt_SRC);

    if (src != NULL)
    {
        string str;
        str.assign(src);
        XmlReader::release(src);
        str = amis::FilePathTools::goRelativePath(mSmilPath, str);

        p_textMedia->setSrc(str);
    }

    //return a pointer to the new text node
    return (Node*) p_text;
}

//--------------------------------------------------
//create an image node
//--------------------------------------------------
Node* NodeBuilder::createImageNode()
{
    //attribute values stored here
    string attribute_value;

    //create a new image node
    ContentNode* p_image = new ContentNode();
    p_image->createNewImage();

    amis::ImageNode* p_imageMedia = (amis::ImageNode*) p_image->getMediaNode();

    //get and save the Id attribute

    const xmlChar * txt_ID = XmlReader::transcode("id");

    const char* id = NULL;
    id = XmlReader::transcode(mpAttributes->getValue(txt_ID));

    XmlReader::release(txt_ID);

    if (id != NULL)
    {
        string str;
        str.assign(id);
        XmlReader::release(id);
        p_imageMedia->setId(str);
        p_image->setElementId(str);

    }

    //get and save the media type attribute

    const xmlChar * txt_TYPE = XmlReader::transcode("type");

    const char* type = XmlReader::transcode(mpAttributes->getValue(txt_TYPE));

    XmlReader::release(txt_TYPE);

    if (type != NULL)
    {
        string str;
        str.assign(type);
        XmlReader::release(type);
        p_imageMedia->setMediaType(str);
    }

    //get and save the content region

    const xmlChar * txt_REGION = XmlReader::transcode("region");

    const char* region = NULL;
    region = XmlReader::transcode(mpAttributes->getValue(txt_REGION));

    XmlReader::release(txt_REGION);

    if (region != NULL)
    {
        string str;
        str.assign(region);
        XmlReader::release(region);

        p_imageMedia->setRegionId(str);

    }

    //get and save the content src

    const xmlChar * txt_SRC = XmlReader::transcode("src");

    const char* src = NULL;
    src = XmlReader::transcode(mpAttributes->getValue(txt_SRC));

    XmlReader::release(txt_SRC);

    if (src != NULL)
    {
        string str;
        str.assign(src);
        XmlReader::release(src);
        str = amis::FilePathTools::goRelativePath(mSmilPath, str);

        p_imageMedia->setSrc(str);
    }

    //return the pointer to a new image node
    return (Node*) p_image;
}

