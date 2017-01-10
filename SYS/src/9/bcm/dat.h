
// misc constants
#define MAXSYSARG	5	/* for mount(fd, mpt, flag, arg, srv) */

/*
 * Time.
 *
 * HZ should divide 1000 evenly, ideally.
 * 100, 125, 200, 250 and 333 are okay.
 */
#define	HZ		100			/* clock frequency */
#define	MS2HZ		(1000/HZ)		/* millisec per clock tick */
#define	TK2SEC(t)	((t)/HZ)		/* ticks to seconds */
enum {
	Mhz	= 1000 * 1000,
};

enum {
	Maxfpregs	= 32,	/* could be 16 or 32, see Mach.fpnregs */
	Nfpctlregs	= 16,
};

// was a default in portdat.h
#define STAGESIZE 64


// pad memory pointers
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

//#define nil (void*)0 in lib.h
#define nilptr 0



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

// defines ??
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

/*
 *  things saved in the Proc structure during a notify
 */
struct Notsave {
	int	emptiness;
};

// TODO add register! but 5c-ocaml does not like it
extern register Cpu* cpu;			/* R10 */
extern register Proc* up;			/* R9 */

extern uintptr kseg0;

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
