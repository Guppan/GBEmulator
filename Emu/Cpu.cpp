#include "Cpu.h"


Cpu::Cpu(Memory& mem)
	: reg{}, status_register{}, memory{ mem }
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

// Load 8-bit immediate into 'to'.
// 06 0E 16 1E 26 2E 3E	
void load(BYTE& to) {
	// to = read_increment_PC();
}

void load() {

}

// Load 'from' into 'to'.
// 78 79 7A 7B 7C 7D 7E 7F 0A 1A	'to' = A
// 40 41 42 43 44 45 46	47			'to' = B
// 48 49 4A 4B 4C 4D 4E	4F			'to' = C
// 50 51 52 53 54 55 56	57			'to' = D
// 58 59 5A 5B 5C 5D 5E	5F			'to' = E
// 60 61 62 63 64 65 66	67			'to' = H
// 68 69 6A 6B 6C 6D 6E	6F			'to' = L
// 70 71 72 73 74 75 77 36			'to' = (HL)
// 02								'to' = (BC)
// 12								'to' = (DE)
void load(BYTE& to, const BYTE& from) {}


BYTE Cpu::read_byte_increment_PC() {
	return memory.read_byte(reg.PC++);
}


WORD Cpu::read_word_increment_PC() {
	WORD word{ memory.read_word(reg.PC) };
	reg.PC += 2;
	return word;
}