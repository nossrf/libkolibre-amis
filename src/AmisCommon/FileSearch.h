/*
 Copyright (C) 2012 Kolibre
 
 This file is part of Kolibre-amis.
 
 Kolibre-amis is free software: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation, either version 2.1 of the License, or
 (at your option) any later version.
 
 Kolibre-amis is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public License
 along with Kolibre-amis.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef FILESEARCH_H
#define FILESEARCH_H

#include "Media.h"
#include "AmisError.h"
#include <iostream>
#include <string>
#include <vector>

struct BookInfo
{
    amis::MediaGroup* mpTitle;
    std::string mFilePath;
};

class FileSearch
{
public:
    FileSearch();
    ~FileSearch();

    int startSearch(std::string, bool);
    void stopSearch();
    void CleanUpLastSearchResults();
    void clearSearchCriteria();
    void addSearchCriteria(std::string);
    void setRecursive(bool);
    int getNumberOfItems();

    BookInfo* getBookInfo(int);
    std::string getFilePath(int);

private:

    int RecursiveSearch(std::string);
    std::vector<BookInfo*> mBookList;
    bool mb_flagRecurse;
    std::vector<std::string> mCriteria;
    bool mbRetrieveBookInfo;
    std::vector<std::string> mFileList;

};

#endif
