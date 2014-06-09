/*s: dat.h */

/*s: enum misc_constants */
enum misc_constants
{
    /*s: constant MAXSYSARG */
    // used in Proc
    MAXSYSARG = 5, /* for mount(fd, afd, mpt, flag, arg) */
    /*e: constant MAXSYSARG */

    /*s: constant PRINTSIZE */
    PRINTSIZE = 256,
    /*e: constant PRINTSIZE */
    /*s: constant KMESGSIZE */
    // used in devcons.c
    KMESGSIZE = (16*1024),  /* put 256*1024 if want acpi debugging */
    /*e: constant KMESGSIZE */
    /*s: constant STAGESIZE */
    // used by devuart.c
    STAGESIZE = 2048, // default is 64
    /*e: constant STAGESIZE */

    NUMSIZE = 12,   /* size of formatted number */
    /* READSTR was 1000, which is way too small for usb's ctl file */
    READSTR = 4000,   /* temporary buffer size for device reads */

    KB      = 1024,
    MB =    (1024*1024),

    /* cpuid instruction result register bits */
    // this is actually only used in 386/ code. 
    /* dx */
    Fpuonchip = 1<<0,
    Vmex  = 1<<1,   /* virtual-mode extensions */
    Pse = 1<<3,   /* page size extensions */
    Tsc = 1<<4,   /* time-stamp counter */
    Cpumsr  = 1<<5,   /* model-specific registers, rdmsr/wrmsr */
    Mce = 1<<7,   /* machine-check exception */
    Mtrr  = 1<<12,  /* memory-type range regs.  */
    Pge = 1<<13,  /* page global extension */
    Fxsr  = 1<<24,  /* have SSE FXSAVE/FXRSTOR */
    Sse2  = 1<<26,  /* thus mfence & lfence instr.s */
};
/*e: enum misc_constants */
/*s: pad memory pointer types */
// physical address
typedef uintptr phys_addr;
// virtual address (which should be a user address)
typedef uintptr virt_addr;
// kernel address (mostly physical + KZERO)
typedef uintptr kern_addr;
/*x: pad memory pointer types */
typedef ulong* kern_addr2;
typedef ulong* virt_addr2;
typedef void* virt_addr3;
typedef void* kern_addr3;
//#define nil (void*)0 in lib.h
#define nilptr 0
/*e: pad memory pointer types */

#include "dat_forward.h"
#include "../port/portdat_forward.h"

// defines Lock (used inline in Cpu in portdat_core.h so must be before)
#include "../port/portdat_concurrency.h"

// defines Conf, Cpu
#include "dat_core.h"
#include "../port/portdat_core.h"

// defines Page, Pagetable, Segment, KImage
#include "dat_memory.h"
#include "../port/portdat_memory.h"

// defines Chan
#include "../port/portdat_files.h"

// defines PCArch
#include "dat_arch.h"

// defines Proc
#include "dat_processes.h"
#include "../port/portdat_processes.h"

// defines Cmd
#include "../port/portdat_misc.h"

// defines Uart
#include "dat_buses.h"
#include "../port/portdat_buses.h"

// defines keyboard queue, consdevtab
#include "../port/portdat_console.h"

// could be put in lib.h
/*s: portdat.h macros */
#define MIN(a, b) ((a) < (b)? (a): (b))
#define HOWMANY(x, y) (((x)+((y)-1))/(y))
#define ROUNDUP(x, y) (HOWMANY((x), (y))*(y)) /* ceiling */
#define ROUND(s, sz)  (((s)+(sz-1))&~(sz-1))
// ex: ROUND(??, ??) = ??

// BY2PG is defined in mem.h, which should always be included before "dat.h"!
#define PGROUND(s)  ROUNDUP(s, BY2PG)
/*e: portdat.h macros */

/*s: portdat.h pragmas */
#pragma varargck  type  "I" uchar*
#pragma varargck  type  "V" uchar*
#pragma varargck  type  "E" uchar*
#pragma varargck  type  "M" uchar*
/*e: portdat.h pragmas */
/*e: dat.h */
