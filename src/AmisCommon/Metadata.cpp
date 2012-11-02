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

#include "Metadata.h"
#include "MetadataSet.h"

#include <string>
#include <iostream>
//#include "../SmallTools/CommandParse.h"
#include "FilePathTools.h"

#include <algorithm>

using namespace std;
using namespace amis;

Metadata* Metadata::pinstance = 0;

Metadata* Metadata::Instance()
{
    if (pinstance == 0) // is it the first call?
    {
        pinstance = new Metadata; // create sole instance
    }
    return pinstance; // address of sole instance
}

void Metadata::DestroyInstance()
{
    delete pinstance;
}

//--------------------------------------------------
//--------------------------------------------------
Metadata::Metadata()
{
    mbBookIsOpen = false;
    mDataSet = NULL;
}

//--------------------------------------------------
//--------------------------------------------------
Metadata::~Metadata()
{
    if (mDataSet != NULL)
    {
        delete mDataSet;
    }
}

//--------------------------------------------------
//--------------------------------------------------
amis::AmisError Metadata::openFile(string filepath)
{
    // Destroy the old metadata if there is any
    close();

    mDataSet = new MetadataSet();

    AmisError err;
    err = mDataSet->openBookFile(filepath);

    if (err.getCode() == amis::OK)
        mbBookIsOpen = true;

    //cerr<<"done opening"<<endl;

    return err;

}

void Metadata::close()
{
    if (mDataSet != NULL)
    {
        delete mDataSet;
        mDataSet = NULL;
        mbBookIsOpen = false;
    }
}

//--------------------------------------------------
//--------------------------------------------------
string Metadata::getMetadata(string metaname)
{
    string meta_value;

    //convert the search string to lower case
    //Damn book producers keep using uppercase and lowercase characters
    //together
    std::transform(metaname.begin(), metaname.end(), metaname.begin(),
            (int (*)(int))tolower);

    if(mbBookIsOpen == true)
    {
        meta_value = mDataSet->getMetadata(metaname);
    }

    return meta_value;
}

//--------------------------------------------------
//--------------------------------------------------
string Metadata::getChecksum()
{
    string checksum;

    if (mbBookIsOpen == true)
        checksum = mDataSet->getChecksum();

    return checksum;
}
