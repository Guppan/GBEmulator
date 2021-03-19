#ifndef CPU_H
#define CPU_H

#include "Device.h"


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

	bool IME; // Interrupt master enable
};

class Cpu : public Device
{
public:
	Cpu();
	
	void read_opcode();

	u8 read_byte(const u16) const override;
	void write_byte(const u16, const u8) override;
#ifndef _DEBUG
private:
#endif
	Register reg;
	unsigned clock_cyles;
	unsigned cycle_matrix[0x200];

	void init_cycle_matrix();
	void set_flags(const u8&, bool, bool, bool, bool);
	
	u8 read_byte_inc_PC();
	u16 read_word_inc_PC();

	void regular_op(const u8);
	void extended_op(const u8);

	// ------ Methods used by instructions -------
	void add_byte(const u8);
	void sub_byte(const u8);
	void and_byte(const u8);
	void or_byte(const u8);
	void xor_byte(const u8);
	void cp_byte(const u8);
	u8 inc(const u8);
	u8 dec(const u8);

	u8 swap(const u8);
	u8 rotate(const u8, const u8);
	u8 shift(const u8, const u8);

	void bit(const u8, const u8);
	u8 set(const u8, const u8);
	u8 res(const u8, const u8);

	void load_hl();
	void add_word(const u16);
	void add_sp();
	void push(const u16);
	void pop(u16&);
	void jump_cp(const u8);
	void jump_relative_cp(const u8);
	void call_cp(const u8);
	void ret_cp(const u8);
	void restart(const u8);

	void store_sp();
	void daa();
	void cpl();
	void ccf();
	void scf();
	void jump_relative();
	void call();
	void reti();
	void halt_cpu(); //implement!
	void stop_cpu(); // implement!
	void di(); // implement!
	void ei(); // implement!
};

#endif // CPU_H
