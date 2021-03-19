#include "InternalRam.h"


InternalRam::InternalRam() :
	ram{ std::make_unique<u8[]>(0x4000) }
{

}


u8 InternalRam::read_byte(const u16 address) const {
	const u16 adjusted_address = address - 0xC000;
	return ram[adjusted_address];
}


void InternalRam::write_byte(const u16 address, const u8 data) {
	static const u16 echo_offset = 0x2000;
	const u16 adjusted_address = address - 0xC000;

	if (address >= 0xC000 && address < 0xDE00) {		// Echo ram.
		ram[adjusted_address] = data;
		ram[adjusted_address + echo_offset] = data;
	}
	else if (address >= 0xE000 && address < 0xFE00) {	// Echo ram.
		ram[adjusted_address] = data;
		ram[adjusted_address - echo_offset] = data;
	}
	else {
		ram[adjusted_address] = data;
	}
}


// FFE0 - FFFF	internal cpu ram

// FEA0 - FEFF	prohibited area

// E000 - FDFF	prohibited area - echo of work ram area

// C000 - CFFF	work ram area