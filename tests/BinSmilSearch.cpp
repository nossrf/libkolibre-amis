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
#include "Metadata.h"
#include "DaisyHandler.h"
#include "SmilEngine.h"
#include "BinarySmilSearch.h"
#include "setup_logging.h"

using namespace amis;

bool findSmilAt( unsigned int seconds )
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;

    BinarySmilSearch search;
    SmilTreeBuilder* treebuilder = search.begin();

    try {
        while( treebuilder != NULL )
        {
            if( search.currentSmilIsBeyond( seconds ) )
            {
                treebuilder = search.next( BinarySmilSearch::DOWN );
            }
            else
            {
                if( search.currentSmilContains( seconds ) )
                {
                    // We found the smil file covering seconds
                    std::cout << "Smil file: " << search.getCurrentSmilPath() << " contains " << seconds << " seconds position" << std::endl;
                    return true;
                }

                treebuilder = search.next( BinarySmilSearch::UP );
            }
        }
    } catch(int param) {
        std::cout << __PRETTY_FUNCTION__ << ": Search caught an exception: " << param << std::endl;
        return false;
    }

    std::cout << __PRETTY_FUNCTION__ << ": no file contains second : " << seconds << std::endl;
    return false;
}

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
    bool lastmark = DaisyHandler::Instance()->setupBook();

    std::string totaltime;
    switch (SmilEngine::Instance()->getDaisyVersion())
    {
    case DAISY3:
        std::cout << "DAISY3" << std::endl;
        totaltime = amis::Metadata::Instance()->getMetadata("dtb:totaltime");
        break;
    case DAISY202:
        std::cout << "DAISY202" << std::endl;
        totaltime = amis::Metadata::Instance()->getMetadata("ncc:totaltime");
        break;
    default:
        std::cout << "Daisy version not supported: " << SmilEngine::Instance()->getDaisyVersion() << std::endl;
        return 1;
    }

    int seconds = stringToSeconds( totaltime );
    int test = 1;

    // Test jumping to out-of-bound positions
    std::cout << "*****************************************************************" << std::endl;
    std::cout << " Test: " << test++ << ", jump to: " << -1 << " [Out-of-bound]" << std::endl;
    std::cout << "*****************************************************************" << std::endl;
    assert( ! findSmilAt( -1 ) );

    std::cout << "*****************************************************************" << std::endl;
    std::cout << " Test: " << test++ << ", jump to: " << seconds+100 << " [Out-of-bound]" << std::endl;
    std::cout << "*****************************************************************" << std::endl;
    assert( ! findSmilAt( seconds+100 ) );

    // Test jumping to border positions
    std::cout << "*****************************************************************" << std::endl;
    std::cout << " Test: " << test++ << ", jump to: " << 0 << " [Border]" << std::endl;
    std::cout << "*****************************************************************" << std::endl;
    assert( findSmilAt( 0 ) );

    std::cout << "*****************************************************************" << std::endl;
    std::cout << " Test: " << test++ << ", jump to: " << seconds << " [Border]" << std::endl;
    std::cout << "*****************************************************************" << std::endl;
    assert( findSmilAt( seconds ) ); // slutet

    // Test jumping to positions inside book
    std::cout << "*****************************************************************" << std::endl;
    std::cout << " Test: " << test++ << ", jump to: " << seconds/10 << " [Inside]" << std::endl;
    std::cout << "*****************************************************************" << std::endl;
    assert( findSmilAt( seconds/10 ) );

    std::cout << "*****************************************************************" << std::endl;
    std::cout << " Test: " << test++ << ", jump to: " << seconds/2 << " [Inside]" << std::endl;
    std::cout << "*****************************************************************" << std::endl;
    assert( findSmilAt( seconds/2 ) );

    // cleanup before exit
    DaisyHandler::Instance()->closeBook();
    DaisyHandler::Instance()->DestroyInstance();

    return 0;
}
