;;; Global Descriptor Table Setup

[BITS 32]
section .text

global gdt_flush
extern gp

gdt_flush:
    lgdt [gp]        ; Load GDT
        mov ax, 0x10     ; Data segment offset
            mov ds, ax
                mov es, ax
                    mov fs, ax
                        mov gs, ax
                            mov ss, ax
                                jmp 0x08:flush2  ; Code segment offset
                                flush2:
                                    ret
                                    