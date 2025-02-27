/*s: init/arm/main.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

#include "io.h"
// for mainmem and imagmem
#include <pool.h>

// initcode binary
#include "init.h"

// rebootcode binary
#include "reboot.h"

// in portscreen.h
extern void swcursor_init(void);

/*s: constant [[Minfirmrev]](arm) */
/* Firmware compatibility */
#define Minfirmrev  326770
/*e: constant [[Minfirmrev]](arm) */
/*s: constant [[Minfirmdate]](arm) */
#define Minfirmdate "19 Aug 2013"
/*e: constant [[Minfirmdate]](arm) */


// part of a trick to remove some backward dependencies
int devcons_print(char*, ...);
int devcons_iprint(char*, ...);
int devcons_pprint(char*, ...);
void devcons_panic(char*, ...);
void devcons__assert(char*);
void proc_dumpaproc(Proc *p);
void proc_error(char*);
void proc_nexterror(void);
void proc_sched(void);
void proc_ready(Proc*);
void proc_sleep(Rendez*, int(*)(void*), void*);
void proc_tsleep(Rendez *r, int (*fn)(void*), void *arg, ulong ms);
Proc* proc_wakeup(Rendez*);
void proc_pexit(char *exitstr, bool freemem);
Proc* proc_proctab(int i);
int proc_postnote(Proc *p, int dolock, char *n, int flag);
void chan_cclose(Chan *c);

uvlong clock_arch_fastticks(uvlong *hz);
void clock_arch_delay(int millisecs);
void clock_arch_microdelay(int microsecs);
void trap_arch_dumpstack(void);
void arm_arch_coherence(void);
void main_arch_exit(int ispanic);
int  main_arch_isaconfig(char *class, int ctlrno, ISAConf *isa);

//*****************************************************************************
// Configuration
//*****************************************************************************
// See globals in portdat_globals.h

// <conf>.c
extern  Dev*  conf_devtab[];
extern  char*   conffile;

/*s: function [[getconf]](arm) */
char*
getconf(char *name)
{
    USED(name);
    //int i;
    // 
    //i = findconf(name);
    //if(i >= 0)
    //  return confval[i];
    return nil;
}
/*e: function [[getconf]](arm) */

//*****************************************************************************
// Boot parameters, see bootconf.c (not used by pad)
//*****************************************************************************

//*****************************************************************************
// Cpu init
//*****************************************************************************
/*s: function [[arch__cpuinit]](arm) */
void
arch__cpuinit(void)
{
    Cpu *cpu0;

    cpu->ticks = 1;
    cpu->perf.period = 1;

    cpu0 = CPUS(0);
    /*s: [[arch__cpuinit()]] it not cpu0 */
    if (cpu->cpuno != 0) {
        /* synchronise with cpu 0 */
        cpu->ticks = cpu0->ticks;
    }
    /*e: [[arch__cpuinit()]] it not cpu0 */
}
/*e: function [[arch__cpuinit]](arm) */

/*s: function [[arch__cpu0init]](arm) */
void
arch__cpu0init(void)
{
    conf.ncpu = 0; // set in machon() instead (machon() is called after cpuinit)

    cpu->cpuno = 0;
    cpus[cpu->cpuno] = cpu;

    arch__cpuinit();
    active.exiting = false;

    up = nil;
}
/*e: function [[arch__cpu0init]](arm) */

/*s: function [[machon]](arm) */
/* enable scheduling of this cpu */
void
machon(uint xcpu)
{
    ulong cpubit;

    cpubit = 1 << xcpu;
    lock(&active);
    if ((active.cpus & cpubit) == 0) {  /* currently off? */
        conf.ncpu++;
        active.cpus |= cpubit;
    }
    unlock(&active);
}
/*e: function [[machon]](arm) */

//*****************************************************************************
// Conf init
//*****************************************************************************

/*s: global [[memsize]](arm) */
// used also in mmu.c
ulong   memsize = 128*1024*1024;
/*e: global [[memsize]](arm) */

