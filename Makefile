# Compiler and linker definitions
CC = gcc
SOURCE = $(wildcard ./source/*.c)
INCLUDE_DIRS = -I./SDL2/include
LIB_DIRS = -L./SDL2/lib
SDL2_LIBS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_net -lSDL2_ttf

# Specify building directory
BUILD_DIR = build

# Default target
all: $(BUILD_DIR)/main

# Build target for app
$(BUILD_DIR)/main: $(SOURCE) | $(BUILD_DIR)
	$(CC) $(INCLUDE_DIRS) $(LIB_DIRS) -o $@ $^ $(SDL2_LIBS)

# Clean up target
clean:
ifeq ($(OS),Windows_NT)
	del $(BUILD_DIR)\*.exe
else
	rm -f $(BUILD_DIR)/*.exe
endif