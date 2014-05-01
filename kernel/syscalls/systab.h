// used by systab.c but also by trap.c for certain codes
#include "/sys/src/libc/9syscall/sys.h"

typedef long Syscall(ulong*);

extern Syscall *systab[];
extern int nsyscall;
extern char *sysctab[];
