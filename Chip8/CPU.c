#include "Cpu.h"
#include <stdlib.h>

// ---------- Debugging -------------
#define __DEBUG_OBJECT__ "Cpu"
#include "dbg/dbg.h"

/*
 * Description 	: Allocate a new Cpu structure.
 * Return		: A pointer to an allocated Cpu.
 */
Cpu *
Cpu_new (void)
{
	Cpu *this;

	if ((this = calloc (1, sizeof(Cpu))) == NULL)
		return NULL;

	if (!Cpu_init (this)) {
		Cpu_free (this);
		return NULL;
	}

	return this;
}

/*
 * Description : Initialize an allocated Cpu structure.
 * Cpu *this : An allocated Cpu to initialize.
 * Return : true on success, false on failure.
 */
bool
Cpu_init (
	Cpu *this
) {
	srand (time(NULL));

	// Reset entirely the CPU state
	memset (this, 0, sizeof(Cpu));

	// Load built-in font set into emulator memory
	uint8_t chip8_fontset [80] = {
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};
	memcpy (&this->memory[FONT_START_ADDRESS], chip8_fontset, sizeof(chip8_fontset));

	// Instruction pointer start at the start of the program
	this->ip = USER_SPACE_START_ADDRESS;

	// Get a profiler
	if (!(this->profiler = ProfilerFactory_getProfiler ("CPU"))) {
		dbg ("Cannot allocate a new Profiler.");
		return false;
	}

	// Default speed
	this->speed = DEFAULT_CPU_SPEED;

	// Ready state
	this->isRunning = true;

	return true;
}

/*
 * Description : Stop the separate thread for the CPU
 * Cpu *this : An allocated Cpu
 * Return : void
 */
void
Cpu_stopThread (
	Cpu *this
) {
	this->isRunning = false;
	sfThread_wait (this->thread);
}

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
) {
	int romSize;
	char *romFile;

	// Read the ROM from a given file
	// Check if the ROM file has been correctly read
	if (!(romFile = file_get_contents_and_size (filename, &romSize))) {
		dbg ("The ROM \"%s\" cannot be loaded.", filename);
		return false;
	}

	// Check if the ROM file is small enough for the Chip8 memory
	if (romSize > USER_PROGRAM_SPACE_SIZE) {
		dbg ("The ROM \"%s\" is too big : %d bytes (max : %d bytes).",
			filename, romSize, USER_PROGRAM_SPACE_SIZE);
		return false;
	}

	// ROM successfully loaded, copy it into the emulator memory
	memcpy (&this->memory[USER_SPACE_START_ADDRESS], romFile, romSize);

	// Clean memory
	free (romFile);

	return true;
}

/*
 * Description : Fetch the next opcode
 * Cpu *this : An allocated Cpu
 * uint16_t ip : The instruction pointer containing the opcodes to fetch
 * Return : uint16_t the opcodes read from memory at IP
 */
inline uint16_t
Cpu_fetchOpcode (
	Cpu *this,
	uint16_t ip
) {
	// 16 bytes instructions : read the next 2 bytes in memory
	return (this->memory[ip] << 8
	     |  this->memory[ip + 1]);
}

/*
 * Description : Prints in the console the current state of the Cpu
 * Cpu *this : An allocated  Cpu
 * Return : void
 */
void
Cpu_debug (
	Cpu *this
) {
	int id = 0;

	for (int h = 0; h < 4; h++) {
		for (int w = 0; w < 4; w++, id++) {
			printf ("V%X : %02X", id, this->V[id]);
			if (w != 3) {
				printf (" | ");
			}
		}
		printf ("\n");
	}

	printf ("IP : %04X | SP : %04X | Index : %04X\n",
		this->ip, this->sp, this->I);

	printf("delayTimer : %03d | soundTimer : %03d\n",
		this->delayTimer, this->soundTimer);

	printf("Current opcode : %04X\n", this->opcode);

	printf ("\n");
}


/*
 * Description : Pop and return the value on the head of the stack
 * Cpu *this : An allocated Cpu
 * Return : uint8_t top byte on the stack
 */
