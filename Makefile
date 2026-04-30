# VibeOS Makefile - Minimal working version
.PHONY: all clean debug run

CC = i686-linux-gnu-gcc
LD = i686-linux-gnu-ld
NASM = nasm
QEMU = qemu-system-i386
OBJCOPY = i686-linux-gnu-objcopy

CFLAGS = -m32 -std=gnu11 -ffreestanding -O2 -Wall -Wextra
CFLAGS += -Iinclude -Ikernel
CFLAGS += -nostdlib -nostdinc -fno-pic -fno-pie
CFLAGS += -fno-builtin -fno-stack-protector
CFLAGS += -march=i686 -mtune=generic

BUILD_DIR = build
KERNEL_ELF = $(BUILD_DIR)/kernel.elf
KERNEL_BIN = $(BUILD_DIR)/kernel.bin
LINKER_SCRIPT = linker.ld

# Essential C source files
C_SOURCES = \
    kernel/kernel.c \
    kernel/console.c \
    kernel/lib/string.c \
    kernel/lib/printf.c \
    kernel/mm/mm.c \
    kernel/arch/x86/gdt.c \
    kernel/arch/x86/idt.c \
    kernel/arch/x86/isr.c \
    kernel/arch/x86/isr_handlers.c \
    kernel/drivers/pic.c \
    kernel/drivers/pic_init.c \
    kernel/drivers/pit.c \
    kernel/drivers/ps2.c \
    kernel/drivers/keyboard.c \
    kernel/fs/vfs.c \
    kernel/fs/ramfs.c \
    kernel/fs/procfs.c \
    kernel/fs/vibefs.c \
    kernel/net/net.c \
    kernel/drivers/pci.c \
    kernel/drivers/rtl8139.c \
    kernel/net/netif.c \
    kernel/net/eth.c \
    kernel/net/arp.c \
    kernel/net/ip.c \
    kernel/net/icmp.c \
    kernel/net/udp.c \
    kernel/net/tcp.c \
    kernel/net/dhcp.c \
    kernel/net/dns.c \
    kernel/klog.c \
    kernel/ps.c \
    kernel/shell_math.c \
    kernel/user.c

ASM_SOURCES = \
    kernel/arch/x86/entry.asm \
    kernel/arch/x86/gdt_flush.asm \
    kernel/arch/x86/isr_common.asm

# Generate object file paths
C_OBJS = $(addprefix $(BUILD_DIR)/,$(C_SOURCES:.c=.o))
ASM_OBJS = $(addprefix $(BUILD_DIR)/,$(ASM_SOURCES:.asm=.o))
OBJS = $(ASM_OBJS) $(C_OBJS)

all: $(KERNEL_ELF) $(KERNEL_BIN)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/kernel/mm
	mkdir -p $(BUILD_DIR)/kernel/fs
	mkdir -p $(BUILD_DIR)/kernel/net
	mkdir -p $(BUILD_DIR)/kernel/drivers
	mkdir -p $(BUILD_DIR)/kernel/lib
	mkdir -p $(BUILD_DIR)/kernel/arch/x86

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.asm | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(NASM) -f elf32 $< -o $@

$(KERNEL_ELF): $(OBJS) $(LINKER_SCRIPT)
	$(CC) -m32 -nostdlib -T $(LINKER_SCRIPT) -o $@ $(OBJS) -lgcc

$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

clean:
	rm -rf $(BUILD_DIR)

run: $(KERNEL_ELF)
	$(QEMU) -kernel $(KERNEL_ELF) -m 128M -net nic,model=rtl8139 -net user

debug: $(KERNEL_ELF)
	$(QEMU) -kernel $(KERNEL_ELF) -m 128M -s -S -net nic,model=rtl8139 -net user

iso: $(KERNEL_ELF)
	mkdir -p $(BUILD_DIR)/iso/boot/grub
	cp $(KERNEL_ELF) $(BUILD_DIR)/iso/boot/kernel.elf
	echo 'menuentry "EarlnuxOS" {' > $(BUILD_DIR)/iso/boot/grub/grub.cfg
	echo '    multiboot /boot/kernel.elf' >> $(BUILD_DIR)/iso/boot/grub/grub.cfg
	echo '    boot' >> $(BUILD_DIR)/iso/boot/grub/grub.cfg
	echo '}' >> $(BUILD_DIR)/iso/boot/grub/grub.cfg
	grub-mkrescue -o $(BUILD_DIR)/os.iso $(BUILD_DIR)/iso

run-iso: iso
	$(QEMU) -cdrom $(BUILD_DIR)/os.iso -m 128M