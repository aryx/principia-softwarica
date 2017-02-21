/*s: devices/sys/386/devarch.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

#include "io.h"

#include <ureg.h>

enum {
    Qdir = 0,
    Qioalloc = 1,
    Qiob,
    Qiow,
    Qiol,
    Qbase,

    Qmax = 16,
};

typedef long Rdwrfn(Chan*, void*, long, vlong);

extern int doi8253set;
extern PCArch* knownarch[];


static Rdwrfn *readfn[Qmax];
static Rdwrfn *writefn[Qmax];

static Dirtab archdir[Qmax] = {
    ".",        { Qdir, 0, QTDIR }, 0,  0555,
    "ioalloc",  { Qioalloc, 0 },    0,  0444,
    "iob",      { Qiob, 0 },        0,  0660,
    "iow",      { Qiow, 0 },        0,  0660,
    "iol",      { Qiol, 0 },        0,  0660,
};
Lock archwlock; /* the lock is only for changing archdir */
int narchdir = Qbase;


/*
 * Add a file to the #P listing.  Once added, you can't delete it.
 * You can't add a file with the same name as one already there,
 * and you get a pointer to the Dirtab entry so you can do things
 * like change the Qid version.  Changing the Qid path is disallowed.
 */
Dirtab*
addarchfile(char *name, int perm, Rdwrfn *rdfn, Rdwrfn *wrfn)
{
    int i;
    Dirtab d;
    Dirtab *dp;

    memset(&d, 0, sizeof d);
    strcpy(d.name, name);
    d.perm = perm;

    lock(&archwlock);
    if(narchdir >= Qmax){
        unlock(&archwlock);
        return nil;
    }

    for(i=0; i<narchdir; i++)
        if(strcmp(archdir[i].name, name) == 0){
            unlock(&archwlock);
            return nil;
        }

    d.qid.path = narchdir;
    archdir[narchdir] = d;
    readfn[narchdir] = rdfn;
    writefn[narchdir] = wrfn;
    dp = &archdir[narchdir++];
    unlock(&archwlock);

    return dp;
}

void devarch_hook_ioalloc() {
  archdir[0].qid.vers++;
}


int
iounused(int start, int end)
{
    IOMap *m;

    for(m = iomap.m; m; m = m->next){
        if(start >= m->start && start < m->end
        || start <= m->start && end > m->start)
            return 0;
    }
    return 1;
}

static void
checkport(int start, int end)
{
    /* standard vga regs are OK */
    if(start >= 0x2b0 && end <= 0x2df+1)
        return;
    if(start >= 0x3c0 && end <= 0x3da+1)
        return;

    if(iounused(start, end))
        return;
    error(Eperm);
}




static Chan*
archattach(char* spec)
{
    return devattach('P', spec);
}

Walkqid*
archwalk(Chan* c, Chan *nc, char** name, int nname)
{
    return devwalk(c, nc, name, nname, archdir, narchdir, devgen);
}

static int
archstat(Chan* c, uchar* dp, int n)
{
    return devstat(c, dp, n, archdir, narchdir, devgen);
}

static Chan*
archopen(Chan* c, int omode)
{
    return devopen(c, omode, archdir, narchdir, devgen);
}

static void
archclose(Chan*)
{
}

enum
{
    Linelen= 31,
};

static long
archread(Chan *c, void *a, long n, vlong offset)
{
    char *buf, *p;
    int port;
    ushort *sp;
    ulong *lp;
    IOMap *m;
    Rdwrfn *fn;

    switch((ulong)c->qid.path){

    case Qdir:
        return devdirread(c, a, n, archdir, narchdir, devgen);

    case Qiob:
        port = offset;
        checkport(offset, offset+n);
        for(p = a; port < offset+n; port++)
            *p++ = inb(port);
        return n;

    case Qiow:
        if(n & 1)
            error(Ebadarg);
        checkport(offset, offset+n);
        sp = a;
        for(port = offset; port < offset+n; port += 2)
            *sp++ = ins(port);
        return n;

    case Qiol:
        if(n & 3)
            error(Ebadarg);
        checkport(offset, offset+n);
        lp = a;
        for(port = offset; port < offset+n; port += 4)
            *lp++ = inl(port);
        return n;

    case Qioalloc:
        break;

    default:
        if(c->qid.path < narchdir && (fn = readfn[c->qid.path]))
            return fn(c, a, n, offset);
        error(Eperm);
        break;
    }

    if((buf = malloc(n)) == nil)
        error(Enomem);
    p = buf;
    n = n/Linelen;
    offset = offset/Linelen;

    lock(&iomap);
    for(m = iomap.m; n > 0 && m != nil; m = m->next){
        if(offset-- > 0)
            continue;
        seprint(p, &buf[n], "%8lux %8lux %-12.12s\n", m->start,
            m->end-1, m->tag);
        p += Linelen;
        n--;
    }
    unlock(&iomap);

    n = p - buf;
    memmove(a, buf, n);
    free(buf);

    return n;
}

