#include "Cpu.h"
#include <stdlib.h>

// ---------- Debugging -------------
#define __DEBUG_OBJECT__ "Cpu"
#include "dbg/dbg.h"

/*
 * Description     : Allocate a new Cpu structure.
 * Return        : A pointer to an allocated Cpu.
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

    // Cpu_disass (this);
    // Cpu_debug (this);

    // Set IP to the next opcode
    this->ip += INSN_SIZE;

    switch (opcode & 0xF000)
    {
        case 0x0000:
            switch (opcode & 0x0F00)
            {
                case 0x0000:
                    switch (opcode & 0x00FF)
                    {
                        case 0x00E0:
                        /*   0x00E0     Clears the screen. */
                            Screen_clear (this->screen);
                        break;

                        case 0x00EE:
                        /*   0x00EE     Returns from a subroutine. */
                            // Pop the return address on the stack
                            this->ip = Cpu_stackPop (this);
                        break;

                        default : Cpu_unknownOpcode (this); break;
                    }
                break;

                default :
                //   0x0NNN     Calls RCA 1802 program at address NNN.
                    dbg ("Unhandled 0x0NNN : Calls RCA 1802 program.");
                    exit (0);
                break;
            }
        break;

        case 0x1000:
        /*   0x1NNN     Jumps to address NNN. */
            this->ip = _NNN;
        break;

        case 0x2000:
        /*   0x2NNN     Calls subroutine at NNN. */
            // Push the return address on the stack
            Cpu_stackPush (this, this->ip);
            // Jump to address NNN.
            this->ip = _NNN;
        break;

        case 0x3000:
        /*   0x3XNN     Skips the next instruction if VX equals NN. */
            if (VX == __NN) {
            	this->ip += INSN_SIZE;
            }
        break;

        case 0x4000:
        /*   0x4XNN     Skips the next instruction if VX doesn't equal NN. */
            if (VX != __NN) {
            	this->ip += INSN_SIZE;
            }
        break;

        case 0x5000:
        /*   0x5XY0     Skips the next instruction if VX equals VY. */
            if (VX == VY) {
            	this->ip += INSN_SIZE;
            }
        break;

        case 0x6000:
        /*   0x6XNN     Sets VX to NN. */
            VX = __NN;
        break;

        case 0x7000:
        /*   0x7XNN     Adds NN to VX. */
            VX += __NN;
        break;

        case 0x8000:
            switch (opcode & 0x000F)
            {
                case 0x0000:
                /*   0x8XY0     Sets VX to the value of VY. */
                    VX = VY;
                break;

                case 0x0001:
                /*   0x8XY1     Sets VX to VX or VY. */
                    VX |= VY;
                break;

                case 0x0002:
                /*   0x8XY2     Sets VX to VX and VY. */
                    VX &= VY;
                break;

                case 0x0003:
                /*   0x8XY3     Sets VX to VX xor VY. */
                    VX ^= VY;
                break;

                case 0x0004:
                /*   0x8XY4     Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't. */
                    VF = ((int) VX + VY) > 0xFF;
                    VX += VY;
                break;

                case 0x0005:
                /*   0x8XY5     VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't. */
                    VF = ((int) VX - VY) < 0;
                    VX -= VY;
                break;

                case 0x0006:
                /*   0x8XY6     Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift. */
                    VF = VX & 1;
                    VX >>= 1;
                break;

                case 0x0007:
                /*   0x8XY7     Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't. */
                    VF = ((int) VY - VX) < 0;
                    VX = VY - VX;
                break;

                case 0x000E:
                /*   0x8XYE     Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift. */
                    VF = VX >= 0x80;
                    VX <<= 1;
                break;

                default :
                    Cpu_unknownOpcode (this);
                break;
            }
        break;

        case 0x9000:
        /*   0x9XY0     Skips the next instruction if VX doesn't equal VY. */
            if (VX != VY) {
            	this->ip += 2;
            }
        break;

        case 0xA000:
        /*   0xANNN     Sets I to the address NNN. */
            this->I = _NNN;
        break;

        case 0xB000:
        /*   0xBNNN     Jumps to the address NNN plus V0. */
            this->ip = _NNN + V[0];
        break;

        case 0xC000:
        /*  CXNN     Sets VX to a random number and NN. */
            VX = (rand() >> 7) & __NN;
        break;

        case 0xD000:
        /*   0xDXYN        Sprites stored in memory at location in index register (I), maximum 8bits wide.
                        Wraps around the screen.
                        If when drawn, clears a pixel, register VF is set to 1 otherwise it is zero.
                        All drawing is XOR drawing (e.g. it toggles the screen pixels)
        */
            // Set VF to 1 if a pixel changed from 1 to 0
            VF = Screen_drawSprite (this->screen, VX, VY, ___N, this->memory, this->I);
        break;

        case 0xE000:
            switch (opcode & 0x00FF)
            {
                case 0x009E:
                /*   0xEX9E     Skips the next instruction if the key stored in VX is pressed. */
                    if (Window_getKeyState (VX) == KEY_PRESSED) {
                    	this->ip += 2;
                    }
                break;

                case 0x00A1:
                /*   0xEXA1     Skips the next instruction if the key stored in VX isn't pressed. */
                    if (Window_getKeyState (VX) == KEY_RELEASED) {
                    	this->ip += 2;
                    }
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
                /*   0xFX07     Sets VX to the value of the delay timer. */
                    VX = this->delayTimer;
                break;

                case 0x000A: {
                /*   0xFX0A     A key press is awaited, and then stored in VX. */
                    bool keyPressed = false;
                    for (C8KeyCode code = 0; !keyPressed && code < keyCodeCount; code++) {
                        if (Window_getKeyState (code) == KEY_PRESSED) {
                            VX = code;
                            keyPressed = true;
                            // The CPU loop is way faster than the I/O handler one.
                            // Thus, the CPU has the right to notify than the key
                            // has been handled as pressed and shouldn't be
                            // handled twice.
                            Window_setKeyState (code, KEY_PUSHED);
                        }
                    }

                    // Only step to the next instruction if a key has been pressed
                    if (!keyPressed) {
                        this->ip -= INSN_SIZE;
                    }
                }
                break;

                case 0x0015:
                /*   0xFX15     Sets the delay timer to VX. */
                    this->delayTimer = VX;
                break;

                case 0x0018:
                /*   0xFX18     Sets the sound timer to VX. */
                    this->soundTimer = VX;
                break;

                case 0x001E:
                /*   0xFX1E     Adds VX to I. */
                    this->I += VX;
                break;

                case 0x0029:
                /*   0xFX29     Sets I to the location of the sprite for the character in VX.
                                Characters 0-F (in hexadecimal) are represented by a 4x5 font. */
                    this->I = 5 * (VX & 0xF);
                break;

                case 0x0033:
                /*   0xFX33     Stores the Binary-coded decimal representation of VX, with the most significant of three digits at the address in I,
                                the middle digit at I plus 1, and the least significant digit at I plus 2.
                                (In other words, take the decimal representation of VX, place the hundreds digit in memory at location in I,
                                the tens digit at location I+1, and the ones digit at location I+2.) */
                    this->memory[this->I]     =  VX / 100;
                    this->memory[this->I + 1] = (VX / 10)  % 10;
                    this->memory[this->I + 2] = VX % 10;
                break;

                case 0x0055:
                /*   0xFX55     Stores V0 to VX in memory starting at address I. */
                    for (int pos = 0; pos <= _X__; pos++) {
                        this->memory [this->I + pos] = V[pos];
                    }
                break;

                case 0x0065:

                /*   0xFX65     Fills V0 to VX with values from memory starting at address I. */
                    for (int pos = 0; pos <= _X__; pos++) {
                        V[pos] = this->memory [this->I + pos];
                    }
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

    // Clean macro namespace
    #undef ___N
    #undef __NN
    #undef _NNN
    #undef VX
    #undef VY
    #undef VF
}


