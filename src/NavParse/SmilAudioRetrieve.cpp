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

//SYSTEM INCLUDES
#include <string>
#include <iostream>
#include <vector>
#include <cstring>

#include "FilePathTools.h"
#include "SmilAudioRetrieve.h"

#include <XmlReader.h>
#include <XmlAttributes.h>
#include <XmlError.h>

#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.amis
log4cxx::LoggerPtr amisSmilAudioRetLog(
        log4cxx::Logger::getLogger("kolibre.amis.smilaudioretrieve"));

using namespace std;

const char* TAG_PAR = "par";
const char* TAG_TEXT = "text";
const char* TAG_AUDIO = "audio";
const char* ATTR_ID = "id";

AudioElement::AudioElement()
{
    this->mbIsOwned = false;
    this->mpAudioData = NULL;
    this->mParId = "";
    this->mTextId = "";
}
AudioElement::~AudioElement()
{
    //only delete the audio data if it is not owned by the calling application (ie, NavParse)
    if (mbIsOwned == false)
    {
        if (mpAudioData != NULL)
        {
            delete mpAudioData;
        }
    }
}

//LIFECYCLES
SmilAudioRetrieve::SmilAudioRetrieve()
{
    mAudioList.clear();
    mCurrentParId = "";
    mCurrentTextId = "";
    mSmilSourceFile = "";
    mpCurrentAudioNode = NULL;
    mError.setSourceModuleName(amis::module_NavEngine);
}

SmilAudioRetrieve::~SmilAudioRetrieve()
{
    cleanVector();
}

amis::AudioNode* SmilAudioRetrieve::findId(string id)
{

    unsigned int i;
    amis::AudioNode* p_return_value = NULL;

    for (i = 0; i < mAudioList.size(); i++)
    {
        if (mAudioList[i]->mParId.compare(id) == 0
                || mAudioList[i]->mTextId.compare(id) == 0)
        {
            mAudioList[i]->mbIsOwned = true;
            p_return_value = mAudioList[i]->mpAudioData;
            break;
        }
    }

    return p_return_value;

}

void SmilAudioRetrieve::cleanVector()
{
    int sz = mAudioList.size();
    AudioElement* tmp_ptr;
    for (int i = sz - 1; i >= 0; i--)
    {
        tmp_ptr = mAudioList[i];
        mAudioList.pop_back();
        delete tmp_ptr;
    }
}

amis::AmisError SmilAudioRetrieve::getAudioReference(string smilFile,
        amis::AudioNode* pNode)
{
    audionodecnt = 0;
    //open a file, if not already open, and get all the audio references from it
    //match them up with ids

    //cout << "SmilAudioRetrieve::getAudioReference(string " << smilFile << ")" << endl;

    //local variables
    const char* cp_file;
    string tmp_string;
    XmlReader parser;

    mbFoundAudio = false;

    string search_id = amis::FilePathTools::getTarget(smilFile);

    string tmp1 = amis::FilePathTools::getAsLocalFilePath(mSmilSourceFile);
    string tmp2 = amis::FilePathTools::getAsLocalFilePath(smilFile);

    if (tmp1.compare(tmp2) != 0)
    {
        //do a SAX parse of this new file
        //save the file path
        mSmilSourceFile = smilFile;

        mSmilSourceFile = amis::FilePathTools::clearTarget(mSmilSourceFile);
        mError.setCode(amis::OK);
        mError.setMessage("");
        mError.setFilename(smilFile);

        cleanVector();

        mCurrentParId = "";
        mCurrentTextId = "";

        //get the local file path version of the SMIL file path
        tmp_string = amis::FilePathTools::getAsLocalFilePath(smilFile);

        cp_file = tmp_string.c_str();

        parser.setContentHandler(this);
        parser.setErrorHandler(this);

        bool ret = parser.parseXml(cp_file);
        if (!ret)
        {
            const XmlError *e = parser.getLastError();
            if (e)
            {
                LOG4CXX_ERROR(amisSmilAudioRetLog,
                        "Error in SmilAudioRetrieve: " << e->getMessage());
                mError.loadXmlError(*e);
            }
            else
            {
                LOG4CXX_ERROR(amisSmilAudioRetLog,
                        "Unknown error in SmilAudioRetrieve");
                mError.setCode(amis::UNDEFINED_ERROR);
            }
        }
    }

    int audioelems = mAudioList.size();

    pNode = findId(search_id);

    return mError;
}

