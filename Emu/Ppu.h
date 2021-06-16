#ifndef PPU_H
#define PPU_H

#include "Device.h"

#include <memory>

class Cpu; // Forward declaration

class Ppu : public Device
{
public:
	Ppu();
	~Ppu() = default;

	void connect_to_cpu(Cpu*);

	void reset_device() override;
	u8 read_byte(const u16) const override;
	void write_byte(const u16, const u8) override;

	void tick_ppu(unsigned);
private:
	Cpu* cpu;

	struct {
		union {
			struct {
				u8 BGE : 1; // Background display enable
				u8 SPRE : 1; // Sprite enable
				u8 SPRSZ : 1; // Sprite size
				u8 BGTM : 1; // Background tile map area
				u8 BGWT : 1; // Background and window tile data area
				u8 WINE : 1; // Window enable
				u8 WINTM : 1; // Window tile map area
				u8 LCDP : 1; // LCD power
			};
			u8 LCDC; // 0xFF40
		};
		union {
			struct {
				u8 LCD_MODE    : 2;
				u8 LYC_LY_FLAG : 1;
				u8 HBLANK_INTR : 1;
				u8 VBLANK_INTR : 1;
				u8 OAM_INTR    : 1;
				u8 LYC_LY_INTR : 1;
				u8 Unused      : 1;
			};
			u8 STAT; // 0xFF41
		};

		u8 SCY; // 0xFF42
		u8 SCX; // 0xFF43
		u8 LY; // 0xFF44
		u8 LYC; // 0xFF45

		u8 BGP; // 0xFF47
	} ppu_reg;

	// Buffer with 160x144 pixels.
	std::unique_ptr<u8[]> pixel_buffer;
	unsigned buffer_offset;

	// The ppu's current render mode.
	u8 render_mode;
	unsigned dot_counter;

	// Addresses to background and window maps and data.
	u16 bg_map_address;
	u16 win_map_address;
	u16 tile_data_address;
	bool signed_address;

	// Offsets into a tile.
	u8 x_offset;
	u8 y_offset;
	unsigned line_offset;

	// Line which the ppu is currently processing.
	u8 lsb_line;
	u8 msb_line;

	void signal_interrupt(const u8);
	void stat_interrupt(const u8);

	void update_render_mode();
	void prepare_scanline();
	void set_current_line();
	void add_pixel();
};

#endif // PPU_H

// kommer använda vram och all lcd-register väldigt mycket.