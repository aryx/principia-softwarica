/*s: machine/5i/syscall.c */
/*s: basic includes */
#include <u.h>
#include <libc.h>
#include <bio.h>
#include <mach.h>

#include "arm.h"
/*e: basic includes */

//#define	ODIRLEN	116	/* compatibility; used in _stat etc. */
/*s: constant [[OERRLEN]] */
#define	OERRLEN	64	/* compatibility; used in _stat etc. */
/*e: constant [[OERRLEN]] */

/*s: global [[errbuf]] */
char 	errbuf[ERRMAX];
/*e: global [[errbuf]] */
/*s: global [[nofunc]] */
ulong	nofunc;
/*e: global [[nofunc]] */

#include "../../lib_core/libc/9syscall/sys.h"

/*s: global [[sysctab]] */
char*	sysctab[] =
{
    [NOP]		"Nop",
    [BIND]		"Bind",
    [CHDIR]		"Chdir",
    [CLOSE]		"Close",
    [DUP]		"Dup",
    [ALARM]		"Alarm",
    [EXEC]		"Exec",
    [EXITS]		"Exits",
    [FAUTH]		"Fauth",
    [SEGBRK]	"Segbrk",
    [MOUNT]		"Mount",
    [OPEN]		"Open",
    [SLEEP]		"Sleep",
    [RFORK]		"Rfork",
    [PIPE]		"Pipe",
    [CREATE]	"Create",
    [FD2PATH]	"Fd2path",
    [BRK]		"Brk",
    [REMOVE]	"Remove",
    [NOTIFY]	"Notify",
    [NOTED]		"Noted",
    [SEGATTACH]		"Segattach",
    [SEGDETACH]		"Segdetach",
    [SEGFREE]		"Segfree",
    [SEGFLUSH]		"Segflush",
    [RENDEZVOUS]	"Rendezvous",
    [UNMOUNT]		"Unmount",
    [SEEK]		"Seek",
    [FVERSION]	"Fversion",
    [ERRSTR]	"Errstr",
    [STAT]		"Stat",
    [FSTAT]		"Fstat",
    [WSTAT]		"Wstat",
    [FWSTAT]	"Fwstat",
    [PREAD]		"Pread",
    [PWRITE]	"Pwrite",
    [AWAIT]		"Await",
};
/*e: global [[sysctab]] */

/*s: function [[sysnop]] */
void
sysnop(void)
{
    Bprint(bout, "nop system call %s\n", sysctab[reg.r[1]]);
    /*s: [[sysnop]] strace */
    if(sysdbg)
        itrace("nop()");
    /*e: [[sysnop]] strace */
}
/*e: function [[sysnop]] */

/*s: function [[syserrstr]] */
void
syserrstr(void)
{
    ulong str;
    int n;

    str = getmem_w(reg.r[REGSP]+4);
    n = getmem_w(reg.r[REGSP]+8);
    if(sysdbg)
        itrace("errstr(0x%lux, 0x%lux)", str, n);

    if(n > strlen(errbuf)+1)
        n = strlen(errbuf)+1;
    memio(errbuf, str, n, MemWrite);
    strcpy(errbuf, "no error");
    reg.r[REGRET] = n;
    
}
/*e: function [[syserrstr]] */
/*s: function [[sysbind]] */
void
sysbind(void)
{ 
    ulong pname, pold, flags;
    char name[1024], old[1024];
    int n;

    pname = getmem_w(reg.r[REGSP]+4);
    pold = getmem_w(reg.r[REGSP]+8);
    flags = getmem_w(reg.r[REGSP]+12);
    memio(name, pname, sizeof(name), MemReadstring);
    memio(old, pold, sizeof(old), MemReadstring);
    if(sysdbg)
        itrace("bind(0x%lux='%s', 0x%lux='%s', 0x%lux)", name, name, old, old, flags);

    n = bind(name, old, flags);
    if(n < 0)
        errstr(errbuf, sizeof errbuf);

    reg.r[REGRET] = n;
}
/*e: function [[sysbind]] */

