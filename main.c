#include "Chip8/CPU.h"

int main(int argc, char **argv)
{
	// sfVideoMode desktop = sfVideoMode_getDesktopMode ();
	// AppWindow *window = AppWindow_new ("VisualBinary", desktop.width * 0.667, desktop.height * 0.667, false);

	Cpu *cpu = NULL;

	if ((cpu = Cpu_new ()) == NULL) {
		printf ("Error : Cannot initialize CPU.\n");
		return -1;
	}

	if (!Cpu_loadRom (cpu, "./games/INVADERS")) {
		printf ("Error : Can't load ROM.\n");
		return -1;
	}

	Cpu_loop (cpu);

	return 0;
}
