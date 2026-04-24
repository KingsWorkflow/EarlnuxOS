# Install Cross-Compiler Toolchain for  EarlnuxOS

## Prerequisites

First, install the required build tools:

```bash
sudo apt update
sudo apt install build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo wget
```

## Build and Install i686-elf-gcc

### 1. Create a build directory
```bash
mkdir -p ~/toolchain
cd ~/toolchain
```

### 2. Download and build binutils (2.38)
```bash
wget https://ftp.gnu.org/gnu/binutils/binutils-2.38.tar.gz
tar -xzf binutils-2.38.tar.gz
cd binutils-2.38
mkdir build && cd build
../configure --target=i686-elf --prefix=/usr/local --with-sysroot --disable-nls --disable-werror
make -j$(nproc)
sudo make install
cd ../..
```

### 3. Download and build GCC (11.2.0)
```bash
wget https://ftp.gnu.org/gnu/gcc/gcc-11.2.0/gcc-11.2.0.tar.gz
tar -xzf gcc-11.2.0.tar.gz
cd gcc-11.2.0
mkdir build && cd build
../configure --target=i686-elf --prefix=/usr/local --disable-nls --enable-languages=c --without-headers
make all-gcc -j$(nproc)
sudo make install-gcc
cd ../..
```

### 4. Verify installation
```bash
i686-elf-gcc --version
i686-elf-ld --version
```

You should see version information for both tools.

## Alternative: Use Pre-built Toolchain

If building from source is too slow, you can try installing a pre-built toolchain:

```bash
# Option 1: Install via package manager (if available)
sudo apt install gcc-i686-linux-gnu binutils-i686-linux-gnu

# Option 2: Download pre-built i686-elf toolchain from:
# https://github.com/nativeos/i686-elf-toolchain/releases
# Extract and add to PATH
```

## Install NASM and QEMU

```bash
sudo apt install nasm qemu-system-i386
```

## Test the Build

Once the toolchain is installed:

```bash
cd /mnt/c/Users/Prashanna/OneDrive/Documents/KINGS\ COLLEGE/session\ 9/BSIT-338/Final\ Project/project-OS
make clean && make
make run
```

The build should now succeed and boot  EarlnuxOS in QEMU!