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

//FilePathTools
#include <string>
#include <map>
#include <iostream>

#include "FilePathTools.h"

#ifdef WIN32
#include <windows.h>
#include <io.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>
#endif


using namespace std;
//! return just the filename and extension
string amis::FilePathTools::getFileName(const string filepath)
{
    string file_name = "";
    string::size_type pos = 0, pos2 = 0;
    //static int substrLen;

    file_name = convertSlashesFwd(filepath);

    pos = file_name.find_last_of('/');
    pos2 = file_name.find_last_of('#');

    if (pos != string::npos)
    {
        if (pos2 != string::npos && pos2 > pos)
        {
            file_name = file_name.substr(pos + 1, pos2 - pos - 1);
        }
        else
        {
            file_name = file_name.substr(pos + 1);
        }
    }
    else
    {
        if (pos2 != string::npos)
        {
            file_name = file_name.substr(0, pos2);
        }
        else
        {
            file_name = filepath;
        }
    }

    return file_name;

}

//! return the target
string amis::FilePathTools::getTarget(const string filepath)
{
    string file_name = filepath;
    string target = "";
    string::size_type pos;

    pos = file_name.find("#");

    //strip the target and save
    if (pos != string::npos)
    {
        //"filename#target"
        if (pos > 0 && pos < file_name.length() - 1)
        {
            target = file_name.substr(pos + 1);
        }
        //filename#  - mysterious pound sign
        else
        {
            target = "";
        }
    }

    return target;
}

//! return the extension
string amis::FilePathTools::getExtension(const string filepath)
{
    string ext = "";
    string file_name = getFileName(filepath);

    string::size_type pos = file_name.find_last_of(".");

    if (pos > 0 && pos < file_name.length() - 1)
    {
        ext = file_name.substr(pos + 1);
    }

    return ext;

}

//!return a string in local path format (windows).  no target is returned.
string amis::FilePathTools::getAsLocalFilePath(const string filepath)
{
    //general idea .. strip off any xyz:// and convert the remainder to backslashes
    //also remove the target if exists

    static string file_path;
    string::size_type pos;

    file_path = convertSlashesFwd(filepath);

    /*pos = file_path.find(":///");

     if (pos != string::npos)
     {
     file_path = file_path.substr(pos+4);
     }

     pos = file_path.find("://");

     if (pos != string::npos)
     {
     file_path = file_path.substr(pos+3);
     }*/

    pos = file_path.find("#");
    if (pos != string::npos)
    {
        file_path = file_path.substr(0, pos);
    }

    //std::cout << "FilePathTools::getAsLocalFilePath: " << filepath << " -> " << file_path << endl;

    return file_path;

}

//! clear the target from the filepath
string amis::FilePathTools::clearTarget(const string filepath)
{
    string file_path = filepath;
    string::size_type pos;

    pos = file_path.find("#");

    //strip the target and discard
    if (pos != string::npos)
    {
        //"filename#target"
        if (pos > 0)
        {
            file_path = file_path.substr(0, pos);
        }

    }

    //std::cout << "FilePathTools::clearTarget: " << filepath << " -> " << file_path << endl;

    return file_path;
}

