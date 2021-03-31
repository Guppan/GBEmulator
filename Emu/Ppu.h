#ifndef PPU_H
#define PPU_H

#include "Device.h"

#include <memory>

struct PPURegister {
	union {
		struct 
		{
			u8 BGE   : 1; // Background Enabled
			u8 SprE  : 1; // Sprite Enabled
			u8 SprSz : 1; // Sprite Size
			u8 BGTM  : 1; // Background Tile Map
			u8 BGWT  : 1; // Background and Window Tileset
			u8 WinE  : 1; // Window Enabled
			u8 WinTM : 1; // Window Tile Map
			u8 LCDP  : 1; // LCD Power
		};
		u8 LCDC; // 0xFF40
	};
	union {
		struct {
			u8 Mode    : 2; // Mode Flag
			u8 LYCFl   : 1; // LYC = LY Flag
			u8 M0Intr  : 1; // Mode 0 HBlank Interrupt
			u8 M1Intr  : 1; // Mode 1 VBlank Interrupt
			u8 M2Intr  : 1; // Mode 2 OAM Interrupt
			u8 LYCIntr : 1; // LYC = LY Interrupt
			u8 Unused  : 1;
		};
		u8 STAT; // 0xFF41
	};

	u8 SCY; // 0xFF42
	u8 SCX; // 0xFF43
	u8 LY;  // 0xFF44 - current scanline.
};

class Ppu : public Device
{
public:
	Ppu();
	~Ppu() = default;

	void reset_device() override;
	u8 read_byte(const u16) const override;
	void write_byte(const u16, const u8) override;

	void tick_ppu(unsigned);
private:
	PPURegister ppu_reg;

	struct {
		u16 BGTM;
		u8 current_pixel;
	} scanline;

	unsigned counter;

	u8 oam[0xA0];
	unsigned oam_index;

	std::unique_ptr<u8[]> video_ram;

	std::unique_ptr<u8[]> pixels;
	unsigned offset;


};

#endif // PPU_H

// kommer använda vram och all lcd-register väldigt mycket.