/*
 AmisCommon: common system objects and utility routines

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

//Amis media objects implementation
#include "Media.h"
using namespace std;

/*
 MediaNode class
 */
//--------------------------------------------------
//--------------------------------------------------
amis::MediaNode::MediaNode()
{
    mId = "";
    mClass = "";
    mSrc = "";
    mSourceModule = "";
    mRegionId = "";
    mHref = "";
    mMediaType = "";
}

//--------------------------------------------------
//--------------------------------------------------
amis::MediaNode::~MediaNode()
{
    mId = "";
    mClass = "";
    mSrc = "";
    mSourceModule = "";
    mRegionId = "";
    mHref = "";
    mMediaType = "";

}

//--------------------------------------------------
//--------------------------------------------------
void amis::MediaNode::setId(std::string id)
{
    mId = id;
}

//--------------------------------------------------
//--------------------------------------------------
void amis::MediaNode::setClass(std::string className)
{
    mClass = className;
}

//--------------------------------------------------
//--------------------------------------------------
void amis::MediaNode::setSrc(std::string src)
{
    mSrc = src;
}

//--------------------------------------------------
//--------------------------------------------------
void amis::MediaNode::setSourceModuleName(std::string sourceModuleName)
{
    mSourceModule = sourceModuleName;
}

//--------------------------------------------------
//--------------------------------------------------
void amis::MediaNode::setRegionId(std::string regionId)
{
    mRegionId = regionId;
}

//--------------------------------------------------
//--------------------------------------------------
void amis::MediaNode::setHref(std::string href)
{
    mHref = href;
}

//--------------------------------------------------
//--------------------------------------------------
void amis::MediaNode::setMediaType(std::string type)
{
    mMediaType = type;
}

//--------------------------------------------------
//--------------------------------------------------
void amis::MediaNode::setLangCode(std::string langcode)
{
    mLangCode = langcode;
}

//--------------------------------------------------
//--------------------------------------------------
std::string amis::MediaNode::getId()
{
    return mId;
}

//--------------------------------------------------
//--------------------------------------------------
std::string amis::MediaNode::getClass()
{
    return mClass;
}

//--------------------------------------------------
//--------------------------------------------------
std::string amis::MediaNode::getSrc()
{
    return mSrc;
}

//--------------------------------------------------
//--------------------------------------------------
std::string amis::MediaNode::getSourceModuleName()
{
    return mSourceModule;
}

//--------------------------------------------------
//--------------------------------------------------
std::string amis::MediaNode::getRegionId()
{
    return mRegionId;
}

//--------------------------------------------------
//--------------------------------------------------
std::string amis::MediaNode::getHref()
{
    return mHref;
}

//--------------------------------------------------
//--------------------------------------------------
std::string amis::MediaNode::getMediaType()
{
    return mMediaType;
}

//--------------------------------------------------
//--------------------------------------------------
std::string amis::MediaNode::getLangCode()
{
    return mLangCode;
}

amis::MediaNodeType amis::MediaNode::getMediaNodeType()
{
    return mMediaNodeType;
}

void amis::MediaNode::setMediaNodeType(amis::MediaNodeType mediaNodeType)
{
    mMediaNodeType = mediaNodeType;
}

/*
 TextNode class
 */
//--------------------------------------------------
//--------------------------------------------------
amis::TextNode::TextNode()
{
    setMediaNodeType(amis::TEXT);
    mText.erase();
    mLangDir = amis::LTR;
}

//--------------------------------------------------
//--------------------------------------------------
amis::TextNode::~TextNode()
{
    mText.erase();
}

//--------------------------------------------------
//--------------------------------------------------
void amis::TextNode::setTextString(std::string text)
{
    mText = text;
}

//--------------------------------------------------
//--------------------------------------------------
void amis::TextNode::setLangDir(TextDirection direction)
{
    if (direction == amis::LTR || direction == amis::RTL)
    {
        mLangDir = direction;
    }
    else
    {
        mLangDir = amis::LTR;
    }
}

//--------------------------------------------------
//--------------------------------------------------
std::string amis::TextNode::getTextString()
{
    return mText;
}

//--------------------------------------------------
//--------------------------------------------------
amis::TextDirection amis::TextNode::getLangDir()
{
    return mLangDir;
}

/*
 AudioNode class
 */
//--------------------------------------------------
//--------------------------------------------------
amis::AudioNode::AudioNode()
{
    setMediaNodeType(amis::AUDIO);
    mClipBegin = "";
    mClipEnd = "";
}

//--------------------------------------------------
//--------------------------------------------------
amis::AudioNode::~AudioNode()
{
    mClipBegin = "";
    mClipEnd = "";
}

//--------------------------------------------------
//--------------------------------------------------
void amis::AudioNode::setClipBegin(const std::string clipBegin)
{
    mClipBegin = clipBegin;
}

