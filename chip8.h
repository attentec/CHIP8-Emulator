#ifndef CHIP8_H
#define CHIP8_H

/*
 * References:
 * http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#2.4
 * https://en.wikipedia.org/wiki/CHIP-8
*/

#include "SDL.h"
#include "stdbool.h"

extern void Chip8Initialize(void);
extern bool Chip8LoadROM(const char * const romPath);
extern void Chip8Cycle(void);
extern void Chip8CycleTimerInterrupts();
extern void Chip8Draw(SDL_Renderer * const renderer);
extern void Chip8KeyDown(const unsigned char key);
extern void Chip8KeyUp(const unsigned char key);

#endif /* CHIP8_H */