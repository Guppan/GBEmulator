#include "VideoRam.h"


VideoRam::VideoRam() :
	Ram{ 0x8000, 0x2000 }
{
	reset_device();
}


void VideoRam::reset_device() {
	for (u16 i{}; i < 0x2000; ++i) {
		ram[i] = 0x00;
	}
}


u8 VideoRam::read_byte(const u16 address) const {
	const u16 adjusted_address = address - address_offset;
	return ram[adjusted_address];
}


void VideoRam::write_byte(const u16 address, const u8 data) {
	const u16 adjusted_address = address - address_offset;
	ram[adjusted_address] = data;
}