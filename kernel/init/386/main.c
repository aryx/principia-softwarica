/*s: init/386/main.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

#include        "io.h"
#include        "mp.h"

// initcode binary
#include        "init.h"
// rebootcode binary
#include        "reboot.h"

#include        <ureg.h>
#include        <pool.h>
#include        <tos.h>

void bootargs(void*);

// part of a trick to remove some backward dependencies
/*s: main.c forward decl for backward deps(x86) */
int devcons_print(char*, ...);
int devcons_iprint(char*, ...);
int devcons_pprint(char*, ...);
void devcons_panic(char*, ...);
void devcons__assert(char*);
void trap_dumpstack(void);
void proc_dumpaproc(Proc *p);
void proc_error(char*);
void proc_nexterror(void);
void i8253_delay(int millisecs);
void i8253_microdelay(int microsecs);
void proc_sched(void);
void proc_ready(Proc*);
void proc_sleep(Rendez*, int(*)(void*), void*);
void proc_tsleep(Rendez *r, int (*fn)(void*), void *arg, ulong ms);
Proc* proc_wakeup(Rendez*);
void proc_pexit(char *exitstr, bool freemem);
Proc* proc_proctab(int i);
int proc_postnote(Proc *p, int dolock, char *n, int flag);
void chan_cclose(Chan *c);
void main_exit(int ispanic);
int  main_isaconfig(char *class, int ctlrno, ISAConf *isa);
void nop(void);
uvlong devarch_arch_fastticks(uvlong *hz);
void devarch_hook_ioalloc();
/*e: main.c forward decl for backward deps(x86) */

// conf.c
extern  Dev*  conf_devtab[];
// ??
extern  char* conffile;
//extern  uchar initcode[]; in init.h

extern void (*i8237alloc)(void);

//*****************************************************************************
// Configuration
//*****************************************************************************
// See globals in portdat_globals.h

//*****************************************************************************
// Boot parameters (not used by pad)
//*****************************************************************************

/*
 * Where configuration info is left for the loaded program.
 * This will turn into a structure as more is done by the boot loader
 * (e.g. why parse the .ini file twice?).
 * There are 3584 bytes available at CONFADDR.
 */
#define BOOTLINE        ((char*)CONFADDR)
#define BOOTLINELEN     64
#define BOOTARGS        ((char*)(CONFADDR+BOOTLINELEN))
#define BOOTARGSLEN     (4096-0x200-BOOTLINELEN)


enum {
        /* space for syscall args, return PC, top-of-stack struct */
        Ustkheadroom    = sizeof(Sargs) + sizeof(uintptr) + sizeof(Tos),
};

//char bootdisk[KNAMELEN];

// Global! set by bootargs()
uchar *sp;      /* user stack of init proc */

//@Scheck: Assembly
extern phys_addr *multiboot;

/*s: function [[options]](x86) */
static void
options(void)
{
        long i, n;
        char *cp, *line[MAXCONF], *p, *q;
        ulong l;
        kern_addr2 m;

        if(multiboot != nil){
                cp = BOOTARGS;
                *cp = 0;
                if((*multiboot & 8) != 0 && multiboot[5] > 0){
                        m = KADDR(multiboot[6]);
                        l = m[1] - m[0];
                        m = KADDR(m[0]);
                        if(l >= BOOTARGSLEN)
                                l = BOOTARGSLEN - 1;
                        memmove(cp, m, l);
                        cp[l] = 0;
                }
        }

        /*
         *  parse configuration args from dos file plan9.ini
         */
        cp = BOOTARGS;  /* where b.com leaves its config */
        cp[BOOTARGSLEN-1] = 0;

        /*
         * Strip out '\r', change '\t' -> ' '.
         */
        p = cp;
        for(q = cp; *q; q++){
                if(*q == '\r')
                        continue;
                if(*q == '\t')
                        *q = ' ';
                *p++ = *q;
        }
        *p = 0;

        n = getfields(cp, line, MAXCONF, 1, "\n");
        for(i = 0; i < n; i++){
                if(*line[i] == '#')
                        continue;
                cp = strchr(line[i], '=');
                if(cp == nil)
                        continue;
                *cp++ = '\0';
                confname[nconf] = line[i];
                confval[nconf] = cp;
                nconf++;
        }
}
/*e: function [[options]](x86) */

