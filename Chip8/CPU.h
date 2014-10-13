// --- Author : Moreau Cyril - Spl3en
#pragma once

// ---------- Includes ------------
#include "Window.h"
#include "Screen.h"
#include "Profiler/ProfilerFactory.h"
#include "Utils/Utils.h"
#include "Ztring/Ztring.h"
#include <stdint.h>

// ---------- Defines -------------
#define MEMORY_SIZE 0x1000
#define REGISTERS_COUNT 16
#define STACK_SIZE 16
#define INSN_SIZE sizeof(((Cpu *)0)->opcode)
#define DEFAULT_CPU_SPEED 5

// Memory layout
#define USER_SPACE_START_ADDRESS 0x200
#define DISPLAY_REFRESH_START_ADDRESS 0xF00
#define USER_PROGRAM_SPACE_SIZE (MEMORY_SIZE - USER_SPACE_START_ADDRESS)
#define FONT_START_ADDRESS 0x000


// ------ Structure declaration -------
typedef struct _Cpu
{
    // All opcodes are coded on 16 bits
    uint16_t opcode;

    // Registers state
    uint8_t V [REGISTERS_COUNT];

    // Virtual memory buffer
    /*    0x000-0x1FF - CHIP-8 interpreter
            0x050-0x0A0 - 4x5 pixel font set (0-F)
        0x200-0xFFF - Program ROM and work RAM
            0xEA0-0XEFF - Call stack, internal use, and other variables.
            0xF00-0xFFF - Display refresh
    */
    uint8_t memory [MEMORY_SIZE];

    // Index register
    uint16_t I;

    // Instruction pointer register
    uint16_t ip;

    // Stack memory buffer
    uint16_t stack [STACK_SIZE];

    // Stack pointer register
    uint16_t sp;

    // Screen display
    Screen *screen;

    // Timers : when set above zero they will count down to zero.
    uint8_t delayTimer;
    uint8_t soundTimer; // The system’s buzzer sounds whenever the sound timer reaches zero.

    // Profiler for the CPU
    Profiler * profiler;

    // CPU virtual speed
    int speed;

    // Running state
    bool isRunning;

    // Thread object pointer
    sfThread *thread;

}    Cpu;



// --------- Allocators ---------

/*
 * Description     : Allocate a new Cpu structure.
 * Return        : A pointer to an allocated Cpu.
 */
Cpu *
Cpu_new (void);

// ----------- Functions ------------

/*
 * Description : Initialize an allocated Cpu structure.
 * Cpu *this : An allocated Cpu to initialize.
 */
bool
Cpu_init (
    Cpu *this
);

/*
 * Description : Main loop of the CPU.
 * Cpu *this : An allocated Cpu
 * Return : void
 */
void
Cpu_loop (
    Cpu *this
);

/*
 * Description : Emulate a CPU cycle
 * Cpu *this : An allocated Cpu
 * Return : void
 */
void
Cpu_emulateCycle (
    Cpu *this
);

/*
 * Description : Execute the current opcode
 * Cpu *this : An allocated Cpu
 * Return : void
 */
void
Cpu_executeOpcode (
    Cpu *this
);

/*
 * Description : Load a ROM into the Chip8 memory
 * Cpu *this : An allocated Cpu
 * char *filename : File name of the ROM to load
 * Return : bool, true on success, false otherwise
 */
bool
Cpu_loadRom (
    Cpu *this,
    char *filename
);

/*
 * Description : Fetch the next opcode
 * Cpu *this : An allocated Cpu
 * uint16_t ip : The instruction pointer containing the opcodes to fetch
 * Return : uint16_t the opcodes read from memory at IP
 */
uint16_t
Cpu_fetchOpcode (
    Cpu *this,
    uint16_t ip
);

/*
 * Description : Update the cpu timers
 * Cpu *this : An allocated Cpu
 * Return : void
 */
void
Cpu_updateTimers (
    Cpu *this
);

/*
 * Description : Print an error about an unknown cpu opcode and skip it
 * Cpu *this : An allocated  Cpu
 * Return : void
 */
void
Cpu_unknownOpcode (
    Cpu *this
);

/*
 * Description : Push an element on the stack
 * Cpu *this : An allocated Cpu
 * uint16_t value : the value to push
 * Return : void
 */
void
Cpu_stackPush (
    Cpu *this,
    uint16_t value
);

/*
 * Description : Pop and return the value on the head of the stack
 * Cpu *this : An allocated Cpu
 * Return : uint8_t top byte on the stack
 */
uint16_t
Cpu_stackPop (
    Cpu *this
);

/*
 * Description : Get the previous instruction before IP
 * Cpu *this : An allocated Cpu
 * uint16_t ip : The instruction before which one we want to get the previous one
 * Return : uint16_t the previous ins
truction
 */
uint16_t
Cpu_getPreviousOpCode (
    Cpu *this,
    uint16_t ip
);

/*
 * Description : Print the current state of the stack in the console
 * Cpu *this : An allocated Cpu
 * Return : void
 */
void
Cpu_debugStack (
    Cpu *this
);

/*
 * Description : Prints in the console the current state of the Cpu
 * Cpu *this : An allocated  Cpu
 * Return : void
 */
void
Cpu_debug (
    Cpu *this
);

/*
 * Description : Start the main loop of the CPU in a separate thread.
 * Cpu *this : An allocated Cpu
 * Return : sfThread * Thread object pointer
 */
sfThread *
Cpu_startThread (
    Cpu *this
);

/*
 * Description : Stop the separate thread for the CPU
 * Cpu *this : An allocated Cpu
 * Return : void
 */
void
Cpu_stopThread (
    Cpu *this
);

// --------- Destructors ----------

/*
 * Description : Free an allocated Cpu structure.
 * Cpu *this : An allocated Cpu to free.
 */
void
Cpu_free (
    Cpu *this
);


