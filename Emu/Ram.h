#ifndef RAM_H
#define RAM_H

#include "Typedefs.h"

#include <memory>

class Ram
{
public:
	Ram(const u16 offset, const u16 size)
		: address_offset{ offset }, ram{ std::make_unique<u8[]>(size) }
	{}
	virtual ~Ram() = default;

protected:
	u16 address_offset;
	std::unique_ptr<u8[]> ram;
};

#endif // RAM_H