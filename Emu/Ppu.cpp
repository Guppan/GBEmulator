#include "Ppu.h"
#include "Bus.h"
#include "Cpu.h"


Ppu::Ppu() :
	cpu{ nullptr },
	ppu_reg{},
	pixel_buffer{ std::make_unique<u8[]>(0x5A00) },
	buffer_offset{}
{
	reset_device();
}


void Ppu::connect_to_cpu(Cpu* main_cpu) {
	cpu = main_cpu;
}


void Ppu::reset_device() {

}


u8 Ppu::read_byte(const u16 address) const {
	return bus->read_byte(address);
}


void Ppu::write_byte(const u16 address, const u8 data) {
	bus->write_byte(address, data);
}

// cycles is always 4 when called from CPU.
void Ppu::tick_ppu(unsigned cycles) {
	if (!ppu_reg.LCDP) { // Is the LCD on?
		return;
	}

	switch (ppu_reg.LCD_MODE) {
	case 0x00: { // HBlank
		dot_counter += cycles;
		if (dot_counter == 456) {
			update_render_mode();
			dot_counter = 0;
		}
	} break;
	case 0x01: { // VBlank
		dot_counter += cycles;
		if (dot_counter == 456) {
			update_render_mode();
			dot_counter = 0;
		}
	} break;
	case 0x02: { // OAM Search
		// We can process all sprites on the first turn since OAM is disabled for the cpu during this mode.
		if (dot_counter == 0) {
			// process all sprites.
		}

		dot_counter += cycles;
		if (dot_counter == 80) {
			update_render_mode();
			prepare_scanline();
			set_current_line(); // kanske ska vara i case 0x03.
		}

	} break;
	case 0x03: { // VRAM to LCD transfer

		while (cycles > 0) {
			add_pixel();

			++dot_counter;
			--cycles;

			if (buffer_offset == 160) {
				buffer_offset = 0;
				update_render_mode();
				dot_counter += cycles;
				break;
			}
		}
		/*
			current_scan.BGTM <- 0x9800 eller 0x9C00

			ppu_reg.LY bestämmer vilken scanline det är. Alltså vilken av LCDns 144 rader som ska bearbetas.

				Exempel.
					LY = 0x00. SCY = 0x90.
						första raden börjar på Y = 0x90.
					LY = 0x8F. SCY = 0x90.
						sista raden börjar på Y = 0x20 <- (SCY + LY)


			LCD block for a given scanline LY, the current pixel offset PX and viewport (SCX, SCY):
				u16 block = (static_cast<u8>(SCX + PX) >> 3) + 0x20 * (static_cast<u8>(SCY + LY) >> 3);


			Now we can determine which tile to process:
				u16 tile_address = [0x9800 eller 0x9C00] + block;

			video_ram[tile_address] ->	detta ger korrekt tile. OBS! kolla vilken mode ppu_reg.BGWT är i.
										om BGWT=0 så är adress 0x9000 base och 'tile_address' är signed.
										det innebär att tiles 0-127 ligger i 0x9000-0x97FF och
										tiles 128-255 ligger i 0x8800-0x8FFF.

										om BGWT=1 så är adress 0x8000 base och 'tile_address' är unsigned.
										då ligger tiles 0-255 i 0x8000-0x8FFF.


			en tile är t.ex. uppbyggd som:

				address | LSB_color	| MSB_color
				--------------------------------
				0x8000	| 0110 0011	| 1011 1001
				0x8002	| 0111 1100	| 0100 0001
				......	| .... .... | .... ....
				0x800E	| 1100 0100 | 0111 1001



			Exempel:
				låt SCX = 154	PX = 3
					SCY = 8		LY = 0

					BGWT = 1	BGTM = 0

				då får vi att
					u16 block = ((154 + 3) >> 3) + 0x20 * ((8 + 0) >> 3) = 0x0033	(51 i decimal)

					u8 tile_offset = video_ram[0x9800 + block] = video_ram[0x9833]

					eftersom BGWT = 1 så:

					u16 tile_address = 0x8000 + 0x10 * tile_offset;

				vi bearbetar alltså en tile som finns på adresserna
					(0x8000 + 0x10 * tile_offset) till och med (0x8000 + 0x10 * tile_offset + 0x0F)


				låt START = 0x8000 + 0x10 * tile_offset
					xpos  = 0x07 - ((SCX + PX) & 0x07) = 2	<- detta är offset i x-led. När xpos == 0 så måste vi börja från en ny tile i nästa iteration.
					ypos  = (((SCY + LY) & 0x07) << 1) = (((8 + 0) & 0x07) << 1) = 0	<- detta adderas till START för att få korrekt byte.

				address	| LSB_color	| MSB_color
				--------------------------------
				START+0	| .... .x.. | .... .x..
				START+2	| .... .... | .... ....
				START+4	| .... .... | .... ....
				START+6	| .... .... | .... ....
				START+8	| .... .... | .... ....
				START+A	| .... .... | .... ....
				START+C	| .... .... | .... ....
				START+E	| .... .... | .... ....


				värdet (färgen) på en pixel är kombinationen av MSB_color och LSB_color.

				u8 LSB_color = (video_ram[START + ypos] & (1 << xpos));
				u8 MSB_color = (video_ram[START + ypos + 1] & (1 << xpos));



				u8 pixel_value = ((MSB_color > 0) << 1) | (LSB_color > 0);

		*/


	} break;
	}
}


