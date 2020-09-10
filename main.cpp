#include "Chip8.h"
#include <iostream>
#include <SDL.h>
#include <chrono>
#include <SDL_ttf.h>

//SDL vars
const int SCREEN_WIDTH = 64;
const int SCREEN_HEIGHT = 32;
const int SCALE = 20;
const int CLOCKSPEED = 3;
bool quit = false;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* texture = NULL;

void initSDL() {
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("JAC8e", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH*SCALE, SCREEN_HEIGHT*SCALE, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
}

void cleanupSDL() {
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

int main(int argc, char* args[]) {
	Chip8 chip8;
	chip8.init();
	chip8.loadROM("ROMs/pong.ch8");
	initSDL();
	SDL_Event e;

	auto pTime = std::chrono::high_resolution_clock::now();
	while (!quit) {

		//process input
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT) {
				quit = true;
			}
			if (e.type == SDL_KEYDOWN) {
				switch (e.key.keysym.sym) {	
				case SDLK_1: chip8.keys[0x1] = 1; break;		
				case SDLK_2: chip8.keys[0x2] = 1; break;
				case SDLK_3: chip8.keys[0x3] = 1; break;
				case SDLK_4: chip8.keys[0xC] = 1; break;
				case SDLK_q: chip8.keys[0x4] = 1; break;
				case SDLK_w: chip8.keys[0x5] = 1; break;
				case SDLK_e: chip8.keys[0x6] = 1; break;
				case SDLK_r: chip8.keys[0xD] = 1; break;
				case SDLK_a: chip8.keys[0x7] = 1; break;
				case SDLK_s: chip8.keys[0x8] = 1; break;
				case SDLK_d: chip8.keys[0x9] = 1; break;
				case SDLK_f: chip8.keys[0xE] = 1; break;
				case SDLK_z: chip8.keys[0xA] = 1; break;
				case SDLK_x: chip8.keys[0x0] = 1; break;
				case SDLK_c: chip8.keys[0xB] = 1; break;
				case SDLK_v: chip8.keys[0xF] = 1; break;
				}
			}
			if (e.type == SDL_KEYUP) {
				switch (e.key.keysym.sym) {
				case SDLK_1: chip8.keys[0x1] = 0; break;
				case SDLK_2: chip8.keys[0x2] = 0; break;
				case SDLK_3: chip8.keys[0x3] = 0; break;
				case SDLK_4: chip8.keys[0xC] = 0; break;
				case SDLK_q: chip8.keys[0x4] = 0; break;
				case SDLK_w: chip8.keys[0x5] = 0; break;
				case SDLK_e: chip8.keys[0x6] = 0; break;
				case SDLK_r: chip8.keys[0xD] = 0; break;
				case SDLK_a: chip8.keys[0x7] = 0; break;
				case SDLK_s: chip8.keys[0x8] = 0; break;
				case SDLK_d: chip8.keys[0x9] = 0; break;
				case SDLK_f: chip8.keys[0xE] = 0; break;
				case SDLK_z: chip8.keys[0xA] = 0; break;
				case SDLK_x: chip8.keys[0x0] = 0; break;
				case SDLK_c: chip8.keys[0xB] = 0; break;
				case SDLK_v: chip8.keys[0xF] = 0; break;
				}
			}
		}

		//cycle and render
		auto time = std::chrono::high_resolution_clock::now();
		float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(time - pTime).count();
		if (dt > CLOCKSPEED) {
			pTime = time;
			chip8.cycle();
			SDL_UpdateTexture(texture, NULL, chip8.pixels, sizeof(chip8.pixels[0]) * SCREEN_WIDTH);
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, texture, NULL, NULL);
			SDL_RenderPresent(renderer);
		}
	}

	cleanupSDL();
	return 0;
}