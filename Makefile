# Maya OS Makefile
# Build configuration
CC = i686-elf-gcc
ASM = nasm
LD = i686-elf-ld
QEMU = qemu-system-i386

# Directories
SRCDIR = .
BUILDDIR = build
ISODIR = $(BUILDDIR)/iso
BOOTDIR = $(ISODIR)/boot

# Compiler flags
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
	 -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c -ffreestanding
LDFLAGS = -T linker.ld -melf_i386
ASMFLAGS = -f elf32

# Source files
BOOT_ASM = boot/boot.asm boot/gdt.asm boot/idt.asm
KERNEL_C = kernel/kernel.c kernel/memory.c kernel/interrupts.c kernel/keyboard.c \
	   kernel/timer.c kernel/process.c
DRIVER_C = drivers/vga.c drivers/serial.c drivers/pci.c drivers/ata.c
LIBC_C = libc/string.c libc/stdio.c libc/stdlib.c libc/memory.c libc/assert.c
GUI_C = gui/window.c gui/graphics.c gui/widgets.c gui/desktop.c \
	gui/maya_desktop.c gui/maya_graphics.c gui/maya_input.c gui/maya_apps.c
FS_C = fs/fat32.c fs/vfs.c fs/file.c

# Object files
BOOT_OBJ = $(BOOT_ASM:%.asm=$(BUILDDIR)/%.o)
KERNEL_OBJ = $(KERNEL_C:%.c=$(BUILDDIR)/%.o)
DRIVER_OBJ = $(DRIVER_C:%.c=$(BUILDDIR)/%.o)
LIBC_OBJ = $(LIBC_C:%.c=$(BUILDDIR)/%.o) $(BUILDDIR)/libc/assert.o
GUI_OBJ = $(GUI_C:%.c=$(BUILDDIR)/%.o)
FS_OBJ = $(FS_C:%.c=$(BUILDDIR)/%.o)

ALL_OBJ = $(BOOT_OBJ) $(KERNEL_OBJ) $(DRIVER_OBJ) $(LIBC_OBJ) $(GUI_OBJ) $(FS_OBJ)

# Default target
all: maya-os.iso

# Create ISO image
maya-os.iso: $(BUILDDIR)/kernel.bin
	mkdir -p $(BOOTDIR)/grub
	cp $(BUILDDIR)/kernel.bin $(BOOTDIR)/
	cp grub.cfg $(BOOTDIR)/grub/
	grub-mkrescue -o maya-os.iso $(ISODIR)

# Link kernel
$(BUILDDIR)/kernel.bin: $(ALL_OBJ) | $(BUILDDIR)
	$(LD) $(LDFLAGS) $(ALL_OBJ) -o $@

# Compile C source files
$(BUILDDIR)/%.o: %.c | $(BUILDDIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I include $< -o $@

# Assemble ASM files
$(BUILDDIR)/%.o: %.asm | $(BUILDDIR)
	@mkdir -p $(dir $@)
	$(ASM) $(ASMFLAGS) $< -o $@

# Create build directory
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Run in QEMU
run: maya-os.iso
	$(QEMU) -cdrom maya-os.iso -m 256M -enable-kvm

# Debug in QEMU
debug: maya-os.iso
	$(QEMU) -cdrom maya-os.iso -m 256M -s -S

# Clean build files
clean:
	rm -rf $(BUILDDIR) maya-os.iso

# Install cross-compiler
install-toolchain:
	@echo "Installing i686-elf cross-compiler..."
	@echo "Please follow: https://wiki.osdev.org/GCC_Cross-Compiler"

.PHONY: all run debug clean install-toolchain