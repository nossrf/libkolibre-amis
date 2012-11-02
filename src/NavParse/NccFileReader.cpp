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

#include "FilePathTools.h"
#include "trim.h"
#include "NccFileReader.h"
#include "AmisCommon.h"

#include <XmlReader.h>
#include <XmlAttributes.h>
#include <XmlError.h>

#include <algorithm>
#include <iostream>
#include <log4cxx/logger.h>

using namespace std;
using namespace amis;
// create logger which will become a child to logger kolibre.amis
log4cxx::LoggerPtr amisNccFileReadLog(
        log4cxx::Logger::getLogger("kolibre.amis.nccfilereader"));

NccFileReader::NccFileReader()
{
    //mpSmilAudio = new SmilAudioRetrieve();

}

NccFileReader::~NccFileReader()
{
    LOG4CXX_DEBUG(amisNccFileReadLog, "deleting nccfilereader");
    //sleep(10);
    //delete mpSmilAudio;
}

void NccFileReader::init()
{
    mPlayOrderCount = 0;
    mbFlag_GetChars = false;

}

amis::AmisError NccFileReader::open(string filepath, NavModel* pModel)
{
    amis::AmisError err;

    LOG4CXX_DEBUG(amisNccFileReadLog, "NccFileReader opening: " << filepath);

    mOpenNodes.clear();
    mpCurrentNavPoint = NULL;

    mError.setCode(amis::OK);
    mError.setMessage("");
    mError.setFilename(filepath);

    XmlReader parser;

    mFilePath = amis::FilePathTools::getAsLocalFilePath(filepath);
    mpNavModel = pModel;

    //push the root onto the open nodes list
    mOpenNodes.push_back(mpNavModel->getNavMap()->getRoot());

    parser.setContentHandler(this);
    parser.setErrorHandler(this);

    if (!parser.parseHtml(mFilePath.c_str()))
    {
        const XmlError *e = parser.getLastError();
        if (e)
        {
            LOG4CXX_ERROR(amisNccFileReadLog,
                    "Error in NccFileReader: " << e->getMessage());
            mError.loadXmlError(*e);
        }
        else
        {
            LOG4CXX_ERROR(amisNccFileReadLog, "Unknown error in NccFileReader");
            mError.setCode(amis::UNDEFINED_ERROR);
        }
    }

    LOG4CXX_DEBUG(amisNccFileReadLog, "NccFileReader done");

    return mError;

}

