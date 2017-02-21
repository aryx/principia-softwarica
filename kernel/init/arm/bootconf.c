/*s: init/arm/bootconf.c */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"

#include "tos.h"

/*s: enum _anon_ (init/arm/bootconf.c)(arm) */
enum {
    /* space for syscall args, return PC, top-of-stack struct */
    Ustkheadroom	= sizeof(Sargs) + sizeof(uintptr) + sizeof(Tos),
};
/*e: enum _anon_ (init/arm/bootconf.c)(arm) */

/*s: constant BOOTARGS(arm) */
/*
 * Where configuration info is left for the loaded programme.
 */
#define BOOTARGS	((char*)CONFADDR)
/*e: constant BOOTARGS(arm) */
/*s: constant BOOTARGSLEN(arm) */
#define	BOOTARGSLEN	(CPUADDR-CONFADDR)
/*e: constant BOOTARGSLEN(arm) */
//#define MAXCONF		64
//#define MAXCONFLINE	160

/*s: global oargc(arm) */
/*
 * Option arguments from the command line.
 * oargv[0] is the boot file.
 */
static int oargc;
/*e: global oargc(arm) */
/*s: global oargv(arm) */
static char* oargv[20];
/*e: global oargv(arm) */
/*s: global oargb(arm) */
static char oargb[128];
/*e: global oargb(arm) */
/*s: global oargblen(arm) */
static int oargblen;
/*e: global oargblen(arm) */

/* store plan9.ini contents here at least until we stash them in #ec */
//static char confname[MAXCONF][KNAMELEN];
//static char confval[MAXCONF][MAXCONFLINE];
//static int nconf;

typedef struct Atag Atag;
/*s: struct Atag(arm) */
struct Atag {
    u32int	size;	/* size of atag in words, including this header */
    u32int	tag;	/* atag type */
    union {
        u32int	data[1];	/* actually [size-2] */
        /* AtagMem */
        struct {
            u32int	size;
            u32int	base;
        } mem;
        /* AtagCmdLine */
        char	cmdline[1];	/* actually [4*(size-2)] */
    };
};
/*e: struct Atag(arm) */

/*s: enum _anon_ (init/arm/bootconf.c)2(arm) */
enum {
    AtagNone	= 0x00000000,
    AtagCore	= 0x54410001,
    AtagMem		= 0x54410002,
    AtagCmdline	= 0x54410009,
};
/*e: enum _anon_ (init/arm/bootconf.c)2(arm) */

/*s: function findconf(arm) */
static int
findconf(char *name)
{
    int i;

    for(i = 0; i < nconf; i++)
        if(cistrcmp(confname[i], name) == 0)
            return i;
    return -1;
}
/*e: function findconf(arm) */


/*s: function addconf(arm) */
void
addconf(char *name, char *val)
{
    int i;

    i = findconf(name);
    if(i < 0){
        if(val == nil || nconf >= MAXCONF)
            return;
        i = nconf++;
        strecpy(confname[i], confname[i]+sizeof(confname[i]), name);
    }
    strecpy(confval[i], confval[i]+sizeof(confval[i]), val);
}
/*e: function addconf(arm) */

/*s: function writeconf(arm) */
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
    memmove(BOOTARGS, p, n);
    memset(BOOTARGS + n, '\n', BOOTARGSLEN - n);
    poperror();
    free(p);
}
/*e: function writeconf(arm) */

/*s: function plan9iniinit(arm) */
static void
plan9iniinit(char *s, int cmdline)
{
    char *toks[MAXCONF];
    int i, c, n;
    char *v;

    if((c = *s) < ' ' || c >= 0x80)
        return;
    if(cmdline)
        n = tokenize(s, toks, MAXCONF);
    else
        n = getfields(s, toks, MAXCONF, 1, "\n");
    for(i = 0; i < n; i++){
        if(toks[i][0] == '#')
            continue;
        v = strchr(toks[i], '=');
        if(v == nil)
            continue;
        *v++ = '\0';
        addconf(toks[i], v);
    }
}
/*e: function plan9iniinit(arm) */

/*s: function ataginit(arm) */
static void
ataginit(Atag *a)
{
    int n;

    if(a->tag != AtagCore){
        plan9iniinit((char*)a, 0);
        return;
    }
    while(a->tag != AtagNone){
        switch(a->tag){
        case AtagMem:
            /* use only first bank */
            if(conf.mem[0].limit == 0 && a->mem.size != 0){
                memsize = a->mem.size;
                conf.mem[0].base = a->mem.base;
                conf.mem[0].limit = a->mem.base + memsize;
            }
            break;
        case AtagCmdline:
            n = (a->size * sizeof(u32int)) - offsetof(Atag, cmdline[0]);
            if(a->cmdline + n < BOOTARGS + BOOTARGSLEN)
                a->cmdline[n] = 0;
            else
                BOOTARGS[BOOTARGSLEN-1] = 0;
            plan9iniinit(a->cmdline, 1);
            break;
        }
        a = (Atag*)((u32int*)a + a->size);
    }
}
/*e: function ataginit(arm) */

/*s: function optionsinit(arm) */
static void
optionsinit(char* s)
{
    strecpy(oargb, oargb+sizeof(oargb), s);

    oargblen = strlen(oargb);
    oargc = tokenize(oargb, oargv, nelem(oargv)-1);
    oargv[oargc] = nil;
}
/*e: function optionsinit(arm) */

/*s: function bootargs(arm) */
uintptr
bootargs(uintptr base)
{
    int i;
    ulong ssize;
    char **av, *p;

    /*
     * Push the boot args onto the stack.
     * The initial value of the user stack must be such
     * that the total used is larger than the maximum size
     * of the argument list checked in syscall.
     */
    i = oargblen+1;
    p = UINT2PTR(STACKALIGN(base + BY2PG - Ustkheadroom - i));
    memmove(p, oargb, i);

    /*
     * Now push the argv pointers.
     * The code jumped to by touser in lproc.s expects arguments
     *	main(char* argv0, ...)
     * and calls
     * 	startboot("/boot/boot", &argv0)
     * not the usual (int argc, char* argv[])
     */
    av = (char**)(p - (oargc+1)*sizeof(char*));
    ssize = base + BY2PG - PTR2UINT(av);
    for(i = 0; i < oargc; i++)
        *av++ = (oargv[i] - oargb) + (p - base) + (USTKTOP - BY2PG);
    *av = nil;
    //sp = USTKTOP - ssize;
    return USTKTOP - ssize;
}
/*e: function bootargs(arm) */
/*e: init/arm/bootconf.c */
