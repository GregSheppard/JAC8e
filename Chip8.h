#pragma once
#include <cstdint>

class Chip8 {
public:
	uint8_t registers[16]{};
	uint8_t memory[4096]{};
	uint16_t index;
	uint16_t pc;
	uint16_t stack[16]{};
	uint8_t sp;
	uint8_t delayTimer;
	uint8_t soundTimer;
	uint8_t keys[16]{};
	uint32_t pixels[64 * 32]{};
	uint16_t opcode;

	Chip8();

	void loadROM(char const* filename);
	void dumpMemory();
	void init();
	void cycle();

	/*
	##########################################
	###############OP CODES###################
	##########################################
	*/

private:
	void op00E0(); //CLS
	void op00EE(); //RET
	void op1nnn(); //JP addr
	void op2nnn(); //CALL addr
	void op3xkk(); //SE Vx, byte
	void op4xkk(); //SNE Vx, byte
	void op5xy0(); //SE Vx, Vy
	void op6xkk(); //LD Vx, byte
	void op7xkk(); //ADD Vx, byte
	void op8xy0(); //LD Vx, Vy
	void op8xy1(); //OR Vx, Vy
	void op8xy2(); //AND Vx, Vy
	void op8xy3(); ///XOR Vx, Vy
	void op8xy4(); //ADD Vx, Vy
	void op8xy5(); //SUB Vx, Vy
	void op8xy6(); //SHR Vx, {, Vy}
	void op8xy7(); //SUBN Vx, Vy
	void op8xyE(); //SHL Vx {, Vy}
	void op9xy0(); //SNE Vx, Vy
	void opAnnn(); //LD I, addr
	void opBnnn(); //JP V0, addr
	void opCxkk(); //RND Vx, byte
	void opDxyn(); //DRW Vx, Vy, nibble
	void opEx9E(); //SKP Vx
	void opExA1(); //SKNP Vx
	void opFx07(); //LD Vx, DT
	void opFx0A(); //LD Vx, K
	void opFx15(); //LD DT, Vx
	void opFx18(); //LD ST, Vx
	void opFx1E(); //ADD I, Vx
	void opFx29(); //LD F, Vx
	void opFx33(); //LD B, Vx
	void opFx55(); //LD [I], Vx
	void opFx65(); //LD Vx, [I]

};

