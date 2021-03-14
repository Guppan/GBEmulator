#include "Cpu.h"
#include "OpParser.h"

#include <iostream>

// ------ Constants ------------------------------------------
// Used primarily in method 'set_status_register' to indicate
// that an argument is unused.
const bool UNUSED{ false };

// Used to indicate wheter a bit is set or cleared.
const bool SET_BIT{ true };
const bool CLEAR_BIT{ false };

ConstNums CONST_NUMS{
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x0000, 0x0001, 0x0002, 0x0003,
	0x0004, 0x0005, 0x0006, 0x0007
};
// -----------------------------------------------------------


Cpu::Cpu(Memory& mem) :
	memory{ mem },
	reg{},
	status_register{},
	clock_cyles{},
	halt{ false },
	interrupt{ false }
{
	reg.D = 0x01;
	reg.B = 0x11;
	OpParser par{ *this, CONST_NUMS };
	std::cout << "before fn call\nD : " << reg.D << "   -   B : " << reg.B << std::endl;
	instr_op[0x50]();
	std::cout << "after fn call\nD : " << reg.D << "   -   B : " << reg.B << std::endl;
}


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


void Cpu::load_instruction_array() {
}


// Sets the status register according to the 'active_mask' and bit-values.
// For example: set_status_register(0x0F, 1 1 0 1) would set Z, N and C to 1
// and clear H.
void Cpu::set_status_register(const u8& active_mask, bool _Z, bool _N, bool _H, bool _C) {
	u8 set_bits = (_Z << 3) + (_N << 2) + (_H << 1) + _C;
	status_register.status |= (active_mask & set_bits);		// set bits to 1.
	status_register.status &= ~(active_mask ^ set_bits);	// clear bits.
}


u8 Cpu::read_byte_increment_PC() {
	return memory[reg.PC++];
}


u16 Cpu::read_word_increment_PC() {
	u16 word{ memory.read_word(reg.PC) };
	reg.PC += 2;
	std::cout << "inside read_word_increment_PC" << std::endl;
	return word;
}

void Cpu::load_byte(u8& to, const u8& from) {
	to = from;
	std::cout << "inside load_byte" << std::endl;
}


void Cpu::load_byte_switch(u8& to, const u8& code) {
	switch (code) {
	case 0x00: {
		to = read_byte_increment_PC();
	} break;
	case 0x01: {
		to = memory[read_word_increment_PC()];
	} break;
	case 0x02: {
		to = memory[0xFF00 + read_byte_increment_PC()];
	} break;
	case 0x03: {
		to = memory[reg.HL++];
	} break;
	case 0x04: {
		to = memory[reg.HL--];
	} break;
	default: {
		throw("Illegal code! - fn() : load_byte_switch");
	} break;
	}
}


void Cpu::load_word(u16& to, const u16& /*unused*/) {
	to = read_word_increment_PC();
}


void Cpu::load_sp_hl() {
	reg.SP = reg.HL;
}


void Cpu::load_stack() {
	memory.write_word(read_word_increment_PC(), reg.SP);
}


void Cpu::load_hl(u16& to, const u16& from) {
	u8 immediate{ read_byte_increment_PC() };
	const bool sign{ (immediate & 0x80) == 0x80 };
	immediate &= 0x7F;

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
}


void Cpu::push(u16& to, const u16& from) {
	memory[--to] = static_cast<u8>(from >> 8);
	memory[--to] = static_cast<u8>(from);
}


void Cpu::pop(u16& to, const u16& /*unused*/) {
	to = memory[reg.SP++];
	to |= (memory[reg.SP++] << 8);
}


void Cpu::add_byte(u8& to, const u8& from) {
	set_status_register(
		0x0F,
		(((to + from) & 0xFF) == 0x00),
		CLEAR_BIT,
		((((to & 0x0F) + (from & 0x0F)) & 0x10) == 0x10),
		(((to + from) & 0x100) == 0x100));

	to += from;
}


void Cpu::add_byte_c(u8& to, const u8& from) {
	add_byte(to, (from + status_register.C));
}


void Cpu::sub_byte(u8& to, const u8& from) {
	set_status_register(
		0x0F,
		(to == from),
		SET_BIT,
		((((to & 0x0F) - (from & 0x0F)) & 0x10) == 0x10),
		(to < from));

	to -= from;
}


void Cpu::sub_byte_c(u8& to, const u8& from) {
	sub_byte(to, (from + status_register.C));
}


void Cpu::and_byte(u8& to, const u8& from) {
	set_status_register(
		0x0F,
		((to & from) == 0x00),
		CLEAR_BIT,
		SET_BIT,
		CLEAR_BIT);

	to &= from;
}


void Cpu::or_byte(u8& to, const u8& from) {
	set_status_register(
		0x0F,
		((to | from) == 0x00),
		CLEAR_BIT,
		CLEAR_BIT,
		CLEAR_BIT);

	to |= from;
}


void Cpu::xor_byte(u8& to, const u8& from) {
	set_status_register(
		0x0F,
		((to ^ from) == 0x00),
		CLEAR_BIT,
		CLEAR_BIT,
		CLEAR_BIT);

	to ^= from;
}


