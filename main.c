#include "SDL.h"
#include "stdio.h"
#include "stdbool.h"
#include "stdint.h"
#include "windows.h"

#include "chip8.h"

#define WINDOW_WIDTH 64
#define WINDOW_HEIGHT 32

#define CHIP8_HZ 500
#define CHIP8_TIMERS_HZ 60

#define CHIP8_US_BETWEEN_CYCLES ((uint64_t)(1000000 / (CHIP8_HZ)))
#define CHIP8_US_BETWEEN_TIMER_INTERRUPTS ((uint64_t)(1000000 / (CHIP8_TIMERS_HZ)))

SDL_Renderer *renderer;
SDL_Window *window;
SDL_Event event;

LARGE_INTEGER performanceCounterFrequency;

static inline uint64_t GetUsTime(void)
{
  LARGE_INTEGER li;
  QueryPerformanceCounter(&li);
  /* Multiply this instead of dividing performanceCounterFrequencyuency as not to lose precision. */
  li.QuadPart *= 1000000;
  return (uint64_t)(li.QuadPart / performanceCounterFrequency.QuadPart);
}

bool InitSDL(void)
{
  if (SDL_Init(SDL_INIT_VIDEO) != 0) return false;
  if (SDL_CreateWindowAndRenderer(WINDOW_WIDTH*4, WINDOW_HEIGHT*4, 0, &window, &renderer) != 0) return false;
  if (SDL_RenderSetLogicalSize(renderer, WINDOW_WIDTH, WINDOW_HEIGHT) != 0) false;

  return true;
}

int main(int argc, char *argv[])
{
  bool running = true;
  uint64_t currentTime;
  uint64_t lastCycleTime;
  uint64_t lastTimerInterruptTime;

  if (argc != 2) {
    printf("Wrong number of arguments\n");
    return -1;
  }

  char *romPath = argv[1];

  Chip8Initialize();

  if (!Chip8LoadROM(romPath)) return -1;

  if (!InitSDL()) {
    printf("SDL initialization failed.\n");
    return -1;
  }

  /* Fetch the performanceCounterFrequencyuency for QueryPerformanceCounter. */
  QueryPerformanceFrequency(&performanceCounterFrequency);

  lastCycleTime = currentTime = lastTimerInterruptTime = GetUsTime();

  while (running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
        running = false;
      }
      else if (event.type == SDL_KEYDOWN) {
        switch( event.key.keysym.sym ) {
          /* First ROW */
          case SDLK_1:
            Chip8KeyDown(0x1);
            break;

          case SDLK_2:
            Chip8KeyDown(0x2);
            break;

          case SDLK_3:
            Chip8KeyDown(0x3);
            break;

          case SDLK_4:
            Chip8KeyDown(0xC);
            break;

          /* Second ROW */
          case SDLK_q:
            Chip8KeyDown(0x4);
            break;

          case SDLK_w:
            Chip8KeyDown(0x5);
            break;

          case SDLK_e:
            Chip8KeyDown(0x6);
            break;

          case SDLK_r:
            Chip8KeyDown(0xD);
            break;

          /* Third ROW */
          case SDLK_a:
            Chip8KeyDown(0x7);
            break;

          case SDLK_s:
            Chip8KeyDown(0x8);
            break;

          case SDLK_d:
            Chip8KeyDown(0x9);
            break;

          case SDLK_f:
            Chip8KeyDown(0xE);
            break;

          /* Fourth ROW */
          case SDLK_z:
            Chip8KeyDown(0xA);
            break;

          case SDLK_x:
            Chip8KeyDown(0x0);
            break;

          case SDLK_c:
            Chip8KeyDown(0xB);
            break;

          case SDLK_v:
            Chip8KeyDown(0xF);
            break;
        }
      }
      else if (event.type == SDL_KEYUP) {
        switch (event.key.keysym.sym) {
          /* First ROW */
          case SDLK_1:
            Chip8KeyUp(0x1);
            break;

          case SDLK_2:
            Chip8KeyUp(0x2);
            break;

          case SDLK_3:
            Chip8KeyUp(0x3);
            break;

          case SDLK_4:
            Chip8KeyUp(0xC);
            break;

          /* Second ROW */
          case SDLK_q:
            Chip8KeyUp(0x4);
            break;

          case SDLK_w:
            Chip8KeyUp(0x5);
            break;

          case SDLK_e:
            Chip8KeyUp(0x6);
            break;

          case SDLK_r:
            Chip8KeyUp(0xD);
            break;

          /* Third ROW */
          case SDLK_a:
            Chip8KeyUp(0x7);
            break;

          case SDLK_s:
            Chip8KeyUp(0x8);
            break;

          case SDLK_d:
            Chip8KeyUp(0x9);
            break;

          case SDLK_f:
            Chip8KeyUp(0xE);
            break;

          /* Fourth ROW */
          case SDLK_z:
            Chip8KeyUp(0xA);
            break;

          case SDLK_x:
            Chip8KeyUp(0x0);
            break;

          case SDLK_c:
            Chip8KeyUp(0xB);
            break;

          case SDLK_v:
            Chip8KeyUp(0xF);
            break;
        }
      }
    }

    currentTime = GetUsTime();

    if ((currentTime - lastCycleTime) >= CHIP8_US_BETWEEN_CYCLES) {
      Chip8Cycle();
      Chip8Draw(renderer);
      lastCycleTime = currentTime;
    }

    if ((currentTime - lastTimerInterruptTime) >= CHIP8_US_BETWEEN_TIMER_INTERRUPTS) {
      Chip8CycleTimerInterrupts();
      lastTimerInterruptTime = currentTime;
    }

#ifdef STEPPING
    getchar();
#endif
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