/*s: function [[sysfd2path]] */
void
sysfd2path(void)
{
    int n;
    uint fd;
    ulong str;
    char buf[1024];

    fd = getmem_w(reg.r[REGSP]+4);
    str = getmem_w(reg.r[REGSP]+8);
    n = getmem_w(reg.r[REGSP]+12);
    if(sysdbg)
        itrace("fd2path(0x%lux, 0x%lux, 0x%lux)", fd, str, n);
    reg.r[1] = -1;
    if(n > sizeof buf){
        strcpy(errbuf, "buffer too big");
        return;
    }
    n = fd2path(fd, buf, sizeof buf);
    if(n < 0)
        errstr(buf, sizeof buf);
    else
        memio(errbuf, str, n, MemWrite);
    reg.r[REGRET] = n;
    
}
/*e: function [[sysfd2path]] */

/*s: function [[syschdir]] */
void
syschdir(void)
{ 
    char file[1024];
    int n;
    ulong name;

    name = getmem_w(reg.r[REGSP]+4);
    memio(file, name, sizeof(file), MemReadstring);
    if(sysdbg)
        itrace("chdir(0x%lux='%s', 0x%lux)", name, file);
    
    n = chdir(file);
    if(n < 0)
        errstr(errbuf, sizeof errbuf);

    reg.r[REGRET] = n;
}
/*e: function [[syschdir]] */

/*s: function [[sysclose]] */
void
sysclose(void)
{
    int n;
    ulong fd;

    fd = getmem_w(reg.r[REGSP]+4);
    if(sysdbg)
        itrace("close(%d)", fd);

    n = close(fd);
    if(n < 0)
        errstr(errbuf, sizeof errbuf);
    reg.r[REGRET] = n;
}
/*e: function [[sysclose]] */

/*s: function [[sysdup]] */
void
sysdup(void)
{
    int oldfd, newfd;
    int n;

    oldfd = getmem_w(reg.r[REGSP]+4);
    newfd = getmem_w(reg.r[REGSP]+8);
    if(sysdbg)
        itrace("dup(%d, %d)", oldfd, newfd);

    n = dup(oldfd, newfd);
    if(n < 0)
        errstr(errbuf, sizeof errbuf);
    reg.r[REGRET] = n;
}
/*e: function [[sysdup]] */

/*s: function [[sysexits]] */
void
sysexits(void)
{
    char buf[OERRLEN];
    ulong str;

    str = getmem_w(reg.r[REGSP]+4);
    if(sysdbg)
        itrace("exits(0x%lux)", str);

    // single step to give opportunity to inspect before exit
    count = 1;
    if(str != 0) {
        memio(buf, str, sizeof buf, MemRead);
        Bprint(bout, "exits(%s)\n", buf);
    }
    else
        Bprint(bout, "exits(0)\n");
}
/*e: function [[sysexits]] */

/*s: function [[sysopen]] */
void
sysopen(void)
{
    char file[1024];
    int n;
    ulong mode, name;

    name = getmem_w(reg.r[REGSP]+4);
    mode = getmem_w(reg.r[REGSP]+8);
    memio(file, name, sizeof(file), MemReadstring);
    
    n = open(file, mode);
    if(n < 0)
        errstr(errbuf, sizeof errbuf);

    if(sysdbg)
        itrace("open(0x%lux='%s', 0x%lux) = %d", name, file, mode, n);

    reg.r[REGRET] = n;
};
/*e: function [[sysopen]] */

/*s: function [[sysread]] */
void
sysread(vlong offset)
{
    int fd;
    ulong size, a;
    char *buf, *p;
    int n, cnt, c;

    fd = getmem_w(reg.r[REGSP]+4);
    a = getmem_w(reg.r[REGSP]+8);
    size = getmem_w(reg.r[REGSP]+12);

    buf = emalloc(size);
    if(fd == 0) {
        print("\nstdin>>");
        p = buf;
        n = 0;
        cnt = size;
        while(cnt) {
            c = Bgetc(bin);
            if(c <= 0)
                break;
            *p++ = c;
            n++;
            cnt--;
            if(c == '\n')
                break;
        }
    }
    else
        n = pread(fd, buf, size, offset);

    if(n < 0)
        errstr(errbuf, sizeof errbuf);
    else
        memio(buf, a, n, MemWrite);

    if(sysdbg)
        itrace("read(%d, 0x%lux, %d, 0x%llx) = %d", fd, a, size, offset, n);

    free(buf);
    reg.r[REGRET] = n;
}
/*e: function [[sysread]] */

