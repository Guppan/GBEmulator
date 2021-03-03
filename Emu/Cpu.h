#pragma once

#include "Memory.h"

struct Register {
	union {
		struct {
			unsigned char F;
			unsigned char A;
		};
		unsigned short AF;
	};
	union {
		struct {
			unsigned char C;
			unsigned char B;
		};
		unsigned short BC;
	};
	union {
		struct {
			unsigned char E;
			unsigned char D;
		};
		unsigned short DE;
	};
	union {
		struct {
			unsigned char L;
			unsigned char H;
		};
		unsigned short HL;
	};

	unsigned short SP; // Stack pointer
	unsigned short PC; // Program counter
};

union StatusRegister {
	struct {
		unsigned char unused : 4;
		unsigned char C : 1; // Carry flag
		unsigned char H : 1; // Half-carry flag
		unsigned char N : 1; // Subtract flag
		unsigned char Z : 1; // Zero flag
	};
	unsigned char status;
};

class Cpu
{
public:
	Cpu(Memory&);

	void read_opcode();
private:
	Register reg;
	StatusRegister status_register;

	Memory& memory;
};