static long
archwrite(Chan *c, void *a, long n, vlong offset)
{
    char *p;
    int port;
    ushort *sp;
    ulong *lp;
    Rdwrfn *fn;

    switch((ulong)c->qid.path){

    case Qiob:
        p = a;
        checkport(offset, offset+n);
        for(port = offset; port < offset+n; port++)
            outb(port, *p++);
        return n;

    case Qiow:
        if(n & 1)
            error(Ebadarg);
        checkport(offset, offset+n);
        sp = a;
        for(port = offset; port < offset+n; port += 2)
            outs(port, *sp++);
        return n;

    case Qiol:
        if(n & 3)
            error(Ebadarg);
        checkport(offset, offset+n);
        lp = a;
        for(port = offset; port < offset+n; port += 4)
            outl(port, *lp++);
        return n;

    default:
        if(c->qid.path < narchdir && (fn = writefn[c->qid.path]))
            return fn(c, a, n, offset);
        error(Eperm);
        break;
    }
    return 0;
}

/*s: global archdevtab(x86) */
Dev archdevtab = {
    .dc       =    'P',
    .name     =    "arch",
               
    .reset    =    devreset,
    .init     =    devinit,
    .shutdown =    devshutdown,
    .attach   =    archattach,
    .walk     =    archwalk,
    .stat     =    archstat,
    .open     =    archopen,
    .create   =    devcreate,
    .close    =    archclose,
    .read     =    archread,
    .bread    =    devbread,
    .write    =    archwrite,
    .bwrite   =    devbwrite,
    .remove   =    devremove,
    .wstat    =    devwstat,
};
/*e: global archdevtab(x86) */

void
nop(void)
{
}



static long
cputyperead(Chan*, void *a, long n, vlong offset)
{
    char str[32];
    ulong mhz;

    mhz = (cpu->cpuhz+999999)/1000000;

    snprint(str, sizeof(str), "%s %lud\n", cputype->name, mhz);
    return readstr(offset, a, n, str);
}

static long
archctlread(Chan*, void *a, long nn, vlong offset)
{
    int n;
    char *buf, *p, *ep;

    p = buf = malloc(READSTR);
    if(p == nil)
        error(Enomem);
    ep = p + READSTR;
    p = seprint(p, ep, "cpu %s %lud%s\n",
        cputype->name, (ulong)(cpu->cpuhz+999999)/1000000,
        cpu->havepge ? " pge" : "");
    p = seprint(p, ep, "pge %s\n", getcr4()&0x80 ? "on" : "off");
    p = seprint(p, ep, "coherence ");
    if(arch_coherence == mb386)
        p = seprint(p, ep, "mb386\n");
    else if(arch_coherence == mb586)
        p = seprint(p, ep, "mb586\n");
    else if(arch_coherence == mfence)
        p = seprint(p, ep, "mfence\n");
    else if(arch_coherence == nop)
        p = seprint(p, ep, "nop\n");
    else
        p = seprint(p, ep, "0x%p\n", arch_coherence);
    p = seprint(p, ep, "cmpswap ");
    if(arch_cmpswap == cmpswap386)
        p = seprint(p, ep, "cmpswap386\n");
    else if(arch_cmpswap == cmpswap486)
        p = seprint(p, ep, "cmpswap486\n");
    else
        p = seprint(p, ep, "0x%p\n", arch_cmpswap);
    p = seprint(p, ep, "i8253set %s\n", doi8253set ? "on" : "off");
    n = p - buf;
    //n += mtrrprint(p, ep - p);
    buf[n] = '\0';

    n = readstr(offset, a, nn, buf);
    free(buf);
    return n;
}

