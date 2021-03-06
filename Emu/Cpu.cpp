#include "Cpu.h"

// ------ Constants ------------------------------------------
// Used primarily in method 'set_status_register' to indicate
// that an argument is unused.
const bool UNUSED{ false };

// Used to indicate wheter a bit is set or cleared.
const bool SET_BIT{ true };
const bool CLEAR_BIT{ false };

// -----------------------------------------------------------

typedef void (Cpu::*instruction_ptr)(BYTE&, const BYTE&, WORD&, const WORD&, unsigned);

Cpu::Cpu(Memory& mem) : 
	memory{ mem },
	reg{}, 
	status_register{},
	clock_cyles{}
{}


void Cpu::read_opcode() {
	/*
	* läs instruktion från adress 'PC'.
	* unsigned char opcode = memory.read_byte(reg.PC);
	*	
	* ifall opcode = 0xCB, 0x01 så är instruktionen ett word.
	* 
	* if (opcode = 0xCB){
	* 
	* }
	* else{
	* 
	* }

	*/
}

// Sets the status register according to the 'active_mask' and bit-values.
// For example: set_status_register(0x0F, 1 1 0 1) would set Z, N and C to 1
// and clear H.
void Cpu::set_status_register(const BYTE& active_mask, bool _Z, bool _N, bool _H, bool _C) {
	BYTE set_bits = (_Z << 3) + (_N << 2) + (_H << 1) + _C;
	status_register.status |= (active_mask & set_bits);		// set bits to 1.
	status_register.status &= ~(active_mask ^ set_bits);	// clear bits.
}


BYTE Cpu::read_byte_increment_PC() {
	return memory[reg.PC++];
}


WORD Cpu::read_word_increment_PC() {
	WORD word{ memory.read_word(reg.PC) };
	reg.PC += 2;
	return word;
}

/*
* Load functions:
* 
* immediate till byte-register -> load(BYTE& to) -> t.ex. load(reg.A)
* 
* byte-register till byte-register -> load(BYTE& to, BYTE& from) -> t.ex. load(reg.A, reg.B)
* 
* adress av word-register till byte-register -> load(BYTE& to, WORD& from) -> t.ex. load(reg.A, reg.BC)
* 
* byte-register till adress av word-register -> load(WORD& to, BYTE& from) -> t.ex. load(reg.BC, reg.A)
* 
*/


// Load 'from' into 'to'.
// 78 79 7A 7B 7C 7D 7E 7F 0A 1A F2	2A 3A F0 FA	3E	'to' = A
// 40 41 42 43 44 45 46	47 06						'to' = B
// 48 49 4A 4B 4C 4D 4E	4F 0E						'to' = C
// 50 51 52 53 54 55 56	57 16						'to' = D
// 58 59 5A 5B 5C 5D 5E	5F 1E						'to' = E
// 60 61 62 63 64 65 66	67 26						'to' = H
// 68 69 6A 6B 6C 6D 6E	6F 2E						'to' = L
// 70 71 72 73 74 75 77 36 22 32					'to' = (HL)
// 02												'to' = (BC)
// 12												'to' = (DE)
// E2												'to' = (0xFF00 + C)
// E0												'to' = (0xFF00 + n)
// EA												'to' = (nn)
void Cpu::load_byte_instruction(BYTE& to, const BYTE& from, [[maybe_unused]] WORD&, [[maybe_unused]] const WORD&, unsigned cycles) {
	to = from;
	clock_cyles += cycles;

	// load(reg.A, reg.B)
	// load(memory[reg.HL], reg.A)
	// load(reg.A, memory[0xFF00 + reg.C]) - F2
	// load(reg.A, memory[reg.HL--]) - 3A
	// load(memory[reg.HL--], reg.A) - 32
	// load(reg.A, memory[reg.HL++]) - 2A
	// load(memory[reg.HL++], reg.A) - 22
	// load(memory[0xFF00 + reg.C], reg.A) - E2
	// load(memory[0xFF00 + read_byte_increment_PC()], reg.A) - E0
	// load(reg.A, memory[0xFF00 + read_byte_increment_PC()]) - F0
	// load(memory[read_word_increment_PC()], reg.A) - EA
	// load(reg.A, memory[read_word_increment_PC()]) - FA
	// load(reg.A, read_byte_increment_PC()) - 3E
	// load(reg.B, read_byte_increment_PC()) - 06

}





/*
* 01				'to' = BC
* 11				'to' = DE
* 21				'to' = HL
* 31 F9				'to' = SP
*/
void Cpu::load_word_instruction([[maybe_unused]] BYTE&, [[maybe_unused]] const BYTE&, WORD& to, const WORD& from, unsigned cycles) {
	to = from;
	clock_cyles += cycles;

	// load(reg.BC, read_word_increment_PC()) - 01
	// load(reg.SP, reg.HL) - F9
}