//!navigate based on a relative path beginning with ./, ../, or /
string amis::FilePathTools::goRelativePath(const string filepath,
        string relativePath)
{

    string file_path;
    string rel_path;
    string rel_instr;
    string path_append;

    string::size_type pos;
    string::size_type i;
    string::size_type len;

    file_path = convertSlashesFwd(filepath);
    rel_path = convertSlashesFwd(relativePath);

    pos = rel_path.find(":/");
    if (pos != string::npos)
    {
        //the relative path is not relative, it contains a drive name
        return rel_path;
    }

    //strip any file name off the file path along with the last slash
    pos = file_path.find_last_of("/");
    if (pos != string::npos)
    {
        file_path = file_path.substr(0, pos);
    }

    //parse the relative path slashes into an array
    len = rel_path.length();
    for (i = 0; i < len; i++)
    {
        pos = rel_path.find("/");
        if (pos != string::npos) // && pos < rel_path.length() - 1)
        {
            //rel_path_array.push_back(rel_path.substr(0, pos));
            rel_instr = rel_path.substr(0, pos + 1);
            rel_path = rel_path.substr(pos + 1);
            len -= pos;
        }
        else
        {
            path_append = rel_path;
            break;
        }

        //new code
        if (rel_instr.compare("/") == 0)
        {
            //do nothing
        }
        else if (rel_instr.compare("./") == 0)
        {
            //do nothing
        }
        else if (rel_instr.compare("../") == 0)
        {
            //go up a folder
            pos = file_path.find_last_of("/");

            if (pos != string::npos)
            {
                file_path = file_path.substr(0, pos);
            }
        }
        else
        {
            //make sure there is a slash somewhere between the two strings
            if (file_path[file_path.length() - 1] != '/'
                    && rel_instr.length() > 0 && rel_instr[0] != '/')
            {
                file_path += "/";
            }
            file_path += rel_instr;
        }
    }

    //make sure there is a slash somewhere between the two strings
    if (file_path.length() > 0 && file_path[file_path.length() - 1] != '/'
            && path_append.length() > 0 && path_append[0] != '/')
    {
        file_path += "/";
    }
    //special case to ensure getting c:/ instead of c:
    if (file_path.length() > 0 && file_path[file_path.length() - 1] == ':')
    {
        file_path += '/';
    }

    file_path.append(path_append);

    return file_path;

}

//--------------------------------------------------
//--------------------------------------------------
string amis::FilePathTools::convertSlashesFwd(const string filepath)
{
    string file_path = filepath;
    string::size_type pos = 0;

    while ((pos = file_path.find_first_of("\\", pos)) != string::npos)
        file_path.replace(pos, 1, "/");

    return file_path;

    /* SLOOW CODE
     //make all the slashes go forward
     for (unsigned int i=0; i<file_path.length(); i++)
     {
     if (file_path.substr(i, 1).compare("\\")==0)
     {
     file_path.replace(i, 1, "/");
     }
     }

     return file_path;
     */
}

//--------------------------------------------------
//--------------------------------------------------
string amis::FilePathTools::convertSlashesBkw(const string filepath)
{
    string file_path = filepath;
    string::size_type pos = 0;

    while ((pos = file_path.find_first_of("/", pos)) != string::npos)
        file_path.replace(pos, 1, "\\");

    return file_path;

    /* SLOOW CODE
     //make all the slashes go forward
     for (i=0; i<file_path.length(); i++)
     {
     if (file_path.substr(i, 1).compare("/")==0)
     {
     file_path.replace(i, 1, "\\");
     }
     }

     return file_path;
     */
}

string amis::FilePathTools::getParentDirectory(const string filepath)
{
    string file_path = convertSlashesFwd(filepath);

    //if filepath is just a path of folders (ends in a slash), remove the slash
    if (file_path[file_path.length() - 1] == '/')
    {
        file_path = file_path.substr(0, file_path.length() - 1);
    }

    //strip any file/folder name off the file path
    string::size_type pos = file_path.find_last_of('/');
    if (pos != string::npos)
    {
        file_path = file_path.substr(0, pos);
    }

    return file_path;
}

bool amis::FilePathTools::isDirectory(const string filepath)
{
    bool isDirectory = false;
#ifdef WIN32
    string _path = convertSlashesBkw(filepath);

    int numchars = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, _path.c_str(), -1, NULL, 0);
    if(numchars) {
        wchar_t *_wpath = (wchar_t*) malloc(numchars * sizeof(wchar_t));
        if (MultiByteToWideChar(CP_UTF8, 0, _path.c_str(), -1, _wpath, numchars) != 0)
        {
            DWORD dwAttrib = GetFileAttributesW(_wpath);
            isDirectory = (dwAttrib != INVALID_FILE_ATTRIBUTES &&
                    (dwAttrib & FILE_ATTRIBUTE_DIRECTORY) );
        }
        free(_wpath);
    }
#else
    isDirectory = fstream( filepath.c_str(), fstream::in ).is_open();
#endif

    return isDirectory;
}


