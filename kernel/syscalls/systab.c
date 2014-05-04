#include    "u.h"
#include    "../port/lib.h"
#include    "mem.h"
#include    "dat.h"
#include    "fns.h"

#include "../port/systab.h"

extern Syscall sysr1;
extern Syscall sys_errstr;
extern Syscall sysbind;
extern Syscall syschdir;
extern Syscall sysclose;
extern Syscall sysdup;
extern Syscall sysalarm;
extern Syscall sysexec;
extern Syscall sysexits;
extern Syscall sys_fsession;
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
extern Syscall sys_wstat;
extern Syscall sys_fwstat;
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

//coupling: debuggers/acid/conf/syscall
//coupling: debuggers/acid/conf/truss
//TODO: remove obsolete calls, all _XXX, and maybe even reorder?
// also use an enum instead?
Syscall *systab[] = {
    [SYSR1]     sysr1,
    [_ERRSTR]   sys_errstr,
    [BIND]      sysbind,
    [CHDIR]     syschdir,
    [CLOSE]     sysclose,
    [DUP]       sysdup,
    [ALARM]     sysalarm,
    [EXEC]      sysexec,
    [EXITS]     sysexits,
    [_FSESSION] sys_fsession,
    [FAUTH]     sysfauth,
    [_FSTAT]    sys_fstat,
    [SEGBRK]    syssegbrk,
    [_MOUNT]    sys_mount,
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
    [_WSTAT]    sys_wstat,
    [_FWSTAT]   sys_fwstat,
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

char *sysctab[] = {
    [SYSR1]     "Running",
    [_ERRSTR]   "_errstr",
    [BIND]      "Bind",
    [CHDIR]     "Chdir",
    [CLOSE]     "Close",
    [DUP]       "Dup",
    [ALARM]     "Alarm",
    [EXEC]      "Exec",
    [EXITS]     "Exits",
    [_FSESSION] "_fsession",
    [FAUTH]     "Fauth",
    [_FSTAT]    "_fstat",
    [SEGBRK]    "Segbrk",
    [_MOUNT]    "_mount",
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
    [_WSTAT]    "_wstat",
    [_FWSTAT]   "_fwstat",
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
