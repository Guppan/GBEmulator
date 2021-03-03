#include "Memory.h"


Memory::Memory()
	: memory{ std::make_unique<unsigned char[]>(0xFFFF) }
{}


// Reads a byte from 'address'.
char Memory::read_byte(const short address) const {
	return memory[address];
}


// Reads a word starting from 'address'.
short Memory::read_word(const short address) const {
	return ((memory[address + 1] << 8) | memory[address]);
}


// Write a byte to 'address'.
void Memory::write_byte(const short address, const unsigned char value) {
	memory[address] = value;
}


// Write a word starting at 'address'.
void Memory::write_word(short address, const short value) {
	memory[address++] = value;
	memory[address] = (value >> 8);
}


