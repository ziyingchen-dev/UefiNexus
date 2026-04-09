
# UefiNexus

## Getting Started: 6 Steps to Dual-Arch UEFI Development

### Step 1: Prepare the Build Environment

#### Goal
Set up a Linux host (or GitHub Codespace) capable of cross-compiling EDK II-based UEFI applications for both **X64** and **AArch64**.

#### 1. Install Required Packages
Run the following command to install the necessary build tools and emulators:

```bash
sudo apt update
sudo apt install -y \
  git build-essential uuid-dev iasl nasm \
  python3 python3-venv python-is-python3 \
  qemu-system-x86 qemu-system-arm qemu-utils \
  gcc-aarch64-linux-gnu binutils-aarch64-linux-gnu
```

#### 2. Clone EDK II Repository
We use EDK II as the base framework. Make sure to initialize the submodules.

```bash
cd /workspaces
git clone https://github.com/tianocore/edk2.git
cd /workspaces/edk2
git submodule update --init --recursive
```

#### 3. Build BaseTools
Compile the required C-based tools for the EDK II build system:

```bash
make -C BaseTools
```

#### 4. Configure Environment Variables
To enable cross-compilation and link your project, run these commands in your terminal (or add them to your `~/.bashrc`):

```bash
source edksetup.sh
export GCC_AARCH64_PREFIX=aarch64-linux-gnu-  # Required for ARM64 build
export PACKAGES_PATH=/workspaces/UefiNexus     # Path to your project
```

### Step 2: Create a Minimal UEFI Application Skeleton

#### Goal
Establish a buildable project structure for `UefiNexus` that:
* Compiles as a standalone UEFI Application.
* Supports both **X64** and **AArch64** architectures.
* Prints a simple startup message to the UEFI console.

#### Why this approach?
The EDK II build system is metadata-driven. Before implementing features like PCI or Memory browsing, we must satisfy the dependency tree with a valid Package (**DEC**), Platform build description (**DSC**), and Module definition (**INF**).

#### 1. Initialize the Directory Structure
Execute these commands from your repository root (`/workspaces/UefiNexus`):

```bash
cd /workspaces/UefiNexus
mkdir -p UefiNexusPkg/Application/UefiNexus
mkdir -p UefiNexusPkg/Include
```

#### 2. Define the Package (.dec)
The **DEC** file declares the package's GUID and makes your include headers available to other modules.

```bash
cat > UefiNexusPkg/UefiNexusPkg.dec <<'EOF'
[Defines]
  DEC_SPECIFICATION              = 0x0001001A
  PACKAGE_NAME                   = UefiNexusPkg
  PACKAGE_GUID                   = 9f4b8a11-52f7-4f7f-9e02-4d9b1b5d1001
  PACKAGE_VERSION                = 0.1

[Includes]
  Include
EOF
```

#### 3. Create the Platform Build Description (.dsc)
The **DSC** file is the "recipe" for the build. It maps abstract Library Classes to specific `.inf` implementations and lists the components to be compiled. 

> **Note**: For ARM/AArch64 support, we include `CompilerIntrinsicsLib` to handle low-level math and memory operations.

```bash
cat > UefiNexusPkg/UefiNexusPkg.dsc <<'EOF'
[Defines]
  PLATFORM_NAME                  = UefiNexusPkg
  PLATFORM_GUID                  = e1a3d2c0-6dd3-4d5d-bf8e-2b0f0f5b2001
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x0001001A
  OUTPUT_DIRECTORY               = Build/UefiNexusPkg
  SUPPORTED_ARCHITECTURES        = X64|AARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT

[LibraryClasses]
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  DebugLib|MdePkg/Library/UefiDebugLibConOut/UefiDebugLibConOut.inf
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  RegisterFilterLib|MdePkg/Library/RegisterFilterLibNull/RegisterFilterLibNull.inf
  StackCheckLib|MdePkg/Library/StackCheckLibNull/StackCheckLibNull.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf

[LibraryClasses.AARCH64]
  CompilerIntrinsicsLib|MdePkg/Library/CompilerIntrinsicsLib/CompilerIntrinsicsLib.inf

[Components]
  UefiNexusPkg/Application/UefiNexus/UefiNexus.inf
EOF
```

#### 4. Write the Source Code (.c)
This is the entry point of your application.