//--------------------------------------------------
//--------------------------------------------------
void amis::AudioNode::setClipEnd(const std::string clipEnd)
{
    mClipEnd = clipEnd;
}

//--------------------------------------------------
//--------------------------------------------------
const std::string amis::AudioNode::getClipBegin()
{
    return mClipBegin;
}

//--------------------------------------------------
//--------------------------------------------------
const std::string amis::AudioNode::getClipEnd()
{
    return mClipEnd;
}
amis::AudioNode* amis::AudioNode::copySelf()
{
    amis::AudioNode* p_new = new amis::AudioNode();

    p_new->setClass(this->getClass());
    p_new->setClipBegin(this->getClipBegin());
    p_new->setClipEnd(this->getClipEnd());
    p_new->setHref(this->getHref());
    p_new->setId(this->getId());
    p_new->setLangCode(this->getLangCode());
    p_new->setMediaNodeType(this->getMediaNodeType());
    p_new->setMediaType(this->getMediaType());
    p_new->setRegionId(this->getRegionId());
    p_new->setSourceModuleName(this->getSourceModuleName());
    p_new->setSrc(this->getSrc());

    return p_new;
}

/*
 ImageNode class
 */
//--------------------------------------------------
//--------------------------------------------------
amis::ImageNode::ImageNode()
{
    setMediaNodeType(amis::IMAGE);
}

//--------------------------------------------------
//--------------------------------------------------
amis::ImageNode::~ImageNode()
{

}

/*
 MediaGroup class
 */
//--------------------------------------------------
//--------------------------------------------------
amis::MediaGroup::MediaGroup()
{
    mpTextNode = NULL;
    mpImageNode = NULL;
    mpAudioNodes.clear();
}

//--------------------------------------------------
/*!
 destructor does not delete the pointers as they may be referenced elsewhere
 */
//--------------------------------------------------
amis::MediaGroup::~MediaGroup()
{
    mpAudioNodes.clear();
}

//--------------------------------------------------
/*!
 important to call this function if MediaGroup is being deleted and you want 
 the nodes it references deleted as well

 @warning
 this deletes the pointers, so use carefully!
 */
//--------------------------------------------------
void amis::MediaGroup::destroyContents()
{

    if (mpTextNode != NULL)
    {
        delete mpTextNode;
    }

    if (mpImageNode != NULL)
    {
        delete mpImageNode;
    }

    AudioNode* tmp_ptr = NULL;
    unsigned int sz = mpAudioNodes.size();

    for (int i = sz - 1; i >= 0; i--)
    {
        tmp_ptr = NULL;
        tmp_ptr = mpAudioNodes[i];
        mpAudioNodes.pop_back();
        if (tmp_ptr != NULL)
        {
            delete tmp_ptr;
        }
    }

}

//--------------------------------------------------
//--------------------------------------------------
void amis::MediaGroup::setText(TextNode* pTextNode)
{
    mpTextNode = pTextNode;
}

//--------------------------------------------------
//--------------------------------------------------
void amis::MediaGroup::setImage(ImageNode* pImageNode)
{
    mpImageNode = pImageNode;
}

//--------------------------------------------------
//--------------------------------------------------
void amis::MediaGroup::addAudioClip(AudioNode* pAudioNode)
{
    if (pAudioNode != NULL)
    {
        mpAudioNodes.push_back(pAudioNode);
    }
}

//--------------------------------------------------
//--------------------------------------------------
bool amis::MediaGroup::hasText()
{
    if (mpTextNode != NULL)
    {
        return true;
    }
    else
    {
        return false;
    }
}

//--------------------------------------------------
//--------------------------------------------------
bool amis::MediaGroup::hasAudio()
{
    if (mpAudioNodes.size() != 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

//--------------------------------------------------
//--------------------------------------------------
bool amis::MediaGroup::hasImage()
{
    if (mpImageNode != NULL)
    {
        return true;
    }
    else
    {
        return false;
    }
}

//--------------------------------------------------
//--------------------------------------------------
amis::TextNode* amis::MediaGroup::getText()
{
    return mpTextNode;
}

//--------------------------------------------------
//--------------------------------------------------
amis::ImageNode* amis::MediaGroup::getImage()
{
    return mpImageNode;
}

//--------------------------------------------------
//--------------------------------------------------
amis::AudioNode* amis::MediaGroup::getAudio(unsigned int index)
{
    if (index < mpAudioNodes.size())
    {
        return mpAudioNodes[index];
    }
    else
    {
        return NULL;
    }
}

//--------------------------------------------------
//--------------------------------------------------
unsigned int amis::MediaGroup::getNumberOfAudioClips()
{
    return mpAudioNodes.size();
}

void amis::MediaGroup::setId(std::string id)
{
    mId = id;
}

std::string amis::MediaGroup::getId()
{
    return mId;
}
