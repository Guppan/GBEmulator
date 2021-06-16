#include "Cpu.h"
#include "Bus.h"
#include "Ppu.h"

#include <fstream>
#include <map>

#include <iostream>
#include <cstdio>

// ------ Constants ------------------------------------------
// Name of the text file containing cycles for instructions.
const std::string CYCLE_FILE{ "cycles.txt" };

// Used primarily in method 'set_flags' to indicate
// that an argument is unused.
const bool UNUSED{ false };

// Used to indicate wheter a bit is set or cleared.
const bool SET_BIT{ true };
const bool CLEAR_BIT{ false };
// -----------------------------------------------------------


Cpu::Cpu() :
	ppu{ nullptr },
	cpu_reg{},
	cycle_info{},
	cycle_counter{}
{
	reset_device();
}


void Cpu::connect_to_ppu(Ppu* main_ppu) {
	ppu = main_ppu;
}


unsigned Cpu::fetch_execute() {
	//printf("PC : %04X    | ", reg.PC);
	u8 opcode = read_byte_inc_PC();
	u16 instruction_cycle = 0x0000;
	//printf("PC : %04X    | Opcode : %02X   | ", reg.PC, opcode);
	// Execute instruction.
	if (opcode != 0xCB) {
		// Execute a regular instruction.
		instruction_cycle = opcode;
		regular_op(opcode);
	}
	else {
		// Execute an extended instruction.
		opcode = read_byte_inc_PC();
		instruction_cycle = 0x0100 + opcode;
		extended_op(opcode);
	}
	
	// Calculate total instruction time.
	unsigned instruction_time = cycle_info.cycle_matrix[instruction_cycle];
	if (cycle_info.is_extended_cycle) {
		instruction_time += cycle_info.extended_cycle;
		cycle_info.is_extended_cycle = false;
	}
	/*
	printf("PC : %04X\n", reg.PC);
	printf("AF= %04X   LCDC=%04X\n", reg.AF, read_byte(0xFF40));
	printf("BC= %04X   STAT=%04X\n", reg.BC, read_byte(0xFF41));
	printf("DE= %04X   LY=  %04X\n", reg.DE, read_byte(0xFF44));
	printf("HL= %04X   SP=  %04X\n", reg.HL, reg.SP);
	*/
	return instruction_time;
}


void Cpu::tick_cpu(unsigned instruction_time) {
	cycle_counter += instruction_time;
	
	// Emulate all the cycles the instruction takes.
	while (instruction_time > 0) {
		// Tick ppu
		--instruction_time;
	}
}


void Cpu::handle_interrupt() {
	if (io_reg.IRQ == 0x00 || io_reg.IE == 0x00 || !cpu_reg.IME) {
		return;
	}

	u8 active_interrupts = (io_reg.IE & io_reg.IRQ) & 0x1F;
	u8 selected_interrupt = 0x00;

	while ((active_interrupts & 0x01) == 0x00) {
		active_interrupts = (active_interrupts >> 1);
		++selected_interrupt;
	}

	// Save program counter and jump to interrupt vector.
	push(cpu_reg.PC);
	cpu_reg.PC = 0x0040 + selected_interrupt * 0x08;
	tick_cpu(20);

	// Reset the requested interrupt and disable interrupts.
	//0xFF0F &= ~(1 << selected_interrupt);
	io_reg.IRQ &= ~(1 << selected_interrupt);
	cpu_reg.IME = false;
}


void Cpu::run_cpu() {
	// kolla om det är något interrupt som väntar.
	// annars kör tick_cpu();

	// Check if there is a pending interrupt.
	handle_interrupt();

	// Fetch and execute the next instruction.
	tick_cpu(fetch_execute());
}


void Cpu::reset_device() {
	// Reset registers
	cpu_reg.AF = 0x01B0;
	cpu_reg.BC = 0x0013;
	cpu_reg.DE = 0x00D8;
	cpu_reg.HL = 0x014D;
	cpu_reg.SP = 0xFFFE;
	cpu_reg.PC = 0x0100;
	cpu_reg.IME = false;
	cpu_reg.IME_DELAY = false;

	// Reset cycle information
	cycle_info.is_extended_cycle = false;
	cycle_info.extended_cycle = 0;

	io_reg.IRQ = 0x00;
	io_reg.IE = 0x00;
}


