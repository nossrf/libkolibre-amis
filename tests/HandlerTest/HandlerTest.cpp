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

enum keyCode
{
    KEY_NONE,
    KEY_REPEAT,
    KEY_PAUSE,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_SPEEDUP,
    KEY_SPEEDDOWN,
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,

    // SPECIAL KEYS
    KEY_NEXT,
    KEY_LASTMARK
};

FakePlayer *player = NULL;

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

keyCode randomKey()
{
    int key = rand() % 20;
    if (key >= 14)
        return KEY_LEFT;
    if (key >= 6)
        return KEY_RIGHT;

    return (keyCode) key;
}

int main(int argc, char *argv[])
{
    cout << "starts? " << endl;
    usleep(5000000);
    char * currentLocale = setlocale(LC_ALL, "");
    cout << "currentLocale=" << currentLocale << endl;

    if (argc < 2)
    {
        cout << "Please specify an ncc.html file on the command line" << endl;
        exit(-1);
    }

    player = new FakePlayer();
    if (player->enable() != false)
        cout << "PLAYER ENABLED" << endl;

    player->setEOSFunction(EOSFunction);

    //DaisyHandler::Instance()->openBook("/tmp/daisy_books/patalomsthlm/ncc.html");
    //DaisyHandler::Instance()->openBook("/mnt/tmp/VBL_2007-06-10/ncc.html");
    //DaisyHandler::Instance()->openBook("https://returnp.ath.cx/VBL_2007-06-10/ncc.html");
    DaisyHandler *dh;

    dh = DaisyHandler::Instance();
    //dh->openBook("https://returnp.ath.cx/VBL_2007-06-10/ncc.html");

    //dh->openBook("/tmp/daisy_books/anne/ncc.html");
    dh->openBook(argv[1]);
    //dh->openBook("/tmp/daisy_books/book3/Mountains_skip/ncc.html");
    //dh->openBook("/tmp/daisy_books/book3/Mountains_noskip/ncc.html");
    //dh->openBook("/tmp/daisy_books/book4/ncc.html");
    //dh->openBook("/tmp/daisy_books/koran/ncc.html");
    //dh->openBook("/tmp/daisy_books/patalomsthlm/ncc.html");
    //dh->openBook("/tmp/daisy_books/kursmaterial/ncc.html");
    //dh->openBook("/tmp/daisy_books/warworld/ncc.html");
    //dh->openBook("/tmp/daisy_books/klee_wyck/ncc.html");
    //dh->openBook("/tmp/daisy_books/valentin_hauy/ncc.html");
    //dh->openBook("/tmp/daisy_books/victor/ncc.html");
    //dh->openBook("/tmp/daisy_books/daisyforalla/ncc.html");
    //dh->openBook("/tmp/daisy_books/DAISY_Sampler/ncc.html");
    //dh->openBook("/tmp/daisy_books/daisydemo/delta5a/ncc.html");
    //dh->openBook("/tmp/daisy_books/daisydemo/lydbok_nynorsk/ncc.html");
    //dh->openBook("/tmp/daisy_books/daisydemo/soria_moria/ncc.html");
    //dh->openBook("/tmp/daisy_books/daisydemo/lydbok_bokmaal/ncc.html");
    //dh->openBook("/tmp/daisy_books/daisydemo/flight/ncc.html");
    //dh->openBook("/tmp/daisy_books/daisydemo/midt_i_vaar_verden/ncc.html");
    //dh->openBook("/tmp/daisy_books/daisy_news_february_2004/ncc.html");

    //dh->openBook("/mnt/tmp/anne/ncc.html");
    //dh->openBook("/mnt/tmp/at ratt at gott ma bra/ncc.html");
    //dh->openBook("/mnt/tmp/book4/ncc.html");
    //dh->openBook("/mnt/tmp/book_of_mormon/ncc.html");
    //dh->openBook("/mnt/tmp/Daisy Esite 2005/ncc.html");
    //dh->openBook("/mnt/tmp/DAISY Reading The Future/ncc.html");
    //dh->openBook("/mnt/tmp/digital_dividends/ncc.html");
    //dh->openBook("/mnt/tmp/KORAN/ncc.html");
    //dh->openBook("/mnt/tmp/Medicare 2003/ncc.html");
    //dh->openBook("/mnt/tmp/ms_conference_dtb/ncc.html");
    //dh->openBook("/mnt/tmp/nakovammaisten palverluopas 2005/ncc.html");
    //dh->openBook("/mnt/tmp/new testament/ncc.html");
    //dh->openBook("/mnt/tmp/old testament/ncc.html");
    //dh->openBook("/mnt/tmp/patalomsthlm/ncc.html");
    //dh->openBook("/mnt/tmp/Prismas stora bok om halsa/ncc.html");
    //dh->openBook("/mnt/tmp/tale_of_two_cities/ncc.html");
    //dh->openBook("/mnt/tmp/techshare2007/ncc.html");
    //dh->openBook("/mnt/tmp/VBL_2007-06-10/ncc.html");
    //dh->openBook("/mnt/tmp/victor/ncc.html");

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

    keyCode key = KEY_NONE;
    int interval = 0;
    bool randomkeyseeder = false;
    char input = getchar();
    cout << "--" << input << "--";
    while (key != KEY_F4)
    {
        if (interval++ == 100)
        {
            player->printState();
            interval = 0;
            usleep(100);
        };

        if (randomkeyseeder && (rand() % 10) == 1)
        {
            randomKey();
            usleep(rand() % 10000);
        }

        if (key != '0')
        {
            ///key = Input::Instance()->getKey();

            cout << endl << "-----------------------------------------" << endl;
            player->pause();

            switch (key)
            {
            case KEY_RIGHT:
                switch (dh->getNaviLevel())
                {
                case DaisyHandler::BEGEND:
                    cout << endl << endl << "GOING TO LAST SECTION" << endl;
                    if (!dh->lastSection())
                        cerr << "HandlerTest:: failed" << endl;
                    break;

                case DaisyHandler::H1:
                case DaisyHandler::H2:
                case DaisyHandler::H3:
                case DaisyHandler::H4:
                case DaisyHandler::H5:
                case DaisyHandler::H6:
                    cout << endl << endl << "GOING TO NEXT SECTION" << endl;
                    if (!dh->nextSection())
                        cerr << "HandlerTest:: failed" << endl;
                    break;

                case DaisyHandler::PAGE:
                    cout << endl << endl << "GOING TO NEXT PAGE" << endl;
                    if (!dh->nextPage())
                        cerr << "HandlerTest:: failed" << endl;
                    break;

                case DaisyHandler::PHRASE:
                    if (!dh->nextPhrase())
                        cerr << "HandlerTest:: failed" << endl;
                    break;

                case DaisyHandler::HISTORY:
                    if (!dh->nextHistory())
                        cerr << "HandlerTest:: failed" << endl;
                    break;

                case DaisyHandler::BOOKMARK:
                    if (!dh->nextBookmark())
                        cerr << "HandlerTest:: failed" << endl;
                    break;
                }

                break;

            case KEY_LEFT:
                switch (dh->getNaviLevel())
                {
                case DaisyHandler::BEGEND:
                    cout << endl << endl << "GOING TO FIRST SECTION" << endl;
                    if (!dh->firstSection())
                        cerr << "HandlerTest:: failed" << endl;
                    break;
                case DaisyHandler::H1:
                case DaisyHandler::H2:
                case DaisyHandler::H3:
                case DaisyHandler::H4:
                case DaisyHandler::H5:
                case DaisyHandler::H6:
                    cout << endl << endl << "GOING TO PREVIOUS SECTION" << endl;
                    if (!dh->previousSection())
                        cerr << "HandlerTest:: failed" << endl;
                    break;

                case DaisyHandler::PAGE:
                    cout << endl << endl << "GOING TO PREVIOUS PAGE" << endl;
                    if (!dh->previousPage())
                        cerr << "HandlerTest:: failed" << endl;
                    break;

                case DaisyHandler::PHRASE:
                    if (!dh->previousPhrase())
                        cerr << "HandlerTest:: failed" << endl;
                    break;

                case DaisyHandler::HISTORY:
                    if (!dh->previousHistory())
                        cerr << "HandlerTest:: failed" << endl;
                    break;

                case DaisyHandler::BOOKMARK:
                    if (!dh->previousBookmark())
                        cerr << "HandlerTest:: failed" << endl;
                    break;

                }
                break;

            case KEY_F1:
                //dh->addBookmark();
                break;

            case KEY_F2:
                dh->getBookInfo();
                //dh->deleteBookmark(dh->getCurrentBookmark());
                break;

            case KEY_F3:
                dh->printHistory();
                break;

            case KEY_UP:
                //currentLevel = dh->getLevel();
                dh->increaseNaviLevel();
                dh->printNaviLevel();

                break;

            case KEY_DOWN:
                //currentLevel = dh->getLevel();
                dh->decreaseNaviLevel();
                dh->printNaviLevel();
                break;

            case KEY_PAUSE:
                player->pause();
                break;

            case KEY_REPEAT:
                dh->previousHistory();
                break;

            case KEY_SPEEDUP:
                randomkeyseeder = !randomkeyseeder;
                break;

            default:
                cout << "Key pressed: " << key << endl;
                break;

            }

            if (key != KEY_PAUSE && player->getState() != FakePlayer::PLAYING)
                player->play();
        }
    }

    //Input *input = Input::Instance();
    //delete input;

    delete player;

    dh->closeBook();
    delete dh;

    /*

     dh->openBook("https://returnp.ath.cx/KORAN/ncc.html");
     while(dh->getState() == DaisyHandler::HANDLER_OPENING) {
     usleep(100000);
     printf("."); fflush(stdout);
     }
     printf("\n");
     dh->setupBook();


     dh->openBook("https://returnp.ath.cx/VBL_2007-06-10/ncc.html");
     while(dh->getState() == DaisyHandler::HANDLER_OPENING) {
     usleep(100000);
     printf("."); fflush(stdout);
     }
     printf("\n");
     dh->setupBook();



     dh->openBook("/mnt/tmp/at ratt at gott ma bra/ncc.html");
     while(dh->getState() == DaisyHandler::HANDLER_OPENING) {
     usleep(100000);
     printf("."); fflush(stdout);
     }
     printf("\n");
     dh->setupBook();
     */

    return 0;
}
