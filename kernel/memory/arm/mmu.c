/*s: memory/arm/mmu.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */
#include "arm.h"

/*s: macro L1X(arm) */
#define L1X(va)     FEXT((va), 20, 12)
/*e: macro L1X(arm) */
/*s: macro L2X(arm) */
#define L2X(va)     FEXT((va), 12, 8)
/*e: macro L2X(arm) */
/*s: macro L2AP(arm) */
#define L2AP(ap)    l2ap(ap)
/*e: macro L2AP(arm) */

/*s: constant L1ptedramattrs(arm) */
#define L1ptedramattrs  soc.l1ptedramattrs
/*e: constant L1ptedramattrs(arm) */
/*s: constant L2ptedramattrs(arm) */
#define L2ptedramattrs  soc.l2ptedramattrs
/*e: constant L2ptedramattrs(arm) */

/*s: enum _anon_ (memory/arm/mmu.c)(arm) */
enum {
/*s: constant L1lo(arm) */
L1lo        = UZERO/MiB,        /* L1X(UZERO)? */
/*e: constant L1lo(arm) */
/*s: constant L1hi(arm) */
L1hi        = (USTKTOP+MiB-1)/MiB,  /* L1X(USTKTOP+MiB-1)? */
/*e: constant L1hi(arm) */
};
/*e: enum _anon_ (memory/arm/mmu.c)(arm) */

/*s: function mmuinit(arm) */
/*
 * Set up initial PTEs for this cpu (called with mmu off)
 */
void
mmuinit(void *a)
{
    PTE *l1, *l2;
    phys_addr pa;
    kern_addr va;

    l1 = (PTE*)a; // PADDR(L1) for cpu0

    /*
     * map all of ram at KZERO
     */
    va = KZERO;
    for(pa = 0; pa < soc.dramsize; pa += MiB, va += MiB)
    {
        l1[L1X(va)] = pa|Dom0|L1AP(Krw)|Section|L1ptedramattrs;
    }

    /*
     * identity map first MB of ram so mmu can be enabled
     */
    // ???
    l1[L1X(0)] = 0|Dom0|L1AP(Krw)|Section|L1ptedramattrs;

    /*
     * map i/o registers 
     */
    va = VIRTIO;
    for(pa = soc.physio; pa < soc.physio + IOSIZE; pa += MiB, va += MiB){
        // No L1ptedramattrs (Cached | Buffered) here, because volatile!
        l1[L1X(va)] = pa|Dom0|L1AP(Krw)|Section; 
    }
    // for raspi2
    pa = soc.armlocal;
    if(pa)
        l1[L1X(va)] = pa|Dom0|L1AP(Krw)|Section;
    
    /*
     * double map exception vectors at top of virtual memory
     */
    // ???
    l2 = (PTE*)PADDR(L2);
    va = HVECTORS;
    l1[L1X(va)] = (uintptr)l2|Dom0|Coarse;
    l2[L2X(va)] = L2AP(Krw)|Small|L2ptedramattrs;
}
/*e: function mmuinit(arm) */

/*s: function mmuinit1(arm) */
void
mmuinit1(void *a)
{
    PTE *l1;

    l1 = (PTE*)a;
    cpu->mmul1 = l1;

    /*
     * undo identity map of first MB of ram
     */
    l1[L1X(0)] = 0;
    /*s: [[mmuinit1()]] write back cache after adjusting page tables(arm) */
    cachedwbse(&l1[L1X(0)], sizeof(PTE));
    /*e: [[mmuinit1()]] write back cache after adjusting page tables(arm) */
    mmuinvalidateaddr(0);
}
/*e: function mmuinit1(arm) */

/*s: function mmul2empty(arm) */
static void
mmul2empty(Proc* proc, bool clear)
{
    PTE *l1;
    Page **l2, *page;

    l1 = cpu->mmul1;
    l2 = &proc->mmul2;
    for(page = *l2; page != nil; page = page->next){
        if(clear)
            memset(UINT2PTR(page->va), 0, BY2PG);
        l1[page->daddr] = Fault;
        l2 = &page->next;
    }
    /*s: [[mmul2empty()]] remember free pages(arm) */
    *l2 = proc->mmul2cache;
    proc->mmul2cache = proc->mmul2;
    proc->mmul2 = nil;
    /*e: [[mmul2empty()]] remember free pages(arm) */
}
/*e: function mmul2empty(arm) */

/*s: function mmul1empty(arm) */
static void
mmul1empty(void)
{
    memset(&cpu->mmul1[L1lo], 0, (L1hi - L1lo)*sizeof(PTE));
}
/*e: function mmul1empty(arm) */

