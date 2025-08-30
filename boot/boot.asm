\
;;; Maya OS Bootloader (Updated 2025-08-29)
;;; Author: AmanNagtodeOfficial
;;; Last Modified: 2025-08-29 10:52:34 UTC

MBOOT_PAGE_ALIGN    equ 1<<0
MBOOT_MEM_INFO      equ 1<<1
MBOOT_HEADER_MAGIC  equ 0x1BADB002
MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

; Added minimum memory requirement
MIN_MEMORY_KB       equ 1024  ; Require at least 1MB of memory

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
    ; Set up stack with bounds checking
    mov esp, stack_top
    cmp esp, stack_bottom
    jb stack_overflow_error
    
    ; Validate multiboot magic
    cmp eax, 0x2BADB002
    jne invalid_boot
    
    ; Check memory size
    mov eax, [ebx + 4]  ; mem_lower field from multiboot info
    cmp eax, MIN_MEMORY_KB
    jb insufficient_memory
    
    ; Save multiboot info
    push ebx    ; Multiboot info structure
    push eax    ; Multiboot magic number
    
    ; Initialize critical subsystems
    call gdt_flush
    call idt_install
    
    ; Enable interrupts
    sti
    
    ; Jump to kernel
    call kernel_main
    
    ; System halt if kernel returns
.halt:
    cli
    hlt
    jmp .halt

invalid_boot:
    mov esi, boot_error_msg
    jmp error_halt

insufficient_memory:
    mov esi, memory_error_msg
    jmp error_halt

stack_overflow_error:
    mov esi, stack_error_msg
    jmp error_halt

error_halt:
    ; Print error message through BIOS
    mov ah, 0x0E
.print_loop:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .print_loop
.done:
    cli
    hlt
    jmp .done

section .data
boot_error_msg db "ERROR: Invalid multiboot magic", 0
memory_error_msg db "ERROR: Insufficient memory", 0
stack_error_msg db "ERROR: Stack overflow", 0

section .bss
align 16
stack_bottom:
resb 32768  ; 32 KiB stack
stack_top:
