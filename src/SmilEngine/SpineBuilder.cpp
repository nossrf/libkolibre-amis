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
#include <iostream>
#include <string>
#include <cctype>
#include <algorithm>
#include <cstring>

//PROJECT INCLUDES
#include "AmisCommon.h"
#include "FilePathTools.h"

#include "SmilEngineConstants.h"
#include "Spine.h"
#include "SpineBuilder.h"

#include <XmlReader.h>
#include <XmlAttributes.h>
#include <XmlError.h>

#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.amis
log4cxx::LoggerPtr amisSpineBuilderLog(
        log4cxx::Logger::getLogger("kolibre.amis.spinebuilder"));

using namespace std;

//--------------------------------------------------
//Default constructor
//--------------------------------------------------
SpineBuilder::SpineBuilder()
{
    mError.setSourceModuleName(amis::module_SmilEngine);
}

//--------------------------------------------------
//Destructor
//--------------------------------------------------
SpineBuilder::~SpineBuilder()
{
    this->mOpfManifest.empty();
    this->mOpfSpine.empty();
    this->mSpineSourceFile.empty();

}

//--------------------------------------------------
/*!
 create a spine from an ncc.html, master.smil, or *.opf file
 @pre
 the file must be well-formed XML, otherwise the parse will fail
 */
//--------------------------------------------------
amis::AmisError SpineBuilder::createSpine(Spine* pSpine, string filePath)
{
    //local variables
    amis::ErrorCode pre_build_check;
    string tmp_string;
    XmlReader parser;

    //reset all the variables
    mSpineSourceFile = "";

    mError.setCode(amis::OK);
    mError.setMessage("");
    mError.setFilename(filePath);

    //save the spine source path
    mSpineSourceFile = filePath;

    //save a pointer to the spine that we will populate
    mpSpine = pSpine;

    //make sure we're ready to start building the spine
    pre_build_check = doPreBuildCheck(filePath);

    //if we are ready
    if (pre_build_check == amis::OK)
    {
        tmp_string = amis::FilePathTools::getAsLocalFilePath(filePath);

        parser.setContentHandler(this);
        parser.setErrorHandler(this);

        bool ret = false;
        switch (mFiletype)
        {
        case FILETYPE_NCC:
            LOG4CXX_DEBUG(amisSpineBuilderLog,
                    "Parsing HTML file: " << tmp_string);
            ret = parser.parseHtml(tmp_string.c_str());
            break;
        case FILETYPE_OPF:
        case FILETYPE_SMIL:
            LOG4CXX_DEBUG(amisSpineBuilderLog,
                    "Parsing XML file: " << tmp_string);
            ret = parser.parseXml(tmp_string.c_str());
            break;
        }

        if (!ret)
        {
            const XmlError *e = parser.getLastError();
            if (e)
            {
                LOG4CXX_ERROR(amisSpineBuilderLog,
                        "Error in SpineBuilder: " << e->getMessage());
                mError.loadXmlError(*e);
                mError.setCode(amis::NOT_INITIALIZED);
            }
            else
            {
                LOG4CXX_ERROR(amisSpineBuilderLog,
                        "Unknown error in SpineBuilder");
                mError.setCode(amis::UNDEFINED_ERROR);
            }
        }
    } //end if the pre build check was okay

    //else, the pre-build check was not OK
    else
    {
        mError.setCode(amis::NOT_SUPPORTED);
        mError.setMessage(filePath + " is not a supported file type.");
    }

    return mError;
} //end SpineBuilder::createSpine function

//--------------------------------------------------
/*!
 check to make sure we are ready to start building the spine.  See if the
 file is an NCC, OPF, or Master.SMIL document.
 */
//--------------------------------------------------
amis::ErrorCode SpineBuilder::doPreBuildCheck(string filePath)
{
    //"if, else-if, else" statement to find out what type of file this is and return OK
    //if it is a NCC, OPF, or Master.Smil.
    //otherwise return NOT_SUPPORTED

    //"if, else if, else" statement to check the file name and make sure it is supported
    //no file validation occurs in this function

    string file_name = amis::FilePathTools::getFileName(filePath);
    string file_ext = amis::FilePathTools::getExtension(filePath);

    //LOG4CXX_DEBUG(amisSpineBuilderLog, "filename: " << ile_name << " extension: " << file_ext);

    //convert the string to lower case before doing a comparison
    std::transform(file_name.begin(), file_name.end(), file_name.begin(),
            (int (*)(int))tolower);

            //convert the string to lower case before doing a comparison
std    ::transform(file_ext.begin(), file_ext.end(), file_ext.begin(),
            (int (*)(int))tolower);

if(    file_name.compare(NCC_HTML) == 0 || file_name.compare(NCC_HTM) == 0)
    {
        mFiletype= FILETYPE_NCC;
        return amis::OK;
    }

    else if (file_ext.compare(EXT_OPF) == 0)
    {
        mFiletype= FILETYPE_OPF;
        return amis::OK;
    }

    else if(file_name.compare(MASTER_SMIL) == 0)
    {
        mFiletype = FILETYPE_SMIL;
        return amis::OK;
    }

    else
    {
        return amis::NOT_SUPPORTED;
    }
    //end "if, else if, else" statement to check file name

} //end SpineBuilder::doPreBuildCheck function

