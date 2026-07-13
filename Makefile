.PHONY: build clean run

# AArch64 bare-metal cross compiler (Raspberry Pi 4B)
CC = aarch64-none-elf-gcc
OBJCOPY = aarch64-none-elf-objcopy

CPU = cortex-a72

CFLAGS= -mcpu=$(CPU) -fno-pic -ffreestanding
CSRCFLAGS= -O2 -Wall -Wextra
LFLAGS= -ffreestanding -O2 -nostdlib

# Location of the files
KER_SRC = src/kernel
KER_HEAD = include
COMMON_SRC = src/common
OBJ_DIR = build/objects
KERSOURCES = $(wildcard $(KER_SRC)/*.c)
COMMONSOURCES = $(wildcard $(COMMON_SRC)/*.c)
ASMSOURCES = $(wildcard $(KER_SRC)/*.S)
OBJECTS = $(patsubst $(KER_SRC)/%.c, $(OBJ_DIR)/%.o, $(KERSOURCES))
OBJECTS += $(patsubst $(COMMON_SRC)/%.c, $(OBJ_DIR)/%.o, $(COMMONSOURCES))
OBJECTS += $(patsubst $(KER_SRC)/%.S, $(OBJ_DIR)/%.o, $(ASMSOURCES))
HEADERS = $(wildcard $(KER_HEAD)/*.h)

ELF_NAME = kernel8.elf
IMG_NAME = kernel8.img

build: $(OBJECTS) $(HEADERS)
	$(CC) -T linker.ld -o $(ELF_NAME) $(LFLAGS) $(OBJECTS)
	$(OBJCOPY) $(ELF_NAME) -O binary $(IMG_NAME)

$(OBJ_DIR)/%.o: $(KER_SRC)/%.c $(HEADERS)
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -I$(KER_SRC) -I$(KER_HEAD) -c $< -o $@ $(CSRCFLAGS)

$(OBJ_DIR)/%.o: $(KER_SRC)/%.S
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -I$(KER_SRC) -c $< -o $@

$(OBJ_DIR)/%.o: $(COMMON_SRC)/%.c $(HEADERS)
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -I$(KER_SRC) -I$(KER_HEAD) -c $< -o $@ $(CSRCFLAGS)

clean:
	rm -rf $(OBJ_DIR)
	rm -f $(ELF_NAME) $(IMG_NAME)

# QEMUにはELFを渡す（アドレス情報が含まれるため）
run: build
	qemu-system-aarch64 -m 2048 -M raspi4b -nographic -kernel $(ELF_NAME)
