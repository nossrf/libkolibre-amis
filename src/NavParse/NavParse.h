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

//Nav Parse class
#ifndef NAVPARSE_H
#define NAVPARSE_H

#include <string> 

#include "AmisError.h"

#include "NavModel.h"

class NavFileReader;

//DLL stuff
#ifdef WIN32
#ifdef AMIS_DLL
#define NAVPARSE_API __declspec(dllexport)
#else
#define NAVPARSE_API __declspec(dllimport)
#endif
#else 
#define NAVPARSE_API
#endif

#define FILENAME_NCC	"ncc.html"
#define FILE_EXT_NCX	"ncx"

namespace amis {

class NAVPARSE_API NavParse
{
public:
    static NavParse* Instance();
    void DestroyInstance();
    ~NavParse();

    amis::AmisError open(std::string);
    void close();

    amis::NavModel* getNavModel();

protected:
    NavParse();

private:
    amis::NavModel* mpNavModel;
    NavFileReader* mpFileReader;
    std::string mFilePath;

    static NavParse* pinstance;

};

}
#endif