/*s: function arch_mmuswitch(arm) */
void
arch_mmuswitch(Proc* proc)
{
    int x;
    PTE *l1;
    Page *page;

    /* do kprocs get here and if so, do they need to? */
/*** "This is plausible, but wrong" - Charles Forsyth 1 Mar 2015
    if(cpu->mmupid == proc->pid && !proc->newtlb)
        return;
***/
    cpu->mmupid = proc->pid;

    /*s: [[arch_mmuswitch()]] write back and invalidate cache before switch(arm) */
    /* write back dirty and invalidate l1 caches */
    cacheuwbinv();
    /*e: [[arch_mmuswitch()]] write back and invalidate cache before switch(arm) */

    if(proc->newtlb){
        mmul2empty(proc, 1);
        proc->newtlb = false;
    }

    mmul1empty();

    /* move in new map */
    l1 = cpu->mmul1;
    for(page = proc->mmul2; page != nil; page = page->next){
        x = page->daddr;
        l1[x] = PPN(page->pa)|Dom0|Coarse;
        /* know here that L1lo < x < L1hi */
        if(x+1 - cpu->mmul1lo < cpu->mmul1hi - x)
            cpu->mmul1lo = x+1;
        else
            cpu->mmul1hi = x;
    }

    /*s: [[arch_mmuswitch()]] write back cache after adjusting page tables(arm) */
    /* make sure map is in memory */
    /* could be smarter about how much? */
    cachedwbse(&l1[L1X(UZERO)], (L1hi - L1lo)*sizeof(PTE));
    /*e: [[arch_mmuswitch()]] write back cache after adjusting page tables(arm) */

    /* lose any possible stale tlb entries */
    mmuinvalidate();
}
/*e: function arch_mmuswitch(arm) */

/*s: function arch_flushmmu(arm) */
void
arch_flushmmu(void)
{
    int s;

    s = arch_splhi();
    up->newtlb = true;
    arch_mmuswitch(up);
    arch_splx(s);
}
/*e: function arch_flushmmu(arm) */

/*s: function arch_mmurelease(arm) */
void
arch_mmurelease(Proc* proc)
{
    Page *page, *next;

    /*s: [[arch_mmurelease()]] invalidate cache before adjusting page tables(arm) */
    /* write back dirty and invalidate l1 caches */
    cacheuwbinv();
    /*e: [[arch_mmurelease()]] invalidate cache before adjusting page tables(arm) */
    mmul2empty(proc, false);
    /*s: [[arch_mmurelease()]] free l2 page cache(arm) */
    for(page = proc->mmul2cache; page != nil; page = next){
        next = page->next;
        if(--page->ref)
            panic("mmurelease: page->ref %d", page->ref);
        pagechainhead(page);
    }
    if(proc->mmul2cache && palloc.freememr.p)
        wakeup(&palloc.freememr);
    proc->mmul2cache = nil;
    /*e: [[arch_mmurelease()]] free l2 page cache(arm) */
    mmul1empty();
    /*s: [[arch_mmurelease()]] write back cache after adjusting page tables(arm) */
    /* make sure map is in memory */
    /* could be smarter about how much? */
    cachedwbse(&cpu->mmul1[L1X(UZERO)], (L1hi - L1lo)*sizeof(PTE));
    /*e: [[arch_mmurelease()]] write back cache after adjusting page tables(arm) */
    /* lose any possible stale tlb entries */
    mmuinvalidate();
}
/*e: function arch_mmurelease(arm) */