/*s: function [[arch__confinit]](arm) */
void
arch__confinit(void)
{
    int i;
    char *p;
    phys_addr pa;
    ulong kpages;
    ulong kmem;

    /*s: [[arch__confinit()]] if maxmem boot parameter(arm) */
    if((p = getconf("*maxmem")) != nil){
        memsize = strtoul(p, 0, 0);
        if (memsize < 16*MB)        /* sanity */
            memsize = 16*MB;
    }
    /*e: [[arch__confinit()]] if maxmem boot parameter(arm) */
    // simpler than for x86 :)
    getramsize(&conf.mem[0]);
    /*s: [[arch__confinit()]] sanity check [[conf.mem[0]]](arm) */
    if(conf.mem[0].limit == 0){
        conf.mem[0].base = 0;
        conf.mem[0].limit = memsize;
    }
    /*s: [[arch__confinit()]] if maxmem parameter and limit mem is zero(arm) */
    else if(p != nil)
        conf.mem[0].limit = conf.mem[0].base + memsize;
    /*e: [[arch__confinit()]] if maxmem parameter and limit mem is zero(arm) */
    /*e: [[arch__confinit()]] sanity check [[conf.mem[0]]](arm) */

    conf.npage = 0;
    pa = PADDR(PGROUND(PTR2UINT(end)));

    /*
     *  we assume that the kernel is at the beginning of one of the
     *  contiguous chunks of memory and fits therein.
     */
    for(i=0; i<nelem(conf.mem); i++){
        /* take kernel out of allocatable space */
        if(pa > conf.mem[i].base && pa < conf.mem[i].limit)
            conf.mem[i].base = pa;

        conf.mem[i].npage = (conf.mem[i].limit - conf.mem[i].base)/BY2PG;
        conf.npage += conf.mem[i].npage;
    }

    conf.upages = (conf.npage*80)/100;
    kpages = conf.npage - conf.upages;

    /* set up other configuration parameters */
    conf.ialloc = (kpages/2)*BY2PG; // max bytes for iallocb

    conf.nproc = 100 + ((conf.npage*BY2PG)/MB)*5;
    if(cpuserver)
        conf.nproc *= 3;
    if(conf.nproc > 2000)
        conf.nproc = 2000;

    conf.nswap = conf.npage*3;
    conf.nswppo = 4096;
    conf.nimage = 200;

    conf.copymode = 1;      /* copy on reference, not copy on write */

    /*
     * Guess how much is taken by the large permanent
     * datastructures. Mntcache and Mntrpc are not accounted for
     * (probably ~300KB).
     */
    kmem = kpages * BY2PG;
    kmem -= 
          conf.upages * sizeof(Page)
        + conf.nproc  * sizeof(Proc)
        + conf.nimage * sizeof(KImage)
        + conf.nswap
        + conf.nswppo * sizeof(Page*); // pad's second bugfix :)

    // memory pool
    mainmem->maxsize = kmem;

    if(!cpuserver)
        /*
         * give terminals lots of image memory, too; the dynamic
         * allocation will balance the load properly, hopefully.
         * be careful with 32-bit overflow.
         */
        imagmem->maxsize = kmem;
}
/*e: function [[arch__confinit]](arm) */

//*****************************************************************************
// First process init
//*****************************************************************************

/*s: global [[sp]](arm) */
static user_addr sp;      /* XXX - must go - user stack of init proc */
/*e: global [[sp]](arm) */

// kernel space instructions executed by first process
/*s: function [[init0]](arm) */
/*
 *  starting place for first process
 */
void
init0(void)
{
    int i;
    char buf[2*KNAMELEN]; // has to be the same than in ksetenv?

    up->nerrlab = 0;

    /*s: [[init0()]] coherence(arm) */
    arch_coherence();
    /*e: [[init0()]] coherence(arm) */
    arch_spllo(); // Allow interrupts!!!

    /*
     * These are o.k. because rootinit is null.
     * Then early kproc's will have a root and dot.
     */
    up->slash = namec("#/", Atodir, 0, 0);
    pathclose(up->slash->path);
    up->slash->path = newpath("/");
    up->dot = cclone(up->slash);

    chandevinit();

    if(!waserror()){
        snprint(buf, sizeof(buf), "%s %s", "ARM", conffile);
        ksetenv("terminal", buf, 0);
        ksetenv("cputype", "arm", 0); // used by mkfile! 
        if(cpuserver)
            ksetenv("service", "cpu", 0);
        else
            ksetenv("service", "terminal", 0);
        snprint(buf, sizeof(buf), "-a %s", getethermac());
        ksetenv("etherargs", buf, 0);

        /* convert plan9.ini variables to #e and #ec */
        for(i = 0; i < nconf; i++) {
            ksetenv(confname[i], confval[i], 0);
            ksetenv(confname[i], confval[i], 1);
        }
        poperror();
    }
    kproc("alarm", alarmkproc, nil); // ??

    // !! Go to user space !!
    arch_touser(sp);

    assert(0);          /* shouldn't have returned */
}
/*e: function [[init0]](arm) */

