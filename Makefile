CC = gcc
FLAGS = -ggdb -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast -fno-dwarf2-cfi-asm -I/usr/include/libdwarf/ -L/usr/lib -l elf -l dwarf -std=c99 -lunwind
FILES = test.c read_types.c dwarf_reader.c

# -gsplit-dwarf

all: test

test: $(FILES)
	$(CC) $(FILES) $(FLAGS) -o test


clean:
	rm test
