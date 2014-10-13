#include "Window.h"
#include <stdlib.h>

// ---------- Debugging -------------
#define __DEBUG_OBJECT__ "Window"
#include "dbg/dbg.h"

// Singleton
Window *g_this = NULL;

/*
 * Description 	: Allocate a new Window structure.
 * Return		: A pointer to an allocated Window.
 */
Window *
Window_new (void)
{
	Window *this;

	if (g_this != NULL) {
		dbg ("Error : A window has already been instanciated.");
		return NULL;
	}

	if ((this = calloc (1, sizeof(Window))) == NULL)
		return NULL;

	if (!Window_init (this)) {
		Window_free (this);
		return NULL;
	}

	g_this = this;

	return this;
}


/*
 * Description : Initialize an allocated Window structure.
 * Window *this : An allocated Window to initialize.
 * Return : true on success, false on failure.
 */
bool
Window_init (
	Window *this
) {
	// Configure SFML RenderWindow
	this->sfmlWindow = sfRenderWindow_create (
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
	sfRenderWindow_setVerticalSyncEnabled (this->sfmlWindow, true);
	sfRenderWindow_setActive (this->sfmlWindow, false);

	// Reset keys state
	memset (this->keysState, KEY_RELEASED, sizeof (this->keysState));

	// Initialize the profiler
	this->profiler = ProfilerFactory_getProfiler ("Window");

	// Ready state
	this->isRunning = true;

	return true;
}


/*
 * Description : Request the Window to emit a beep
 * Window *this : An allocated Window
 * Return : void
 */
void
Window_requestBeep (void) {
	g_this->beepRequest = true;
}


/*
 * Description : Set a key at a given state
 * C8KeyCode code : The key targeted
 * KeyState state : The new state of the key
 * Return : void
 */
void
Window_setKeyState (
	C8KeyCode code,
	KeyState state
) {
	g_this->keysState[code] = state;
}


/*
 * Description : Retrieve the state of a given key
 * C8KeyCode code : The requested key
 * Return : keyState
 */
KeyState
Window_requestKeyState (
	C8KeyCode code
) {
	return g_this->keysState[code];
}


/*
 * Description : Start the main loop of the Window in a separate thread.
 * Window *this : An allocated Window
 * Return : sfThread * Thread object pointer
 */
sfThread *
Window_startThread (
	Window *this
) {
	this->thread = sfThread_create ((void (*)(void*)) Window_loop, this);
	sfThread_launch (this->thread);

	return this->thread;
}


/*
 * Description : Stop the separate thread for the Window
 * Window *this : An allocated Window
 * Return : void
 */
void
Window_stopThread (
	Window *this
) {
	this->isRunning = false;
	sfThread_wait (this->thread);
}


/*
 * Description :
 * Window *this : An allocated Window
 * Return : void
 */
void
Window_loop (
	Window *this
) {
	sfEvent event;

	// Association CHIP-8 keycode <-> SFML KeyCode
	C8KeyCode sfmlToC8Codes [] = {
		[sfKeyNum1] = keyCode_1,
		[sfKeyNum2] = keyCode_2,
		[sfKeyNum3] = keyCode_3,
		[sfKeyNum4] = keyCode_4,
		[sfKeyA] = keyCode_A,
		[sfKeyZ] = keyCode_Z,
		[sfKeyE] = keyCode_E,
		[sfKeyR] = keyCode_R,
		[sfKeyQ] = keyCode_Q,
		[sfKeyS] = keyCode_S,
		[sfKeyD] = keyCode_D,
		[sfKeyC] = keyCode_C,
		[sfKeyW] = keyCode_W,
		[sfKeyX] = keyCode_X,
		[sfKeyF] = keyCode_F,
		[sfKeyV] = keyCode_V
	};

	// Poll SFML window events
	while (this->isRunning)
	{
		Profiler_tick (this->profiler);

        while (sfRenderWindow_pollEvent (this->sfmlWindow, &event))
        {
			switch (event.type)
			{
				case sfEvtClosed:
					this->isRunning = false;
					break;
				break;

				case sfEvtKeyPressed:
				case sfEvtKeyReleased:
					switch (event.key.code) {
						case sfKeyEscape:
							// ESCAPE : Quit
							this->isRunning = false;
							break;
						break;

						case sfKeyNum1:
						case sfKeyNum2:
						case sfKeyNum3:
						case sfKeyNum4:
						case sfKeyA:
						case sfKeyZ:
						case sfKeyE:
						case sfKeyR:
						case sfKeyQ:
						case sfKeyS:
						case sfKeyD:
						case sfKeyW:
						case sfKeyX:
						case sfKeyC:
						case sfKeyV: {
							C8KeyCode code = sfmlToC8Codes[event.key.code];
							if (event.type == sfEvtKeyPressed) {
								switch (this->keysState[code]) {
									case KEY_PRESSED:
										// Don't accept inputs already pushed, set the key state in a waiting state
										this->keysState[code] = KEY_PUSHED;
									break;

									case KEY_RELEASED:
										this->keysState[code] = KEY_PRESSED;
									break;

									default: // KEY_PUSHED : Do nothing
									break;
								}
							} else {
								this->keysState[code] = KEY_RELEASED;
							}
						}
						break;

						default:
							dbg ("Warning : keycode = '%x' unhandled", event.key.code);
						break;
					}
				break;

				default :
				break;
			}
        }

		// Check if a beep is requested
		if (this->beepRequest) {
			#ifdef WIN32
				Beep (440, 120);
			#else
				dbg ("Beep !");
			#endif

			this->beepRequest = false;
		}

		// Sleep a bit so the CPU doesn't burn
		sfSleep(sfMilliseconds(1));
	}

}

/*
 * Description : Free an allocated Window structure.
 * Window *this : An allocated Window to free.
 */
void
Window_free (
	Window *this
) {
	if (this != NULL)
	{
		free (this);
	}
}


/*
 * Description : Unit tests checking if a Window is coherent
 * Window *this : The instance to test
 * Return : true on success, false on failure
 */
bool
Window_test (
	Window *this
) {

	return true;
}