/*s: function [[syspread]] */
void
syspread(void)
{
    sysread(getmem_v(reg.r[REGSP]+16));
}
/*e: function [[syspread]] */

/*s: function [[sysseek]] */
void
sysseek(void)
{
    int fd;
    ulong mode;
    ulong retp;
    vlong v;

    retp = getmem_w(reg.r[REGSP]+4);
    fd = getmem_w(reg.r[REGSP]+8);
    v = getmem_v(reg.r[REGSP]+16);
    mode = getmem_w(reg.r[REGSP]+20);
    if(sysdbg)
        itrace("seek(%d, %lld, %d)", fd, v, mode);

    v = seek(fd, v, mode);
    if(v < 0)
        errstr(errbuf, sizeof errbuf);	

    putmem_v(retp, v);
}
/*e: function [[sysseek]] */

/*s: function [[syssleep]] */
void
syssleep(void)
{
    ulong len;
    int n;

    len = getmem_w(reg.r[REGSP]+4);
    if(sysdbg)
        itrace("sleep(%d)", len);

    n = sleep(len);
    if(n < 0)
        errstr(errbuf, sizeof errbuf);	

    reg.r[REGRET] = n;
}
/*e: function [[syssleep]] */

/*s: function [[sysstat]] */
void
sysstat(void)
{
    char nambuf[1024];
    byte buf[STATMAX];
    ulong edir, name;
    int n;

    name = getmem_w(reg.r[REGSP]+4);
    edir = getmem_w(reg.r[REGSP]+8);
    n = getmem_w(reg.r[REGSP]+12);
    memio(nambuf, name, sizeof(nambuf), MemReadstring);
    if(sysdbg)
        itrace("stat(0x%lux='%s', 0x%lux, 0x%lux)", name, nambuf, edir, n);
    if(n > sizeof buf)
        errstr(errbuf, sizeof errbuf);
    else{	
        n = stat(nambuf, buf, n);
        if(n < 0)
            errstr(errbuf, sizeof errbuf);
        else
            memio((char*)buf, edir, n, MemWrite);
    }
    reg.r[REGRET] = n;
}
/*e: function [[sysstat]] */

/*s: function [[sysfstat]] */
void
sysfstat(void)
{
    byte buf[STATMAX];
    ulong edir;
    int n, fd;

    fd = getmem_w(reg.r[REGSP]+4);
    edir = getmem_w(reg.r[REGSP]+8);
    n = getmem_w(reg.r[REGSP]+12);
    if(sysdbg)
        itrace("fstat(%d, 0x%lux, 0x%lux)", fd, edir, n);

    reg.r[REGRET] = -1;
    if(n > sizeof buf){
        strcpy(errbuf, "stat buffer too big");
        return;
    }
    n = fstat(fd, buf, n);
    if(n < 0)
        errstr(errbuf, sizeof errbuf);
    else
        memio((char*)buf, edir, n, MemWrite);
    reg.r[REGRET] = n;
}
/*e: function [[sysfstat]] */

/*s: function [[syswrite]] */
void
syswrite(vlong offset)
{
    int fd;
    ulong size, a;
    char *buf;
    int n;

    fd = getmem_w(reg.r[REGSP]+4);
    a = getmem_w(reg.r[REGSP]+8);
    size = getmem_w(reg.r[REGSP]+12);

    Bflush(bout);
    buf = memio(0, a, size, MemRead);
    n = pwrite(fd, buf, size, offset);
    if(n < 0)
        errstr(errbuf, sizeof errbuf);	

    if(sysdbg)
        itrace("write(%d, %lux, %d, 0x%llx) = %d", fd, a, size, offset, n);

    free(buf);

    reg.r[REGRET] = n;
}
/*e: function [[syswrite]] */

/*s: function [[syspwrite]] */
void
syspwrite(void)
{
    syswrite(getmem_v(reg.r[REGSP]+16));
}
/*e: function [[syspwrite]] */

/*s: function [[syspipe]] */
void
syspipe(void)
{
    int n, p[2];
    ulong fd;

    fd = getmem_w(reg.r[REGSP]+4);
    if(sysdbg)
        itrace("pipe(%lux)", fd);

    n = pipe(p);
    if(n < 0)
        errstr(errbuf, sizeof errbuf);
    else {
        putmem_w(fd, p[0]);
        putmem_w(fd+4, p[1]);
    }
    reg.r[REGRET] = n;
}
/*e: function [[syspipe]] */

