/*s: x86.c */
/*s: kernel basic includes */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "../port/error.h"
/*e: kernel basic includes */
#include "io.h"

//#define X86STEPPING(x)  ((x) & 0x0F)
/* incorporates extended-model and -family bits */
#define X86MODEL(x) ((((x)>>4) & 0x0F) | (((x)>>16) & 0x0F)<<4)

/*s: global cputype */
X86type *cputype;
/*e: global cputype */


//@Scheck: Assembly l.s
void    _cycles(uvlong*);   

enum {
    CR4Osfxsr = 1 << 9,
};

enum {              /* cpuid standard function codes */
    Highstdfunc = 0,    /* also returns vendor string */
    Procsig,
    Proctlbcache,
    Procserial,
};



/* cpuid ax is 0x0ffMTFmS, where 0xffF is family, 0xMm is model */
static X86type x86intel[] =
{
    { 4,    0,  22, "486DX", }, /* known chips */
    { 4,    1,  22, "486DX50", },
    { 4,    2,  22, "486SX", },
    { 4,    3,  22, "486DX2", },
    { 4,    4,  22, "486SL", },
    { 4,    5,  22, "486SX2", },
    { 4,    7,  22, "DX2WB", }, /* P24D */
    { 4,    8,  22, "DX4", },   /* P24C */
    { 4,    9,  22, "DX4WB", }, /* P24CT */
    { 5,    0,  23, "P5", },
    { 5,    1,  23, "P5", },
    { 5,    2,  23, "P54C", },
    { 5,    3,  23, "P24T", },
    { 5,    4,  23, "P55C MMX", },
    { 5,    7,  23, "P54C VRT", },
    { 6,    1,  16, "PentiumPro", },/* trial and error */
    { 6,    3,  16, "PentiumII", },
    { 6,    5,  16, "PentiumII/Xeon", },
    { 6,    6,  16, "Celeron", },
    { 6,    7,  16, "PentiumIII/Xeon", },
    { 6,    8,  16, "PentiumIII/Xeon", },
    { 6,    0xB,    16, "PentiumIII/Xeon", },
    { 6,    0xF,    16, "Core 2/Xeon", },
    { 6,    0x16,   16, "Celeron", },
    { 6,    0x17,   16, "Core 2/Xeon", },
    { 6,    0x1A,   16, "Core i7/Xeon", },
    { 6,    0x1C,   16, "Atom", },
    { 6,    0x1D,   16, "Xeon MP", },
    { 6,    0x1E,   16, "Core i5/i7/Xeon", },
    { 6,    0x1F,   16, "Core i7/Xeon", },
    { 6,    0x22,   16, "Core i7", },
    { 6,    0x25,   16, "Core i3/i5/i7", },
    { 6,    0x2A,   16, "Core i7", },
    { 6,    0x2C,   16, "Core i7/Xeon", },
    { 6,    0x2D,   16, "Core i7", },
    { 6,    0x2E,   16, "Xeon MP", },
    { 6,    0x2F,   16, "Xeon MP", },
    { 6,    0x3A,   16, "Core i7", },
    { 0xF,  1,  16, "P4", },    /* P4 */
    { 0xF,  2,  16, "PentiumIV/Xeon", },
    { 0xF,  6,  16, "PentiumIV/Xeon", },

    { 3,    -1, 32, "386", },   /* family defaults */
    { 4,    -1, 22, "486", },
    { 5,    -1, 23, "P5", },
    { 6,    -1, 16, "P6", },
    { 0xF,  -1, 16, "P4", },    /* P4 */

    { -1,   -1, 16, "unknown", },   /* total default */
};

/*
 * The AMD processors all implement the CPUID instruction.
 * The later ones also return the processor name via functions
 * 0x80000002, 0x80000003 and 0x80000004 in registers AX, BX, CX
 * and DX:
 *  K5  "AMD-K5(tm) Processor"
 *  K6  "AMD-K6tm w/ multimedia extensions"
 *  K6 3D   "AMD-K6(tm) 3D processor"
 *  K6 3D+  ?
 */
static X86type x86amd[] =
{
    { 5,    0,  23, "AMD-K5", },    /* guesswork */
    { 5,    1,  23, "AMD-K5", },    /* guesswork */
    { 5,    2,  23, "AMD-K5", },    /* guesswork */
    { 5,    3,  23, "AMD-K5", },    /* guesswork */
    { 5,    4,  23, "AMD Geode GX1", }, /* guesswork */
    { 5,    5,  23, "AMD Geode GX2", }, /* guesswork */
    { 5,    6,  11, "AMD-K6", },    /* trial and error */
    { 5,    7,  11, "AMD-K6", },    /* trial and error */
    { 5,    8,  11, "AMD-K6-2", },  /* trial and error */
    { 5,    9,  11, "AMD-K6-III", },/* trial and error */
    { 5,    0xa,    23, "AMD Geode LX", },  /* guesswork */

    { 6,    1,  11, "AMD-Athlon", },/* trial and error */
    { 6,    2,  11, "AMD-Athlon", },/* trial and error */

    { 0x1F, 9,  11, "AMD-K10 Opteron G34", },/* guesswork */

    { 4,    -1, 22, "Am486", }, /* guesswork */
    { 5,    -1, 23, "AMD-K5/K6", }, /* guesswork */
    { 6,    -1, 11, "AMD-Athlon", },/* guesswork */
    { 0xF,  -1, 11, "AMD-K8", },    /* guesswork */
    { 0x1F, -1, 11, "AMD-K10", },   /* guesswork */

    { -1,   -1, 11, "unknown", },   /* total default */
};