/*s: function [[writeconf]](x86) */
static void
writeconf(void)
{
        char *p, *q;
        int n;

        p = getconfenv();

        if(waserror()) {
                free(p);
                nexterror();
        }

        /* convert to name=value\n format */
        for(q=p; *q; q++) {
                q += strlen(q);
                *q = '=';
                q += strlen(q);
                *q = '\n';
        }
        n = q - p + 1;
        if(n >= BOOTARGSLEN)
                error("kernel configuration too large");
        memset(BOOTLINE, 0, BOOTLINELEN);
        memmove(BOOTARGS, p, n);
        poperror();
        free(p);
}
/*e: function [[writeconf]](x86) */

//*****************************************************************************
// Cpu init
//*****************************************************************************

/*s: function [[cpuinit]](x86) */
void
cpuinit(void)
{
    int cpuno;
    kern_addr2 mmupd;
    Segdesc *gdt;

    cpuno = cpu->cpuno;
    mmupd = cpu->pdproto;
    gdt = cpu->gdt;
    memset(cpu, 0, sizeof(Cpu));
    cpu->cpuno = cpuno;
    cpu->pdproto = mmupd;
    cpu->gdt = gdt;

    cpu->perf.period = 1;
    /*
     * For polled uart output at boot, need
     * a default delay constant. 100000 should
     * be enough for a while. Cpuidentify will
     * calculate the real value later.
     */
    cpu->loopconst = 100000;
}
/*e: function [[cpuinit]](x86) */

/*s: function [[cpu0init]](x86) */
void
cpu0init(void)
{
    conf.ncpu = 1;
    // note that cpu points to CPUADDR, which then is mapped to CPU0CPU
    // via the page directory/page table setup in cr3 on the first processor
    CPUS(0) = (Cpu*)CPU0CPU;
    cpu->pdproto = (kern_addr2)CPU0PD;
    cpu->gdt = (Segdesc*)CPU0GDT;

    cpuinit();

    active.cpus = 1;
    active.exiting = false;
}
/*e: function [[cpu0init]](x86) */

//*****************************************************************************
// Conf init
//*****************************************************************************

/*s: function [[confinit]](x86) */
// precondition: meminit() have initialized Conf.mem
void
confinit(void)
{
    char *p;
    int i, userpcnt;
    ulong kpages;
    ulong kmem;

    if(p = getconf("*kernelpercent"))
        userpcnt = 100 - strtol(p, 0, 0);
    else
        userpcnt = 0;

    conf.npage = 0;
    for(i=0; i<nelem(conf.mem); i++)
        conf.npage += conf.mem[i].npage;

    // 1 process per 250Ko of available memory
    conf.nproc = 100 + ((conf.npage*BY2PG)/MB)*5;
    if(cpuserver)
        conf.nproc *= 3;
    if(conf.nproc > 2000)
        conf.nproc = 2000;

    conf.nimage = 200;

    conf.nswap = conf.nproc*80;
    conf.nswppo = 4096;

    if(cpuserver) {
        if(userpcnt < 10)
            userpcnt = 70;
        kpages = conf.npage - (conf.npage*userpcnt)/100;

        /*
         * Hack for the big boys. Only good while physmem < 4GB.
         * Give the kernel fixed max + enough to allocate the
         * page pool.
         * This is an overestimate as conf.upages < conf.npages.
         * The patch of nimage is a band-aid, scanning the whole
         * page list in imagereclaim just takes too long.
         */
        if(kpages > (128*MB + conf.npage*sizeof(Page))/BY2PG){
            kpages = (128*MB + conf.npage*sizeof(Page))/BY2PG;
            conf.nimage = 2000;
            kpages += (conf.nproc*KSTACK)/BY2PG;
        }
    } else {
        if(userpcnt < 10) {
            if(conf.npage*BY2PG < 16*MB)
                userpcnt = 40;
            else
                userpcnt = 80;
        }
        kpages = conf.npage - (conf.npage*userpcnt)/100;

        /*
         * Make sure terminals with low memory get at least
         * 4MB on the first Image chunk allocation.
         */
        if(conf.npage*BY2PG < 16*MB)
            imagmem->minarena = 4*MB;
    }

    /*
     * can't go past the end of virtual memory
     */
    if(kpages > MAXKPA/BY2PG)
        kpages = MAXKPA/BY2PG;

    conf.upages = conf.npage - kpages;
    conf.ialloc = (kpages/2)*BY2PG;

    /*
     * Guess how much is taken by the large permanent
     * datastructures. Mntcache and Mntrpc are not accounted for
     * (probably ~300KB).
     */
    kmem = kpages * BY2PG;
    kmem -= conf.upages*sizeof(Page)
            + conf.nproc*sizeof(Proc)
            + conf.nimage*sizeof(KImage)
            + conf.nswap
            + conf.nswppo*sizeof(Page*); // pad's second bugfix :)
    mainmem->maxsize = kmem;
    if(!cpuserver){
        /*
         * give terminals lots of image memory, too; the dynamic
         * allocation will balance the load properly, hopefully.
         * be careful with 32-bit overflow.
         */
        imagmem->maxsize = kmem;
    }
}
/*e: function [[confinit]](x86) */

