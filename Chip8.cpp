#include "Chip8.h"
#include <fstream>
#include <iostream>
#include <cstdint>
#include <random>
#include <chrono>

const unsigned int START_ADDRESS = 0x200; //0x0 -> 0x1FF is reserved
const unsigned int FONTSET_SIZE = 16 * 5; //16 chars, 5 bytes each
const int SCREEN_WIDTH = 64;
const int SCREEN_HEIGHT = 32;

//implementing FONTSET that is hardcoded onto the CHIP-8, loaded at 0x50
uint8_t fontset[FONTSET_SIZE] = //16 chars, 5 bytes each
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

Chip8::Chip8() {
}

void Chip8::loadROM(char const* filename) {
	std::streampos size;
	char* buffer;
	std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::ate); //open at end of file to get file size
	
	if (file.is_open()) {
		std::cout << "opened ROM" << std::endl;
		size = file.tellg(); //get filesize
		buffer = new char[size]; //create buffer of size filesize
		file.seekg(0, std::ios::beg);
		file.read(buffer, size);
		file.close();

		for (long i = 0; i < size; ++i) {
			memory[START_ADDRESS + i] = buffer[i];
		}

		delete[] buffer;
	}
	else { std::cout << "Unable to open " << filename << std::endl; }
}

void Chip8::dumpMemory() {
	unsigned int counter = 0;
	for (int i = 0; i < 4096; i++) {
		counter++;
		std::cout << std::hex << std::uppercase << unsigned(memory[i]) << std::nouppercase << std::dec <<  " ";
		if (counter == 16) {
			counter = 0;
			std::cout << std::endl;
		}
	}
}

void Chip8::init() {
	//init memory, registers, stack, etc to 0
	memset(memory, 0, sizeof(memory));
	memset(registers, 0, sizeof(registers));
	memset(stack, 0, sizeof(stack));
	memset(keys, 0, sizeof(keys));
	memset(pixels, 0, sizeof(pixels));
	delayTimer = 0;
	soundTimer = 0;
	opcode = 0;
	sp = 0;
	index = 0;
	//set the program counter to the start of the ROM
	pc = START_ADDRESS;
	//load fontset into memory at 0x50
	for (unsigned int i = 0; i < FONTSET_SIZE; ++i) {
		memory[0x50 + i] = fontset[i];
	}

	srand(time(NULL));
}

void Chip8::cycle() {
	opcode = (memory[pc] << 0x8u) + memory[pc+1]; //fetch opcode from program counter
	std::cout << std::hex << opcode << std::endl;
	pc += 2;

	//decode and execute opcode
	switch ((opcode & 0xF000u)) {
	case 0x0000u:
		switch ((opcode & 0x00FFu)) {
		case 0x00E0: op00E0(); break;
		case 0x00EE: op00EE(); break;
		}
		break;

	case 0x1000u: op1nnn(); break;
	case 0x2000u: op2nnn(); break;
	case 0x3000u: op3xkk(); break;
	case 0x4000u: op4xkk(); break;
	case 0x5000u: op5xy0(); break;
	case 0x6000u: op6xkk(); break;
	case 0x7000u: op7xkk(); break;
	case 0x8000u:
		switch ((opcode & 0x000Fu)) {
		case 0x0000: op8xy0(); break;
		case 0x0001: op8xy1(); break;
		case 0x0002: op8xy2(); break;
		case 0x0003: op8xy3(); break;
		case 0x0004: op8xy4(); break;
		case 0x0005: op8xy5(); break;
		case 0x0006: op8xy6(); break;
		case 0x0007: op8xy7(); break;
		case 0x000E: op8xyE(); break;
		}
		break;

	case 0x9000u: op9xy0(); break;
	case 0xA000u: opAnnn(); break;
	case 0xB000u: opBnnn(); break;
	case 0xC000u: opCxkk(); break;
	case 0xD000u: opDxyn(); break;
	case 0xE000u:
		switch ((opcode & 0x00FFu)) {
		case 0x009E: opEx9E(); break;
		case 0x00A1: opExA1(); break;
		}
		break;

	case 0xF000u:
		switch ((opcode & 0x00FFu)) {
		case 0x0007: opFx07(); break;	
		case 0x000A: opFx0A(); break;
		case 0x0015: opFx15(); break;
		case 0x0018: opFx18(); break;
		case 0x001E: opFx1E(); break;
		case 0x0029: opFx29(); break;
		case 0x0033: opFx33(); break;
		case 0x0055: opFx55(); break;
		case 0x0065: opFx65(); break;	
		}

		break;
	}


	//decrement timers
	if (delayTimer > 0) {
		delayTimer--;
	}
	if (soundTimer > 0) {
		soundTimer--;
	}
}

