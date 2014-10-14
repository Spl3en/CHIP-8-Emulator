#include "Screen.h"
#include <stdlib.h>

// ---------- Debugging -------------
#define __DEBUG_OBJECT__ "Screen"
#include "dbg/dbg.h"

/*
 * Description     : Allocate a new Screen structure.
 * sfRenderWindow *sfmlWindow : A SFML render window context
 * Return         : A pointer to an allocated Screen.
 */
Screen *
Screen_new (
    sfRenderWindow *sfmlWindow
) {
    Screen *this;

    if ((this = calloc (1, sizeof(Screen))) == NULL)
        return NULL;

    if (!Screen_init (this, sfmlWindow)) {
        Screen_free (this);
        return NULL;
    }

    return this;
}


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
) {
    // Get a profiler
    if (!(this->profiler = ProfilerFactory_getProfiler ("Screen"))) {
        dbg ("Cannot allocate a new Profiler.");
        return false;
    }

    // Share the sfmlWindow pointer
    this->window = sfmlWindow;

    // Initialize the pixels array
    for (int y = 0, id = 0; y < RESOLUTION_H; y++) {
        for (int x = 0; x < RESOLUTION_W; x++, id++) {
            if (!(this->pixels[id] = Pixel_new (x, y))) {
                dbg ("Cannot allocate a new Pixel.");
                return false;
            }
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
        Pixel_setValue (this->pixels[i], PIXEL_BLACK);
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
            printf ((this->pixels[(y * RESOLUTION_W) + x]->value == PIXEL_WHITE) ? "x" : " ");
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
    // Information for displaying profilers
    int profilersArraySize;
    Profiler **profilersArray = ProfilerFactory_getArray (&profilersArraySize);

	// Scan lines effect
	sfRectangleShape *scanLines[(RESOLUTION_H * PIXEL_SIZE) / 2];
	for (int i = 0; i < sizeof_array(scanLines); i++) {
		scanLines[i] = sfRectangleShape_create();
		sfRectangleShape_setPosition (scanLines[i], (sfVector2f){.x = 0, .y = i*2});
		sfRectangleShape_setSize (scanLines[i], (sfVector2f){.x = RESOLUTION_W * PIXEL_SIZE, .y=1});
		sfRectangleShape_setFillColor (scanLines[i], sfColor_fromRGBA(0, 0, 0, 100));
	}

    // Rendering loop
    while (this->isRunning)
    {
        // Increment frame counter
        Profiler_tick (this->profiler);

        // Draw screen
        for (int pos = 0; pos < RESOLUTION_H * RESOLUTION_W; pos++) {
            Pixel *pixel = this->pixels[pos];
            sfRenderWindow_drawRectangleShape (this->window, pixel->rect, NULL);
        }

		// Draw scan lines
        for (int i = 0; i < sizeof_array(scanLines); i++) {
			sfRenderWindow_drawRectangleShape(this->window, scanLines[i], NULL);
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

    // Request to close the window
    sfRenderWindow_close (this->window);
}


/*
 * Description : Start the main loop of the screen rendering in a separate thread.
 * Screen *this : An allocated Screen
 */
void
Screen_startThread (
    Screen *this
) {
    this->thread = sfThread_create ((void (*)(void*)) Screen_loop, this);
    sfThread_launch (this->thread);
}


/*
 * Description : Stop the separate thread for the Screen
 * Screen *this : An allocated Screen
 * Return : void
 */
void
Screen_stopThread (
    Screen *this
) {
    this->isRunning = false;
    sfThread_wait (this->thread);
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

    if (height == 0) {
		height = 16;
    }

    for (int posY = 0; posY < height; posY++)
    {
        mByte = memory[index + posY];

        for (int posX = 0; posX < 8; posX++)
        {
            if ((mByte & (0x80 >> posX)) != 0)
            {
            	// Check in bound pixel
				if ((x + posX) < RESOLUTION_W
				&&  (y + posY) < RESOLUTION_H
				&&  (x + posX) >= 0
				&&  (y + posY) >= 0)
				{
					Pixel *pixel = this->pixels [x + posX + ((y + posY) * RESOLUTION_W)];

					if (pixel->value == PIXEL_WHITE) {
						// A pixel changed from PIXEL_WHITE to PIXEL_BLACK
						result = true;
					}

					// Invert pixel color
					Pixel_invertColor (pixel);
				}
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
        Profiler_free (this->profiler);
        free (this);
    }
}