//*****************************************************************************
// First process init
//*****************************************************************************

/*s: function [[init0]](x86) */
// set by userinit to sched.pc
void
init0(void)
{
    int i;
    char buf[2*KNAMELEN]; // has to be the same than in ksetenv?
    
    up->nerrlab = 0;

    arch_spllo();

    /*
     * These are o.k. because rootinit is null.
     * Then early kproc's will have a root and dot.
     */
    up->slash = namec("#/", Atodir, 0, 0);
    pathclose(up->slash->path);
    up->slash->path = newpath("/"); // prefer / to #/
    up->dot = cclone(up->slash);

    chandevinit();

    if(!waserror()){
        snprint(buf, sizeof(buf), "%s %s", arch->id, conffile);
        ksetenv("terminal", buf, false);
        ksetenv("cputype", "386", false); // used by mkfile! 
        if(cpuserver)
                ksetenv("service", "cpu", false);
        else
                ksetenv("service", "terminal", false);
        for(i = 0; i < nconf; i++){
                if(confname[i][0] != '*')
                        ksetenv(confname[i], confval[i], false);
                ksetenv(confname[i], confval[i], true);
        }
        poperror();
    }
    kproc("kalarm", alarmkproc, nil);
    cgapost(0x9);
    arch_touser(sp);
}
/*e: function [[init0]](x86) */

/*s: function [[pusharg]](x86) */
uchar *
pusharg(char *p)
{
        int n;

        n = strlen(p)+1;
        sp -= n;
        memmove(sp, p, n);
        return sp;
}
/*e: function [[pusharg]](x86) */

//TODO: get rid of as have simplified boot process, no plan9.ini
/*s: function [[bootargs]](x86) */
void
bootargs(void *base)
{
        int i, ac;
        uchar *av[32];
        uchar **lsp;
        char *cp = BOOTLINE;
        char buf[64];

        sp = (uchar*)base + BY2PG - Ustkheadroom;

        ac = 0;
        av[ac++] = pusharg("/386/9dos");

        /* when boot is changed to only use rc, this code can go away */
        cp[BOOTLINELEN-1] = 0;
        buf[0] = 0;
        if(strncmp(cp, "fd", 2) == 0){
                snprint(buf, sizeof buf, "local!#f/fd%lddisk",
                        strtol(cp+2, 0, 0));
                av[ac++] = pusharg(buf);
        } else if(strncmp(cp, "sd", 2) == 0){
                snprint(buf, sizeof buf, "local!#S/sd%c%c/fs", *(cp+2), *(cp+3));
                av[ac++] = pusharg(buf);
        } else if(strncmp(cp, "ether", 5) == 0)
                av[ac++] = pusharg("-n");

        /* 4 byte word align stack */
        sp = (uchar*)((ulong)sp & ~3);

        /* build argc, argv on stack */
        sp -= (ac+1)*sizeof(sp);
        lsp = (uchar**)sp;
        for(i = 0; i < ac; i++)
                *lsp++ = av[i] + ((USTKTOP - BY2PG) - (ulong)base);
        *lsp = 0;
        sp += (USTKTOP - BY2PG) - (ulong)base - sizeof(ulong);
}
/*e: function [[bootargs]](x86) */

