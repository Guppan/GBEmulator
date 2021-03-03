
#include <iostream>

int main() {
	using namespace std;

	char mem[0xFF];
	
	for (unsigned short i{ 0x00 }; i < 0xFF; i += 0x01) {
		mem[i] = static_cast<char>(i);
	}



	cout << ((mem[0x01] << 8) | mem[0x02]) << endl;

	short af = 0x3132;

	mem[0x01] = af;
	mem[0x02] = (af >> 8);

	cout << mem[0x01] << " " << mem[0x02] << endl;


	return 0;
}