/*
##########################################
###############OP CODES###################
##########################################
*/

void Chip8::op00E0() { //CLS
	memset(pixels, 0, sizeof(pixels));
}

void Chip8::op00EE() { //RET
	sp--;
	pc = stack[sp];
}

void Chip8::op1nnn() { //JP addr
	pc = (opcode & 0x0FFFu);
}

void Chip8::op2nnn() { //CALL addr
	stack[sp] = pc;
	sp++;
	pc = (opcode & 0x0FFFu);
}

void Chip8::op3xkk() { //SE Vx, byte
	if (registers[(opcode & 0x0F00u) >> 0x8u] == (opcode & 0x00FFu)) {
		pc += 2;
	}
}

void Chip8::op4xkk() { //SNE Vx, byte
	if (registers[(opcode & 0x0F00u) >> 0x8u] != (opcode & 0x00FFu)) {
		pc += 2;
	}
}

void Chip8::op5xy0() { //SE Vx, Vy
	if (registers[(opcode & 0x0F00u) >> 0x8u] == registers[(opcode & 0x00F0u) >> 0x4u]) {
		pc += 2;
	}
}

void Chip8::op6xkk() { //LD Vx, Byte
	registers[(opcode & 0x0F00u) >> 0x8u] = (opcode & 0x00FFu);
}

void Chip8::op7xkk() { //ADD Vx, byte
	registers[(opcode & 0x0F00u) >> 0x8u] += (opcode & 0x00FFu);
}

void Chip8::op8xy0() { //LD Vx, Vy
	registers[(opcode & 0x0F00u) >> 0x8u] = registers[(opcode & 0x00F0u) >> 0x4u];
}

void Chip8::op8xy1() { //OR Vx, Vy
	registers[(opcode & 0x0F00u) >> 0x8u] |= registers[(opcode & 0x00F0u) >> 0x4u];
}

void Chip8::op8xy2() { //AND Vx, Vy
	registers[(opcode & 0x0F00u) >> 0x8u] &= registers[(opcode & 0x00F0u) >> 0x4u];
}

void Chip8::op8xy3() { //XOR Vx, Vy
	registers[(opcode & 0x0F00u) >> 0x8u] ^= registers[(opcode & 0x00F0u) >> 0x4u];
}

void Chip8::op8xy4() { //ADD Vx, Vy
	uint16_t overflow = registers[(opcode & 0x0F00u) >> 0x8u] + registers[(opcode & 0x00F0u) >> 0x4u];
	if (overflow > 0xFFu) {
		registers[0xFu] = 1;
	}
	else { registers[0xFu] = 0; }
	registers[(opcode & 0x0F00u) >> 0x8u] += registers[(opcode & 0x00F0u) >> 0x4u];
}

void Chip8::op8xy5() { //SUB Vx, Vy
	if (registers[(opcode & 0x0F00u) >> 0x8u] > registers[(opcode & 0x00F0u) >> 0x4u]) {
		registers[0xFu] = 1;
	}
	else { registers[0xFu] = 0; }
	registers[(opcode & 0x0F00u) >> 0x8u] -= registers[(opcode & 0x00F0u) >> 0x4u];
}

void Chip8::op8xy6() { //SHR Vx, {,Vy}
	registers[0xFu] = (registers[(opcode & 0x0F00u) >> 0x8u] & 0x0001u); //least sig bit in Vf
	registers[(opcode & 0x0F00u) >> 0x8u] >>= 1;
}

void Chip8::op8xy7() { //SUBN Vx, Vy
	if (registers[(opcode & 0x00F0u) >> 0x4u] > registers[(opcode & 0x0F00u) >> 0x8u]) {
		registers[0xFu] = 1;
	}
	else { registers[0xFu] = 0; }
	registers[(opcode & 0x0F00u) >> 0x8u] -= registers[(opcode & 0x00F0u) >> 0x4u];
}