//--------------------------------------------------
/*!
 collect data based on the type of file we are processing.
 @note
 This function is long but not too complicated.
 */
//--------------------------------------------------
bool SpineBuilder::startElement(const xmlChar* uri, const xmlChar* localname,
        const xmlChar* qname, const XmlAttributes& attributes)
{
    string tmp_string;
    string file_path;

    ManifestItem manifest_entry;

    const char* element_name = XmlReader::transcode(qname);

    //LOG4CXX_DEBUG(amisSpineBuilderLog, "got: " << string(element_name));

    //convert the node name to lowercase letters
    //std::transform(element_name.begin(), element_name.end(),
    //           element_name.begin(), (int(*)(int))tolower);

    //std::cout << "SpineBuilder::startElement: '" << element_name << "'" << endl;
    //usleep(5000);

    //if we are processing an NCC file and this is an "a" element
    if (mFiletype == FILETYPE_NCC && strcmp(element_name, TAG_A) == 0)
    {
        const xmlChar* txt_HREF = XmlReader::transcode("href");

        const char* href = NULL;
        href = XmlReader::transcode(attributes.getValue(txt_HREF));

        XmlReader::release(txt_HREF);

        if (href != NULL)
        {
            tmp_string.assign(href);

            XmlReader::release(href);

            file_path = amis::FilePathTools::goRelativePath(mSpineSourceFile,
                    "./" + tmp_string);
            mpSpine->addFile(file_path);

        }

    } //end if file is NCC

      //else if filetype is OPF
    else if (mFiletype == FILETYPE_OPF)
    {
        //LOG4CXX_DEBUG(amisSpineBuilderLog, "Parsing OPF file element " << string(element_name));

        //collect all <manifest> items and then <spine> items; 
        //filter #1 with results from #2
        if (strcmp(element_name, TAG_MANIFEST) == 0)
        {
            mbProcessingOpfManifest = true;
        }

        else if (strcmp(element_name, TAG_SPINE) == 0)
        {
            mbProcessingOpfSpine = true;
        }

        else
        {
            //empty
        }

        //if we find an "item" element and we are processing the <manifest> of an opf file
        if (strcmp(element_name, TAG_ITEM) == 0
                && mbProcessingOpfManifest == true)
        {
            //make a new manifest entry
            manifest_entry.mFileHref = "";
            manifest_entry.mId = "";
            string entry_id = "";
            bool b_use_tag = false;

            //get the media type

            const xmlChar* txt_MEDIATYPE = XmlReader::transcode("media-type");

            const char* media_type = NULL;
            media_type = XmlReader::transcode(
                    attributes.getValue(txt_MEDIATYPE));
            XmlReader::release(txt_MEDIATYPE);

            //LOG4CXX_DEBUG(amisSpineBuilderLog, "Got media_type " << string(media_type));

            if (media_type != NULL)
            {
                //if it is a application/smil media item, then note that we will
                //want to use this tag
                if (strcmp(media_type, VAL_APP_SMIL) == 0)
                {
                    b_use_tag = true;
                }

                XmlReader::release(media_type);
            }

            const xmlChar* txt_HREF = XmlReader::transcode("href");
            const char* href = NULL;
            href = XmlReader::transcode(attributes.getValue(txt_HREF));
            XmlReader::release(txt_HREF);

            //LOG4CXX_DEBUG(amisSpineBuilderLog, "Got href " << string(href));

            if (href != NULL)
            {
                tmp_string.assign(href);
                XmlReader::release(href);
                file_path = amis::FilePathTools::goRelativePath(
                        mSpineSourceFile, "./" + tmp_string);
            }

            const xmlChar* txt_ID = XmlReader::transcode("id");
            const char* id = NULL;
            id = XmlReader::transcode(attributes.getValue(txt_ID));
            XmlReader::release(txt_ID);

            if (id != NULL)
            {
                entry_id.assign(id);
                XmlReader::release(id);
            }

            //LOG4CXX_DEBUG(amisSpineBuilderLog, "Got id " << string(id));

            //if this element was identified as being an item of type application/smil
            if (b_use_tag == true)
            {
                //LOG4CXX_DEBUG(amisSpineBuilderLog, "Using entry " << file_path);

                //copy the values to a manifest entry and add it to the manifest list as a full filePath
                manifest_entry.mFileHref = file_path;
                manifest_entry.mId = entry_id;

                mOpfManifest.push_back(manifest_entry);

            }
        } //end if element name is "item"

        //if we find an "itemref" element and we are processing the <spine> of an opf file
        else if (strcmp(element_name, TAG_ITEMREF) == 0
                && mbProcessingOpfSpine == true)
        {
            const xmlChar* txt_IDREF = XmlReader::transcode("idref");

            const char* idref = NULL;
            idref = XmlReader::transcode(attributes.getValue(txt_IDREF));

            XmlReader::release(txt_IDREF);

            if (idref != NULL)
            {
                tmp_string.assign(idref);
                XmlReader::release(idref);
                mOpfSpine.push_back(tmp_string);
            }
        }

        else
        {
            //empty
        }

    } //end if we are processing an OPF file

    //if we are processing a master.smil file and this is a "ref" tag
    else if (mFiletype == FILETYPE_SMIL && strcmp(element_name, TAG_REF) == 0)
    {
        const xmlChar *txtSRC = XmlReader::transcode("src");

        const char* src = NULL;
        src = XmlReader::transcode(attributes.getValue(txtSRC));

        XmlReader::release(txtSRC);

        if (src != NULL)
        {
            tmp_string.assign(src);
            XmlReader::release(src);
            file_path = amis::FilePathTools::goRelativePath(mSpineSourceFile,
                    "./" + tmp_string);
            mpSpine->addFile(file_path);
        }

    } //end else if we are processing a master.smil file

    else
    {
        //Empty
    }

    XmlReader::release(element_name);
    return true;
} //end SpineBuilder::startElement function

