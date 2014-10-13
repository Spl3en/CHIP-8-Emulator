// --- Author : Moreau Cyril - Spl3en
#pragma once

// ---------- Includes ------------
#include "Utils/Utils.h"
#include "Screen.h"
#include <SFML/Graphics.h>
#include <stdint.h>

// ---------- Defines -------------
#define KEYS_COUNT 16

// Window properties
#define WINDOW_TITLE         "CHIP-8 Emulator"
#define WINDOW_FULLSCREEN     false


// ------ Structure declaration -------

/*
 *    Key associated with each keycode
 */
typedef enum {
    keyCode_X = 0x0,
    keyCode_1 = 0x1,
    keyCode_2 = 0x2,
    keyCode_3 = 0x3,
    keyCode_A = 0x4,
    keyCode_Z = 0x5,
    keyCode_E = 0x6,
    keyCode_Q = 0x7,
    keyCode_S = 0x8,
    keyCode_D = 0x9,
    keyCode_W = 0xA,
    keyCode_C = 0xB,
    keyCode_4 = 0xC,
    keyCode_R = 0xD,
    keyCode_F = 0xE,
    keyCode_V = 0xF,

    keyCodeCount // Always at the end
} C8KeyCode;

typedef enum {

    KEY_RELEASED,
    KEY_PRESSED,
    KEY_PUSHED

} KeyState;

typedef struct _Window
{
    // SFML window object
    sfRenderWindow *sfmlWindow;

    // Keys states
    uint8_t keysState [KEYS_COUNT];

    // Running state
    bool isRunning;

    // Flag true if CPU requested a beep
    bool beepRequest;

    // Thread object pointer
    sfThread *thread;

    // Profiler
    Profiler * profiler;

}    Window;



// --------- Allocators ---------

/*
 * Description     : Allocate a new Window structure.
 * Return        : A pointer to an allocated Window.
 */
Window *
Window_new (void);

// ----------- Functions ------------

/*
 * Description : Initialize an allocated Window structure.
 * Window *this : An allocated Window to initialize.
 */
bool
Window_init (
    Window *this
);

/*
 * Description : Unit tests checking if a Window is coherent
 * Window *this : The instance to test
 * Return : true on success, false on failure
 */
bool
Window_test (
    Window *this
);

/*
 * Description :
 * Window *this : An allocated Window
 * Return : void
 */
void
Window_loop (
    Window *this
);

/*
 * Description : Stop the separate thread for the Window
 * Window *this : An allocated Window
 * Return : void
 */
void
Window_stopThread (
    Window *this
);

/*
 * Description : Start the main loop of the Window in a separate thread.
 * Window *this : An allocated Window
 * Return : sfThread * Thread object pointer
 */
sfThread *
Window_startThread (
    Window *this
);

/*
 * Description : Request the Window to emit a beep
 * Return : void
 */
void
Window_requestBeep (void);

/*
 * Description : Retrieve the state of a given key
 * C8KeyCode code : The requested key
 * Return : keyState
 */
KeyState
Window_requestKeyState (
    C8KeyCode code
);

/*
 * Description : Set a key at a given state
 * C8KeyCode code : The key targeted
 * KeyState state : The new state of the key
 * Return : void
 */
void
Window_setKeyState (
    C8KeyCode code,
    KeyState state
);

// --------- Destructors ----------

/*
 * Description : Free an allocated Window structure.
 * Window *this : An allocated Window to free.
 */
void
Window_free (
    Window *this
);


