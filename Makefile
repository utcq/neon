
SRC_DIR := neonc

EXEC := neon_0t

CC := clang++
CCFLAGS := \
	-Wextra \
	-Wno-missing-field-initializers

SOURCES := $(shell find $(SRC_DIR) -name "*.cpp")

ASMES := $(shell find .neon_tmp/ -name "*.s")

all: build

build:
	$(CC) $(CCFLAGS) $(SOURCES) -o $(EXEC)

test:
	@./$(EXEC)

assemble:
	as $(ASMES) -o tmp.o
	@ld tmp.o -o tmpexec -lc -dynamic-linker /lib64/ld-linux-x86-64.so.2
	@rm tmp.o