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

#ifndef AMISMEDIA_H
#define AMISMEDIA_H

#include "AmisCommon.h"
#include <string>
#include <vector>

namespace amis
{
/**
 * @brief Media node is the base class for specific media objects
 */
class AMISCOMMON_API MediaNode //amis::MediaNode
{
public:
    //!default constructor
    MediaNode();
    //!virtual destructor
    virtual ~MediaNode() = 0;

    //!set the id
    void setId(std::string id);
    //!set the class name
    void setClass(std::string className);
    //!set the src
    void setSrc(std::string src);
    //!set the language code
    void setLangCode(std::string langCode);
    //!set the source module name
    void setSourceModuleName(std::string sourceModuleName);
    //!set the region id
    void setRegionId(std::string regionId);
    //!set the href
    void setHref(std::string href);
    //!set the media type
    void setMediaType(std::string type);
    //!set the media node type (amis-specific property)
    void setMediaNodeType(amis::MediaNodeType);

    //!get the id
    std::string getId();
    //!get the class name
    std::string getClass();
    //!get the src
    std::string getSrc();
    //!get the language code
    std::string getLangCode();
    //!get the source module name
    std::string getSourceModuleName();
    //!get the region id
    std::string getRegionId();
    //!get the href
    std::string getHref();
    //!get the media type
    std::string getMediaType();
    //!get the media node type (amis-specific property)
    amis::MediaNodeType getMediaNodeType();

private:
    //!the id
    std::string mId;
    //!the class name
    std::string mClass;
    //!the src
    std::string mSrc;
    //!the source module name (where this node came from)
    std::string mSourceModule;
    //!the region id
    /*!
     @todo: move this to SmilMediaGroup in the smil engine?
     */
    std::string mRegionId;
    //!the href
    std::string mHref;
    //!the media tpe
    std::string mMediaType;
    //!the language code
    std::string mLangCode;
    MediaNodeType mMediaNodeType;

};

/**
 * @brief A text node is a type of MediaNode
 *
 * @details
 *  a text node represents raw text or a text reference (via MediaNode::href).
 */
class AMISCOMMON_API TextNode: public amis::MediaNode
{

public:
    //!default constructor
    TextNode();
    //!destructor
    ~TextNode();

    //!set the text string
    void setTextString(std::string text);
    //!set the language direction
    void setLangDir(TextDirection direction);

    //!get the text string
    std::string getTextString();
    //!get the language direction
    TextDirection getLangDir();

private:
    //!the raw text string
    std::string mText;
    //!the language direction
    TextDirection mLangDir;
};

/**
 * @brief Audio node is a type of MediaNode
 */
class AMISCOMMON_API AudioNode: public amis::MediaNode
{
public:
    //!default constructor
    AudioNode();
    //destructor
    ~AudioNode();

    //!set clip begin time
    void setClipBegin(const std::string clipBegin);
    //!set clip end time
    void setClipEnd(const std::string clipEnd);

    //!get clip begin time
    const std::string getClipBegin();
    //!get clip end time
    const std::string getClipEnd();

    //@todo: eventually put this on all MediaNode-derived objects
    AudioNode* copySelf();

private:
    //!clip begin
    std::string mClipBegin;
    //!clip end
    std::string mClipEnd;
};

//!Image node is a type of MediaNode
class AMISCOMMON_API ImageNode: public amis::MediaNode
{
public:
    //!default constructor
    ImageNode();
    //!destructor
    ~ImageNode();
};

/**
 *
 * @brief Media group represents a group of media nodes to be rendered in parallel
 *
 * @details
 * The reason for having multiple audio nodes is if a sequence needs to be formed
 * for example to prepend a media group with an audio resource label
 */
class AMISCOMMON_API MediaGroup
{
public:
    //!default constructor
    MediaGroup();
    //!destructor
    ~MediaGroup();

    //!destroy the node data pointed to by this media group
    void destroyContents();

    //!set the text node pointer
    void setText(TextNode* pTextNode);
    //!set the image node pointer
    void setImage(ImageNode* pImageNode);
    //!add an audio node pointer
    void addAudioClip(AudioNode* pAudioNode);

    //!does this group have text?
    bool hasText();
    //!does this group have audio?
    bool hasAudio();
    //!does this group have an image?
    bool hasImage();

    //!get the text node pointer
    TextNode* getText();
    //!get the image node pointer
    ImageNode* getImage();
    //!get the audio node pointer at the given index
    AudioNode* getAudio(unsigned int index);

    //!get the number of audio clips
    unsigned int getNumberOfAudioClips();

    //!get the id of this media group (could be SMIL time container id)
    std::string getId();

    //!set the id of this media group
    void setId(std::string);

private:
    //!the text node pointer
    TextNode* mpTextNode;
    //!the image node pointer
    ImageNode* mpImageNode;
    //!the audio node pointer collection
    std::vector<AudioNode*> mpAudioNodes;
    //!the id of this media group
    std::string mId;
};

}
#endif
