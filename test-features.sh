#!/bin/bash

# HueOS Feature Test Script
# Tests the new IDE and hardware detection features

echo "================================================"
echo "HueOS IDE & Hardware Detection Test Suite"
echo "================================================"
echo ""

cd "$(dirname "$0")"

# Test 1: Normal boot
echo "Test 1: Normal Boot"
echo "-------------------"
rm -f test1.log
timeout 5s qemu-system-i386 -kernel build/hueos.bin \
  -serial file:test1.log \
  -display none 2>/dev/null

if grep -q "HueOS Kernel Starting" test1.log; then
    echo "✓ Kernel boots successfully"
else
    echo "✗ Kernel failed to boot"
fi

if grep -q "IDE Devices" test1.log; then
    echo "✓ IDE driver initialized"
else
    echo "✗ IDE driver not initialized"
fi

echo ""

# Test 2: Verbose mode
echo "Test 2: Verbose Mode"
echo "--------------------"
rm -f test2.log
timeout 5s qemu-system-i386 -kernel build/hueos.bin \
  -append "verbose" \
  -serial file:test2.log \
  -display none 2>/dev/null

if grep -q "Verbose mode enabled" test2.log; then
    echo "✓ Verbose mode activated"
else
    echo "✗ Verbose mode not activated"
fi

if grep -q "CPU Information" test2.log; then
    echo "✓ CPU detection working"
else
    echo "✗ CPU detection not working"
fi

if grep -q "PCI Devices" test2.log; then
    echo "✓ PCI scanning working"
else
    echo "✗ PCI scanning not working"
fi

echo ""

# Test 3: IDE device detection
echo "Test 3: IDE Device Detection"
echo "-----------------------------"

# Create test disk if it doesn't exist
if [ ! -f test_disk.img ]; then
    dd if=/dev/zero of=test_disk.img bs=1M count=10 2>/dev/null
fi

rm -f test3.log
timeout 5s qemu-system-i386 -kernel build/hueos.bin \
  -hda test_disk.img \
  -serial file:test3.log \
  -display none 2>/dev/null

if grep -q "QEMU HARDDISK" test3.log; then
    echo "✓ ATA hard disk detected"
else
    echo "✗ ATA hard disk not detected"
fi

if grep -q "ATA HDD" test3.log || grep -q "ATA Hard Disk" test3.log; then
    echo "✓ Device type correctly identified"
else
    echo "✗ Device type not correctly identified"
fi

echo ""

# Test 4: CD-ROM detection
echo "Test 4: CD-ROM Detection"
echo "------------------------"
rm -f test4.log
timeout 5s qemu-system-i386 -kernel build/hueos.bin \
  -cdrom hueos.iso \
  -serial file:test4.log \
  -display none 2>/dev/null

if grep -q "QEMU DVD-ROM" test4.log; then
    echo "✓ ATAPI optical drive detected"
else
    echo "✗ ATAPI optical drive not detected"
fi

if grep -q "ATAPI CD/DVD" test4.log || grep -q "ATAPI Optical Drive" test4.log; then
    echo "✓ Drive type correctly identified"
else
    echo "✗ Drive type not correctly identified"
fi

echo ""

# Test 5: Multiple devices
echo "Test 5: Multiple Devices"
echo "-------------------------"
rm -f test5.log
timeout 5s qemu-system-i386 -kernel build/hueos.bin \
  -hda test_disk.img \
  -cdrom hueos.iso \
  -append "verbose" \
  -serial file:test5.log \
  -display none 2>/dev/null

device_count=$(grep -c "Type: \(ATA\|ATAPI\)" test5.log)
echo "✓ Detected $device_count device(s)"

if [ $device_count -ge 2 ]; then
    echo "✓ Multiple device detection working"
else
    echo "⚠ Expected 2+ devices, found $device_count"
fi

echo ""

# Test 6: ISO boot (GRUB menu)
echo "Test 6: ISO Boot with GRUB"
echo "---------------------------"
if [ -f hueos.iso ]; then
    echo "✓ ISO file exists"
    
    # Check GRUB config
    if [ -f isodir/boot/grub/grub.cfg ]; then
        menu_count=$(grep -c "menuentry" isodir/boot/grub/grub.cfg)
        echo "✓ GRUB config has $menu_count boot option(s)"
        
        if [ $menu_count -ge 2 ]; then
            echo "✓ Verbose boot option available"
        else
            echo "⚠ Only standard boot option found"
        fi
    else
        echo "⚠ GRUB config not found"
    fi
else
    echo "✗ ISO file not found"
fi

echo ""

# Summary
echo "================================================"
echo "Test Summary"
echo "================================================"
echo ""

total_tests=6
passed_tests=$(grep -c "✓" test*.log 2>/dev/null || echo 0)

echo "Kernel Size: $(ls -lh build/hueos.bin | awk '{print $5}')"
echo "ISO Size: $(ls -lh hueos.iso | awk '{print $5}')"
echo ""

# Show sample output from verbose mode
echo "Sample Output (Verbose Mode):"
echo "------------------------------"
if [ -f test5.log ]; then
    echo ""
    grep -A3 "IDE Devices:" test5.log | head -8
    echo ""
    grep -A2 "CPU Information:" test5.log | head -5
fi

echo ""
echo "================================================"
echo "All tests completed!"
echo "================================================"
echo ""
echo "To test manually:"
echo "  make qemu              # Normal boot"
echo "  make qemu-iso          # Boot from ISO with GRUB menu"
echo ""
echo "For verbose mode with devices:"
echo "  qemu-system-i386 -kernel build/hueos.bin \\"
echo "    -hda test_disk.img \\"
echo "    -cdrom hueos.iso \\"
echo "    -append \"verbose\""
echo ""

# Cleanup
rm -f test*.log 2>/dev/null