/*
 * Description : Disassemble in the console the current opcode
 * Cpu *this : An allocated Cpu
 * Return : void
 */
void
Cpu_disass (
	Cpu *this
) {
    uint16_t opcode = this->opcode;

    char v1[3] = "Vx";
    char v2[3] = "Vx";
    sprintf(v1, "V%X", ((opcode & 0x0F00) >> 8));
    sprintf(v2, "V%X", ((opcode & 0x00F0) >> 4));

    printf ("IP = %04X| %04X - ", this->ip, opcode);

    switch (opcode>>12)
    {
		case 0:
		switch (opcode&0xfff)
		{
			case 0xe0:	printf ("CLS          ; Clear screen");												break;
			case 0xee:	printf ("RET          ; Return from subroutine call");								break;
			case 0xfb:	printf("SCR           ; Scroll right");												break;
			case 0xfc:	printf("SCL           ; Scroll left");												break;
			case 0xfd:	printf("EXIT          ; Terminate the interpreter");								break;
			case 0xfe:	printf("LOW           ; Disable extended screen mode");								break;
			case 0xff:	printf("HIGH          ; Enable extended screen mode");								break;
			default:	printf ("SYS  %03X     ; Unknown system call",opcode&0xff);							break;
		}
		break;

		case 1:	printf ("JP   %03X     ; Jump to address",opcode&0xfff);									break;
		case 2:	printf ("CALL %03X     ; Call subroutine",opcode&0xfff);									break;
		case 3:	printf ("SE   %s,%02X   ; Skip if register == constant",v1,opcode&0xff);					break;
		case 4:	printf ("SNE  %s,%02X   ; Skip if register <> constant",v1,opcode&0xff);					break;
		case 5:	printf ("SE   %s,%s   ; Skip if register == register",v1,v2);								break;
		case 6:	printf ("LD   %s,%02X   ; Set VX = Byte",v1,opcode&0xff);									break;
		case 7:	printf ("ADD  %s,%02X   ; Set VX = VX + Byte",v1,opcode&0xff);								break;

		case 8:
		switch (opcode & 0x0f)
		{
			case 0:	printf ("LD   %s,%s   ; Set VX = VY, VF updates",v1,v2);								break;
			case 1:	printf ("OR   %s,%s   ; Set VX = VX | VY, VF updates",v1,v2);							break;
			case 2:	printf ("AND  %s,%s   ; Set VX = VX & VY, VF updates",v1,v2);							break;
			case 3:	printf ("XOR  %s,%s   ; Set VX = VX ^ VY, VF updates",v1,v2);							break;
			case 4:	printf ("ADD  %s,%s   ; Set VX = VX + VY, VF = carry",v1,v2);							break;
			case 5:	printf ("SUB  %s,%s   ; Set VX = VX - VY, VF = !borrow",v1,v2);							break;
			case 6:	printf ("SHR  %s,%s   ; Set VX = VX >> 1, VF = carry",v1,v2);							break;
			case 7:	printf ("SUBN %s,%s   ; Set VX = VY - VX, VF = !borrow",v1,v2);							break;
			case 14:	printf ("SHL  %s,%s   ; Set VX = VX << 1, VF = carry",v1,v2);						break;
			default:	printf ("Illegal opcode");															break;
		}
		break;

		case 9:	printf ("SNE  %s,%s   ; Skip next instruction iv VX!=VY",v1,v2);							break;
		case 10:	printf ("LD   I,%03X   ; Set I = Addr",opcode&0xfff);									break;
		case 11:	printf ("JP   V0,%03X  ; Jump to Addr + V0",opcode&0xfff);								break;
		case 12:	printf ("RND  %s,%02X   ; Set VX = random & Byte",v1,opcode&0xff);						break;
		case 13:	printf ("DRW  %s,%s,%X ; Draw n byte sprite stored at [i] at VX,VY.",v1,v2,opcode & 0x0f); break;

		case 14:
		switch (opcode & 0xff)
		{
			case 0x9e:	printf ("SKP  %s      ; Skip next instruction if key VX down",v1);					break;
			case 0xa1:	printf ("SKNP %s      ; Skip next instruction if key VX up",v1);					break;
			default:	printf ("%04X        ; Illegal opcode", opcode);									break;
		}
		break;

		case 15:
		switch (opcode & 0xff)
		{
			case 0x07:	printf ("LD   %s,DT   ; Set VX = delaytimer",v1);									break;
			case 0x0a:	printf ("LD   %s,K    ; Set VX = key, wait for keypress",v1);						break;
			case 0x15:	printf ("LD   DT,%s   ; Set delaytimer = VX",v1);									break;
			case 0x18:	printf ("LD   ST,%s   ; Set soundtimer = VX",v1);									break;
			case 0x1e:	printf ("ADD  I,%s    ; Set I = I + VX",v1);										break;
			case 0x29:	printf ("LD  LF,%s    ; Point I to 5 byte numeric sprite for value in VX",v1);		break;
			case 0x30:	printf ("LD  HF,%s    ; Point I to 10 byte numeric sprite for value in VX",v1);		break;
			case 0x33:	printf ("LD   B,%s    ; Store BCD of VX in [I], [I+1], [I+2]",v1);					break;
			case 0x55:	printf ("LD   [I],%s  ; Store V0..VX in [I]..[I+X]",v1);							break;
			case 0x65:	printf ("LD   %s,[I]  ; Read V0..VX from [I]..[I+X]",v1);							break;
			case 0x75:	printf ("LD   R,%s    ; Store V0..VX in RPL user flags (X<=7)",v1);					break;
			case 0x85:	printf ("LD   %s,R    ; Read V0..VX from RPL user flags (X<=7)",v1);				break;
			default:	printf ("%04X        ; Illegal opcode", opcode);									break;
		}
		break;
    }

    printf(" (%s = %x |", v1, this->V[((opcode & 0x0F00) >> 8)]);
    printf("  %s = %x)",  v2, this->V[((opcode & 0x00F0) >> 4)]);

    printf("\n");
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
			// Update CPU timers
			Cpu_updateTimers (this);

            sfSleep (sfSeconds(0.01));
        }
    }
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
