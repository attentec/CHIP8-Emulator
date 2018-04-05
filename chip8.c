#include "chip8.h"
#include "stdio.h"
#include "stdlib.h"
#include "time.h"

bool shouldDraw = false;

bool isWaitingForKey = false;
unsigned char isWaitingForKeyRegister;

unsigned char fontset[80] =
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

unsigned char memory[4096U];

unsigned char V[16U];

unsigned char screen[32][64U];

unsigned short stack[16U];
unsigned short sp;

unsigned char keys[16U];

unsigned char delayTimer;
unsigned char soundTimer;

unsigned short I;
unsigned short pc;

void Chip8Initialize()
{
  for (int i = 0; i < 4096; ++i) {
    memory[i] = 0U;
  }

  for (int i = 0; i < 80; ++i) {
    memory[i] = fontset[i];
  }

  for (int i = 0; i < 16; ++i) {
    keys[i] = 0U;
    V[i] = 0U;
  }

  sp = 0;
  pc = 0x200;
  I = 0;

  delayTimer = 0U;
  soundTimer = 0U;

  srand((unsigned int)time(NULL));
}

bool Chip8LoadROM(const char * const romPath)
{
  FILE *f;

  f = fopen(romPath, "rb");

  if (f == NULL) {
    printf("Can't open file %s", romPath);
    return false;
  }

  fseek(f, 0L, SEEK_END);
  long size = ftell(f);

  rewind(f);

  unsigned char *data = malloc(sizeof(unsigned char) * size);

  fread(data, 1, size, f);

  for (int i = 0; i < size; ++i) {
    memory[i + 0x200] = data[i];
  }

  printf("Loaded %s into memory, %ld bytes loaded.\n", romPath, size);

  fclose(f);
  free(data);

  return true;
}

