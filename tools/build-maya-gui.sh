#!/bin/bash

# Maya OS Ubuntu Desktop Build Script

echo "Building Maya OS with Ubuntu Desktop Environment..."

# Check dependencies
echo "Checking build dependencies..."

if ! command -v i686-elf-gcc &> /dev/null; then
    echo "Error: i686-elf-gcc not found!"
    echo "Please install the cross-compiler first:"
    echo "https://wiki.osdev.org/GCC_Cross-Compiler"
    exit 1
fi

if ! command -v nasm &> /dev/null; then
    echo "Error: NASM assembler not found!"
    echo "Install with: sudo apt-get install nasm"
    exit 1
fi

if ! command -v qemu-system-i386 &> /dev/null; then
    echo "Error: QEMU not found!"
    echo "Install with: sudo apt-get install qemu-system-x86"
    exit 1
fi

if ! command -v grub-mkrescue &> /dev/null; then
    echo "Error: GRUB mkrescue not found!"
    echo "Install with: sudo apt-get install grub-pc-bin"
    exit 1
fi

echo "All dependencies found!"

# Create necessary directories
echo "Creating build directories..."
mkdir -p build/boot/grub
mkdir -p build/kernel
mkdir -p build/drivers
mkdir -p build/libc
mkdir -p build/gui
mkdir -p build/fs

# Build the OS
echo "Building Maya OS..."
make clean
make

if [ $? -eq 0 ]; then
    echo ""
    echo "✅ Maya OS with Ubuntu Desktop built successfully!"
    echo ""
    echo "To run Maya OS:"
    echo "  make run"
    echo ""
    echo "To debug Maya OS:"
    echo "  make debug"
    echo ""
    echo "Ubuntu Desktop Features:"
    echo "  • Purple-orange gradient wallpaper"
    echo "  • Unity-style launcher with app icons"
    echo "  • Top panel with Activities button"
    echo "  • Application dash with search"
    echo "  • Desktop icons and file manager"
    echo "  • Built-in applications (Terminal, Text Editor, Calculator)"
    echo "  • System notifications"
    echo "  • Settings application"
    echo ""
    echo "Controls:"
    echo "  • Arrow keys: Move mouse cursor"
    echo "  • Space: Click/Select"
    echo "  • Space on Activities: Open Application Dash"
    echo "  • ESC: Close overlays"
    echo "  • H: Toggle launcher visibility"
    echo "  • T: Toggle taskbar visibility"
    echo ""
else
    echo "❌ Build failed!"
    exit 1
fi