/*s: function [[userinit]](x86) */
void
userinit(void)
{
    void *v;
    Proc *p;
    Segment *s;
    Page *pg;

    p = newproc();

    p->pgrp = newpgrp();
    p->fgrp = dupfgrp(nil);
    p->egrp = smalloc(sizeof(Egrp)); //todo: newegrp()
    p->egrp->ref = 1;
    p->rgrp = newrgrp();

    p->procmode = 0640;

    kstrdup(&eve, "");
    kstrdup(&p->text, "*init*");
    kstrdup(&p->user, eve);

    /*s: [[userinit()]] fp setup(x86) */
        p->fpstate = FPinit;
        fpoff();
    /*e: [[userinit()]] fp setup(x86) */

    /*
     * Kernel Stack
     *
     * N.B. make sure there's enough space for syscall to check
     *      for valid args and 
     *      4 bytes for arch_gotolabel's return PC
     */
    p->sched.pc = (kern_addr)init0;
    p->sched.sp = (kern_addr)p->kstack+KSTACK-(sizeof(Sargs)+BY2WD);

    /*
     * User Stack
     *
     * N.B. cannot call newpage() with clear=1, because PC arch_kmap()
     * requires up != nil.  use tmpmap() instead.
     */
    s = newseg(SG_STACK, USTKTOP-USTKSIZE, USTKSIZE/BY2PG);
    p->seg[SSEG] = s;
    pg = newpage(false, nil, USTKTOP-BY2PG);
    v = tmpmap(pg);
    memset(v, 0, BY2PG);
    segpage(s, pg);
    bootargs(v); // populate user stack for boot/boot program
    tmpunmap(v);

    /*
     * Text
     */
    s = newseg(SG_TEXT, UTZERO, 1); // 1 page
    s->flushme = true;
    p->seg[TSEG] = s;
    pg = newpage(false, nil, UTZERO);
    memset(pg->cachectl, PG_TXTFLUSH, sizeof(pg->cachectl));
    segpage(s, pg);
    v = tmpmap(pg);
    memset(v, 0, BY2PG);
    memmove(v, initcode, sizeof initcode); // must be < BY2PG!
    tmpunmap(v);

    ready(p);
}
/*e: function [[userinit]](x86) */

//*****************************************************************************
// Math coprocessor
//*****************************************************************************

/*s: global [[mathmsg]](x86) */
static char* mathmsg[] =
{
        nil,    /* handled below */
        "denormalized operand",
        "division by zero",
        "numeric overflow",
        "numeric underflow",
        "precision loss",
};
/*e: global [[mathmsg]](x86) */

/*s: function [[mathstate]](x86) */
static void
mathstate(ulong *stsp, ulong *pcp, ulong *ctlp)
{
        ulong sts, fpc, ctl;
        Arch_FPsave *f = &up->fpsave;

        if(fpsave == fpx87save){
                sts = f->status;
                fpc = f->pc;
                ctl = f->control;
        } else {
                sts = f->fsw;
                fpc = f->fpuip;
                ctl = f->fcw;
        }
        if(stsp)
                *stsp = sts;
        if(pcp)
                *pcp = fpc;
        if(ctlp)
                *ctlp = ctl;
}
/*e: function [[mathstate]](x86) */