/*s: function arch_putmmu(arm) */
void
arch_putmmu(virt_addr va, uintptr pa, Page* page)
{
    int x;
    Page *pg;
    PTE *l1, *pte;

    x = L1X(va);
    l1 = &cpu->mmul1[x];
    if(*l1 == Fault){
        /* wasteful - l2 pages only have 256 entries - fix */
        /*s: [[arch_putmmu()]] if mmul2cache not empty(arm) */
        if(up->mmul2cache != nil){
            pg = up->mmul2cache;
            up->mmul2cache = pg->next;
            memset(UINT2PTR(pg->va), 0, BY2PG);
        }
        /*e: [[arch_putmmu()]] if mmul2cache not empty(arm) */
        else{
            /* auxpg since we don't need much? memset if so */
            pg = newpage(true, nil, 0);
            pg->va = VA(arch_kmap(pg));
        }
        pg->daddr = x;

        //add_list(pg, up->mmul2)
        pg->next = up->mmul2;
        up->mmul2 = pg;

        /*s: [[arch_putmmu()]] write back cache before fixing va fault(arm) */
        /* force l2 page to memory */
        cachedwbse((void *)pg->va, BY2PG);
        /*e: [[arch_putmmu()]] write back cache before fixing va fault(arm) */

        *l1 = PPN(pg->pa)|Dom0|Coarse;
        /*s: [[arch_putmmu()]] write back cache after changing pde(arm) */
        cachedwbse(l1, sizeof *l1);
        /*e: [[arch_putmmu()]] write back cache after changing pde(arm) */

        /*s: [[arch_putmmu()]] maintain mmul1lo and mmul1hi(arm) */
        if(x >= cpu->mmul1lo && x < cpu->mmul1hi){
            if(x+1 - cpu->mmul1lo < cpu->mmul1hi - x)
                cpu->mmul1lo = x+1;
            else
                cpu->mmul1hi = x;
        }
        /*e: [[arch_putmmu()]] maintain mmul1lo and mmul1hi(arm) */
    }
    pte = UINT2PTR(KADDR(PPN(*l1)));

    /* protection bits are
     *  PTERONLY|PTEVALID;
     *  PTEWRITE|PTEVALID;
     *  PTEWRITE|PTEUNCACHED|PTEVALID;
     */
    x = Small;
    /*s: [[arch_putmmu()]] if PTEUNCACHED, do not change x(arm) */
    if(pa & PTEUNCACHED) {
    }
    /*e: [[arch_putmmu()]] if PTEUNCACHED, do not change x(arm) */
    else
        x |= L2ptedramattrs;

    if(pa & PTEWRITE)
        x |= L2AP(Urw);
    else
        x |= L2AP(Uro);

    pte[L2X(va)] = PPN(pa)|x;
    /*s: [[arch_putmmu()]] write back cache after changing pte(arm) */
    cachedwbse(&pte[L2X(va)], sizeof pte[0]);
    /*e: [[arch_putmmu()]] write back cache after changing pte(arm) */

    /* clear out the current entry */
    mmuinvalidateaddr(PPN(va));

    /*  write back dirty entries - we need this because the pio() in
     *  fault.c is writing via a different virt addr and won't clean
     *  its changes out of the dcache.  Page coloring doesn't work
     *  on this mmu because the virtual cache is set associative
     *  rather than direct mapped.
     */
    /*s: [[arch_putmmu()]] if PG_TXTFLUSH, invalidate cache(arm) */
    if(page->cachectl[cpu->cpuno] == PG_TXTFLUSH){
        /* pio() sets PG_TXTFLUSH whenever a text pg has been written */
        cacheiinv();
        page->cachectl[cpu->cpuno] = PG_NOFLUSH;
    }
    /*e: [[arch_putmmu()]] if PG_TXTFLUSH, invalidate cache(arm) */
    arch_checkmmu(va, PPN(pa));
}
/*e: function arch_putmmu(arm) */

/*s: function arch_cankaddr(arm) */
/*
 * Return the number of bytes that can be accessed via KADDR(pa).
 * If pa is not a valid argument to KADDR, return 0.
 */
uintptr
arch_cankaddr(uintptr pa)
{
    if(pa < memsize)        /* assumes PHYSDRAM is 0 */
        return memsize - pa;
    return 0;
}
/*e: function arch_cankaddr(arm) */

/*s: function mmukmap(arm) */
uintptr
mmukmap(uintptr va, uintptr pa, usize size)
{
    int o;
    usize n;
    PTE *pte, *pte0;

    assert((va & (MiB-1)) == 0);
    o = pa & (MiB-1);
    pa -= o;
    size += o;
    pte = pte0 = &cpu->mmul1[L1X(va)];
    for(n = 0; n < size; n += MiB)
        if(*pte++ != Fault)
            return 0;
    pte = pte0;
    for(n = 0; n < size; n += MiB){
        *pte++ = (pa+n)|Dom0|L1AP(Krw)|Section;
        mmuinvalidateaddr(va+n);
    }
    /*s: [[mmukmap()]] write back cache after changing page tables(arm) */
    cachedwbse(pte0, (uintptr)pte - (uintptr)pte0);
    /*e: [[mmukmap()]] write back cache after changing page tables(arm) */
    return va + o;
}
/*e: function mmukmap(arm) */


/*s: function arch_checkmmu(arm) */
void
arch_checkmmu(uintptr va, uintptr pa)
{
    USED(va);
    USED(pa);
}
/*e: function arch_checkmmu(arm) */

/*s: function arch_kmap(arm) */
Arch_KMap*
arch_kmap(Page *p) {
  return (Arch_KMap*)((p)->pa|KZERO);
}
/*e: function arch_kmap(arm) */

/*s: function arch_kunmap(arm) */
void
arch_kunmap(Arch_KMap *k)
{
    /*s: [[arch_kunmap()]] write back cache for page(arm) */
    cachedwbinvse(k, BY2PG);
    /*e: [[arch_kunmap()]] write back cache for page(arm) */
}
/*e: function arch_kunmap(arm) */
/*e: memory/arm/mmu.c */
