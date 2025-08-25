# Maya OS - Additional Files to Implement

## Critical Priority Files (Implement First)

### kernel/syscalls.c - System Call Interface
```c
#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "kernel/kernel.h"
#include "kernel/process.h"

// System call numbers
#define SYS_EXIT    1
#define SYS_FORK    2  
#define SYS_READ    3
#define SYS_WRITE   4
#define SYS_OPEN    5
#define SYS_CLOSE   6
#define SYS_WAITPID 7
#define SYS_CREAT   8
#define SYS_LINK    9
#define SYS_UNLINK  10
#define SYS_EXEC    11
#define SYS_CHDIR   12
#define SYS_TIME    13
#define SYS_GETPID  14
#define SYS_SLEEP   15
#define SYS_MALLOC  16
#define SYS_FREE    17

// System call handler
void syscall_handler(struct registers *regs);
void syscall_init(void);

// Individual system call implementations
int sys_exit(int status);
int sys_fork(void);
int sys_read(int fd, void *buf, size_t count);
int sys_write(int fd, const void *buf, size_t count);
int sys_open(const char *pathname, int flags);
int sys_close(int fd);
int sys_getpid(void);
#endif
```

### drivers/pci.c - PCI Bus Driver
```c
#include "drivers/pci.h"
#include "kernel/kernel.h"

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

// PCI device structure
typedef struct {
    uint16_t vendor_id;
        uint16_t device_id;
            uint8_t class_code;
                uint8_t subclass;
                    uint8_t bus;
                        uint8_t slot;
                            uint8_t func;
                            } pci_device_t;

                            void pci_init(void);
                            uint32_t pci_config_read_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
                            void pci_config_write_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value);
                            pci_device_t *pci_find_device(uint16_t vendor_id, uint16_t device_id);
                            void pci_enumerate_devices(void);
                            ```

                            ### drivers/mouse.c - Mouse Driver
                            ```c
                            #include "drivers/mouse.h"
                            #include "kernel/interrupts.h"

                            #define MOUSE_DATA_PORT    0x60
                            #define MOUSE_STATUS_PORT  0x64
                            #define MOUSE_COMMAND_PORT 0x64

                            typedef struct {
                                int x, y;
                                    uint8_t buttons;
                                        int x_velocity, y_velocity;
                                        } mouse_state_t;

                                        static mouse_state_t mouse_state = {160, 100, 0, 0, 0};

                                        void mouse_init(void);
                                        void mouse_handler(struct registers *regs);
                                        mouse_state_t mouse_get_state(void);
                                        void mouse_set_position(int x, int y);
                                        ```

                                        ## High Priority Files

                                        ### net/ethernet.c - Network Stack Foundation
                                        ```c
                                        #include "net/ethernet.h"

                                        #define ETHERNET_TYPE_ARP  0x0806
                                        #define ETHERNET_TYPE_IP   0x0800

                                        typedef struct {
                                            uint8_t dest_mac[6];
                                                uint8_t src_mac[6]; 
                                                    uint16_t type;
                                                    } __attribute__((packed)) ethernet_header_t;

                                                    void ethernet_init(void);
                                                    void ethernet_send_packet(uint8_t *dest_mac, uint16_t type, uint8_t *data, uint32_t length);
                                                    void ethernet_receive_handler(uint8_t *packet, uint32_t length);
                                                    ```

                                                    ### drivers/rtl8139.c - Network Card Driver
                                                    ```c
                                                    #include "drivers/rtl8139.h"
                                                    #include "drivers/pci.h"

                                                    #define RTL8139_VENDOR_ID 0x10EC
                                                    #define RTL8139_DEVICE_ID 0x8139

                                                    // RTL8139 register offsets
                                                    #define RTL8139_MAC        0x00
                                                    #define RTL8139_RBSTART    0x30
                                                    #define RTL8139_CMD        0x37
                                                    #define RTL8139_IMR        0x3C
                                                    #define RTL8139_ISR        0x3E
                                                    #define RTL8139_RCR        0x44
                                                    #define RTL8139_CONFIG1    0x52

                                                    void rtl8139_init(void);
                                                    void rtl8139_send_packet(uint8_t *data, uint32_t length);
                                                    void rtl8139_receive_handler(struct registers *regs);
                                                    ```

                                                    ### drivers/ac97.c - Audio Controller
                                                    ```c
                                                    #include "drivers/ac97.h"
                                                    #include "drivers/pci.h"

                                                    #define AC97_VENDOR_ID_INTEL 0x8086
                                                    #define AC97_DEVICE_ID_ICH   0x2415

                                                    // AC97 mixer registers
                                                    #define AC97_MASTER_VOLUME    0x02
                                                    #define AC97_PCM_VOLUME       0x18
                                                    #define AC97_MIC_VOLUME       0x0E

                                                    typedef struct {
                                                        uint32_t nabm_base;  // Native Audio Bus Master
                                                            uint32_t nam_base;   // Native Audio Mixer
                                                            } ac97_controller_t;

                                                            void ac97_init(void);
                                                            void ac97_play_sound(uint8_t *buffer, uint32_t length);
                                                            void ac97_set_volume(uint8_t left, uint8_t right);
                                                            ```

                                                            ## Medium Priority Files

                                                            ### fs/ext2.c - EXT2 Filesystem
                                                            ```c
                                                            #include "fs/ext2.h"

                                                            #define EXT2_SIGNATURE 0xEF53
                                                            #define EXT2_BLOCK_SIZE 1024

                                                            typedef struct {
                                                                uint32_t inodes_count;
                                                                    uint32_t blocks_count;
                                                                        uint32_t reserved_blocks_count;
                                                                            uint32_t free_blocks_count;
                                                                                uint32_t free_inodes_count;
                                                                                    uint32_t first_data_block;
                                                                                        uint32_t log_block_size;
                                                                                            // ... more fields
                                                                                            } __attribute__((packed)) ext2_superblock_t;

                                                                                            int ext2_init(uint8_t drive);
                                                                                            int ext2_read_file(const char *path, uint8_t *buffer, uint32_t size);
                                                                                            int ext2_write_file(const char *path, uint8_t *data, uint32_t size);
                                                                                            ```

                                                                                            ### apps/filemgr/filemanager.c - File Manager Application
                                                                                            ```c
                                                                                            #include "apps/filemanager.h"
                                                                                            #include "gui/window.h"
                                                                                            #include "gui/widgets.h"
                                                                                            #include "fs/vfs.h"

                                                                                            typedef struct {
                                                                                                window_t *window;
                                                                                                    char current_path[256];
                                                                                                        file_entry_t *files;
                                                                                                            int file_count;
                                                                                                                int selected_index;
                                                                                                                } file_manager_t;

                                                                                                                file_manager_t *file_manager_create(void);
                                                                                                                void file_manager_refresh(file_manager_t *fm);
                                                                                                                void file_manager_navigate(file_manager_t *fm, const char *path);
                                                                                                                void file_manager_run(file_manager_t *fm);
                                                                                                                ```

                                                                                                                ## Implementation Suggestions by Category

                                                                                                                ### 1. System Calls (CRITICAL)
                                                                                                                Your OS needs a proper system call interface to bridge user programs and kernel services. This is fundamental for any real application support.

                                                                                                                **Files to implement:**
                                                                                                                - `kernel/syscalls.c` - Main system call dispatcher
                                                                                                                - `kernel/syscall_table.c` - System call table and handlers
                                                                                                                - `include/kernel/syscalls.h` - System call definitions

                                                                                                                ### 2. PCI Bus Support (CRITICAL)
                                                                                                                Modern hardware relies on PCI/PCIe. This driver enables detection and configuration of PCI devices.

                                                                                                                **Files to implement:**
                                                                                                                - `drivers/pci.c` - PCI bus enumeration
                                                                                                                - `drivers/pci_config.c` - PCI configuration space access
                                                                                                                - `include/drivers/pci.h` - PCI structures and definitions

                                                                                                                ### 3. Mouse Support (HIGH PRIORITY)
                                                                                                                Essential for GUI interaction.

                                                                                                                **Files to implement:**
                                                                                                                - `drivers/mouse.c` - PS/2 mouse driver
                                                                                                                - `drivers/usb_mouse.c` - USB mouse support (later)

                                                                                                                ### 4. Network Stack (HIGH PRIORITY)
                                                                                                                For internet connectivity and network applications.

                                                                                                                **Files to implement:**
                                                                                                                - Network drivers: `drivers/rtl8139.c`, `drivers/e1000.c`
                                                                                                                - Network stack: `net/ethernet.c`, `net/arp.c`, `net/ip.c`, `net/tcp.c`, `net/udp.c`
                                                                                                                - Socket interface: `net/socket.c`

                                                                                                                ### 5. Audio Support (MEDIUM PRIORITY)
                                                                                                                For multimedia applications.

                                                                                                                **Files to implement:**
                                                                                                                - `drivers/ac97.c` - AC97 audio controller
                                                                                                                - `drivers/sb16.c` - Sound Blaster 16 (legacy)
                                                                                                                - `audio/mixer.c` - Audio mixing system

                                                                                                                ### 6. USB Support (MEDIUM PRIORITY)
                                                                                                                For modern peripherals.

                                                                                                                **Files to implement:**
                                                                                                                - `drivers/usb_ohci.c` - USB 1.1 Open Host Controller
                                                                                                                - `drivers/usb_ehci.c` - USB 2.0 Enhanced Host Controller
                                                                                                                - `drivers/usb_hid.c` - USB Human Interface Device support

                                                                                                                ### 7. Advanced File Systems (MEDIUM PRIORITY)
                                                                                                                Support for more file systems.

                                                                                                                **Files to implement:**
                                                                                                                - `fs/ext2.c` - Linux EXT2 filesystem
                                                                                                                - `fs/ntfs.c` - Windows NTFS (read-only)
                                                                                                                - `fs/iso9660.c` - CD-ROM filesystem
                                                                                                                - `fs/procfs.c` - Process information filesystem

                                                                                                                ### 8. Additional Applications (LOW PRIORITY)
                                                                                                                User-facing applications to demonstrate OS capabilities.

                                                                                                                **Files to implement:**
                                                                                                                - `apps/calculator/calc.c` - Calculator app
                                                                                                                - `apps/terminal/terminal.c` - Terminal emulator
                                                                                                                - `apps/paint/paint.c` - Graphics editor
                                                                                                                - `apps/clock/clock.c` - System clock

                                                                                                                ## Missing Header Files

                                                                                                                Several header files are referenced but not fully defined:

                                                                                                                ### include/kernel/syscalls.h
                                                                                                                ```c
                                                                                                                #ifndef SYSCALLS_H
                                                                                                                #define SYSCALLS_H

                                                                                                                #define MAX_SYSCALLS 256

                                                                                                                typedef int (*syscall_handler_t)(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5);

                                                                                                                extern syscall_handler_t syscall_table[MAX_SYSCALLS];

                                                                                                                void syscall_init(void);
                                                                                                                void syscall_handler(struct registers *regs);

                                                                                                                #endif
                                                                                                                ```

                                                                                                                ### include/drivers/pci.h
                                                                                                                ```c
                                                                                                                #ifndef PCI_H
                                                                                                                #define PCI_H

                                                                                                                #include <stdint.h>

                                                                                                                #define PCI_VENDOR_ID      0x00
                                                                                                                #define PCI_DEVICE_ID      0x02
                                                                                                                #define PCI_COMMAND        0x04
                                                                                                                #define PCI_STATUS         0x06
                                                                                                                #define PCI_CLASS_CODE     0x0B
                                                                                                                #define PCI_HEADER_TYPE    0x0E
                                                                                                                #define PCI_BAR0           0x10
                                                                                                                #define PCI_BAR1           0x14

                                                                                                                typedef struct {
                                                                                                                    uint16_t vendor_id;
                                                                                                                        uint16_t device_id; 
                                                                                                                            uint8_t class_code;
                                                                                                                                uint8_t subclass;
                                                                                                                                    uint8_t prog_if;
                                                                                                                                        uint8_t revision;
                                                                                                                                            uint8_t bus;
                                                                                                                                                uint8_t slot;
                                                                                                                                                    uint8_t function;
                                                                                                                                                        uint32_t bars[6];
                                                                                                                                                        } pci_device_t;

                                                                                                                                                        void pci_init(void);
                                                                                                                                                        uint32_t pci_read_config_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
                                                                                                                                                        void pci_write_config_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value);
                                                                                                                                                        pci_device_t *pci_find_device(uint16_t vendor_id, uint16_t device_id);

                                                                                                                                                        #endif
                                                                                                                                                        ```

                                                                                                                                                        ## Next Steps for Development

                                                                                                                                                        1. **Start with System Calls** - This is the foundation for user applications
                                                                                                                                                        2. **Implement PCI Bus Driver** - Required for most modern hardware
                                                                                                                                                        3. **Add Mouse Support** - Essential for GUI interaction  
                                                                                                                                                        4. **Create Network Driver** - Choose RTL8139 for simplicity
                                                                                                                                                        5. **Build Network Stack** - Start with Ethernet, then ARP, IP, TCP
                                                                                                                                                        6. **Add Audio Support** - AC97 is widely supported
                                                                                                                                                        7. **Expand Applications** - File manager, calculator, terminal

                                                                                                                                                        ## Build Integration

                                                                                                                                                        Update your Makefile to include new source files:

                                                                                                                                                        ```makefile
                                                                                                                                                        # Additional source files
                                                                                                                                                        SYSCALL_C = kernel/syscalls.c kernel/syscall_table.c
                                                                                                                                                        ADVANCED_DRIVER_C = drivers/pci.c drivers/mouse.c drivers/rtl8139.c drivers/ac97.c
                                                                                                                                                        NETWORK_C = net/ethernet.c net/arp.c net/ip.c net/tcp.c net/udp.c
                                                                                                                                                        ADDITIONAL_FS_C = fs/ext2.c fs/procfs.c
                                                                                                                                                        NEW_APPS_C = apps/calculator/calc.c apps/terminal/terminal.c apps/filemgr/filemanager.c

                                                                                                                                                        # Update object files
                                                                                                                                                        SYSCALL_OBJ = $(SYSCALL_C:%.c=$(BUILDDIR)/%.o)
                                                                                                                                                        ADVANCED_DRIVER_OBJ = $(ADVANCED_DRIVER_C:%.c=$(BUILDDIR)/%.o)  
                                                                                                                                                        NETWORK_OBJ = $(NETWORK_C:%.c=$(BUILDDIR)/%.o)
                                                                                                                                                        ADDITIONAL_FS_OBJ = $(ADDITIONAL_FS_C:%.c=$(BUILDDIR)/%.o)
                                                                                                                                                        NEW_APPS_OBJ = $(NEW_APPS_C:%.c=$(BUILDDIR)/%.o)

                                                                                                                                                        # Add to ALL_OBJ
                                                                                                                                                        ALL_OBJ += $(SYSCALL_OBJ) $(ADVANCED_DRIVER_OBJ) $(NETWORK_OBJ) $(ADDITIONAL_FS_OBJ) $(NEW_APPS_OBJ)
                                                                                                                                                        ```

                                                                                                                                                        Your Maya OS project is already quite comprehensive! Focus on the critical components first to create a solid foundation, then expand to add more advanced features.