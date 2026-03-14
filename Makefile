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
	   kernel/timer.c kernel/process.c kernel/scheduler.c kernel/syscall.c \
	   kernel/syscall_table.c kernel/logging.c kernel/crash_handler.c \
	   kernel/power.c kernel/security.c kernel/update.c kernel/spinlock.c \
	   kernel/mutex.c kernel/semaphore.c kernel/condition.c kernel/message_queue.c \
	   kernel/pipe.c
DRIVER_C = drivers/vga.c drivers/serial.c drivers/pci.c drivers/ata.c \
	   drivers/rtl8139.c drivers/ac97.c drivers/rtc.c drivers/pit.c drivers/mouse.c \
	   drivers/ahci.c drivers/wifi.c drivers/bluetooth.c drivers/hdmi.c drivers/gpu.c
LIBC_C = libc/string.c libc/stdio.c libc/stdlib.c libc/memory.c libc/assert.c
GUI_C = gui/window.c gui/graphics.c gui/widgets.c gui/desktop.c gui/taskview.c \
	gui/apps/terminal.c gui/apps/file_manager.c gui/apps/notepad.c gui/apps/control_panel.c \
	gui/apps/time_applet.c gui/apps/virtual_keyboard.c gui/apps/settings.c \
	gui/launcher.c gui/taskbar.c gui/input.c gui/compositor.c gui/notification.c \
	gui/apps/package_manager.c gui/apps/hardware_manager.c
FS_C = fs/fat32.c fs/ext2.c fs/vfs.c fs/file.c
NET_C = net/nic.c net/ethernet.c net/ip.c net/tcp.c net/udp.c net/dhcp.c net/dns.c net/icmp.c net/arp.c

APPS_C = apps/shell/shell.c


NET_OBJ = $(NET_C:%.c=$(BUILDDIR)/%.o)
APPS_OBJ = $(APPS_C:%.c=$(BUILDDIR)/%.o)

ALL_OBJ = $(BOOT_OBJ) $(KERNEL_OBJ) $(DRIVER_OBJ) $(LIBC_OBJ) $(GUI_OBJ) $(FS_OBJ) $(NET_OBJ) $(APPS_OBJ)

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