u8 Cpu::read_byte(const u16 address) const {
	u8 data = 0x00;

	if (address < 0xFE00) {
		data = bus->read_byte(address);
	}
	else if (address < 0xFEA0) {
		// OAM in ppu
	}
	else {
		data = cpu_ram[address - 0xFEA0];
	}

	return data;
}


void Cpu::write_byte(const u16 address, const u8 data) {
	if (address < 0xFE00) {
		bus->write_byte(address, data);
	}
	else if (address < 0xFEA0) {
		// write to OAM in ppu
	}
	else {
		cpu_ram[address - 0xFEA0] = data;
	}
}


u8 Cpu::read_io(const u16 address) const {
	switch (address) {
	case 0xFF0F: { return io_reg.IRQ; } break;
	case 0xFFFF: {return io_reg.IE; } break;
	}
}


void Cpu::write_io(const u16 address, const u8 data) {
	switch (address) {
	case 0xFF0F: {io_reg.IRQ = data; } break;
	case 0xFFFF: {io_reg.IE = data; } break;
	}
}



void Cpu::init_cycle_info() {
	std::ifstream file{ CYCLE_FILE , std::ios::in};

	for (u16 idx = 0x0000; idx < 0x0200; ++idx) {
		file >> cycle_info.cycle_matrix[idx];
	}

	cycle_info.is_extended_cycle = false;
	cycle_info.extended_cycle = 0;
}


// Sets the status register according to the 'active_mask' and bit-values.
// For example: set_flags(0x0F, 1 1 0 1) would set Z, N and C to 1
// and clear H.
void Cpu::set_flags(const u8& active_mask, bool _Z, bool _N, bool _H, bool _C) {
	u8 set_bits = (_Z << 3) + (_N << 2) + (_H << 1) + _C;
	cpu_reg.F |= (active_mask & set_bits);	// set bits to 1.
	cpu_reg.F &= ~(active_mask ^ set_bits);	// clear bits.
}


u8 Cpu::read_byte_inc_PC() {
	return read_byte(cpu_reg.PC++);
}


u16 Cpu::read_word_inc_PC() {
	u16 word = read_byte(cpu_reg.PC++);
	word |= (read_byte(cpu_reg.PC++) << 8);
	return word;
}