/*
* LD (nn),SP instruction
* 
* 08
* 
* load_word_stack_instruction(read_word_increment_PC(), reg.SP)
*/
void Cpu::load_word_stack_instruction(WORD& to, const WORD& from, unsigned cycles) {
	memory.write_word(to, from);
	clock_cyles += cycles;
}


/*
* LDHL SP,n instruction
* 
* F8
* 
* load_word_hl_instruction(reg.HL, reg.SP)
*/
void Cpu::load_word_hl_instruction(WORD& to, const WORD& from, unsigned cycles) {
	BYTE immediate{ read_byte_increment_PC() };
	const bool sign{ (immediate & 0x80) == 0x80 };
	immediate = (immediate & 0x7F);

	bool _H, _C;
	if (sign) { // immediate is signed so we must subtract.
		_H = ((((from & 0x0F) - (immediate & 0x0F)) & 0x10) == 0x10);
		_C = (from < immediate);
		to = (from - immediate);
	}
	else { // immediate is positive so we must add.
		_H = ((((from & 0x0F) + (immediate & 0x0F)) & 0x10) == 0x10);
		_C = (((from + immediate) & 0x100) == 0x100);
		to = (from + immediate);
	}

	set_status_register(
		0x0F,
		CLEAR_BIT,
		CLEAR_BIT,
		_H,
		_C);
	clock_cyles += cycles;
}


/*
* push instruction.
* 
* C5 D5 E5 F5
*/
void Cpu::push_instruction(const WORD& from, unsigned cycles) {
	memory[--reg.SP] = static_cast<BYTE>(from >> 8);
	memory[--reg.SP] = static_cast<BYTE>(from);

	clock_cyles += cycles;
}


/*
* pop instruction.
* 
* C1 D1 E1 F1
*/
void Cpu::pop_instruction(WORD& to, unsigned cycles) {
	to = memory[reg.SP++];
	to += (memory[reg.SP++] << 8);

	clock_cyles += cycles;
}


/*
* add instruction.
*	
*	0xFF + 0x01 = 0x00 -> Z=1, H=1, C=1
*	
*	zero-check		 : (((a + b) & 0xFF) == 0x00)
*	half-carry check : ((((a & 0x0F) + (b & 0x0F)) & 0x10) == 0x10)
*	carry-check      : (((a + b) & 0x100) == 0x100)
* 
*	80 81 82 83 84 85 86 87 C6	'to' = A
*	t.ex. add(reg.A, reg.B)
* 
*	88 89 8A 8B 8C 8D 8E 8F CE	'to' = A
*	t.ex. add(reg.A, (reg.B + status_register.C))
*/
void Cpu::add_instruction(BYTE& to, const BYTE& from, unsigned cycles) {
	set_status_register(
		0x0F,
		(((to + from) & 0xFF) == 0x00),
		CLEAR_BIT,
		((((to & 0x0F) + (from & 0x0F)) & 0x10) == 0x10),
		(((to + from) & 0x100) == 0x100));

	to += from;
	clock_cyles += cycles;
}


/*
* sub instruction.
* 
* 90 91 92 93 94 95 96 97 D6	'to' = A
* t.ex. sub(reg.A, reg.B)
* 
* 98 99 9A 9B 9C 9D 9E 9F DE	'to' = A
* t.ex. add(reg.A, (reg.B + status_register.C))
*/
void Cpu::sub_instruction(BYTE& to, const BYTE& from, unsigned cycles) {
	set_status_register(
		0x0F,
		(to == from),
		SET_BIT,
		((((to & 0x0F) - (from & 0x0F)) & 0x10) == 0x10),
		(to < from));

	to -= from;
	clock_cyles += cycles;
}


/*
* and instruction.
* 
* A0 A1 A2 A3 A4 A5 A6 A7 E6
* 
*/
void Cpu::and_instruction(BYTE& to, const BYTE& from, unsigned cycles) {
	set_status_register(
		0x0F,
		((to & from) == 0x00),
		CLEAR_BIT,
		SET_BIT,
		CLEAR_BIT);

	to &= from;
	clock_cyles += cycles;
}


/*
* or instruction
* 
* B0 B1 B2 B3 B4 B5 B6 B7 F6
*/
void Cpu::or_instruction(BYTE& to, const BYTE& from, unsigned cycles) {
	set_status_register(
		0x0F,
		((to | from) == 0x00),
		CLEAR_BIT,
		CLEAR_BIT,
		CLEAR_BIT);

	to |= from;
	clock_cyles += cycles;
}


/*
* xor instruction.
* 
* A8 A9 AA AB AC AD AE AF EE
*/
void Cpu::xor_instruction(BYTE& to, const BYTE& from, unsigned cycles) {
	set_status_register(
		0x0F,
		((to ^ from) == 0x00),
		CLEAR_BIT,
		CLEAR_BIT,
		CLEAR_BIT);

	to ^= from;
	clock_cyles += cycles;
}


