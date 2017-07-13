/*s: core/arm/dat.h */
// pad's memory pointers (used in Lock so must be early)
/*s: pad memory pointer types(arm) */
// physical address
typedef uintptr phys_addr;
// virtual address (a user_addr or kern_addr)
typedef uintptr virt_addr;

// user address (under KZERO)
typedef uintptr user_addr;
// kernel address (mostly physical + KZERO)
typedef uintptr kern_addr;
/*x: pad memory pointer types(arm) */
// wp = word pointer, vp = void pointer
typedef ulong* virt_wp; 
typedef void* virt_vp;
typedef ulong* kern_wp;
typedef void* kern_vp;
typedef ulong* user_wp;
typedef void* user_vp;
/*e: pad memory pointer types(arm) */

#include "dat_forward.h"
#include "../port/portdat_forward.h"
/*s: constant STAGESIZE(arm) */
// was a default in portdat.h
#define STAGESIZE 64 // for struct Uart
/*e: constant STAGESIZE(arm) */

/*s: dat.h includes(arm) */
#include "dat_core.h"                    // Arch_Cpu
#include "../port/portdat_core.h"        // Cpu!!, Conf, Label
#include "../port/portdat_concurrency.h" // Lock
#include "dat_memory.h"                  // Arch_Proc
#include "../port/portdat_memory.h"      // Page, Pagetable, Segment, KImage
#include "../port/portdat_files.h"       // Chan!! Dev!!
#include "dat_time.h"                    // Arch_HZ
#include "../port/portdat_time.h"        // Timer
#include "dat_processes.h"               // Arch_FPsave
#include "../port/portdat_processes.h"   // Proc!!!
#include "../port/portdat_console.h"     // keyboard queue, consdevtab
#include "../port/portdat_devices.h"     // DevConf, DevPort (not that used)
#include "../port/portdat_misc.h"        // Cmd
#include "../port/portdat_buses.h"       // Uart
#include "dat_arch.h"                    // Soc
/*e: dat.h includes(arm) */

// DO NOT switch those declarations. 5c allocates R10
// for the first 'extern register' declaration seen in a file
// and R9 for the second one. See Compiler.nw.
// Then mem.h relies on this to define the UP and CPU macros used
// inside assembly code.
/*s: global cpu(arm) */
extern register Cpu* cpu;           /* R10 */
/*e: global cpu(arm) */
/*s: global up(arm) */
extern register Proc* up;           /* R9 */
/*e: global up(arm) */

// in main.c (used in mmu.c)
extern ulong memsize;

/*s: constant KMESGSIZE(arm) */
#define KMESGSIZE (16*1024) // for /dev/kmesg
/*e: constant KMESGSIZE(arm) */

// TODO: put that in portdat.h or in netif.h?
#pragma varargck  type  "I" uchar*
#pragma varargck  type  "V" uchar*
#pragma varargck  type  "E" uchar*
#pragma varargck  type  "M" uchar*
/*e: core/arm/dat.h */
