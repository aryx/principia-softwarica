/*s: dat.h */

/*s: enum misc_constants(x86) */
enum misc_constants
{
    /*s: constant KMESGSIZE(x86) */
    // used in devcons.c
    KMESGSIZE = (16*1024),  /* put 256*1024 if want acpi debugging */
    /*e: constant KMESGSIZE(x86) */
    /*s: constant STAGESIZE(x86) */
    // used by devuart.c
    STAGESIZE = 2048, // default is 64
    /*e: constant STAGESIZE(x86) */
};
/*e: enum misc_constants(x86) */
/*s: pad memory pointer types(x86) */
// physical address
typedef uintptr phys_addr;
// virtual address (which should be a user address)
typedef uintptr virt_addr;
// kernel address (mostly physical + KZERO)
typedef uintptr kern_addr;
/*x: pad memory pointer types(x86) */
typedef ulong* kern_addr2;
typedef ulong* virt_addr2;
typedef void* virt_addr3;
typedef void* kern_addr3;
//#define nil (void*)0 in lib.h
#define nilptr 0
/*e: pad memory pointer types(x86) */

#include "dat_forward.h"
#include "../port/portdat_forward.h"

// defines Lock (used inline in Cpu in portdat_core.h so must be before)
#include "../port/portdat_concurrency.h"

// defines Conf, Cpu, ... Label
#include "dat_core.h"
#include "../port/portdat_core.h"

// defines Page, Pagetable, Segment, KImage
#include "dat_memory.h"
#include "../port/portdat_memory.h"

// defines Chan
#include "../port/portdat_files.h"

// defines PCArch, a few arch-specific constants
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

// defines DevConf, DevPort (not that used)
#include "../port/portdat_devices.h"

// ref<Cpu>, the actual Cpu is where??
extern Cpu *cpu;

/*s: macro up(x86) */
// up = user process
#define up (cpu->proc)
/*e: macro up(x86) */

/*s: portdat.h pragmas(x86) */
#pragma varargck  type  "I" uchar*
#pragma varargck  type  "V" uchar*
#pragma varargck  type  "E" uchar*
#pragma varargck  type  "M" uchar*
/*e: portdat.h pragmas(x86) */
/*e: dat.h */
