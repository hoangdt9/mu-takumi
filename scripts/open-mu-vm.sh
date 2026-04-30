#!/usr/bin/env bash
# Open BNS-2020 VMware VM. VM lives under takumi/VMWare.
# IMPORTANT: This .vmx is Intel (x86); it will NOT power on on Apple Silicon (ARM) — see docs/MU-SERVER-MAC-VMWARE.md .
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
VMX="${ROOT}/VMWare/BNS-2020.vmx"
if [[ ! -f "$VMX" ]]; then
	echo "Missing: $VMX" >&2
	exit 1
fi
for app in "/Applications/VMware Fusion.app" "/Applications/VMware Fusion Tech Preview.app"; do
	if [[ -d "$app" ]]; then
		open -a "$app" "$VMX"
		exit 0
	fi
done
echo "Install VMware Fusion (or Tech Preview); expected one of:" >&2
echo "  /Applications/VMware Fusion.app" >&2
echo "  /Applications/VMware Fusion Tech Preview.app" >&2
exit 1
