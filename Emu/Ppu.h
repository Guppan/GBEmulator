#ifndef PPU_H
#define PPU_H

#include "Device.h"

class Ppu : public Device
{
public:
	Ppu() = default;
	~Ppu() = default;

	u8 read_byte(const u16) const override { return 0x00; }
	void write_byte(const u16, const u8) override {}
private:
	u8 oam[0xA0];
};

#endif // PPU_H