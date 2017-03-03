/*s: core/arm/dat.h */
/*s: constant STAGESIZE(arm) */
// was a default in portdat.h
#define STAGESIZE 64 // for devuart
/*e: constant STAGESIZE(arm) */
/*s: constant KMESGSIZE(arm) */
#define KMESGSIZE (16*1024) // for /dev/kmesg
/*e: constant KMESGSIZE(arm) */

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

// DO NOT switch those declarations. 5c allocates R10
// for the first 'extern register' declaration seen in a file
// and R9 for the second one. See Compiler.nw.
// Then mem.h relies on this to define the UP and CPU macros used
// inside assembly code.
extern register Cpu* cpu;			/* R10 */
extern register Proc* up;			/* R9 */

// in main.c (used in mmu.c)
extern ulong memsize;

// TODO: put that in portdat.h or in netif.h?
#pragma varargck  type  "I" uchar*
#pragma varargck  type  "V" uchar*
#pragma varargck  type  "E" uchar*
#pragma varargck  type  "M" uchar*
/*e: core/arm/dat.h */
