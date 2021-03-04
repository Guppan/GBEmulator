#include "Memory.h"


Memory::Memory()
	: memory{ std::make_unique<BYTE[]>(0x10000) }
{}


// Reads a byte from 'address'.
BYTE Memory::read_byte(const WORD address) const {
	return memory[address];
}


// Reads a word starting from 'address'.
WORD Memory::read_word(const WORD address) const {
	return ((memory[address + 1] << 8) | memory[address]);
}


// Write a byte to 'address'.
void Memory::write_byte(const WORD address, const BYTE value) {
	memory[address] = value;
}


// Write a word starting at 'address'.
void Memory::write_word(WORD address, const WORD value) {
	memory[address++] = static_cast<BYTE>(value);
	memory[address] = (value >> 8);
}


