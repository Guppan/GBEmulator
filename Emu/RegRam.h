#ifndef REGRAM_H
#define REGRAM_H

#include "Device.h"
#include "Ram.h"

class RegRam : public Device, public Ram
{
public:
	RegRam();

	void reset_device() override;

	u8 read_byte(const u16) const override;
	void write_byte(const u16, const u8) override;
};

#endif // REGRAM_H