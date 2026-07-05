#!/usr/bin/env python3
# claude: this script was written by Claude Code (2026-07-05)
"""Parse a Plan 9 a.out symbol table and resolve addresses to functions.

Usage: aout_syms.py <kernel-with-symtab> <hex-addr> [<hex-addr> ...]

The shipped 9pi kernel is linked -H6 (headerless, no symbols); relink the
same objects without -H6 to get a symbol-carrying twin (see
docs/qemu_arm_debug.txt), then feed it PCs sampled from QEMU's
'info registers' to turn them into function+offset.
"""
import struct, sys

f = open(sys.argv[1], 'rb')
hdr = struct.unpack('>8I', f.read(32))
magic, text, data, bss, syms, entry, spsz, pcsz = hdr
f.seek(32 + text + data)
symdata = f.read(syms)

funcs = []  # (addr, name)
i = 0
while i + 5 <= len(symdata):
    val = struct.unpack('>I', symdata[i:i+4])[0]
    typ = symdata[i+4]
    i += 5
    if not (typ & 0x80):
        break
    typ &= 0x7f
    c = chr(typ)
    # name: NUL-terminated; 'z'/'Z' (file/frame) have special encoding
    if c in ('z', 'Z'):
        i += 1  # skip fname count byte? actually pairs of u16 until 0
        while i + 1 < len(symdata):
            if symdata[i] == 0 and symdata[i+1] == 0:
                i += 2
                break
            i += 2
        continue
    j = symdata.index(b'\0', i)
    name = symdata[i:j].decode('utf-8', 'replace')
    i = j + 1
    if c in ('T', 't', 'L', 'l'):
        funcs.append((val, name))

funcs.sort()
for addr_s in sys.argv[2:]:
    a = int(addr_s, 16)
    import bisect
    k = bisect.bisect_right(funcs, (a, '\xff')) - 1
    if 0 <= k < len(funcs):
        fa, fn = funcs[k]
        print(f"{addr_s} -> {fn} +0x{a-fa:x}")
    else:
        print(f"{addr_s} -> ??")