void Chip8Cycle()
{
  unsigned short opcode = (memory[pc] << 8) | memory[pc + 1];

  /* 0x0NNN */
  unsigned short NNN = opcode & 0x0FFF;
  /* 0x00NN */
  unsigned short NN  = opcode & 0x00FF;
  /* 0x000N */
  unsigned short N   = opcode & 0x000F;
  /* 0x0X00 */
  unsigned char  X   = (opcode & 0x0F00) >> 8;
  /* 0x00Y0 */
  unsigned char  Y   = (opcode & 0x00F0) >> 4;

#ifdef STEPPING
  system("cls");
  printf("opcode: %04x\n", opcode);
  printf(" V          S\n");
  for (int i = 0; i < 16; ++i) {
    printf("%2x: %3x    %2x: %3x\n", i, V[i], i, stack[i]);
  }
  printf("\nPC: %x\nSP: %x\nI: %x\n", pc, sp, I);
  printf("\nDT: %d\nST: %d\n", delayTimer, soundTimer);
#endif

  if (isWaitingForKey) {
    return;
  }

  switch (opcode & 0xF000) {

    case 0x0000:
    {
      switch(opcode & 0x0FFF) {

        /* 00E0: disp_clear() Clears the screen. */
        case 0x00E0:
        {
          for (int y = 0; y < 32; ++y) {
            for (int x = 0; x < 64; ++x) {
              screen[y][x] = 0;
            }
          }
          shouldDraw = true;
          pc += 2;
        }
        break;

        /* 00EE: Returns from a subroutine. */
        case 0x00EE:
        {
          --sp;
          pc = stack[sp];
        }
        break;

        default:
        {
          printf("Unkown opcode: 0x%04X\n", opcode);
        }
      }
    }
    break;

    /* 1NNN: goto NNN; Jumps to address NNN. */
    case 0x1000:
    {
      pc = NNN;
    }
    break;

    /* 2NNN: *(0xNNN)() Calls subroutine at NNN. */
    case 0x2000:
    {
      stack[sp] = pc + 2U;
      ++sp;
      pc = NNN;
    }
    break;

    /* 3XNN: if(Vx==NN)  Skips the next instruction if VX equals NN. */
    case 0x3000:
    {
      if (V[X] == NN) {
        pc += 2;
      }
      pc += 2;
    }
    break;

    /* 4XNN: if(Vx!=NN)  Skips the next instruction if VX doesn't equal NN. */
    case 0x4000:
    {
      if (V[X] != (NN)) {
        pc += 2;
      }
      pc += 2;
    }
    break;

    /* 6XNN: Vx = NN */
    case 0x6000:
    {
      V[X] = NN;
      pc += 2;
    }
    break;

    /* 7XNN: Vx += NN  Adds NN to VX. (Carry flag is not changed) */
    case 0x7000:
    {
      V[X] += NN;
      pc += 2;
    }
    break;

    case 0x8000:
    {
      switch (opcode & 0x000F) {

        /* 8XY0:  Vx=Vy Sets VX to the value of VY. */
        case 0x0000:
        {
          V[X] = V[Y];
          pc += 2;
        }
        break;

        /* 8XY1 BitOp Vx=Vx|Vy  Sets VX to VX or VY. (Bitwise OR operation) */
        case 0x0001:
        {
          V[X] |= V[Y];
          pc += 2;
        }
        break;

        /* 8XY2: Vx=Vx&Vy  Sets VX to VX and VY. (Bitwise AND operation) */
        case 0x0002:
        {
          V[X] &= V[Y];
          pc += 2;
        }
        break;

        /* 8XY3: Vx=Vx^Vy  Sets VX to VX xor VY. */
        case 0x0003:
        {
          V[X] ^= V[Y];
          pc += 2;
        }
        break;

        /* 8XY4: Vx += Vy  Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't. */
        case 0x0004:
        {
          if ((unsigned short)V[X] + (unsigned short)V[Y] > 255U) {
            V[0xF] = 1U;
          }
          else {
            V[0xF] = 0U;
          }
          V[X] += V[Y];

          pc += 2;
        }
        break;

        /* 8XY5 Math  Vx -= Vy  VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't. */
        case 0x0005:
        {
          if (V[Y] > V[X]) {
            V[0xF] = 0U;
          }
          else {
            V[0xF] = 1U;
          }
          V[X] -= V[Y];
          pc += 2;
        }
        break;

        /* 8XY6: If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0. Then Vx is divided by 2. */
        case 0x0006:
        {
          V[0xF] = V[X] & 0x1;
          V[X] = V[X] >> 1;
          pc += 2;
        }
        break;

        /* 8XY7: Vx=Vy-Vx  Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't. */
        case 0x0007:
        {
          if (V[X] > V[Y]) {
            V[0xF] = 0U;
          }
          else {
            V[0xF] = 1U;
          }
          V[X] = V[Y] - V[X];
          pc += 2;
        }
        break;

        /* 8XYE: If the most-significant bit of Vx is 1, then VF is set to 1, otherwise to 0. Then Vx is multiplied by 2. */
        case 0x000E:
        {
          V[0xF] = V[X] >> 7;
          V[X] = V[X] << 1;
          pc += 2;
        }
        break;

        default:
        {
          printf("Unkown opcode: 0x%04X\n", opcode);
        }
      }
    }
    break;

    /* ANNN: I = NNN */
    case 0xA000:
    {
      I = NNN;
      pc += 2;
    }
    break;

    /* CXNN: Vx=rand()&NN  Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN. */
    case 0xC000:
    {
      V[X] = (rand() % 256) & NN;
      pc += 2;
    }
    break;

    /* DXYN: draw(Vx,Vy,N) */
    case 0xD000:
    {
      unsigned char xStart = V[X];
      unsigned char yStart = V[Y];
      unsigned char height = N;

      V[0xF] = 0U;

      for (int y = 0; y < height; ++y) {
        unsigned char pixel = memory[I + y];

        for (int x = 0; x < 8; ++x) {

          /* Check if bit will be flipped from 1 to 0. */
          if (screen[yStart + y][xStart + x] == 1 && ((pixel >> (7 - x)) & 0x1) == 1U) {
            V[0xF] = 1U;
          }

          /* Update pixel. */
          screen[yStart + y][xStart + x] ^= ((pixel >> (7 - x)) & 0x1);
        }
      }

      shouldDraw = true;
      pc += 2;
    }
    break;

    case 0xE000:
    {
      switch (opcode & 0x00FF) {

        /* EX9E: if(key()==Vx) Skips the next instruction if the key stored in VX is pressed. */
        case 0x009E:
        {
          if (keys[V[X]] == 1) {
            pc += 2;
          }
          pc += 2;
        }
        break;

        /* EXA1: if(key()!=Vx) Skips the next instruction if the key stored in VX isn't pressed. */
        case 0x00A1:
        {
          if (keys[V[X]] == 0) {
            pc += 2;
          }
          pc += 2;
        }
        break;

        default:
        {
          printf("Unkown opcode: 0x%04X\n", opcode);
        }
      }
    }
    break;

    case 0xF000:
    {
      switch (opcode & 0x00FF) {

        /* FX07: Vx = get_delay()  Sets VX to the value of the delay timer. */
        case 0x0007:
        {
          V[X] = delayTimer;
          pc += 2;
        }
        break;

        /* FX0A: Vx = get_key()  A key press is awaited, and then stored in VX. (Blocking Operation. All instruction halted until next key event) */
        case 0x000A:
        {
          isWaitingForKey = true;
          isWaitingForKeyRegister = X;
          pc += 2;
        }
        break;

        /* FX15: delay_timer(Vx) Sets the delay timer to VX. */
        case 0x0015:
        {
          delayTimer = V[X];
          pc += 2;
        }
        break;

        /* FX18: sound_timer(Vx) Sets the sound timer to VX. */
        case 0x0018:
        {
          soundTimer = V[X];
          pc += 2;
        }
        break;

        /* FX1E: I +=Vx  Adds VX to I. */
        case 0x001E:
        {
          I += V[X];
          pc += 2;
        }
        break;

        /* FX29: I=sprite_addr[Vx] Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font. */
        case 0x0029:
        {
          /* Characters are place in address 0-80 and are 5 bytes long, therefore we can multiply the character in Vx with 5 to get the correct position. */
          I = V[X] * 5U;
          pc += 2;
        }
        break;

        /* FX33: Stores the binary-coded decimal representation of VX, with the most significant
         * of three digits at the address in I, the middle digit at I plus 1, and the least significant
         * digit at I plus 2.
         */
        case 0x0033:
        {
          unsigned char digit = V[X];

          memory[I] = digit / 100U;
          memory[I + 1] = (digit / 10U) % 10U;
          memory[I + 2] = digit % 10U;
          pc += 2;
        }
        break;

        /* FX55: reg_dump(Vx,&I) Stores V0 to VX (including VX) in memory starting at address I. I is increased by 1 for each value written. */
        case 0x0055:
        {
          for (int i = 0; i <= X; ++i) {
            memory[I] = V[i];
            ++I;
          }
          pc += 2;
        }
        break;

        /* FX65: reg_load(Vx,&I) Fills V0 to VX (including VX) with values from memory starting at address I. I is increased by 1 for each value written. */
        case 0x0065:
        {
          for (int v = 0; v <= X; ++v) {
            V[v] = memory[I];
            ++I;
          }
          pc += 2;
        }
        break;

        default:
        {
          printf("Unkown opcode: 0x%04X\n", opcode);
        }
      }
    }
    break;

    default:
    {
      printf("Unkown opcode: 0x%04X\n", opcode);
    }
  }
}

void Chip8CycleTimerInterrupts()
{
  if (delayTimer > 0) --delayTimer;
  if (soundTimer > 0) --soundTimer;
}

void Chip8Draw(SDL_Renderer * const renderer)
{
  if (shouldDraw)
  {
    /* Set draw color to black. */
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);

    /* Clear screen with black. */
    SDL_RenderClear(renderer);

    /* Set draw color to white. */
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    for (int y = 0; y < 32; ++y) {
      for (int x = 0; x < 64; ++x) {
        if (screen[y][x] == 1) {
          SDL_RenderDrawPoint(renderer, x, y);
        }
      }
    }

    SDL_RenderPresent(renderer);

    shouldDraw = false;
  }
}

void Chip8KeyDown(const unsigned char key)
{
  keys[key] = 1U;
  if (isWaitingForKey) {
    V[isWaitingForKeyRegister] = key;
    isWaitingForKey = false;
  }
}

void Chip8KeyUp(const unsigned char key)
{
  keys[key] = 0U;
}