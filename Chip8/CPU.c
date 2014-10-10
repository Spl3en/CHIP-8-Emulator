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

	if ((this = malloc (sizeof(Cpu))) == NULL)
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
	memcpy (&this->memory[FONT_START_ADDRESS], chip8_fontset, sizeof(chip8_fontset));

	// Instruction pointer start at the start of the program
	this->ip = USER_SPACE_START_ADDRESS;

	// Load screen component
	this->screen = Graphic_new (this->memory, &this->index);

	// The program is ready to run from this point
	this->isRunning = true;

	return true;
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
	char *romFile = file_get_contents_and_size (filename, &romSize);

	// Check if the ROM file has been correctly read
	if (!romFile) {
		dbg ("The ROM \"%s\" cannot be loaded.\n", filename);
		return false;
	}

	// Check if the ROM file is small enough for the Chip8 memory
	if (romSize > USER_PROGRAM_SPACE_SIZE) {
		dbg ("The ROM \"%s\" is too big : %d bytes (max : %d bytes).\n",
			filename, romSize, USER_PROGRAM_SPACE_SIZE);
		return false;
	}

	// ROM successfully loaded, copy it in emulator memory
	memcpy (&this->memory[USER_SPACE_START_ADDRESS], romFile, romSize);

	// Clean memory
	free (romFile);

	return true;
}


/*
 * Description : Fetch the next opcode
 * Cpu *this : An allocated Cpu
 * Return : void
 */