//!xmlreader start element event
bool NccFileReader::startElement(const xmlChar* const uri,
        const xmlChar* const localname, const xmlChar* const qname,
        const XmlAttributes& attributes)
{
    //get the name of the node from xmlreader 
    const char* node_name_ = XmlReader::transcode(qname);
    string node_name;
    node_name.assign(node_name_);
    XmlReader::release(node_name_);

    //convert the node name to lowercase letters
    std::transform(node_name.begin(), node_name.end(), node_name.begin(), (int (*)(int))tolower);

    int len;

    int heading_level = 0;
    heading_level = isHeading(node_name);

    //if this is a heading node
    if (heading_level > 0)
    {
        //check and see if we can close an open node
        len = mOpenNodes.size();

        if (len != 0)
        {
            NavPoint* p_tmp = mOpenNodes[len - 1];

            //if this heading is lesser/equal to the previous one

            int diff = p_tmp->getLevel() - heading_level + 1;

            if (diff > 0)
            {
                for (int i = 0; i < diff; i++)
                {
                    //then the previous one(s) will not be accepting children
                    mOpenNodes.pop_back();
                }
            }
        }

        mListType = 0;

        //allocate memory for the new node
        mpCurrentNavPoint = new NavPoint();

        const char* class_name = NULL;

        const xmlChar* txt_CLASS = XmlReader::transcode("class");
        class_name = XmlReader::transcode(attributes.getValue(txt_CLASS));
        XmlReader::release(txt_CLASS);

        if (class_name != NULL)
        {
            string str;
            str.assign(class_name);
            XmlReader::release(class_name);
            mpCurrentNavPoint->setClass(str);
        }

        const char* id = NULL;

        const xmlChar* txt_ID = XmlReader::transcode("id");
        id = XmlReader::transcode(attributes.getValue(txt_ID));
        XmlReader::release(txt_ID);

        if (id != NULL)
        {
            string str;
            str.assign(id);
            XmlReader::release(id);
            mpCurrentNavPoint->setId(str);
        }

        mpCurrentNavPoint->setLevel(heading_level);

        //set label (gather text and retrieve audio clip)
        //done during <a> tag processing
        amis::MediaGroup* p_media_label = new amis::MediaGroup();
        mpCurrentNavPoint->setLabel(p_media_label);

        //set play order
        mpCurrentNavPoint->setPlayOrder(mPlayOrderCount);
        mPlayOrderCount++;

        //get the last item from the open nodes list and set it as the parent for this node
        len = mOpenNodes.size();
        NavPoint* p_parent = mOpenNodes[len - 1];

        mpNavModel->getNavMap()->recordNewDepth(mpCurrentNavPoint->getLevel());
        p_parent->addChild(mpCurrentNavPoint);

        mOpenNodes.push_back(mpCurrentNavPoint);

    }

    else if (node_name.compare("span") == 0)
    {
        string class_value;

        const char* class_name = NULL;

        const xmlChar* txt_CLASS = XmlReader::transcode("class");
        class_name = XmlReader::transcode(attributes.getValue(txt_CLASS));
        XmlReader::release(txt_CLASS);

        if (class_name != NULL)
        {
            class_value.assign(class_name);
            XmlReader::release(class_name);
        }

        if (class_value.compare("page-front") == 0
                || class_value.compare("page-normal") == 0
                || class_value.compare("page-special") == 0)
        {
            mListType = 2;

            class_value = class_value.substr(5);
            PageTarget* p_page = new PageTarget();
            p_page->setClass(class_value);
            p_page->setPlayOrder(mPlayOrderCount);

            const char* id = NULL;

            const xmlChar* txt_ID = XmlReader::transcode("id");
            id = XmlReader::transcode(attributes.getValue(txt_ID));
            XmlReader::release(txt_ID);
            if (id != NULL)
            {
                string str;
                str.assign(id);
                XmlReader::release(id);
                p_page->setId(str);
            }

            if (class_value.compare("front") == 0)
            {
                p_page->setType(PageTarget::PAGE_FRONT);
            }
            else if (class_value.compare("normal") == 0)
            {
                p_page->setType(PageTarget::PAGE_NORMAL);
            }
            else if (class_value.compare("special") == 0)
            {
                p_page->setType(PageTarget::PAGE_SPECIAL);
            }
            else
            {
                // empty
                cerr << "unknown class type: '" << class_value << "'" << endl;
            }

            mPlayOrderCount++;

            amis::MediaGroup* p_media_label = new amis::MediaGroup();
            p_page->setLabel(p_media_label);

            PageList* p_page_list;
            p_page_list = mpNavModel->getPageList();

            p_page_list->addNode(p_page);

            mpCurrentPageTarget = p_page;

            addCustomTest("pagenumber", true, true, "");

        }

        else if (class_value.compare("sidebar") == 0
                || class_value.compare("optional-prodnote") == 0
                || class_value.compare("noteref") == 0
                || class_value.compare("group") == 0)
        {
            mListType = 1;

            NavTarget* p_navt = new NavTarget();

            p_navt->setClass(class_value);
            p_navt->setPlayOrder(mPlayOrderCount);

            const char* id = NULL;

            const xmlChar* txt_ID = XmlReader::transcode("id");
            id = XmlReader::transcode(attributes.getValue(txt_ID));
            XmlReader::release(txt_ID);

            if (id != NULL)
            {
                string str;
                str.assign(id);
                XmlReader::release(id);
                p_navt->setId(str);
            }

            mPlayOrderCount++;

            amis::MediaGroup* p_media_label = new amis::MediaGroup();
            p_navt->setLabel(p_media_label);

            NavList* p_nav_list;
            p_nav_list = mpNavModel->getNavList(class_value);

            if (p_nav_list == NULL)
            {
                int newidx = mpNavModel->addNavList(class_value);

                p_nav_list = mpNavModel->getNavList(newidx);
            }

            p_nav_list->addNode(p_navt);

            mpCurrentNavTarget = p_navt;

            //see if we should add a custom test to the list
            string custom_test;

            if (class_value.compare("sidebar") == 0)
            {
                custom_test = "sidebar";
            }
            else if (class_value.compare("optional-prodnote") == 0)
            {
                custom_test = "prodnote";
            }
            else if (class_value.compare("noteref") == 0)
            {
                custom_test = "footnote";
            }
            else
            {
                custom_test = "";
            }

            if (custom_test.compare("") != 0)
            {
                addCustomTest(custom_test, true, true, "");
            }

        }

        else
        {
            //empty
        }
    }

    else if (node_name.compare("div") == 0)
    {
        const char* class_name = NULL;

        const xmlChar* txt_CLASS = XmlReader::transcode("class");
        class_name = XmlReader::transcode(attributes.getValue(txt_CLASS));
        XmlReader::release(txt_CLASS);

        string class_value;
        if (class_name != NULL)
        {
            class_value.assign(class_name);
            XmlReader::release(class_name);
        }

        mListType = 1;

        NavTarget* p_navt = new NavTarget();

        p_navt->setClass(class_value);
        p_navt->setPlayOrder(mPlayOrderCount);
        amis::MediaGroup* p_media_label = new amis::MediaGroup();
        p_navt->setLabel(p_media_label);

        mPlayOrderCount++;

        NavList* p_nav_list;
        p_nav_list = mpNavModel->getNavList(class_value);

        if (p_nav_list == NULL)
        {
            int newidx = mpNavModel->addNavList(class_value);

            p_nav_list = mpNavModel->getNavList(newidx);
        }

        p_nav_list->addNode(p_navt);

        mpCurrentNavTarget = p_navt;
    }

    //special condition for links due to the nested <a> element
    else if (node_name.compare("a") == 0)
    {
        string href;
        const char* href_val = NULL;

        const xmlChar* txt_HREF = XmlReader::transcode("href");
        href_val = XmlReader::transcode(attributes.getValue(txt_HREF));
        XmlReader::release(txt_HREF);

        if (href_val != NULL)
        {
            href.assign(href_val);
            XmlReader::release(href_val);
        }

        //cout << "a element: getting href" << endl;
        href = amis::FilePathTools::goRelativePath(mFilePath, href);

        //resolve the audio label here, based on the href
        //amis::AudioNode* p_audio = NULL;

        // Disabled for speedup
        //amis::AmisError err;
        //err.setCode(amis::OK);
        //cout << "a element: getting audio reference for " << href << endl;
        //p_audio = mpSmilAudio->getAudioReference(href, &err);

        mbFlag_GetChars = true;
        mTempChars.erase();
        if (mListType == 0)
        {
            mpCurrentNavPoint->setContent(href);

            //amis::MediaGroup* p_media_label = mpCurrentNavPoint->getLabel();
            //p_media_label->addAudioClip(p_audio);	
        }
        else if (mListType == 1)
        {
            //nav list
            mpCurrentNavTarget->setContent(href);

            //amis::MediaGroup* p_media_label = mpCurrentNavTarget->getLabel();
            //p_media_label->addAudioClip(p_audio);	
        }
        else if (mListType == 2)
        {
            //page list
            mpCurrentPageTarget->setContent(href);

            //amis::MediaGroup* p_media_label = mpCurrentPageTarget->getLabel();
            //p_media_label->addAudioClip(p_audio);	
        }
        else
        {
            //empty
        }

        /*if(err.getCode() != amis::OK) {
         mBuildErrorFlag = err.getCode();
         mBuildErrorMsg = err.getMessage();			
         }*/
    }

    else
    {
        //cerr << "Unknown tag type '" << node_name << "'" << endl; //empty
    }

    return true;
}

