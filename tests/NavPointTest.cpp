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

    // set navi level to smallest possible heading
    while(DaisyHandler::Instance()->getNaviLevel() >= DaisyHandler::PAGE)
        DaisyHandler::Instance()->increaseNaviLevel();

    // go to the beginning of book
    DaisyHandler::Instance()->firstSection();

    DaisyHandler::BookInfo *bookInfo = DaisyHandler::Instance()->getBookInfo();
    DaisyHandler::PosInfo *posInfo = DaisyHandler::Instance()->getPosInfo();

    // start looping through each navigation point
    std::cout << "Book contains " << bookInfo->mTocItems << " toc items, " << bookInfo->mNormalPages
            << " pages, and " << bookInfo->mSections << " sections" << std::endl;
    DaisyHandler::Instance()->printNaviLevel();

    for(int i=posInfo->currentSectionIdx; i<bookInfo->mSections; i++) {
        posInfo = DaisyHandler::Instance()->getPosInfo();
        std::cout << "Reading at: " << posInfo->currentSectionIdx << std::endl;;
        assert(DaisyHandler::Instance()->nextSection());
    }

    // we have now reached the last section and sould not be able to jump to the next section
    assert(!DaisyHandler::Instance()->nextSection());

    // go the the end of the book
    DaisyHandler::Instance()->lastSection();

    // we have now reachde the last section and sould not be able to jump to the next section
    assert(!DaisyHandler::Instance()->nextSection());

    // get navigation points and jump to each point
    DaisyHandler::NavPoints* navPoints = DaisyHandler::Instance()->getNavPoints();
    std::cout << "trying jump to each page by id" << std::endl;
    for(int i=0; i<navPoints->pages.size(); i++)
    {
        std::cout << "jumping to page with id " << navPoints->pages[i].id << " '" << navPoints->pages[i].text << "'" << std::endl;
        assert(DaisyHandler::Instance()->goToId(navPoints->pages[i].id));
    }
    std::cout << "trying jump to each section by id" << std::endl;
    for(int i=0; i<navPoints->sections.size(); i++)
    {
        std::cout << "jumping to section with id " << navPoints->sections[i].id << " '" << navPoints->sections[i].text << "'" << std::endl;
        assert(DaisyHandler::Instance()->goToId(navPoints->sections[i].id));
    }


    // cleanup before exit
    DaisyHandler::Instance()->closeBook();
    DaisyHandler::Instance()->DestroyInstance();

    return 0;
}
