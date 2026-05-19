#!/bin/bash

set -e

ARCH=$1

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

WORKSPACE="$REPO_ROOT"
EDK_BUILD_DIR="$REPO_ROOT/edk2/Build/UefiNexusPkg"

if [[ "$ARCH" != "x64" && "$ARCH" != "arm" ]]; then
    echo "Usage: ./scripts/qemu.sh <x64|arm>"
    exit 1
fi

# X64
if [ "$ARCH" == "x64" ]; then
    RUN_DIR="$WORKSPACE/run-x64-usb"
    mkdir -p "$RUN_DIR"

    APP_EFI=$(find "$EDK_BUILD_DIR" -path "*X64*" -name "UefiNexus.efi" | head -n 1)

    if [ -z "$APP_EFI" ]; then
        echo "Error: X64 EFI binary not found."
        echo "Run ./scripts/build.sh x64 first."
        exit 1
    fi

    cp "$APP_EFI" "$RUN_DIR/"

    if ! pgrep -x "websockify" > /dev/null; then
        echo "Starting noVNC bridge on port 6080..."
        websockify --web=/usr/share/novnc 6080 localhost:5901 &
        sleep 2
    fi

    echo "Launching QEMU X64..."
    echo "Open: http://localhost:6080/vnc.html"

    qemu-system-x86_64 \
        -machine q35 \
        -m 512 \
        -bios /usr/share/ovmf/OVMF.fd \
        -hda fat:rw:"$RUN_DIR" \
        -net none \
        -vnc 127.0.0.1:1
fi

# ARM
if [ "$ARCH" == "arm" ]; then
    RUN_DIR="$WORKSPACE/run-aa64-usb"
    mkdir -p "$RUN_DIR"

    APP_EFI=$(find "$EDK_BUILD_DIR" -path "*AARCH64*" -name "UefiNexus.efi" | head -n 1)

    if [ -z "$APP_EFI" ]; then
        echo "Error: ARM EFI binary not found."
        echo "Run ./scripts/build.sh arm first."
        exit 1
    fi

    cp "$APP_EFI" "$RUN_DIR/"

    if ! pgrep -x "websockify" > /dev/null; then
        echo "Starting noVNC bridge on port 6080..."
        websockify --web=/usr/share/novnc 6080 localhost:5901 &
        sleep 2
    fi

    echo "Launching QEMU AArch64..."
    echo "Open: http://localhost:6080/vnc.html"

    qemu-system-aarch64 \
        -M virt \
        -cpu cortex-a72 \
        -m 1024 \
        -smp 2 \
        -bios /usr/share/qemu-efi-aarch64/QEMU_EFI.fd \
        -drive format=raw,file=fat:rw:"$RUN_DIR" \
        -net none \
        -vnc 127.0.0.1:1 \
        -device virtio-gpu-pci \
        -device qemu-xhci \
        -device usb-kbd \
        -device usb-tablet \
        -display none
fi