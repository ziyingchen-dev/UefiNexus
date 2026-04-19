#!/bin/bash

ARCH=$1
WORKSPACE=/workspaces/UefiNexus
EDK_BUILD_DIR=/workspaces/edk2/Build/UefiNexusPkg

if [[ "$ARCH" != "x64" && "$ARCH" != "arm" ]]; then
    echo "Usage: ./qemu.sh <x64|arm>"
    exit 1
fi

# Logic for X64 (VNC Mode)
if [ "$ARCH" == "x64" ]; then
    RUN_DIR="$WORKSPACE/run-x64-usb"
    mkdir -p "$RUN_DIR"
    
    # Locate the X64 EFI binary
    APP_EFI=$(find "$EDK_BUILD_DIR" -path "*X64*" -name "UefiNexus.efi" | head -n 1)
    
    if [ -z "$APP_EFI" ]; then 
        echo "Error: X64 EFI binary not found. Please run build.sh first."
        exit 1
    fi
    cp "$APP_EFI" "$RUN_DIR/"

    if ! pgrep -x "websockify" > /dev/null; then
        echo "Starting noVNC bridge on Port 6080..."
        websockify --web=/usr/share/novnc 6080 localhost:5901 &
        sleep 2
    fi

    echo "Launching QEMU X64 (VNC mode)..."
    echo "Tip: Open Port 6080 in 'Ports' tab and click vnc.html"
    
    qemu-system-x86_64 \
      -machine q35 -m 512 \
      -bios /usr/share/ovmf/OVMF.fd \
      -hda fat:rw:"$RUN_DIR" \
      -net none \
      -vnc 127.0.0.1:1
# Logic for ARM (Updated to VNC Mode)
elif [ "$ARCH" == "arm" ]; then
    RUN_DIR="$WORKSPACE/run-aa64-usb"
    mkdir -p "$RUN_DIR"
    
    # Locate the AARCH64 EFI binary
    APP_EFI=$(find "$EDK_BUILD_DIR" -path "*AARCH64*" -name "UefiNexus.efi" | head -n 1)

    if [ -z "$APP_EFI" ]; then 
        echo "Error: ARM EFI binary not found. Please run build.sh first."
        exit 1
    fi
    cp "$APP_EFI" "$RUN_DIR/"

    # 啟動 noVNC 橋接（與 x64 邏輯一致）
    if ! pgrep -x "websockify" > /dev/null; then
        echo "Starting noVNC bridge on Port 6080..."
        websockify --web=/usr/share/novnc 6080 localhost:5901 &
        sleep 2
    fi

    echo "Launching QEMU AArch64 (VNC mode)..."
    echo "Tip: Open Port 6080 in 'Ports' tab and click vnc.html"
    
    qemu-system-aarch64 \
      -M virt -cpu cortex-a72 -m 1024 -smp 2 \
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

# # Logic for ARM (Nographic/Serial Mode)
# elif [ "$ARCH" == "arm" ]; then
#     RUN_DIR="$WORKSPACE/run-aa64-usb"
#     mkdir -p "$RUN_DIR"
    
#     # Locate the AARCH64 EFI binary
#     APP_EFI=$(find "$EDK_BUILD_DIR" -path "*AARCH64*" -name "UefiNexus.efi" | head -n 1)

#     if [ -z "$APP_EFI" ]; then 
#         echo "Error: ARM EFI binary not found. Please run build.sh first."
#         exit 1
#     fi
#     cp "$APP_EFI" "$RUN_DIR/"

#     echo "Launching QEMU AArch64 (Serial mode)..."
#     echo "Tip: Press 'Ctrl+A' then 'X' to exit the emulator."
    
#     qemu-system-aarch64 \
#       -M virt -cpu cortex-a72 -m 1024 -smp 2 \
#       -nographic \
#       -bios /usr/share/qemu-efi-aarch64/QEMU_EFI.fd \
#       -drive format=raw,file=fat:rw:"$RUN_DIR" \
#       -net none
# fi
