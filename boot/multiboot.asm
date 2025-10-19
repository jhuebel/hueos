; Multiboot header for GRUB2 bootloader
; This file defines the multiboot header required by GRUB2

section .multiboot
align 4

; Multiboot constants
MULTIBOOT_MAGIC     equ 0x1BADB002
MULTIBOOT_FLAGS     equ 0x00000007  ; Page align + memory info + video mode
MULTIBOOT_CHECKSUM  equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

; Multiboot header
dd MULTIBOOT_MAGIC
dd MULTIBOOT_FLAGS
dd MULTIBOOT_CHECKSUM

; Address fields (required for aout kludge, must be 0 for ELF)
dd 0                    ; header_addr (0 = using ELF, not aout kludge)
dd 0                    ; load_addr
dd 0                    ; load_end_addr
dd 0                    ; bss_end_addr
dd 0                    ; entry_addr

; Video mode fields (used when flag bit 2 is set)
dd 1                    ; mode_type (1 = text mode, 0 = graphics)
dd 0                    ; width (0 = no preference)
dd 0                    ; height (0 = no preference)
dd 0                    ; depth (0 = no preference)

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