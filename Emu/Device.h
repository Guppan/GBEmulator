#ifndef DEVICE_H
#define DEVICE_H

#include "Typedefs.h"

class Bus;

/*
* This is the base class for all devices like the cpu, the cartridge etc.
* All devices must have a means to communicate. This is done with the methods
* 'read_byte' and 'write_byte'.
* A device must also be able to connect to the system bus.
*/
class Device
{
public:
	Device() : bus{ nullptr } {};
	virtual ~Device() { bus = nullptr; }

	void connect_to_bus(Bus* system_bus) {
		bus = system_bus;
	}

	virtual u8 read_byte(const u16 address) const = 0;
	virtual void write_byte(const u16 address, const u8 data) = 0;
protected:
	Bus* bus;
};

#endif // DEVICE_H
