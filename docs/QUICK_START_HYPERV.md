# Quick Start: Booting HueOS on Hyper-V

## TL;DR - The Problem
Your HueOS ISO won't boot in Hyper-V because you're using a **Generation 2 VM** which requires UEFI. HueOS uses BIOS boot.

## TL;DR - The Solution
**Use a Generation 1 VM instead.** The kernel works perfectly.

---

## Step-by-Step: Create Hyper-V Generation 1 VM

### Option 1: PowerShell (Fastest)

Open PowerShell as Administrator and run:

```powershell
# Create the VM
New-VM -Name "HueOS" -Generation 1 -MemoryStartupBytes 512MB -NoVHD

# Attach the ISO (change path to your ISO location)
Add-VMDvdDrive -VMName "HueOS" -Path "C:\Path\To\hueos.iso"

# Set boot order
Set-VMBios -VMName "HueOS" -StartupOrder @('CD', 'IDE', 'LegacyNetworkAdapter', 'Floppy')

# Start the VM
Start-VM -Name "HueOS"

# Connect to VM
vmconnect localhost HueOS
```

### Option 2: Hyper-V Manager GUI

1. **Open Hyper-V Manager**
   - Press Windows+R
   - Type `virtmgmt.msc`
   - Press Enter

2. **Create New VM**
   - Click "New" → "Virtual Machine"
   - Click "Next"

3. **Name the VM**
   - Name: `HueOS`
   - Click "Next"

4. **⚠️ CRITICAL STEP: Select Generation**
   - Choose **"Generation 1"**
   - Do NOT choose Generation 2
   - Click "Next"

5. **Assign Memory**
   - Startup memory: 512 MB (or more)
   - Click "Next"

6. **Configure Networking**
   - Leave as "Not Connected" (optional)
   - Click "Next"

7. **Connect Virtual Hard Disk**
   - Select "Use an existing virtual hard disk" and skip
   - OR "Create a virtual hard disk" if you want persistence
   - Click "Next"

8. **Installation Options**
   - Select "Install an operating system from a bootable CD/DVD-ROM"
   - Choose "Image file (.iso)"
   - Browse to `hueos.iso`
   - Click "Next"

9. **Finish**
   - Review settings
   - Click "Finish"

10. **Start VM**
    - Right-click the VM → "Connect"
    - Click "Start"

## What You Should See

1. **GRUB Bootloader** (5 second timeout)
   ```
   GNU GRUB version 2.xx
   
   HueOS
   ```

2. **Kernel Boot Messages** (on black screen with white text)
   ```
   HueOS Kernel Starting...
   ========================
   Multiboot loader detected
   Initializing GDT...
   GDT initialized
   Initializing IDT...
   IDT initialized
   Initializing memory...
   Memory initialized
   Initializing Hyper-V...
   Hyper-V hypervisor detected
   Initializing Hyper-V hypercalls...
   Hyper-V hypercalls initialized
   ...
   HueOS kernel initialization complete!
   Kernel is now running...
   ```

3. **Kernel Running**
   - Kernel enters infinite halt loop
   - This is expected behavior for a minimal OS
   - Press Ctrl+Alt+← to release mouse/keyboard

## Troubleshooting

### VM Won't Boot / Black Screen

**Cause**: Using Generation 2 VM
**Fix**: Delete VM and recreate as Generation 1

### "No bootable device" Error

**Cause**: ISO not properly attached
**Fix**: 
1. Shut down VM
2. VM Settings → DVD Drive
3. Browse to hueos.iso
4. Apply and start VM

### Can't See Any Output

**Cause**: VM window might not be focused
**Fix**: Click in the VM window after starting

### Hyper-V Not Available

**Cause**: Hyper-V feature not enabled
**Fix**: 
1. Open PowerShell as Administrator
2. Run: `Enable-WindowsOptionalFeature -Online -FeatureName Microsoft-Hyper-V -All`
3. Reboot
4. Try again

## Why Generation 1?

| Feature | Generation 1 | Generation 2 |
|---------|--------------|--------------|
| Boot Type | BIOS/Legacy | UEFI only |
| HueOS Support | ✅ Yes | ❌ No (yet) |
| 32-bit OS | ✅ Yes | ❌ No |
| Multiboot | ✅ Yes | ❌ No |

HueOS uses:
- Multiboot specification (BIOS only)
- 32-bit protected mode
- Legacy boot sequence

Generation 2 VMs require:
- UEFI boot loader
- 64-bit OS (preferred)
- Different boot protocol

**Future versions** of HueOS may add UEFI support.

## Verify Hyper-V Detection

When HueOS boots in Hyper-V, it will detect the hypervisor and display:
```
Checking for Hyper-V support...
Hyper-V hypervisor detected
VP Runtime available
Partition Reference Time available
Basic SynIC MSRs available
Synthetic Timer MSRs available
```

This confirms the Hyper-V integration is working!

## Next Steps

Once booted successfully:
- The kernel runs in an infinite halt loop (expected)
- You can study the code to add more features
- Extend with device drivers, file systems, etc.

## Quick Reference

```bash
# Build the ISO
make iso

# Test in QEMU first
make qemu

# Check serial output
timeout 5s qemu-system-i386 -kernel build/hueos.bin -serial stdio

# Create Hyper-V VM (from WSL)
./create-hyperv-vm.sh
```

## Summary

✅ **HueOS works perfectly**  
✅ **Boots successfully on BIOS systems**  
✅ **Detects and integrates with Hyper-V**  
⚠️ **Must use Generation 1 VM**  
❌ **Generation 2 not supported yet**  

**Bottom line**: Your ISO is fine. Just use Generation 1 VM in Hyper-V.