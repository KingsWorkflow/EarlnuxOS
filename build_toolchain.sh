#!/bin/bash
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}=== Building i686-elf Cross-Compiler Toolchain ===${NC}"

# Install prerequisites
echo -e "${YELLOW}[1/5] Installing prerequisites...${NC}"
sudo apt update
sudo apt install -y build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo wget

# Create build directory
BUILD_HOME="$HOME/toolchain_build"
mkdir -p "$BUILD_HOME"
cd "$BUILD_HOME"

# Get the project directory
PROJECT_DIR="/mnt/c/Users/Prashanna/OneDrive/Documents/KINGS COLLEGE/session 9/BSIT-338/Final Project/project-OS"

# Function to extract archives (handles .tar.gz and .tar.gz.1)
extract_archive() {
    local file=$1
    local pattern=$2
    
    if [ -f "$file" ]; then
        tar -xzf "$file"
        return 0
    elif [ -f "${file}.1" ]; then
        tar -xzf "${file}.1"
        return 0
    else
        # Try to find similar files
        found=$(ls "$PROJECT_DIR"/$pattern* 2>/dev/null | head -1)
        if [ -n "$found" ]; then
            cp "$found" .
            tar -xzf "$(basename "$found")"
            return 0
        fi
        return 1
    fi
}

# Build Binutils
echo -e "${YELLOW}[2/5] Building binutils-2.38...${NC}"
if extract_archive "binutils-2.38.tar.gz" "binutils-2.38*"; then
    cd binutils-2.38
    mkdir -p build && cd build
    ../configure --target=i686-elf --prefix=/usr/local --with-sysroot --disable-nls --disable-werror
    make -j$(nproc)
    sudo make install
    cd ../..
    echo -e "${GREEN}✓ Binutils installed${NC}"
else
    echo -e "${RED}✗ Failed to find binutils source${NC}"
    exit 1
fi

# Build GCC
echo -e "${YELLOW}[3/5] Building gcc-11.2.0...${NC}"
if extract_archive "gcc-11.2.0.tar.gz" "gcc-11.2.0*"; then
    cd gcc-11.2.0
    mkdir -p build && cd build
    ../configure --target=i686-elf --prefix=/usr/local --disable-nls --enable-languages=c --without-headers
    make all-gcc -j$(nproc)
    sudo make install-gcc
    cd ../..
    echo -e "${GREEN}✓ GCC installed${NC}"
else
    echo -e "${RED}✗ Failed to find gcc source${NC}"
    exit 1
fi

# Verify installation
echo -e "${YELLOW}[4/5] Verifying installation...${NC}"
if command -v i686-elf-gcc &> /dev/null; then
    echo -e "${GREEN}✓ i686-elf-gcc found: $(i686-elf-gcc --version | head -1)${NC}"
else
    echo -e "${RED}✗ i686-elf-gcc not found in PATH${NC}"
    echo -e "${YELLOW}Adding /usr/local/bin to PATH...${NC}"
    echo 'export PATH=/usr/local/bin:$PATH' >> ~/.bashrc
    source ~/.bashrc
fi

if command -v i686-elf-ld &> /dev/null; then
    echo -e "${GREEN}✓ i686-elf-ld found: $(i686-elf-ld --version | head -1)${NC}"
else
    echo -e "${RED}✗ i686-elf-ld not found${NC}"
fi

# Test the build
echo -e "${YELLOW}[5/5] Testing OS build...${NC}"
cd "$PROJECT_DIR"
make clean && make
echo -e "${GREEN}✓ Build successful!${NC}"

echo -e "${GREEN}=== Toolchain setup complete! ===${NC}"
