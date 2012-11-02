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

//Input header file

#ifndef INPUT_H
#define INPUT_H

#include <pthread.h>
#include <termio.h>

using namespace std;

#define MOUSE_TRIG       4 // coordinates to move before triggering
#define MOUSE_TYPE_NONE  0 // type none
#define MOUSE_TYPE_PS2   1 // type ps2
#define MOUSE_TYPE_IMPS2 2 // type imps2
#include <queue>

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

class Input
{
protected:
    Input();
public:
    static Input *Instance();
    ~Input();

    bool keyWaiting();
    void injectKey(keyCode key);
    keyCode getKey();
    void flushQueue();

private:
    pthread_mutex_t *queueMutex;
    queue<keyCode> keyQueue;

    static Input *pinstance;

    pthread_t inputThread;
    friend void *input_thread(void *input);
    bool running;

    int kb_startup_mode;
    struct termios kb_startup_settings;
    int keyboardfd;

    int read_mouse_id(int mousefd);
    int write_to_mouse(int mousefd, unsigned char *data, size_t len);
    int mousefd;
    int mousetype;
    unsigned char mbuttons;
    int mx, my, mouse_state;
    int mb1, mb2, mb3;

};

#endif
