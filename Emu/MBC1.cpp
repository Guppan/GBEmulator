#include "MBC1.h"


MBC1::MBC1()
	: Mapper{}
{}


void MBC1::write_control(const u16 address, const u8 data) {
	if (address >= 0x0000 && address < 0x2000) {
		set_ram_enable(data);
	}
	else if (address >= 0x2000 && address < 0x4000) {
		select_rom_bank(data);
	}
	else if (address >= 0x4000 && address < 0x6000) {
		select_extended_bank(data);
	}
	else if (address >= 0x6000 && address < 0x8000) {
		set_mode(data);
	}
}


void MBC1::set_ram_enable(const u8 data) {
	if ((data & 0x0F) == 0x0A) {
		ram_enable = true;
	}
	else {
		ram_enable = false;
	}
}


void MBC1::select_rom_bank(const u8 data) {
	u8 selected_bank = (data & 0x1F);

	if (selected_bank == 0x00) {
		selected_bank = 0x01;
	}

	rom_bank = selected_bank;
}


void MBC1::select_extended_bank(const u8 data) {
	if (extended_rom_bank) {
		rom_bank |= ((0x10 << data) & 0xE0);
		extended_rom_bank = false;
	}
	else if (extended_ram_bank) {
		ram_bank = data;
		extended_ram_bank = false;
	}
}


void MBC1::set_mode(const u8 data) {
	if (data == 0x00) {
		extended_rom_bank = true;
		extended_ram_bank = false;
	}
	else if (data == 0x01) {
		extended_rom_bank = false;
		extended_ram_bank = true;
	}
	else {
		extended_rom_bank = false;
		extended_ram_bank = false;
	}
}