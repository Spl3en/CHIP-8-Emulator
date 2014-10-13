// --- Author : Moreau Cyril - Spl3en
#pragma once

// ---------- Includes ------------
#include "Utils/Utils.h"
#include <SFML/System.h>
#include <SFML/Graphics.h>

// ---------- Defines -------------


// ------ Structure declaration -------
typedef int ProfilerId;

typedef struct _Profiler
{
    unsigned int ticksCount;
    sfClock *clock;

    sfText *text;
    const char *name;

    ProfilerId id;

}    Profiler;



// --------- Allocators ---------

/*
 * Description      : Allocate a new Profiler structure.
 * ProfilerId id : ID of the profiler. Should be unique.
 * char * name   : Name of the profiler (optional, can be NULL)
 * Return         : A pointer to an allocated Profiler.
 */
Profiler *
Profiler_new (
    ProfilerId id,
    char *name
);

// ----------- Functions ------------

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
);

/*
 * Description : Unit tests checking if a Profiler is coherent
 * Profiler *this : The instance to test
 * Return : true on success, false on failure
 */
bool
Profiler_test (
    Profiler *this
);

/*
 * Description : Tick the profiler
 * Profiler *this : An allocated Profiler
 * Return : void
 */
void
Profiler_tick (
    Profiler *this
);

/*
 * Description : Get seconds elasped since the last restart
 * Profiler *this : An allocated Profiler
 * Return : float, seconds
 */
float
Profiler_getTime (
    Profiler *this
);

/*
 * Description :
 * Profiler *this : An allocated Profiler
 * Return : void
 */
void
Profiler_restart (
    Profiler *this
);

/*
 * Description : Start profiling
 * Profiler *this : An allocated Profiler to initialize.
 * Return : void
 */
void
Profiler_start (
    Profiler * this
);

/*
 * Description : Update a profiler text
 * Profiler *this : An allocated Profiler
 * Return : void
 */
void
Profiler_update (
    Profiler *this
);


// --------- Destructors ----------

/*
 * Description : Free an allocated Profiler structure.
 * Profiler *this : An allocated Profiler to free.
 */
void
Profiler_free (
    Profiler *this
);


