/*s: dat.h */
#include "dat_forward.h"
#include "../port/portdat_forward.h"

/*s: enum miscsize_dat */
enum miscsize_dat {
  // used in devcons.c
  KMESGSIZE = (256*1024),  /* lots, for acpi debugging */ // default is 16*1024
  // used by devuart.c
  STAGESIZE = 2048, // default is 64
  // used in Proc
  MAXSYSARG = 5, /* for mount(fd, afd, mpt, flag, arg) */
};
/*e: enum miscsize_dat */

// defines Lock (used inline in Mach in portdat_core.h so must be before)
#include "../port/portdat_concurrency.h"

// defines Conf, Mach
#include "dat_core.h"
#include "../port/portdat_core.h"

// defines Page
#include "dat_memory.h"
#include "../port/portdat_memory.h"

// defines Chan
#include "../port/portdat_files.h"

#include "dat_arch.h"

// defines Proc
#include "dat_processes.h"
#include "../port/portdat_processes.h"

// defines Cmd (use ??)
#include "../port/portdat_misc.h"

// defines Uart
#include "dat_buses.h"
#include "../port/portdat_buses.h"

#include "../port/portdat_console.h"

#include "../port/portdat.h"

/*s: constant AOUT_MAGIC */
/*
 *  parameters for sysproc.c
 */
// I_MAGIC is defined in include/a.out.h
#define AOUT_MAGIC  (I_MAGIC)
/*e: constant AOUT_MAGIC */

/*s: enum misc_dat */
/* cpuid instruction result register bits */
// this is actually only used in 386/ code. 
enum misc_dat {
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
/*e: enum misc_dat */
/*e: dat.h */
