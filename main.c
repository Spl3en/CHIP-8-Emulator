#include "Chip8/CPU.h"

int main (int argc, char **argv)
{
	if (argc < 2) {
		printf ("Usage : %s <game>\n", get_filename (argv[0]));
		return 0;
	}

	Cpu *cpu;

	// Instantiate a new CHIP-8 CPU emulator
	if ((cpu = Cpu_new ()) == NULL) {
		printf ("Error : Cannot initialize CPU.\n");
		return -1;
	}

	// Load a ROM into it
	if (!Cpu_loadRom (cpu, "./games/BLINKY")) {
		printf ("Error : Can't load ROM.");
		return -1;
	}

	// Start separate threads (CPU & I/O manager)
	IoManager_startThread (cpu->io);
	Cpu_startThread (cpu);

	// Start the rendering loop
	Screen_loop (cpu->screen);

	// Request threads to exit gracefully
	IoManager_stopThread (cpu->io);
	Cpu_stopThread (cpu);

	// Clean memory gracefully
	Cpu_free (cpu);

	return 0;
}
