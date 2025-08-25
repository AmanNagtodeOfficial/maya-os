;;; Interrupt Descriptor Table Setup

[BITS 32]
section .text

global idt_load
global isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7
global isr8, isr9, isr10, isr11, isr12, isr13, isr14, isr15
global isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23
global isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31
global irq0, irq1, irq2, irq3, irq4, irq5, irq6, irq7
global irq8, irq9, irq10, irq11, irq12, irq13, irq14, irq15

idt_load:
    lidt [idtp]
        ret

        ; ISR macros
        %macro ISR_NOERRCODE 1
        isr%1:
            cli
                push byte 0
                    push byte %1
                        jmp isr_common_stub
                        %endmacro

                        %macro ISR_ERRCODE 1
                        isr%1:
                            cli
                                push byte %1
                                    jmp isr_common_stub
                                    %endmacro

                                    %macro IRQ 2
                                    irq%1:
                                        cli
                                            push byte 0
                                                push byte %2
                                                    jmp irq_common_stub
                                                    %endmacro

                                                    ; ISRs without error codes
                                                    ISR_NOERRCODE 0
                                                    ISR_NOERRCODE 1
                                                    ISR_NOERRCODE 2
                                                    ISR_NOERRCODE 3
                                                    ISR_NOERRCODE 4
                                                    ISR_NOERRCODE 5
                                                    ISR_NOERRCODE 6
                                                    ISR_NOERRCODE 7
                                                    ISR_ERRCODE   8
                                                    ISR_NOERRCODE 9
                                                    ISR_ERRCODE   10
                                                    ISR_ERRCODE   11
                                                    ISR_ERRCODE   12
                                                    ISR_ERRCODE   13
                                                    ISR_ERRCODE   14
                                                    ISR_NOERRCODE 15
                                                    ISR_NOERRCODE 16
                                                    ISR_NOERRCODE 17
                                                    ISR_NOERRCODE 18
                                                    ISR_NOERRCODE 19
                                                    ISR_NOERRCODE 20
                                                    ISR_NOERRCODE 21
                                                    ISR_NOERRCODE 22
                                                    ISR_NOERRCODE 23
                                                    ISR_NOERRCODE 24
                                                    ISR_NOERRCODE 25
                                                    ISR_NOERRCODE 26
                                                    ISR_NOERRCODE 27
                                                    ISR_NOERRCODE 28
                                                    ISR_NOERRCODE 29
                                                    ISR_NOERRCODE 30
                                                    ISR_NOERRCODE 31

                                                    ; IRQs
                                                    IRQ 0, 32
                                                    IRQ 1, 33
                                                    IRQ 2, 34
                                                    IRQ 3, 35
                                                    IRQ 4, 36
                                                    IRQ 5, 37
                                                    IRQ 6, 38
                                                    IRQ 7, 39
                                                    IRQ 8, 40
                                                    IRQ 9, 41
                                                    IRQ 10, 42
                                                    IRQ 11, 43
                                                    IRQ 12, 44
                                                    IRQ 13, 45
                                                    IRQ 14, 46
                                                    IRQ 15, 47

                                                    extern isr_handler
                                                    isr_common_stub:
                                                        pusha
                                                            push ds
                                                                push es
                                                                    push fs
                                                                        push gs
                                                                            mov ax, 0x10
                                                                                mov ds, ax
                                                                                    mov es, ax
                                                                                        mov fs, ax
                                                                                            mov gs, ax
                                                                                                mov eax, esp
                                                                                                    push eax
                                                                                                        mov eax, isr_handler
                                                                                                            call eax
                                                                                                                pop eax
                                                                                                                    pop gs
                                                                                                                        pop fs
                                                                                                                            pop es
                                                                                                                                pop ds
                                                                                                                                    popa
                                                                                                                                        add esp, 8
                                                                                                                                            iret

                                                                                                                                            extern irq_handler
                                                                                                                                            irq_common_stub:
                                                                                                                                                pusha
                                                                                                                                                    push ds
                                                                                                                                                        push es
                                                                                                                                                            push fs
                                                                                                                                                                push gs
                                                                                                                                                                    mov ax, 0x10
                                                                                                                                                                        mov ds, ax
                                                                                                                                                                            mov es, ax
                                                                                                                                                                                mov fs, ax
                                                                                                                                                                                    mov gs, ax
                                                                                                                                                                                        mov eax, esp
                                                                                                                                                                                            push eax
                                                                                                                                                                                                mov eax, irq_handler
                                                                                                                                                                                                    call eax
                                                                                                                                                                                                        pop eax
                                                                                                                                                                                                            pop gs
                                                                                                                                                                                                                pop fs
                                                                                                                                                                                                                    pop es
                                                                                                                                                                                                                        pop ds
                                                                                                                                                                                                                            popa
                                                                                                                                                                                                                                add esp, 8
                                                                                                                                                                                                                                    iret

                                                                                                                                                                                                                                    section .data
                                                                                                                                                                                                                                    idtp:
                                                                                                                                                                                                                                        dw 2048
                                                                                                                                                                                                                                            dd 0
                                                                                                                                                                                                                                            