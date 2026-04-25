# EarlnuxOS
A professional-grade 32-bit x86 Operating System built from scratch with a clean, modular architecture.

## Overview
EarlnuxOS is a monolithic kernel implementation for the i686 architecture. It demonstrates modern OS abstractions including multiboot-compliant loading, advanced memory management (Buddy/SLAB), a Virtual Filesystem (VFS) layer, and a robust interrupt handling system with PIC/PIT synchronization.

## New in v1.0.0 "Prism" (Senior Dev Edition)
- **Production-Grade Security**: Full multi-user authentication system with `/etc/passwd` storage and password masking.
- **Advanced Navigation**: Global Current Working Directory (CWD) system with support for relative paths and `..` traversal.
- **Enhanced Productivity**: New `write`, `touch`, `logout`, and `pwd` commands with full interactive help manual.
- **High-Impact UI**: Striking ASCII boot banner and system health dashboard.

## Features
- **Microkernel-Inspired Monolithic Design**: Modular subsystems with clean interfaces.
- **Advanced Memory Management**: Physical Buddy Allocator, Two-Level Paging (VMM), and SLAB-style Kernel Heap.
- **Multi-User Security**: Authentication gate with session management and user-specific prompts.
- **Pluggable VFS**: Virtual Filesystem supporting RAMFS, PROCFS, and native VibeFS.
- **Context-Aware Shell**: Interactive CLI with path-aware command execution and 20+ built-in utilities.
- **Integrated Network Stack**: Core TCP/IP implementation (IPv4, ARP, ICMP, UDP, TCP, DHCP, DNS).

## Architecture
```text
┌─────────────────────────────────────────┐
│          User Space (Multi-User Shell)   │
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
qemu-system-i386 -cdrom build/os.iso -boot d -m 256M
```

## Project Status
| Component      | Status | Description |
|----------------|--------|-------------|
| **Bootloader** | ✅ | Multiboot 1.0 compliant (GRUB support) |
| **Security**   | ✅ | User authentication and session management |
| **IDT / ISR**  | ✅ | 256 vectors with hardened segment isolation |
| **PIC / PIT**  | ✅ | Production-grade IRQ remapping & 1000Hz timer |
| **Keyboard**   | ✅ | PS/2 driver with scancode translation |
| **PMM / VMM**  | ✅ | Buddy allocator and two-level paging |
| **VFS / RAMFS**| ✅ | Full POSIX-style path traversal & node management |
| **Networking** | ✅ | RTL8139 Driver + Core protocols (ARP/IP/ICMP) |
| **Shell**      | ✅ | Location-aware CLI with 20+ commands |

> ✅ Fully Operational | ⚠️ Partial/Skeleton | ❌ Planned

## License
EarlnuxOS is released under the **MIT License**. See `LICENSE` for details.
© 2026 EarlnuxOS Project.