void Cpu::cp_byte(u8& to, const u8& from) {
	set_status_register(
		0x0F,
		(to == from),
		SET_BIT,
		((((to & 0x0F) - (from & 0x0F)) & 0x10) == 0x10),
		(to < from));
}


void Cpu::inc_byte(u8& to, const u8& /*unused*/) {
	set_status_register(
		0x0E,
		((to + 0x01) == 0x00),
		CLEAR_BIT,
		((((to & 0x0F) + 0x01) & 0x10) == 0x10),
		UNUSED);

	++to;
}


void Cpu::dec_byte(u8& to, const u8& /*unused*/) {
	set_status_register(
		0x0E,
		((to - 0x01) == 0x00),
		SET_BIT,
		((((to & 0x0F) - 0x01) & 0x10) == 0x10),
		UNUSED);

	--to;
}


void Cpu::add_word(u16& to, const u16& from) {
	set_status_register(
		0x07,
		UNUSED,
		CLEAR_BIT,
		((((to & 0x0FFF) + (from & 0x0FFF)) & 0x1000) == 0x1000),
		(((to + from) & 0x10000) == 0x10000));

	to += from;
}


void Cpu::add_sp(u16& to, const u16& /*unused*/) {
	u8 immediate{ read_byte_increment_PC() };
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
}


void Cpu::inc_word(u16& to, const u16& /*unused*/) {
	++to;
}


void Cpu::dec_word(u16& to, const u16& /*unused*/) {
	--to;
}


void Cpu::load_memory_inc() {
	memory[reg.HL++] = reg.A;
}


void Cpu::load_memory_dec() {
	memory[reg.HL--] = reg.A;
}


void Cpu::load_memory_imm() {
	memory[read_word_increment_PC()] = reg.A;
}


void Cpu::alu_byte_imm(u8& to, const u8& code) {
	const u8 immediate{ read_byte_increment_PC() };

	switch (code) {
	case 0x00: {
		add_byte(to, immediate);
	} break;
	case 0x01: {
		add_byte_c(to, immediate);
	} break;
	case 0x02: {
		sub_byte(to, immediate);
	} break;
	case 0x03: {
		sub_byte_c(to, immediate);
	} break;
	case 0x04: {
		and_byte(to, immediate);
	} break;
	case 0x05: {
		or_byte(to, immediate);
	} break;
	case 0x06: {
		xor_byte(to, immediate);
	} break;
	case 0x07: {
		cp_byte(to, immediate);
	} break;
	default: {
		throw("Illegal code! - fn() = alu_byte_imm");
	} break;
	}
}


void Cpu::daa() {
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
}


void Cpu::cpl() {
	set_status_register(
		0x06,
		UNUSED,
		SET_BIT,
		SET_BIT,
		UNUSED);

	reg.A = ~reg.A;
}


void Cpu::ccf() {
	set_status_register(
		0x07,
		UNUSED,
		CLEAR_BIT,
		CLEAR_BIT,
		~status_register.C);
}


void Cpu::scf() {
	set_status_register(
		0x07,
		UNUSED,
		CLEAR_BIT,
		CLEAR_BIT,
		SET_BIT);
}


void Cpu::nop() {}


void Cpu::swap_nibble(u8& to, const u8& /*unused*/) {
	const u8 new_high_nibble{ static_cast<u8>(to << 4) };
	to = static_cast<u8>(new_high_nibble | (to >> 4));

	set_status_register(
		0x0F,
		(to == 0),
		CLEAR_BIT,
		CLEAR_BIT,
		CLEAR_BIT);
}


void Cpu::rotate_byte(u8& to, const u8& code) {
	u8 new_carry{};

	switch (code) {
	case 0x00: { // Rotate left.
		new_carry = ((to & 0x80) == 0x80);
		to = (to << 1) | new_carry;
	} break;
	case 0x01: { // Rotate left through carry.
		new_carry = ((to & 0x80) == 0x80);
		to = (to << 1) | status_register.C;
	} break;
	case 0x02: { // Rotate right.
		new_carry = (to & 0x01);
		to = (to >> 1) | (new_carry << 7);
	} break;
	case 0x03: { // Rotate right through carry.
		new_carry = (to & 0x01);
		to = (to >> 1) | (status_register.C << 7);
	} break;
	default: {
		throw("Illegal code! - fn() = rotate_byte");
	} break;
	}

	set_status_register(
		0x0F,
		(to == 0x00),
		CLEAR_BIT,
		CLEAR_BIT,
		new_carry);
}


void Cpu::shift_byte(u8& to, const u8& code) {
	u8 new_carry{};

	switch (code) {
	case 0x00: { // Shift left to carry. LSB = 0;
		new_carry = ((to & 0x80) == 0x80);
		to = (to << 1);
	} break;
	case 0x01: { // Shift right to carry. MSB unaffected.
		new_carry = (to & 0x01);
		to = (to >> 1) | (to & 0x80);
	} break;
	case 0x02: { // Shift right to carry. MSB = 0.
		new_carry = (to & 0x01);
		to = (to >> 1);
	} break;
	default: {
		throw("Illegal code! - fn() = shift_byte");
	} break;
	}

	set_status_register(
		0x0F,
		(to == 0x00),
		CLEAR_BIT,
		CLEAR_BIT,
		new_carry);
}


