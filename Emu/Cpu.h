#ifndef CPU_H
#define CPU_H

#include "Memory.h"
#include "Typedefs.h"

struct Register {
	union {
		struct {
			BYTE F;
			BYTE A;
		};
		WORD AF;
	};
	union {
		struct {
			BYTE C;
			BYTE B;
		};
		WORD BC;
	};
	union {
		struct {
			BYTE E;
			BYTE D;
		};
		WORD DE;
	};
	union {
		struct {
			BYTE L;
			BYTE H;
		};
		WORD HL;
	};

	WORD SP; // Stack pointer
	WORD PC; // Program counter
};

union StatusRegister {
	struct {
		BYTE unused : 4;
		BYTE C : 1; // Carry flag
		BYTE H : 1; // Half-carry flag
		BYTE N : 1; // Subtract flag
		BYTE Z : 1; // Zero flag
	};
	BYTE status;
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
	
	BYTE read_byte_increment_PC();
	WORD read_word_increment_PC();
};

#endif