//--------------------------------------------------
/*!
 Signal that the list is ready
 */
//--------------------------------------------------
bool SpineBuilder::endDocument()
{
    //if we are working with an OPF file, we need to arrange the Smil references in 
    //default order
    if (mFiletype == FILETYPE_OPF)
    {
        sortOpfSpine();
    }

    if (mpSpine->isEmpty() == true)
    {
        mError.setCode(amis::PARSE_ERROR);
        mError.setMessage("No SMIL files found");
    }

    //mpSpine->printList();
    return true;
}

//--------------------------------------------------
/*!
 note if this marks the end of a manifest or spine section 
 */
//--------------------------------------------------
bool SpineBuilder::endElement(const xmlChar* uri, const xmlChar* localname,
        const xmlChar* qname)
{
    const char* element_name = XmlReader::transcode(qname);

    //if this is a manifest or spine closing tag, set the appropriate flags
    //to indicate that we have finished processing those sections
    if (strcmp(element_name, TAG_MANIFEST) == 0)
    {
        mbProcessingOpfManifest = false;
    }

    else if (strcmp(element_name, TAG_SPINE) == 0)
    {
        mbProcessingOpfSpine = false;
    }

    else
    {
        //empty
    }

    XmlReader::release(element_name);
    return true;
}

//--------------------------------------------------
//(SAX Event) error
//--------------------------------------------------
bool SpineBuilder::error(const XmlError& e)
{
    //ignore it, it's non-fatal
    return true;
}

//--------------------------------------------------
/*!
 record a parse error with Xmlreader' message
 */
//--------------------------------------------------
bool SpineBuilder::fatalError(const XmlError& e)
{
    mError.loadXmlError(e);
    return false;
}

//--------------------------------------------------
//(SAX Event) warning
//--------------------------------------------------
bool SpineBuilder::warning(const XmlError& e)
{
    //ignore this, it's non-fatal
    return true;
}

//--------------------------------------------------
/*!
 sort the data collected from the opf file. 	this function sorts the manifest 
 list based on the ids in the opf spine list. the id's in the opf spine list 
 are in the correct reading order.
 */
//--------------------------------------------------
void SpineBuilder::sortOpfSpine()
{
    //local variables
    unsigned int i;
    unsigned int j;
    string search_id;

    //LOG4CXX_DEBUG(amisSpineBuilderLog, "Sorting Opf spine");

    //for-loop through the opf spine list
    for (i = 0; i < mOpfSpine.size(); i++)
    {
        search_id = mOpfSpine[i];

        //for-loop through the manifest list
        for (j = 0; j < mOpfManifest.size(); j++)
        {

            //LOG4CXX_DEBUG(amisSpineBuilderLog, "Searching for id " << search_id << " in " << mOpfManifest[j].mId);

            //if the ids match, then add this Smil file path to our master spine list
            if (mOpfManifest[j].mId == search_id)
            {

                mpSpine->addFile(mOpfManifest[j].mFileHref);
                break;
            }
        } //end for-loop through manifest

    } //end for-loop through opf spine list

}

//--------------------------------------------------
//(SAX Event) start document
//--------------------------------------------------
bool SpineBuilder::startDocument()
{
    //Empty
    return true;
}
