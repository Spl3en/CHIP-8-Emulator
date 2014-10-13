// --- Author : Moreau Cyril - Spl3en
#pragma once

// ---------- Includes ------------
#include "Profiler.h"
#include "Utils/Utils.h"
#include "BbQueue/BbQueue.h"

// ---------- Defines -------------


// ------ Structure declaration -------
typedef struct _ProfilerFactory ProfilerFactory;


// --------- Allocators ---------

// ----------- Functions ------------

/*
 * Description : Initialize an allocated ProfilerFactory structure.
 * ProfilerFactory *this : An allocated ProfilerFactory to initialize.
 */
bool
ProfilerFactory_init (void);

/*
 * Description : Unit tests checking if a ProfilerFactory is coherent
 * ProfilerFactory *this : The instance to test
 * Return : true on success, false on failure
 */
bool
ProfilerFactory_test (
    ProfilerFactory *this
);

/*
 * Description : Add a profiler to the ProfilerFactory
 * char * name : Name of the profiler (optional, can be NULL)
 * Return : Profiler * a new Profiler
 */
Profiler *
ProfilerFactory_getProfiler (
    char *name
);

/*
 * Description : Get all the profilers as an array. Once it is called, free the queue
 * int *size : (out) size of the profiler array
 * Return : Profiler **profilersArray, array of Profiler *
 */
Profiler **
ProfilerFactory_getArray (
    int *size
);

// --------- Destructors ----------

/*
 * Description : Free an allocated ProfilerFactory structure.
 * ProfilerFactory *this : An allocated ProfilerFactory to free.
 */
void
ProfilerFactory_free (
    ProfilerFactory *this
);


