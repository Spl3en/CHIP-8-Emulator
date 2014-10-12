#include "ProfilerFactory.h"
#include <stdlib.h>

// ---------- Debugging -------------
#define __DEBUG_OBJECT__ "ProfilerFactory"
#include "dbg/dbg.h"

// Private structure declaration
struct _ProfilerFactory
{
	BbQueue  *profilersQueue;	// The queue receive all the profilers
	Profiler **profilersArray;	// The array is created from the queue once all the profilers have been received
	int profilersArraySize;

	// Singleton
} * this = NULL;

/*
 * Description 	: Allocate and initialize a new ProfilerFactory structure.
 * Return		: bool, true on success, false otherwise
 */
bool
ProfilerFactory_init (void)
{
	if ((this = calloc (1, sizeof(ProfilerFactory))) == NULL)
		return false;

	this->profilersQueue = bb_queue_new ();
	this->profilersArray = NULL;

	return true;
}


/*
 * Description : Add a profiler to the ProfilerFactory
 * Return : Profiler * a new Profiler
 */
Profiler *
ProfilerFactory_getProfiler (char *name)
{
	if (!this) {
		ProfilerFactory_init ();
	}

	// Get a new Profiler ID
	ProfilerId id = bb_queue_get_length (this->profilersQueue);

	// Instantiate a new profiler
	Profiler *profiler = Profiler_new (id, name);

	// Add it to the queue of profilers
	bb_queue_add (this->profilersQueue, profiler);

	return profiler;
}


/*
 * Description : Get all the profilers as an array
 * int *size : (out) size of the profiler array
 * Return : Profiler **profilersArray, array of Profiler *
 */
Profiler **
ProfilerFactory_getArray (int *size)
{
	if (this->profilersArray == NULL)
	{
		// Queue to array so we have a O(1) access to profilers
		this->profilersArraySize = bb_queue_get_length (this->profilersQueue);
		this->profilersArray = malloc (sizeof(Profiler *) * this->profilersArraySize);

		while (bb_queue_get_length (this->profilersQueue))
		{
			Profiler *profiler = bb_queue_pop (this->profilersQueue);
			ProfilerId id = profiler->id;
			this->profilersArray[id] = profiler;
		}

		// Clean queue memory
		bb_queue_free (this->profilersQueue);
		this->profilersQueue = NULL;
	}

	*size = this->profilersArraySize;
	return this->profilersArray;
}




/*
 * Description : Free an allocated ProfilerFactory structure.
 * ProfilerFactory *this : An allocated ProfilerFactory to free.
 */
void
ProfilerFactory_free (
	ProfilerFactory *this
) {
	if (this != NULL)
	{
		free (this);
	}
}


/*
 * Description : Unit tests checking if a ProfilerFactory is coherent
 * ProfilerFactory *this : The instance to test
 * Return : true on success, false on failure
 */
bool
ProfilerFactory_test (
	ProfilerFactory *this
) {

	return true;
}