enum
{
    CMpge,
    CMcoherence,
    CMi8253set,
    CMcache,
};

static Cmdtab archctlmsg[] =
{
    CMpge,      "pge",      2,
    CMcoherence,    "coherence",    2,
    CMi8253set, "i8253set", 2,
    CMcache,        "cache",        4,
};

static long
archctlwrite(Chan*, void *a, long n, vlong)
{
    uvlong base, size;
    Cmdbuf *cb;
    Cmdtab *ct;
    char *ep;

    cb = parsecmd(a, n);
    if(waserror()){
        free(cb);
        nexterror();
    }
    ct = lookupcmd(cb, archctlmsg, nelem(archctlmsg));
    switch(ct->index){
    case CMpge:
        if(!cpu->havepge)
            error("processor does not support pge");
        if(strcmp(cb->f[1], "on") == 0)
            putcr4(getcr4() | 0x80);
        else if(strcmp(cb->f[1], "off") == 0)
            putcr4(getcr4() & ~0x80);
        else
            cmderror(cb, "invalid pge ctl");
        break;
    case CMcoherence:
        if(strcmp(cb->f[1], "mb386") == 0)
            arch_coherence = mb386;
        else if(strcmp(cb->f[1], "mb586") == 0){
            if(X86FAMILY(cpu->cpuidax) < 5)
                error("invalid coherence ctl on this cpu family");
            arch_coherence = mb586;
        }else if(strcmp(cb->f[1], "mfence") == 0){
            if((cpu->cpuiddx & Sse2) == 0)
                error("invalid coherence ctl on this cpu family");
            arch_coherence = mfence;
        }else if(strcmp(cb->f[1], "nop") == 0){
            /* only safe on vmware */
            if(conf.ncpu > 1)
                error("cannot disable coherence on a multiprocessor");
            arch_coherence = nop;
        }else
            cmderror(cb, "invalid coherence ctl");
        break;
    case CMi8253set:
        if(strcmp(cb->f[1], "on") == 0)
            doi8253set = 1;
        else if(strcmp(cb->f[1], "off") == 0){
            doi8253set = 0;
            (*arch->timerset)(0);
        }else
            cmderror(cb, "invalid i2853set ctl");
        break;
    case CMcache:
        base = strtoull(cb->f[1], &ep, 0);
        if(*ep)
            error("cache: parse error: base not a number?");
        size = strtoull(cb->f[2], &ep, 0);
        if(*ep)
            error("cache: parse error: size not a number?");
        //mtrr(base, size, cb->f[3]);
                error("mtrr: disabled");
        break;
    }
    free(cb);
    poperror();
    return n;
}

/*s: function archinit(x86) */
void
archinit(void)
{
    PCArch **p;

    arch = nil;
    for(p = knownarch; *p; p++){
        if((*p)->ident && (*p)->ident() == 0){
            arch = *p;
            break;
        }
    }
    if(arch == nil)
        arch = &archgeneric;
    else{
        if(arch->id == 0)
            arch->id = archgeneric.id;
        if(arch->reset == 0)
            arch->reset = archgeneric.reset;
        if(arch->serialpower == 0)
            arch->serialpower = archgeneric.serialpower;
        if(arch->modempower == 0)
            arch->modempower = archgeneric.modempower;
        if(arch->intrinit == 0)
            arch->intrinit = archgeneric.intrinit;
        if(arch->intrenable == 0)
            arch->intrenable = archgeneric.intrenable;
    }

    /*
     *  Decide whether to use copy-on-reference (386 and mp).
     *  We get another chance to set it in mpinit() for a
     *  multiprocessor.
     */
    if(X86FAMILY(cpu->cpuidax) == 3)
        conf.copymode = true;

    if(X86FAMILY(cpu->cpuidax) >= 4)
        arch_cmpswap = cmpswap486;

    if(X86FAMILY(cpu->cpuidax) >= 5)
        arch_coherence = mb586;

    if(cpu->cpuiddx & Sse2)
        arch_coherence = mfence;

    addarchfile("cputype", 0444, cputyperead, nil);
    addarchfile("archctl", 0664, archctlread, archctlwrite);
}
/*e: function archinit(x86) */
/*e: devices/sys/386/devarch.c */
