# CHIP8 Emulator
A simple CHIP-8 emulator written in C for Windows.
The emulator is using [QueryPerformanceCounter](https://msdn.microsoft.com/en-us/library/windows/desktop/ms644904%28v=vs.85%29.aspx) to emulate accurate clock frequencies.
The clock frequencies can be edited in **main.c** by altering  the defines `CHIP8_HZ` and `CHIP8_TIMERS_HZ`.
The repository also includes some public roms in the ROMS directory.

# Compiling and Running
This repository includes SDL2 so the only thing needed to compile is gcc and preferably make.
**Compile:**

    make all

**Run:**

    chip8.exe <ROM file>

**Using:**
The CHIP-8 uses a 16-key hexadecimal keypad with the following layout:

    _________
    |1|2|3|C|
    |4|5|6|D|
    |7|8|9|E|
    |A|0|B|F|
    ---------

The emulator uses the following keyboard layout to match this:

    _________
    |1|2|3|4|
    |Q|W|E|R|
    |A|S|D|F|
    |Z|X|C|V|
    ---------
