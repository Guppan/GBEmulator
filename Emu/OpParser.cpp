#include "OpParser.h"

#include <fstream>
#include <sstream>
#include <iostream>

// ------------- Constants -------------
const std::string CYCLE_NAME{ "cycles.txt" };
const std::string OPCODE_NAME{ "opcodes.txt" };
// -------------------------------------

OpParser::OpParser(Cpu& _cpu, ConstNums& CONST_NUMS)
	: cpu{ _cpu }, c_nums{ CONST_NUMS }
{
	parse_cycles();
	parse_instructions();
}


u16 OpParser::string_to_word(const std::string& str) const {
	return static_cast<u16>(std::stoul(str, nullptr, 16));
}


u16 OpParser::parse_opcode(const std::string& str) const {
	u16 opcode{ string_to_word(str) };

	if (opcode > 0x00FF) {
		opcode = static_cast<u16>(0x0100 | (opcode & 0x00FF));
	}

	return opcode;
}


void OpParser::parse_cycles() {
	std::ifstream input_stream{ CYCLE_NAME, std::ios::in };
	std::string line;

	u16 index{ 0x0000 };
	while (std::getline(input_stream, line)) {
		std::stringstream stream;
		stream.str(line);

		for (u16 offset{ 0x0000 }; offset < 0x0010; ++offset) {
			stream >> cpu.cycle_matrix[index + offset];
		}
		
		index += 0x0010;
	}
}


void OpParser::parse_instructions() {
	typedef void(Cpu::* BytePtr)(u8&, const u8&);	// 8bit function pointer.
	typedef void(Cpu::* WordPtr)(u16&, const u16&); // 16bit function pointer.
	typedef void(Cpu::* VoidPtr)();					// void function pointer.

	const std::map<std::string, BytePtr> func_byte_map{
		{"ldd", &Cpu::load_byte},   {"add", &Cpu::add_byte},    {"adc", &Cpu::add_byte_c},
		{"sub",& Cpu::sub_byte},    {"sbc", &Cpu::sub_byte_c},  {"and", &Cpu::and_byte},
		{"or", &Cpu::or_byte},      {"xor", &Cpu::xor_byte},    {"cp", &Cpu::cp_byte},
		{"inc", &Cpu::inc_byte},    {"dec", &Cpu::dec_byte},    {"lds",&Cpu::load_byte_switch},
		{"tst", &Cpu::test_bit},    {"rst", &Cpu::reset_bit},   {"rot", &Cpu::rotate},
		{"sft", &Cpu::shift_byte},  {"swp", &Cpu::swap_nibble}, {"alu", &Cpu::alu_byte_imm},
		{"set", &Cpu::set_bit},     {"ldmem", &Cpu::load_byte_mem}
	};

	const std::map<std::string, WordPtr> func_word_map{
		{"ldw", &Cpu::load_word}, {"ldhl", &Cpu::load_hl}, {"rtcp", &Cpu::ret_cp},
		{"addw", &Cpu::add_word}, {"addsp", &Cpu::add_sp}, {"incw", &Cpu::inc_word},
		{"decw", &Cpu::dec_word}, {"push", &Cpu::push},    {"pop", &Cpu::pop},
		{"jpcp", &Cpu::jump_cp},  {"clcp", &Cpu::call_cp}, {"jprl", &Cpu::jump_relative_cp},
		{"rstr", &Cpu::restart}
	};

	const std::map<std::string, VoidPtr> func_void_map{
		{"ldi+", &Cpu::load_memory_inc},  {"daa", &Cpu::daa},   {"cpl", &Cpu::cpl},
		{"ldi-", &Cpu::load_memory_dec},  {"ccf", &Cpu::ccf},   {"scf", &Cpu::scf}, 
		{"ldimm", &Cpu::load_memory_imm}, {"nop", &Cpu::nop},   {"jmp", &Cpu::jump},
		{"jmpr", &Cpu::jump_relative},    {"ret", &Cpu::ret},   {"call", &Cpu::call},
		{"jmphl", &Cpu::jump_hl},         {"reti", &Cpu::reti}, {"ldhlsp", &Cpu::load_sp_hl},
		{"ldsp", &Cpu::load_stack},       {"ei", &Cpu::ei},     {"halt", &Cpu::halt_cpu},
		{"stop", &Cpu::stop_cpu},         {"di", &Cpu::di},     {"ldoff", &Cpu::load_imm_off}
	};

	const std::map<std::string, u8*> register_8bit_map{
		{"A", &cpu.reg.A}, {"B", &cpu.reg.B}, {"C", &cpu.reg.C}, {"D", &cpu.reg.D},
		{"E", &cpu.reg.E}, {"F", &cpu.reg.F}, {"H", &cpu.reg.H}, {"L", &cpu.reg.L},

		{"0", &c_nums.b0}, {"1", &c_nums.b1}, {"2", &c_nums.b2}, {"3", &c_nums.b3},
		{"4", &c_nums.b4}, {"5", &c_nums.b5}, {"6", &c_nums.b6}, {"7", &c_nums.b7},
		{"...", &c_nums.b0}
	};

	const std::map<std::string, u16*> register_16bit_map{
		{"AF", &cpu.reg.AF}, {"BC",& cpu.reg.BC}, {"DE", &cpu.reg.DE},
		{"HL", &cpu.reg.HL}, {"PC", &cpu.reg.PC}, {"SP", &cpu.reg.SP},

		{"0", &c_nums.w0}, {"1", &c_nums.w1}, {"2", &c_nums.w2}, {"3", &c_nums.w3},
		{"4", &c_nums.w4}, {"5", &c_nums.w5}, {"6", &c_nums.w6}, {"7", &c_nums.w7},
		{"...",& c_nums.w0}
	};
	
	std::ifstream input_stream{ OPCODE_NAME, std::ios::in };
	std::string line;

	while (std::getline(input_stream, line)) {
		std::stringstream stream;
		stream.str(line);

		char func_type;
		std::string _opcode, _func, operand1, operand2;
		stream >> func_type >> _opcode >> _func >> operand1 >> operand2;

		std::function<void()> instruction;
		if (func_type == 'B') {
			BytePtr func{ get_map<BytePtr>(_func, func_byte_map) };
			u8* op1 = get_map<u8*>(operand1, register_8bit_map);
			u8* op2 = get_map<u8*>(operand2, register_8bit_map);
			instruction = std::bind(func, &cpu, std::ref(*op1), std::ref(*op2));
		}
		else if (func_type == 'W') {
			WordPtr func{ get_map<WordPtr>(_func, func_word_map) };
			u16* op1 = get_map<u16*>(operand1, register_16bit_map);
			u16* op2 = get_map<u16*>(operand2, register_16bit_map);
			instruction = std::bind(func, &cpu, std::ref(*op1), std::ref(*op2));
		}
		else if (func_type == 'V') {
			VoidPtr func{ get_map<VoidPtr>(_func, func_void_map) };
			instruction = std::bind(func, &cpu);
		}

		cpu.op_matrix[parse_opcode(_opcode)] = instruction;
	}
}