void Cpu::regular_op(const u8 opcode) {
	static const std::map<u8, u8&> reg_map{
		{0x07, cpu_reg.A}, {0x00, cpu_reg.B}, {0x01, cpu_reg.C}, {0x02, cpu_reg.D},
		{0x03, cpu_reg.E}, {0x04, cpu_reg.H}, {0x05, cpu_reg.L}
	};

	const u8 code = ((opcode & 0xC0) >> 6);
	const u8 left_opr = ((opcode & 0x38) >> 3);
	const u8 right_opr = (opcode & 0x07);

	switch (code) {
	case 0x00: {
		switch (right_opr) {
		case 0x00: {
			switch (left_opr) {
			case 0x00: { /*NOP*/ } break;					// nop
			case 0x01: { store_sp(); } break;				// ld (nn), sp
			case 0x02: { stop_cpu(); } break;
			case 0x03: { jump_relative(); } break;			// jr e
			case 0x04: { jump_relative_cp(0x00); } break;	// jr cc, e
			case 0x05: { jump_relative_cp(0x01); } break;	// jr cc, e
			case 0x06: { jump_relative_cp(0x02); } break;	// jr cc, e
			case 0x07: { jump_relative_cp(0x03); } break;	// jr cc, e
			}
		} break;
		case 0x01: {
			switch (left_opr) {
			case 0x00: { cpu_reg.BC = read_word_inc_PC(); } break;	// ld bc, nn
			case 0x01: { add_word(cpu_reg.BC); } break;
			case 0x02: { cpu_reg.DE = read_word_inc_PC(); } break;	// ld de, nn
			case 0x03: { add_word(cpu_reg.DE); } break;
			case 0x04: { cpu_reg.HL = read_word_inc_PC(); } break;	// ld hl, nn
			case 0x05: { add_word(cpu_reg.HL); } break;
			case 0x06: { cpu_reg.SP = read_word_inc_PC(); } break;	// ld sp, nn
			case 0x07: { add_word(cpu_reg.SP); } break;
			}
		} break;
		case 0x02: {
			switch (left_opr) {
			case 0x00: { write_byte(cpu_reg.BC, cpu_reg.A); } break;
			case 0x01: { cpu_reg.A = read_byte(cpu_reg.BC); } break;
			case 0x02: { write_byte(cpu_reg.DE, cpu_reg.A); } break;
			case 0x03: { cpu_reg.A = read_byte(cpu_reg.DE); } break;
			case 0x04: { write_byte(cpu_reg.HL++, cpu_reg.A); } break;
			case 0x05: { cpu_reg.A = read_byte(cpu_reg.HL++); } break;
			case 0x06: { write_byte(cpu_reg.HL--, cpu_reg.A); } break;
			case 0x07: { cpu_reg.A = read_byte(cpu_reg.HL--); } break;
			}
		} break;
		case 0x03: {
			switch (left_opr) {
			case 0x00: { ++cpu_reg.BC; } break;
			case 0x01: { --cpu_reg.BC; } break;
			case 0x02: { ++cpu_reg.DE; } break;
			case 0x03: { --cpu_reg.DE; } break;
			case 0x04: { ++cpu_reg.HL; } break;
			case 0x05: { --cpu_reg.HL; } break;
			case 0x06: { ++cpu_reg.SP; } break;
			case 0x07: { --cpu_reg.SP; } break;
			}
		} break;
		case 0x04: {
			switch (left_opr) {
			case 0x06: { write_byte(cpu_reg.HL, inc(read_byte(cpu_reg.HL))); } break;
			default: { reg_map.at(left_opr) = inc(reg_map.at(left_opr)); } break;					// inc r
			}
		} break;
		case 0x05: {
			switch (left_opr) {
			case 0x06: { write_byte(cpu_reg.HL, dec(read_byte(cpu_reg.HL))); } break;
			default: { reg_map.at(left_opr) = dec(reg_map.at(left_opr)); } break;					// dec r
			}
		} break;
		case 0x06: {
			switch (left_opr) {
			case 0x06: { write_byte(cpu_reg.HL, read_byte_inc_PC()); } break;
			default: { reg_map.at(left_opr) = read_byte_inc_PC(); } break;
			}
		} break;
		case 0x07: {
			switch (left_opr) {
			case 0x00: { cpu_reg.A = rotate(cpu_reg.A, 0x00); } break;	// rlc r
			case 0x01: { cpu_reg.A = rotate(cpu_reg.A, 0x02); } break;	// rrc r
			case 0x02: { cpu_reg.A = rotate(cpu_reg.A, 0x01); } break;	// rl  r
			case 0x03: { cpu_reg.A = rotate(cpu_reg.A, 0x03); } break;	// rr  r
			case 0x04: { daa(); } break;
			case 0x05: { cpl(); } break;
			case 0x06: { scf(); } break;
			case 0x07: { ccf(); } break;
			}
		} break;
		}
	} break;

	case 0x01: {
		switch (right_opr) {
		case 0x06: {
			switch (left_opr) {
			case 0x06: { halt_cpu(); } break;
			default: {reg_map.at(left_opr) = read_byte(cpu_reg.HL); } break;
			}
		} break;
		default: {
			switch (left_opr) {
			case 0x06: {write_byte(cpu_reg.HL, reg_map.at(right_opr)); } break;
			default: {reg_map.at(left_opr) = reg_map.at(right_opr); } break;
			}
		} break;
		}
	} break;

	case 0x02: {
		switch (right_opr) {
		case 0x06: {
			switch (left_opr) {
			case 0x00: { add_byte(read_byte(cpu_reg.HL)); } break;					// add a, (hl)
			case 0x01: { add_byte(read_byte(cpu_reg.HL) + cpu_reg.flag.C); } break;		// adc a, (hl)
			case 0x02: { sub_byte(read_byte(cpu_reg.HL)); } break;					// sub a, (hl)
			case 0x03: { sub_byte(read_byte(cpu_reg.HL) + cpu_reg.flag.C); } break;		// sbc a, (hl)
			case 0x04: { and_byte(read_byte(cpu_reg.HL)); } break;					// and a, (hl)
			case 0x05: { xor_byte(read_byte(cpu_reg.HL)); } break;					// xor a, (hl)
			case 0x06: { or_byte(read_byte(cpu_reg.HL)); } break;					// or  a, (hl)
			case 0x07: { cp_byte(read_byte(cpu_reg.HL)); } break;					// cp  a, (hl)
			}
		} break;
		default: {
			switch (left_opr) {
			case 0x00: { add_byte(reg_map.at(right_opr)); } break;				// add a, r
			case 0x01: { add_byte(reg_map.at(right_opr) + cpu_reg.flag.C); } break;	// adc a, r
			case 0x02: { sub_byte(reg_map.at(right_opr)); } break;				// sub a, r
			case 0x03: { sub_byte(reg_map.at(right_opr) + cpu_reg.flag.C); } break;	// sbc a, r
			case 0x04: { and_byte(reg_map.at(right_opr)); } break;				// and a, r
			case 0x05: { xor_byte(reg_map.at(right_opr)); } break;				// xor a, r
			case 0x06: { or_byte(reg_map.at(right_opr)); } break;				// or  a, r
			case 0x07: { cp_byte(reg_map.at(right_opr)); } break;				// cp  a, r
			}
		} break;
		}
	} break;

	case 0x03: {
		switch (right_opr) {
		case 0x00: {
			switch (left_opr) {
			case 0x00: { ret_cp(0x00); } break;	// ret cc 
			case 0x01: { ret_cp(0x01); } break;	// ret cc
			case 0x02: { ret_cp(0x02); } break;	// ret cc
			case 0x03: { ret_cp(0x03); } break;	// ret cc
			case 0x04: { write_byte(0xFF00 + read_byte_inc_PC(), cpu_reg.A); } break;
			case 0x05: { add_sp(); } break;		// add sp, e
			case 0x06: { cpu_reg.A = read_byte(0xFF00 + read_byte_inc_PC()); } break;
			case 0x07: { load_hl(); } break;	// ldhl sp, e
			}
		} break;
		case 0x01: {
			switch (left_opr) {
			case 0x00: { pop(cpu_reg.BC); } break;	// pop bc
			case 0x01: { pop(cpu_reg.PC); } break;	// ret
			case 0x02: { pop(cpu_reg.DE); } break;
			case 0x03: { reti(); } break;	// reti
			case 0x04: { pop(cpu_reg.HL); } break;
			case 0x05: { cpu_reg.PC = cpu_reg.HL; } break;	// jp hl
			case 0x06: { pop(cpu_reg.AF); } break;
			case 0x07: { cpu_reg.SP = cpu_reg.HL; } break;	// ld sp, hl
			}
		} break;
		case 0x02: {
			switch (left_opr) {
			case 0x00: { jump_cp(0x00); } break;	// jp cc, nn
			case 0x01: { jump_cp(0x01); } break;	// jp cc, nn
			case 0x02: { jump_cp(0x02); } break;	// jp cc, nn
			case 0x03: { jump_cp(0x03); } break;	// jp cc, nn
			case 0x04: { write_byte(0xFF00 + cpu_reg.C, cpu_reg.A); } break;
			case 0x05: { cpu_reg.A = read_byte(read_word_inc_PC()); } break;
			case 0x06: { cpu_reg.A = read_byte(0xFF00 + cpu_reg.C); } break;
			case 0x07: { write_byte(read_word_inc_PC(), cpu_reg.A); } break;
			}
		} break;
		case 0x03: {
			switch (left_opr) {
			case 0x00: { cpu_reg.PC = read_word_inc_PC(); } break;	// jp nn
			case 0x06: { cpu_reg.IME = false; } break;
			case 0x07: { cpu_reg.IME = true; cpu_reg.IME_DELAY = true; } break;
			}
		} break;
		case 0x04: {
			switch (left_opr) {
			case 0x00: { call_cp(0x00); } break;	// call cc, nn
			case 0x01: { call_cp(0x01); } break;	// call cc, nn
			case 0x02: { call_cp(0x02); } break;	// call cc, nn
			case 0x03: { call_cp(0x03); } break;	// call cc, nn
			}
		} break;
		case 0x05: {
			switch (left_opr) {
			case 0x00: { push(cpu_reg.BC); } break;	// push bc
			case 0x01: { call(); } break;	// call nn
			case 0x02: { push(cpu_reg.DE); } break;	// push de
			case 0x04: { push(cpu_reg.HL); } break;	// push hl
			case 0x06: { push(cpu_reg.AF); } break;	// push af
			}
		} break;
		case 0x06: {
			switch (left_opr) {
			case 0x00: { add_byte(read_byte_inc_PC()); } break;					// add a, n
			case 0x01: { add_byte(read_byte_inc_PC() + cpu_reg.flag.C); } break;	// adc a, n
			case 0x02: { sub_byte(read_byte_inc_PC()); } break;					// sub a, n
			case 0x03: { sub_byte(read_byte_inc_PC() + cpu_reg.flag.C); } break;	// sbc a, n
			case 0x04: { and_byte(read_byte_inc_PC()); } break;					// and a, n
			case 0x05: { xor_byte(read_byte_inc_PC()); } break;					// xor a, n
			case 0x06: { or_byte(read_byte_inc_PC()); } break;					// or  a, n
			case 0x07: { cp_byte(read_byte_inc_PC()); } break;					// cp  a, n
			}
		} break;
		case 0x07: {
			restart(left_opr);
		} break;
		}
	} break;
	}
}


