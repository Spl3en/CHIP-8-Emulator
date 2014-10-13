// --- Author : Moreau Cyril - Spl3en
#pragma once

// ---------- Includes ------------
#include "Utils/Utils.h"
#include "Screen.h"
#include <stdint.h>

// ---------- Defines -------------


// ------ Structure declaration -------
typedef struct _IoManager
{

	// Loop state
	bool listening;

	// Profiler
	Profiler * profiler;

	// Running thread state
	bool isRunning;

	// Thread object pointer
	sfThread *thread;

	// CPU requested a beep
	bool beepRequest;

}	IoManager;

// --------- Allocators ---------

/*
 * Description 	: Allocate a new IoManager structure.
 * Return		: A pointer to an allocated IoManager.
 */
IoManager *
IoManager_new (void);

// ----------- Functions ------------

/*
 * Description : Initialize an allocated IoManager structure.
 * IoManager *this : An allocated IoManager to initialize.
 * Return : true on success, false on failure.
 */
bool
IoManager_init (
	IoManager *this
);

/*
 * Description : Start the main loop of the IoManager in a separate thread.
 * IoManager *this : An allocated IoManager
 * Return : sfThread * Thread object pointer
 */
sfThread *
IoManager_startThread (
	IoManager *this
);

/*
 * Description : Stop the separate thread for the IoManager
 * IoManager *this : An allocated IoManager
 * Return : void
 */
void
IoManager_stopThread (
	IoManager *this
);

/*
 * Description :
 * IoManager *this : An allocated IoManager
 * Return : void
 */
void
IoManager_loop (
	IoManager *this
);

/*
 * Description :
 * IoManager *this : An allocated IoManager
 * Return : void
 */
void
IoManager_requestBeep (
	IoManager *this
);

// --------- Destructors ----------

/*
 * Description : Free an allocated IoManager structure.
 * IoManager *this : An allocated IoManager to free.
 */
void
IoManager_free (
	IoManager *this
);