void Cpu::test_bit(u8& to, const u8& bit) {
	set_status_register(
		0x0E,
		((to & (0x01 << bit)) == 0x00),
		CLEAR_BIT,
		SET_BIT,
		UNUSED);
}


void Cpu::set_bit(u8& to, const u8& bit) {
	to |= (0x01 << bit);
}


void Cpu::reset_bit(u8& to, const u8& bit) {
	to &= ~(0x01 << bit);
}


void Cpu::jump() {
	reg.PC = read_word_increment_PC();
}


void Cpu::jump_hl() {
	reg.PC = memory.read_word(reg.HL);
}

void Cpu::jump_relative() {
	u8 immediate{ read_byte_increment_PC() };
	const bool sign{ (immediate & 0x80) == 0x80 };
	immediate &= 0x7F;

	if (sign) { // Subtract immediate.
		reg.PC -= static_cast<u16>(immediate);
	}
	else { // Add immediate.
		reg.PC += static_cast<u16>(immediate);
	}
}


void Cpu::jump_cp(u16& to, const u16& code) {
	const u16 address{ read_word_increment_PC() };
	auto help_fn = [this, &to, &address](const u8& flag, const u8& val) {
		if (flag == val) {
			to = address;
			clock_cyles += 4;
		}
	};

	switch (code) {
	case 0x00: {
		help_fn(status_register.Z, 0);
	} break;
	case 0x01: {
		help_fn(status_register.Z, 1);
	} break;
	case 0x02: {
		help_fn(status_register.C, 0);
	} break;
	case 0x03: {
		help_fn(status_register.C, 1);
	} break;
	default: {
		throw("Illegal code! - fn() = jump_cp");
	} break;
	}
}


void Cpu::jump_relative_cp(u16& to, const u16& code) {
	auto help_fn = [this, &to](const u8& flag, const u8& val) {
		if (flag == val) {
			jump_relative();
			clock_cyles += 4;
		}
		else {
			to += 1;
		}
	};

	switch (code) {
	case 0x00: {
		help_fn(status_register.Z, 0);
	} break;
	case 0x01: {
		help_fn(status_register.Z, 1);
	} break;
	case 0x02: {
		help_fn(status_register.C, 0);
	} break;
	case 0x03: {
		help_fn(status_register.C, 1);
	} break;
	default: {
		throw("Illegal code! - fn() = jump_relative_cp");
	} break;
	}
}


void Cpu::call() {
	push(reg.SP, (reg.PC + 3));
	reg.PC = read_word_increment_PC();
}


void Cpu::call_cp(u16& to, const u16& code) {
	auto help_fn = [this, &to](const u8& flag, const u8& val) {
		if (flag == val) {
			call();
			clock_cyles += 12;
		}
		else {
			to += 2;
		}
	};

	switch (code) {
	case 0x00: {
		help_fn(status_register.Z, 0);
	} break;
	case 0x01: {
		help_fn(status_register.Z, 1);
	} break;
	case 0x02: {
		help_fn(status_register.C, 0);
	} break;
	case 0x03: {
		help_fn(status_register.C, 1);
	} break;
	default: {
		throw("Illegal code! - fn() = call_cp");
	} break;
	}
}


void Cpu::ret() {
	pop(reg.PC, static_cast<u16>(UNUSED));
}


void Cpu::ret_cp(u16& to, const u16& code) {
	auto help_fn = [this, &to](const u8& flag, const u8& val) {
		if (flag == val) {
			pop(to, static_cast<u16>(UNUSED));
			clock_cyles += 12;
		}
	};

	switch (code) {
	case 0x00: {
		help_fn(status_register.Z, 0);
	} break;
	case 0x01: {
		help_fn(status_register.Z, 1);
	} break;
	case 0x02: {
		help_fn(status_register.C, 0);
	} break;
	case 0x03: {
		help_fn(status_register.C, 1);
	} break;
	default: {
		throw("Illegal code! - fn() = ret_cp");
	} break;
	}
}


void Cpu::reti() {
	ret();
	interrupt = true;
}


void Cpu::restart(u16& to, const u16& code) {
	const u16 offset{ static_cast<u16>(0x0000 + (code * 0x0008)) };
	push(reg.SP, to);
	to = offset;
}

void Cpu::load_imm_off() {
	memory[0xFF00 + read_byte_increment_PC()] = reg.A;
}


void Cpu::halt_cpu() {
	// implement!
}

void Cpu::stop_cpu() {
	const u8 op{ read_byte_increment_PC() };
	if (op != 0x00) {
		throw("Illegal opcode! fn() = stop_cpu");
	}

	// implement!
}

void Cpu::di() {
	// implement!
}

void Cpu::ei() {
	// implement!
}