void Chip8::op8xyE() { //SHL Vx {, Vy}
	registers[0xFu] = (registers[(opcode & 0x0F00u) >> 0x8u] >> 0x7u); //least sig bit in Vf
	registers[(opcode & 0x0F00u) >> 0x8u] <<= 1;
}

void Chip8::op9xy0() { //SNE Vx, Vy
	if (registers[(opcode & 0x00F0u) >> 0x4u] != registers[(opcode & 0x0F00u) >> 0x8u]) {
		pc += 2;
	}
}

void Chip8::opAnnn() { //LD I, addr
	index = (opcode & 0x0FFFu);
}

void Chip8::opBnnn() { //JP V0, addr
	pc = registers[0x0u] + (opcode & 0x0FFFu);
}

void Chip8::opCxkk() { //RND Vx, byte
	registers[(opcode & 0x0F00u) >> 0x8u] = (rand() % 0x100) & (opcode & 0x00FF);
}

void Chip8::opDxyn() { //DRW Vx, Vy, nibble
	uint8_t x = registers[(opcode & 0x0F00u) >> 0x8u];
	uint8_t y = registers[(opcode & 0x00F0u) >> 0x4u];
	uint8_t rows = (opcode & 0x000Fu);
	//wrap
	uint8_t xpos = x % SCREEN_WIDTH;
	uint8_t ypos = y % SCREEN_HEIGHT;
	registers[0xFu] = 0;

	for (unsigned int row = 0; row < rows; row++) {
		uint8_t currentByte = memory[index + row];
		for (unsigned int col = 0; col < 8; col++) { //sprites HAVE to be 8 pixels wide
			uint8_t pixel = currentByte & (0x80u >> col);
			//draw pixel
			if (pixel) {
				//check if pixel is colliding
				if (pixels[(ypos + row) * SCREEN_WIDTH + (xpos + col)] == 0xFFFFFFFF) {
					registers[0xFu] = 1;
				}
				//XOR with any possible pixel inside
				pixels[(ypos + row) * SCREEN_WIDTH + (xpos + col)] ^= 0xFFFFFFFF;
			}
		}
	}
}

void Chip8::opEx9E() { //SKP Vx
	if (keys[registers[(opcode & 0x0F00u) >> 0x8u]]) {
		pc += 2;
	}
}

void Chip8::opExA1() { //SKNP Vx
	if (!keys[registers[(opcode & 0x0F00u) >> 0x8u]]) {
		pc += 2;
	}
}

void Chip8::opFx07() { //LD Vx, DT
	registers[(opcode & 0x0F00u) >> 0x8u] = delayTimer;
}

void Chip8::opFx0A() { //LD Vx, K
	for (int i = 0; i < 16; i++) {
		if (keys[i]) {
			registers[(opcode & 0x0F00u) >> 0x8u] = i;
			break;
		}
		else {
			pc -= 2; //repeat instruction until a keypress is found
		}
	}
}

void Chip8::opFx15() { //LD DT, Vx
	delayTimer = registers[(opcode & 0x0F00u) >> 0x8u];
}

void Chip8::opFx18() { //LD ST, Vx
	soundTimer = registers[(opcode & 0x0F00u) >> 0x8u];
}

void Chip8::opFx1E() { //ADD I, Vx
	index += registers[(opcode & 0x0F00u) >> 0x8u];
}

void Chip8::opFx29() { //LD F, Vx
	index = 0x50 + registers[(opcode & 0x0F00u) >> 0x8u] * 5;
}

void Chip8::opFx33() { //LD B, Vx
	uint8_t value = registers[(opcode & 0x0F00u) >> 0x8u];
	//ones
	memory[index + 2] = value % 10;
	value /= 10;
	//tens
	memory[index + 1] = value % 10;
	value /= 10;
	//hundreds
	memory[index] = value % 10;
}

void Chip8::opFx55() { //LD [I], Vx
	for (int i = 0; i <= ((opcode & 0x0F00u) >> 0x8u); i++) {
		memory[index + i] = registers[i];
	}
}

void Chip8::opFx65() { //LD Vx, [I]
	for (int i = 0; i <= ((opcode & 0x0F00u) >> 0x8u); i++) {
		registers[i] = memory[index + i];
	}
}