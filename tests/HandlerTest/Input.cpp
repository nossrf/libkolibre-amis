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

#include <stdio.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/kd.h>
//#include <linux/keyboard.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>

using namespace std;

//#include <../Common/Common.h>

#include "Input.h"

#define INPUT_PREFIX "input"

Input * Input::pinstance = 0;

Input * Input::Instance()
{
    if (pinstance == 0)
    {
        pinstance = new Input;
    }

    return pinstance;
}

/*
 char *to_bin(unsigned char c) {
 static char str[9];

 sprintf(str, "%c%c%c%c%c%c%c%c\0",
 c & 1 ? '1' : '0',
 c & 2 ? '1' : '0',
 c & 4 ? '1' : '0',
 c & 8 ? '1' : '0',
 c & 16 ? '1' : '0',
 c & 32 ? '1' : '0',
 c & 64 ? '1' : '0',
 c & 128 ? '1' : '0');
 return str;
 }
 */
//--------------------------------------------------
//--------------------------------------------------
// The playback thread code
void *input_thread(void *input)
{
    Input *i = (Input *) input;

    struct timeval tv;
    fd_set readfds;
    int largest_fd;

    keyCode key_pressed = KEY_NONE;

    int bytes_read;
    int bytes_to_read;
    unsigned char buffer[128];
    bool cont = true;

    if (i->mousetype == MOUSE_TYPE_PS2)
        bytes_to_read = 3;
    else if (i->mousetype = MOUSE_TYPE_IMPS2
        ) bytes_to_read = 4;

    // Loop
    do
    {
        //Common::LOG(LOG_INFO, "%s: Clearing readfds" , INPUT_PREFIX);

        // Reset checker parameters
        FD_ZERO(&readfds);

        if (i->keyboardfd != -1)
        {
            FD_SET(i->keyboardfd, &readfds);
            if (largest_fd < i->keyboardfd)
                largest_fd = i->keyboardfd;
        }

        if (i->mousefd != -1)
        {
            FD_SET(i->mousefd, &readfds);
            if (largest_fd < i->mousefd)
                largest_fd = i->mousefd;
        }

        // Reset timeout
        tv.tv_sec = 0;
        tv.tv_usec = 50000;

        // Check for a key on the fd's we have chosen
        select(largest_fd + 1, &readfds, NULL, NULL, &tv);

        // Check the input from the keyboard
        if (i->keyboardfd != -1 && FD_ISSET(i->keyboardfd, &readfds))
        {
            // We have data waiting
            memset(&buffer, 0, 128);
            bytes_read = read(i->keyboardfd, &buffer, sizeof(buffer));

            if (bytes_read)
            {
                // Check what bytes we read and process them
                /*cout << "Buf: ";
                 for(int c = 0; c < bytes_read; c++)
                 cout << "'" << (int) buffer[c] << "' ";
                 cout , INPUT_PREFIX);*/
                //buffer[bytes_read] = '\0';
                switch (buffer[0])
                {
                case 10:
                    key_pressed = KEY_REPEAT;
                    break;
                case 49:
                    key_pressed = KEY_F1;
                    break;
                case 50:
                    key_pressed = KEY_F2;
                    break;
                case 51:
                    key_pressed = KEY_F3;
                    break;
                case 52:
                    key_pressed = KEY_F4;
                    break;
                case 66:
                    if (i->mousetype == MOUSE_TYPE_IMPS2)
                        key_pressed = KEY_F4;
                    break;

                case 27:
                    switch (buffer[1])
                    {
                    case 91:
                        switch (buffer[2])
                        {
                        case 65:
                            key_pressed = KEY_UP;
                            break;
                        case 66:
                            key_pressed = KEY_DOWN;
                            break;
                        case 67:
                            key_pressed = KEY_RIGHT;
                            break;
                        case 68:
                            key_pressed = KEY_LEFT;
                            break;
                        case 52:
                            if (buffer[3] == 126)
                                key_pressed = KEY_PAUSE;
                            break;
                        case 53:
                            if (buffer[3] == 126)
                                key_pressed = KEY_SPEEDUP;
                            break;
                        case 54:
                            if (buffer[3] == 126)
                                key_pressed = KEY_SPEEDDOWN;
                            break;
                        case 91:
                            if (buffer[3]
                                    == 69&& i->mousetype == MOUSE_TYPE_IMPS2)key_pressed = KEY_PAUSE; break;
                            //default: Common::LOG(LOG_WARN, "%s: UNKNOWN KEY", INPUT_PREFIX); break;
                        }
                        break;

                        default:
                        if(i->mousetype == MOUSE_TYPE_IMPS2) key_pressed = KEY_PAUSE;
                        //else Common::LOG(LOG_WARN, "%s: UNKNOWN KEY", INPUT_PREFIX); break;
                    }
                    break;
                    //default: Common::LOG(LOG_WARN, "%s: UNKNOWN KEY", INPUT_PREFIX); break;
                }
            }

        }

        char data[4] =
        { 0, 0, 0, 0 };

        // Check the input from the mouse
        if (i->mousefd != -1 && FD_ISSET(i->mousefd, &readfds))
        {

            bytes_read = read(i->mousefd, data, bytes_to_read);
            if (bytes_read == bytes_to_read)
            {
                char dx, dy;
                // check for a button press
                char mbuttons = ((data[0] & 1) << 2) // left button
                | ((data[0] & 6) >> 1); // middle and right button

                // check for mouse movement 
                dx = (data[0] & 0x10) ? data[1] - 256 : data[1];
                dy = (data[0] & 0x20) ? -(data[2] - 256) : -data[2];

                if (i->mouse_state > 1)
                {
                    i->mx += dx;
                    i->my += dy;
                }
                else if (i->mouse_state == 0)
                {
                    //if(!(mb1 || mb2)) beep();
                    i->mx = dx;
                    i->my = dy;
                }

                if (!(i->mb1 || i->mb2 || i->mb3))
                {
                    //cout << "Mouse started moving" , INPUT_PREFIX);
                    i->mouse_state = 5;
                }
                else
                    i->mouse_state = 2;

                //printf("mbuttons %s\n", to_bin(mbuttons));
                //printf("mx: %d\t, my: %d\t mbuttons: %d mouse_state %d data %s %x %x %x %x\n", i->mx, i->my, i->mbuttons, i->mouse_state, 
                //to_bin(data[0]),
                //     data[0], data[1], data[2], data[3]);

                if (mbuttons == 1)
                    i->mb2 = 1;
                else if (mbuttons == 2)
                    i->mb3 = 1;
                else if (mbuttons == 4)
                    i->mb1 = 1;
            } //else { printf("bytes_read: %d\n", bytes_read); }
        }

        // Check the mouse_state, if we've stopped moving make it a key
        if (i->mousefd != -1 && i->mouse_state > 1)
        {
            if (i->mouse_state > 1)
                i->mouse_state--;
            //printw("mouse_state: %d\n", mouse_state);
            if (i->mouse_state == 1)
            {
                //printf("Stopped moving at %d, %d with buttons %d and data %x %x %x %x\n", i->mx, i->my, i->mbuttons,
                //   data[0], data[1], data[2], data[3]);
                if (!(i->mb1 || i->mb2 || i->mb3))
                {
                    //cout << "Mouse stopped moving" , INPUT_PREFIX);
                    if (abs(i->mx) > abs(i->my))
                    {
                        if (i->mx > MOUSE_TRIG)
                            key_pressed = KEY_RIGHT; //RIGHT
                        else if (i->mx < -MOUSE_TRIG)
                            key_pressed = KEY_LEFT; //LEFT
                    }
                    else
                    {
                        if (i->my > MOUSE_TRIG)
                            key_pressed = KEY_DOWN; //DOWN
                        else if (i->my < -MOUSE_TRIG)
                            key_pressed = KEY_UP; //UP
                    }
                }
                if (!i->mbuttons && i->mb1)
                {
                    key_pressed = KEY_REPEAT;
                    i->mb1 = 0;
                } // pause
                if (!i->mbuttons && i->mb2)
                {
                    key_pressed = KEY_PAUSE;
                    i->mb2 = 0;
                } // repeat
                if (!i->mbuttons && i->mb3)
                {
                    key_pressed = KEY_F4;
                    i->mb3 = 0;
                } // repeat
                i->mouse_state = 0;
            }
        }

        if (key_pressed != KEY_NONE)
        {
            /*switch(key_pressed) {
             case PAUSE: cout << "PAUSE pressed" , INPUT_PREFIX); break;
             case REPEAT: cout << "REPEAT pressed" , INPUT_PREFIX); break;
             case UP: cout << "UP pressed" , INPUT_PREFIX); break;
             case DOWN: cout << "DOWN pressed" , INPUT_PREFIX); break;
             case LEFT: cout << "LEFT pressed" , INPUT_PREFIX); break;
             case RIGHT: cout << "RIGHT pressed" , INPUT_PREFIX); break;
             case SPEEDUP: cout << "SPEEDUP pressed" , INPUT_PREFIX); break;
             case SPEEDDOWN: cout << "SPEEDDOWN pressed" , INPUT_PREFIX); break;
             case F1: cout << "F1 pressed" , INPUT_PREFIX); break;
             case F2: cout << "F2 pressed" , INPUT_PREFIX); break;
             case F3: cout << "F3 pressed" , INPUT_PREFIX); break;
             case F4: cout << "F4 pressed" , INPUT_PREFIX); break;
             default: cout << "UNKNOWN KEY PRESSED" , INPUT_PREFIX); break;
             }*/

            // Add the key to the queue
            pthread_mutex_lock(i->queueMutex);
            i->keyQueue.push(key_pressed);
            pthread_mutex_unlock(i->queueMutex);

            key_pressed = KEY_NONE;
        }

        pthread_mutex_lock(i->queueMutex);
        cont = i->running;
        pthread_mutex_unlock(i->queueMutex);

    } while (cont);

    //Common::LOG(LOG_INFO, "%s: Input thread exiting" , INPUT_PREFIX);
    cout << "Input thread exiting" << endl;

    //pthread_mutex_lock(n->queueMutex);
    //queueitems = n->playList.size();
    //pthread_mutex_unlock(n->queueMutex);

    // Check for keys here
}

