#include "Bus.h"


Bus::Bus() :
	cartridge{}
{
	cartridge.connect_to_bus(this);
	cpu.connect_to_bus(this);
	work_ram.connect_to_bus(this);
	ppu.connect_to_bus(this);
}


u8 Bus::read_byte(const u16 address) {
	u8 data = 0x00;

	if (address >= 0x0000 && address < 0x8000) {		// Read from cartridge rom
		data = cartridge.read_byte(address);
	}
	else if (address >= 0x8000 && address < 0xA000) {	// Read from video ram

	}
	else if (address >= 0xA000 && address < 0xC000) {	// Read from cartridge ram
		data = cartridge.read_byte(address);
	}
	else if (address >= 0xC000 && address <= 0xFFFF) {	// Read from internal ram
		data = work_ram.read_byte(address);
	}

	return data;
}


void Bus::write_byte(const u16 address, const u8 data) {
	if (address >= 0x0000 && address < 0x8000) {		// Write to cartridge MBC controller
		cartridge.write_byte(address, data);
	}
	else if (address >= 0x8000 && address < 0xA000) {	// Write to video ram

	}
	else if (address >= 0xA000 && address < 0xC000) {	// Write to cartridge ram
		cartridge.write_byte(address, data);
	}
	else if (address >= 0xC000 && address <= 0xFFFF) {
		work_ram.write_byte(address, data);
	}
}


// FFE0 - FFFF	internal cpu ram

// FEA0 - FEFF	prohibited area

// E000 - FDFF	prohibited area - echo of work ram area

// C000 - CFFF	work ram area

// A000 - BFFF	external expansion ram area

// 8000 - 9FFF	video ram

// 4000 - 7FFF	switchable rom bank - depending on which rom bank is currently active

// 0000 - 3FFF	rom area bank 0 - static bank, always the  16kb of the cartridge