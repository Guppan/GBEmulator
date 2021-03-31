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


	u8 SCX{}, SCY{}, LY{}, PX{};

	cout << "SCX, SCY, LY, PX : ";
	scanf("%hhx %hhx %hhx %hhx", &SCX, &SCY, &LY, &PX);

	u16 block = (static_cast<u8>(SCX + PX) >> 3) + 0x20 * (static_cast<u8>(SCY + LY) >> 3);

	printf("((%u + %u) >> 3) = %u\n", SCX, PX, (static_cast<u8>(SCX + PX) >> 3));

	printf("block = %04X  = %u\n", block, block);

	return 0;
}