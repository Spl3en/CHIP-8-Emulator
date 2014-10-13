// --- Author : Moreau Cyril - Spl3en
#pragma once

// ---------- Includes ------------
#include "Utils/Utils.h"
#include "Pixel.h"
#include "Profiler/ProfilerFactory.h"
#include <stdint.h>
#include <SFML/Graphics.h>

// ---------- Defines -------------
#define RESOLUTION_W 64
#define RESOLUTION_H 32

// Window properties
#define WINDOW_TITLE "CHIP-8 Emulator"
#define WINDOW_FULLSCREEN false

// ------ Structure declaration -------
typedef struct _Screen
{
	// Screen display buffer
	Pixel pixels [RESOLUTION_W * RESOLUTION_H];

	// Emulator CPU memory buffer shared pointer
	uint8_t * memory;

	// Index register shared pointer
	uint16_t * index;

	// SFML window object
	sfRenderWindow *window;

	// Font used for displaying info
	sfFont *font;

	// Profiler for the Screen display
	Profiler * profiler;

}	Screen;



// --------- Allocators ---------

/*
 * Description 	: Allocate a new Screen structure.
 * uint8_t *memory : Pointer to the emulator memory buffer
 * uint16_t *index : Pointer to the index register
 * Return 		: A pointer to an allocated Screen.
 */
Screen *
Screen_new (
	uint8_t *memory,
	uint16_t *index
);

// ----------- Functions ------------

/*
 * Description : Initialize an allocated Screen structure.
 * Screen *this : An allocated Screen to initialize.
 * uint8_t *memory : Pointer to the emulator memory buffer
 * uint16_t *index : Pointer to the index register
 * Return : true on success, false on failure.
 */
bool
Screen_init (
	Screen *this,
	uint8_t *memory,
	uint16_t *index
);

/*
 * Description : Unit tests checking if a Screen is coherent
 * Screen *this : The instance to test
 * Return : true on success, false on failure
 */
bool
Screen_test (
	Screen *this
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
 * Description : Draw a sprite in the screen buffer
 * Screen *this : An allocated Cpu
 * Return : bool, true if a pixel changed from 1 to 0, false otherwise
 */
bool
Screen_drawSprite (
	Screen *this,
	uint8_t x,
	uint8_t y,
	uint8_t height
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
 * Return : sfThread * Thread object pointer
 */
sfThread *
Screen_startThread (
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


