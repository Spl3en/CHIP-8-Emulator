#include "Pixel.h"
#include <stdlib.h>

// ---------- Debugging -------------
#define __DEBUG_OBJECT__ "Pixel"
#include "dbg/dbg.h"

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
) {
	Pixel *this;

	if ((this = calloc (1, sizeof(Pixel))) == NULL)
		return NULL;

	if (!Pixel_init (this, x, y)) {
		Pixel_free (this);
		return NULL;
	}

	return this;
}


/*
 * Description : Initialize an allocated Pixel structure.
 * Pixel *this : An allocated Pixel to initialize.
 * int x,
 * int y : Position of the pixel on the virtual screen
 * Return : void
 */
bool
Pixel_init (
	Pixel *this,
	int x, int y
) {
	if ((this->rect = sfRectangleShape_create ()) == NULL) {
		return false;
	}

	this->value = PIXEL_BLACK;

	sfRectangleShape_setPosition  (this->rect, (sfVector2f) {.x = PIXEL_SIZE * x, .y = PIXEL_SIZE * y});
	sfRectangleShape_setSize      (this->rect, (sfVector2f) {.x = PIXEL_SIZE, .y = PIXEL_SIZE});
	sfRectangleShape_setFillColor (this->rect, sfBlack);

	return true;
}


/*
 * Description : Free an allocated Pixel structure.
 * Pixel *this : An allocated Pixel to free.
 */
void
Pixel_free (
	Pixel *this
) {
	if (this != NULL)
	{
		sfRectangleShape_destroy (this->rect);
		free (this);
	}
}


/*
 * Description : Invert the color of a given Pixel
 * Pixel *this : An allocated Pixel
 * Return : void
 */
void
Pixel_invertColor (
	Pixel *this
) {
	Pixel_setValue (this, (this->value == PIXEL_WHITE) ? PIXEL_BLACK : PIXEL_WHITE);
}


/*
 * Description : 	Set a new value to a pixel on the screen.
					If its value doesn't change, do nothing.
 * Pixel *this : An allocated Pixel
 * PixelValue value : The new pixel value
 * Return : void
 */
void
Pixel_setValue (
	Pixel *this,
	PixelValue value
) {
	if (this->value != value) {
		this->value = value;
		sfRectangleShape_setFillColor (this->rect, (value == PIXEL_WHITE) ? sfWhite : sfBlack);
	}
}


/*
 * Description : Unit tests checking if a Pixel is coherent
 * Pixel *this : The instance to test
 * Return : true on success, false on failure
 */
bool
Pixel_test (
	Pixel *this
) {

	return true;
}
