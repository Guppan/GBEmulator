#ifndef MEMORY_H
#define MEMORY_H

#include "Typedefs.h"

#include <memory>

class Memory
{
public:
	Memory();
	~Memory() = default;

	BYTE operator[](const WORD address) const;
	BYTE& operator[](const WORD address);

	WORD read_word(const WORD address) const;
	void write_word(WORD address, const WORD value);
private:
	std::unique_ptr<BYTE[]> memory;
};

#endif