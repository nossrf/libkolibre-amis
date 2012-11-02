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

#include "FileSearch.h"
#include "TitleAuthorParse.h"

#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <algorithm>

using namespace std;

FileSearch::FileSearch()
{
    mb_flagRecurse = true;
}

FileSearch::~FileSearch()
{
    CleanUpLastSearchResults();
}

int FileSearch::startSearch(string path, bool getBookInfo)
{
    mbRetrieveBookInfo = getBookInfo;

    CleanUpLastSearchResults();

    //call the search routine
    //cerr << "Searching '" << path << "'" << endl;
    int files_found = RecursiveSearch(path);

    cerr << "Found in total, " << files_found << " files" << endl;
    return files_found;

}

BookInfo* FileSearch::getBookInfo(int i)
{
    if (i >= 0 && i < mBookList.size() && mBookList.size() != 0)
    {
        //cerr << "returning bookinfo with index " << i << endl;
        return mBookList[i];
    }
    else
    {
        return NULL;
    }
}

//use this function to get number of searchresults
int FileSearch::getNumberOfItems()
{
    // Return the number of books in the list
    return mBookList.size();
}

//use this function to get a path if you weren't searching for books
string FileSearch::getFilePath(int i)
{
    if (i >= 0 && i < mFileList.size() && mFileList.size() != 0)
    {
        return mFileList[i];
    }
    else
    {
        return "";
    }
}

void FileSearch::setRecursive(bool recurse)
{
    mb_flagRecurse = recurse;
}

/*!
 search criteria is defined as a desired substring of a filename
 so to find all the opf files, just ask for .opf.  not *.opf.
 */
void FileSearch::addSearchCriteria(string new_criteria)
{
    mCriteria.push_back(new_criteria);
}

void FileSearch::clearSearchCriteria()
{
    mCriteria.clear();
}

int FileSearch::RecursiveSearch(string path)
{

    /*	string msg;
     msg = _T("Searching ");
     msg += path;

     AfxMessageBox(msg);*/

    //cerr << "Searching: '" << path << "'" <<endl;
    DIR *dp;
    int status;
    struct dirent *resultp;
    struct stat statbuf;

    dp = opendir(path.c_str());

    if (dp != NULL)
    {
        string filename;

        amis::TitleAuthorParse* title_parser;
        title_parser = new amis::TitleAuthorParse();

        int files_found = 0;

        do
        {

            resultp = readdir(dp);

            if (resultp == NULL)
            {
                //cerr << "resultp == NULL, breaking" << endl;
                break;
            }

            filename = resultp->d_name;

            string filepath = path;
            filepath.append("/");
            filepath.append(resultp->d_name);

            if (stat(filepath.c_str(), &statbuf) == -1)
            {
                cerr << "Error stat-ting '" << filepath << "'" << endl;
                continue;
            }

            if (S_ISDIR(statbuf.st_mode)
                    && (filename.compare(".") == 0
                            || filename.compare("..") == 0))
            {
                // We do not want the .. or . directories
                //cerr << "Got dots directory: '" << filename << "'" << endl;
                continue;
            }

            // if it's a directory, recursively search it
            if (S_ISDIR(statbuf.st_mode) && mb_flagRecurse == true)
            {
                files_found += RecursiveSearch(filepath);

            }
            else
            { //if(S_IFREG(statbuf.st_mode) || statbuf.st_mode == S_IFLNK) { 

                // If it is a file, check the criteria
                //cerr << "Found a file named: '" << filename << "'" << endl;

                //create a lowercase version of the filename to compare to
                string lc_filename = filename;

                std::transform(lc_filename.begin(), lc_filename.end(),
                        lc_filename.begin(), (int (*)(int))tolower);

bool                bfound = false;

                string lc_criteria;

                for (int i = 0; i < mCriteria.size(); i++)
                {

                    lc_criteria = mCriteria[i].c_str();
                    //convert the criteria to lower case
                    std::transform(lc_criteria.begin(), lc_criteria.end(),
                            lc_criteria.begin(), (int (*)(int))tolower);

if(                    filename.find(lc_criteria, 0) != string::npos)
                    {
                        //cerr << "Found '" << lc_criteria << "' in '"
                        //   << path << "/" << lc_filename << "'" << endl;
                        bfound = true;
                        break;
                    } /*else {
                     cerr << "Did not find '" << lc_criteria << "' in '"
                     << path << "/" << lc_filename << "'" << endl;
                     }*/
                }

                //if there are no search criteria, then any file in this dir will be ok as a result
                if (mCriteria.size() == 0)
                {
                    bfound = true;
                }

                if (bfound == true)
                {
                    files_found++;

                    string filePath;

                    filePath = path;
                    filePath.append("/");
                    filePath.append(resultp->d_name);

                    if (mbRetrieveBookInfo == true)
                    {
                        //cerr << "Getting book info for " << filePath << endl;
                        BookInfo* book_info;
                        book_info = new BookInfo;
                        book_info->mpTitle = NULL;

                        book_info->mFilePath = filePath;
                        amis::AmisError err = title_parser->openFile(
                                book_info->mFilePath);

                        if (err.getCode() == amis::OK)
                        {
                            book_info->mpTitle = title_parser->getTitleInfo();
                        }
                        else
                        {
                            cerr << "Could not load title info for book "
                                    << book_info->mFilePath << endl;
                        }

                        //cerr << "Finished getting book info" << endl;

                        mBookList.push_back(book_info);
                        //cerr << "Pushed back book info" << endl;
                    }

                    //save the file path
                    mFileList.push_back(filePath);
                    //cerr << "Pushed back file path" << endl;
                }

            } /*else {
             cerr << "Unknown: '" << resultp->d_name << "'" 
             << " Type: " << resultp->d_type << endl;
             }*/

        } while (resultp != NULL);
        //cerr << "Deleting title parser" << endl;
        delete title_parser;
    }
    else
    {
        cerr << "Could not open directory: '" << path << "'" << endl;
    }

    //cerr << "Closing directory" << endl;
    closedir(dp);

    //cerr << "mFileList.size() = " << mFileList.size() << endl;

    return mFileList.size();
}

void FileSearch::CleanUpLastSearchResults()
{
    BookInfo* p_temp;
    amis::MediaGroup* p_mg;

    int len = mBookList.size();

    for (int i = len - 1; i >= 0; i--)
    {
        p_temp = NULL;
        p_mg = NULL;

        p_temp = mBookList[i];

        if (p_temp != NULL)
        {
            p_mg = p_temp->mpTitle;

            if (p_mg != NULL)
            {
                p_mg->destroyContents();

                delete p_mg;
            }

            delete p_temp;
        }

        mBookList.pop_back();
    }

    mFileList.clear();

}