/*s: function [[syscreate]] */
void
syscreate(void)
{
    char file[1024];
    int n;
    ulong mode, name, perm;

    name = getmem_w(reg.r[REGSP]+4);
    mode = getmem_w(reg.r[REGSP]+8);
    perm = getmem_w(reg.r[REGSP]+12);
    memio(file, name, sizeof(file), MemReadstring);
    if(sysdbg)
        itrace("create(0x%lux='%s', 0x%lux, 0x%lux)", name, file, mode, perm);
    
    n = create(file, mode, perm);
    if(n < 0)
        errstr(errbuf, sizeof errbuf);

    reg.r[REGRET] = n;
}
/*e: function [[syscreate]] */

/*s: function [[sysbrk]] */
void
sysbrk(void)
{
    ulong addr, osize, nsize;
    Segment *s;

    addr = getmem_w(reg.r[REGSP]+4);
    if(sysdbg)
        itrace("brk(0x%lux)", addr);

    reg.r[REGRET] = -1;
    if(addr < memory.seg[Data].base+datasize) {
        strcpy(errbuf, "address below segment");
        return;
    }
    if(addr > memory.seg[Stack].base) {
        strcpy(errbuf, "segment too big");
        return;
    }
    s = &memory.seg[Bss];
    if(addr > s->end) {
        osize = ((s->end-s->base)/BY2PG)*sizeof(byte*);
        addr = ((addr)+(BY2PG-1))&~(BY2PG-1);
        s->end = addr;
        nsize = ((s->end-s->base)/BY2PG)*sizeof(byte*);
        s->table = erealloc(s->table, osize, nsize);
    }	

    reg.r[REGRET] = 0;	
}
/*e: function [[sysbrk]] */

/*s: function [[sysremove]] */
void
sysremove(void)
{
    char nambuf[1024];
    ulong name;
    int n;

    name = getmem_w(reg.r[REGSP]+4);
    memio(nambuf, name, sizeof(nambuf), MemReadstring);
    if(sysdbg)
        itrace("remove(0x%lux='%s')", name, nambuf);

    n = remove(nambuf);
    if(n < 0)
        errstr(errbuf, sizeof errbuf);
    reg.r[REGRET] = n;
}
/*e: function [[sysremove]] */

/*s: function [[sysnotify]] */
void
sysnotify(void)
{
    nofunc = getmem_w(reg.r[REGSP]+4);
    if(sysdbg)
        itrace("notify(0x%lux)\n", nofunc);

    reg.r[REGRET] = 0;
}
/*e: function [[sysnotify]] */




/*s: function [[sysawait]] */
void
sysawait(void)
{
    Bprint(bout, "No system call %s\n", sysctab[reg.r[REGARG]]);
    exits(0);
}
/*e: function [[sysawait]] */
/*s: function [[sysrfork]] */
void
sysrfork(void)
{
    Bprint(bout, "No system call %s\n", sysctab[reg.r[REGARG]]);
    exits(0);
}
/*e: function [[sysrfork]] */
/*s: function [[syswstat]] */
void
syswstat(void)
{
    Bprint(bout, "No system call %s\n", sysctab[reg.r[REGARG]]);
    exits(0);
}
/*e: function [[syswstat]] */
/*s: function [[sysfwstat]] */
void
sysfwstat(void)
{
    Bprint(bout, "No system call %s\n", sysctab[reg.r[REGARG]]);
    exits(0);
}
/*e: function [[sysfwstat]] */
/*s: function [[sysnoted]] */
void
sysnoted(void)
{
    Bprint(bout, "No system call %s\n", sysctab[reg.r[REGARG]]);
    exits(0);
}
/*e: function [[sysnoted]] */
/*s: function [[syssegattach]] */
void
syssegattach(void)
{
    Bprint(bout, "No system call %s\n", sysctab[reg.r[REGARG]]);
    exits(0);
}
/*e: function [[syssegattach]] */
/*s: function [[syssegdetach]] */
void
syssegdetach(void)
{
    Bprint(bout, "No system call %s\n", sysctab[reg.r[REGARG]]);
    exits(0);
}
/*e: function [[syssegdetach]] */
/*s: function [[syssegfree]] */
void
syssegfree(void)
{
    Bprint(bout, "No system call %s\n", sysctab[reg.r[REGARG]]);
    exits(0);
}
/*e: function [[syssegfree]] */
/*s: function [[syssegflush]] */
void
syssegflush(void)
{
    Bprint(bout, "No system call %s\n", sysctab[reg.r[REGARG]]);
    exits(0);
}
/*e: function [[syssegflush]] */
/*s: function [[sysrendezvous]] */
void
sysrendezvous(void)
{
    Bprint(bout, "No system call %s\n", sysctab[reg.r[REGARG]]);
    exits(0);
}
/*e: function [[sysrendezvous]] */
/*s: function [[sysunmount]] */
void
sysunmount(void)
{
    Bprint(bout, "No system call %s\n", sysctab[reg.r[REGARG]]);
    exits(0);
}
/*e: function [[sysunmount]] */

