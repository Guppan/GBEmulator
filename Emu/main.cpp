#include "Typedefs.h"
#include "Memory.h"
#include <iostream>

BYTE mem[0xFF];

void load_byte(const BYTE& val) {
	mem[0x1F] = val;
}

void load_word(const BYTE& val) {
	mem[0x20] = val;
	mem[0x21] = val;
}

int main() {
	using namespace std;

	
	
	for (WORD i{ 0x00 }; i < 0xFF; i += 0x01) {
		mem[i] = static_cast<BYTE>(i);
	}



	cout << ((mem[0x01] << 8) | mem[0x02]) << endl;

	WORD af = 0x3132;

	mem[0x01] = af;
	mem[0x02] = (af >> 8);

	cout << mem[0x01] << " " << mem[0x02] << endl;

	union StatusRegister {
		struct {
			BYTE C : 1; // Carry flag
			BYTE H : 1; // Half-carry flag
			BYTE N : 1; // Subtract flag
			BYTE Z : 1; // Zero flag
			BYTE unused : 4;
		};
		BYTE status;
	} regg;

	regg.status = 0x00;

	regg.Z = 1;
	regg.C = 1;

	cout << (regg.status == 0x09) << endl;

	BYTE hej = (regg.Z << 7) + (regg.N << 6) + (regg.H << 5) + (regg.C << 4);

	cout << (hej == 0x90) << endl;

	cout << "hej : "<<  ((((0xAFFF & 0x0FFF) + 0x7F) & 0x1000) == 0x1000) << endl;

	return 0;
}