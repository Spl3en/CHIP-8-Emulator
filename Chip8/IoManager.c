#include "IoManager.h"
#include <SFML/Window.h>

// ---------- Debugging -------------
#define __DEBUG_OBJECT__ "IoManager"
#include "dbg/dbg.h"

/*
 * Description 	: Allocate a new IoManager structure.
 * Screen *screen : Initialized screen component
 * Return		: A pointer to an allocated IoManager.
 */
IoManager *
IoManager_new (
	Screen *screen
) {
	IoManager *this;

	if ((this = calloc (1, sizeof(IoManager))) == NULL)
		return NULL;

	if (!IoManager_init (this, screen)) {
		IoManager_free (this);
		return NULL;
	}

	return this;
}


/*
 * Description : Initialize an allocated IoManager structure.
 * IoManager *this : An allocated IoManager to initialize.
 * Screen *screen : Initialized screen component
 * Return : true on success, false on failure.
 */
bool
IoManager_init (
	IoManager *this,
	Screen *screen
) {
	// Reset keys state
	memset(this->keysState, 0, sizeof (this->keysState));

	// Get a pointer copy of the screen component
	this->screen = screen;

	// Start listening
	this->listening = true;

	// Initialize the profiler
	this->profiler = ProfilerFactory_getProfiler ("IoManager");

	return true;
}


/*
 * Description : Loop listening for i/o events
 * IoManager *this : An allocated IoManager
 * Return : void
 */
void
IoManager_loop (
	IoManager *this
) {
	// Association CHIP-8 keycode <-> SFML KeyCode
	sfKeyCode sfmlKeyCodes[] = {
		[keyCode_X] = sfKeyX,
		[keyCode_1] = sfKeyNum1,
		[keyCode_2] = sfKeyNum2,
		[keyCode_3] = sfKeyNum3,
		[keyCode_A] = sfKeyA,
		[keyCode_Z] = sfKeyZ,
		[keyCode_E] = sfKeyE,
		[keyCode_Q] = sfKeyQ,
		[keyCode_S] = sfKeyS,
		[keyCode_D] = sfKeyD,
		[keyCode_W] = sfKeyW,
		[keyCode_C] = sfKeyC,
		[keyCode_4] = sfKeyNum4,
		[keyCode_R] = sfKeyR,
		[keyCode_F] = sfKeyF,
		[keyCode_V] = sfKeyV
	};

	while (sfRenderWindow_isOpen (this->screen->window))
	{
		Profiler_tick (this->profiler);

		if (sfKeyboard_isKeyPressed (sfKeyEscape)) {
			// ESCAPE : Quit
			sfRenderWindow_close (this->screen->window);
		}

		// Check if a key is pressed
		for (int code = 0; code < keyCodeCount; code++) {
			this->keysState[code] = sfKeyboard_isKeyPressed (sfmlKeyCodes[code]);
		}

		// Sleep a bit so the CPU doesn't burn
		sfSleep(sfMilliseconds(1));
	}
}

/*
 * Description : Start the main loop of the IoManager in a separate thread.
 * IoManager *this : An allocated IoManager
 * Return : void
 */
void
IoManager_startThread (
	IoManager *this
) {
	sfThread *thread = sfThread_create ((void (*)(void*)) IoManager_loop, this);
	sfThread_launch (thread);
}

/*
 * Description : Free an allocated IoManager structure.
 * IoManager *this : An allocated IoManager to free.
 */
void
IoManager_free (
	IoManager *this
) {
	if (this != NULL)
	{
		free (this);
	}
}


/*
 * Description : Unit tests checking if a IoManager is coherent
 * IoManager *this : The instance to test
 * Return : true on success, false on failure
 */
bool
IoManager_test (
	IoManager *this
) {

	return true;
}