// user space instructions executed by first process
// see initcode in init.h (comes from ../port/initcode.c and init9.s

extern uintptr bootargs(uintptr base);
/*s: function [[arch__userinit]](arm) */
/*
 *  create the first process
 */
void
arch__userinit(void)
{
    Proc *p;
    Segment *s;
    Arch_KMap *k;
    Page *pg;

    /* no processes yet */
    up = nil;

    p = newproc();
    p->pgrp = newpgrp();
    p->egrp = smalloc(sizeof(Egrp));
    p->egrp->ref = 1;
    p->fgrp = dupfgrp(nil);
    p->rgrp = newrgrp();
    p->procmode = 0640;

    kstrdup(&eve, "");
    kstrdup(&p->text, "*init*");
    kstrdup(&p->user, eve);

    /*
     * Kernel Stack
     */
    p->sched.pc = PTR2UINT(init0);
    p->sched.sp = PTR2UINT(p->kstack + KSTACK
                           - sizeof(up->sargs.args)
                           - sizeof(uintptr));
    p->sched.sp = STACKALIGN(p->sched.sp);

    /*
     * User Stack
     *
     * Technically, newpage can't be called here because it
     * should only be called when in a user context as it may
     * try to sleep if there are no pages available, but that
     * shouldn't be the case here.
     */
    s = newseg(SG_STACK, USTKTOP-USTKSIZE, USTKSIZE/BY2PG);
    /*s: [[arch__userinit()]] set flushme for stack segment */
    s->flushme = true;
    /*e: [[arch__userinit()]] set flushme for stack segment */
    p->seg[SSEG] = s;
    pg = newpage(true, nil, USTKTOP-BY2PG);
    segpage(s, pg);

    k = arch_kmap(pg);
    sp = bootargs(VA(k));
    arch_kunmap(k);

    /*
     * Text
     */
    s = newseg(SG_TEXT, UTZERO, 1); // initcode needs only 1 page
    p->seg[TSEG] = s;
    pg = newpage(true, nil, UTZERO);
    /*s: [[arch__userinit()]] set cachectl(arm) */
    memset(pg->cachectl, PG_TXTFLUSH, sizeof(pg->cachectl));
    /*e: [[arch__userinit()]] set cachectl(arm) */
    segpage(s, pg);

    k = arch_kmap(s->pagedir[0]->pagetab[0]);
    memmove(UINT2PTR(VA(k)), initcode, sizeof initcode);
    arch_kunmap(k);

    // ready to go!
    ready(p);
}
/*e: function [[arch__userinit]](arm) */


//*****************************************************************************
// Shutdown/reboot
//*****************************************************************************

/*s: function [[shutdown]]([[(init/arm/main.c)(arm)]]) */
static void
shutdown(int ispanic)
{
    int ms, once;

    lock(&active);
    if(ispanic)
        active.ispanic = ispanic;
    else if(cpu->cpuno == 0 && (active.cpus & (1 << cpu->cpuno)) == 0)
        active.ispanic = 0;
    once = active.cpus & (1 << cpu->cpuno);
    active.cpus &= ~(1 << cpu->cpuno);
    active.exiting = 1;
    unlock(&active);

    if(once)
        iprint("cpu%d: exiting\n", cpu->cpuno);
    arch_spllo();
    for(ms = 5*1000; ms > 0; ms -= TK2MS(2)){
        arch_delay(TK2MS(2));
        if(active.cpus == 0 && consactive() == 0)
            break;
    }
    arch_delay(100*cpu->cpuno);
}
/*e: function [[shutdown]]([[(init/arm/main.c)(arm)]]) */

