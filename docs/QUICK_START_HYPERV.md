# Quick Start: Booting HueOS on Hyper-V

## TL;DR - The Solution

HueOS now supports **both Generation 1 (BIOS) and Generation 2 (UEFI)** virtual machines!

- **Generation 1 VM** - Traditional BIOS boot (fully tested)
- **Generation 2 VM** - Modern UEFI boot (with Secure Boot disabled)

Choose based on your preference. Generation 2 is recommended for modern systems.

---

## Step-by-Step: Create Hyper-V VM

### Option A: Generation 2 VM (UEFI - Recommended)

**PowerShell (Fastest):**

```powershell
# Create the VM
New-VM -Name "HueOS" -Generation 2 -MemoryStartupBytes 512MB -NoVHD

# Disable Secure Boot (required - kernel is not signed)
Set-VMFirmwareConfiguration -VMName "HueOS" -EnableSecureBoot Off

# Attach the ISO
Add-VMDvdDrive -VMName "HueOS" -Path "C:\Path\To\hueos.iso"

# Start the VM
Start-VM -Name "HueOS"

# Connect to VM
vmconnect localhost HueOS
```

**Hyper-V Manager GUI:**

1. Create New VM → Select **Generation 2**
2. Assign Memory: 512 MB minimum
3. Configure Networking: Optional
4. Installation Options: Mount `hueos.iso`
5. **Before Starting**: Settings → Security → **Disable Secure Boot**
6. Start the VM

**Hyper-V Manager GUI:**

1. Create New VM → Select **Generation 2**
2. Assign Memory: 512 MB minimum
3. Configure Networking: Optional
4. Installation Options: Mount `hueos.iso`
5. **Before Starting**: Settings → Security → **Disable Secure Boot**
6. Start the VM

### Option B: Generation 1 VM (BIOS - Legacy Support)

**PowerShell:**

```powershell
# Create the VM
New-VM -Name "HueOS" -Generation 1 -MemoryStartupBytes 512MB -NoVHD

# Attach the ISO
Add-VMDvdDrive -VMName "HueOS" -Path "C:\Path\To\hueos.iso"

# Set boot order
Set-VMBios -VMName "HueOS" -StartupOrder @('CD', 'IDE', 'LegacyNetworkAdapter', 'Floppy')

# Start the VM
Start-VM -Name "HueOS"

# Connect to VM
vmconnect localhost HueOS
```

**Hyper-V Manager GUI:**

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

4. **Select Generation**
   - Choose **"Generation 1"** for BIOS boot
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

## Storage Options

HueOS supports both IDE and SCSI storage:

### Add a Virtual Hard Disk

**For Generation 1 (IDE):**
```powershell
New-VHD -Path "C:\HyperV\HueOS\disk.vhdx" -SizeBytes 100MB -Dynamic
Add-VMHardDiskDrive -VMName "HueOS" -Path "C:\HyperV\HueOS\disk.vhdx"
```

**For Generation 2 (SCSI):**
```powershell
New-VHD -Path "C:\HyperV\HueOS\disk.vhdx" -SizeBytes 100MB -Dynamic
Add-VMScsiController -VMName "HueOS"
Add-VMHardDiskDrive -VMName "HueOS" -ControllerType SCSI -ControllerNumber 0 -Path "C:\HyperV\HueOS\disk.vhdx"
```

HueOS will automatically detect:
- **IDE/ATAPI drives** (Generation 1)
- **SCSI drives** (Generation 2)
- **CD/DVD drives** (both generations)

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
   Detecting hardware...
   Initializing IDE...
   IDE device detection complete
   Initializing SCSI...
   SCSI device scan complete
   ...
   HueOS kernel initialization complete!
   Kernel is now running...
   ```

3. **Kernel Running**
   - Kernel enters infinite halt loop
   - This is expected behavior for a minimal OS
   - Press Ctrl+Alt+← to release mouse/keyboard

## Troubleshooting

### Generation 2 VM Won't Boot

**Cause**: Secure Boot is enabled
**Fix**: 
1. Shut down VM
2. VM Settings → Security
3. Uncheck "Enable Secure Boot"
4. Apply and start VM

### VM Boots to Black Screen (Generation 2)

**Cause**: Secure Boot is enabled
**Fix**: 
1. Shut down VM
2. VM Settings → Security
3. Uncheck "Enable Secure Boot"
4. Apply and start VM

### VM Boots to Black Screen (Generation 2)

**Cause**: May need to wait for GRUB menu
**Fix**: Wait 5-10 seconds, GRUB menu should appear

### "No bootable device" Error

**Cause**: ISO not properly attached
**Fix**: 
1. Shut down VM
2. VM Settings → DVD Drive (or SCSI DVD for Gen 2)
3. Browse to hueos.iso
4. Apply and start VM

### No Storage Devices Detected

**Cause**: No virtual disk attached
**Fix**: Add a virtual hard disk using PowerShell commands above
- Generation 1: Uses IDE controller
- Generation 2: Uses SCSI controller

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

## Boot Mode Comparison

| Feature | Generation 1 | Generation 2 |
|---------|--------------|--------------|
| Boot Type | BIOS/Legacy | UEFI |
| HueOS Support | ✅ Yes | ✅ Yes (v2.0+) |
| 32-bit OS | ✅ Yes | ✅ Yes |
| Multiboot | ✅ Yes | ✅ Yes |
| Secure Boot | N/A | ⚠️ Must disable |
| Storage | IDE/ATAPI | SCSI |
| Speed | Normal | Faster boot |

HueOS now supports:
- **BIOS boot** (Multiboot specification)
- **UEFI boot** (GRUB2 EFI)
- **Hybrid ISO** (boots on both)

Both Generation 1 and Generation 2 VMs work correctly!

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

### Verify Storage Detection

**Generation 1 (IDE):**
```
IDE Devices:
============
Device 0: Primary Master - ATA HDD
  Model: Msft Virtual Disk
  Size: 100 MB
```

**Generation 2 (SCSI):**
```
SCSI Devices:
=============
Device 0: Target 0 - SCSI HDD
  Vendor: Msft
  Product: Virtual Disk
  Size: 100 MB
```

This confirms both Hyper-V integration and storage detection are working!

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

# Test UEFI boot in QEMU
make qemu-iso-uefi

# Create Hyper-V Generation 2 VM (from PowerShell)
New-VM -Name "HueOS" -Generation 2 -MemoryStartupBytes 512MB
Set-VMFirmwareConfiguration -VMName "HueOS" -EnableSecureBoot Off
Add-VMDvdDrive -VMName "HueOS" -Path "C:\Path\To\hueos.iso"
```

## Advanced: Verbose Mode

For detailed hardware information, select "HueOS (Verbose)" in the GRUB menu.

This displays:
- CPU information (vendor, model, features)
- PCI bus devices
- Memory map details
- Complete storage device information
- Extended Hyper-V features

## Summary

✅ **HueOS supports both VM generations**  
✅ **Generation 1**: BIOS boot with IDE storage  
✅ **Generation 2**: UEFI boot with SCSI storage  
✅ **Boots successfully on both types**  
✅ **Detects and integrates with Hyper-V**  
✅ **Automatic storage detection (IDE/SCSI)**  
⚠️ **Generation 2 requires Secure Boot disabled**  

**Recommended**: Use **Generation 2** for modern features and better performance.

**Bottom line**: HueOS works on both Generation 1 and Generation 2 Hyper-V VMs!