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
#include "NcxFileReader.h"

#include <XmlReader.h>
#include <XmlAttributes.h>
#include <XmlError.h>

#include <iostream>
#include <log4cxx/logger.h>

using namespace std;
using namespace amis;

// create logger which will become a child to logger kolibre.amis
log4cxx::LoggerPtr amisNcxFileReadLog(
        log4cxx::Logger::getLogger("kolibre.amis.ncxfilereader"));

NcxFileReader::NcxFileReader()
{
    //mpSmilAudio = new SmilAudioRetrieve();

}

NcxFileReader::~NcxFileReader()
{
    //delete mpSmilAudio;
}

void NcxFileReader::init()
{
    mbFlag_GetChars = false;

    this->mpCurrentMediaGroup = NULL;
    this->mpCurrentNavContainer = NULL;
    this->mpCurrentNavPoint = NULL;
    this->mpCurrentNavTarget = NULL;
    this->mpCurrentNode = NULL;
    this->mpCurrentPageTarget = NULL;

}

amis::AmisError NcxFileReader::open(string filepath, NavModel* pModel)
{
    amis::AmisError err;

    LOG4CXX_DEBUG(amisNcxFileReadLog, "NcxFileReader opening: " << filepath);

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

    if (!parser.parseXml(mFilePath.c_str()))
    {
        const XmlError *e = parser.getLastError();
        if (e)
        {
            LOG4CXX_ERROR(amisNcxFileReadLog,
                    "Error in NcxFileReader: " << e->getMessage());
            mError.loadXmlError(*e);
        }
        else
        {
            LOG4CXX_ERROR(amisNcxFileReadLog, "Unknown error in NcxFileReader");
            mError.setCode(amis::UNDEFINED_ERROR);
        }

    }

    return mError;

}

