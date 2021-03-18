#ifndef CPU_H
#define CPU_H

#include "Device.h"

#include <functional>

class OpParser; // forward declaration.

struct Register {
	union {
		struct {
			union {	// ----------- Flag register
				struct {
					u8 C : 1;	// Carry flag
					u8 H : 1;	// Half carry flag
					u8 N : 1;	// Negative flag
					u8 Z : 1;	// Zero flag
					u8 unused : 4;
				} flag;
				u8 F;
			};	// ----------------------------
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

struct ConstNums {
	u8  b0, b1, b2, b3, b4, b5, b6, b7;
	u16 w0, w1, w2, w3, w4, w5, w6, w7;
};

class Cpu : Device
{
public:
	Cpu();

	void read_opcode();

	u8 read_byte(const u16) override;
	void write_byte(const u16, const u8) override;
#ifndef _DEBUG
private:
#endif
	friend OpParser;

	Register reg;
	unsigned clock_cyles;

	bool halt;
	bool interrupt;

	std::function<void()> op_matrix[0x200];
	unsigned cycle_matrix[0x200];
	
	void load_instruction_array();

	void set_flags(const u8&, bool, bool, bool, bool);
	
	u8 read_byte_inc_PC();
	u16 read_word_inc_PC();

	void load(const u8);
	void arithmetic(const u8);
	void bitop(const u8);
	void wordop(const u8);

	// -------- Instruction Methods --------
	// Methods used to construct all the 500 instructions.
	// 8bit arguments
	void add_byte(const u8);
	void sub_byte(const u8);
	void and_byte(const u8);
	void or_byte(const u8);
	void xor_byte(const u8);
	void cp_byte(const u8);
	void inc(const u8);
	void inc_reg(u8&);
	void dec(const u8);
	void dec_reg(u8&);

	u8 swap(const u8);
	u8 rotate(const u8, const u8);
	u8 shift(const u8, const u8);

	void bit(const u8, const u8);
	u8 set(const u8, const u8);
	u8 res(const u8, const u8);

	// 16bit arguments
	void load_hl();
	void add_word(const u16);
	void add_sp();
	void inc_word(u16&, [[maybe_unused]] const u16&);
	void dec_word(u16&, [[maybe_unused]] const u16&);
	void push(const u16);
	void pop(u16&);
	void jump_cp(const u8);
	void jump_relative_cp(const u8);
	void call_cp(const u8);
	void ret_cp(const u8);
	void restart(const u8);

	// no arguments
	void load_imm_off();
	void store_sp();
	void load_sp_hl();
	void load_memory_inc();
	void load_memory_dec();
	void load_memory_imm();
	void daa();
	void cpl();
	void ccf();
	void scf();
	void nop();
	void jump();
	void jump_hl();
	void jump_relative();
	void call();
	void ret();
	void reti();
	void halt_cpu(); //implement!
	void stop_cpu(); // implement!
	void di(); // implement!
	void ei(); // implement!
};

#endif
