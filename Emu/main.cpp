#include "Typedefs.h"
#include "Memory.h"
#include <iostream>
#include <functional>
#include <sstream>
#include "Cpu.h"


int main() {
	using namespace std;

	u8 ref{ 0b11000010 };

	u8 bitt{ 0x06 };
	u8 bt{ 0x01 };

	u8 res{ static_cast<u8>(bt << bitt) };

	cout << ((ref & (0x01 << bitt)) == 0x00) << endl;

	ref &= ~(0x01 << bitt);

	cout << (ref == 0b10000010) << endl;

	u8 a{ 0b10010110 }; // 1001 0110
	u8 nh{ static_cast<u8>(a << 4) };
	u8 b{ static_cast<u8>(nh | (a >> 4)) };
	cout << (b == 0b01101001) << endl;

	Memory mem;
	Cpu cpu{ mem };

	u16 code{ 0x0003 };
	u16 off{ static_cast<u16>(0x0000 + (code * 0x0008)) };

	cout << (off == 0x0018) << endl;



	return 0;
}