void Cpu::extended_op(const u8 opcode) {
	static const std::map<u8, u8&> reg_map{
	{0x07, cpu_reg.A}, {0x00, cpu_reg.B}, {0x01, cpu_reg.C}, {0x02, cpu_reg.D},
	{0x03, cpu_reg.E}, {0x04, cpu_reg.H}, {0x05, cpu_reg.L}
	};

	const u8 code = ((opcode & 0xC0) >> 6);
	const u8 left_opr = ((opcode & 0x38) >> 3);
	const u8 right_opr = (opcode & 0x07);

	switch (code) {
	case 0x00: {
		switch (right_opr) {
		case 0x06: {
			switch (left_opr) {
			case 0x00: { write_byte(cpu_reg.HL, rotate(read_byte(cpu_reg.HL), 0x00)); } break;	// rlc (hl)
			case 0x01: { write_byte(cpu_reg.HL, rotate(read_byte(cpu_reg.HL), 0x02)); } break;	// rrc (hl)
			case 0x02: { write_byte(cpu_reg.HL, rotate(read_byte(cpu_reg.HL), 0x01)); } break;	// rl  (hl)
			case 0x03: { write_byte(cpu_reg.HL, rotate(read_byte(cpu_reg.HL), 0x03)); } break;	// rr  (hl)
			case 0x04: { write_byte(cpu_reg.HL, shift(read_byte(cpu_reg.HL), 0x00)); } break;	// sla (hl)
			case 0x05: { write_byte(cpu_reg.HL, shift(read_byte(cpu_reg.HL), 0x01)); } break;	// sra (hl)
			case 0x06: { write_byte(cpu_reg.HL, swap(read_byte(cpu_reg.HL))); } break;			// swap (hl)
			case 0x07: { write_byte(cpu_reg.HL, shift(read_byte(cpu_reg.HL), 0x01)); } break;	// srl (hl)
			}
		} break;
		default: {
			switch (left_opr) {
			case 0x00: { reg_map.at(right_opr) = rotate(reg_map.at(right_opr), 0x00); } break;	// rlc r
			case 0x01: { reg_map.at(right_opr) = rotate(reg_map.at(right_opr), 0x02); } break;	// rrc r
			case 0x02: { reg_map.at(right_opr) = rotate(reg_map.at(right_opr), 0x01); } break;	// rl  r
			case 0x03: { reg_map.at(right_opr) = rotate(reg_map.at(right_opr), 0x03); } break;	// rr  r
			case 0x04: { reg_map.at(right_opr) = shift(reg_map.at(right_opr), 0x00); } break;	// sla r
			case 0x05: { reg_map.at(right_opr) = shift(reg_map.at(right_opr), 0x01); } break;	// sra r
			case 0x06: { reg_map.at(right_opr) = swap(reg_map.at(right_opr)); } break;			// swap r
			case 0x07: { reg_map.at(right_opr) = shift(reg_map.at(right_opr), 0x02); } break;	// srl r
			}
		} break;
		}
	} break;

	case 0x01: {
		switch (right_opr) {
		case 0x06: { bit(read_byte(cpu_reg.HL), left_opr); } break;
		default: { bit(reg_map.at(right_opr), left_opr); } break;
		}
	} break;

	case 0x02: {
		switch (right_opr) {
		case 0x06: { write_byte(cpu_reg.HL, res(read_byte(cpu_reg.HL), left_opr)); } break;
		default: { reg_map.at(right_opr) = res(reg_map.at(right_opr), left_opr); } break;
		}
	} break;

	case 0x03: {
		switch (right_opr) {
		case 0x06: { write_byte(cpu_reg.HL, set(read_byte(cpu_reg.HL), left_opr)); } break;
		default: { reg_map.at(right_opr) = set(reg_map.at(right_opr), left_opr); } break;
		}
	} break;
	}
}


