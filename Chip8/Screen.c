#include "Screen.h"
#include <stdlib.h>

// ---------- Debugging -------------
#define __DEBUG_OBJECT__ "Screen"
#include "dbg/dbg.h"

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
) {
	Screen *this;

	if ((this = calloc (1, sizeof(Screen))) == NULL)
		return NULL;

	if (!Screen_init (this, memory, index)) {
		Screen_free (this);
		return NULL;
	}

	return this;
}


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
) {
	// Shared pointers
	this->index = index;
	this->memory = memory;

	// Clear the screen
	Screen_clear (this);

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

	// Activate vertical sync
	sfRenderWindow_setVerticalSyncEnabled (this->window, true);

	// Enable rendering in a separate thread
	sfRenderWindow_setActive (this->window, false);

	// Load from a font file on disk
	if ((this->font = sfFont_createFromFile("verdana.ttf")) == NULL) {
		dbg ("Font loading error.");
		return false;
	}

	// Get a profiler
	this->profiler = ProfilerFactory_getProfiler ("Screen");

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
 * Screen *this : An allocated Screen
 * Return : void
 */
void
Screen_clear (
	Screen *this
) {
	for (int i = 0; i < RESOLUTION_W * RESOLUTION_H; i++) {
		Pixel_setValue (&this->pixels[i], 0);
	}
}


/*
 * Description : Draw the screen buffer to the user screen
 * Screen *this : An allocated Screen
 * Return : void
 */
void
Screen_loop (
	Screen *this
) {
	sfEvent event;

	// Information for displaying profilers
	int profilersArraySize;
	Profiler **profilersArray = ProfilerFactory_getArray (&profilersArraySize);

    // Rendering loop
    while (sfRenderWindow_isOpen (this->window))
    {
    	// Increment frame counter
    	Profiler_tick (this->profiler);

		// Poll SFML window events
        while (sfRenderWindow_pollEvent (this->window, &event)) {
            if (event.type == sfEvtClosed) {
                sfRenderWindow_close (this->window);
            }
        }

		// Draw screen
		for (int pos = 0; pos < RESOLUTION_H * RESOLUTION_W; pos++) {
			Pixel *pixel = &this->pixels[pos];
			sfRenderWindow_drawRectangleShape (this->window, pixel->rect, NULL);
		}

		// Draw profiling information
		for (int i = 0; i < profilersArraySize; i++)
		{
			Profiler *profiler = profilersArray[i];

			// Compute tick per second
			if (Profiler_getTime (profiler) >= 1.0f) {
				Profiler_update (profiler);
				Profiler_restart (profiler);
			}

			sfRenderWindow_drawText (this->window, profiler->text, NULL);
		}

		sfRenderWindow_display (this->window);

		// Sleep a bit so the CPU doesn't burn
		sfSleep(sfMilliseconds(1));
    }
}

/*
 * Description : Start the main loop of the screen rendering in a separate thread.
 * Screen *this : An allocated Screen
 * Return : void
 */
void
Screen_startThread (
	Screen *this
) {
	sfThread *thread = sfThread_create ((void (*)(void*)) Screen_loop, this);
	sfThread_launch (thread);
}


/*
 * Description : Draw a sprite in the screen buffer
 * Screen *this : An allocated Screen
 * Return : bool, true if a pixel changed from 1 to 0, false otherwise
 */
bool
Screen_drawSprite (
	Screen *this,
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

	return result;
}


/*
 * Description : Free an allocated Screen structure.
 * Screen *this : An allocated Screen to free.
 */
void
Screen_free (
	Screen *this
) {
	if (this != NULL)
	{
		free (this);
	}
}


/*
 * Description : Unit tests checking if a Screen is coherent
 * Screen *this : The instance to test
 * Return : true on success, false on failure
 */
bool
Screen_test (
	Screen *this
) {

	return true;
}
