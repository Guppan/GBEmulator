#ifndef INTERNALRAM_H
#define INTERNALRAM_H

#include "Device.h"

#include <memory>

class InternalRam : public Device
{
public:
	InternalRam();
	~InternalRam() = default;

	u8 read_byte(const u16) const override;
	void write_byte(const u16, const u8) override;
private:
	std::unique_ptr<u8[]> ram;
};

#endif // INTERNALRAM_H