// --- Author : Moreau Cyril - Spl3en
#pragma once

// ---------- Includes ------------
#include "Utils/Utils.h"
#include "Pixel.h"
#include <stdint.h>
#include <SFML/Graphics.h>

// ---------- Defines -------------
#define RESOLUTION_W 64
#define RESOLUTION_H 32

// Window properties
#define WINDOW_TITLE "CHIP-8 Emulator"
#define WINDOW_FULLSCREEN false

// ------ Structure declaration -------
typedef struct _Graphic
{
	// Screen display buffer
	Pixel pixels [RESOLUTION_W * RESOLUTION_H];

	// true if the display need to be refreshed
	bool update;

	// Emulator CPU memory buffer shared pointer
	uint8_t * memory;

	// Index register shared pointer
	uint16_t * index;

	// SFML window object
	sfRenderWindow *window;

}	Graphic;



// --------- Allocators ---------

/*
 * Description 	: Allocate a new Graphic structure.
 * uint8_t *memory : Pointer to the emulator memory buffer
 * uint16_t *index : Pointer to the index register
 * Return 		: A pointer to an allocated Graphic.
 */
Graphic *
Graphic_new (
	uint8_t *memory,
	uint16_t *index
);

// ----------- Functions ------------

/*
 * Description : Initialize an allocated Graphic structure.
 * Graphic *this : An allocated Graphic to initialize.
 * uint8_t *memory : Pointer to the emulator memory buffer
 * uint16_t *index : Pointer to the index register
 * Return : true on success, false on failure.
 */
bool
Graphic_init (
	Graphic *this,
	uint8_t *memory,
	uint16_t *index
);

/*
 * Description : Unit tests checking if a Graphic is coherent
 * Graphic *this : The instance to test
 * Return : true on success, false on failure
 */
bool
Graphic_test (
	Graphic *this
);


/*
 * Description : Draw the screen buffer to the user screen
 * Graphic *this : An allocated Graphic
 * Return : void
 */
void
Graphic_render (
	Graphic *this
);

/*
 * Description : Draw a sprite in the screen buffer
 * Graphic *this : An allocated Cpu
 * Return : bool, true if a pixel changed from 1 to 0, false otherwise
 */
bool
Graphic_drawSprite (
	Graphic *this,
	uint8_t x,
	uint8_t y,
	uint8_t height
);

/*
 * Description : Clear the screen
 * Graphic *this : An allocated Graphic
 * Return : void
 */
void
Graphic_clearScreen (
	Graphic *this
);

/*
 * Description : Start the render thread
 * Graphic *this : An allocated Graphic
 * Return : void
 */
void
Graphic_startThread (
	Graphic *this
);

// --------- Destructors ----------

/*
 * Description : Free an allocated Graphic structure.
 * Graphic *this : An allocated Graphic to free.
 */
void
Graphic_free (
	Graphic *this
);


