#include "pch.h"
#include "../Emu/Memory.h"
#include "../Emu/Cpu.h"

Memory mem;
Cpu cpu{ mem };

TEST(MemoryTest, WriteReadBytes) {
	for (WORD i{ 0x0000 }; i < 0xFFFF; ++i) {
		mem.write_byte(i, static_cast<BYTE>(i));
	}

	for (WORD i{ 0x0000 }; i < 0xFFFF; ++i) {
		ASSERT_EQ(mem.read_byte(i), static_cast<BYTE>(i));
	}
}

TEST(MemoryTest, WriteReadWord) {
	mem.write_word(0xFF00, 0x1234);
	ASSERT_EQ(mem.read_word(0xFF00), 0x1234);
}