/* ------------ 8bit argument methods ------------
*  -----------------------------------------------
*  These methods take one or two 8bit arguments.
* 
*/


void Cpu::add_byte(const u8 value) {
	set_flags(
		0x0F,
		(((cpu_reg.A + value) & 0xFF) == 0x00),
		CLEAR_BIT,
		((((cpu_reg.A & 0x0F) + (value & 0x0F)) & 0x10) == 0x10),
		(((cpu_reg.A + value) & 0x100) == 0x100));

	cpu_reg.A += value;
}


void Cpu::sub_byte(const u8 value) {
	set_flags(
		0x0F,
		(cpu_reg.A == value),
		SET_BIT,
		((((cpu_reg.A & 0x0F) - (value & 0x0F)) & 0x10) == 0x10),
		(cpu_reg.A < value));

	cpu_reg.A -= value;
}


void Cpu::and_byte(const u8 value) {
	set_flags(
		0x0F,
		((cpu_reg.A & value) == 0x00),
		CLEAR_BIT,
		SET_BIT,
		CLEAR_BIT);

	cpu_reg.A &= value;
}


void Cpu::or_byte(const u8 value) {
	set_flags(
		0x0F,
		((cpu_reg.A | value) == 0x00),
		CLEAR_BIT,
		CLEAR_BIT,
		CLEAR_BIT);

	cpu_reg.A |= value;
}


