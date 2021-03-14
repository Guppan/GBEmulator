#include "Memory.h"


Memory::Memory()
	: memory{ std::make_unique<u8[]>(0x10000) }
{}


// Returns a read-only byte at 'address'.
u8 Memory::operator[](const u16 address) const {
	return memory[address];
}


// Returns a r/w reference to a byte at 'address'.
u8& Memory::operator[](const u16 address) {
	return memory[address];
}


// Reads a word starting from 'address'.
u16 Memory::read_word(const u16 address) const {
	return ((memory[address + 1] << 8) | memory[address]);
}


// Write a word starting at 'address'.
void Memory::write_word(u16 address, const u16 value) {
	memory[address++] = static_cast<u8>(value);
	memory[address] = (value >> 8);
}
