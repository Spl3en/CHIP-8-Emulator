#include "Chip8/CPU.h"

int main (int argc, char **argv)
{
    Window *window;
    Cpu *cpu;

    argc = 2;
    argv[1] = "./games/HIDDEN";

    if (argc < 2) {
        printf ("Usage : %s <game>\n", get_filename (argv[0]));
        return 0;
    }

    // Open a new SFML Window
    if ((window = Window_new ()) == NULL) {
        printf ("Error : Cannot open a SFML window.\n");
        return -1;
    }

    // Instantiate a new CHIP-8 CPU emulator
    if ((cpu = Cpu_new ()) == NULL) {
        printf ("Error : Cannot initialize CPU.\n");
        return -1;
    }

    // Load a ROM into it
    if (!Cpu_loadRom (cpu, argv[1])) {
        printf ("Error : Can't load ROM.");
        return -1;
    }

    // Load screen component
    cpu->screen = Screen_new (window->sfmlWindow);

    // Start separate threads (CPU & Rendering)
    Cpu_startThread (cpu);
    Screen_startThread (cpu->screen);

    // Start the event listener window
    Window_loop (window);

    // Request threads to exit gracefully
    Screen_stopThread (cpu->screen);
    Cpu_stopThread (cpu);

    // Clean memory gracefully
    Cpu_free (cpu);

    return 0;
}
