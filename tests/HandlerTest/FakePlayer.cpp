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
#include <iostream>
#include <unistd.h>
#include <cstdlib>

#include "FakePlayer.h"
#define PLAYER "FakePlayer: "
#define PLAYERTHREAD "FakePlayert: "

#ifdef WIN32
#define usleep(n) Sleep(n*1000);
#endif

FakePlayer::FakePlayer()
{
    mStartms = mCurms = 0;
    mStopms = 1;
    mFilename = "";
    reopen = false;
    printdelaycount = 0;

    EOSFunction = NULL;

    playerMutex = (pthread_mutex_t *) std::malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(playerMutex, NULL);
    setState(NOT_INITIALIZED);

}

FakePlayer::~FakePlayer()
{
    cout << PLAYER << "Stopping thread" << endl;

    setState(EXIT);
    pthread_join(playerThread, NULL);
}

bool FakePlayer::enable()
{
    if (pthread_create(&playerThread, NULL, staticPlayerThread, this))
    {
        usleep(50000);
        setState(ENABLED);
        return true;
    }
    return false;
}

bool FakePlayer::staticPlay(string filename, long long start, long long stop,
        void *player)
{
    FakePlayer *p = (FakePlayer *) player;
    p->play(filename, start, stop);
}

bool FakePlayer::play(string filename, long long start, long long stop)
{

    // Update the file data
    pthread_mutex_lock(playerMutex);
    mCurms = start;
    mStartms = start;
    mStopms = stop;
    mFilename = filename;
    reopen = true;
    printdelaycount = 20;
    pthread_mutex_unlock(playerMutex);

    // Set state to playing
    if (getState() != PLAYING)
        setState(PLAYING);
}

bool FakePlayer::play()
{
    PlayerState state = getState();
    if (state == PAUSED || state == STOPPED)
        setState(PLAYING);
    else
        return false;
}

bool FakePlayer::pause()
{
    PlayerState state = getState();
    if (state == STOPPED || state == PLAYING)
        setState(PAUSED);
    else
        return false;
}

bool FakePlayer::stop()
{
    PlayerState state = getState();
    if (state == STOPPED || state == PAUSED)
        setState(STOPPED);
    else
        return false;
}

void FakePlayer::setState(PlayerState state)
{
    cout << PLAYER << "Setting state to: ";
    switch (state)
    {
    case NOT_INITIALIZED:
        cout << "NOT INITIALIZED" << endl;
        break;
    case ENABLED:
        cout << "ENABLED" << endl;
        break;
    case STOPPED:
        cout << "STOPPED" << endl;
        break;
    case PLAYING:
        cout << "PLAYING" << endl;
        break;
    case PAUSED:
        cout << "PAUSED" << endl;
        break;
    }

    pthread_mutex_lock(playerMutex);
    currentState = state;
    pthread_mutex_unlock(playerMutex);
}

FakePlayer::PlayerState FakePlayer::getState()
{
    PlayerState state = NOT_INITIALIZED;
    pthread_mutex_lock(playerMutex);
    state = currentState;
    pthread_mutex_unlock(playerMutex);
    return state;
}

void FakePlayer::printState()
{
    pthread_mutex_lock(playerMutex);
    int delaycount = printdelaycount;
    pthread_mutex_unlock(playerMutex);

    if (printdelaycount > 0)
    {
        printdelaycount--;
        return;
    }

    long long curms, startms, stopms;
    string filename;

    pthread_mutex_lock(playerMutex);
    curms = mCurms;
    startms = mStartms;
    stopms = mStopms;
    filename = mFilename;
    pthread_mutex_unlock(playerMutex);

    switch (getState())
    {
    case PLAYING:
        cout << PLAYER << "Playing segment " << startms << "->" << stopms << "@"
                << curms << " in " << filename << "\r";
        break;
    case PAUSED:
        break;
        //scout << PLAYER << "Pausing segment " << startms << "->" << stopms << "@" << curms << " in " << filename << "\r";
        break;
    }
}

void FakePlayer::setEOSFunction(bool (*ptr)())
{
    EOSFunction = ptr;
}

bool FakePlayer::callEOSFunction()
{
    pthread_mutex_lock(playerMutex);
    printdelaycount = 100;
    pthread_mutex_unlock(playerMutex);

    if (EOSFunction != NULL)
    {
        cout << PLAYER << "Calling Continue callback" << endl;
        return EOSFunction();
    }

    return false;
}

void *FakePlayer::staticPlayerThread(void *player)
{
    FakePlayer *p = (FakePlayer*) player;
    // Call the class's own playerthread function
    p->PlayerThread();
    return NULL;
}

void *FakePlayer::PlayerThread()
{
    long long curms = 0;
    long long startms = 0;
    long long stopms = 0;
    string filename = "";
    bool reposition = false;
    bool calledback = false;

    PlayerState state, lastState;
    state = lastState = getState();

    cout << PLAYERTHREAD << "Starting player thread" << endl;

    while (getState() != EXIT)
    {
        state = getState();
        switch (state)
        {
        case ENABLED:
        case STOPPED:
            break;

        case PLAYING:
            if (state != lastState || calledback)
            {
                // Check if we have a new filename, or new positions
                bool open = false;
                pthread_mutex_lock(playerMutex);
                if (filename != mFilename || reopen)
                {
                    filename = mFilename;
                    curms = mCurms;
                    startms = mStartms;
                    stopms = mStopms;
                    open = true;
                    reopen = false;
                }
                else if (startms != mStartms || stopms != mStopms)
                {
                    curms = mCurms;
                    startms = mStartms;
                    stopms = mStopms;
                    reposition = true;
                }
                pthread_mutex_unlock(playerMutex);

                if (open)
                {
                    cout << PLAYERTHREAD << "Opening " << filename << endl;
                    calledback = false;
                }

                if (reposition)
                {
                    cout << PLAYERTHREAD << "Repositioning in " << filename
                            << endl;
                    calledback = false;
                }
            }

            // Check if we're at the point where we are supposed to stop playing
            if (curms >= stopms && !calledback)
            {
                // If the callback returns a false, we shouldn't continue
                if (callEOSFunction() == false)
                {
                    setState(STOPPED);
                }
                calledback = true;
            }
            else
            {
                // Increase the current position by one ms
                curms += 1;
                pthread_mutex_lock(playerMutex);
                mCurms = curms;
                pthread_mutex_unlock(playerMutex);
            }

            break;
        case PAUSED:
            // Do nothing
            break;
        }
        lastState = state;
        usleep(1);
    }

    cout << PLAYERTHREAD << "Exiting player thread" << endl;
    pthread_exit(NULL);
    return NULL;
}
