#ifndef CPU_H
#define CPU_H

#include "Memory.h"
#include "Typedefs.h"

#include <functional>

class OpParser; // forward declaration.

struct Register {
	union {
		struct {
			u8 F;
			u8 A;
		};
		u16 AF;
	};
	union {
		struct {
			u8 C;
			u8 B;
		};
		u16 BC;
	};
	union {
		struct {
			u8 E;
			u8 D;
		};
		u16 DE;
	};
	union {
		struct {
			u8 L;
			u8 H;
		};
		u16 HL;
	};

	u16 SP; // Stack pointer
	u16 PC; // Program counter
};

union StatusRegister {
	struct {
		u8 C : 1; // Carry flag
		u8 H : 1; // Half-carry flag
		u8 N : 1; // Subtract flag
		u8 Z : 1; // Zero flag
		u8 unused : 4;
	};
	u8 status;
};

struct ConstNums {
	u8  b0, b1, b2, b3, b4, b5, b6, b7;
	u16 w0, w1, w2, w3, w4, w5, w6, w7;
};

class Cpu
{
public:
	Cpu(Memory&);

	void read_opcode();
#ifndef _DEBUG
private:
#endif
	typedef std::function<void()> OpInstr;
	friend OpParser;

	Memory& memory;
	Register reg;
	StatusRegister status_register;
	unsigned clock_cyles;

	bool halt;
	bool interrupt;

	OpInstr instr_op[0x200];
	
	void load_instruction_array();

	void set_status_register(const u8&, bool, bool, bool, bool);
	
	u8 read_byte_increment_PC();
	u16 read_word_increment_PC();

	// 8-bit functions
	void load_byte(u8&, const u8&);
	void load_byte_switch(u8&, const u8&);

	void add_byte(u8&, const u8&);
	void add_byte_c(u8&, const u8&);
	void sub_byte(u8&, const u8&);
	void sub_byte_c(u8&, const u8&);

	void and_byte(u8&, const u8&);
	void or_byte(u8&, const u8&);
	void xor_byte(u8&, const u8&);

	void cp_byte(u8&, const u8&);
	void inc_byte(u8&, [[maybe_unused]] const u8&);
	void dec_byte(u8&, [[maybe_unused]] const u8&);

	void load_sp_hl();
	void load_memory_inc();
	void load_memory_dec();
	void load_memory_imm();
	void alu_byte_imm(u8&, const u8&);
	void daa();
	void cpl();
	void ccf();
	void scf();
	void nop();

	void swap_nibble(u8&, [[maybe_unused]] const u8&);

	void rotate_byte(u8&, const u8&);
	void shift_byte(u8&, const u8&);

	void test_bit(u8&, const u8&);
	void set_bit(u8&, const u8&);
	void reset_bit(u8&, const u8&);

	void jump();
	void jump_hl();
	void jump_relative();

	void call();
	void ret();
	void reti();
	void load_imm_off();

	void halt_cpu(); //implement!
	void stop_cpu(); // implement!
	void di(); // implement!
	void ei(); // implement!

	// 16-bit functions
	void load_word(u16&, [[maybe_unused]] const u16&);
	void load_stack();
	void load_hl(u16&, const u16&);

	void add_word(u16&, const u16&);
	void add_sp(u16&, [[maybe_unused]] const u16&);
	void inc_word(u16&, [[maybe_unused]] const u16&);
	void dec_word(u16&, [[maybe_unused]] const u16&);

	void push(u16&, const u16&);
	void pop(u16&, [[maybe_unused]] const u16&);

	void jump_cp(u16&, const u16&);
	void jump_relative_cp(u16&, const u16&);

	void call_cp(u16&, const u16&);
	void ret_cp(u16&, const u16&);
	void restart(u16&, const u16&);
};

#endif