```bash
cat > UefiNexusPkg/Application/UefiNexus/UefiNexus.c <<'EOF'
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>

EFI_STATUS
EFIAPI
UefiNexusMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  Print(L"UefiNexus: minimal app started.\n");
  return EFI_SUCCESS;
}
EOF
```

#### 5. Define the Module (.inf)
The **INF** file describes how to compile the source code and which packages/libraries it depends on.

```bash
cat > UefiNexusPkg/Application/UefiNexus/UefiNexus.inf <<'EOF'
[Defines]
  INF_VERSION                    = 0x0001001A
  BASE_NAME                      = UefiNexus
  FILE_GUID                      = 7d2d8f13-6d51-4df2-9c58-91a5f0cf3001
  MODULE_TYPE                    = UEFI_APPLICATION
  VERSION_STRING                 = 0.1
  ENTRY_POINT                    = UefiNexusMain

[Sources]
  UefiNexus.c

[Packages]
  MdePkg/MdePkg.dec
  UefiNexusPkg/UefiNexusPkg.dec

[LibraryClasses]
  UefiApplicationEntryPoint
  UefiLib

[LibraryClasses.AARCH64]
  CompilerIntrinsicsLib
EOF
```

#### Summary Checklist
Before proceeding to the build step, verify your file structure:

* [ ] `UefiNexusPkg/UefiNexusPkg.dec`
* [ ] `UefiNexusPkg/UefiNexusPkg.dsc`
* [ ] `UefiNexusPkg/Application/UefiNexus/UefiNexus.inf`
* [ ] `UefiNexusPkg/Application/UefiNexus/UefiNexus.c`

This is a solid **Step 3**. You've correctly identified the shift from the legacy `GCC5` tag to the generic `GCC` tag in modern EDK II, which is a common stumbling block for developers following older tutorials.

Since your goal is to support both **X64** and **AArch64**, let's refine this step to ensure the build instructions are clear and the environment is properly validated for a cross-platform workflow. 

### Step 3: Build for X64 Architecture

#### Goal
Compile the minimal `UefiNexus` application for the **X64** (Intel/AMD 64-bit) architecture.

#### Why the `GCC` tag?
In recent EDK II releases, the legacy `GCC5` toolchain tag has been deprecated or removed in favor of the generic `GCC` tag. The build system now automatically detects the installed version of GCC on your Linux host.

#### 1. Prepare the Environment
Before triggering the build, ensure your `PACKAGES_PATH` includes your project directory so the build system can locate `UefiNexusPkg`.

```bash
cd /workspaces/edk2
source edksetup.sh

# Link your project to the EDK II build system
export PACKAGES_PATH=/workspaces/UefiNexus
```

#### 2. Execute the Build Command
We will use the `build` tool to target the X64 architecture using the `DEBUG` profile.

```bash
build \
  -p UefiNexusPkg/UefiNexusPkg.dsc \
  -a X64 \
  -t GCC \
  -b DEBUG
```

**Breakdown of flags:**
* `-p`: Points to the Platform Description file (**DSC**).
* `-a`: Sets the target Architecture (**X64**).
* `-t`: Selects the Toolchain tag (**GCC**).
* `-b`: Sets the Build target (**DEBUG** for extra symbols/logs).

#### 3. Locate the Output
Upon success, your UEFI executable (`.efi`) will be generated in the `Build/` directory.

```bash
find Build/UefiNexusPkg -name "UefiNexus.efi"
```

### Step 4 - Run X64 in GitHub Codespaces

#### Goal
Boot the **OVMF** (Open Virtual Machine Firmware) in QEMU, enter the built-in UEFI Shell, and manually launch `UefiNexus.efi`.

#### Why this method?
Using the **UEFI Shell** is ideal for firmware utility development because:
* **Flexibility**: You can launch the `.efi` file manually without renaming it to `BOOTX64.EFI`.
* **Real-world Testing**: It mimics how a technician would use your tool on a physical server.
* **NoVNC Bridge**: Since GitHub Codespaces doesn't support raw VNC ports in the browser, we use **noVNC** to bridge the graphical output to a web-friendly format.

#### 1. Prepare the Virtual Drive
We will create a directory that QEMU will treat as a FAT-formatted USB drive.

```bash
cd /workspaces/UefiNexus
mkdir -p run-x64-usb

# Find your compiled binary and copy it to the virtual drive
APP_EFI=$(find /workspaces/edk2/Build/UefiNexusPkg -name "UefiNexus.efi" | head -n 1)
cp "$APP_EFI" run-x64-usb/
```