void Cpu::xor_byte(const u8 value) {
	set_flags(
		0x0F,
		((cpu_reg.A ^ value) == 0x00),
		CLEAR_BIT,
		CLEAR_BIT,
		CLEAR_BIT);

	cpu_reg.A ^= value;
}


void Cpu::cp_byte(const u8 value) {
	set_flags(
		0x0F,
		(cpu_reg.A == value),
		SET_BIT,
		((((cpu_reg.A & 0x0F) - (value & 0x0F)) & 0x10) == 0x10),
		(cpu_reg.A < value));
}


u8 Cpu::inc(const u8 value) {
	set_flags(
		0x0E,
		((value + 0x01) == 0x00),
		CLEAR_BIT,
		((((value & 0x0F) + 0x01) & 0x10) == 0x10),
		UNUSED);

	return value + 1;
}


u8 Cpu::dec(const u8 value) {
	set_flags(
		0x0E,
		((value - 0x01) == 0x00),
		SET_BIT,
		((((value & 0x0F) - 0x01) & 0x10) == 0x10),
		UNUSED);

	return value - 1;
}


u8 Cpu::swap(const u8 value) {
	u8 result = (value << 4);
	result |= (value >> 4);

	set_flags(
		0x0F,
		(result == 0),
		CLEAR_BIT,
		CLEAR_BIT,
		CLEAR_BIT);

	return result;
}


u8 Cpu::rotate(const u8 value, const u8 code) {
	u8 result = 0x00;
	u8 new_carry = 0x00;

	switch (code) {
	case 0x00: { // Rotate left.
		new_carry = ((value & 0x80) == 0x80);
		result = (value << 1) | new_carry;
	} break;
	case 0x01: { // Rotate left through carry.
		new_carry = ((value & 0x80) == 0x80);
		result = (value << 1) | cpu_reg.flag.C;
	} break;
	case 0x02: { // Rotate right.
		new_carry = (value & 0x01);
		result = (value >> 1) | (new_carry << 7);
	} break;
	case 0x03: { // Rotate right through carry.
		new_carry = (value & 0x01);
		result = (value >> 1) | (cpu_reg.flag.C << 7);
	} break;
	}

	set_flags(
		0x0F,
		(result == 0x00),
		CLEAR_BIT,
		CLEAR_BIT,
		new_carry);

	return result;
}


u8 Cpu::shift(const u8 value, const u8 code) {
	u8 result = 0x00;
	u8 new_carry = 0x00;

	switch (code) {
	case 0x00: { // Shift left to carry. LSB = 0;
		new_carry = ((value & 0x80) == 0x80);
		result = (value << 1);
	} break;
	case 0x01: { // Shift right to carry. MSB unaffected.
		new_carry = (value & 0x01);
		result = (value >> 1) | (value & 0x80);
	} break;
	case 0x02: { // Shift right to carry. MSB = 0.
		new_carry = (value & 0x01);
		result = (value >> 1);
	} break;
	}

	set_flags(
		0x0F,
		(result == 0x00),
		CLEAR_BIT,
		CLEAR_BIT,
		new_carry);

	return result;
}