void
Cpu_fetchOpcode (
	Cpu *this
) {
	// 16 bytes instructions : read the next 2 bytes in memory
	this->opcode = (this->memory[this->ip] << 8
						|  this->memory[this->ip + 1]);
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
			printf ("V%X : %04X", id, this->V[id]);
			if (w != 3) {
				printf (" | ");
			}
		}
		printf ("\n");
	}

	printf ("IP : %04X | SP : %04X | Index : %04X\n",
		this->ip, this->sp, this->index);

	printf("delayTimer : %03d | soundTimer : %03d\n",
		this->delayTimer, this->soundTimer);

	printf("Current opcode : %04X\n", this->opcode);

	printf ("\n");
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

	// Cpu_debug (this);
	// Graphic_debug (this->screen);
	// system("cls");

	switch (opcode & 0xF000)
	{
		//   0x0XXX
		case 0x0000:
			switch (opcode & 0x0F00)
			{
				//   0x00XX
				case 0x0000:
					switch (opcode & 0x00FF)
					{
						/*   0x00E0 	Clears the screen. */
						case 0x00E0:
							Graphic_clearScreen (this->screen);
						break;

						/*   0x00EE 	Returns from a subroutine. */
						case 0x00EE:
							// Pop the return address in ip
							this->ip = this->stack[this->sp--];
						break;


						default : Cpu_unknownOpcode (this); break;
					}
				break;

				//   0x0NNN 	Calls RCA 1802 program at address NNN.
				default :
					Cpu_unknownOpcode(this);
				break;
			}
		break;

		/*   0x1NNN 	Jumps to address NNN. */
		case 0x1000:
			this->ip = _NNN;
		break;

		/*   0x2NNN 	Calls subroutine at NNN. */
		case 0x2000:
			// Push the return address on the stack
			this->stack[this->sp++] = this->ip + 2;
			// Jump to address NNN.
			this->ip = _NNN;
		break;

		/*   0x3XNN 	Skips the next instruction if VX equals NN. */
		case 0x3000:
			this->ip += (VX == __NN) ? 4 : 2;
		break;

		/*   0x4XNN 	Skips the next instruction if VX doesn't equal NN. */
		case 0x4000:
			this->ip += (VX != __NN) ? 4 : 2;
		break;

		/*   0x5XY0 	Skips the next instruction if VX equals VY. */
		case 0x5000:
			this->ip += (VX == VY) ? 4 : 2;
		break;

		/*   0x6XNN 	Sets VX to NN. */
		case 0x6000:
			VX = __NN;
			this->ip += 2;
		break;

		/*   0x7XNN 	Adds NN to VX. */
		case 0x7000:
			VX += __NN;
			this->ip += 2;
		break;

		//   0x8XXX
		case 0x8000:
			switch (opcode & 0x000F)
			{
				/*   0x8XY0 	Sets VX to the value of VY. */
				case 0x0000:
					VX  = VY;
					this->ip += 2;
				break;

				/*   0x8XY1 	Sets VX to VX or VY. */
				case 0x0001:
					VX |= VY;
					this->ip += 2;
				break;

				/*   0x8XY2 	Sets VX to VX and VY. */
				case 0x0002:
					VX &= VY;
					this->ip += 2;
				break;

				/*   0x8XY3 	Sets VX to VX xor VY. */
				case 0x0003:
					VX ^= VY;
					this->ip += 2;
				break;

				/*   0x8XY4 	Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't. */
				case 0x0004:
					VF = (VY > (0xFF - VX)) ? 1 : 0; // carry
					VX += VY;
					this->ip += 2;
				break;

				/*   0x8XY5 	VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't. */
				case 0x0005:
					VF = (VY > VX) ? 0 : 1; // borrow
					VX -= VY;
					this->ip += 2;
				break;

				/*   0x8XY6 	Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift. */
				case 0x0006:
					VF = VX & 0x1;
					VX >>= 1;
				break;

				/*   0x8XY7 	Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't. */
				case 0x0007:
					VF = (VX > VY) ? 0 : 1; // borrow
					VX = VY - VX;
					this->ip += 2;
				break;

				/*   0x8XYE 	Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift. */
				case 0x000E:
					VF = VX >> 7;
					VX <<= 1;
					this->ip += 2;
				break;

				default : Cpu_unknownOpcode (this); break;
			}
		break;

		/*   0x9XY0 	Skips the next instruction if VX doesn't equal VY. */
		case 0x9000:
			this->ip += (VX != VY) ? 4 : 2;
		break;

		/*   0xANNN 	Sets I to the address NNN. */
		case 0xA000:
			this->index = _NNN;
			this->ip += 2;
		break;

		/*   0xBNNN 	Jumps to the address NNN plus V0. */
		case 0xB000:
			this->ip = _NNN + V[0];
		break;

		/*  CXNN 	Sets VX to a random number and NN. */
		case 0xC000:
			VX = (rand() % 0xFF) & __NN;
			this->ip += 2;
		break;

		/*   0xDXYN		Sprites stored in memory at location in index register (I), maximum 8bits wide.
						Wraps around the screen.
						If when drawn, clears a pixel, register VF is set to 1 otherwise it is zero.
						All drawing is XOR drawing (e.g. it toggles the screen pixels)
		*/
		case 0xD000: {
			// Set VF to 1 if a pixel changed from 1 to 0
			VF = Graphic_drawSprite (this->screen, VX, VY, ___N);
			this->ip += 2;
		}
		break;

		//   0xEXXX
		case 0xE000:
			switch (opcode & 0x00FF)
			{
				/*   0xEX9E 	Skips the next instruction if the key stored in VX is pressed. */
				case 0x009E:
					this->ip += (this->keysState[VX] != 0) ? 4 : 2;
				break;

				/*   0xEXA1 	Skips the next instruction if the key stored in VX isn't pressed. */
				case 0x00A1:
					this->ip += (this->keysState[VX] == 0) ? 4 : 2;
				break;
			}
		break;

		//   0xFXXX
		case 0xF000:
			switch (opcode & 0x00F0)
			{
				//   0xFX0X
				case 0x0000:
					switch (opcode & 0x000F)
					{
						/*   0xFX07 	Sets VX to the value of the delay timer. */
						case 0x0007:
							VX = this->delayTimer;
							this->ip += 2;
						break;

						/*   0xFX0A 	A key press is awaited, and then stored in VX. */
						case 0x000A: {
							bool keyPressed = false;
							for (int i = 0; i < 16; ++i) {
								if (this->keysState[i] != 0) {
									VX = i;
									keyPressed = true;
								}
							}

							// If we didn't received a keypress, skip this cycle and try again.
							if (keyPressed) {
								this->ip += 2;
							}
						}
						break;

						default : Cpu_unknownOpcode (this); break;
					}
				break;

				//   0xFX1X
				case 0x0010:
					switch (opcode & 0x000F)
					{
						/*   0xFX15 	Sets the delay timer to VX. */
						case 0x0005:
							this->delayTimer = VX;
							this->ip += 2;
						break;

						/*   0xFX18 	Sets the sound timer to VX. */
						case 0x0008:
							this->soundTimer = VX;
							this->ip += 2;
						break;

						/*   0xFX1E 	Adds VX to I. */
						case 0x000E:
							this->index += VX;
							this->ip += 2;
						break;

						default : Cpu_unknownOpcode (this); break;
					}
				break;

				/*   0xFX29 	Sets I to the location of the sprite for the character in VX.
								Characters 0-F (in hexadecimal) are represented by a 4x5 font. */
				case 0x0029:
					this->index = VX * 5;
					this->ip += 2;
				break;

				/*   0xFX33 	Stores the Binary-coded decimal representation of VX, with the most significant of three digits at the address in I,
								the middle digit at I plus 1, and the least significant digit at I plus 2.
								(In other words, take the decimal representation of VX, place the hundreds digit in memory at location in I,
								the tens digit at location I+1, and the ones digit at location I+2.) */
				case 0x0030:
					this->memory[this->index]     =  VX / 100;
					this->memory[this->index + 1] = (VX / 10)  % 10;
					this->memory[this->index + 2] = (VX % 100) % 10;
					this->ip += 2;
				break;

				/*   0xFX55 	Stores V0 to VX in memory starting at address I. */
				case 0x0050:
					for (int pos = 0; pos <= VX; pos++) {
						this->memory[this->index + pos] = V[pos];
					}

					// On the original interpreter, when the operation is done, I = I + X + 1.
					this->index += VX + 1;
					this->ip += 2;
				break;

				/*   0xFX65 	Fills V0 to VX with values from memory starting at address I. */
				case 0x0060:
					for (int pos = 0; pos <= VX; pos++) {
						V[pos] = this->memory[this->index + pos];
					}

					// On the original interpreter, when the operation is done, I = I + X + 1.
					this->index += VX + 1;
					this->ip += 2;
				break;

				default : Cpu_unknownOpcode (this); break;
			}
		break;

		default : Cpu_unknownOpcode (this); break;
	}


	// Update CPU timers
	Cpu_updateTimers (this);

	// Update the number of cycle executed
	this->cyclesCount++;

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
	dbg ("Unsupported instruction : %04X", this->opcode);
	this->ip += 2;
	exit(0);
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
			dbg ("Beep!\n");
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
	sfClock *clock = sfClock_create();

	Graphic_startThread (this->screen);

	while (this->isRunning)
	{
		// Emulate one CPU cycle
		Cpu_emulateCycle (this);
		sfTime timeCPU = sfClock_getElapsedTime(clock);
		sfClock_restart(clock);

		// Poll SFML window events
		sfEvent event;
        while (sfRenderWindow_pollEvent (this->screen->window, &event))
        {
            if (event.type == sfEvtClosed) {
                sfRenderWindow_close (this->screen->window);
            }
        }

		// Store key press state (Press and Release)
		Cpu_setKeys (this);

		// Sleep a bit so the CPU doesn't burn
		sfSleep(sfMilliseconds(1));
	}
}


/*
 * Description : Emulate a CPU cycle. Update internal states accordingly.
 * Cpu *this : An allocated Cpu
 * Return : void
 */
void
Cpu_emulateCycle (
	Cpu *this
) {
	Cpu_fetchOpcode (this);
	Cpu_executeOpcode (this);
}

/*
 * Description : Update all keys state
 * Cpu *this : An allocated Cpu
 * Return : void
 */
void
Cpu_setKeys (
	Cpu *this
) {

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
		free (this);
	}
}


/*
 * Description : Unit tests checking if a Cpu is coherent
 * Cpu *this : The instance to test
 * Return : true on success, false on failure
 */
bool
Cpu_test (
	Cpu *this
) {

	return true;
}
