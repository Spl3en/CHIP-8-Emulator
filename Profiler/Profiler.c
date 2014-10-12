#include "Profiler.h"
#include <stdlib.h>

// ---------- Debugging -------------
#define __DEBUG_OBJECT__ "Profiler"
#include "dbg/dbg.h"

/*
 * Description 	 : Allocate a new Profiler structure.
 * ProfilerId id : ID of the profiler. Should be unique.
 * Return		 : A pointer to an allocated Profiler.
 */
Profiler *
Profiler_new (
	ProfilerId id,
	char *name
) {
	Profiler *this;

	if ((this = calloc (1, sizeof(Profiler))) == NULL)
		return NULL;

	if (!Profiler_init (this, id, name)) {
		Profiler_free (this);
		return NULL;
	}

	return this;
}


/*
 * Description : Initialize an allocated Profiler structure.
 * Profiler *this : An allocated Profiler to initialize.
 * ProfilerId id : ID of the profiler. Should be unique.
 * char * name : Name of the profiler (optional, can be NULL)
 * Return : true on success, false on failure.
 */
bool
Profiler_init (
	Profiler *this,
	ProfilerId id,
	char *name
) {
	this->id = id;
	this->ticksCount = 0;
	this->clock = NULL;
	this->name = name;

	this->text = sfText_create ();
	sfText_setFont (this->text, sfFont_createFromFile("verdana.ttf"));
	sfText_setCharacterSize (this->text, 15);
	sfText_setColor (this->text, sfRed);
	sfText_setPosition (this->text, (sfVector2f) {.x = 0, .y = 0 + (id * 15)});

	return true;
}


/*
 * Description : Tick the profiler
 * Profiler *this : An allocated Profiler
 * Return : void
 */
inline void
Profiler_tick (
	Profiler *this
) {
	if (!this->clock) {
		Profiler_start(this);
	}
	this->ticksCount++;
}


/*
 * Description : Update a profiler text
 * Profiler *this : An allocated Profiler
 * Return : void
 */
void
Profiler_update (
	Profiler *this
) {
	unsigned char buffer[1024];
	sprintf (buffer, "%s = %u TPS\n", this->name, this->ticksCount);
	sfText_setString (this->text, buffer);
}


/*
 * Description : Get seconds elasped since the last restart
 * Profiler *this : An allocated Profiler
 * Return : float, seconds
 */
float
Profiler_getTime (
	Profiler *this
) {
	return sfTime_asSeconds (sfClock_getElapsedTime (this->clock));
}

/*
 * Description :
 * Profiler *this : An allocated Profiler
 * Return : void
 */
void
Profiler_restart (
	Profiler *this
) {
	this->ticksCount = 0;
	sfClock_restart (this->clock);
}

/*
 * Description : Start profiling
 * Profiler *this : An allocated Profiler to initialize.
 * Return : void
 */
void
Profiler_start (
	Profiler * this
) {
	this->clock = sfClock_create ();
	this->ticksCount = 0;
}


/*
 * Description : Free an allocated Profiler structure.
 * Profiler *this : An allocated Profiler to free.
 */
void
Profiler_free (
	Profiler *this
) {
	if (this != NULL) {
		free (this);
	}
}


/*
 * Description : Unit tests checking if a Profiler is coherent
 * Profiler *this : The instance to test
 * Return : true on success, false on failure
 */
bool
Profiler_test (
	Profiler *this
) {

	return true;
}
