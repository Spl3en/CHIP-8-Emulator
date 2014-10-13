#include "Screen.h"
#include <stdlib.h>

// ---------- Debugging -------------
#define __DEBUG_OBJECT__ "Screen"
#include "dbg/dbg.h"

/*
 * Description 	: Allocate a new Screen structure.
 * Return 		: A pointer to an allocated Screen.
 */
Screen *
Screen_new (void) {
	Screen *this;

	if ((this = calloc (1, sizeof(Screen))) == NULL)
		return NULL;

	if (!Screen_init (this)) {
		Screen_free (this);
		return NULL;
	}

	return this;
}


/*
 * Description : Initialize an allocated Screen structure.
 * Screen *this : An allocated Screen to initialize.
 * Return : true on success, false on failure.
 */
bool
Screen_init (
	Screen *this
) {
	// Configure SFML RenderWindow
	this->window = sfRenderWindow_create (
		(sfVideoMode) {
			.width  = RESOLUTION_W * PIXEL_SIZE,
			.height = RESOLUTION_H * PIXEL_SIZE,
			.bitsPerPixel = 32
		},
		WINDOW_TITLE,
		(WINDOW_FULLSCREEN) ? sfFullscreen : sfNone,
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
			this->pixels[id] = Pixel_new (x, y);
		}
	}

	// Clear the screen
	Screen_clear (this);

	// Ready state
	this->isRunning = true;

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
		Pixel_setValue (this->pixels[i], 0);
	}
}


/*
 * Description : Debug the screen buffer in the console
 * Screen *this : An allocated Screen
 * Return : void
 */
void Screen_debug (
	Screen *this
) {
	for (int y = 0; y < RESOLUTION_H; ++y) {
		for (int x = 0; x < RESOLUTION_W; ++x) {
			printf ((this->pixels[(y * RESOLUTION_W) + x]->value) ? "x" : " ");
		}
		printf ("\n");
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
    while (this->isRunning)
    {
		// Poll SFML window events
        while (sfRenderWindow_pollEvent (this->window, &event)) {
            if (event.type == sfEvtClosed) {
                sfRenderWindow_close (this->window);
                this->isRunning = false;
            }
        }

		// Handle special key events
		if (sfKeyboard_isKeyPressed (sfKeyEscape)) {
			// ESCAPE : Quit
			sfRenderWindow_close (this->window);
            this->isRunning = false;
		}

    	// Increment frame counter
    	Profiler_tick (this->profiler);

		// Draw screen
		for (int pos = 0; pos < RESOLUTION_H * RESOLUTION_W; pos++) {
			Pixel *pixel = this->pixels[pos];
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

		// Request display
		sfRenderWindow_display (this->window);

		// Sleep a bit so the CPU doesn't burn
		sfSleep(sfMilliseconds(1));
    }
}

/*
 * Description : Start the main loop of the screen rendering in a separate thread.
 * Screen *this : An allocated Screen
 * Return : sfThread * Thread object pointer
 */
sfThread *
Screen_startThread (
	Screen *this
) {
	sfThread *thread = sfThread_create ((void (*)(void*)) Screen_loop, this);
	sfThread_launch (thread);

	return thread;
}


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
) {
	bool result = false;
	uint8_t mByte;

	if (x > RESOLUTION_W || y > RESOLUTION_H) {
		dbg ("Warning : drawing out of screen : \n"
			 "(x=0x%x, max=0x%x) | (y=0x%x, max=0x%x)", x, RESOLUTION_W, y, RESOLUTION_H);
		exit (0);
	}

	for (int posY = 0; posY < height; posY++) {
		mByte = memory[index + posY];

		for (int posX = 0; posX < 8; posX++) {
			if ((mByte & (0x80 >> posX)) != 0) {
				Pixel *pixel = this->pixels [x + posX + ((y + posY) * RESOLUTION_W)];

				if (pixel->value == 1) { // A pixel changed from 1 to 0
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
		for (int i = 0; i < RESOLUTION_W * RESOLUTION_H; i++) {
			Pixel_free (this->pixels[i]);
		}

		sfRenderWindow_destroy (this->window);
		sfFont_destroy (this->font);
		Profiler_free (this->profiler);
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
