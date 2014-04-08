#include        "u.h"
#include        "../port/lib.h"
#include        "mem.h"
#include        "dat.h"
#include        "fns.h"


#include        "io.h"
#include        "ureg.h"

#include        "pool.h"

#include        "init.h"
#include        "reboot.h"

#include        "mp.h"

#include        <tos.h>

void confinit(void);
void userinit(void);
void bootargs(void*);
void mach0init(void);
void mathinit(void);

// to avoid backward deps
void devcons__assert(char*);
char* main_getconf(char*);
void trap_dumpstack(void);
void proc_error(char*);
int devcons_iprint(char*, ...);
int devcons_print(char*, ...);
void proc_nexterror(void);
void devcons_panic(char*, ...);
int devcons_pprint(char*, ...);
void i8253_delay(int millisecs);
void i8253_microdelay(int microsecs);
Proc* proc_wakeup(Rendez*);
void proc_sched(void);
void proc_ready(Proc*);
void proc_sleep(Rendez*, int(*)(void*), void*);
void main_exit(int ispanic);
int  main_isaconfig(char *class, int ctlrno, ISAConf *isa);
void nop(void);
void proc_dumpaproc(Proc *p);
uvlong devarch_fastticks(uvlong *hz);
void chan_cclose(Chan *c);
Proc* proc_proctab(int i);
void proc_tsleep(Rendez *r, int (*fn)(void*), void *arg, ulong ms);
int proc_postnote(Proc *p, int dolock, char *n, int flag);
int sysproc_return0(void*);
void proc_pexit(char *exitstr, int freemem);

extern void mmuinit0(void);
extern void (*i8237alloc)(void);

//*****************************************************************************
// Globals
//*****************************************************************************

// now in portdat_globals.c
//Conf conf;

//*****************************************************************************
// Boot parameters (not used by pad)
//*****************************************************************************

/*
 * Where configuration info is left for the loaded programme.
 * This will turn into a structure as more is done by the boot loader
 * (e.g. why parse the .ini file twice?).
 * There are 3584 bytes available at CONFADDR.
 */
#define BOOTLINE        ((char*)CONFADDR)
#define BOOTLINELEN     64
#define BOOTARGS        ((char*)(CONFADDR+BOOTLINELEN))
#define BOOTARGSLEN     (4096-0x200-BOOTLINELEN)
#define MAXCONF         64

enum {
        /* space for syscall args, return PC, top-of-stack struct */
        Ustkheadroom    = sizeof(Sargs) + sizeof(uintptr) + sizeof(Tos),
};

//char bootdisk[KNAMELEN];
char *confname[MAXCONF];
char *confval[MAXCONF];
int nconf;

// Global! set by bootargs()
uchar *sp;      /* user stack of init proc */

int delaylink;

//@Scheck: Assembly
extern ulong *multiboot;

static void
options(void)
{
        long i, n;
        char *cp, *line[MAXCONF], *p, *q;
        ulong *m, l;

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

//*****************************************************************************
// Conf init
//*****************************************************************************

// getconf()
char*
main_getconf(char *name)
{
        int i;

        for(i = 0; i < nconf; i++)
                if(cistrcmp(confname[i], name) == 0)
                        return confval[i];
        return 0;
}

//*****************************************************************************
// Main entry point!
//*****************************************************************************

//@Scheck: not dead, entry point :) jumped to by qemu (via elf header)
void
main(void)
{

  // initial assignment made to avoid circular dependencies in codegraph
  print = devcons_print;
  iprint = devcons_iprint;
  pprint = devcons_pprint;
  
  panic = devcons_panic;
  _assert = devcons__assert;
  
  error = proc_error;
  nexterror = proc_nexterror;
  
  dumpstack = trap_dumpstack;
  dumpaproc = proc_dumpaproc;
  
  devtab = conf_devtab;
  getconf = main_getconf;
  
  delay = i8253_delay;
  microdelay = i8253_microdelay;

  wakeup = proc_wakeup;
  sched = proc_sched;
  ready = proc_ready;
  sleep = proc_sleep;
  tsleep = proc_tsleep;

  exit = main_exit;
  isaconfig = main_isaconfig;
  
  /*
   * On a uniprocessor, you'd think that coherence could be nop,
   * but it can't.  We still need a barrier when using coherence() in
   * device drivers.
   *
   * On VMware, it's safe (and a huge win) to set this to nop.
   * Aux/vmware does this via the #P/archctl file.
   */
  coherence = nop;
  
  fastticks = devarch_fastticks;
  
  cclose = chan_cclose;

  proctab = proc_proctab;
  postnote = proc_postnote;
  return0 = sysproc_return0;
  pexit = proc_pexit;

  // end patch, back to original code

        cgapost(0);

        mach0init();
        options();
        ioinit();
        i8250console();
        quotefmtinstall();
        screeninit();

        print("\nPlan 99999999999999\n");

        // the init0 means this is really early on (malloc is not available?!)
        trapinit0();
        mmuinit0();

        kbdinit();
        i8253init();
        cpuidentify();

        meminit();
        confinit();
        archinit();
        xinit();
        if(i8237alloc != nil)
                i8237alloc();
        trapinit();
        printinit();
        cpuidprint();
        mmuinit();
        fpsavealloc();
        if(arch->intrinit)      /* launches other processors on an mp */
                arch->intrinit();
        timersinit();
        mathinit();
        kbdenable();
        if(arch->clockenable)
                arch->clockenable();

        procinit0();
        initseg();
        if(delaylink) {
                bootlinks();
                pcimatch(0, 0, 0);
        } else
                links();
        conf.monitor = 1;
        chandevreset();
        cgapost(0xcd);

        pageinit();
        i8253link();
        swapinit();

        userinit();
        active.thunderbirdsarego = 1;

        cgapost(0x99);
        schedinit();
}

void
mach0init(void)
{
        conf.nmach = 1;
        MACHP(0) = (Mach*)CPU0MACH;
        m->pdb = (ulong*)CPU0PDB;
        m->gdt = (Segdesc*)CPU0GDT;

        machinit();

        active.machs = 1;
        active.exiting = 0;
}


// set by userinit to sched.pc
void
init0(void)
{
        int i;
        char buf[2*KNAMELEN];
        
        up->nerrlab = 0;

        spllo();

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
                snprint(buf, sizeof(buf), "%s %s", arch->id, conffile);
                ksetenv("terminal", buf, 0);
                ksetenv("cputype", "386", 0);
                if(cpuserver)
                        ksetenv("service", "cpu", 0);
                else
                        ksetenv("service", "terminal", 0);
                for(i = 0; i < nconf; i++){
                        if(confname[i][0] != '*')
                                ksetenv(confname[i], confval[i], 0);
                        ksetenv(confname[i], confval[i], 1);
                }
                poperror();
        }
        kproc("alarm", alarmkproc, 0);
        cgapost(0x9);
        touser(sp);
}

