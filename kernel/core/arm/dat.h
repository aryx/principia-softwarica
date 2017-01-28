// was a default in portdat.h
#define STAGESIZE 64 // for devuart
#define KMESGSIZE (16*1024) // for /dev/kmesg

// pad memory pointers (used in Lock so must be early)
// physical address
typedef uintptr phys_addr;
// virtual address (which should be a user address)
typedef uintptr virt_addr;
// kernel address (mostly physical + KZERO)
typedef uintptr kern_addr;

typedef ulong* kern_addr2;
typedef ulong* virt_addr2;
typedef void* virt_addr3;
typedef void* kern_addr3;


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
// defines Soc
#include "dat_arch.h"
// defines Timer
#include "dat_time.h"
#include "../port/portdat_time.h"
// defines Proc
#include "dat_processes.h"
#include "../port/portdat_processes.h"
// defines Cmd
#include "../port/portdat_misc.h"
// defines Uart
#include "../port/portdat_buses.h"
// defines keyboard queue, consdevtab
#include "../port/portdat_console.h"
// defines DevConf, DevPort (not that used)
#include "../port/portdat_devices.h"

extern register Cpu* cpu;			/* R10 */
extern register Proc* up;			/* R9 */

// in main.c (used in mmu.c)
extern ulong memsize;

extern int normalprint;

/*
 * Horrid. But the alternative is 'defined'.
 */
#ifdef _DBGC_
#define DBGFLG		(dbgflg[_DBGC_])
#else
#define DBGFLG		(0)
#endif /* _DBGC_ */
int vflag;
extern char dbgflg[256];
#define dbgprint	print		/* for now */

// TODO: put that in portdat.h or in netif.h?
#pragma varargck  type  "I" uchar*
#pragma varargck  type  "V" uchar*
#pragma varargck  type  "E" uchar*
#pragma varargck  type  "M" uchar*
