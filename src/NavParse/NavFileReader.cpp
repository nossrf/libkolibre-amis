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
#include "AmisCommon.h"
#include "CustomTest.h"

#include "NavFileReader.h"
#include <XmlReader.h>
#include <XmlAttributes.h>
#include <XmlError.h>

#include <iostream>
#include <fstream>
#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.amis
log4cxx::LoggerPtr amisNavFileReadLog(
        log4cxx::Logger::getLogger("kolibre.amis.navfilereader"));

NavFileReader::NavFileReader()
{
    mError.setSourceModuleName(amis::module_NavEngine);
}

NavFileReader::~NavFileReader()
{
}

amis::AmisError NavFileReader::open(std::string filepath, amis::NavModel* pModel)
{
    LOG4CXX_DEBUG(amisNavFileReadLog, "NavFileReader opening: " << filepath);

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
            LOG4CXX_ERROR(amisNavFileReadLog,
                    "Error in NavFileReader: " << e->getMessage());
            mError.loadXmlError(*e);
        }
        else
        {
            LOG4CXX_ERROR(amisNavFileReadLog, "Unknown error in NavFileReader");
            mError.setCode(amis::UNDEFINED_ERROR);
        }
    }

    return mError;
}

bool NavFileReader::startDocument()
{
    return true;
}

bool NavFileReader::endDocument()
{
    //printf("end of document\n");
    return true;
}

bool NavFileReader::error(const XmlError& e)
{
    //ignore this, it's non-fatal
    return true;
}

bool NavFileReader::fatalError(const XmlError& e)
{
    mError.loadXmlError(e);
    return false;
}

bool NavFileReader::warning(const XmlError& e)
{
    //ignore, it's non-fatal
    return true;
}

void NavFileReader::addCustomTest(std::string id, bool override, bool defaultState,
		std::string bookStruct)
{

    amis::CustomTest* p_temp;
    bool b_found = false;

    //make sure it doesn't already exist
    for (unsigned int i = 0; i < mpNavModel->getNumberOfCustomTests(); i++)
    {
        p_temp = mpNavModel->getCustomTest(i);
        if (id.compare(p_temp->getId()) == 0)
        {
            b_found = true;
            break;
        }
    }

    if (b_found == false)
    {
        amis::CustomTest* p_custom_test;
        p_custom_test = new amis::CustomTest();

        p_custom_test->setId(id);
        p_custom_test->setOverride(override);
        p_custom_test->setDefaultState(defaultState);
        p_custom_test->setBookStruct(bookStruct);
        p_custom_test->setCurrentState(defaultState);

        mpNavModel->addCustomTest(p_custom_test);
    }

}