/** 
 * Constructor
 * 
 */
Input::Input()
{
    // Setup mutexes
    queueMutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(queueMutex, NULL);

    //////////////////////////////////
    // Setup keyboard file descriptor
    //Common::LOG(LOG_WARN, "%s: Initializing input 'KEYBOARD'", INPUT_PREFIX);
    cout << "Initializing input 'KEYBOARD'" << endl;

    struct termios newsettings;

    keyboardfd = STDIN_FILENO;
    int flags;

    // save old attributes
    tcgetattr(keyboardfd, &kb_startup_settings);

    // copy old attributes to the new ones
    newsettings = kb_startup_settings;

    // invert the ICANON and ECHO attribute
    newsettings.c_lflag &= ~(ICANON | ECHO);

    // set new attributes
    if (tcsetattr(keyboardfd, TCSANOW, &newsettings) == -1)
        cout << "tcsetattr error:" << strerror(errno) << endl;
    //Common::LOG(LOG_ERR, "%s: tcsetattr error: %s", INPUT_PREFIX, strerror(errno));

    // make reads non-blocking
    if ((flags = fcntl(keyboardfd, F_GETFL, 0)) == -1)
        cout << "error getting flags: " << strerror(errno) << endl;
    //Common::LOG(LOG_ERR, "%s: error getting flags: %s", INPUT_PREFIX, strerror(errno));

    else if (fcntl(keyboardfd, F_SETFL, flags | O_RDONLY | O_NONBLOCK) == -1)
        cout << "error setting flags: " << strerror(errno) << endl;
    //Common::LOG(LOG_ERR, "%s: error setting flags: %s", INPUT_PREFIX, strerror(errno));

    //////////////////////////////////
    // Setup mouse file descriptor

    mousefd = open("/dev/input/mice", O_RDWR | O_NONBLOCK, NULL);
    mouse_state = mb1 = mb2 = mb3 = 0;

    // Initialize mouse in imps2 mode
    static unsigned char basic_init[] =
    { 0xF4, 0xF3, 100 };
    static unsigned char ps2_init[] =
    { 0xE6, 0xF4, 0xF3, 100, 0xE8, 3, };
    static unsigned char imps2_init[] =
    { 0xF3, 200, 0xF3, 100, 0xF3, 80, };

    int id = 0;

    if (mousefd != -1)
    {

        /* Do a basic init in case the mouse is confused */
        write_to_mouse(mousefd, basic_init, sizeof(basic_init));

        /* Now try again and make sure we have a PS/2 mouse */
        if (write_to_mouse(mousefd, basic_init, sizeof(basic_init)) == 0)
        {

            /* Try to switch to 3 button mode */
            if (write_to_mouse(mousefd, imps2_init, sizeof(imps2_init)) == 0)
            {

                /* Read the mouse id */
                id = read_mouse_id(mousefd);
                if (id == -1)
                    id = 0; // id error

                /* And do the real initialisation */
                if (write_to_mouse(mousefd, ps2_init, sizeof(ps2_init)) == 0)
                {

                    if (id == 3)
                    { // IMPS2 mouse
                        //Common::LOG(LOG_WARN, "%s: Initializing input 'IMPS2 MOUSE'", INPUT_PREFIX);
                        cout << "Initializing input 'IMPS2 MOUSE'" << endl;
                        mousetype = MOUSE_TYPE_IMPS2;
                    }
                    else
                    {
                        if (id == 0)
                        { // PS2 mouse
                            //Common::LOG(LOG_WARN, "%s: Initializing input 'PS2 MOUSE'", INPUT_PREFIX);
                            cout << "Initializing input 'PS2 MOUSE'" << endl;
                            mousetype = MOUSE_TYPE_PS2;
                        }
                        else
                        {
                            //Common::LOG(LOG_INFO, "%s: No mouse found", INPUT_PREFIX);
                            cout << "No mouse found" << endl;
                            mousetype = MOUSE_TYPE_NONE;
                        }
                    }
                }
            }

        }
    } /* ps2 was not found!!! */

    //Common::LOG(LOG_INFO, "%s: Setting up input thread" , INPUT_PREFIX);
    cout << "Setting up input thread" << endl;
    running = true;
    if (pthread_create(&inputThread, NULL, input_thread, this))
    {
        usleep(500000);
    }
}

