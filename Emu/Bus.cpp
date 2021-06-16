#include "Bus.h"


Bus::Bus() :
	cartridge{}
{
	cartridge.connect_to_bus(this);
	cpu.connect_to_bus(this);
	ppu.connect_to_bus(this);
	reg_ram.connect_to_bus(this);
	work_ram.connect_to_bus(this);
	video_ram.connect_to_bus(this);

	cpu.connect_to_ppu(&ppu);
	ppu.connect_to_cpu(&cpu);
}


u8 Bus::read_byte(const u16 address) {
	u8 data = 0x00;

	if (address < 0x8000) {	// Read from cartridge rom
		data = cartridge.read_byte(address);
	}
	else if (address < 0xA000) { // Read from video ram
		data = video_ram.read_byte(address);
	}
	else if (address < 0xC000) { // Read from cartridge ram
		data = cartridge.read_byte(address);
	}
	else if (address < 0xFE00) { // Read from work ram
		data = work_ram.read_byte(address);
	}
	else { // Write to register ram.
		data = reg_ram.read_byte(address);
	}

	return data;
}


void Bus::write_byte(const u16 address, const u8 data) {
	if (address < 0x8000) {	// Write to cartridge MBC controller
		cartridge.write_byte(address, data);
	}
	else if (address < 0xA000) { // Write to video ram
		video_ram.write_byte(address, data);
	}
	else if (address < 0xC000) { // Write to cartridge ram
		cartridge.write_byte(address, data);
	}
	else if (address < 0xFE00) { // Write to work ram
		work_ram.write_byte(address, data);
	}
	else { // Write to register ram.
		reg_ram.write_byte(address, data);
	}
}