#!/bin/bash

# HueOS Build and Test Script
# This script helps build and test the HueOS kernel

set -e  # Exit on any error

echo "HueOS Build and Test Script"
echo "=========================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if required tools are installed
check_dependencies() {
    print_status "Checking dependencies..."
    
    local missing_tools=()
    
    if ! command -v gcc &> /dev/null; then
        missing_tools+=("gcc")
    fi
    
    if ! command -v nasm &> /dev/null; then
        missing_tools+=("nasm")
    fi
    
    if ! command -v ld &> /dev/null; then
        missing_tools+=("binutils/ld")
    fi
    
    if ! command -v make &> /dev/null; then
        missing_tools+=("make")
    fi
    
    if [ ${#missing_tools[@]} -gt 0 ]; then
        print_error "Missing required tools: ${missing_tools[*]}"
        echo "On Ubuntu/Debian, install with:"
        echo "  sudo apt-get install build-essential nasm"
        return 1
    fi
    
    # Check optional tools
    if ! command -v qemu-system-i386 &> /dev/null; then
        print_warning "qemu-system-i386 not found - cannot run tests"
        print_warning "Install with: sudo apt-get install qemu-system-x86"
    fi
    
    if ! command -v grub-mkrescue &> /dev/null; then
        print_warning "grub-mkrescue not found - cannot create ISO"
        print_warning "Install with: sudo apt-get install grub-pc-bin grub-common xorriso"
    fi
    
    print_status "Dependency check complete"
    return 0
}

# Build the kernel
build_kernel() {
    print_status "Building HueOS kernel..."
    
    if make clean && make; then
        print_status "Kernel built successfully"
        return 0
    else
        print_error "Kernel build failed"
        return 1
    fi
}

# Create ISO image
create_iso() {
    print_status "Creating bootable ISO..."
    
    if make iso; then
        print_status "ISO created successfully: hueos.iso"
        return 0
    else
        print_error "ISO creation failed"
        return 1
    fi
}

# Test in QEMU
test_qemu() {
    print_status "Testing kernel in QEMU..."
    print_status "Press Ctrl+Alt+G to release mouse, Ctrl+Alt+2 for QEMU monitor"
    print_status "Press CTRL+C to stop QEMU"
    
    if command -v qemu-system-i386 &> /dev/null; then
        make qemu
    else
        print_error "QEMU not available"
        return 1
    fi
}

# Test ISO in QEMU
test_iso() {
    print_status "Testing ISO in QEMU..."
    print_status "Press Ctrl+Alt+G to release mouse, Ctrl+Alt+2 for QEMU monitor"
    print_status "Press CTRL+C to stop QEMU"
    
    if command -v qemu-system-i386 &> /dev/null; then
        make qemu-iso
    else
        print_error "QEMU not available"
        return 1
    fi
}

# Main script logic
case "${1:-build}" in
    "deps"|"dependencies")
        check_dependencies
        ;;
    "build")
        check_dependencies && build_kernel
        ;;
    "iso")
        check_dependencies && build_kernel && create_iso
        ;;
    "test")
        check_dependencies && build_kernel && test_qemu
        ;;
    "test-iso")
        check_dependencies && build_kernel && create_iso && test_iso
        ;;
    "all")
        check_dependencies && build_kernel && create_iso
        ;;
    "clean")
        print_status "Cleaning build files..."
        make clean
        print_status "Clean complete"
        ;;
    "help"|"-h"|"--help")
        echo "Usage: $0 [command]"
        echo ""
        echo "Commands:"
        echo "  deps        - Check dependencies only"
        echo "  build       - Build kernel (default)"
        echo "  iso         - Build kernel and create ISO"
        echo "  test        - Build and test kernel in QEMU"
        echo "  test-iso    - Build, create ISO, and test in QEMU"
        echo "  all         - Build kernel and create ISO"
        echo "  clean       - Clean build files"
        echo "  help        - Show this help message"
        ;;
    *)
        print_error "Unknown command: $1"
        echo "Use '$0 help' for usage information"
        exit 1
        ;;
esac