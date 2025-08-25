# Maya OS - x86 Operating System

A complete x86 operating system with GUI, built from scratch.

## Features
- Custom bootloader
- 32-bit protected mode kernel
- Memory management
- Interrupt handling
- Keyboard and mouse support
- Basic GUI system
- File system support
- Multi-tasking

## Build Instructions
1. Install dependencies: `sudo apt-get install build-essential nasm qemu-system-x86 genisoimage`
2. Run `make` to build the OS
3. Run `make run` to test in QEMU

## Project Structure
- boot/ - Bootloader files
- kernel/ - Kernel source code
- drivers/ - Hardware drivers
- libc/ - Standard C library implementation
- gui/ - Graphics user interface
- apps/ - User applications
- fs/ - File system implementation
- tools/ - Build tools and utilities
