# VibeOS - A Modern Hobby Operating System
# Build system for x86 (i686) architecture

# Toolchain
CROSS_COMPILE = i686-elf-
CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)as
LD = $(CROSS_COMPILE)ld
NASM = nasm
QEMU = qemu-system-i386
OBJCOPY = $(CROSS_COMPILE)objcopy

# Architecture & Platform
ARCH = i686
TARGET = vibos

# Directories
BUILD_DIR = build
KERNEL_DIR = kernel
BOOT_DIR = boot
DRIVERS_DIR = drivers
INCLUDE_DIR = include
SCRIPTS_DIR = scripts

# Kernel C sources
KERNEL_SOURCES = \
    $(KERNEL_DIR)/kernel.c \
    $(KERNEL_DIR)/console.c \
    $(KERNEL_DIR)/mm/pmm.c \
    $(KERNEL_DIR)/mm/vmm.c \
    $(KERNEL_DIR)/mm/heap.c \
    $(KERNEL_DIR)/fs/vfs.c \
    $(KERNEL_DIR)/fs/ramfs.c \
    $(KERNEL_DIR)/fs/procfs.c \
    $(KERNEL_DIR)/fs/vibefs.c \
    $(KERNEL_DIR)/net/net.c \
    $(KERNEL_DIR)/net/eth.c \
    $(KERNEL_DIR)/net/arp.c \
    $(KERNEL_DIR)/net/ip.c \
    $(KERNEL_DIR)/net/icmp.c \
    $(KERNEL_DIR)/net/udp.c \
    $(KERNEL_DIR)/net/tcp.c \
    $(KERNEL_DIR)/net/dhcp.c \
    $(KERNEL_DIR)/net/dns.c \
    $(KERNEL_DIR)/net/netif.c \
    $(KERNEL_DIR)/drivers/pic.c \
    $(KERNEL_DIR)/drivers/pit.c \
    $(KERNEL_DIR)/drivers/ps2.c \
    $(KERNEL_DIR)/drivers/keyboard.c \
    $(KERNEL_DIR)/lib/string.c \
    $(KERNEL_DIR)/lib/printf.c \
    $(KERNEL_DIR)/arch/x86/gdt.c \
    $(KERNEL_DIR)/arch/x86/idt.c \
    $(KERNEL_DIR)/arch/x86/isr.c

# Kernel assembly sources (linked into kernel ELF)
KERNEL_ASM = \
    $(KERNEL_DIR)/arch/x86/entry.asm \
    $(KERNEL_DIR)/arch/x86/isr.asm \
    $(KERNEL_DIR)/arch/x86/isr_common.asm

# Object files
OBJS = $(addprefix $(BUILD_DIR)/,$(KERNEL_SOURCES:.c=.o) $(KERNEL_ASM:.asm=.o))

# Compiler flags
CFLAGS = -m32 -std=gnu11 -ffreestanding -O2 -Wall -Wextra \
         -I$(INCLUDE_DIR) -I$(KERNEL_DIR) \
         -nostdlib -nostdinc -fno-pic -fno-pie \
         -fno-builtin -fno-stack-protector \
         -march=i686 -mtune=generic

ASFLAGS = -f elf32
NASMFLAGS = -f bin

# Linker script and flags
LINKER_SCRIPT = $(KERNEL_DIR)/linker.ld
LDFLAGS = -T $(LINKER_SCRIPT) -m elf_i386 -nostdlib

# Outputs
KERNEL_ELF = $(BUILD_DIR)/$(TARGET).elf
KERNEL_BIN = $(BUILD_DIR)/$(TARGET).bin
BOOTLOADER_BIN = $(BUILD_DIR)/boot.bin
OS_IMG = $(BUILD_DIR)/$(TARGET).img

# Default target
.PHONY: all
all: $(OS_IMG)

# Create build directories
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	mkdir -p $(dir $(OBJS))

# Build bootloader
$(BUILD_DIR)/boot.bin: $(BOOT_DIR)/boot.asm | $(BUILD_DIR)
	$(NASM) $(NASMFLAGS) $< -o $@

$(BUILD_DIR)/stage2.bin: $(BOOT_DIR)/stage2.asm | $(BUILD_DIR)
	$(NASM) $(NASMFLAGS) $< -o $@

# Compile C sources
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble ASM sources (NASM for ELF objects)
$(BUILD_DIR)/%.o: %.asm | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(NASM) -f elf32 $< -o $@

# Link kernel
$(KERNEL_ELF): $(OBJS) $(LINKER_SCRIPT)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
	@echo "Kernel linked: $@"

# Copy kernel binary
$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

# Create floppy image (1.44MB)
$(OS_IMG): $(BOOTLOADER_BIN) $(KERNEL_BIN)
	@echo "Creating floppy image..."
	dd if=/dev/zero of=$@ bs=512 count=2880 2>/dev/null
	dd if=$(BOOTLOADER_BIN) of=$@ conv=notrunc 2>/dev/null
	dd if=$(KERNEL_BIN) of=$@ bs=512 seek=1 conv=notrunc 2>/dev/null
	@echo "OS image created: $@"

# Run in QEMU
.PHONY: run
run: $(OS_IMG)
	$(QEMU) -drive format=raw,file=$(OS_IMG) -m 128M

# Run with debugging
.PHONY: debug
debug: $(OS_IMG)
	$(QEMU) -drive format=raw,file=$(OS_IMG) -m 128M -s -S

# Clean build artifacts
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

# Format code
.PHONY: fmt
fmt:
	find . -name '*.c' -o -name '*.h' -o -name '*.asm' | xargs indent -linux 2>/dev/null || true

# Show project structure
.PHONY: tree
tree:
	@echo "VibeOS Directory Structure:"
	@find . -type f -not -path './build/*' -not -path './.git/*' | sort