/*s: function [[mathnote]](x86) */
static void
mathnote(void)
{
        int i;
        ulong status, pc;
        char *msg, note[ERRMAX];

        mathstate(&status, &pc, nil);

        /*
         * Some attention should probably be paid here to the
         * exception masks and error summary.
         */
        msg = "unknown exception";
        for(i = 1; i <= 5; i++){
                if(!((1<<i) & status))
                        continue;
                msg = mathmsg[i];
                break;
        }
        if(status & 0x01){
                if(status & 0x40){
                        if(status & 0x200)
                                msg = "stack overflow";
                        else
                                msg = "stack underflow";
                }else
                        msg = "invalid operation";
        }
        snprint(note, sizeof note, "sys: fp: %s fppc=%#lux status=%#lux",
                msg, pc, status);
        postnote(up, 1, note, NDebug);
}
/*e: function [[mathnote]](x86) */

/*s: function [[matherror]](x86) */
/*
 *  math coprocessor error
 */
static void
matherror(Ureg *ur, void*)
{
        ulong status, pc;

        /*
         *  a write cycle to port 0xF0 clears the interrupt latch attached
         *  to the error# line from the 387
         */
        if(!(cpu->cpuiddx & Fpuonchip))
                outb(0xF0, 0xFF);

        /*
         *  save floating point state to check out error
         */
        fpenv(&up->fpsave);     /* result ignored, but masks fp exceptions */
        fpsave(&up->fpsave);            /* also turns fpu off */
        fpon();
        mathnote();

        if((ur->pc & 0xf0000000) == KZERO){
                mathstate(&status, &pc, nil);
                panic("fp: status %#lux fppc=%#lux pc=%#lux", status, pc, ur->pc);
        }
}
/*e: function [[matherror]](x86) */

/*s: function [[mathemu]](x86) */
/*
 *  math coprocessor emulation fault
 */
static void
mathemu(Ureg *ureg, void*)
{
        ulong status, control;

        if(up->fpstate & FPillegal){
                /* someone did floating point in a note handler */
                postnote(up, 1, "sys: floating point in note handler", NDebug);
                return;
        }
        switch(up->fpstate){
        case FPinit:
                fpinit();
                up->fpstate = FPactive;
                break;
        case FPinactive:
                /*
                 * Before restoring the state, check for any pending
                 * exceptions, there's no way to restore the state without
                 * generating an unmasked exception.
                 * More attention should probably be paid here to the
                 * exception masks and error summary.
                 */
                mathstate(&status, nil, &control);
                if((status & ~control) & 0x07F){
                        mathnote();
                        break;
                }
                fprestore(&up->fpsave);
                up->fpstate = FPactive;
                break;
        case FPactive:
                panic("math emu pid %ld %s pc %#lux",
                        up->pid, up->text, ureg->pc);
                break;
        }
}
/*e: function [[mathemu]](x86) */

/*s: function [[mathover]](x86) */
/*
 *  math coprocessor segment overrun
 */
static void
mathover(Ureg*, void*)
{
        pexit("math overrun", false);
}
/*e: function [[mathover]](x86) */

/*s: function [[mathinit]](x86) */
void
mathinit(void)
{
        trapenable(VectorCERR, matherror, 0, "matherror");
        if(X86FAMILY(cpu->cpuidax) == 3)
                arch_intrenable(IrqIRQ13, matherror, 0, BUSUNKNOWN, "matherror");
        trapenable(VectorCNA, mathemu, 0, "mathemu");
        trapenable(VectorCSO, mathover, 0, "mathover");
}
/*e: function [[mathinit]](x86) */

//*****************************************************************************
// Shutdown/reboot
//*****************************************************************************

/*s: function [[shutdown]](x86) */
static void
shutdown(bool ispanic)
{
    int ms, once;

    lock(&active);
    if(ispanic)
        active.ispanic = ispanic;
    else if(cpu->cpuno == 0 && (active.cpus & (1<<cpu->cpuno)) == 0)
        active.ispanic = false;
    once = active.cpus & (1<<cpu->cpuno);
    /*
     * setting exiting will make hzclock() on each processor call exit(0),
     * which calls shutdown(0) and arch->reset(), which on mp systems is
     * mpshutdown, which idles non-bootstrap cpus and returns on bootstrap
     * processors (to permit a reboot).  clearing our bit in cpus avoids
     * calling exit(0) from hzclock() on this processor.
     */
    active.cpus &= ~(1<<cpu->cpuno);
    active.exiting = true;
    unlock(&active);

    if(once)
        iprint("cpu%d: exiting\n", cpu->cpuno);

    /* wait for any other processors to shutdown */
    arch_spllo();
    for(ms = 5*1000; ms > 0; ms -= TK2MS(2)){
        arch_delay(TK2MS(2));
        if(active.cpus == 0 && consactive() == 0)
            break;
    }

    if(active.ispanic){
        if(!cpuserver)
            for(;;)
               halt();
        if(getconf("*debug"))
            arch_delay(5*60*1000);
        else
            arch_delay(10000);
    }else
        arch_delay(1000);
}
/*e: function [[shutdown]](x86) */

