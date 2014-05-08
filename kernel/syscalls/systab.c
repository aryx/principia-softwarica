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
extern Syscall sys_fstat;
extern Syscall syssegbrk;
extern Syscall sys_mount;
extern Syscall sysopen;
extern Syscall sys_read;
extern Syscall sysoseek;
extern Syscall syssleep;
extern Syscall sys_stat;
extern Syscall sysrfork;
extern Syscall sys_write;
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
extern Syscall sys_wait;
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
    [BIND]      sysbind,
    [CHDIR]     syschdir,
    [CLOSE]     sysclose,
    [DUP]       sysdup,
    [ALARM]     sysalarm,
    [EXEC]      sysexec,
    [EXITS]     sysexits,
    [FAUTH]     sysfauth,
    [_FSTAT]    sys_fstat,
    [SEGBRK]    syssegbrk,
    [OPEN]      sysopen,
    [_READ]     sys_read,
    [OSEEK]     sysoseek,
    [SLEEP]     syssleep,
    [_STAT]     sys_stat,
    [RFORK]     sysrfork,
    [_WRITE]    sys_write,
    [PIPE]      syspipe,
    [CREATE]    syscreate,
    [FD2PATH]   sysfd2path,
    [BRK_]      sysbrk_,
    [REMOVE]    sysremove,
    [NOTIFY]    sysnotify,
    [NOTED]     sysnoted,
    [SEGATTACH] syssegattach,
    [SEGDETACH] syssegdetach,
    [SEGFREE]   syssegfree,
    [SEGFLUSH]  syssegflush,
    [RENDEZVOUS]    sysrendezvous,
    [UNMOUNT]   sysunmount,
    [_WAIT]     sys_wait,
    [SEMACQUIRE]    syssemacquire,
    [SEMRELEASE]    syssemrelease,
    [SEEK]      sysseek,
    [FVERSION]  sysfversion,
    [ERRSTR]    syserrstr,
    [STAT]      sysstat,
    [FSTAT]     sysfstat,
    [WSTAT]     syswstat,
    [FWSTAT]    sysfwstat,
    [MOUNT]     sysmount,
    [AWAIT]     sysawait,
    [PREAD]     syspread,
    [PWRITE]    syspwrite,
    [TSEMACQUIRE]   systsemacquire,
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
    [_FSTAT]    "_fstat",
    [SEGBRK]    "Segbrk",
    [OPEN]      "Open",
    [_READ]     "_read",
    [OSEEK]     "Oseek",
    [SLEEP]     "Sleep",
    [_STAT]     "_stat",
    [RFORK]     "Rfork",
    [_WRITE]    "_write",
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
    [_WAIT]     "_wait",
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
