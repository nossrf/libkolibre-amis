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
        exit(-1);
    }

    setup_logging();

    if(not DaisyHandler::Instance()->openBook(argv[1])) {
        std::cout << "Unable to open requested file: " << argv[1] << std::endl;;
        exit(1);
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
    DaisyHandler::Instance()->setupBook();

    // play title, returns true if title was played
    assert(DaisyHandler::Instance()->playTitle());

    // cleanup before exit
    DaisyHandler::Instance()->closeBook();
    DaisyHandler::Instance()->DestroyInstance();

    return 0;
}
