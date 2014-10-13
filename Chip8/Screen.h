// --- Author : Moreau Cyril - Spl3en
#pragma once

// ---------- Includes ------------
#include "Utils/Utils.h"
#include "Pixel.h"
#include "Profiler/ProfilerFactory.h"
#include <SFML/Graphics.h>

// ---------- Defines -------------
#define RESOLUTION_W 64
#define RESOLUTION_H 32

// ------ Structure declaration -------
typedef struct _Screen
{
    // Screen display buffer
    Pixel * pixels [RESOLUTION_W * RESOLUTION_H];

    // SFML window object shared with Window
    sfRenderWindow *window;

    // Profiler for the Screen display
    Profiler * profiler;

    // Running state
    bool isRunning;

    // Thread object pointer
    sfThread *thread;

}    Screen;



// --------- Allocators ---------

/*
 * Description     : Allocate a new Screen structure.
 * sfRenderWindow *sfmlWindow : A SFML render window context
 * Return         : A pointer to an allocated Screen.
 */
Screen *
Screen_new (
    sfRenderWindow *sfmlWindow
);

// ----------- Functions ------------

/*
 * Description : Initialize an allocated Screen structure.
 * Screen *this : An allocated Screen to initialize.
 * sfRenderWindow *sfmlWindow : A SFML render window context
 * Return : true on success, false on failure.
 */
bool
Screen_init (
    Screen *this,
    sfRenderWindow *sfmlWindow
);

/*
 * Description : Draw a sprite in the screen buffer
 * Screen *this : An allocated Screen
 * uint8_t x : Position X on the screen of the sprite
 * uint8_t y : Position Y on the screen of the sprite
 * uint8_t height : Height of the sprite
 * uint8_t *memory : CPU memory pointer (it loads pixels from memory)
 * uint16_t index : Index register
 * Return : bool, true if a pixel changed from 1 to 0, false otherwise
 */
bool
Screen_drawSprite (
    Screen *this,
    uint8_t x,
    uint8_t y,
    uint8_t height,
    uint8_t *memory,
    uint16_t index
);

/*
 * Description : Draw the screen buffer to the user screen
 * Screen *this : An allocated Screen
 * Return : void
 */
void
Screen_render (
    Screen *this
);

/*
 * Description : Clear the screen
 * Screen *this : An allocated Screen
 * Return : void
 */
void
Screen_clear (
    Screen *this
);

/*
 * Description : Start the main loop of the screen rendering in a separate thread.
 * Screen *this : An allocated Screen
 */
void
Screen_startThread (
    Screen *this
);

/*
 * Description : Stop the separate thread for the Screen
 * Screen *this : An allocated Screen
 * Return : void
 */
void
Screen_stopThread (
    Screen *this
);

/*
 * Description : Draw the screen buffer to the user screen
 * Screen *this : An allocated Screen
 * Return : void
 */
void
Screen_loop (
    Screen *this
);

// --------- Destructors ----------

/*
 * Description : Free an allocated Screen structure.
 * Screen *this : An allocated Screen to free.
 */
void
Screen_free (
    Screen *this
);


