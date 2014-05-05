/*s: systab.h */
// used by systab.c but also by trap.c for certain codes
#include "/sys/src/libc/9syscall/sys.h"

/*s: typedef Syscall */
typedef long Syscall(ulong*);
/*e: typedef Syscall */

extern Syscall *systab[];
extern int nsyscall;
extern char *sysctab[];
/*e: systab.h */
