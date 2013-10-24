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
 
#ifndef AMISCOMMON_H
#define AMISCOMMON_H

//SYSTEM INCLUDES
#include <string>

#ifdef WIN32

#ifdef _MSC_VER
#pragma warning(disable : 4251)
#pragma warning(disable : 4786)
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4251)
#pragma warning(disable : 4786)
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4251)
#pragma warning(disable : 4786)
#endif

//DLL stuff
#ifdef AMIS_DLL
#define AMISCOMMON_API __declspec(dllexport)
#else
#define AMISCOMMON_API __declspec(dllimport)
#endif
#else // not WIN32
#define AMISCOMMON_API
#endif // WIN32

//DLL stuff
#ifdef WIN32
#ifdef AMIS_DLL
#define SMILENGINE_API __declspec(dllexport)
#else
#define SMILENGINE_API __declspec(dllimport)
#endif
#else
#define SMILENGINE_API
#endif
//!amis namespace declaration
/*!
 all these common classes are organized under the "amis" namespace.  to refer to them
 programatically, use the syntax amis::ClassName.
 */
namespace amis
{
//media objects
class MediaNode;
class TextNode;
class ImageNode;
class AudioNode;
class MediaGroup;

//the error object
class AmisError;

//custom test
class CustomTest;

//meta data classes
class Metadata;
class MetadataSet;
struct MetaItem;

//utility for file path processing
class FilePathTools;

//classes to extract title and author media info from a book
class OpfItemExtract;
class TitleAuthorParse;
class SmilAudioExtract;

//!text direction: "left to right" or "right to left"
enum TextDirection
{
    LTR, RTL
};

//!Media node types
enum MediaNodeType
{
    AUDIO, TEXT, IMAGE
};

//!identifies the smil engine module
static std::string module_SmilEngine = "SmilEngine";
//!identifies the nav engine module
static std::string module_NavEngine = "NavEngine";
//!identifies the amiscommon module
static std::string module_AmisCommon = "AmisCommon";
//!identifies the amiscommon module
static std::string module_DaisyHandler = "DaisyHandler";

//!Error Codes
/*!
 Modules communicate these error codes to the calling application
 */
enum ErrorCode
{
    OK = 1,
    UNDEFINED_ERROR = -100,
    AT_BEGINNING = -200,
    AT_END = -300,
    NOT_FOUND = -400,
    NOT_SUPPORTED = -500,
    PARSE_ERROR = -600,
    PERMISSION_ERROR = -700,
    IO_ERROR = -800,
    NOT_INITIALIZED = -900,
};

}

#endif
