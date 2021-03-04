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


	struct OpInfo {
		void (*fn)(const BYTE&);
		unsigned args;
	};

	OpInfo opcodes[2]{
		{&load_byte, 0}, {&load_word, 1}
	};

	int op;
	BYTE val;
	cout << "Input opcode : ";
	cin >> op;
	cout << "Input value : ";
	cin >> val;

	struct OpInfo& selected{ opcodes[op] };
	selected.fn(val);

	int s{ INT32_MAX - 1 };
	++s;
	++s;
	

	return 0;
}