#### 2. Launch QEMU (Terminal 1)
Run the following command to start the virtual machine in the background with VNC enabled. 

> **Tip**: We use the simplified `-bios` flag for easier environment setup.

```bash
qemu-system-x86_64 \
  -machine q35 -m 512 \
  -bios /usr/share/ovmf/OVMF.fd \
  -hda fat:rw:run-x64-usb \
  -net none \
  -display none \
  -vnc 127.0.0.1:1
```

#### 3. Start the Web Bridge (Terminal 2)
Open a **second terminal** to install and run noVNC. This converts the VNC traffic into a format your browser can display.

```bash
# Install noVNC if not already present
sudo apt update && sudo apt install -y novnc python3-websockify

# Start the bridge on port 6080
websockify --web=/usr/share/novnc 6080 localhost:5901
```

#### 4. Access the UI
1. In the **Ports** tab of your Codespace, locate port `6080`.
2. Click the **Open in Browser** icon (the small globe).
3. In the new tab, click the **vnc.html**.
4. Once the UEFI Shell appears, type the following to run your app:

```efi
fs0:
UefiNexus.efi
```

### Step 5 - Build AArch64

#### Goal
Cross-compile the `UefiNexus` application for the **AArch64** (ARM 64-bit) architecture using the generic GCC toolchain.

#### Why
In modern EDK II environments (like Ubuntu 24.04 in Codespaces), the build system has consolidated many GCC versions under the single **`GCC`** tag. To successfully cross-compile, we must point this tag to the ARM compiler using the `GCC_AARCH64_PREFIX` environment variable.

#### 1. Configure the Cross-Compiler
Before building, set the prefix so EDK II knows to use `aarch64-linux-gnu-gcc` instead of the local x86 host compiler.

```bash
cd /workspaces/edk2
source edksetup.sh

# Essential: Link your project and set the correct ARM prefix
export PACKAGES_PATH=/workspaces/UefiNexus
export GCC_AARCH64_PREFIX=aarch64-linux-gnu-

# Double-check the compiler is found
which aarch64-linux-gnu-gcc
```

#### 2. Execute the Build
Use the `-t GCC` flag along with the AArch64 architecture.

```bash
build \
  -p UefiNexusPkg/UefiNexusPkg.dsc \
  -a AARCH64 \
  -t GCC \
  -b DEBUG
```

#### 3. Verify the Output
If the build succeeds, your AArch64 binary will be located here:
`Build/UefiNexusPkg/DEBUG_GCC/AARCH64/UefiNexus.efi`

### Step 6 - Run AArch64 in GitHub Codespaces

#### Goal
Execute the AArch64 version of `UefiNexus.efi` using the QEMU ARM emulator and interact via the serial console.

#### Why this method?
* **Efficiency**: AArch64 systems often boot to a serial console by default. Using `-nographic` allows you to stay within the Codespaces terminal without needing noVNC.
* **Platform Standard**: The `virt` machine is the standard for ARM virtual machines, supported by the `ArmVirtPkg` firmware in EDK II.

#### 1. Prepare the AArch64 Virtual Drive
We’ll isolate the ARM64 files in their own directory to avoid architecture confusion.

```bash
cd /workspaces/UefiNexus
mkdir -p run-aa64-usb

# Find the AArch64 binary (ensuring it's the one from the AARCH64 build folder)
APP_EFI=$(find /workspaces/edk2/Build -path "*AARCH64*" -type f -name "UefiNexus.efi" | head -n 1)
cp "$APP_EFI" run-aa64-usb/
```

#### 2. Launch QEMU AArch64
Since we are using `-nographic`, the QEMU output will appear directly in your current terminal.

```bash
qemu-system-aarch64 \
  -M virt \
  -cpu cortex-a72 \
  -m 1024 \
  -smp 2 \
  -nographic \
  -bios /usr/share/qemu-efi-aarch64/QEMU_EFI.fd \
  -drive format=raw,file=fat:rw:run-aa64-usb \
  -net none
```

#### 3. Usage & Exit the Emulator
* Launch: Once the shell appears, type fs0: then UefiNexus.efi.
* Exit: When you are finished testing, use the QEMU exit sequence, press **`Ctrl+A`** then **`X`**.
