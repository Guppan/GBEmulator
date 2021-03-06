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
		BYTE C : 1; // Carry flag
		BYTE H : 1; // Half-carry flag
		BYTE N : 1; // Subtract flag
		BYTE Z : 1; // Zero flag
		BYTE unused : 4;
	};
	BYTE status;
};

class Cpu
{
public:
	Cpu(Memory&);

	void read_opcode();
//private: // comment this line for testing.
	Memory& memory;
	Register reg;
	StatusRegister status_register;
	unsigned clock_cyles;

	void set_status_register(const BYTE&, bool, bool, bool, bool);
	
	BYTE read_byte_increment_PC();
	WORD read_word_increment_PC();

	void load_byte_instruction(BYTE&, const BYTE&, WORD&, const WORD&, unsigned);
	void load_word_instruction(BYTE&, const BYTE&, WORD&, const WORD&, unsigned);
	void load_word_stack_instruction(WORD&, const WORD&, unsigned);
	void load_word_hl_instruction(WORD&, const WORD&, unsigned);

	void push_instruction(const WORD&, unsigned);
	void pop_instruction(WORD&, unsigned);

	void add_instruction(BYTE&, const BYTE&, unsigned);
	void sub_instruction(BYTE&, const BYTE&, unsigned);

	void and_instruction(BYTE&, const BYTE&, unsigned);
	void or_instruction(BYTE&, const BYTE&, unsigned);
	void xor_instruction(BYTE&, const BYTE&, unsigned);

	void cp_instruction(BYTE&, const BYTE&, unsigned);
	
	void inc_instruction(BYTE&, unsigned);
	void dec_instruction(BYTE&, unsigned);

	void add_word_instruction(WORD&, const WORD&, unsigned);
	void add_word_sp_instruction(WORD&, unsigned);

	void inc_word_instruction(WORD&, unsigned);
	void dec_word_instruction(WORD&, unsigned);

	void daa_instruction(unsigned);
	void cpl_instruction(unsigned);
	void ccf_instruction(unsigned);
	void scf_instruction(unsigned);
};

#endif