/*s: function [[syssegbrk]] */
void
syssegbrk(void)
{
    Bprint(bout, "No system call %s\n", sysctab[reg.r[REGARG]]);
    exits(0);
}
/*e: function [[syssegbrk]] */
/*s: function [[sysmount]] */
void
sysmount(void)
{
    Bprint(bout, "No system call %s\n", sysctab[reg.r[REGARG]]);
    exits(0);
}
/*e: function [[sysmount]] */
/*s: function [[sysalarm]] */
void
sysalarm(void)
{
    Bprint(bout, "No system call %s\n", sysctab[reg.r[REGARG]]);
    exits(0);
}
/*e: function [[sysalarm]] */
/*s: function [[sysexec]] */
void
sysexec(void)
{
    Bprint(bout, "No system call %s\n", sysctab[reg.r[REGARG]]);
    exits(0);
}
/*e: function [[sysexec]] */

/*s: function [[sysfauth]] */
void
sysfauth(void)
{
    Bprint(bout, "No system call %s\n", sysctab[reg.r[REGARG]]);
    exits(0);
}
/*e: function [[sysfauth]] */
/*s: function [[sysfversion]] */
void
sysfversion(void)
{
    Bprint(bout, "No system call %s\n", sysctab[reg.r[REGARG]]);
    exits(0);
}
/*e: function [[sysfversion]] */

/*s: global [[systab]] */
void	(*systab[])(void) =
{
    [NOP]		sysnop,

    [RFORK]		sysrfork,
    [EXEC]		sysexec,
    [EXITS]		sysexits,
    [AWAIT]		sysawait,

    [BRK]		sysbrk,

    [OPEN]		sysopen,
    [CLOSE]		sysclose,
    [PREAD]		syspread,
    [PWRITE]	syspwrite,
    [SEEK]		sysseek,

    [CREATE]	syscreate,
    [REMOVE]	sysremove,
    [CHDIR]		syschdir,
    [FD2PATH]	sysfd2path,
    [STAT]		sysstat,
    [FSTAT]		sysfstat,
    [WSTAT]		syswstat,
    [FWSTAT]	sysfwstat,

    [BIND]		sysbind,
    [MOUNT]		sysmount,
    [UNMOUNT]	sysunmount,

    [SLEEP]		syssleep,
    [ALARM]		sysalarm,

    [PIPE]		syspipe,
    [NOTIFY]	sysnotify,
    [NOTED]		sysnoted,

    [SEGATTACH]	syssegattach,
    [SEGDETACH]	syssegdetach,
    [SEGFREE]	syssegfree,
    [SEGFLUSH]	syssegflush,
    [SEGBRK]	syssegbrk,

    [RENDEZVOUS]	sysrendezvous,

    [DUP]		sysdup,
    [FVERSION]	sysfversion,
    [FAUTH]		sysfauth,

    [ERRSTR]	syserrstr,
};
/*e: global [[systab]] */

/*s: function [[Ssyscall]] */
void
Ssyscall(instruction _unused)
{
    int call;
    USED(_unused);

    call = reg.r[REGARG];

    if(call < 0 || call >= nelem(systab) || systab[call] == nil) {
        Bprint(bout, "bad system call %d (%#ux)\n", call, call);
        dumpreg();
        Bflush(bout);
        return;
    }

    if(trace)
        itrace("SWI\t%s", sysctab[call]);

    // dispatch!
    (*systab[call])();

    Bflush(bout);
}
/*e: function [[Ssyscall]] */
/*e: machine/5i/syscall.c */
