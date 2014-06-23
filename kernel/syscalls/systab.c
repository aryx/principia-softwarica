/*s: systab.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

#include "../port/systab.h"

/*s: systab.c forward decl */
extern Syscall sysnop;
extern Syscall sysbind;
extern Syscall syschdir;
extern Syscall sysclose;
extern Syscall sysdup;
extern Syscall sysalarm;
extern Syscall sysexec;
extern Syscall sysexits;
extern Syscall sysfauth;
extern Syscall syssegbrk;
extern Syscall sysopen;
extern Syscall syssleep;
extern Syscall sysrfork;
extern Syscall syspipe;
extern Syscall syscreate;
extern Syscall sysfd2path;
extern Syscall sysbrk;
extern Syscall sysremove;
extern Syscall sysnotify;
extern Syscall sysnoted;
extern Syscall syssegattach;
extern Syscall syssegdetach;
extern Syscall syssegfree;
extern Syscall syssegflush;
extern Syscall sysrendezvous;
extern Syscall sysunmount;
extern Syscall syssemacquire;
extern Syscall syssemrelease;
extern Syscall sysseek;
extern Syscall sysfversion;
extern Syscall syserrstr;
extern Syscall sysstat;
extern Syscall sysfstat;
extern Syscall syswstat;
extern Syscall sysfwstat;
extern Syscall sysmount;
extern Syscall sysawait;
extern Syscall syspread;
extern Syscall syspwrite;
extern Syscall systsemacquire;
//@Scheck: TODO? dead?
extern Syscall sysdeath;
/*e: systab.c forward decl */

//coupling: debuggers/acid/conf/syscall
//coupling: debuggers/acid/conf/truss
/*s: global systab */
Syscall *systab[] = {
    [NOP]     sysnop,
/*s: systab process syscalls */
    [RFORK]     sysrfork,
    [EXEC]      sysexec,
    [EXITS]     sysexits,
    [AWAIT]     sysawait,
/*e: systab process syscalls */
/*s: systab memory syscalls */
    [BRK]      sysbrk,
/*e: systab memory syscalls */
/*s: systab file syscalls */
    [OPEN]      sysopen,
    [CLOSE]     sysclose,
    [PREAD]     syspread,
    [PWRITE]    syspwrite,
    [SEEK]      sysseek,
/*e: systab file syscalls */
/*s: systab directory syscalls */
    [CREATE]    syscreate,
    [REMOVE]    sysremove,

    [CHDIR]     syschdir,
    [FD2PATH]   sysfd2path, // =~ pwd

    [STAT]      sysstat,
    [FSTAT]     sysfstat,
    [WSTAT]     syswstat,
    [FWSTAT]    sysfwstat,
/*e: systab directory syscalls */
/*s: systab namespace syscalls */
    [BIND]      sysbind,
    [MOUNT]     sysmount,
    [UNMOUNT]   sysunmount,
/*e: systab namespace syscalls */
/*s: systab time syscalls */
    [SLEEP]     syssleep,
    [ALARM]     sysalarm,
/*e: systab time syscalls */
/*s: systab ipc syscalls */
    [NOTIFY]    sysnotify,
    [NOTED]     sysnoted,

    [PIPE]      syspipe,

    [SEGATTACH] syssegattach,
    [SEGDETACH] syssegdetach,
    [SEGFREE]   syssegfree,
    [SEGFLUSH]  syssegflush,
    [SEGBRK]    syssegbrk,
/*e: systab ipc syscalls */
/*s: systab concurrency syscalls */
    [RENDEZVOUS]    sysrendezvous,

    [SEMACQUIRE]    syssemacquire,
    [SEMRELEASE]    syssemrelease,
    [TSEMACQUIRE]   systsemacquire,
/*e: systab concurrency syscalls */
/*s: systab special file syscalls */
    [DUP]       sysdup,
/*e: systab special file syscalls */
/*s: systab security syscalls */
    [FAUTH]     sysfauth,
    [FVERSION]  sysfversion,
/*e: systab security syscalls */
    [ERRSTR]    syserrstr,
};
int nsyscall = nelem(systab);
/*e: global systab */

/*s: global sysstab */
char *sysctab[] = {
    [NOP]     "Nop",
    [BIND]      "Bind",
    [CHDIR]     "Chdir",
    [CLOSE]     "Close",
    [DUP]       "Dup",
    [ALARM]     "Alarm",
    [EXEC]      "Exec",
    [EXITS]     "Exits",
    [FAUTH]     "Fauth",
    [SEGBRK]    "Segbrk",
    [OPEN]      "Open",
    [SLEEP]     "Sleep",
    [RFORK]     "Rfork",
    [PIPE]      "Pipe",
    [CREATE]    "Create",
    [FD2PATH]   "Fd2path",
    [BRK]      "Brk",
    [REMOVE]    "Remove",
    [NOTIFY]    "Notify",
    [NOTED]     "Noted",
    [SEGATTACH] "Segattach",
    [SEGDETACH] "Segdetach",
    [SEGFREE]   "Segfree",
    [SEGFLUSH]  "Segflush",
    [RENDEZVOUS]    "Rendez",
    [UNMOUNT]   "Unmount",
    [SEMACQUIRE]    "Semacquire",
    [SEMRELEASE]    "Semrelease",
    [SEEK]      "Seek",
    [FVERSION]  "Fversion",
    [ERRSTR]    "Errstr",
    [STAT]      "Stat",
    [FSTAT]     "Fstat",
    [WSTAT]     "Wstat",
    [FWSTAT]    "Fwstat",
    [MOUNT]     "Mount",
    [AWAIT]     "Await",
    [PREAD]     "Pread",
    [PWRITE]    "Pwrite",
    [TSEMACQUIRE]   "Tsemacquire",
};
/*e: global sysstab */
/*e: systab.c */
