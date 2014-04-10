#include "dat_forward.h"

#include "dat_core.h"
#include "dat_concurrency.h"
#include "dat_memory.h"
#include "dat_arch.h"
#include "dat_processes.h"
//empty: #include "dat_globals.h"
#include "dat_misc.h"
#include "dat_buses.h"
#include "dat_devices.h"

// used in portdat.h
#define KMESGSIZE (256*1024)  /* lots, for acpi debugging */
#define STAGESIZE 2048
#define MAXSYSARG 5 /* for mount(fd, afd, mpt, flag, arg) */
#include "../port/portdat.h"

/*
 *  parameters for sysproc.c
 */
// I_MAGIC is defined in include/a.out.h
#define AOUT_MAGIC  (I_MAGIC)

/* cpuid instruction result register bits */
// this is actually only used in 386/ code. 
enum {
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
//  Pae = 1<<6,   /* physical-addr extensions */
//  Cmpxchg8b = 1<<8,
//  Cpuapic = 1<<9,
//  Pse2  = 1<<17,  /* more page size extensions */
//  Clflush = 1<<19,
//  Mmx = 1<<23,
//  Sse = 1<<25,  /* thus sfence instr. */
};
