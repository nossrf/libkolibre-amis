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

#include <string>
#include <pthread.h>

using namespace std;

class FakePlayer
{

public:
    FakePlayer();
    ~FakePlayer();

    bool enable();
    bool play(string filename, long long startms, long long stopms);
    static bool staticPlay(string filename, long long start, long long stop,
            void *player);
    bool stop();
    bool pause();
    bool play();

    void printState();

    void setEOSFunction(bool (*ptr)());

    enum PlayerState
    {
        NOT_INITIALIZED, ENABLED, STOPPED, PLAYING, PAUSED, EXIT
    };

    PlayerState getState();
    void setState(PlayerState state);

private:
    long long mCurms;
    long long mStartms;
    long long mStopms;
    string mFilename;

    bool reopen;
    int printdelaycount;

    // pointer to the continue function
    bool (*EOSFunction)();
    bool callEOSFunction();

    PlayerState currentState;

    pthread_t playerThread;
    pthread_mutex_t *playerMutex;

    static void *staticPlayerThread(void *player);
    void *PlayerThread();
};