void
userinit(void)
{
        void *v;
        Proc *p;
        Segment *s;
        Page *pg;

        p = newproc();
        p->pgrp = newpgrp();
        p->egrp = smalloc(sizeof(Egrp)); //todo: newegrp()
        p->egrp->ref = 1;
        p->fgrp = dupfgrp(nil);
        p->rgrp = newrgrp();
        p->procmode = 0640;

        kstrdup(&eve, "");
        kstrdup(&p->text, "*init*");
        kstrdup(&p->user, eve);

        p->fpstate = FPinit;
        fpoff();

        /*
         * Kernel Stack
         *
         * N.B. make sure there's enough space for syscall to check
         *      for valid args and 
         *      4 bytes for gotolabel's return PC
         */
        p->sched.pc = (ulong)init0;
        p->sched.sp = (ulong)p->kstack+KSTACK-(sizeof(Sargs)+BY2WD);

        /*
         * User Stack
         *
         * N.B. cannot call newpage() with clear=1, because pc kmap
         * requires up != nil.  use tmpmap instead.
         */
        s = newseg(SG_STACK, USTKTOP-USTKSIZE, USTKSIZE/BY2PG);
        p->seg[SSEG] = s;
        pg = newpage(0, 0, USTKTOP-BY2PG);
        v = tmpmap(pg);
        memset(v, 0, BY2PG);
        segpage(s, pg);

        bootargs(v);
        tmpunmap(v);

        /*
         * Text
         */
        s = newseg(SG_TEXT, UTZERO, 1);
        s->flushme++;
        p->seg[TSEG] = s;
        pg = newpage(0, 0, UTZERO);
        memset(pg->cachectl, PG_TXTFLUSH, sizeof(pg->cachectl));
        segpage(s, pg);
        v = tmpmap(pg);
        memset(v, 0, BY2PG);
        memmove(v, initcode, sizeof initcode);
        tmpunmap(v);

        ready(p);
}

uchar *
pusharg(char *p)
{
        int n;

        n = strlen(p)+1;
        sp -= n;
        memmove(sp, p, n);
        return sp;
}

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

void
confinit(void)
{
        char *p;
        int i, userpcnt;
        ulong kpages;

        if(p = getconf("*kernelpercent"))
                userpcnt = 100 - strtol(p, 0, 0);
        else
                userpcnt = 0;

        conf.npage = 0;
        for(i=0; i<nelem(conf.mem); i++)
                conf.npage += conf.mem[i].npage;

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
                                userpcnt = 60;
                }
                kpages = conf.npage - (conf.npage*userpcnt)/100;

                /*
                 * Make sure terminals with low memory get at least
                 * 4MB on the first Image chunk allocation.
                 */
                if(conf.npage*BY2PG < 16*MB)
                        imagmem->minarena = 4*1024*1024;
        }

        /*
         * can't go past the end of virtual memory
         * (ulong)-KZERO is 2^32 - KZERO
         */
        if(kpages > ((ulong)-KZERO)/BY2PG)
                kpages = ((ulong)-KZERO)/BY2PG;

        conf.upages = conf.npage - kpages;
        conf.ialloc = (kpages/2)*BY2PG;

        /*
         * Guess how much is taken by the large permanent
         * datastructures. Mntcache and Mntrpc are not accounted for
         * (probably ~300KB).
         */
        kpages *= BY2PG;
        kpages -= conf.upages*sizeof(Page)
                + conf.nproc*sizeof(Proc)
                + conf.nimage*sizeof(KImage)
                + conf.nswap
                + conf.nswppo*sizeof(Page);
        mainmem->maxsize = kpages;
        if(!cpuserver){
                /*
                 * give terminals lots of image memory, too; the dynamic
                 * allocation will balance the load properly, hopefully.
                 * be careful with 32-bit overflow.
                 */
                imagmem->maxsize = kpages;
        }
}

