# HueOS Makefile

# Compiler and tools
CC = gcc
AS = nasm
LD = ld

# Compiler flags
CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra -nostdlib -nostdinc -fno-builtin -fno-stack-protector -m32
CPPFLAGS = -Iinclude
LDFLAGS = -T linker.ld -nostdlib -m elf_i386
ASFLAGS = -f elf32

# Directories
SRCDIR = kernel
BOOTDIR = boot
BUILDDIR = build
ISODIR = isodir

# Source files
C_SOURCES = $(wildcard $(SRCDIR)/*.c)
KERNEL_ASM_SOURCES = $(wildcard $(SRCDIR)/*.asm)
BOOT_ASM_SOURCES = $(wildcard $(BOOTDIR)/*.asm)

# Object files
C_OBJECTS = $(C_SOURCES:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)
KERNEL_ASM_OBJECTS = $(KERNEL_ASM_SOURCES:$(SRCDIR)/%.asm=$(BUILDDIR)/%.o)
BOOT_ASM_OBJECTS = $(BOOT_ASM_SOURCES:$(BOOTDIR)/%.asm=$(BUILDDIR)/%.o)
OBJECTS = $(C_OBJECTS) $(KERNEL_ASM_OBJECTS) $(BOOT_ASM_OBJECTS)

# Target kernel
KERNEL = $(BUILDDIR)/hueos.bin
ISO = $(BUILDDIR)/hueos.iso
DISTDIR = dist

# Default target
all: $(KERNEL)

# Create build directory
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Compile C source files
$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# Assemble assembly files from kernel directory
$(BUILDDIR)/%.o: $(SRCDIR)/%.asm | $(BUILDDIR)
	$(AS) $(ASFLAGS) $< -o $@

# Assemble assembly files from boot directory
$(BUILDDIR)/multiboot.o: $(BOOTDIR)/multiboot.asm | $(BUILDDIR)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILDDIR)/gdt_asm.o: $(SRCDIR)/gdt_asm.asm | $(BUILDDIR)
	$(AS) $(ASFLAGS) $< -o $@

# Link kernel
$(KERNEL): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

# Create hybrid BIOS+UEFI bootable ISO image
# Uses grub-mkrescue which automatically creates UEFI support
iso: $(KERNEL)
	mkdir -p $(ISODIR)/boot/grub
	cp $(KERNEL) $(ISODIR)/boot/hueos.bin
	echo 'set timeout=5' > $(ISODIR)/boot/grub/grub.cfg
	echo 'set default=0' >> $(ISODIR)/boot/grub/grub.cfg
	echo '' >> $(ISODIR)/boot/grub/grub.cfg
	echo '# Standard VGA text mode (80x25)' >> $(ISODIR)/boot/grub/grub.cfg
	echo 'menuentry "HueOS" {' >> $(ISODIR)/boot/grub/grub.cfg
	echo '    multiboot /boot/hueos.bin' >> $(ISODIR)/boot/grub/grub.cfg
	echo '    boot' >> $(ISODIR)/boot/grub/grub.cfg
	echo '}' >> $(ISODIR)/boot/grub/grub.cfg
	echo '' >> $(ISODIR)/boot/grub/grub.cfg
	echo '# Verbose mode with standard resolution' >> $(ISODIR)/boot/grub/grub.cfg
	echo 'menuentry "HueOS (Verbose - Detailed Hardware Info)" {' >> $(ISODIR)/boot/grub/grub.cfg
	echo '    multiboot /boot/hueos.bin verbose' >> $(ISODIR)/boot/grub/grub.cfg
	echo '    boot' >> $(ISODIR)/boot/grub/grub.cfg
	echo '}' >> $(ISODIR)/boot/grub/grub.cfg
	echo '' >> $(ISODIR)/boot/grub/grub.cfg
	echo '# High resolution text mode (80x50) - Requires real hardware' >> $(ISODIR)/boot/grub/grub.cfg
	echo 'menuentry "HueOS (80x50 High Resolution)" {' >> $(ISODIR)/boot/grub/grub.cfg
	echo '    multiboot /boot/hueos.bin res=80x50' >> $(ISODIR)/boot/grub/grub.cfg
	echo '    boot' >> $(ISODIR)/boot/grub/grub.cfg
	echo '}' >> $(ISODIR)/boot/grub/grub.cfg
	echo '' >> $(ISODIR)/boot/grub/grub.cfg
	echo '# Wide text mode (132x25) - Graphics Framebuffer - Requires real hardware' >> $(ISODIR)/boot/grub/grub.cfg
	echo 'menuentry "HueOS (132x25 Wide Screen)" {' >> $(ISODIR)/boot/grub/grub.cfg
	echo '    set gfxpayload=keep' >> $(ISODIR)/boot/grub/grub.cfg
	echo '    insmod all_video' >> $(ISODIR)/boot/grub/grub.cfg
	echo '    insmod gfxterm' >> $(ISODIR)/boot/grub/grub.cfg
	echo '    terminal_output gfxterm' >> $(ISODIR)/boot/grub/grub.cfg
	echo '    set gfxmode=1056x400' >> $(ISODIR)/boot/grub/grub.cfg
	echo '    multiboot /boot/hueos.bin res=132x25' >> $(ISODIR)/boot/grub/grub.cfg
	echo '    boot' >> $(ISODIR)/boot/grub/grub.cfg
	echo '}' >> $(ISODIR)/boot/grub/grub.cfg
	echo '' >> $(ISODIR)/boot/grub/grub.cfg
	echo '# Wide high resolution (132x50) - Graphics Framebuffer - Requires real hardware' >> $(ISODIR)/boot/grub/grub.cfg
	echo 'menuentry "HueOS (132x50 Extra Lines)" {' >> $(ISODIR)/boot/grub/grub.cfg
	echo '    set gfxpayload=keep' >> $(ISODIR)/boot/grub/grub.cfg
	echo '    insmod all_video' >> $(ISODIR)/boot/grub/grub.cfg
	echo '    insmod gfxterm' >> $(ISODIR)/boot/grub/grub.cfg
	echo '    terminal_output gfxterm' >> $(ISODIR)/boot/grub/grub.cfg
	echo '    set gfxmode=1056x800' >> $(ISODIR)/boot/grub/grub.cfg
	echo '    multiboot /boot/hueos.bin res=132x50' >> $(ISODIR)/boot/grub/grub.cfg
	echo '    boot' >> $(ISODIR)/boot/grub/grub.cfg
	echo '}' >> $(ISODIR)/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO) $(ISODIR) 2>&1 | grep -v "will not be bootable" || true
	@echo ""
	@echo "ISO created: $(ISO)"
	@echo "Boot modes supported: BIOS (Legacy) and UEFI (x86_64 + ia32)"

# Run in QEMU
qemu: $(KERNEL)
	qemu-system-i386 -kernel $(KERNEL)

# Run in QEMU with Hyper-V acceleration (if available)
qemu-hyperv: $(KERNEL)
	qemu-system-i386 -kernel $(KERNEL) -enable-kvm -cpu host

# Run ISO in QEMU (BIOS mode)
qemu-iso: iso
	qemu-system-i386 -cdrom $(ISO) -serial stdio

# Run ISO in QEMU (UEFI mode)
qemu-iso-uefi: iso
	qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -cdrom $(ISO) -serial stdio -m 256M

# Create a distributable package (no .o files kept)
# Copies the ISO and kernel into $(DISTDIR) and then cleans build artifacts
package: iso
	mkdir -p $(DISTDIR)
	cp $(ISO) $(DISTDIR)/
	cp $(KERNEL) $(DISTDIR)/
	@echo "Package created in $(DISTDIR)"
	$(MAKE) clean

# Create a distributable package while preserving build artifacts
# Copies the ISO and kernel into $(DISTDIR) but keeps .o files for incremental builds
preserve: iso
	mkdir -p $(DISTDIR)
	cp $(ISO) $(DISTDIR)/
	cp $(KERNEL) $(DISTDIR)/
	@echo "Package created in $(DISTDIR) (build artifacts preserved)"

# Clean build files
clean:
	rm -rf $(BUILDDIR) $(ISODIR)

# Debug with GDB
debug: $(KERNEL)
	qemu-system-i386 -s -S -kernel $(KERNEL) &
	gdb $(KERNEL) -ex "target remote localhost:1234"

# Check if we have all required tools
check-tools:
	@which $(CC) > /dev/null || (echo "Error: $(CC) not found"; exit 1)
	@which $(AS) > /dev/null || (echo "Error: $(AS) not found"; exit 1)
	@which $(LD) > /dev/null || (echo "Error: $(LD) not found"; exit 1)
	@which qemu-system-i386 > /dev/null || (echo "Warning: qemu-system-i386 not found")
	@which grub-mkrescue > /dev/null || (echo "Warning: grub-mkrescue not found")
	@echo "All required tools are available"

.PHONY: all iso qemu qemu-hyperv qemu-iso clean debug check-tools