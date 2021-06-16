#include "Typedefs.h"
#include "Memory.h"
#include <iostream>
#include <functional>
#include <sstream>
#include "Cpu.h"
#include "Cartridge.h"
#include "Bus.h"
#include <chrono>

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif




int main() {
	using namespace std;

	char cont = 't';

	Bus bus{};
	Cpu* cpu = bus.get_cpu();
	Cartridge* cart = bus.get_cart();

	cart->load_cartridge("tetris.gb");
	/*
	while (cont != 'n') {
		int i = 0;
		cout << "Nr of instructions? : ";
		cin >> i;
		for (int x{}; x < i; ++x) {
			cpu->fetch_execute();
		}
		cout << "Continue? : ";
		cin >> cont;
	}
	*/
	/*
	auto t1 = chrono::high_resolution_clock::now();
	unsigned time = 0;
	while (time < 4194000) {
		time += cpu->fetch_execute();
	}
	auto t2 = chrono::high_resolution_clock::now();

	// Getting number of milliseconds as an integer. 
	auto ms_int = chrono::duration_cast<chrono::milliseconds>(t2 - t1);

	// Getting number of milliseconds as a double. 
	chrono::duration<double, std::milli> ms_double = t2 - t1;

	std::cout << ms_int.count() << "ms\n";
	std::cout << ms_double.count() << "ms";
	*/

	u8 data[] = {
		0b11111111, 0b00000000,
		0b01111110, 0b11111111,
		0b10000101, 0b10000001,
		0b10001001, 0b10000011,
		0b10010011, 0b10000101,
		0b10100101, 0b10001011,
		0b11001001, 0b10010111,
		0b01111110, 0b11111111
	};

	/*
	u8 SCX{}, SCY{}, LY{}, PX{};

	cout << "SCX, SCY, LY, PX : ";
	if (!scanf("%hhx %hhx %hhx %hhx", &SCX, &SCY, &LY, &PX)) {
		return -1;
	}

	u8 xoff = SCX + PX;													// 152 + 0
	u8 yoff = SCY + LY;

	u8 xpos = 0x07 - (xoff & 0x07);										// 7 - (152 & 7) = 3
	u8 ypos = ((yoff & 0x07) << 1);

	printf("xpos : %X\nypos : %X\n", xpos, ypos);

	u8 LSB_color = (data[0 + ypos] & (1 << xpos));
	u8 MSB_color = (data[0 + ypos + 1] & (1 << xpos));

	u8 pixel_value = ((MSB_color > 0) << 1) | (LSB_color > 0);

	printf("value : %02X\n", pixel_value);

	*/
	/*
	int i;
	cout << "input 1 or 0 : ";
	cin >> i;

	u16 base = 0x9000;

	base += (i == 1) ? 0x20 * 2 : -0x20 * 2;

	printf("Base = %04X\n", base);
	*/


	return 0;
}