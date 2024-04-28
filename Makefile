
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