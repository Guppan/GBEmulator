#ifndef MAPPER_H
#define MAPPER_H

#include "Typedefs.h"

class Mapper
{
public:
	Mapper() :
		ram_enable{ false },
		extended_rom_bank{ false },
		extended_ram_bank{ false },
		rom_bank{ 0x01 },
		ram_bank{ 0x00 } {}

	~Mapper() = default;

	bool get_ram_enable() const { return ram_enable; }
	u8 get_rom_bank() const { return rom_bank; }
	u8 get_ram_bank() const { return ram_bank; }

	virtual void write_control(const u16, const u8) {};
protected:
	bool ram_enable;
	bool extended_rom_bank;
	bool extended_ram_bank;
	u8 rom_bank;
	u8 ram_bank;
};

#endif // MAPPER_H