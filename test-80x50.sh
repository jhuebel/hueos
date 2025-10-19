#!/bin/bash
echo "Testing 80x50 mode..."
echo ""
echo "Expected behavior:"
echo "- Characters should be half the height (8 pixels instead of 16)"
echo "- You should see 50 lines of text (double the normal 25)"
echo "- Font should be readable but smaller"
echo ""
echo "Starting QEMU in 3 seconds..."
sleep 3
qemu-system-i386 -cdrom hueos.iso
