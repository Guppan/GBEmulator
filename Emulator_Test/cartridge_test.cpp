#include "pch.h"
#include "../Emu/Cartridge.h"


class CartridgeTest : public ::testing::Test {
public:
    CartridgeTest() = default;
    ~CartridgeTest() = default;

    void SetUp() {
        cart.load_cartridge("../Emu/Zelda.gb");
    }

    void TearDown() {}


    Cartridge cart;
};

TEST_F(CartridgeTest, RomBankTest) {
    // Read from static bank 0x00 (range 0x0000 - 0x3FFF).
    u8 data = cart.read_byte(0x0000);
    ASSERT_EQ(data, 0xC3);

    data = cart.read_byte(0x2000);
    ASSERT_EQ(data, 0xFE);

    data = cart.read_byte(0x3FFF);
    ASSERT_EQ(data, 0xFF);

    // Read from extended bank (should be bank 0x01 when cart is initialized).
    // In range 0x4000 - 0x7FFF.
    data = cart.read_byte(0x4000);
    ASSERT_EQ(data, 0xFA);

    data = cart.read_byte(0x6000);
    ASSERT_EQ(data, 0xFE);

    data = cart.read_byte(0x7FFF);
    ASSERT_EQ(data, 0xFF);

    // Switch to bank 0x00. This should default to bank 0x01.
    cart.write_byte(0x2000, 0x00);
    data = cart.read_byte(0x4000);
    ASSERT_EQ(data, 0xFA);

    data = cart.read_byte(0x6000);
    ASSERT_EQ(data, 0xFE);

    data = cart.read_byte(0x7FFF);
    ASSERT_EQ(data, 0xFF);

    // Switch to bank 0x01.
    cart.write_byte(0x2000, 0x01);
    data = cart.read_byte(0x4000);
    ASSERT_EQ(data, 0xFA);

    data = cart.read_byte(0x6000);
    ASSERT_EQ(data, 0xFE);

    data = cart.read_byte(0x7FFF);
    ASSERT_EQ(data, 0xFF);

    // Switch to bank 0x02. This is physical addresses 0x8000 - 0xBFFF.
    cart.write_byte(0x2000, 0x02);
    data = cart.read_byte(0x4000);
    ASSERT_EQ(data, 0x06);

    data = cart.read_byte(0x6000);
    ASSERT_EQ(data, 0xE5);

    data = cart.read_byte(0x7FFF);
    ASSERT_EQ(data, 0xFF);

    // Switch to bank 0x0F. This is physical addresses 0x3C000 - 0x3FFFF.
    cart.write_byte(0x2000, 0x0F);
    data = cart.read_byte(0x4000);
    ASSERT_EQ(data, 0x87);

    data = cart.read_byte(0x6000);
    ASSERT_EQ(data, 0xE7);

    data = cart.read_byte(0x7FFF);
    ASSERT_EQ(data, 0x00);
}


TEST_F(CartridgeTest, RamBankTest) {
    u8 data = 0x00;

    // Write/read to/from ram with ram enabled.
    cart.write_byte(0x0000, 0x0A);
    cart.write_byte(0xA000, 0x23);
    cart.write_byte(0xB000, 0x34);
    cart.write_byte(0xBFFF, 0x45);

    data = cart.read_byte(0xA000);
    ASSERT_EQ(data, 0x23);

    data = cart.read_byte(0xB000);
    ASSERT_EQ(data, 0x34);

    data = cart.read_byte(0xBFFF);
    ASSERT_EQ(data, 0x45);

    // Write/read to/from ram with ram disabled.
    cart.write_byte(0x0000, 0x00);
    cart.write_byte(0xA000, 0x11);
    cart.write_byte(0xB000, 0x22);
    cart.write_byte(0xBFFF, 0x33);

    data = cart.read_byte(0xA000);
    ASSERT_EQ(data, 0x00);

    data = cart.read_byte(0xB000);
    ASSERT_EQ(data, 0x00);

    data = cart.read_byte(0xBFFF);
    ASSERT_EQ(data, 0x00);
}