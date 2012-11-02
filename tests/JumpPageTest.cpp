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

#include <iostream>
#include <assert.h>
#include <unistd.h>
#include "DaisyHandler.h"
#include "setup_logging.h"

using namespace amis;

int main(int argc, char *argv[])
{
    if(argc < 2) {
        std::cout << "Please specify an ncc.html file on the command line" << std::endl;
        return -1;
    }

    setup_logging();

    if(not DaisyHandler::Instance()->openBook(argv[1])) {
        std::cout << "Unable to open requested file: " << argv[1] << std::endl;;
        return 1;
    }

    while(DaisyHandler::Instance()->getState() == DaisyHandler::HANDLER_OPENING) {
        usleep(1000);
    }

    if(DaisyHandler::Instance()->getState() != DaisyHandler::HANDLER_OPEN) {
        std::cout << "The file " << argv[1] << " could not be open, please check for errors" << std::endl;
        DaisyHandler::Instance()->DestroyInstance();
        return 1;
    }

    // setup the book, returns true if the book has a lastmark
    bool lastmark = DaisyHandler::Instance()->setupBook();

    DaisyHandler::BookInfo *bookInfo = DaisyHandler::Instance()->getBookInfo();

    if (not bookInfo->hasPages) {
        std::cout << "This book " << argv[1] << " has no pages" << std::endl;
        DaisyHandler::Instance()->closeBook();
        DaisyHandler::Instance()->DestroyInstance();
        return 0;
    }

    // set navi level to PAGE
    while(DaisyHandler::Instance()->getNaviLevel() >= DaisyHandler::PHRASE)
        DaisyHandler::Instance()->increaseNaviLevel();

    std::cout << "***********************************************************" << std::endl;
    std::cout << "Book has: ";
    std::cout << bookInfo->mFrontPages << " front pages, ";
    std::cout << bookInfo->mNormalPages << " normal pages, ";
    std::cout << bookInfo->mSpecialPages << " special pages" << std::endl;
    std::cout << "first page is " << bookInfo->mMinPageNum << " and last page is " << bookInfo->mMaxPageNum << std::endl;
    std::cout << "***********************************************************" << std::endl;

    std::string pageBefore, pageAfter;

    // go to first page
    assert(DaisyHandler::Instance()->firstPage());

    // previousPage should stay on current page
    pageBefore = DaisyHandler::Instance()->getCurrentPage();
    assert(DaisyHandler::Instance()->previousPage()); // this call should succeed
    pageAfter = DaisyHandler::Instance()->getCurrentPage();
    assert(!DaisyHandler::Instance()->previousPage()); // this call should fail
    pageAfter = DaisyHandler::Instance()->getCurrentPage();
    assert(pageBefore == pageAfter);
    assert(DaisyHandler::Instance()->getLastError().getCode() == amis::AT_BEGINNING);

    // go to the last page
    assert(DaisyHandler::Instance()->lastPage());

    // nextPage should stay on current page
    pageBefore = DaisyHandler::Instance()->getCurrentPage();
    assert(DaisyHandler::Instance()->nextPage()); // this call should succeed
    pageAfter = DaisyHandler::Instance()->getCurrentPage();
    assert(!DaisyHandler::Instance()->nextPage()); // this call should fail
    pageAfter = DaisyHandler::Instance()->getCurrentPage();
    assert(pageBefore == pageAfter);
    assert(DaisyHandler::Instance()->getLastError().getCode() == amis::AT_END);

    // navigate through all pages (beginning->end->beginning)
    int totalPages = 0;
    totalPages += bookInfo->mFrontPages;
    totalPages += bookInfo->mNormalPages;
    totalPages += bookInfo->mSpecialPages;
    assert(DaisyHandler::Instance()->firstPage());
    for (int i=0; i<totalPages-1; i++)
    {
        pageBefore = DaisyHandler::Instance()->getCurrentPage();
        assert(DaisyHandler::Instance()->nextPage());
        pageAfter = DaisyHandler::Instance()->getCurrentPage();
        assert(pageBefore != pageAfter);
    }
    for (int i=0; i<totalPages-1; i++)
    {
        pageBefore = DaisyHandler::Instance()->getCurrentPage();
        assert(DaisyHandler::Instance()->previousPage());
        pageAfter = DaisyHandler::Instance()->getCurrentPage();
        assert(pageBefore != pageAfter);
    }

    // cleanup before exit
    DaisyHandler::Instance()->closeBook();
    DaisyHandler::Instance()->DestroyInstance();

    return 0;
}
