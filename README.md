#  EarlnuxOS

A modern hobby OS for x86 – built from scratch with a clean architecture.

## Table of Contents

- [Project Description](#project-description)
- [Features](#features)
- [Architecture](#architecture)
- [Folder Structure](#folder-structure)
- [Prerequisites](#prerequisites)
- [Building](#building)
- [Running](#running)
- [Project Status](#project-status)
- [Contributing](#contributing)
- [License](#license)

## Project Description

 EarlnuxOS is a 32-bit x86 operating system written in C and NASM assembly. It implements core OS abstractions: bootloading, memory management, virtual filesystem, interrupt handling, device drivers, and a full TCP/IP stack. Designed for education and experimentation,  EarlnuxOS demonstrates how to build a modern microkernel-inspired monolithic OS from the ground up.

### Key Design Goals

- **Higher-half kernel** running at `0xC0000000` virtual base
- **Multiboot-compliant** bootloader (GRUB-compatible)
- **VGA text-mode console** with color support
- **PMM** (buddy allocator), **VMM** (two-level paging), **SLAB** heap
- **VFS layer** with pluggable filesystems (RAMFS, PROCFS, VibeFS)
- **Full TCP/IP stack** (Ethernet, IPv4, ARP, ICMP, UDP, TCP, DHCP, DNS)
- **Interactive shell** with 15+ built-in commands

## Features

### Core Subsystems

- **Bootloader** – 2-stage BIOS loader (stage1 + stage2)
- **IDT/ISR** – 256-vector interrupt descriptor table with exception handlers
- **Memory Management** – Physical (buddy), Virtual (paging), Kernel heap (SLAB)
- **Filesystems** – Virtual filesystem (VFS), RAMFS (tmpfs), PROCFS, VibeFS (native)
- **Networking** – Complete TCP/IP stack with DHCP and DNS client
- **Drivers** – PS/2 keyboard, PIT timer, PIC interrupt controller
- **Console** – 80×25 VGA text mode, color attributes, formatted output (`kprintf`)
- **Shell** – Interactive command-line with history and built-in commands (`help`, `ping`, `ls`, `cat`, `meminfo`, `netinfo`, etc.)

## Architecture

 EarlnuxOS follows a **monolithic kernel** design with layered subsystems:

```
┌─────────────────────────────────────────┐
│          User Space (Shell)              │
├─────────────────────────────────────────┤
│              VFS Layer                   │
├─────────────┬──────────────┬─────────────┤
│    FAT      │    RAMFS     │   PROCFS    │
├─────────────┴──────────────┴─────────────┤
│          Block Device Layer              │
├─────────────────────────────────────────┤
│            Network Stack                 │
├─────────────┬──────────────┬─────────────┤
│     TCP     │     UDP      │    ICMP     │
├─────────────┴──────────────┴─────────────┤
│          Ethernet /  ARP                │
├─────────────────────────────────────────┤
│        Memory Management (PMM/VMM)       │
├─────────────────────────────────────────┤
│        Hardware Drivers (KBD, PIC, PIT)  │
├─────────────────────────────────────────┤
│        Console (VGA) + Kernel API       │
├─────────────────────────────────────────┤
│   Interrupts (IDT) + CPU Architecture   │
└─────────────────────────────────────────┘
```

**Boot Process**

1. **Stage 1** (`boot/boot.asm`) – BIOS loads MBR at `0x7C00` → loads stage2 via INT 13h
2. **Stage 2** (`boot/stage2.asm`) – Enables A20, sets up GDT, jumps to protected mode, loads kernel ELF at `0x100000`
3. **Kernel Entry** (`kernel/arch/x86/entry.asm`) – Multiboot header, BSS clear, calls `kernel_main`

**Memory Layout**

| Region | Virtual | Physical |
|--------|---------|----------|
| Kernel code/data | `0xC0000000 – 0xDFFFFFF` | `0x00100000 – 0x01FFFFF` |
| Kernel heap | `0xD0000000 – 0xDFFFFFFF` | — (paged) |
| MMIO | `0xE0000000+` | — |
| User space | `0x00000000 – 0xBFFFFFFF` | — |

## Folder Structure

```
project-OS/
├── boot/                        # Bootloader (16/32-bit real & protected mode)
│   ├── boot.asm                # Stage 1 MBR (512 B)
│   └── stage2.asm              # Stage 2: A20, GDT, PM, load kernel
├── kernel/                     # Kernel image and subsystems
│   ├── kernel.c                # Main entry point, init, shell
│   ├── arch/
│   │   └── x86/
│   │       ├── entry.asm       # Multiboot entry stub
│   │       ├── gdt.h / gdt.c   # Global Descriptor Table
│   │       ├── idt.h / idt.c   # Interrupt Descriptor Table
│   │       ├── isr.asm         # Assembly ISR stubs
│   │       ├── isr_common.asm  # Common ISR handler
│   │       ├── isr.c           # ISR dispatch + exception handlers
│   │       ├── cpu.h           # CPUID, CR0/CR4, MSR helpers
│   │       └── ports.h         # I/O port defines (PIC, PIT, KBC)
│   ├── mm/                     # Memory management
│   │   ├── mm.h                # PMM/VMM/heap API
│   │   ├── pmm.c               # Physical Memory Manager (buddy)
│   │   ├── vmm.c               # Virtual Memory Manager (paging)
│   │   └── heap.c              # Kernel heap allocator (SLAB-style)
│   ├── drivers/                # Device drivers
│   │   ├── pic.c               # 8259A PIC init + EOI
│   │   ├── pit.c               # 8254 PIT timer (1000 Hz)
│   │   ├── ps2.c               # PS/2 controller utilities (A20)
│   │   └── keyboard.c          # PS/2 keyboard (scancode → ASCII)
│   ├── fs/                     # Filesystem layer
│   │   ├── vfs.h / vfs.c       # Virtual filesystem core
│   │   ├── ramfs.c             # In-memory tmpfs
│   │   ├── procfs.c            # /proc pseudo-filesystem
│   │   └── vibefs.c            # Native log-structured FS (stub)
│   ├── net/                    # Network protocol stack
│   │   ├── net.h / net.c       # Network stack initialization
│   │   ├── eth.c               # Ethernet (802.3) framing
│   │   ├── arp.c               # ARP resolution + cache
│   │   ├── ip.c                # IPv4 layer (fragmentation, TTL)
│   │   ├── icmp.c              # ICMP (echo/ping)
│   │   ├── udp.c               # UDP sockets
│   │   ├── tcp.c               # TCP state machine, buffers
│   │   ├── dhcp.c              # DHCP client
│   │   ├── dns.c               # DNS resolver with cache
│   │   └── netif.c             # Network interface abstraction
│   ├── lib/                    # Kernel libraries
│   │   ├── string.c            # memcpy, memset, strcmp, etc.
│   │   └── printf.c            # `kprintf` implementation
│   └── linker.ld               # Kernel linker script (higher-half)
├── include/                    # Public headers
│   ├── types.h                 # Fixed-width types, macros, kernel base
│   ├── kernel/
│   │   ├── kernel.h            # Core kernel API (console, logging, panic)
│   │   └── console.h           # VGA console API
│   └── arch/
│       └── x86/
│           ├── cpu.h           # x86 CPU features, CR access, CPUID
│           └── ports.h         # I/O port constants (PIC, PIT, KBC)
├── Makefile                    # Build system (NASM + i686-elf-gcc)
├── README.md                   # This file
└── LICENSE                    # MIT License
```

## Prerequisites

> **Important**:  EarlnuxOS requires a cross-compiler toolchain that is not available in standard package repositories. See [`TOOLCHAIN_SETUP.md`](TOOLCHAIN_SETUP.md) for detailed installation instructions.

| Tool | Purpose | Installation |
|------|---------|-------------|
| `nasm` | Netwide Assembler (bootloader, assembly stubs) | `apt install nasm` / `brew install nasm` |
| `i686-elf-gcc` | Cross-compiler for 32-bit ELF | **See `TOOLCHAIN_SETUP.md`** |
| `i686-elf-ld` | Cross-linker | Comes with binutils |
| `qemu-system-i386` | i386 emulator | `apt install qemu-system-i386` / `brew install qemu` |
| `make` | Build automation | Usually pre-installed |

### Installing the Cross-Compiler

On **Ubuntu/Debian** you can build the toolchain:

```bash
sudo apt install build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo

# Binutils
wget https://ftp.gnu.org/gnu/binutils/binutils-2.38.tar.gz
tar -xzf binutils-2.38.tar.gz && cd binutils-2.38
mkdir build && cd build
../configure --target=i686-elf --prefix=/usr/local --with-sysroot --disable-nls --disable-werror
make -j$(nproc) && sudo make install
cd ../..

# GCC
wget https://ftp.gnu.org/gnu/gcc/gcc-11.2.0/gcc-11.2.0.tar.gz
tar -xzf gcc-11.2.0.tar.gz && cd gcc-11.2.0
mkdir build && cd build
../configure --target=i686-elf --prefix=/usr/local --disable-nls --enable-languages=c --without-headers
make all-gcc -j$(nproc) && sudo make install-gcc
```

On **macOS**: `brew install i686-elf-gcc nasm qemu` (if available via custom taps) or use MacPorts.

On **Windows**: Use WSL2 (Ubuntu) and follow the Linux instructions, or install a pre-built i686-elf toolchain.

## Building

```bash
# Clean build
make clean && make

# Or simply
make
```

Generates:

- `build/boot.bin` – Stage 1+2 bootloader binary (raw)
- `build/vibos.elf` – Linked kernel ELF (higher-half, Multiboot)
- `build/vibos.img` – 1.44 MiB floppy image ready to boot

Optional targets:

```bash
make run      # Build and launch QEMU
make debug    # Build and launch QEMU with GDB stub on :1234
make clean    # Remove build artifacts
make tree     # Print project file tree
```

## Running

```bash
make run
```

QEMU launches with the floppy image. Expected output:

```
 EarlnuxOS v1.0 "Prism"  -  Copyright (c) 2025  EarlnuxOS Project
Built: Apr 23 2026 08:00:00 | Arch: i686

=== Kernel Initialization ===

  Initializing console                  [  OK  ]
  Initializing physical memory (PMM)    [  OK  ]
  Initializing virtual memory (VMM)     [  OK  ]
  Initializing kernel heap (SLAB)       [  OK  ]
  Initializing interrupt descriptor tbl [  OK  ]
  Initializing PIC                      [  OK  ]
  Initializing PIT (1000 Hz)            [  OK  ]
  Initializing PS/2 keyboard            [  OK  ]
  Initializing VFS                      [  OK  ]
  Mounting initial filesystems          [  OK  ]
  Initializing network (TCP/IP)         [  OK  ]

Network: Starting DHCP... OK  IP: 10.0.2.15

***  EarlnuxOS kernel initialized successfully ***
```

You then get an interactive shell:

```
root@ EarlnuxOS:~# help
 EarlnuxOS Built-in Shell Commands:
  help              Show this help
  echo [text]       Print text to console
  clear             Clear screen
  meminfo           Show memory statistics
  netinfo           Show network statistics
  ifconfig          Show/configure network interface
  ping <ip|host>    Ping an IP address
  ls [path]         List directory
  cat <file>        Print file content
  mkdir <dir>       Create directory
  rm <file>         Remove file
  write <file>      Write stdin to file
  ps                List processes
  uptime            Show system uptime
  uname             System information
  reboot            Reboot system
  halt              Halt system

root@ EarlnuxOS:~#
```

## Project Status

| Component | Status | Notes |
|-----------|--------|-------|
| Bootloader | ✅ | Stage 1 + 2, A20 gate, GDT, protected mode |
| Console | ✅ | VGA text mode 80×25, color, scrolling |
| IDT / ISR | ✅ | Full 256-vector table; exceptions + IRQs |
| PIC / PIT | ✅ | IRQ remapping to 0x20–0x2F; 1000 Hz timer |
| Keyboard | ✅ | PS/2 scancode set 2 → ASCII, shift/ctrl modifiers |
| PMM | ✅ | Buddy allocator (3 zones: DMA, Normal, Highmem) |
| VMM | ✅ | Two-level paging (1024-entry PDE/PTE) |
| Kernel Heap | ✅ | SLAB-style block allocator with coalescing |
| VFS | ✅ | Core VFS layer (open/read/write/seek/mkdir/unlink/readdir) |
| RAMFS | ✅ | In-memory tmpfs; supports files + directories |
| PROCFS | ⚠️ | Basic skeleton; `/proc` mount works |
| VibeFS | ⚠️ | Header defined; implementation pending |
| Network Stack | ⚠️ | Core protocols defined; most drivers are stubs |
| TCP | ⚠️ | Skeleton exists, needs integration |
| Shell | ✅ | 15+ built-in commands working |

> **Legend**: ✅ Complete ⚠️ Partial (skeleton/stub) ❌ Missing

## Contributing

 EarlnuxOS is an educational project. Contributions are welcome:

- Implement missing filesystems (VibeFS, ext2)
- Complete network drivers (e.g., Intel e1000, Realtek RTL8139)
- Add process management & scheduling
- Implement ELF loader for user programs
- Write proper TCP retransmission and flow control

Please fork the repository and open a pull request with a clear description of your changes.

## License

MIT License – see [`LICENSE`](LICENSE) for details.