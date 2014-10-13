#include "Chip8/CPU.h"

int main (int argc, char **argv)
{
	Cpu *cpu = NULL;

	// Instantiate a new CHIP-8 CPU emulator
	if ((cpu = Cpu_new ()) == NULL) {
		printf ("Error : Cannot initialize CPU.\n");
		return -1;
	}

	// Load a ROM into it
	if (!Cpu_loadRom (cpu, "./games/PONG")) {
		printf ("Error : Can't load ROM.\n");
		return -1;
	}

	// Start i/o manager thread
	sfThread * ioThread = IoManager_startThread (cpu->io);

	// Start rendering thread
	sfThread * screenThread = Screen_startThread (cpu->screen);

	// Start CPU loop
	Cpu_loop (cpu);

	// Wait until separate threads exit gracefully
	sfThread_wait (ioThread);
	sfThread_wait (screenThread);

	return 0;
}
