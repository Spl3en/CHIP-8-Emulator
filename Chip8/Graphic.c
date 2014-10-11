#include "Graphic.h"
#include <stdlib.h>

// ---------- Debugging -------------
#define __DEBUG_OBJECT__ "Graphic"
#include "dbg/dbg.h"

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
) {
	Graphic *this;

	if ((this = calloc (1, sizeof(Graphic))) == NULL)
		return NULL;

	if (!Graphic_init (this, memory, index)) {
		Graphic_free (this);
		return NULL;
	}

	return this;
}


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
) {
	// Shared pointers
	this->index = index;
	this->memory = memory;

	// Clear the screen
	Graphic_clearScreen (this);

	// Update the next frame
	this->update = true;

	// Configure SFML RenderWindow
	this->window = sfRenderWindow_create (
		(sfVideoMode) {
			.width  = RESOLUTION_W * PIXEL_SIZE,
			.height = RESOLUTION_H * PIXEL_SIZE,
			.bitsPerPixel = 32
		},
		WINDOW_TITLE,
		(WINDOW_FULLSCREEN) ? sfFullscreen : sfDefaultStyle,
		(sfContextSettings[]) {{
			.depthBits = 32,
			.stencilBits = 8,
			.antialiasingLevel = 0,
			.majorVersion = 2,
			.minorVersion = 1,
		}}
	);

	sfRenderWindow_setVerticalSyncEnabled (this->window, true);
	sfRenderWindow_setActive (this->window, false);

	/* Initialize the pixels array */
	int id = 0;
	for (int y = 0; y < RESOLUTION_H; y++) {
		for (int x = 0; x < RESOLUTION_W; x++, id++) {
			Pixel_init (&this->pixels[id], x, y);
		}
	}

	return true;
}

/*
 * Description : Clear the screen
 * Graphic *this : An allocated Graphic
 * Return : void
 */
void
Graphic_clearScreen (
	Graphic *this
) {
	for (int i = 0; i < RESOLUTION_W * RESOLUTION_H; i++) {
		Pixel_setValue (&this->pixels[i], 0);
	}

	this->update = true;
}


/*
 * Description : Draw the screen buffer to the user screen
 * Graphic *this : An allocated Graphic
 * Return : void
 */
void
Graphic_render (
	Graphic *this
) {
    // the rendering loop
    while (sfRenderWindow_isOpen(this->window))
    {
		// Only refresh if the screen has been updated
		if (this->update)
		{
			// Draw
			for (int pos = 0; pos < RESOLUTION_H * RESOLUTION_W; pos++) {
				Pixel *pixel = &this->pixels[pos];
				sfRenderWindow_drawRectangleShape (this->window, pixel->rect, NULL);
			}

			// Display
			sfRenderWindow_display (this->window);
		}

		// Sleep a bit so the CPU doesn't burn
		sfSleep(sfMilliseconds(1));
    }
}

/*
 * Description : Start the render thread
 * Graphic *this : An allocated Graphic
 * Return : void
 */
void
Graphic_startThread (
	Graphic *this
) {
	sfThread *thread = sfThread_create ((void (*)(void*)) Graphic_render, this);
	sfThread_launch (thread);
}


/*
 * Description : Draw a sprite in the screen buffer
 * Graphic *this : An allocated Graphic
 * Return : bool, true if a pixel changed from 1 to 0, false otherwise
 */
bool
Graphic_drawSprite (
	Graphic *this,
	uint8_t x,
	uint8_t y,
	uint8_t height
) {
	uint16_t index = *this->index;
	bool result = false;
	uint8_t mempix;

	if (x > RESOLUTION_W || y > RESOLUTION_H) {
		dbg ("Warning : drawing out of screen : \n"
			 "(x=0x%x, max=0x%x) | (y=0x%x, max=0x%x)", x, RESOLUTION_W, y, RESOLUTION_H);
		exit (0);
	}

	for (int yline = 0; yline < height; yline++)
	{
		mempix = this->memory[index + yline];

		for (int xline = 0; xline < 8; xline++)
		{
			if ((mempix & (0x80 >> xline)) != 0)
			{
				Pixel *pixel = &this->pixels [x + xline + ((y + yline) * RESOLUTION_W)];

				if (pixel->value == 1) {
					// A pixel changed from 1 to 0
					result = true;
				}

				Pixel_setValue (pixel, pixel->value ^ 1);
			}
		}
	}

	this->update = true;

	return result;
}


/*
 * Description : Free an allocated Graphic structure.
 * Graphic *this : An allocated Graphic to free.
 */
void
Graphic_free (
	Graphic *this
) {
	if (this != NULL)
	{
		free (this);
	}
}


/*
 * Description : Unit tests checking if a Graphic is coherent
 * Graphic *this : The instance to test
 * Return : true on success, false on failure
 */
bool
Graphic_test (
	Graphic *this
) {

	return true;
}