/*s: function [[exit]](x86) */
void
main_exit(bool ispanic)
{
        shutdown(ispanic);
        arch->reset();
}
/*e: function [[exit]](x86) */

/*s: function [[reboot]](x86) */
void
arch_reboot(kern_addr3 entry, kern_addr3 code, ulong size)
{
    void (*f)(ulong, ulong, ulong);
    kern_addr2 mmupd;

    writeconf();

    /*
     * the boot processor is cpu0.  execute this function on it
     * so that the new kernel has the same cpu0.  this only matters
     * because the hardware has a notion of which processor was the
     * boot processor and we look at it at start up.
     */
    if (cpu->cpuno != 0) {
        procwired(up, 0);
        sched();
    }

    if(conf.ncpu > 1) {
        /*
         * the other cpus could be holding locks that will never get
         * released (e.g., in the print path) if we put them into
         * reset now, so force them to shutdown gracefully first.
         */
        lock(&active);
        active.rebooting = true;
        unlock(&active);
        shutdown(0);
        if(arch->resetothers)
            arch->resetothers();
        arch_delay(20);
    }

    /*
     * should be the only processor running now
     */
    active.cpus = 0;
    if (cpu->cpuno != 0)
        print("on cpu%d (not 0)!\n", cpu->cpuno);

    print("shutting down...\n");
    arch_delay(200);

    arch_splhi();

    /*s: [[reboot()]] reset serialoq(x86) */
    /* turn off buffered serial console */
    serialoq = nil;
    /*e: [[reboot()]] reset serialoq(x86) */

    /* shutdown devices */
    chandevshutdown();
    arch->introff();

    /*
     * Modify the processor page table to directly map the low 4MB of memory
     * This allows the reboot code to turn off the page mapping
     */
    mmupd = cpu->pdproto;
    mmupd[PDX(0)] = mmupd[PDX(KZERO)];
    mmuflushtlb(PADDR(mmupd));

    /* setup reboot trampoline function */
    f = (void*)REBOOTADDR;
    memmove(f, rebootcode, sizeof(rebootcode));

    print("rebooting...\n");

    /* off we go - never to return */
    arch_coherence();
    (*f)(PADDR(entry), PADDR(code), size);
}
/*e: function [[reboot]](x86) */

//*****************************************************************************
// Misc
//*****************************************************************************

/*s: function [[isaconfig]](x86) */
int
main_isaconfig(char *class, int ctlrno, ISAConf *isa)
{
        char cc[32], *p;
        int i;

        snprint(cc, sizeof cc, "%s%d", class, ctlrno);
        p = getconf(cc);
        if(p == nil)
                return 0;

        isa->type = "";
        isa->nopt = tokenize(p, isa->opt, NISAOPT);
        for(i = 0; i < isa->nopt; i++){
                p = isa->opt[i];
                if(cistrncmp(p, "type=", 5) == 0)
                        isa->type = p + 5;
                else if(cistrncmp(p, "port=", 5) == 0)
                        isa->port = strtoul(p+5, &p, 0);
                else if(cistrncmp(p, "irq=", 4) == 0)
                        isa->irq = strtoul(p+4, &p, 0);
                else if(cistrncmp(p, "dma=", 4) == 0)
                        isa->dma = strtoul(p+4, &p, 0);
                else if(cistrncmp(p, "mem=", 4) == 0)
                        isa->mem = strtoul(p+4, &p, 0);
                else if(cistrncmp(p, "size=", 5) == 0)
                        isa->size = strtoul(p+5, &p, 0);
                else if(cistrncmp(p, "freq=", 5) == 0)
                        isa->freq = strtoul(p+5, &p, 0);
        }
        return 1;
}
/*e: function [[isaconfig]](x86) */

