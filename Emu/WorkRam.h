#ifndef INTERNALRAM_H
#define INTERNALRAM_H

#include "Device.h"
#include "Ram.h"

#include <memory>

class WorkRam : public Device, public Ram
{
public:
	WorkRam();
	~WorkRam() = default;

	void reset_device() override;

	u8 read_byte(const u16) const override;
	void write_byte(const u16, const u8) override;
};

#endif // INTERNALRAM_H