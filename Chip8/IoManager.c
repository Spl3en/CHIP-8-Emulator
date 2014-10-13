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
) {
	IoManager *this;

	if ((this = calloc (1, sizeof(IoManager))) == NULL)
		return NULL;

	if (!IoManager_init (this)) {
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
	IoManager *this
) {
	// Reset keys state
	memset(this->keysState, 0, sizeof (this->keysState));

	// Start listening
	this->listening = true;

	// Initialize the profiler
	this->profiler = ProfilerFactory_getProfiler ("IoManager");

	// Ready state
	this->isRunning = true;

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
		[keyCode_1] = sfKeyNum1,
		[keyCode_2] = sfKeyNum2,
		[keyCode_3] = sfKeyNum3,
		[keyCode_4] = sfKeyNum4,
		[keyCode_A] = sfKeyA,
		[keyCode_Z] = sfKeyZ,
		[keyCode_E] = sfKeyE,
		[keyCode_R] = sfKeyR,
		[keyCode_Q] = sfKeyQ,
		[keyCode_S] = sfKeyS,
		[keyCode_D] = sfKeyD,
		[keyCode_C] = sfKeyC,
		[keyCode_W] = sfKeyW,
		[keyCode_X] = sfKeyX,
		[keyCode_F] = sfKeyF,
		[keyCode_V] = sfKeyV
	};

	while (this->isRunning)
	{
		Profiler_tick (this->profiler);

		// Check if a key is pressed
		for (int code = 0; code < keyCodeCount; code++) {
			this->keysState[code] = sfKeyboard_isKeyPressed (sfmlKeyCodes[code]);
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
 * Description : Start the main loop of the IoManager in a separate thread.
 * IoManager *this : An allocated IoManager
 * Return : sfThread * Thread object pointer
 */
sfThread *
IoManager_startThread (
	IoManager *this
) {
	this->thread = sfThread_create ((void (*)(void*)) IoManager_loop, this);
	sfThread_launch (this->thread);

	return this->thread;
}


/*
 * Description :
 * IoManager *this : An allocated IoManager
 * Return : void
 */
void
IoManager_requestBeep (
	IoManager *this
) {
	this->beepRequest = true;
}


/*
 * Description : Stop the separate thread for the IoManager
 * IoManager *this : An allocated IoManager
 * Return : void
 */
void
IoManager_stopThread (
	IoManager *this
) {
	this->isRunning = false;
	sfThread_wait (this->thread);
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
		Profiler_free (this->profiler);
		free (this);
	}
}