/*
* cp instruction.
* Same as sub_instruction but discards the result.
* 
* B8 B9 BA BB BC BD BE BF FE	'to' = A
*/
void Cpu::cp_instruction(BYTE& to, const BYTE& from, unsigned cycles) {
	set_status_register(
		0x0F,
		(to == from),
		SET_BIT,
		((((to & 0x0F) - (from & 0x0F)) & 0x10) == 0x10),
		(to < from));

	clock_cyles += cycles;
}


/*
* inc instruction.
* 
* 04 14 24 34 0C 1C 2C 3C
* 
* t.ex. inc(reg.A)
*/
void Cpu::inc_instruction(BYTE& to, unsigned cycles) {
	set_status_register(
		0x0E,
		((to + 0x01) == 0x00),
		CLEAR_BIT,
		((((to & 0x0F) + 0x01) & 0x10) == 0x10),
		UNUSED);

	++to;
	clock_cyles += cycles;
}


/*
* dec instruction.
* 
* 05 15 25 35 0D 1D 2D 3D
* 
* t.ex. dec(reg.A)
*/
void Cpu::dec_instruction(BYTE& to, unsigned cycles) {
	set_status_register(
		0x0E,
		((to - 0x01) == 0x00),
		SET_BIT,
		((((to & 0x0F) - 0x01) & 0x10) == 0x10),
		UNUSED);

	--to;
	clock_cyles += cycles;
}



/*
* add nn to HL
* 
* 09 19 29 39
* 
* t.ex. add_word_instruction(reg.HL, reg.BC)
*/
void Cpu::add_word_instruction(WORD& to, const WORD& from, unsigned cycles) {
	set_status_register(
		0x07,
		UNUSED,
		CLEAR_BIT,
		((((to & 0x0FFF) + (from & 0x0FFF)) & 0x1000) == 0x1000),
		(((to + from) & 0x10000) == 0x10000));

	to += from;
	clock_cyles += cycles;
}



/*
* add immediate byte to SP.
* 
* E8
* 
* t.ex. add_word_sp_instruction(reg.SP)
*/
void Cpu::add_word_sp_instruction(WORD& to, unsigned cycles) {
	BYTE immediate{ read_byte_increment_PC() };
	const bool sign{ (immediate & 0x80) == 0x80 };
	immediate &= 0x7F;

	bool _H, _C;
	if (sign) {
		_H = ((((to & 0x0FFF) - immediate) & 0x1000) == 0x1000);
		_C = (to < immediate);
		to -= immediate;
	}
	else {
		_H = ((((to & 0x0FFF) + immediate) & 0x1000) == 0x1000);
		_C = (((to + immediate) & 0x10000) == 0x10000);
		to += immediate;
	}

	set_status_register(
		0x0F,
		CLEAR_BIT,
		CLEAR_BIT,
		_H,
		_C);
	clock_cyles += cycles;
}


/*
* inc word with 0x01.
* 
* 03 13 23 33
* 
* t.ex. inc(reg.BC)
*/
void Cpu::inc_word_instruction(WORD& to, unsigned cycles) {
	++to;
	clock_cyles += cycles;
}


/*
* dec word with 0x01.
* 
* 0B 1B 2B 3B
* 
* t.ex. dec(reg.BC)
*/
void Cpu::dec_word_instruction(WORD& to, unsigned cycles) {
	--to;
	clock_cyles += cycles;
}


/*
* bcd adjust register A
* 
* 27
*/
void Cpu::daa_instruction(unsigned cycles) {
	if (!status_register.N) { // Was the last operation an addition?
		if (status_register.C || (reg.A > 0x99)) {
			reg.A += 0x60;
			status_register.C = 0x01;
		}
		if (status_register.H || ((reg.A & 0x0F) > 0x09)) {
			reg.A += 0x06;
		}
	}
	else { // Was the last operation a subtraction?
		if (status_register.C) {
			reg.A -= 0x60;
		}
		if (status_register.H) {
			reg.A -= 0x06;
		}
	}

	status_register.Z = (reg.A == 0x00);
	status_register.H = 0;
	clock_cyles += cycles;
}


/*
* complement A.
* 
* 2F
*/
void Cpu::cpl_instruction(unsigned cycles) {
	set_status_register(
		0x06,
		UNUSED,
		SET_BIT,
		SET_BIT,
		UNUSED);

	reg.A = ~reg.A;
	clock_cyles += cycles;
}


/*
* complement carry flag.
* 
* 3F
*/
void Cpu::ccf_instruction(unsigned cycles) {
	set_status_register(
		0x07,
		UNUSED,
		CLEAR_BIT,
		CLEAR_BIT,
		~status_register.C);
	clock_cyles += cycles;
}


/*
* set carry flag.
* 
* 37
*/
void Cpu::scf_instruction(unsigned cycles) {
	set_status_register(
		0x07,
		UNUSED,
		CLEAR_BIT,
		CLEAR_BIT,
		SET_BIT);
	clock_cyles += cycles;
}