//!xmlreader start element event
bool NcxFileReader::startElement(const xmlChar* const uri,
        const xmlChar* const localname, const xmlChar* const qname,
        const XmlAttributes& attributes)
{
    //get the name of the node from xmlreader 
    const char* node_name_ = XmlReader::transcode(qname);
    string node_name;
    node_name.assign(node_name_);
    XmlReader::release(node_name_);

    //if this is a heading node
    if (node_name.compare("navPoint") == 0)
    {
        //allocate memory for the new node
        mpCurrentNavPoint = new NavPoint();

        int lvl = mOpenNodes.size();

        for (int i = 0; i < lvl; i++)
            cout << "\t";
        //cout << "lvl:" << lvl;

        const char* class_name = NULL;

        const xmlChar* txt_CLASS = XmlReader::transcode("class");
        class_name = XmlReader::transcode(attributes.getValue(txt_CLASS));
        XmlReader::release(txt_CLASS);

        //assign values to the node
        if (class_name != NULL)
        {
            string str;
            str.assign(class_name);
            XmlReader::release(class_name);
            mpCurrentNavPoint->setClass(str);
            //cout << " class: " << str;
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
            //cout << " id: " << str;
        }

        mpCurrentNavPoint->setLevel(lvl);

        //set play order
        const char* play_order = NULL;

        const xmlChar* txt_PLAYORDER = XmlReader::transcode("playOrder");
        play_order = XmlReader::transcode(attributes.getValue(txt_PLAYORDER));
        XmlReader::release(txt_PLAYORDER);

        if (play_order != NULL)
        {
            int i_play_order = atoi(play_order);
            mpCurrentNavPoint->setPlayOrder(i_play_order);
            XmlReader::release(play_order);
            //cout << " playorder: " << i_play_order;
        }
        else
        {
            LOG4CXX_ERROR(amisNcxFileReadLog,
                    "found NavPoint without playorder " << mpCurrentNavPoint->getId());
        }

        //cout << endl;

        //get the last item from the open nodes list and set it as the parent for this node
        int len = mOpenNodes.size();
        NavPoint* p_parent = mOpenNodes[len - 1];

        mpNavModel->getNavMap()->recordNewDepth(mpCurrentNavPoint->getLevel());
        p_parent->addChild(mpCurrentNavPoint);

        mOpenNodes.push_back(mpCurrentNavPoint);

        mpCurrentNode = mpCurrentNavPoint;

    }

    else if (node_name.compare("docTitle") == 0)
    {
        //mpCurrentMediaGroup = new amis::MediaGroup();
    }
    else if (node_name.compare("docAuthor") == 0)
    {
        //mpCurrentMediaGroup = new amis::MediaGroup();

    }
    else if (node_name.compare("navLabel") == 0)
    {
        mpCurrentMediaGroup = new amis::MediaGroup();
        if (mpCurrentNode != NULL)
        {
            mpCurrentNode->setLabel(mpCurrentMediaGroup);
        }
        else if (mpCurrentNavContainer != NULL)
        {
            mpCurrentNavContainer->setLabel(mpCurrentMediaGroup);
        }
    }
    else if (node_name.compare("text") == 0)
    {
        if (mpCurrentMediaGroup != NULL)
        {
            amis::TextNode* p_text = new amis::TextNode();
            mTempChars.clear();
            mpCurrentMediaGroup->setText(p_text);
            this->mbFlag_GetChars = true;
        }
    }
    else if (node_name.compare("audio") == 0)
    {
        if (mpCurrentMediaGroup != NULL)
        {
            amis::AudioNode* p_audio = new amis::AudioNode();

            //get and save the content src
            const char* src = NULL;

            const xmlChar* txt_SRC = XmlReader::transcode("src");
            src = XmlReader::transcode(attributes.getValue(txt_SRC));
            XmlReader::release(txt_SRC);

            if (src != NULL)
            {
                string str;
                str.assign(src);
                XmlReader::release(src);
                str = amis::FilePathTools::goRelativePath(mFilePath, str);
                p_audio->setSrc(str);
            }

            //get the clip begin attribute
            const char* clipbegin = NULL;

            const xmlChar* txt_CLIPBEGIN = XmlReader::transcode("clipBegin");
            clipbegin = XmlReader::transcode(
                    attributes.getValue(txt_CLIPBEGIN));
            XmlReader::release(txt_CLIPBEGIN);

            if (clipbegin != NULL)
            {
                string str;
                str.assign(clipbegin);
                XmlReader::release(clipbegin);
                //assign the media node's clipbegin value
                p_audio->setClipBegin(str);
            }

            //get the clip begin attribute
            //it could be in two forms: clip-begin or clipBegin
            const char* clipend = NULL;

            const xmlChar* txt_CLIPEND = XmlReader::transcode("clipEnd");
            clipend = XmlReader::transcode(attributes.getValue(txt_CLIPEND));
            XmlReader::release(txt_CLIPEND);

            if (clipend != NULL)
            {
                string str;
                str.assign(clipend);
                XmlReader::release(clipend);
                //assign the media node's clipbegin value
                p_audio->setClipEnd(str);
            }

            mpCurrentMediaGroup->addAudioClip(p_audio);
        }
    }
    else if (node_name.compare("navTarget") == 0)
    {
        mpCurrentNavTarget = new NavTarget();

        const char* class_name = NULL;

        const xmlChar* txt_CLASS = XmlReader::transcode("class");
        class_name = XmlReader::transcode(attributes.getValue(txt_CLASS));
        XmlReader::release(txt_CLASS);

        if (class_name != NULL)
        {
            string str;
            str.assign(class_name);
            XmlReader::release(class_name);
            mpCurrentNavTarget->setClass(str);
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
            mpCurrentNavTarget->setId(str);
        }
        //set play order
        const char* play_order = NULL;

        const xmlChar* txt_PLAYORDER = XmlReader::transcode("playOrder");
        play_order = XmlReader::transcode(attributes.getValue(txt_PLAYORDER));
        XmlReader::release(txt_PLAYORDER);

        if (play_order != NULL)
        {
            int i_play_order = atoi(play_order);
            XmlReader::release(play_order);
            mpCurrentNavTarget->setPlayOrder(i_play_order);
        }
        else
        {
            LOG4CXX_ERROR(amisNcxFileReadLog,
                    "found NavTarget without playorder " + mpCurrentNavTarget->getId());
        }

        NavList* p_nav_list = NULL;

        int num = mpNavModel->getNumberOfNavLists();
        if (num > 0)
        {
            p_nav_list = mpNavModel->getNavList(num - 1);

            p_nav_list->addNode(mpCurrentNavTarget);

            mpCurrentNode = mpCurrentNavTarget;
        }
    }
    else if (node_name.compare("pageTarget") == 0)
    {
        PageList* p_page_list;
        p_page_list = mpNavModel->getPageList();

        PageTarget* p_page = new PageTarget();

        const char* class_name = NULL;

        const xmlChar* txt_CLASS = XmlReader::transcode("class");
        class_name = XmlReader::transcode(attributes.getValue(txt_CLASS));
        XmlReader::release(txt_CLASS);

        if (class_name != NULL)
        {
            string str;
            str.assign(class_name);
            XmlReader::release(class_name);
            p_page->setClass(str);
        }

        const char* type_name = NULL;

        const xmlChar* txt_TYPE = XmlReader::transcode("type");
        type_name = XmlReader::transcode(attributes.getValue(txt_TYPE));
        XmlReader::release(txt_TYPE);

        if (type_name != NULL)
        {
            string type_value;
            type_value.assign(type_name);
            XmlReader::release(type_name);

            if (type_value.compare("front") == 0)
            {
                p_page->setType(PageTarget::PAGE_FRONT);
            }
            else if (type_value.compare("normal") == 0)
            {
                p_page->setType(PageTarget::PAGE_NORMAL);
            }
            else if (type_value.compare("special") == 0)
            {
                p_page->setType(PageTarget::PAGE_SPECIAL);
            }
            else
            {
                // empty
                cerr << "unknown class type: '" << type_value << "'" << endl;
            }
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
            p_page->setId(str);
        }

        //set play order
        const char* play_order = NULL;

        const xmlChar* txt_PLAYORDER = XmlReader::transcode("playOrder");
        play_order = XmlReader::transcode(attributes.getValue(txt_PLAYORDER));
        XmlReader::release(txt_PLAYORDER);

        if (play_order != NULL)
        {
            int i_play_order = atoi(play_order);
            XmlReader::release(play_order);
            p_page->setPlayOrder(i_play_order);
        }
        else
        {
            LOG4CXX_ERROR(amisNcxFileReadLog,
                    "found PageTarget without playorder " << p_page->getId());
        }

        p_page_list->addNode(p_page);

        mpCurrentPageTarget = p_page;

        mpCurrentNode = mpCurrentPageTarget;
    }
    else if (node_name.compare("content") == 0)
    {
        const char* src = NULL;

        const xmlChar* txt_SRC = XmlReader::transcode("src");
        src = XmlReader::transcode(attributes.getValue(txt_SRC));
        XmlReader::release(txt_SRC);

        if (mpCurrentNode != NULL)
        {
            if (src != NULL)
            {
                string str_src;
                str_src.assign(src);
                XmlReader::release(src);
                str_src = amis::FilePathTools::goRelativePath(mFilePath,
                        str_src);
                mpCurrentNode->setContent(str_src);
            }
        }
    }
    else if (node_name.compare("navList") == 0)
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

        int idx = mpNavModel->addNavList(class_value);

        NavList* p_list = mpNavModel->getNavList(idx);

        mpCurrentNavContainer = p_list;

    }
    else if (node_name.compare("pageList") == 0)
    {
        PageList* p_list = mpNavModel->getPageList();
        mpCurrentNavContainer = p_list;
    }
    else if (node_name.compare("navInfo") == 0)
    {
        mpCurrentMediaGroup = new amis::MediaGroup();
        mpCurrentNavContainer->setNavInfo(mpCurrentMediaGroup);
    }
    else if (node_name.compare("navMap") == 0)
    {
        mpCurrentNavContainer = mpNavModel->getNavMap();
    }
    else if (node_name.compare("smilCustomTest") == 0)
    {

        string id_str;

        const char* id = NULL;

        const xmlChar* txt_ID = XmlReader::transcode("id");
        id = XmlReader::transcode(attributes.getValue(txt_ID));
        XmlReader::release(txt_ID);

        if (id != NULL)
        {
            id_str.assign(id);
            XmlReader::release(id);
        }

        const char* book_struct = NULL;

        const xmlChar* txt_BOOKSTRUCT = XmlReader::transcode("bookStruct");
        book_struct = XmlReader::transcode(attributes.getValue(txt_BOOKSTRUCT));
        XmlReader::release(txt_BOOKSTRUCT);

        string book_struct_str;
        if (book_struct != NULL)
        {
            book_struct_str.assign(book_struct);
            XmlReader::release(book_struct);
        }

        addCustomTest(id_str, true, true, book_struct_str);
    }
    return true;
}