//SAX METHODS
//--------------------------------------------------
//! (SAX Event) analyze the element type and collect data to build a node
//--------------------------------------------------
bool SmilAudioRetrieve::startElement(const xmlChar* const uri,
        const xmlChar* const localname, const xmlChar* const qname,
        const XmlAttributes &attributes)
{
    //local variables
    const char* element_name = NULL;
    const char* attribute_name = NULL;

    int len = attributes.getLength();

    //get the element name as a string
    element_name = XmlReader::transcode(qname);
    //cout << "SmilAudioRetrieve::startElement: '" << element_name << "'" << endl;

    //large "if, else if, else" statement to match the element name

    if (strcmp(element_name, TAG_PAR) == 0
            || strcmp(element_name, TAG_TEXT) == 0)
    {
        const char* id = NULL;

        const xmlChar* txt_ID = XmlReader::transcode("id");
        id = XmlReader::transcode(attributes.getValue(txt_ID));
        XmlReader::release(txt_ID);

        if (id != NULL)
        {
            if (strcmp(element_name, TAG_PAR) == 0)
            {
                mCurrentParId.assign(id);
            }
            else
            {
                mCurrentTextId.assign(id);
            }

            XmlReader::release(id);
        } //end if attribute = "id" exists 

    } //end if element name = "par" or "text"

    else if (strcmp(element_name, TAG_AUDIO) == 0)
    {

        mbFoundAudio = true;
        const char* clipbegin = NULL;
        const char* clipend = NULL;
        const char* src = NULL;

        const xmlChar* txt_SRC = XmlReader::transcode("src");
        src = XmlReader::transcode(attributes.getValue(txt_SRC));
        XmlReader::release(txt_SRC);

        const xmlChar* txt_CLIP_BEGIN = XmlReader::transcode("clip-begin");
        clipbegin = XmlReader::transcode(attributes.getValue(txt_CLIP_BEGIN));
        XmlReader::release(txt_CLIP_BEGIN);

        const xmlChar* txt_CLIP_END = XmlReader::transcode("clip-end");
        clipend = XmlReader::transcode(attributes.getValue(txt_CLIP_END));
        XmlReader::release(txt_CLIP_END);

        //fix some memory leaks when there are more audio nodes than we are using in our list
        if (mpCurrentAudioNode != NULL && mbUsingCurrentAudioNode == false)
        {
            delete mpCurrentAudioNode;
        }

        mbUsingCurrentAudioNode = false;

        mpCurrentAudioNode = new amis::AudioNode();
        audionodecnt++;

        string str;
        str.assign(clipbegin);
        XmlReader::release(clipbegin);
        mpCurrentAudioNode->setClipBegin(str);

        string strz;
        strz.assign(clipend);
        XmlReader::release(clipend);
        mpCurrentAudioNode->setClipEnd(strz);

        string strzy;
        strzy.assign(src);
        XmlReader::release(src);
        mpCurrentAudioNode->setSrc(strzy);

        AudioElement* audio_elem = NULL;
        audio_elem = new AudioElement;
        //add a new audio element
        audio_elem->mParId = mCurrentParId;
        audio_elem->mTextId = mCurrentTextId;
        audio_elem->mpAudioData = mpCurrentAudioNode;
        mbUsingCurrentAudioNode = true;

        mAudioList.push_back(audio_elem);

    }
    XmlReader::release(element_name);
    return true;
} //end SmilTreeBuilder::startElement function

//--------------------------------------------------
//! (SAX Event) close this element so that no additional child nodes are added to it in the Smil Tree
//--------------------------------------------------
bool SmilAudioRetrieve::endElement(const xmlChar* const uri,
        const xmlChar* const localname, const xmlChar* const qname)
{
    //local variable
    const char* element_name = XmlReader::transcode(qname);

    AudioElement* audio_elem = NULL;

    //if this element is a par, then create all data collected since the open-par tag
    //and add it to the audio list
    if (strcmp(element_name, TAG_PAR) == 0)
    {
        if (mbFoundAudio == true)
        {
            /*audio_elem = new AudioElement;
             //add a new audio element
             audio_elem->mParId = mCurrentParId;
             audio_elem->mTextId = mCurrentTextId;
             audio_elem->mpAudioData = mpCurrentAudioNode;
             mbUsingCurrentAudioNode = true;
             
             mAudioList.push_back(audio_elem);*/

            mCurrentParId = "";
            mCurrentTextId = "";
        }
    }

    XmlReader::release(element_name);

    return true;
}
/*
 amis::AudioNode* SmilAudioRetrieve::buildAudioNode(char*)
 {
 
 char* attribute_name;
 char* attribute_value;
 string tmpstr;

 int len = attributes->getLength();
 
 amis::AudioNode* p_node = new amis::AudioNode();

 //get and save the content src
 char* src = NULL;
 src =
 XMLSting::transcode(attributes->getValue(XMLSting::transcode("src")));
 if (src != NULL)
 {
 string src_str = src;
 src_str = amis::FilePathTools::goRelativePath(mSmilSourceFile, src_str);

 p_node->setSrc(src_str);
 }

 XMLSting::release(src);

 //get the clip begin attribute
 char* clipbegin = NULL;
 clipbegin = 
 XMLSting::transcode(attributes->getValue(XMLSring::transcode("clip-begin")));

 if (clipbegin != NULL)
 {
 //assign the media node's clipbegin value
 p_node->setClipBegin(clipbegin);
 }

 XMLSting::release(clipbegin);

 //get the clip end attribute
 //it could be in two forms: clip-begin or clipBegin
 char* clipend = NULL;
 clipend = 
 XMLSting::transcode(attributes->getValue(XMLSting::transcode("clip-end")));

 if (clipend != NULL)
 {
 //assign the media node's clipbegin value
 p_node->setClipEnd(clipend);
 }

 XMLSting::release(clipend);

 return p_node;
 }

 */
//--------------------------------------------------
//! (SAX Event) error
//--------------------------------------------------
bool SmilAudioRetrieve::error(const XmlError& e)
{
    //cerr<<"error"<<endl;
    return true;
}

//--------------------------------------------------
//! (SAX Event) fatal error
//--------------------------------------------------
bool SmilAudioRetrieve::fatalError(const XmlError& e)
{
    mError.loadXmlError(e);
    return false;
}

//--------------------------------------------------
//! (SAX Event) warning
//--------------------------------------------------
bool SmilAudioRetrieve::warning(const XmlError& e)
{
    //	cerr<<"warning"<<endl;
    return true;
}