/*s: function [[main_arch_exit]](arm) */
/*
 *  exit kernel either on a panic or user request
 */
void
main_arch_exit(int code)
{
    void (*f)(ulong, ulong, ulong);

    shutdown(code);
    splfhi();
    if(cpu->cpuno == 0)
        archreboot();
    else{
        f = (void*)REBOOTADDR;
        intrcpushutdown();
        cacheuwbinv();
        l2cacheuwbinv();
        (*f)(0, 0, 0);
        for(;;){}
    }
}
/*e: function [[main_arch_exit]](arm) */

/*s: function [[arch_reboot]](arm) */
/*
 * the new kernel is already loaded at address `code'
 * of size `size' and entry point `entry'.
 */
void
arch_reboot(void *entry, void *code, ulong size)
{
    void (*f)(ulong, ulong, ulong);

    //writeconf();

    /*
     * the boot processor is cpu0.  execute this function on it
     * so that the new kernel has the same cpu0.
     */
    if (cpu->cpuno != 0) {
        procwired(up, 0);
        sched();
    }
    if (cpu->cpuno != 0)
        print("on cpu%d (not 0)!\n", cpu->cpuno);

    /* setup reboot trampoline function */
    f = (void*)REBOOTADDR;
    memmove(f, rebootcode, sizeof(rebootcode));
    cachedwbse(f, sizeof(rebootcode));

    shutdown(0);

    /*
     * should be the only processor running now
     */

    arch_delay(5000);
    print("active.machs = %x\n", active.cpus);
    print("reboot entry %#lux code %#lux size %ld\n",
        PADDR(entry), PADDR(code), size);
    arch_delay(100);

    /* turn off buffered serial console */
    serialoq = nil;
    kprintoq = nil;
    screenputs = nil;

    /* shutdown devices */
    if(!waserror()){
        chandevshutdown();
        poperror();
    }

    /* stop the clock (and watchdog if any) */
    clockshutdown();

    splfhi();
    intrshutdown();

    /* off we go - never to return */
    cacheuwbinv();
    l2cacheuwbinv();
    (*f)(PADDR(entry), PADDR(code), size);

    iprint("loaded kernel returned!\n");
    arch_delay(1000);
    archreboot();
}
/*e: function [[arch_reboot]](arm) */

//*****************************************************************************
// Misc
//*****************************************************************************

/*s: function [[main_arch_isaconfig]](arm) */
/*
 * stub for ../omap/devether.c
 */
int
main_arch_isaconfig(char *class, int ctlrno, ISAConf *isa)
{
    char cc[32], *p;
    int i;

    if(strcmp(class, "ether") != 0)
        return 0;
    snprint(cc, sizeof cc, "%s%d", class, ctlrno);
    p = getconf(cc);
    if(p == nil)
        return (ctlrno == 0);
    isa->type = "";
    isa->nopt = tokenize(p, isa->opt, NISAOPT);
    for(i = 0; i < isa->nopt; i++){
        p = isa->opt[i];
        if(cistrncmp(p, "type=", 5) == 0)
            isa->type = p + 5;
    }
    return 1;
}
/*e: function [[main_arch_isaconfig]](arm) */

/*s: function [[arch_memorysummary]](arm) */
// called from devcons.c
void
arch_memorysummary(void) {
}
/*e: function [[arch_memorysummary]](arm) */

//*****************************************************************************
// Main entry point!
//*****************************************************************************

