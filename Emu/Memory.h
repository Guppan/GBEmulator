#ifndef MEMORY_H
#define MEMORY_H

#include "Typedefs.h"

#include <memory>

class Memory
{
public:
	Memory();
	~Memory() = default;

	BYTE read_byte(const WORD address) const;
	WORD read_word(const WORD address) const;

	void write_byte(const WORD address, const BYTE value);
	void write_word(WORD address, const WORD value);

	BYTE& operator[](const WORD address);
	const BYTE& operator[](const WORD address) const;
private:
	std::unique_ptr<BYTE[]> memory;
};

#endif