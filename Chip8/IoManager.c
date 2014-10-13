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
	while (this->isRunning)
	{
		Profiler_tick (this->profiler);

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