uint16_t
Cpu_stackPop (
	Cpu *this
) {
	if (this->sp <= 0) {
		dbg ("Error : nothing on the stack.");
		exit (0);
	}

	return this->stack[--this->sp];
}


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
) {
	if (this->sp >= STACK_SIZE) {
		dbg ("Error : Stack overflow");
		exit (0);
	}

	this->stack[this->sp++] = value;
}

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
) {
	if (ip > MEMORY_SIZE) {
		dbg ("Error : Out of memory");
		exit (0);
	}

	return Cpu_fetchOpcode (this, ip - INSN_SIZE);
}


/*
 * Description : Print the current state of the stack in the console
 * Cpu *this : An allocated Cpu
 * Return : void
 */
void
Cpu_debugStack (
	Cpu *this
) {
	for (int i = 0; i < STACK_SIZE; i++) {
		printf("[%d] : %x\n", i, this->stack[i]);
	}

	printf("\n");
}

/*
 * Description : Execute the current opcode
 * Cpu *this : An allocated Cpu
 * Return : void
 */
void
Cpu_executeOpcode (
	Cpu *this
) {
	// Macro helpers
	#define ___N (opcode & 0x000F)
	#define __NN (opcode & 0x00FF)
	#define _NNN (opcode & 0x0FFF)
	#define _X__ ((opcode & 0x0F00) >> 8)
	#define __Y_ ((opcode & 0x00F0) >> 4)
	#define VX   V[_X__]
	#define VY   V[__Y_]
	#define VF   V[0x0F]

	// Dereferencing CPU variables
	uint16_t opcode = this->opcode;
	uint8_t *V      = this->V;

	switch (opcode & 0xF000)
	{
		case 0x0000:
			switch (opcode & 0x0F00)
			{
				case 0x0000:
					switch (opcode & 0x00FF)
					{
						case 0x00E0:
						/*   0x00E0 	Clears the screen. */
							Screen_clear (this->screen);
							this->ip += INSN_SIZE;
						break;

						case 0x00EE:
						/*   0x00EE 	Returns from a subroutine. */
							// Pop the return address on the stack
							this->ip = Cpu_stackPop (this) + INSN_SIZE;
						break;

						default : Cpu_unknownOpcode (this); break;
					}
				break;

				default :
				//   0x0NNN 	Calls RCA 1802 program at address NNN.
					dbg ("Unhandled 0x0NNN : Calls RCA 1802 program.");
					exit (0);
				break;
			}
		break;

		case 0x1000:
		/*   0x1NNN 	Jumps to address NNN. */
			this->ip = _NNN;
		break;

		case 0x2000:
		/*   0x2NNN 	Calls subroutine at NNN. */
			// Push the return address on the stack
			Cpu_stackPush (this, this->ip);
			// Jump to address NNN.
			this->ip = _NNN;
		break;

		case 0x3000:
		/*   0x3XNN 	Skips the next instruction if VX equals NN. */
			this->ip += (VX == __NN) ? 4 : 2;
		break;

		case 0x4000:
		/*   0x4XNN 	Skips the next instruction if VX doesn't equal NN. */
			this->ip += (VX != __NN) ? 4 : 2;
		break;

		case 0x5000:
		/*   0x5XY0 	Skips the next instruction if VX equals VY. */
			this->ip += (VX == VY) ? 4 : 2;
		break;

		case 0x6000:
		/*   0x6XNN 	Sets VX to NN. */
			VX = __NN;
			this->ip += INSN_SIZE;
		break;

		case 0x7000:
		/*   0x7XNN 	Adds NN to VX. */
			VX += __NN;
			this->ip += INSN_SIZE;
		break;

		case 0x8000:
			switch (opcode & 0x000F)
			{
				case 0x0000:
				/*   0x8XY0 	Sets VX to the value of VY. */
					VX = VY;
					this->ip += INSN_SIZE;
				break;

				case 0x0001:
				/*   0x8XY1 	Sets VX to VX or VY. */
					VX |= VY;
					this->ip += INSN_SIZE;
				break;

				case 0x0002:
				/*   0x8XY2 	Sets VX to VX and VY. */
					VX &= VY;
					this->ip += INSN_SIZE;
				break;

				case 0x0003:
				/*   0x8XY3 	Sets VX to VX xor VY. */
					VX ^= VY;
					this->ip += INSN_SIZE;
				break;

				case 0x0004:
				/*   0x8XY4 	Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't. */
					VF = (VY > (0xFF - VX)) ? 1 : 0; // carry
					VX += VY;
					this->ip += INSN_SIZE;
				break;

				case 0x0005:
				/*   0x8XY5 	VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't. */
					VF = (VY > VX) ? 0 : 1; // borrow
					VX -= VY;
					this->ip += INSN_SIZE;
				break;

				case 0x0006:
				/*   0x8XY6 	Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift. */
					VF = VX & 0x1;
					VX >>= 1;
					this->ip += INSN_SIZE;
				break;

				case 0x0007:
				/*   0x8XY7 	Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't. */
					VF = (VX > VY) ? 0 : 1; // borrow
					VX = VY - VX;
					this->ip += INSN_SIZE;
				break;

				case 0x000E:
				/*   0x8XYE 	Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift. */
					VF = VX >> 7;
					VX <<= 1;
					this->ip += INSN_SIZE;
				break;

				default :
					Cpu_unknownOpcode (this);
				break;
			}
		break;

		case 0x9000:
		/*   0x9XY0 	Skips the next instruction if VX doesn't equal VY. */
			this->ip += (VX != VY) ? 4 : 2;
		break;

		case 0xA000:
		/*   0xANNN 	Sets I to the address NNN. */
			this->I = _NNN;
			this->ip += INSN_SIZE;
		break;

		case 0xB000:
		/*   0xBNNN 	Jumps to the address NNN plus V0. */
			this->ip = _NNN + V[0];
		break;

		case 0xC000:
		/*  CXNN 	Sets VX to a random number and NN. */
			VX = (rand() % 0xFF) & __NN;
			this->ip += INSN_SIZE;
		break;

		case 0xD000:
		/*   0xDXYN		Sprites stored in memory at location in index register (I), maximum 8bits wide.
						Wraps around the screen.
						If when drawn, clears a pixel, register VF is set to 1 otherwise it is zero.
						All drawing is XOR drawing (e.g. it toggles the screen pixels)
		*/
			// Set VF to 1 if a pixel changed from 1 to 0
			VF = Screen_drawSprite (this->screen, VX, VY, ___N, this->memory, this->I);
			this->ip += INSN_SIZE;
		break;

		case 0xE000:
			switch (opcode & 0x00FF)
			{
				case 0x009E:
				/*   0xEX9E 	Skips the next instruction if the key stored in VX is pressed. */
					this->ip += (Window_requestKeyState (VX) == KEY_PRESSED) ? INSN_SIZE * 2 : 2;
				break;

				case 0x00A1:
				/*   0xEXA1 	Skips the next instruction if the key stored in VX isn't pressed. */
					this->ip += (Window_requestKeyState (VX) == KEY_RELEASED) ? 4 : 2;
				break;

				default :
					Cpu_unknownOpcode (this);
				break;
			}
		break;

		case 0xF000:
			switch (opcode & 0x00FF)
			{
				case 0x0007:
				/*   0xFX07 	Sets VX to the value of the delay timer. */
					VX = this->delayTimer;
					this->ip += INSN_SIZE;
				break;

				case 0x000A: {
				/*   0xFX0A 	A key press is awaited, and then stored in VX. */
					bool keyPressed = false;
					for (int i = 0; i < 16; ++i) {
						if (Window_requestKeyState (i) == KEY_PRESSED) {
							VX = i;
							keyPressed = true;
							// The CPU loop is way faster than the I/O handler one.
							// Thus, the CPU has the right to notify than the key
							// has been handled as pressed and shouldn't be
							// handled twice.
							Window_setKeyState (i, KEY_PUSHED);
						}
					}

					// Only step to the next instruction if a key has been pressed
					if (keyPressed) {
						this->ip += INSN_SIZE;
					}
				}
				break;

				case 0x0015:
				/*   0xFX15 	Sets the delay timer to VX. */
					this->delayTimer = VX;
					this->ip += INSN_SIZE;
				break;

				case 0x0018:
				/*   0xFX18 	Sets the sound timer to VX. */
					this->soundTimer = VX;
					this->ip += INSN_SIZE;
				break;

				case 0x001E:
				/*   0xFX1E 	Adds VX to I. */
					VF = (this->I + VX > 0xFFF)	? 1 : 0; // VF = 1 when overflow
					this->I += VX;
					this->ip += INSN_SIZE;
				break;

				case 0x0029:
				/*   0xFX29 	Sets I to the location of the sprite for the character in VX.
								Characters 0-F (in hexadecimal) are represented by a 4x5 font. */
					this->I = VX * 5;
					this->ip += INSN_SIZE;
				break;

				case 0x0033:
				/*   0xFX33 	Stores the Binary-coded decimal representation of VX, with the most significant of three digits at the address in I,
								the middle digit at I plus 1, and the least significant digit at I plus 2.
								(In other words, take the decimal representation of VX, place the hundreds digit in memory at location in I,
								the tens digit at location I+1, and the ones digit at location I+2.) */
					this->memory[this->I]     =  VX / 100;
					this->memory[this->I + 1] = (VX / 10)  % 10;
					this->memory[this->I + 2] = (VX % 100) % 10;
					this->ip += INSN_SIZE;
				break;

				case 0x0055:
				/*   0xFX55 	Stores V0 to VX in memory starting at address I. */
					for (int pos = 0; pos <= _X__; pos++) {
						this->memory [this->I + pos] = V[pos];
					}
					// On the original interpreter, when the operation is done, I = I + X + 1.
					this->I += _X__ + 1;
					this->ip += INSN_SIZE;
				break;

				case 0x0065:

				/*   0xFX65 	Fills V0 to VX with values from memory starting at address I. */
					for (int pos = 0; pos <= _X__; pos++) {
						V[pos] = this->memory [this->I + pos];
					}

					// On the original interpreter, when the operation is done, I = I + X + 1.
					this->I += _X__ + 1;
					this->ip += INSN_SIZE;
				break;

				default :
					Cpu_unknownOpcode (this);
				break;
			}
		break;

		default :
			Cpu_unknownOpcode (this);
		break;
	}


	// Update CPU timers
	Cpu_updateTimers (this);

	// Clean macro namespace
	#undef ___N
	#undef __NN
	#undef _NNN
	#undef VX
	#undef VY
	#undef VF
}


