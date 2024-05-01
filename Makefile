
SRC_DIR := neonc

EXEC := neon_0t

CC := clang++
CCFLAGS := \
	-Wextra

SOURCES := $(shell find $(SRC_DIR) -name "*.cpp")

all: build

build:
	$(CC) $(CCFLAGS) $(SOURCES) -o $(EXEC)

test:
	@./$(EXEC)

assemble:
	as std/mem.s -o mem.o
	as .neon_tmp/mod_n0.s -o modn0.o
	ld modn0.o mem.o -o modn0