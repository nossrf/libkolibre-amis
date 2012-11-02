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

/**
 * @class amis::NavParse
 *
 * @brief Daisy book parser. Parses the navigation nodes and stores them in a navigation model
 *
 * @author Kolibre (www.kolibre.org)
 *
 * Contact: info@kolibre.org
 *
 */

#include <cctype>
#include <algorithm>

#include "AmisCommon.h"
#include "FilePathTools.h"
#include "TitleAuthorParse.h"

#include "NccFileReader.h"
#include "NcxFileReader.h"
#include "NavFileReader.h"

#include "NavParse.h"

#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.amis
log4cxx::LoggerPtr amisNavParseLog(
        log4cxx::Logger::getLogger("kolibre.amis.navparse"));

using namespace std;
using namespace amis;

NavParse* NavParse::pinstance = 0;

NavParse* NavParse::Instance()
{
    if (pinstance == 0) // is it the first call?
    {
        pinstance = new NavParse; // create sole instance
    }
    return pinstance; // address of sole instance
}

void NavParse::DestroyInstance()
{
    delete pinstance;
}

NavParse::NavParse()
{
    mpNavModel = NULL;
    mpFileReader = NULL;
}

NavParse::~NavParse()
{
    LOG4CXX_DEBUG(amisNavParseLog, "deleting navmodel");

    if (mpNavModel != NULL)
    {
        delete mpNavModel;
        mpNavModel = NULL;
    }

    LOG4CXX_DEBUG(amisNavParseLog, "deleting filereader");

    if (mpFileReader != NULL)
    {
        delete mpFileReader;
        mpFileReader = NULL;
    }
}

/**
 * Open a filepath and parse the model
 *
 * @param filepath Filepath to open
 * @return Returns an amis error on error
 * @return amis::OK if all went well
 */
amis::AmisError NavParse::open(std::string filepath)
{
    amis::AmisError err;

    err.setCode(amis::OK);

    //mFilePath = amis::FilePathTools::getAsLocalFilePath(filepath);
    mFilePath = filepath;

    //determine the file type (ncc or ncx)
    string file_name = amis::FilePathTools::getFileName(filepath);
    string file_ext = amis::FilePathTools::getExtension(filepath);

    //convert the string to lower case before doing a comparison
    std::transform(file_name.begin(), file_name.end(), file_name.begin(),
            (int (*)(int))tolower);

    //create appropriate file reader
    if(file_name.compare(FILENAME_NCC) == 0)
    {
        mpFileReader = new NccFileReader();
        ((NccFileReader*)mpFileReader)->init();
    }

    else if (file_ext.compare(FILE_EXT_NCX) == 0)
    {
        mpFileReader = new NcxFileReader();
        ((NcxFileReader*)mpFileReader)->init();
    }

    else
    {
        err.setCode(amis::NOT_SUPPORTED);
        err.setMessage("Unsupported file type: " + file_ext);
        return err;
    }

    //create a nav model data structure
    mpNavModel = new amis::NavModel();

    //set the title & author info on the model
    amis::TitleAuthorParse title_author_parse;
    err = title_author_parse.openFile(mFilePath);

    if (err.getCode() == amis::OK)
    {
        this->mpNavModel->setDocTitle(title_author_parse.getTitleInfo());
        this->mpNavModel->setDocAuthor(title_author_parse.getAuthorInfo());
    }
    else
        return err;

    //read the file and fill in the data structure
    err = mpFileReader->open(mFilePath, mpNavModel);

    return err;
}

amis::NavModel* NavParse::getNavModel()
{
    return mpNavModel;
}

/**
 * Close the parser
 */
void NavParse::close()
{
    LOG4CXX_DEBUG(amisNavParseLog,
            "closing book, deleting navmodel and filereader");

    if (mpNavModel != NULL)
    {
        delete mpNavModel;
        mpNavModel = NULL;
    }

    if (mpFileReader != NULL)
    {
        delete mpFileReader;
        mpFileReader = NULL;
    }
}
