#include "WorkRam.h"


WorkRam::WorkRam() :
	Ram{ 0xC000, 0x3E00 }
{
	reset_device();
}


void WorkRam::reset_device() {
	for (u16 i{}; i < 0x3E00; ++i) {
		ram[i] = 0x00;
	}
}


u8 WorkRam::read_byte(const u16 address) const {
	const u16 adjusted_address = address - address_offset;
	return ram[adjusted_address];
}


void WorkRam::write_byte(const u16 address, const u8 data) {
	static const u16 echo_offset = 0x2000;
	const u16 adjusted_address = address - address_offset;

	if (address < 0xDE00) {	// Echo ram.
		ram[adjusted_address] = data;
		ram[adjusted_address + echo_offset] = data;
	}
	else if (address < 0xE000) {
		ram[adjusted_address] = data;
	}
	else { // Echo ram.
		ram[adjusted_address] = data;
		ram[adjusted_address - echo_offset] = data;
	}
}