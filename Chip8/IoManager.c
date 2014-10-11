#include "IoManager.h"
#include <stdlib.h>

// ---------- Debugging -------------
#define __DEBUG_OBJECT__ "IoManager"
#include "dbg/dbg.h"

/*
 * Description 	: Allocate a new IoManager structure.
 * Return		: A pointer to an allocated IoManager.
 */
IoManager *
IoManager_new (void)
{
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
 * Return : true on success, false on failure.
 */
bool
IoManager_init (
	IoManager *this
) {
	memset(this->keysState, 0, sizeof (this->keysState));

	return true;
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
