#include "Cpu.h"


Cpu::Cpu(Memory& mem)
	: reg{}, status_register{}, memory{ mem }
{}


void Cpu::read_opcode() {
	/*
	* l�s instruktion fr�n adress 'PC'.
	* unsigned char opcode = memory.read_byte(reg.PC);
	*	
	* ifall opcode = 0xCB, 0x01 s� �r instruktionen ett word.
	* 
	* if (opcode = 0xCB){
	* 
	* }
	* else{
	* 
	* }

	*/
}