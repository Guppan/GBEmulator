#include "Ppu.h"


Ppu::Ppu() :
	ppu_reg{},
	counter{},
	pixels{ std::make_unique<u8[]>(0x5A00) },
	offset{}
{
	reset_device();
}


void Ppu::reset_device() {

}


u8 Ppu::read_byte(const u16 address) const {
	return 0x00;
}


void Ppu::write_byte(const u16 address, const u8 data) {

}


void Ppu::tick_ppu(unsigned cycles) {
	if (!ppu_reg.LCDP) {
		return;
	}

	while (cycles > 0) {
		switch (ppu_reg.Mode) {
		case 0x00: { // HBlank
		
		} break;
		case 0x01: { // VBlank

		} break;
		case 0x02: { // OAM Search
			// Always takes 80 cycles
			/*
				4 cycles är 2 st sprites.


			*/



		} break;
		case 0x03: { // VRAM to LCD transfer
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






			*/


		} break;
		}
	}
}