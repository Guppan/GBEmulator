#ifndef VIDEORAM_H
#define VIDEORAM_H

#include "Device.h"
#include "Ram.h"

class VideoRam : public Device, public Ram
{
public:
	VideoRam();
	~VideoRam() = default;

	void reset_device() override;

	u8 read_byte(const u16) const override;
	void write_byte(const u16, const u8) override;
};

#endif // VIDEORAM_H