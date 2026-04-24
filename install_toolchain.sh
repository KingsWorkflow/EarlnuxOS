#!/bin/bash
set -e

echo "Installing i686-linux-gnu toolchain..."
sudo apt-get update
sudo apt-get install -y gcc-i686-linux-gnu binutils-i686-linux-gnu

echo "Verifying installation..."
i686-linux-gnu-gcc --version
i686-linux-gnu-ld --version

echo "Toolchain installed successfully!"
