OBJS = main.c
OBJS += chip8.c

CC = gcc

INCLUDE_PATHS = -ISDL2\x64\include\SDL2

LIBRARY_PATHS = -LSDL2\x64\lib

COMPILER_FLAGS = -Wall -Wextra -Wimplicit-fallthrough -O2 -flto

LINKER_FLAGS = -lmingw32 -lSDL2main -lSDL2

OBJ_NAME = chip8

all : $(OBJS)
	$(CC) $(OBJS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)

stepping : $(OBJS)
	$(CC) $(OBJS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) -DSTEPPING $(LINKER_FLAGS) -o $(OBJ_NAME)

clean :
	del main.exe