//*****************************************************************************
// Main entry point!
//*****************************************************************************

/*s: function [[main]](x86) */
//@Scheck: not dead, entry point :) jumped from assembly at _startpg() end
void main(void)
{
    // initial assignment made to avoid circular dependencies in codegraph
    /*s: [[main()]] initial assignments for backward deps(x86) */
    devtab = conf_devtab;

    print = devcons_print;
    iprint = devcons_iprint;
    pprint = devcons_pprint;
    panic = devcons_panic;
    _assert = devcons__assert;
    error = proc_error;
    nexterror = proc_nexterror;
    dumpaproc = proc_dumpaproc;
    wakeup = proc_wakeup;
    sched = proc_sched;
    ready = proc_ready;
    sleep = proc_sleep;
    tsleep = proc_tsleep;
    cclose = chan_cclose;
    proctab = proc_proctab;
    postnote = proc_postnote;
    pexit = proc_pexit;
    hook_ioalloc = devarch_hook_ioalloc;

    arch_delay = i8253_delay;
    arch_microdelay = i8253_microdelay;
    /*
     * On a uniprocessor, you'd think that coherence could be nop,
     * but it can't.  We still need a barrier when using coherence() in
     * device drivers.
     *
     * On VMware, it's safe (and a huge win) to set this to nop.
     * Aux/vmware does this via the #P/archctl file.
     */
    arch_coherence = nop;
    arch_fastticks = devarch_arch_fastticks;
    arch_exit = main_exit;
    arch_dumpstack = trap_dumpstack;
    arch_isaconfig = main_isaconfig;
    /*e: [[main()]] initial assignments for backward deps(x86) */

    cgapost(0);

    cpu0init(); // cpu0 initialization (calls cpuinit())

    options(); // setup values for getconf() from bootloader config
    // example of manual config:
    // TODO confname(``console'') = 1?

    ioinit(); // does some getconf("ioexclude") so must be after options()

    arch__screeninit(); // screenputs = cgascreenputs
    quotefmtinstall(); // libc printf initialization
    i8250console(); // setup optional serial console if getconf("console") == 1
    print("\nPlan 9\n");

    // the init0 means this is really early on (e.g. malloc is not available)
    trapinit0();
    mmuinit0();

    kbdinit(); // kbd is then fully enabled below via kdbenable()
    i8253init(); // clock controller

    cpuidentify(); // setup cpu, to know which advanced features we can enable
    arch_cpuidprint();

    meminit(); // setup conf.mem memory banks and setup more PDEs and PTEs
    confinit(); // setup conf (and mainmem->maxsize, imagmem->maxsize)
    archinit(); // setup arch
    xinit(); // setup xlists for xalloc() and palloc.mem for pageinit()
    if(i8237alloc != nil)
            i8237alloc(); // setup DMA, need low memory below 16MB

    arch__trapinit();
    arch_mmuinit();

    fpsavealloc();
    if(arch->intrinit)      /* launches other processors on an mp */
        arch->intrinit();

    timersinit();
    mathinit();

    kbdqinit(); // setup kbdq
    kbdenable(); // enable interrupts
    lineqinit(); // setup lineq

    if(arch->clockenable)
        arch->clockenable();

    procinit();
    imageinit();

    links();

    // initialize all devices
    chandevreset();
    cgapost(0xcd); // 0xcd, for chandev :)

    pageinit(); // setup palloc.pages and swapalloc.highwater
    swapinit(); // setup swapalloc

    // let's craft our first process (that will then exec("boot/boot"))
    userinit();
    /*s: [[main()]] before schedinit()(x86) */
    active.main_reached_sched = true;
    /*e: [[main()]] before schedinit()(x86) */
    cgapost(0x99); // done!
    schedinit();
    panic("should never reach this point");
}
/*e: function [[main]](x86) */
/*e: init/386/main.c */