bool amis::FilePathTools::createDirectory(const string filepath)
{
    bool success = false;
#ifdef WIN32
    string _path = convertSlashesBkw(filepath);

    //if filepath is just a path of folders (ends in a slash), remove the slash
    if (_path[_path.length() - 1] == '\\') {
        _path = _path.substr(0, _path.length() - 1);
    }

    int numchars = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, _path.c_str(), -1, NULL, 0);
    if(numchars) {
        wchar_t *_wpath = (wchar_t*) malloc(numchars * sizeof(wchar_t));
        if (MultiByteToWideChar(CP_UTF8, 0, _path.c_str(), -1, _wpath, numchars) != 0)
        {
            success = CreateDirectoryW(_wpath, NULL);
        }
        free(_wpath);
    }
#else
    string _path = convertSlashesFwd(filepath);

    //if filepath is just a path of folders (ends in a slash), remove the slash
    if (_path[_path.length() - 1] == '/') {
        _path = _path.substr(0, _path.length() - 1);
    }

    success = (mkdir(_path.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == 0);
#endif
    return success;
}

bool amis::FilePathTools::fileIsReadable(const string filepath)
{
    bool isReadable = false;
#ifdef WIN32
    string _path = convertSlashesBkw(filepath);

    int numchars = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, _path.c_str(), -1, NULL, 0);
    if(numchars) {
        wchar_t *_wpath = (wchar_t*) malloc(numchars * sizeof(wchar_t));
        if (MultiByteToWideChar(CP_UTF8, 0, _path.c_str(), -1, _wpath, numchars) != 0)
        {
            DWORD dwAttrib = GetFileAttributesW(_wpath);
            isReadable = (dwAttrib != INVALID_FILE_ATTRIBUTES);
        }
        free(_wpath);
    }
#else
    isReadable = fstream( filepath.c_str(), fstream::in ).is_open();
#endif
    return isReadable;
}

bool amis::FilePathTools::fileIsWriteable(const string filepath)
{
    bool isWriteable = false;
#ifdef WIN32
    string _path = convertSlashesBkw(filepath);

    int numchars = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, _path.c_str(), -1, NULL, 0);
    if(numchars) {
        wchar_t *_wpath = (wchar_t*) malloc(numchars * sizeof(wchar_t));
        if (MultiByteToWideChar(CP_UTF8, 0, _path.c_str(), -1, _wpath, numchars) != 0)
        {
            DWORD dwAttrib = GetFileAttributesW(_wpath);
            isWriteable = (dwAttrib != INVALID_FILE_ATTRIBUTES &&
                    !(dwAttrib & FILE_ATTRIBUTE_READONLY) );
        }
        free(_wpath);
    }
#else
    isWriteable =  fstream( filepath.c_str(), fstream::out ).is_open();
#endif
    return isWriteable;
}

bool amis::FilePathTools::renameFile(const string oldname, const string newname)
{
    bool success = false;
#ifdef WIN32
    string _oldname = convertSlashesBkw(oldname);
    wchar_t *_woldname = NULL;
    string _newname = convertSlashesBkw(newname);
    wchar_t *_wnewname = NULL;

    int numchars;

    numchars = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, _oldname.c_str(), -1, NULL, 0);
    if(numchars) {
        _woldname = (wchar_t*) malloc(numchars * sizeof(wchar_t));
        if (MultiByteToWideChar(CP_UTF8, 0, _oldname.c_str(), -1, _woldname, numchars) == 0)
        {
            free(_woldname);
            return false;
        }
    }

    numchars = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, _newname.c_str(), -1, NULL, 0);
    if(numchars) {
        _wnewname = (wchar_t*) malloc(numchars * sizeof(wchar_t));
        if (MultiByteToWideChar(CP_UTF8, 0, _oldname.c_str(), -1, _wnewname, numchars) == 0)
        {
            free(_woldname);
            free(_wnewname);
            return false;
        }
    }

    success = MoveFileW(_woldname, _wnewname);

    free(_woldname);
    free(_wnewname);
#else
    success = (rename( oldname.c_str() , newname.c_str() ) == 0);
#endif
    return success;
}
