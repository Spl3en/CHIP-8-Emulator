// --- Author : Moreau Cyril - Spl3en
#pragma once

// ---------- Includes ------------
#include "Utils/Utils.h"
#include <SFML/Graphics.h>
#include <stdint.h>

// ---------- Defines -------------
#define PIXEL_SIZE 16


// ------ Structure declaration -------
typedef enum {
	PIXEL_BLACK = 0,
	PIXEL_WHITE = 1,
} PixelValue;

typedef struct _Pixel
{
	sfRectangleShape *rect;
	PixelValue value;

} 	Pixel;


// --------- Allocators ---------

/*
 * Description 	: Allocate a new Pixel structure.
 * int x,
 * int y : Position of the pixel on the virtual screen
 * Return		: A pointer to an allocated Pixel.
 */
Pixel *
Pixel_new (
	int x,
	int y
);

// ----------- Functions ------------


/*
 * Description : Initialize an allocated Pixel structure.
 * Pixel *this : An allocated Pixel to initialize.
 * int x, int y : Position of the pixel on the virtual screen
 * Return : void
 */
bool
Pixel_init (
	Pixel *this,
	int x, int y
);

/*
 * Description : Unit tests checking if a Pixel is coherent
 * Pixel *this : The instance to test
 * Return : true on success, false on failure
 */
bool
Pixel_test (
	Pixel *this
);

/*
 * Description : Invert the color of a given Pixel
 * Pixel *this : An allocated Pixel
 * Return : void
 */
void
Pixel_invertColor (
	Pixel *this
);

/*
 * Description : 	Set a new value to a pixel on the screen.
					If its value doesn't change, do nothing.
 * Pixel *this : An allocated Pixel
 * Return : void
 */
void
Pixel_setValue (
	Pixel *this,
	PixelValue value
);

// --------- Destructors ----------

/*
 * Description : Free an allocated Pixel structure.
 * Pixel *this : An allocated Pixel to free.
 */
void
Pixel_free (
	Pixel *this
);