/** 
 * Destructor 
 * 
 */
Input::~Input()
{
    //Common::LOG(LOG_INFO, "%s: Destructor" , INPUT_PREFIX);
    cout << "InputThread destructor" << endl;

    // Clear the list
    while (!keyQueue.empty())
    {
        //Common::LOG(LOG_INFO, "%s: Removing :'" << keyQueue.front() << "'" , INPUT_PREFIX);
        keyQueue.pop();
    }

    pthread_mutex_lock(queueMutex);
    running = false;
    pthread_mutex_unlock(queueMutex);

    // wait for thread to exit
    pthread_join(inputThread, NULL);

    // restore old terminal attributes
    tcsetattr(keyboardfd, TCSANOW, &kb_startup_settings);
    //ioctl (keyboardfd, KDSKBMODE, kb_startup_mode);

    //close(keyboardfd);
    if (mousefd != 0)
        close(mousefd);

}

int Input::read_mouse_id(int mousefd)
{
    unsigned char c = 0xF2;
    unsigned char id = 0;

    write(mousefd, &c, 1);
    read(mousefd, &c, 1);
    if (c != 0xFA)
    {
        return (-1);
    }
    read(mousefd, &id, 1);

    return (id);
}

int Input::write_to_mouse(int mousefd, unsigned char *data, size_t len)
{
    int i;
    int error = 0;
    for (i = 0; i < len; i++)
    {
        unsigned char c;
        write(mousefd, &data[i], 1);
        read(mousefd, &c, 1);
        if (c != 0xFA)
            error++; // not an ack
    }

    /* flush any left-over input */
    usleep(30000);
    tcflush(mousefd, TCIFLUSH);
    return (error);
}

