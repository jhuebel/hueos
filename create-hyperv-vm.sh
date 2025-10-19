#!/bin/bash

# HueOS Hyper-V VM Creation Script
# This script helps create a properly configured Hyper-V Generation 1 VM for HueOS

echo "============================================"
echo "HueOS Hyper-V VM Creation Helper"
echo "============================================"
echo ""

# Check if running on Windows (WSL)
if ! command -v powershell.exe &> /dev/null; then
    echo "ERROR: This script must be run in WSL with access to PowerShell"
    echo "Alternatively, run the PowerShell commands manually in Windows."
    exit 1
fi

VM_NAME="HueOS-Test"
MEMORY_MB=512
ISO_PATH=$(wslpath -w "$(pwd)/hueos.iso")

echo "Configuration:"
echo "  VM Name: $VM_NAME"
echo "  Memory: ${MEMORY_MB}MB"
echo "  ISO Path: $ISO_PATH"
echo ""

read -p "Create Hyper-V VM with these settings? (y/n) " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Cancelled."
    exit 0
fi

echo ""
echo "Creating Hyper-V Generation 1 VM..."

# Create the VM using PowerShell
powershell.exe -Command "
    \$ErrorActionPreference = 'Stop'
    
    # Check if VM already exists
    if (Get-VM -Name '$VM_NAME' -ErrorAction SilentlyContinue) {
        Write-Host 'VM already exists. Removing old VM...'
        Stop-VM -Name '$VM_NAME' -Force -ErrorAction SilentlyContinue
        Remove-VM -Name '$VM_NAME' -Force
    }
    
    Write-Host 'Creating new Generation 1 VM...'
    New-VM -Name '$VM_NAME' -Generation 1 -MemoryStartupBytes ${MEMORY_MB}MB -NoVHD
    
    Write-Host 'Configuring VM...'
    # Add DVD drive with ISO
    Add-VMDvdDrive -VMName '$VM_NAME' -Path '$ISO_PATH'
    
    # Set boot order (DVD first)
    Set-VMBios -VMName '$VM_NAME' -StartupOrder @('CD', 'IDE', 'LegacyNetworkAdapter', 'Floppy')
    
    Write-Host ''
    Write-Host '============================================'
    Write-Host 'VM Created Successfully!'
    Write-Host '============================================'
    Write-Host 'VM Name: $VM_NAME'
    Write-Host 'Generation: 1 (BIOS/Legacy boot)'
    Write-Host 'Memory: ${MEMORY_MB}MB'
    Write-Host 'ISO Attached: Yes'
    Write-Host ''
    Write-Host 'To start the VM, run:'
    Write-Host '  Start-VM -Name $VM_NAME'
    Write-Host '  vmconnect localhost $VM_NAME'
    Write-Host ''
    Write-Host 'Or use Hyper-V Manager GUI'
    Write-Host '============================================'
"

if [ $? -eq 0 ]; then
    echo ""
    echo "SUCCESS! VM created and configured."
    echo ""
    echo "Next steps:"
    echo "1. Open Hyper-V Manager (virtmgmt.msc)"
    echo "2. Find the VM named: $VM_NAME"
    echo "3. Right-click → Connect"
    echo "4. Click Start"
    echo ""
    echo "You should see GRUB boot menu, then HueOS kernel messages."
else
    echo ""
    echo "ERROR: Failed to create VM."
    echo "Please ensure:"
    echo "  1. You are running as Administrator"
    echo "  2. Hyper-V is enabled"
    echo "  3. You have permission to create VMs"
fi