void Cpu::bit(const u8 param, const u8 bit) {
	set_flags(
		0x0E,
		((param & (0x01 << bit)) == 0x00),
		CLEAR_BIT,
		SET_BIT,
		UNUSED);
}


u8 Cpu::set(const u8 param, const u8 bit) {
	return (param | (0x01 << bit));
}


u8 Cpu::res(const u8 param, const u8 bit) {
	return (param & ~(0x01 << bit));
}



/* ------------ 16bit argument methods ------------
*  ------------------------------------------------
*  These methods take one or two 16bit arguments.
*
*/


// KAN BEHÖVA VARA 2s-complement!
void Cpu::load_hl() {
	u8 immediate = read_byte_inc_PC();
	const bool sign = ((immediate & 0x80) == 0x80);
	immediate &= 0x7F;

	bool _H, _C;
	if (sign) { // immediate is signed so we must subtract.
		_H = ((((cpu_reg.SP & 0x0F) - (immediate & 0x0F)) & 0x10) == 0x10);
		_C = (cpu_reg.SP < immediate);
		cpu_reg.HL = (cpu_reg.SP - immediate);
	}
	else { // immediate is positive so we must add.
		_H = ((((cpu_reg.SP & 0x0F) + (immediate & 0x0F)) & 0x10) == 0x10);
		_C = (((cpu_reg.SP + immediate) & 0x100) == 0x100);
		cpu_reg.HL = (cpu_reg.SP + immediate);
	}

	set_flags(
		0x0F,
		CLEAR_BIT,
		_H,
		CLEAR_BIT,
		_C);
}


void Cpu::add_word(const u16 param) {
	set_flags(
		0x07,
		UNUSED,
		CLEAR_BIT,
		((((cpu_reg.HL & 0x0FFF) + (param & 0x0FFF)) & 0x1000) == 0x1000),
		(((cpu_reg.HL + param) & 0x10000) == 0x10000));

	cpu_reg.HL += param;
}


// KAN BEHÖVA VARA 2s-complement!
void Cpu::add_sp() {
	u8 immediate = read_byte_inc_PC();
	const bool sign = ((immediate & 0x80) == 0x80);
	immediate &= 0x7F;

	bool _H, _C;
	if (sign) {
		_H = ((((cpu_reg.SP & 0x0FFF) - immediate) & 0x1000) == 0x1000);
		_C = (cpu_reg.SP < immediate);
		cpu_reg.SP -= immediate;
	}
	else {
		_H = ((((cpu_reg.SP & 0x0FFF) + immediate) & 0x1000) == 0x1000);
		_C = (((cpu_reg.SP + immediate) & 0x10000) == 0x10000);
		cpu_reg.SP += immediate;
	}

	set_flags(
		0x0F,
		CLEAR_BIT,
		CLEAR_BIT,
		_H,
		_C);
}


void Cpu::push(const u16 param) {
	write_byte(--cpu_reg.SP, static_cast<u8>(param >> 8));
	write_byte(--cpu_reg.SP, static_cast<u8>(param));
}


void Cpu::pop(u16& param) {
	param = read_byte(cpu_reg.SP++);
	param |= (read_byte(cpu_reg.SP++) << 8);
}


void Cpu::jump_cp(const u8 code) {
	const u16 address = read_word_inc_PC();
	auto help_fn = [this, &address](const u8& flag, const u8& val) {
		if (flag == val) {
			cpu_reg.PC = address;
			cycle_info.is_extended_cycle = true;
			cycle_info.extended_cycle = 4;
		}
	};

	switch (code) {
	case 0x00: {
		help_fn(cpu_reg.flag.Z, 0);
	} break;
	case 0x01: {
		help_fn(cpu_reg.flag.Z, 1);
	} break;
	case 0x02: {
		help_fn(cpu_reg.flag.C, 0);
	} break;
	case 0x03: {
		help_fn(cpu_reg.flag.C, 1);
	} break;
	}
}


void Cpu::jump_relative_cp(const u8 code) {
	auto help_fn = [this](const u8& flag, const u8& val) {
		if (flag == val) {
			jump_relative();
			cycle_info.is_extended_cycle = true;
			cycle_info.extended_cycle = 4;
		}
		else {
			cpu_reg.PC += 1;
		}
	};

	switch (code) {
	case 0x00: {
		help_fn(cpu_reg.flag.Z, 0);
	} break;
	case 0x01: {
		help_fn(cpu_reg.flag.Z, 1);
	} break;
	case 0x02: {
		help_fn(cpu_reg.flag.C, 0);
	} break;
	case 0x03: {
		help_fn(cpu_reg.flag.C, 1);
	} break;
	}
}


