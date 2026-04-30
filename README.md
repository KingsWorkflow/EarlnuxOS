# EarlnuxOS
A professional-grade 32-bit x86 Operating System built from scratch with a clean, modular architecture.

## Overview
EarlnuxOS is a monolithic kernel implementation for the i686 architecture. It demonstrates modern OS abstractions including multiboot-compliant loading, advanced memory management (Buddy/SLAB), a Virtual Filesystem (VFS) layer, a complete TCP/IP network stack, and a robust interrupt handling system with PIC/PIT synchronization.

## New in v1.0.0 "Prism" (Senior Dev Edition)
- **Complete Network Stack**: Full TCP/IP implementation with RTL8139 driver, ARP, IPv4, ICMP, UDP, TCP, DHCP, DNS, and ping utility.
- **Advanced Navigation**: Global Current Working Directory (CWD) system with support for relative paths and `..` traversal.
- **Enhanced Productivity**: New `write`, `touch`, `logout`, `pwd`, `uptime`, and `ping` commands with full interactive help manual.
- **High-Impact UI**: Striking ASCII boot banner and system health dashboard.

## Features
- **Microkernel-Inspired Monolithic Design**: Modular subsystems with clean interfaces.
- **Advanced Memory Management**: Physical Buddy Allocator, Two-Level Paging (VMM), and SLAB-style Kernel Heap.
- **Multi-User Security**: Authentication gate with session management and user-specific prompts.
- **Pluggable VFS**: Virtual Filesystem supporting RAMFS, PROCFS, and native VibeFS.
- **Context-Aware Shell**: Interactive CLI with path-aware command execution and 20+ built-in utilities.
- **Integrated Network Stack**: Complete TCP/IP implementation with RTL8139 Ethernet driver, ARP, IPv4, ICMP (ping), UDP, TCP, DHCP, and DNS resolution.

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
The build system uses a standard `Makefile` to generate a bootable kernel image.
### Build Targets:
- `make clean`: Remove all build artifacts.
- `make run`: Build and launch immediately in QEMU with network support.
- `make debug`: Build and launch in QEMU with debugging enabled.
- `make iso`: Generate a bootable ISO image.

## Deployment & Universal Access

### Network Configuration
When running in QEMU, the OS automatically detects the RTL8139 network interface and configures it with a static IP (10.0.2.15) for user-mode networking. Use the `netif` command to view interface details and `ping` to test connectivity.

### 🌐 Web Browser Demo (v86)
EarlnuxOS can be run directly in your browser without installation.
1. Enable **GitHub Pages** for your repository.
2. Ensure `os.iso` is in the `build/` folder.
3. Access the demo via `https://kingsworkflow.github.io/EarlnuxOS/`.

### 🐳 Docker Build (Zero Installation)
Build the OS without installing any local toolchains:
```bash
# Build the environment
docker build -t earlnux .

# Build the ISO inside Docker
docker run --rm -v $(pwd):/os earlnux
```

# Run in QEMU
qemu-system-i386 -cdrom build/os.iso -boot d -m 128M -net nic,model=rtl8139 -net user

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
| **Networking** | ✅ | RTL8139 Driver + Full TCP/IP stack (ARP/IP/ICMP/UDP/TCP/DHCP/DNS) with ping utility |
| **Shell**      | ✅ | Location-aware CLI with 20+ commands |

> ✅ Fully Operational | ⚠️ Partial/Skeleton | ❌ Planned

## License
EarlnuxOS is released under the **MIT License**. See `LICENSE` for details.
© 2026 EarlnuxOS Project. Built with ❤️ by Prashanna.