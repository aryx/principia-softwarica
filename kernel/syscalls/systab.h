#include "/sys/src/libc/9syscall/sys.h"

typedef long Syscall(ulong*);

Syscall sysr1;
Syscall sys_errstr;
Syscall sysbind;
Syscall syschdir;
Syscall sysclose;
Syscall sysdup;
Syscall sysalarm;
Syscall sysexec;
Syscall sysexits;
Syscall sys_fsession;
Syscall sysfauth;
Syscall sys_fstat;
Syscall syssegbrk;
Syscall sys_mount;
Syscall sysopen;
Syscall sys_read;
Syscall sysoseek;
Syscall syssleep;
Syscall sys_stat;
Syscall sysrfork;
Syscall sys_write;
Syscall syspipe;
Syscall syscreate;
Syscall sysfd2path;
Syscall sysbrk_;
Syscall sysremove;
Syscall sys_wstat;
Syscall sys_fwstat;
Syscall sysnotify;
Syscall sysnoted;
Syscall syssegattach;
Syscall syssegdetach;
Syscall syssegfree;
Syscall syssegflush;
Syscall sysrendezvous;
Syscall sysunmount;
Syscall sys_wait;
Syscall syssemacquire;
Syscall syssemrelease;
Syscall sysseek;
Syscall sysfversion;
Syscall syserrstr;
Syscall sysstat;
Syscall sysfstat;
Syscall syswstat;
Syscall sysfwstat;
Syscall sysmount;
Syscall sysawait;
Syscall syspread;
Syscall syspwrite;
Syscall systsemacquire;
//@Scheck: TODO? dead?
Syscall sysdeath;

extern Syscall *systab[];
extern char *sysctab[];
extern int nsyscall;
