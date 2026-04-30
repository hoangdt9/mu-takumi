#!/usr/bin/env python3
"""
Patch ELF PT_LOAD segment alignment from 4KB (0x1000) to 16KB (0x4000)
for Android 15+ compatibility.
"""
import struct
import os
import sys
import shutil

PAGE_16KB = 0x4000

def patch_elf(path):
    with open(path, 'rb') as f:
        data = bytearray(f.read())

    # Check ELF magic
    if data[:4] != b'\x7fELF':
        print(f"  SKIP (not ELF): {path}")
        return False

    ei_class = data[4]  # 1=32bit, 2=64bit
    ei_data  = data[5]  # 1=LE, 2=BE
    endian   = '<' if ei_data == 1 else '>'

    if ei_class == 2:  # 64-bit
        # ELF64 header: e_phoff at offset 32, e_phentsize at 54, e_phnum at 56
        e_phoff     = struct.unpack_from(endian + 'Q', data, 32)[0]
        e_phentsize = struct.unpack_from(endian + 'H', data, 54)[0]
        e_phnum     = struct.unpack_from(endian + 'H', data, 56)[0]
        # Phdr64: p_type(4) p_flags(4) p_offset(8) p_vaddr(8) p_paddr(8) p_filesz(8) p_memsz(8) p_align(8)
        p_align_off = 48
    else:  # 32-bit
        # ELF32 header: e_phoff at offset 28, e_phentsize at 42, e_phnum at 44
        e_phoff     = struct.unpack_from(endian + 'I', data, 28)[0]
        e_phentsize = struct.unpack_from(endian + 'H', data, 42)[0]
        e_phnum     = struct.unpack_from(endian + 'H', data, 44)[0]
        # Phdr32: p_type(4) p_offset(4) p_vaddr(4) p_paddr(4) p_filesz(4) p_memsz(4) p_flags(4) p_align(4)
        p_align_off = 28

    PT_LOAD = 1
    patched = 0

    for i in range(e_phnum):
        phdr_off = e_phoff + i * e_phentsize
        p_type = struct.unpack_from(endian + 'I', data, phdr_off)[0]
        if p_type != PT_LOAD:
            continue

        align_offset = phdr_off + p_align_off
        if ei_class == 2:
            p_align = struct.unpack_from(endian + 'Q', data, align_offset)[0]
        else:
            p_align = struct.unpack_from(endian + 'I', data, align_offset)[0]

        if p_align < PAGE_16KB:
            print(f"  PT_LOAD #{i}: align 0x{p_align:x} -> 0x{PAGE_16KB:x}")
            if ei_class == 2:
                struct.pack_into(endian + 'Q', data, align_offset, PAGE_16KB)
            else:
                struct.pack_into(endian + 'I', data, align_offset, PAGE_16KB)
            patched += 1

    if patched > 0:
        # Backup original
        shutil.copy2(path, path + '.bak')
        with open(path, 'wb') as f:
            f.write(data)
        print(f"  [PATCHED] {patched} PT_LOAD segment(s): {os.path.basename(path)}")
        return True
    else:
        print(f"  [OK] Already aligned: {os.path.basename(path)}")
        return False

def main():
    jni_dirs = [
        r"D:\backupspmu\onlinesp\64bit\MuMain-main\android\app\src\main\jniLibs\arm64-v8a",
        r"D:\backupspmu\onlinesp\64bit\MuMain-main\android\app\src\main\jniLibs\armeabi-v7a",
        r"D:\backupspmu\onlinesp\64bit\MuMain-main\android\app\src\main\jniLibs\x86",
    ]

    total = 0
    for d in jni_dirs:
        if not os.path.isdir(d):
            continue
        print(f"\nDIR: {d}")
        for f in sorted(os.listdir(d)):
            if f.endswith('.so'):
                full = os.path.join(d, f)
                if patch_elf(full):
                    total += 1

    print(f"\nDone! Patched {total} file(s).")

if __name__ == '__main__':
    main()
