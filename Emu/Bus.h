#ifndef BUS_H
#define BUS_H

#include "Cartridge.h"
#include "Typedefs.h"

/*
* This class represent the sytem bus.
* Devices, like the cpu, ppu etc., should be able to connect to the bus and
* communicate with other devices connected to the bus.
*/

class Bus
{
public:
	Bus();
	~Bus() = default;

	u8 read_byte(const u16 address);
	void write_byte(const u16 address, const u8 data);
private:
	Cartridge cartridge;
};


/*
	cpu:
		read_byte{
			return bus->read_byte(0x0000);
		}
		write_byte{
			bus->write_byte(0x0000, 0x00);
		}

		connect_to_bus(Bus& _bus){
			bus = _bus;
		}

	bus:
		cpu.connect_to_bus(*this);

		read_byte{
			return device->read_byte(0x0000);
		}

		
*/

// FFE0 - FFFF	internal cpu ram

// FEA0 - FEFF	prohibited area

// E000 - FDFF	prohibited area - echo of work ram area

// C000 - CFFF	work ram area

// A000 - BFFF	external expansion ram area

// 8000 - 9FFF	video ram

// 4000 - 7FFF	switchable rom bank - depending on which rom bank is currently active

// 0000 - 3FFF	rom area bank 0 - static bank, always the  16kb of the cartridge

#endif