/*
 * Description : Print an error about an unknown cpu opcode and skip it
 * Cpu *this : An allocated  Cpu
 * Return : void
 */
void
Cpu_unknownOpcode (
	Cpu *this
) {
	dbg ("Error : Unsupported instruction %04X", this->opcode);
	exit (0);
}


/*
 * Description : Update the cpu timers
 * Cpu *this : An allocated Cpu
 * Return : void
 */
void
Cpu_updateTimers (
	Cpu *this
) {
	if (this->delayTimer > 0) {
		this->delayTimer--;
	}

	if (this->soundTimer > 0) {
		this->soundTimer--;

		if (this->soundTimer == 0) {
			Window_requestBeep ();
		}
	}
}

/*
 * Description : Main loop of the CPU.
 * Cpu *this : An allocated Cpu
 * Return : void
 */
void
Cpu_loop (
	Cpu *this
) {
	while (this->isRunning)
	{
		Profiler_tick (this->profiler);

		// Emulate one CPU cycle
		Cpu_emulateCycle (this);

		// Sleep a bit so the CPU doesn't burn
		if (this->profiler->ticksCount % this->speed == 0) {
			sfSleep (sfSeconds(0.001));
		}
	}
}

/*
 * Description : Emulate a CPU cycle. Update internal states accordingly.
 * Cpu *this : An allocated Cpu
 * Return : void
 */
inline void
Cpu_emulateCycle (
	Cpu *this
) {
	this->opcode = Cpu_fetchOpcode (this, this->ip);
	Cpu_executeOpcode (this);
}


/*
 * Description : Start the main loop of the CPU in a separate thread.
 * Cpu *this : An allocated Cpu
 * Return : sfThread * Thread object pointer
 */
sfThread *
Cpu_startThread (
	Cpu *this
) {
	this->thread = sfThread_create ((void (*)(void *)) Cpu_loop, this);
	sfThread_launch (this->thread);

	return this->thread;
}


/*
 * Description : Free an allocated Cpu structure.
 * Cpu *this : An allocated Cpu to free.
 */
void
Cpu_free (
	Cpu *this
) {
	if (this != NULL)
	{
		Screen_free (this->screen);
		Profiler_free (this->profiler);
		sfThread_destroy (this->thread);
		free (this);
	}
}
