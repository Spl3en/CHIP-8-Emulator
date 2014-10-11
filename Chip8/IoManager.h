// --- Author : Moreau Cyril - Spl3en
#pragma once

// ---------- Includes ------------
#include "Utils/Utils.h"
#include <stdint.h>

// ---------- Defines -------------
#define KEYS_COUNT 16


// ------ Structure declaration -------
typedef struct _IoManager
{
	// Keys states
	uint8_t keysState [KEYS_COUNT];

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
 */
bool
IoManager_init (
	IoManager *this
);

/*
 * Description : Unit tests checking if a IoManager is coherent
 * IoManager *this : The instance to test
 * Return : true on success, false on failure
 */
bool
IoManager_test (
	IoManager *this
);

/*
 * Description : Start the main loop of the IoManager in a separate thread.
 * IoManager *this : An allocated IoManager
 * Return : void
 */
void
IoManager_startThread (
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