//!xmlreader end element event
bool NcxFileReader::endElement(const xmlChar* const uri,
        const xmlChar* const localname, const xmlChar* const qname)
{
    //local variable
    const char* element_name = XmlReader::transcode(qname);

    if (strcmp(element_name, "navPoint") == 0)
    {
        mOpenNodes.pop_back();
        mpCurrentNavPoint = NULL;
        mpCurrentNode = NULL;
    }
    else if (strcmp(element_name, "navTarget") == 0)
    {
        mpCurrentNavTarget = NULL;
        mpCurrentNode = NULL;
    }
    else if (strcmp(element_name, "pageTarget") == 0)
    {
        mpCurrentPageTarget = NULL;
        mpCurrentNode = NULL;
    }
    else if (strcmp(element_name, "pageList") == 0)
    {
        mpCurrentNavContainer = NULL;
    }
    else if (strcmp(element_name, "navList") == 0)
    {
        mpCurrentNavContainer = NULL;
    }
    else if (strcmp(element_name, "navMap") == 0)
    {
        mpCurrentNavContainer = NULL;
    }

    else if (strcmp(element_name, "text") == 0)
    {
        mTempChars.clear();
        mbFlag_GetChars = false;
    }

    XmlReader::release(element_name);
    return true;
}

bool NcxFileReader::characters(const xmlChar* const chars,
        const unsigned int length)
{
    if (mbFlag_GetChars == true)
    {
        amis::MediaGroup* p_media_label = NULL;
        amis::TextNode* p_text = NULL;

        if (mpCurrentNode != NULL)
        {
            p_media_label = mpCurrentNode->getLabel();
            if (p_media_label != NULL)
            {
                p_text = p_media_label->getText();

                if (p_text != NULL)
                {
                    const char* tmpchars = XmlReader::transcode(chars);
                    mTempChars.append(tmpchars, length);
                    XmlReader::release(tmpchars);

                    p_text->setTextString(mTempChars);
                }
            }
        }

    }
    return true;
}

