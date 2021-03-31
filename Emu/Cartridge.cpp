#include "Cartridge.h"
#include "MBC1.h"

#include <fstream>
#include <iostream>

// -------- Constants ----------
const u16 ROM_BANK_START = 0x4000;
const u16 ROM_PAGE_SIZE =  0x4000;
const u16 RAM_BANK_START = 0xA000;
const u16 RAM_PAGE_SIZE =  0x2000;
// -----------------------------

Cartridge::Cartridge() :
	rom{ nullptr },
	ram{ nullptr },
	mapper{ nullptr },
	rom_offset{ ROM_BANK_START },
	ram_offset{}
{}


void Cartridge::load_cartridge(const std::string& file_path) {
	u8 mbc_mode, rom_size, ram_size;
	std::ifstream file{ file_path, std::ios::binary };
	

	// Read and store the rom info at addresses 0147h, 0148,h and 0149h.
	file.seekg(0x0147, std::ios::beg);
	file.read(reinterpret_cast<char*>(&mbc_mode), sizeof(mbc_mode));
	file.read(reinterpret_cast<char*>(&rom_size), sizeof(rom_size));
	file.read(reinterpret_cast<char*>(&ram_size), sizeof(ram_size));
	file.seekg(0, std::ios::beg);

	// Select which mapper the cartridge should use.
	select_mapper(mbc_mode);

	// Allocate memory for the rom and copy its content.
	unsigned size = set_rom_size(rom_size);
	file.read(reinterpret_cast<char*>(&rom[0]), size);
	file.close();

	// Allocate memory for the ram.
	set_ram_size(ram_size);

}


void Cartridge::reset_device() {
	rom = nullptr;
	ram = nullptr;
	mapper = nullptr;
	rom_offset = ROM_BANK_START;
	ram_offset = 0x0000;
}


// Read a byte from either rom or ram.
u8 Cartridge::read_byte(const u16 address) const {
	u8 data = 0x00;

	if (address < 0x8000) {
		data = read_from_rom(address);
	}
	else {
		data = read_from_ram(address);
	}

	return data;
}


// Write a byte to either the mapper control or ram.
void Cartridge::write_byte(const u16 address, const u8 data) {
	if (address < 0x8000) {
		write_to_mapper(address, data);
	}
	else {
		write_to_ram(address, data);
	}
}


// ---------- Private Methods ----------
// -------------------------------------


// Determine which mapper the cartridge is using.
void Cartridge::select_mapper(const u8 data) {
	if (data >= 0x01 && data < 0x04) {
		mapper = std::make_unique<MBC1>();
	}
}


// Allocate rom memory equal to the amount specified in the cartridge.
unsigned Cartridge::set_rom_size(const u8 data) {
	const u8 low_nibble = data & 0x0F;
	const u8 high_nibble = ((data & 0xF0) == 0x50);
	unsigned size = 0x100000 * high_nibble + 0x8000 * (1 << low_nibble);

	rom = std::make_unique<u8[]>(size);
	return size;
}


// Allocate ram memory equal to the amount specified in the cartridge.
void Cartridge::set_ram_size(const u8 data) {
	const unsigned size[6]{ 0, 0x800, 0x2000, 0x8000, 0x20000, 0x10000 };
	ram = std::make_unique<u8[]>(size[data]);
}


// Read from static rom bank 0 or the currently selected extended rom bank.
u8 Cartridge::read_from_rom(const u16 address) const {
	u8 data = 0x00;

	if (address < ROM_BANK_START) { // Read from static rom bank 0.
		data = rom[address];
	}
	else { // Read from extended rom bank.
		data = rom[rom_offset + (address - ROM_BANK_START)];
	}

	return data;
}


// Read from the ram bank currently selected.
u8 Cartridge::read_from_ram(const u16 address) const {
	u8 data = 0x00;

	if (mapper->get_ram_enable()) {
		data = ram[ram_offset + (address - RAM_BANK_START)];
	}

	return data;
}


// Write to the mapper control and update the offsets.
void Cartridge::write_to_mapper(const u16 address, const u8 data) {
	mapper->write_control(address, data);
	update_offset();
}


// Write to the ram bank currently selected.
void Cartridge::write_to_ram(const u16 address, const u8 data) {
	if (mapper->get_ram_enable()) {
		ram[ram_offset + (address - RAM_BANK_START)] = data;
	}
}


// Updates both rom and ram offsets.
void Cartridge::update_offset() {
	rom_offset = ROM_PAGE_SIZE * mapper->get_rom_bank();
	ram_offset = RAM_PAGE_SIZE * mapper->get_ram_bank();
}

// behöver en offset till bank.

/*
* read address 0x0147 to check which MCB mode to use.
*
 00h  ROM ONLY                 19h  MBC5
 01h  MBC1                     1Ah  MBC5+RAM
 02h  MBC1+RAM                 1Bh  MBC5+RAM+BATTERY
 03h  MBC1+RAM+BATTERY         1Ch  MBC5+RUMBLE
 05h  MBC2                     1Dh  MBC5+RUMBLE+RAM
 06h  MBC2+BATTERY             1Eh  MBC5+RUMBLE+RAM+BATTERY
 08h  ROM+RAM                  20h  MBC6
 09h  ROM+RAM+BATTERY          22h  MBC7+SENSOR+RUMBLE+RAM+BATTERY
 0Bh  MMM01
 0Ch  MMM01+RAM
 0Dh  MMM01+RAM+BATTERY
 0Fh  MBC3+TIMER+BATTERY
 10h  MBC3+TIMER+RAM+BATTERY   FCh  POCKET CAMERA
 11h  MBC3                     FDh  BANDAI TAMA5
 12h  MBC3+RAM                 FEh  HuC3
 13h  MBC3+RAM+BATTERY         FFh  HuC1+RAM+BATTERY

* address 0x0148 contains the rom size.
* 
 00h -  32KByte (no ROM banking)
 01h -  64KByte (4 banks)
 02h - 128KByte (8 banks)
 03h - 256KByte (16 banks)
 04h - 512KByte (32 banks)
 05h -   1MByte (64 banks)  - only 63 banks used by MBC1
 06h -   2MByte (128 banks) - only 125 banks used by MBC1
 07h -   4MByte (256 banks)
 08h -   8MByte (512 banks)
 52h - 1.1MByte (72 banks)
 53h - 1.2MByte (80 banks)
 54h - 1.5MByte (96 banks)

*
* address 0x0149 contains ram size.
* 
 00h - None
 01h - 2 KBytes
 02h - 8 Kbytes
 03h - 32 KBytes (4 banks of 8KBytes each)
 04h - 128 KBytes (16 banks of 8KBytes each)
 05h - 64 KBytes (8 banks of 8KBytes each)
*/