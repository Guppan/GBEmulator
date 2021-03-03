#pragma once

#include <memory>

class Memory
{
public:
	Memory();
	~Memory() = default;

	char read_byte(const short address) const;
	short read_word(const short address) const;

	void write_byte(const short address, const unsigned char value);
	void write_word(short address, const short value);
private:
	std::unique_ptr<unsigned char[]> memory;
};