/*s: function [[launchinit]](arm) */
void
launchinit(int ncpus)
{
    int xcpu;
    Cpu *mm;
    PTE *l1;

    /*s: [[launchinit()]] sanity check ncpus(arm) */
    if(ncpus > MAXCPUS)
        ncpus = MAXCPUS;
    /*e: [[launchinit()]] sanity check ncpus(arm) */
    for(xcpu = 1; xcpu < ncpus; xcpu++){
        cpus[xcpu] = mm = mallocalign(CPUSIZE, CPUSIZE, 0, 0);
        l1 = mallocalign(L1SIZE, L1SIZE, 0, 0);
        if(mm == nil || l1 == nil)
            panic("launchinit");
        memset(mm, 0, CPUSIZE);
        mm->cpuno = xcpu;

        memmove(l1, cpu->mmul1, L1SIZE);  /* clone cpu0's l1 table */
        cachedwbse(l1, L1SIZE);
        mm->mmul1 = l1;
        cachedwbse(mm, CPUSIZE);

    }
    cachedwbse(cpus, sizeof cpus);
    xcpu = startcpus(ncpus);
    if(xcpu < ncpus)
            panic("only %d cpu%s started", xcpu, xcpu == 1? "" : "s");
}
/*e: function [[launchinit]](arm) */

/*s: function [[main]](arm) */
void
main(void)
{
    /*s: [[main()]] locals(arm) */
    uint firmware, board;
    /*e: [[main()]] locals(arm) */

    /*s: [[main()]] initial assignments for backward deps(arm) */
    // initial assignments to avoid circular dependencies in codegraph
    // backward deps breaker!
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

    arch_exit = main_arch_exit;
    arch_dumpstack = trap_arch_dumpstack;
    arch_delay = clock_arch_delay;
    arch_microdelay = clock_arch_microdelay;
    arch_coherence = arm_arch_coherence;
    arch_fastticks = clock_arch_fastticks;
    arch_isaconfig = main_arch_isaconfig;
    /*e: [[main()]] initial assignments for backward deps(arm) */

    /*s: [[main()]] clear bss(arm) */
    memset(edata, 0, end - edata);  /* clear bss */
    /*e: [[main()]] clear bss(arm) */

    // Let's go!

    cpu = (Cpu*)CPUADDR;
    arch__cpu0init(); // cpu0 initialization (calls arch__cpuinit())
    mmuinit1((void*)L1); // finish mmu initialization started in mmuinit0
    machon(0); // enable cpu0 for scheduling

    /*s: [[main()]] parsing options and boot arguments(arm) */
    //optionsinit("/boot/boot boot"); // setup values for getconf() 
    //ataginit((Atag*)BOOTARGS); // from bootloader config
    // example of manual config:
    // TODO confname(``console'') = 1?
    /*e: [[main()]] parsing options and boot arguments(arm) */
    arch__confinit();     /* figures out amount of memory */
    xinit();

    uartconsinit();

    arch__screeninit(); // screenputs = swconsole_screenputs
    quotefmtinstall(); // libc printf initialization

    print("\nPlan 9 from Bell Labs\n"); // yeah!

    /*s: [[main()]] print board and firmware information(arm) */
    board = getboardrev();
    firmware = getfirmware();
    print("board rev: %#ux firmware rev: %d\n", board, firmware);
    if(firmware < Minfirmrev){
        print("Sorry, firmware (start*.elf) must be at least rev %d"
              " or newer than %s\n", Minfirmrev, Minfirmdate);
        for(;;)
            ;
    }
    /*e: [[main()]] print board and firmware information(arm) */
    /* set clock rate to arm_freq from config.txt (default pi1:700Mhz pi2:900MHz) */
    setclkrate(ClkArm, 0);

    arch__trapinit();
    clockinit(); // enable timer interrupts

    //TODO? kbdqinit(); // setup kbdq
    lineqinit(); // setup lineq

    timersinit(); // set initial timer for timer interrupt
    swcursor_init();

    arch_cpuidprint();
    archreset();

    procinit();
    imageinit();

    links();

    // initialize all devices
    chandevreset();         /* most devices are discovered here */

    pageinit(); // setup palloc.pages and swapalloc.highwater
    swapinit(); // setup swapalloc

    // let's craft our first process (that will then exec("boot/boot"))
    arch__userinit();

    /*s: [[main()]] start the other processors(arm) */
    launchinit(getncpus());
    /*e: [[main()]] start the other processors(arm) */

    // schedule the only ready user process (the one created by userinit)
    schedinit();

    assert(0);          /* shouldn't have returned */
}
/*e: function [[main]](arm) */
/*e: init/arm/main.c */