/** 
 * Get the next key in the queue
 * 
 * @return the key
 */
keyCode Input::getKey()
{

    keyCode key = KEY_NONE;
    pthread_mutex_lock(queueMutex);
    // Pop the topmost key on the queue
    if (keyQueue.size() > 0)
    {
        key = keyQueue.front();
        keyQueue.pop();
    }
    pthread_mutex_unlock(queueMutex);

    //Common::LOG(LOG_INFO, "%s: Returning key: " << key , INPUT_PREFIX);

    return key;
}

/** 
 * Inject a key into the queue
 * 
 * @param key - the key to injeect
 */
void Input::injectKey(keyCode key)
{
    pthread_mutex_lock(queueMutex);
    keyQueue.push(key);
    pthread_mutex_unlock(queueMutex);
}

/** 
 * Check if we have a key waiting on the queue
 * 
 * @return true or false
 */
bool Input::keyWaiting()
{
    bool haveKey = false;

    pthread_mutex_lock(queueMutex);
    // Pop the topmost key on the queue
    if (keyQueue.size() > 0)
    {
        haveKey = true;
    }
    pthread_mutex_unlock(queueMutex);

    return haveKey;

}

/** 
 * Flush the queue
 * 
 */
void Input::flushQueue()
{
    pthread_mutex_lock(queueMutex);

    // Clear the list
    while (!keyQueue.empty())
    {
        //Common::LOG(LOG_INFO, "%s: Removing :'" << keyQueue.front() << "'" , INPUT_PREFIX);
        keyQueue.pop();
    }

    pthread_mutex_unlock(queueMutex);
}