static char* mathmsg[] =
{
        nil,    /* handled below */
        "denormalized operand",
        "division by zero",
        "numeric overflow",
        "numeric underflow",
        "precision loss",
};

static void
mathstate(ulong *stsp, ulong *pcp, ulong *ctlp)
{
        ulong sts, fpc, ctl;
        FPsave *f = &up->fpsave;

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
        if(!(m->cpuiddx & Fpuonchip))
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

/*
 *  math coprocessor segment overrun
 */
static void
mathover(Ureg*, void*)
{
        pexit("math overrun", 0);
}

void
mathinit(void)
{
        trapenable(VectorCERR, matherror, 0, "matherror");
        if(X86FAMILY(m->cpuidax) == 3)
                intrenable(IrqIRQ13, matherror, 0, BUSUNKNOWN, "matherror");
        trapenable(VectorCNA, mathemu, 0, "mathemu");
        trapenable(VectorCSO, mathover, 0, "mathover");
}


static void
shutdown(int ispanic)
{
        int ms, once;

        lock(&active);
        if(ispanic)
                active.ispanic = ispanic;
        else if(m->machno == 0 && (active.machs & (1<<m->machno)) == 0)
                active.ispanic = 0;
        once = active.machs & (1<<m->machno);
        /*
         * setting exiting will make hzclock() on each processor call exit(0),
         * which calls shutdown(0) and arch->reset(), which on mp systems is
         * mpshutdown, which idles non-bootstrap cpus and returns on bootstrap
         * processors (to permit a reboot).  clearing our bit in machs avoids
         * calling exit(0) from hzclock() on this processor.
         */
        active.machs &= ~(1<<m->machno);
        active.exiting = 1;
        unlock(&active);

        if(once)
                iprint("cpu%d: exiting\n", m->machno);

        /* wait for any other processors to shutdown */
        spllo();
        for(ms = 5*1000; ms > 0; ms -= TK2MS(2)){
                delay(TK2MS(2));
                if(active.machs == 0 && consactive() == 0)
                        break;
        }

        if(active.ispanic){
                if(!cpuserver)
                        for(;;)
                                halt();
                if(getconf("*debug"))
                        delay(5*60*1000);
                else
                        delay(10000);
        }else
                delay(1000);
}

void
reboot(void *entry, void *code, ulong size)
{
        void (*f)(ulong, ulong, ulong);
        ulong *pdb;

        writeconf();

        /*
         * the boot processor is cpu0.  execute this function on it
         * so that the new kernel has the same cpu0.  this only matters
         * because the hardware has a notion of which processor was the
         * boot processor and we look at it at start up.
         */
        if (m->machno != 0) {
                procwired(up, 0);
                sched();
        }

        if(conf.nmach > 1) {
                /*
                 * the other cpus could be holding locks that will never get
                 * released (e.g., in the print path) if we put them into
                 * reset now, so force them to shutdown gracefully first.
                 */
                lock(&active);
                active.rebooting = 1;
                unlock(&active);
                shutdown(0);
                if(arch->resetothers)
                        arch->resetothers();
                delay(20);
        }

        /*
         * should be the only processor running now
         */
        active.machs = 0;
        if (m->machno != 0)
                print("on cpu%d (not 0)!\n", m->machno);

        print("shutting down...\n");
        delay(200);

        splhi();

        /* turn off buffered serial console */
        serialoq = nil;

        /* shutdown devices */
        chandevshutdown();
        arch->introff();

        /*
         * Modify the machine page table to directly map the low 4MB of memory
         * This allows the reboot code to turn off the page mapping
         */
        pdb = m->pdb;
        pdb[PDX(0)] = pdb[PDX(KZERO)];
        mmuflushtlb(PADDR(pdb));

        /* setup reboot trampoline function */
        f = (void*)REBOOTADDR;
        memmove(f, rebootcode, sizeof(rebootcode));

        print("rebooting...\n");

        /* off we go - never to return */
        coherence();
        (*f)(PADDR(entry), PADDR(code), size);
}


// exit()
void
main_exit(int ispanic)
{
        shutdown(ispanic);
        arch->reset();
}

// isaconfig()
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
