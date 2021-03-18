#ifndef MBC1_H
#define MBC1_H

#include "Mapper.h"

class MBC1 : public Mapper
{
public:
	MBC1();
	~MBC1() = default;

	void write_control(const u16, const u8) override;
private:
	void set_ram_enable(const u8);
	void select_rom_bank(const u8);
	void select_extended_bank(const u8);
	void set_mode(const u8);
};

#endif // MBC1_H