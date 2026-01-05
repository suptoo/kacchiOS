# Makefile for kacchiOS - Extended Version
# CSE 3202 Operating Systems Project

CC = gcc
LD = ld
AS = as

CFLAGS = -m32 -ffreestanding -O2 -Wall -Wextra -nostdinc \
         -fno-builtin -fno-stack-protector -fno-pie -I.
ASFLAGS = --32
LDFLAGS = -m elf_i386 -no-pie

# Object files (consolidated: serial+string merged into io.o, types.h is header-only)
OBJS = boot.o kernel.o io.o memory.o process.o scheduler.o

# Default target
all: kernel.elf

# Link all object files into kernel.elf
kernel.elf: $(OBJS)
	$(LD) $(LDFLAGS) -T link.ld -o $@ $^
	@echo "========================================="
	@echo "Build completed successfully!"
	@echo "Run with: make run"
	@echo "========================================="

# Compile C files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile assembly files
%.o: %.S
	$(AS) $(ASFLAGS) $< -o $@

# Run in QEMU (serial output only)
run: kernel.elf
	@echo "Starting kacchiOS in QEMU..."
	@echo "Press Ctrl+A then X to exit QEMU"
	@echo "========================================="
	qemu-system-i386 -kernel kernel.elf -m 64M -serial stdio -display none

# Run in QEMU with VGA window
run-vga: kernel.elf
	@echo "Starting kacchiOS in QEMU with VGA..."
	@echo "Serial output in this terminal"
	@echo "========================================="
	qemu-system-i386 -kernel kernel.elf -m 64M -serial mon:stdio

# Debug mode (wait for GDB)
debug: kernel.elf
	@echo "Starting kacchiOS in debug mode..."
	@echo "Waiting for GDB connection on port 1234"
	@echo "In another terminal run:"
	@echo "  gdb -ex 'target remote localhost:1234' -ex 'symbol-file kernel.elf'"
	@echo "========================================="
	qemu-system-i386 -kernel kernel.elf -m 64M -serial stdio -display none -s -S &

# Clean build artifacts
clean:
	rm -f *.o kernel.elf

# Help target
help:
	@echo "kacchiOS Build System"
	@echo "========================================="
	@echo "Available targets:"
	@echo "  make          - Build kernel.elf"
	@echo "  make run      - Run in QEMU (serial only)"
	@echo "  make run-vga  - Run in QEMU (with VGA)"
	@echo "  make debug    - Run in debug mode (GDB ready)"
	@echo "  make clean    - Remove build artifacts"
	@echo "  make help     - Show this help"
	@echo "========================================="

.PHONY: all run run-vga debug clean help
