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
#endif

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <iostream>

#include "SmilEngine.h"
#include "NavParse.h"
#include "DaisyHandler.h"

#include "FakePlayer.h"

using namespace amis;
using namespace std;

FakePlayer *player = NULL;

#include <termios.h>

static struct termios stored_settings;

void set_keypress(void)
{
    struct termios new_settings;

    tcgetattr(0, &stored_settings);

    new_settings = stored_settings;

    /* Disable canonical mode, and set buffer size to 1 byte */
    new_settings.c_lflag &= (~ICANON);
    new_settings.c_cc[VTIME] = 0;
    new_settings.c_cc[VMIN] = 1;

    tcsetattr(0, TCSANOW, &new_settings);
    return;
}

void reset_keypress(void)
{
    tcsetattr(0, TCSANOW, &stored_settings);
    return;
}

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
    return DaisyHandler::Instance()->nextPhrase();
}

void got_alarm(int sig)
{
    ; //   fprintf(stderr, "Got signal %d\n", sig);
}

int main(int argc, char *argv[])
{
    char * currentLocale = setlocale(LC_ALL, "");
    cout << "currentLocale=" << currentLocale << endl;

    setenv("KOLIBRE_BOOKMARK_PATH", ".", 0);

    if (argc < 2)
    {
        cout << "Please specify an ncc.html file on the command line" << endl;
        exit(-1);
    }

    player = new FakePlayer();
    if (player->enable() != false)
        cout << "PLAYER ENABLED" << endl;

    player->setEOSFunction(EOSFunction);

    DaisyHandler *dh;

    dh = DaisyHandler::Instance();
    dh->openBook(argv[1]);

    while (dh->getState() == DaisyHandler::HANDLER_OPENING)
    {
        usleep(100000);
        printf(".");
        fflush (stdout);
    }

    dh->setPlayFunction(PlayFunction);

    printf("\n");
    dh->setupBook();

    srand(time(NULL));

    //dh->printNavLists();

    char key = 0;

    // tell terminal to only wait for one byte at a time
    set_keypress();

    cout << " r = right " << endl;
    cout << " l = left " << endl;
    cout << " u = up " << endl;
    cout << " d = down " << endl << endl;

    cout << " 1 = add bookmark " << endl;
    cout << " 2 = delete bookmark " << endl;
    cout << " 3 = print history " << endl;
    cout << " 4 = print book info " << endl << endl;

    cout << " q = quit " << endl;

    while (key != 'q')
    {
        signal(SIGALRM, got_alarm);
        alarm(1);
        key = getchar();

        if (key != 0)
        {

            cout << endl << "-----------------------------------------" << endl;

            switch (key)
            {
            case 'r': //KEY_RIGHT:
                switch (dh->getNaviLevel())
                {
                case DaisyHandler::BEGEND:
                    cout << endl << endl << "GOING TO LAST SECTION" << endl;
                    if (!dh->lastSection())
                        cerr << "JumpTest:: failed" << endl;
                    break;

                case DaisyHandler::H1:
                case DaisyHandler::H2:
                case DaisyHandler::H3:
                case DaisyHandler::H4:
                case DaisyHandler::H5:
                case DaisyHandler::H6:
                    cout << endl << endl << "GOING TO NEXT SECTION" << endl;
                    if (!dh->nextSection())
                        cerr << "JumpTest:: failed" << endl;
                    break;

                case DaisyHandler::PAGE:
                    cout << endl << endl << "GOING TO NEXT PAGE" << endl;
                    if (!dh->nextPage())
                        cerr << "JumpTest:: failed" << endl;
                    break;

                case DaisyHandler::PHRASE:
                    if (!dh->nextPhrase())
                        cerr << "JumpTest:: failed" << endl;
                    break;

                case DaisyHandler::HISTORY:
                    if (!dh->nextHistory())
                        cerr << "JumpTest:: failed" << endl;
                    break;

                case DaisyHandler::BOOKMARK:
                    if (!dh->nextBookmark())
                        cerr << "JumpTest:: failed" << endl;
                    break;
                }

                break;

            case 'l': //KEY_LEFT:
                switch (dh->getNaviLevel())
                {
                case DaisyHandler::BEGEND:
                    cout << endl << endl << "GOING TO FIRST SECTION" << endl;
                    if (!dh->firstSection())
                        cerr << "JumpTest:: failed" << endl;
                    break;
                case DaisyHandler::H1:
                case DaisyHandler::H2:
                case DaisyHandler::H3:
                case DaisyHandler::H4:
                case DaisyHandler::H5:
                case DaisyHandler::H6:
                    cout << endl << endl << "GOING TO PREVIOUS SECTION" << endl;
                    if (!dh->previousSection())
                        cerr << "JumpTest:: failed" << endl;
                    break;

                case DaisyHandler::PAGE:
                    cout << endl << endl << "GOING TO PREVIOUS PAGE" << endl;
                    if (!dh->previousPage())
                        cerr << "JumpTest:: failed" << endl;
                    break;

                case DaisyHandler::PHRASE:
                    if (!dh->previousPhrase())
                        cerr << "JumpTest:: failed" << endl;
                    break;

                case DaisyHandler::HISTORY:
                    if (!dh->previousHistory())
                        cerr << "JumpTest:: failed" << endl;
                    break;

                case DaisyHandler::BOOKMARK:
                    if (!dh->previousBookmark())
                        cerr << "JumpTest:: failed" << endl;
                    break;

                }
                break;

            case '1': //KEY_F1:
                //dh->addBookmark();
                break;

            case '2': //KEY_F2:
                //dh->deleteBookmark(dh->getCurrentBookmark());
                break;

            case '3': //KEY_F3:
                dh->printHistory();
                break;

            case '4':
                //dh->readBookInfo();
                break;

            case '5':
                dh->printNaviPos();
                break;

            case '6':
                dh->printNavLists();
                break;

            case '7':
                dh->printPageList();
                break;

            case '8':
                dh->printNavNodes();
                break;

            case 'u': //KEY_UP:
                //currentLevel = dh->getLevel();
                dh->increaseNaviLevel();
                dh->printNaviLevel();

                break;

            case 'd': //KEY_DOWN:
                //currentLevel = dh->getLevel();
                dh->decreaseNaviLevel();
                dh->printNaviLevel();
                break;

            case 'p': //KEY_PAUSE:
                if (player->getState() != FakePlayer::PLAYING)
                    player->play();
                else
                    player->pause();

                break;

            case 'b': //KEY_REPEAT:
                dh->previousHistory();
                break;

            default:
                cout << "Key pressed: " << key << endl;
                break;

            }

            if (key != 'p' && player->getState() != FakePlayer::PLAYING)
                player->play();
        }
    }

    delete player;

    dh->closeBook();
    delete dh;

    return 0;
}