/*
 * WinChip 240MHz
 */
static X86type x86winchip[] =
{
    {5, 4,  23, "Winchip",},    /* guesswork */
    {6, 7,  23, "Via C3 Samuel 2 or Ezra",},
    {6, 8,  23, "Via C3 Ezra-T",},
    {6, 9,  23, "Via C3 Eden-N",},
    { -1,   -1, 23, "unknown", },   /* total default */
};

/*
 * SiS 55x
 */
static X86type x86sis[] =
{
    {5, 0,  23, "SiS 55x",},    /* guesswork */
    { -1,   -1, 23, "unknown", },   /* total default */
};


/*
 *  figure out:
 *  - cpu type
 *  - whether or not we have a TSC (cycle counter)
 *  - whether or not it supports page size extensions
 *      (if so turn it on)
 *  - whether or not it supports machine check exceptions
 *      (if so turn it on)
 *  - whether or not it supports the page global flag
 *      (if so turn it on)
 */
int
cpuidentify(void)
{
    char *p;
    int family, model, nomce;
    X86type *t, *tab;
    ulong cr4;
    ulong regs[4];
    vlong mca, mct;

    cpuid(Highstdfunc, regs);
    memmove(m->cpuidid,   &regs[1], BY2WD); /* bx */
    memmove(m->cpuidid+4, &regs[3], BY2WD); /* dx */
    memmove(m->cpuidid+8, &regs[2], BY2WD); /* cx */
    m->cpuidid[12] = '\0';

    cpuid(Procsig, regs);
    m->cpuidax = regs[0];
    m->cpuiddx = regs[3];

    if(strncmp(m->cpuidid, "AuthenticAMD", 12) == 0 ||
       strncmp(m->cpuidid, "Geode by NSC", 12) == 0)
        tab = x86amd;
    else if(strncmp(m->cpuidid, "CentaurHauls", 12) == 0)
        tab = x86winchip;
    else if(strncmp(m->cpuidid, "SiS SiS SiS ", 12) == 0)
        tab = x86sis;
    else
        tab = x86intel;

    family = X86FAMILY(m->cpuidax);
    model = X86MODEL(m->cpuidax);
    for(t=tab; t->name; t++)
        if((t->family == family && t->model == model)
        || (t->family == family && t->model == -1)
        || (t->family == -1))
            break;

    m->cpuidtype = t->name;

    /*
     *  if there is one, set tsc to a known value
     */
    if(m->cpuiddx & Tsc){
        m->havetsc = 1;
        cycles = _cycles;
        if(m->cpuiddx & Cpumsr)
            wrmsr(0x10, 0);
    }

    /*
     *  use i8253 to guess our cpu speed
     */
    guesscpuhz(t->aalcycles);

    /*
     * If machine check exception, page size extensions or page global bit
     * are supported enable them in CR4 and clear any other set extensions.
     * If machine check was enabled clear out any lingering status.
     */
    if(m->cpuiddx & (Pge|Mce|Pse)){
        cr4 = 0;
        if(m->cpuiddx & Pse)
            cr4 |= 0x10;        /* page size extensions */
        if(p = getconf("*nomce"))
            nomce = strtoul(p, 0, 0);
        else
            nomce = 0;
        if((m->cpuiddx & Mce) && !nomce){
            cr4 |= 0x40;        /* machine check enable */
            if(family == 5){
                rdmsr(0x00, &mca);
                rdmsr(0x01, &mct);
            }
        }

        /*
         * Detect whether the chip supports the global bit
         * in page directory and page table entries.  When set
         * in a particular entry, it means ``don't bother removing
         * this from the TLB when CR3 changes.''
         *
         * We flag all kernel pages with this bit.  Doing so lessens the
         * overhead of switching processes on bare hardware,
         * even more so on VMware.  See mmu.c:/^memglobal.
         *
         * For future reference, should we ever need to do a
         * full TLB flush, it can be accomplished by clearing
         * the PGE bit in CR4, writing to CR3, and then
         * restoring the PGE bit.
         */
        if(m->cpuiddx & Pge){
            cr4 |= 0x80;        /* page global enable bit */
            m->havepge = 1;
        }

        putcr4(cr4);
        if(m->cpuiddx & Mce)
            rdmsr(0x01, &mct);
    }

    if(m->cpuiddx & Fxsr){          /* have sse fp? */
        fpsave = fpssesave;
        fprestore = fpsserestore;
        putcr4(getcr4() | CR4Osfxsr);
    } else {
        fpsave = fpx87save;
        fprestore = fpx87restore;
    }

    cputype = t;
    return t->family;
}

/*e: x86.c */
