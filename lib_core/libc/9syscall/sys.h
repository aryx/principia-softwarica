/*s: lib_core/libc/9syscall/sys.h */
#define NOP     0
/*s: [[sys.h]] process syscalls */
#define RFORK       1
#define EXEC        2
#define EXITS       3
#define AWAIT       4
/*e: [[sys.h]] process syscalls */
/*s: [[sys.h]] memory syscalls */
#define BRK     5
/*e: [[sys.h]] memory syscalls */
/*s: [[sys.h]] file syscalls */
#define OPEN        6
#define CLOSE       7
#define DUP         8
#define FD2PATH     9
#define PREAD       10
#define PWRITE      11
#define SEEK        12
/*e: [[sys.h]] file syscalls */
/*s: [[sys.h]] directory syscalls */
#define CREATE      13
#define REMOVE      14
#define CHDIR       15
#define STAT        16
#define FSTAT       17
#define WSTAT       18
#define FWSTAT      19
/*e: [[sys.h]] directory syscalls */
/*s: [[sys.h]] namespace syscalls */
#define BIND        20
#define MOUNT       21
#define UNMOUNT     22
/*e: [[sys.h]] namespace syscalls */
/*s: [[sys.h]] time syscalls */
#define SLEEP       23
#define ALARM       24
/*e: [[sys.h]] time syscalls */
/*s: [[sys.h]] ipc syscalls */
#define PIPE        27

#define NOTIFY      25
#define NOTED       26

#define SEGATTACH   28
#define SEGDETACH   29
#define SEGFREE     30
#define SEGFLUSH    31
#define SEGBRK      32
/*e: [[sys.h]] ipc syscalls */
/*s: [[sys.h]] concurrency syscalls */
#define RENDEZVOUS  33
#define SEMACQUIRE  34
#define SEMRELEASE  35
#define TSEMACQUIRE 36
/*e: [[sys.h]] concurrency syscalls */
/*s: [[sys.h]] security syscalls */
#define FVERSION    37
#define FAUTH       38
/*e: [[sys.h]] security syscalls */
#define ERRSTR      39
/*e: lib_core/libc/9syscall/sys.h */
