#ifndef MEMORY_H
#define MEMORY_H

#include "Typedefs.h"

#include <memory>

class Memory
{
public:
	Memory();
	~Memory() = default;

	u8 operator[](const u16 address) const;
	u8& operator[](const u16 address);

	u16 read_word(const u16 address) const;
	void write_word(u16 address, const u16 value);
private:
	std::unique_ptr<u8[]> memory;
};

#endif