// --- Author : Moreau Cyril - Spl3en
#pragma once

// ---------- Includes ------------
#include "FontSet.h"
#include "Screen.h"
#include "IoManager.h"
#include "Utils/Utils.h"
#include "Ztring/Ztring.h"
#include <stdint.h>

// ---------- Defines -------------
#define MEMORY_SIZE 0x1000
#define REGISTERS_COUNT 16
#define STACK_SIZE 16

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

	// Memory buffer
	/*	0x000-0x1FF - CHIP-8 interpreter
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
    uint8_t stack [STACK_SIZE];

    // Stack pointer register
    uint8_t sp;

	// Screen display
    Screen *screen;

    // I/O manager
    IoManager *io;

	// Timers : when set above zero they will count down to zero.
	uint8_t delayTimer;
	uint8_t soundTimer; // The system’s buzzer sounds whenever the sound timer reaches zero.

	// Running state
	bool isRunning;

	// Number of cycles since the beginning
	uint64_t cyclesCount;

}	Cpu;



// --------- Allocators ---------

/*
 * Description 	: Allocate a new Cpu structure.
 * Return		: A pointer to an allocated Cpu.
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
 * Description : Unit tests checking if a Cpu is coherent
 * Cpu *this : The instance to test
 * Return : true on success, false on failure
 */
bool
Cpu_test (
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
 * Description : Update all keys state
 * Cpu *this : An allocated Cpu
 * Return : void
 */
void
Cpu_setKeys (
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
 * Return : void
 */
void
Cpu_fetchOpcode (
	Cpu *this
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
 * uint8_t value : the value to push
 * Return : void
 */
void
Cpu_stackPush (
	Cpu *this,
	uint8_t value
);

/*
 * Description : Pop and return the value on the head of the stack
 * Cpu *this : An allocated Cpu
 * Return : uint8_t top byte on the stack
 */
uint8_t
Cpu_stackPop (
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


// --------- Destructors ----------

/*
 * Description : Free an allocated Cpu structure.
 * Cpu *this : An allocated Cpu to free.
 */
void
Cpu_free (
	Cpu *this
);


