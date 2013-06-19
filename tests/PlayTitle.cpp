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
#include <Player.h>
#include <string>
#include "DaisyHandler.h"
#include "setup_logging.h"

using namespace amis;

bool playerMessageSlot( Player::playerMessage message )
{
    switch (message)
    {
        case Player::PLAYER_ATEOS:
            Player::Instance()->stop();
            return false;
        default:
            return true;
    }
}

bool playerStateSlot( playerState state )
{
    return true;
    /*
    switch (state)
    {
        case INACTIVE:
        case BUFFERING:
        case PLAYING:
        case PAUSING:
        case STOPPED:
        case EXITING:
            return true;
    }

    return false;
    */
}

static bool playAudio(std::string filename, long long startms, long long stopms)
{
    std::cout << "open file " << filename << std::endl;
    //Player::Instance()->doOnPlayerMessage( boost::bind(&playerMessageSlot, this, _1) );
    //Player::Instance()->doOnPlayerState( boost::bind(&playerStateSlot, this, _1) );
    Player::Instance()->doOnPlayerMessage(&playerMessageSlot);
    Player::Instance()->enable(NULL, NULL);
    Player::Instance()->open(filename, startms, stopms);
    Player::Instance()->resume();
    usleep(500000); while (Player::Instance()->isPlaying()) usleep(100000);
    return true;
};

int main(int argc, char *argv[])
{
    if(argc < 2) {
        std::cout << "Please specify an ncc.html file on the command line" << std::endl;
        exit(-1);
    }

    setup_logging();
    log4cxx::LoggerPtr amisLogger(log4cxx::Logger::getLogger("kolibre.amis"));
    amisLogger->setLevel(log4cxx::Level::getFatal());

    for (int i=1; i<argc; i++)
    {

    if(not DaisyHandler::Instance()->openBook(argv[i])) {
        std::cout << "Unable to open requested file: " << argv[i] << std::endl;;
        exit(1);
    }

    while(DaisyHandler::Instance()->getState() == DaisyHandler::HANDLER_OPENING) {
        usleep(1000);
    }

    if(DaisyHandler::Instance()->getState() != DaisyHandler::HANDLER_OPEN) {
        std::cout << "The file " << argv[i] << " could not be open, please check for errors" << std::endl;
        DaisyHandler::Instance()->DestroyInstance();
        return 1;
    }

    // setup the book, returns true if the book has a lastmark
    DaisyHandler::Instance()->setupBook();

    DaisyHandler::Instance()->setPlayFunction(playAudio);

    std::cout << "START playing title" << std::endl;
    // play title, returns true if title was played
    assert(DaisyHandler::Instance()->playTitle());
    std::cout << "STOPPED playing title" << std::endl;

    // cleanup before exit
    DaisyHandler::Instance()->closeBook();
    }
    DaisyHandler::Instance()->DestroyInstance();


    return 0;
}
