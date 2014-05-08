/*s: systab.c */
/*s: kernel basic includes */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "../port/error.h"
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
extern Syscall sysoseek;
extern Syscall syssleep;
extern Syscall sysrfork;
extern Syscall syspipe;
extern Syscall syscreate;
extern Syscall sysfd2path;
extern Syscall sysbrk_;
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

    [SLEEP]     syssleep,
    [ALARM]     sysalarm,
/*e: systab process syscalls */
/*s: systab file syscalls */
    [CREATE]    syscreate,
    [REMOVE]    sysremove,

    [OPEN]      sysopen,
    [CLOSE]     sysclose,
    [PREAD]     syspread,
    [PWRITE]    syspwrite,
    [SEEK]      sysseek,
    [OSEEK]     sysoseek,

    [STAT]      sysstat,
    [FSTAT]     sysfstat,
    [WSTAT]     syswstat,
    [FWSTAT]    sysfwstat,

    [CHDIR]     syschdir,
    [FD2PATH]   sysfd2path, // pwd?
/*e: systab file syscalls */
/*s: systab special file syscalls */
    [DUP]       sysdup,
    [PIPE]      syspipe,
/*e: systab special file syscalls */
/*s: systab namespace syscalls */
    [BIND]      sysbind,
    [MOUNT]     sysmount,
    [UNMOUNT]   sysunmount,
/*e: systab namespace syscalls */
/*s: systab concurrency syscalls */
    [AWAIT]     sysawait,
    [RENDEZVOUS]    sysrendezvous,

    [SEMACQUIRE]    syssemacquire,
    [SEMRELEASE]    syssemrelease,
    [TSEMACQUIRE]   systsemacquire,
/*e: systab concurrency syscalls */
/*s: systab note syscalls */
    [NOTIFY]    sysnotify,
    [NOTED]     sysnoted,
/*e: systab note syscalls */
/*s: systab memory syscalls */
    [BRK_]      sysbrk_,

    [SEGATTACH] syssegattach,
    [SEGDETACH] syssegdetach,
    [SEGFREE]   syssegfree,
    [SEGFLUSH]  syssegflush,
    [SEGBRK]    syssegbrk,
/*e: systab memory syscalls */
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
    [OSEEK]     "Oseek",
    [SLEEP]     "Sleep",
    [RFORK]     "Rfork",
    [PIPE]      "Pipe",
    [CREATE]    "Create",
    [FD2PATH]   "Fd2path",
    [BRK_]      "Brk",
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
