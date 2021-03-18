#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include "Device.h"
#include "Mapper.h"

#include <memory>
#include <string>

class Cartridge : public Device
{
public:
	Cartridge();
	~Cartridge() = default;

	void load_cartridge(const std::string&);

	u8 read_byte(const u16) override;
	void write_byte(const u16, const u8) override;
#ifndef _DEBUG
private:
#endif
	std::unique_ptr<u8[]> rom;
	std::unique_ptr<u8[]> ram;
	std::unique_ptr<Mapper> mapper;
	unsigned rom_offset;
	unsigned ram_offset;

	void select_mapper(const u8);
	unsigned set_rom_size(const u8);
	void set_ram_size(const u8);

	u8 read_from_rom(const u16);
	u8 read_from_ram(const u16);
	void write_to_mapper(const u16, const u8);
	void write_to_ram(const u16, const u8);
	void update_offset();
};

#endif // CARTRIDGE_H