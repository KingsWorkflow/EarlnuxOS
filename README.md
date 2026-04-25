# EarlnuxOS

A professional-grade 32-bit x86 Operating System built from scratch with a clean, modular architecture.

## Overview

EarlnuxOS is a monolithic kernel implementation for the i686 architecture. It demonstrates modern OS abstractions including multiboot-compliant loading, advanced memory management (Buddy/SLAB), a Virtual Filesystem (VFS) layer, and a robust interrupt handling system with PIC/PIT synchronization.

## Features

- **Microkernel-Inspired Monolithic Design**: Modular subsystems with clean interfaces.
- **Advanced Memory Management**: Physical Buddy Allocator, Two-Level Paging (VMM), and SLAB-style Kernel Heap.
- **Robust Interrupt System**: 256-vector IDT with hardware IRQ remapping and production-grade PIC synchronization.
- **Pluggable VFS**: Virtual Filesystem supporting RAMFS, PROCFS, and native VibeFS.
- **Interactive Shell**: Full-featured shell with command history, path traversal, and 15+ built-in utilities.
- **Integrated Network Stack**: Core TCP/IP implementation (IPv4, ARP, ICMP, UDP, TCP, DHCP, DNS).

## Architecture

```text
┌─────────────────────────────────────────┐
│          User Space (Interactive Shell)  │
├─────────────────────────────────────────┤
│              VFS Layer (POSIX)           │
├─────────────┬──────────────┬─────────────┤
│    RAMFS    │    PROCFS    │   VibeFS    │
├─────────────┴──────────────┴─────────────┤
│            Network Stack (L2-L4)         │
├─────────────────────────────────────────┤
│        Memory Management (PMM/VMM)       │
├─────────────────────────────────────────┤
│        Hardware Drivers (KBD, PIC, PIT)  │
├─────────────────────────────────────────┤
│   Interrupts (IDT) + ISR Dispatcher     │
└─────────────────────────────────────────┘
```

## Prerequisites

On **Ubuntu/Debian/WSL**:
```bash
sudo apt update
sudo apt install -y nasm gcc-i686-linux-gnu binutils-i686-linux-gnu qemu-system-i386 make xorriso grub-pc-bin mtools
```

## Building and Running

The build system uses a standard `Makefile` to generate a bootable ISO.

```bash
# Build the ISO image
make iso

# Run in QEMU
qemu-system-i386 -cdrom build/os.iso -boot d -m 128M
```

### Optional Build Targets:
- `make clean`: Remove all build artifacts.
- `make run`: Build and launch immediately in QEMU.

## Project Status

| Component      | Status | Description |
|----------------|--------|-------------|
| **Bootloader** | ✅ | Multiboot 1.0 compliant (GRUB support) |
| **Console**    | ✅ | VGA Text Mode (80x25) with full color support |
| **IDT / ISR**  | ✅ | 256 vectors with hardened segment isolation |
| **PIC / PIT**  | ✅ | Production-grade IRQ remapping & 1000Hz timer |
| **Keyboard**   | ✅ | PS/2 driver with scancode translation |
| **PMM / VMM**  | ✅ | Buddy allocator and two-level paging |
| **Kernel Heap**| ✅ | SLAB allocator with coalescing |
| **VFS / RAMFS**| ✅ | Full POSIX-style path traversal & node management |
| **Networking** | ⚠️ | Core protocols implemented; driver stubs pending |
| **Shell**      | ✅ | Interactive CLI with history and 15 commands |

> ✅ Fully Operational | ⚠️ Partial/Skeleton | ❌ Planned

## License

EarlnuxOS is released under the **MIT License**. See `LICENSE` for details.
© 2026 EarlnuxOS Project.