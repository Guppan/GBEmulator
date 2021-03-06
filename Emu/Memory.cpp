#include "Memory.h"


Memory::Memory()
	: memory{ std::make_unique<BYTE[]>(0x10000) }
{}


// Returns a read-only byte at 'address'.
BYTE Memory::operator[](const WORD address) const {
	return memory[address];
}


// Returns a r/w reference to a byte at 'address'.
BYTE& Memory::operator[](const WORD address) {
	return memory[address];
}


// Reads a word starting from 'address'.
WORD Memory::read_word(const WORD address) const {
	return ((memory[address + 1] << 8) | memory[address]);
}


// Write a word starting at 'address'.
void Memory::write_word(WORD address, const WORD value) {
	memory[address++] = static_cast<BYTE>(value);
	memory[address] = (value >> 8);
}
