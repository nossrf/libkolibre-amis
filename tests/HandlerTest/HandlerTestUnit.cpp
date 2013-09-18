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

#include <locale.h>

#ifdef WIN32
#include <windows.h>
#include <stdio.h>
#endif

#include <unistd.h>
#include <iostream>
#include <pthread.h>
#include <assert.h>
#include "SpineBuilder.h"
#include "SmilTreeBuilder.h"
#include "SmilEngine.h"
#include "NavParse.h"
#include "DaisyHandler.h"

#include "FakePlayer.h"
#include "../setup_logging.h"
//#include "Input.h"

using namespace amis;
using namespace std;

FakePlayer *player = NULL;
static bool running = true;
static bool sendprev = true;

bool PlayFunction(string filename, long long startms, long long stopms)
{
    if (player != NULL)
    {
        //cout << "Calling player for " << filename << "@" << startms << "->" << stopms << endl;
        return player->play(filename, startms, stopms);
    }
    else
        return false;
}

bool EOSFunction()
{
    //cout << "Calling EOS function" << endl;
    if(player->getState()!=FakePlayer::STOPPED)
        return DaisyHandler::Instance()->nextPhrase();
    else
        return false;
}

bool openBook(DaisyHandler *dh, std::string book)
{
    dh->openBook(book);
    while(dh->getState() == DaisyHandler::HANDLER_OPENING) {
        usleep(500000);
        printf("."); fflush(stdout);
    }

    printf("\n");
    dh->setupBook();
    return player->play();
}

void *prevCaller(void *pHandler){
    DaisyHandler *dh = (DaisyHandler *)pHandler;
    srand(time(NULL));

    while(running){
        if(sendprev)
            dh->previousSection();
        usleep(rand() % 1000000);
    }
}

int main(int argc, char *argv[])
{
    cout << "starts? " << endl;

    char * currentLocale = setlocale(LC_ALL, "");
    cout << "currentLocale=" << currentLocale << endl;

    if (argc < 2)
    {
        cout << "Please specify an ncc.html file on the command line" << endl;
        exit(-1);
    }

    setup_logging();

    player = new FakePlayer();
    if (player->enable() != false)
        cout << "PLAYER ENABLED" << endl;

    player->setEOSFunction(EOSFunction);

    DaisyHandler *dh;

    dh = DaisyHandler::Instance();
    dh->setPlayFunction(PlayFunction);
    openBook(dh, argv[1]);

    pthread_t prevCallerThread;

    assert(!pthread_create(&prevCallerThread, NULL, prevCaller, dh));
    bool firstOpen = true;

    for(int i = 5000000; i > 0; i-=1000000){
        // Open book
        openBook(dh, argv[1]);
        usleep(500000);
        assert(dh->increaseNaviLevel());
        assert(dh->increaseNaviLevel());
        // Call previus
        usleep(i);
        //create history
        assert(dh->nextSection()==amis::OK);
        assert(dh->nextSection()==amis::OK);
        usleep(i/10);
        sendprev=true;

        player->stop();
        dh->closeBook();
        usleep(i);
        sendprev=false;
    }

    running = false;
    pthread_join(prevCallerThread, NULL);

    delete player;
    delete dh;

    return 0;
}