void Ppu::signal_interrupt(const u8 interrupt) {
	u8 irq = cpu->read_io(0xFF0F) | interrupt;
	cpu->write_io(0xFF0F, irq);
}


void Ppu::stat_interrupt(const u8 interrupt) {
	static const u8 STAT_INTR = 0x02;
	const u8 result = (ppu_reg.STAT ^ interrupt) & 0x78;

	if (result > interrupt) {
		signal_interrupt(STAT_INTR);
	}
}


void Ppu::update_render_mode() {
	// render_mode	next mode
	//	0x00		0x02 om LY < 144 annars 0x01
	//	0x01		0x01 om LY < 154, om LY == 154 så 0x02
	//	0x02		0x03
	//	0x03		0x00
	static const u8 VBLANK_INTR = 0x01;

	switch (ppu_reg.LCD_MODE) {
	case 0x00: {
		if (ppu_reg.LY < 144) {
			ppu_reg.LCD_MODE = 0x02;
			stat_interrupt(0x20); // OAM Interrupt.
		}
		else {
			ppu_reg.LCD_MODE = 0x01;
			signal_interrupt(VBLANK_INTR);
			stat_interrupt(0x10); // VBLANK Interrupt.
		}

		++ppu_reg.LY;
		ppu_reg.LYC_LY_FLAG = (ppu_reg.LYC != ppu_reg.LY) ? 0x00 : 0x01;
		if (ppu_reg.LYC_LY_FLAG) {
			stat_interrupt(0x40); // LYC=LY Interrupt.
		}
	} break;
	case 0x01: {
		if (ppu_reg.LY == 153) {
			ppu_reg.LCD_MODE = 0x02;
			ppu_reg.LY = 0x00;
			stat_interrupt(0x20); // OAM Interrupt.
		}
		else {
			++ppu_reg.LY;
		}

		ppu_reg.LYC_LY_FLAG = (ppu_reg.LYC != ppu_reg.LY) ? 0x00 : 0x01;
		if (ppu_reg.LYC_LY_FLAG) {
			stat_interrupt(0x40); // LYC=LY Interrupt.
		}
	} break;
	case 0x02: {
		ppu_reg.LCD_MODE = 0x03;
	} break;
	case 0x03: {
		ppu_reg.LCD_MODE = 0x00;
		stat_interrupt(0x08); // HBLANK Interrupt.
	} break;
	}
}



void Ppu::prepare_scanline() {
	bg_map_address = 0x9800 + 0x0400 * ppu_reg.BGTM; // Background tile map location: 0x9800 or 0x9C00.
	win_map_address = 0x9800 + 0x0400 * ppu_reg.WINTM; // Window tile map location: 0x9800 or 0x9C00.
	tile_data_address = 0x9000 - 0x1000 * ppu_reg.BGWT; // Background & window tile data location: 0x9000 or 0x8000.
	signed_address = ppu_reg.BGWT; // Should we use signed values for tile data location?

	x_offset = ppu_reg.SCX; // The current pixels x-position relative the LCD-viewport.
	y_offset = ppu_reg.SCY + ppu_reg.LY; // The current pixels y-position relative the LCD_viewport.
	line_offset = ppu_reg.LY * 160; // The current line offset into the pixel buffer. (y-direction)
}


// Sets the line (in a tile) the ppu is currently processing.
void Ppu::set_current_line() {
	const u16 block = (x_offset >> 3) + 0x20 * (y_offset >> 3);
	u8 tile_offset = read_byte(bg_map_address + block);
	u16 tile_address = tile_data_address;

	if (signed_address) { // Base pointer is 0x9000 so it's a signed number.
		const bool sign = ((tile_offset & 0x80) == 0x80);
		tile_offset = ~tile_offset + 0x01;

		// Add or subtract the offset depending on if it's positive or negative.
		tile_address += sign ? -0x10 * tile_offset : 0x10 * tile_offset;
	}
	else { // Base pointer is 0x8000 so it's an unsigned number.
		tile_address += 0x10 * tile_offset;
	}

	// Offset in the y-direction to get the correct line in the tile.
	tile_address += ((y_offset & 0x07) << 1);

	lsb_line = read_byte(tile_address);
	msb_line = read_byte(tile_address + 1);
}


// Add a background pixel to the pixel_buffer.
void Ppu::add_pixel() {
	const u8 pixel_offset = (1 << (0x07 - (x_offset & 0x07)));
	const u8 lsb_color = lsb_line & pixel_offset;
	const u8 msb_color = msb_line & pixel_offset;
	const u8 color_id = ((msb_color > 0) << 1) | (lsb_color > 0);
	const u8 pixel_color = (ppu_reg.BGP >> (color_id << 1)) & 0x03; // For background and window palette.

	pixel_buffer[line_offset + buffer_offset++] = pixel_color;
	++x_offset;

	if (pixel_offset == 0) { // The next line needs to be loaded.
		set_current_line();
	}
}