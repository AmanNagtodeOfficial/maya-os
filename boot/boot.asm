;;; Maya OS Bootloader
;;; Multiboot compliant boot sector

MBOOT_PAGE_ALIGN    equ 1<<0
MBOOT_MEM_INFO      equ 1<<1
MBOOT_HEADER_MAGIC  equ 0x1BADB002
MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

[BITS 32]

section .multiboot
align 4
    dd MBOOT_HEADER_MAGIC
        dd MBOOT_HEADER_FLAGS
            dd MBOOT_CHECKSUM

            section .text
            global _start
            extern kernel_main
            extern gdt_flush
            extern idt_install

            _start:
                ; Set up stack
                    mov esp, stack_top
                        
                            ; Push multiboot info to stack for kernel
                                push ebx    ; Multiboot info structure
                                    push eax    ; Multiboot magic number
                                        
                                            ; Initialize GDT and IDT
                                                call gdt_flush
                                                    call idt_install
                                                        
                                                            ; Enable interrupts
                                                                sti
                                                                    
                                                                        ; Call kernel main
                                                                            call kernel_main
                                                                                
                                                                                    ; Halt if kernel returns
                                                                                    .halt:
                                                                                        cli
                                                                                            hlt
                                                                                                jmp .halt

                                                                                                section .bss
                                                                                                align 16
                                                                                                stack_bottom:
                                                                                                resb 16384  ; 16 KiB stack
                                                                                                stack_top:
                                                                                                