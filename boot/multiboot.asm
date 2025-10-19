; Multiboot header for GRUB2 bootloader
; This file defines the multiboot header required by GRUB2

section .multiboot
align 4

; Multiboot constants
MULTIBOOT_MAGIC     equ 0x1BADB002
MULTIBOOT_FLAGS     equ 0x00000003  ; Page align + memory info
MULTIBOOT_CHECKSUM  equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

; Multiboot header
dd MULTIBOOT_MAGIC
dd MULTIBOOT_FLAGS
dd MULTIBOOT_CHECKSUM

section .text
global _start
extern kernel_main

_start:
    ; Disable interrupts
    cli
    
    ; Set up stack
    mov esp, stack_top
    
    ; Push multiboot info for kernel
    push ebx    ; Multiboot info structure
    push eax    ; Multiboot magic number
    
    ; Call kernel main function directly (don't enable paging yet)
    call kernel_main
    
    ; Halt if kernel returns
    cli
.hang:
    hlt
    jmp .hang

section .bss
align 16
stack_bottom:
    resb 16384  ; 16 KB stack
stack_top: