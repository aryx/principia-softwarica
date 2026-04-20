#!/usr/bin/env python3
# kernel_loc_table.py — Recompute the LOC table in kernel/Kernel.nw
# (\section{Code organization}, Table~\ref{tab:kernel-code-orga}).
#
# Usage:
#   ./kernel_loc_table.py                    # assumes kernel/ at repo root
#   ./kernel_loc_table.py <path-to-kernel>   # explicit kernel directory
#
# For each subsystem directory it reports:
#   Port  = .c/.h/.s/.ha files directly under the subsystem dir (portable)
#   ARM   = all .c/.h/.s/.ha files under <subsys>/arm/    (9pi port)
#   x86   = all .c/.h/.s/.ha files under <subsys>/386/    (x86 port)
#   User  = all .c/.h/.s/.ha files under <subsys>/user/   (userspace helpers)
#
# The devices/ and network/ rows aggregate their sub-trees (devices/audio,
# network/ip, ...). devices/screen/ is kept on its own row since the Graphics
# book covers it. The conf/ row counts every file (build scripts like master,
# mkdevc, mkfile, ...), not just .c/.h.

import os
import sys

SRC_EXT = ('.c', '.h', '.s', '.ha')


def count_file(path):
    try:
        with open(path, 'rb') as f:
            return sum(1 for _ in f)
    except OSError:
        return 0


def flat_loc(dirpath, exts=SRC_EXT):
    """Lines in source files directly under dirpath (non-recursive)."""
    if not os.path.isdir(dirpath):
        return 0
    n = 0
    for name in os.listdir(dirpath):
        fp = os.path.join(dirpath, name)
        if os.path.isfile(fp) and (exts is None or name.endswith(exts)):
            n += count_file(fp)
    return n


def rec_loc(dirpath, exts=SRC_EXT):
    """Lines in source files anywhere under dirpath (recursive)."""
    if not os.path.isdir(dirpath):
        return 0
    n = 0
    for root, _, files in os.walk(dirpath):
        for name in files:
            if exts is None or name.endswith(exts):
                n += count_file(os.path.join(root, name))
    return n


def split(dirpath, exts=SRC_EXT):
    """Return (port, arm, x86, user) LOC for one subsystem directory."""
    return (
        flat_loc(dirpath, exts),
        rec_loc(os.path.join(dirpath, 'arm'), exts),
        rec_loc(os.path.join(dirpath, '386'), exts),
        rec_loc(os.path.join(dirpath, 'user'), exts),
    )


def aggregate(dirs, exts=SRC_EXT):
    """Sum split() across multiple subsystem directories."""
    acc = [0, 0, 0, 0]
    for d in dirs:
        for i, v in enumerate(split(d, exts)):
            acc[i] += v
    return tuple(acc)


# One entry per row in Table~\ref{tab:kernel-code-orga}:
#   (label, [dirs to aggregate], extensions)
ROWS = [
    ('core/',           ['core'],         SRC_EXT),
    ('concurrency/',    ['concurrency'],  SRC_EXT),
    ('memory/',         ['memory'],       SRC_EXT),
    ('processes/',      ['processes'],    SRC_EXT),
    ('interrupts/',     ['interrupts'],   SRC_EXT),
    ('syscalls/',       ['syscalls'],     SRC_EXT),
    ('time/',           ['time'],         SRC_EXT),
    ('init/',           ['init'],         SRC_EXT),
    ('files/',          ['files'],        SRC_EXT),
    ('devices/',        ['devices', 'devices/audio', 'devices/keyboard',
                         'devices/mouse', 'devices/storage', 'devices/sys'],
                                          SRC_EXT),
    ('devices/screen/', ['devices/screen'], SRC_EXT),
    ('buses/',          ['buses'],        SRC_EXT),
    ('console/',        ['console'],      SRC_EXT),
    ('filesystems/',    ['filesystems'],  SRC_EXT),
    ('network/',        ['network', 'network/ip'], SRC_EXT),
    ('arch/',           ['arch'],         SRC_EXT),
    ('lib/, misc/',     ['lib', 'misc'],  SRC_EXT),
    ('security/',       ['security'],     SRC_EXT),
    # conf/ is build scripts (master, mkdevc, mkfile, ...), not .c/.h.
    ('conf/',           ['conf'],         None),
]


def main():
    if len(sys.argv) >= 2:
        kernel_dir = sys.argv[1]
    else:
        here = os.path.dirname(os.path.abspath(__file__))
        # default: <repo>/kernel, with this script at <repo>/docs/scripts
        kernel_dir = os.path.normpath(os.path.join(here, '..', '..', 'kernel'))

    if not os.path.isdir(kernel_dir):
        sys.exit(f"error: kernel directory not found: {kernel_dir}")
    os.chdir(kernel_dir)
    print(f"# Kernel root: {kernel_dir}\n")

    header = f"{'Directory':<18} {'Port':>7} {'ARM':>7} {'x86':>7} {'User':>7} {'Total9pi':>9}"
    print(header)
    print('-' * len(header))
    tot = [0, 0, 0, 0]
    for name, dirs, exts in ROWS:
        p, a, x, u = aggregate(dirs, exts)
        total_9pi = p + a + u  # ARM port total, x86 excluded
        print(f"{name:<18} {p:>7} {a:>7} {x:>7} {u:>7} {total_9pi:>9}")
        tot[0] += p; tot[1] += a; tot[2] += x; tot[3] += u
    print('-' * len(header))
    p, a, x, u = tot
    print(f"{'Total':<18} {p:>7} {a:>7} {x:>7} {u:>7} {p + a + u:>9}")

    print()
    print(f"Plan 9 kernel for 9pi (Port + ARM + User):   {p + a + u:>6}")
    print(f"  Port + ARM (no userspace):                 {p + a:>6}")
    print(f"  Port only (portable C, no arch):           {p:>6}")
    print(f"x86 code (omitted from main table):          {x:>6}")

    # "Kernel proper" = discount network and graphics stack.
    net = aggregate(['network', 'network/ip'])
    gfx = aggregate(['devices/screen'])
    kp = p - net[0] - gfx[0]
    ka = a - net[1] - gfx[1]
    ku = u - net[3] - gfx[3]
    print()
    print("Discounting network/ and devices/screen/ (graphics):")
    print(f"  Port:                                      {kp:>6}")
    print(f"  ARM:                                       {ka:>6}")
    print(f"  User:                                      {ku:>6}")
    print(f"  Port + ARM (pure kernel, 9pi):             {kp + ka:>6}")


if __name__ == '__main__':
    main()
