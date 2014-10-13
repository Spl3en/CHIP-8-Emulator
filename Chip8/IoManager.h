// --- Author : Moreau Cyril - Spl3en
#pragma once

// ---------- Includes ------------
#include "Utils/Utils.h"
#include "Screen.h"
#include <stdint.h>

// ---------- Defines -------------
#define KEYS_COUNT 16


// ------ Structure declaration -------
typedef struct _IoManager
{
	// Keys states
	uint8_t keysState [KEYS_COUNT];

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

/*
 *	Key associated with each keycode
 */
typedef enum {
	keyCode_X = 0x0,
	keyCode_1 = 0x1,
	keyCode_2 = 0x2,
	keyCode_3 = 0x3,
	keyCode_A = 0x4,
	keyCode_Z = 0x5,
	keyCode_E = 0x6,
	keyCode_Q = 0x7,
	keyCode_S = 0x8,
	keyCode_D = 0x9,
	keyCode_W = 0xA,
	keyCode_C = 0xB,
	keyCode_4 = 0xC,
	keyCode_R = 0xD,
	keyCode_F = 0xE,
	keyCode_V = 0xF,

	keyCodeCount // Always at the end
} keyCode;

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


