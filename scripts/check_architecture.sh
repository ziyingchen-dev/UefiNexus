#!/usr/bin/env bash
set -euo pipefail

if grep -R '#include.*Uefi' UefiNexusPkg/Core; then
    echo "ERROR: Core layer must not include UEFI headers"
    exit 1
fi

if grep -R 'gBS->' UefiNexusPkg/Core; then
    echo "ERROR: Core layer must not access gBS"
    exit 1
fi

if grep -R '#include.*UI/' UefiNexusPkg/Core; then
    echo "ERROR: Core layer must not depend on UI"
    exit 1
fi

echo "Architecture checks passed"
