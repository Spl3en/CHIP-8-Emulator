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

	this->value = 0;

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
		free (this);
	}
}


/*
 * Description : 	Set a new value to a pixel on the screen.
					If its value doesn't change, do nothing.
 * Pixel *this : An allocated Pixel
 * Return : bool true on pixel value changed, false otherwise
 */
bool
Pixel_setValue (
	Pixel *this,
	uint8_t value
) {
	if (this->value != value) {
		this->value = value;
		this->update = true;
		sfRectangleShape_setFillColor (this->rect, (value) ? sfWhite : sfBlack);
		return true;
	}

	return false;
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