void Cpu::call_cp(const u8 code) {
	auto help_fn = [this](const u8& flag, const u8& val) {
		if (flag == val) {
			call();
			cycle_info.is_extended_cycle = true;
			cycle_info.extended_cycle = 12;
		}
		else {
			cpu_reg.PC += 2;
		}
	};

	switch (code) {
	case 0x00: {
		help_fn(cpu_reg.flag.Z, 0);
	} break;
	case 0x01: {
		help_fn(cpu_reg.flag.Z, 1);
	} break;
	case 0x02: {
		help_fn(cpu_reg.flag.C, 0);
	} break;
	case 0x03: {
		help_fn(cpu_reg.flag.C, 1);
	} break;
	}
}


void Cpu::ret_cp(const u8 code) {
	auto help_fn = [this](const u8& flag, const u8& val) {
		if (flag == val) {
			pop(cpu_reg.PC);
			cycle_info.is_extended_cycle = true;
			cycle_info.extended_cycle = 12;
		}
	};

	switch (code) {
	case 0x00: {
		help_fn(cpu_reg.flag.Z, 0);
	} break;
	case 0x01: {
		help_fn(cpu_reg.flag.Z, 1);
	} break;
	case 0x02: {
		help_fn(cpu_reg.flag.C, 0);
	} break;
	case 0x03: {
		help_fn(cpu_reg.flag.C, 1);
	} break;
	}
}


void Cpu::restart(const u8 code) {
	const u16 offset = static_cast<u16>(0x0000 + (code * 0x0008));
	push(cpu_reg.PC);
	cpu_reg.PC = offset;
}


/* ------------ no argument methods ------------
*  ---------------------------------------------
*  These methods takes no arguments.
*
*/



void Cpu::store_sp() {
	const u16 address = read_word_inc_PC();
	write_byte(address, static_cast<u8>(cpu_reg.SP));
	write_byte(address + 1, static_cast<u8>(cpu_reg.SP >> 8));
}









void Cpu::daa() {
	if (!cpu_reg.flag.N) { // Was the last operation an addition?
		if (cpu_reg.flag.C || (cpu_reg.A > 0x99)) {
			cpu_reg.A += 0x60;
			cpu_reg.flag.C = 0x01;
		}
		if (cpu_reg.flag.H || ((cpu_reg.A & 0x0F) > 0x09)) {
			cpu_reg.A += 0x06;
		}
	}
	else { // Was the last operation a subtraction?
		if (cpu_reg.flag.C) {
			cpu_reg.A -= 0x60;
		}
		if (cpu_reg.flag.H) {
			cpu_reg.A -= 0x06;
		}
	}

	cpu_reg.flag.Z = (cpu_reg.A == 0x00);
	cpu_reg.flag.H = 0;
}


void Cpu::cpl() {
	set_flags(
		0x06,
		UNUSED,
		SET_BIT,
		SET_BIT,
		UNUSED);

	cpu_reg.A = ~cpu_reg.A;
}


void Cpu::ccf() {
	set_flags(
		0x07,
		UNUSED,
		CLEAR_BIT,
		CLEAR_BIT,
		~cpu_reg.flag.C);
}


void Cpu::scf() {
	set_flags(
		0x07,
		UNUSED,
		CLEAR_BIT,
		CLEAR_BIT,
		SET_BIT);
}


void Cpu::jump_relative() {
	u8 immediate = ~read_byte_inc_PC();
	const bool sign = ((immediate & 0x80) == 0x00);
	immediate += 0x01;

	if (sign) { // Subtract immediate.
		cpu_reg.PC -= static_cast<u16>(immediate);
	}
	else { // Add immediate.
		cpu_reg.PC += static_cast<u16>(immediate);
	}
}


void Cpu::call() {
	push(cpu_reg.PC + 3);
	cpu_reg.PC = read_word_inc_PC();
}



void Cpu::reti() {
	pop(cpu_reg.PC);
	cpu_reg.IME = true;
	cpu_reg.IME_DELAY = false;
}


void Cpu::halt_cpu() {
	// implement!
}


void Cpu::stop_cpu() {
	const u8 op = read_byte_inc_PC();
	if (op != 0x00) {
		throw("Illegal opcode! fn() = stop_cpu");
	}

	// implement!
}