//!xmlreader end element event
bool NccFileReader::endElement(const xmlChar* const uri,
        const xmlChar* const localname, const xmlChar* const qname)
{
    //get the name of the node from xmlreader 
    const char* node_name_ = XmlReader::transcode(qname);
    string node_name;
    node_name.assign(node_name_);
    XmlReader::release(node_name_);

    //convert the node name to lowercase letters
    std::transform(node_name.begin(), node_name.end(), node_name.begin(),
            (int (*)(int))tolower);

if(    node_name.compare("a") == 0)
    {
        mbFlag_GetChars = false;
    }
    return true;
}

bool NccFileReader::characters(const xmlChar* const chars,
        const unsigned int length)
{
    if (mbFlag_GetChars == true)
    {

        amis::MediaGroup* p_media_label = NULL;

        //unsigned int length = XmlReader::stringLen(chars);
        //std::cerr << XmlReader::transcode(chars) << std::endl;
        //wchar_t *chars_conv = alloc_utf16_wchart(chars, length);
        //free(chars_conv);

        //wchar_t *conv_str = mbstowcs_alloc((const char*) chars, XmlReader::stringLen(chars));

        const char* tmpchars = XmlReader::transcode(chars);
        mTempChars.append(tmpchars, length);
        XmlReader::release(tmpchars);

        //wcerr << conv_str << endl;
        //free (conv_str);

        if (mListType == 0)
        {
            p_media_label = mpCurrentNavPoint->getLabel();
        }
        else if (mListType == 1)
        {
            p_media_label = mpCurrentNavTarget->getLabel();
        }
        else if (mListType == 2)
        {
            p_media_label = mpCurrentPageTarget->getLabel();
        }
        else
        {
        }

        amis::TextNode* p_text = new amis::TextNode();
        p_text->setTextString(trim(mTempChars));

        //@TODO: set other text properties (RTL, lang, etc)

        p_media_label->setText(p_text);

    }

    return true;
}

//return 1...6 indicating the heading level; otherwise return 0
int NccFileReader::isHeading(string nodeName)
{
    //check type
    if (nodeName.compare("h1") == 0 || nodeName.compare("h2") == 0
            || nodeName.compare("h3") == 0 || nodeName.compare("h4") == 0
            || nodeName.compare("h5") == 0 || nodeName.compare("h6") == 0)
    {
        string str_level = nodeName.substr(1);

        const char* c_level = str_level.c_str();
        int i_level = atoi(c_level);

        return i_level;
    }
    return 0;
}
