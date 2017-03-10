/*s: memory/arm/mmu.c */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"

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
    L1lo        = UZERO/MiB,        /* L1X(UZERO)? */
    L1hi        = (USTKTOP+MiB-1)/MiB,  /* L1X(USTKTOP+MiB-1)? */
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
    uintptr pa, va;

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
    cachedwbse(&l1[L1X(0)], sizeof(PTE));

    mmuinvalidateaddr(0);
}
/*e: function mmuinit1(arm) */

/*s: function mmul2empty(arm) */
static void
mmul2empty(Proc* proc, int clear)
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
    *l2 = proc->mmul2cache;
    proc->mmul2cache = proc->mmul2;
    proc->mmul2 = nil;
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

    /* write back dirty and invalidate l1 caches */
    cacheuwbinv();

    if(proc->newtlb){
        mmul2empty(proc, 1);
        proc->newtlb = 0;
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

    /* make sure map is in memory */
    /* could be smarter about how much? */
    cachedwbse(&l1[L1X(UZERO)], (L1hi - L1lo)*sizeof(PTE));

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
    up->newtlb = 1;
    arch_mmuswitch(up);
    arch_splx(s);
}
/*e: function arch_flushmmu(arm) */

/*s: function arch_mmurelease(arm) */
void
arch_mmurelease(Proc* proc)
{
    Page *page, *next;

    /* write back dirty and invalidate l1 caches */
    cacheuwbinv();

    mmul2empty(proc, 0);
    for(page = proc->mmul2cache; page != nil; page = next){
        next = page->next;
        if(--page->ref)
            panic("mmurelease: page->ref %d", page->ref);
        pagechainhead(page);
    }
    if(proc->mmul2cache && palloc.freememr.p)
        wakeup(&palloc.freememr);
    proc->mmul2cache = nil;

    mmul1empty();

    /* make sure map is in memory */
    /* could be smarter about how much? */
    cachedwbse(&cpu->mmul1[L1X(UZERO)], (L1hi - L1lo)*sizeof(PTE));

    /* lose any possible stale tlb entries */
    mmuinvalidate();
}
/*e: function arch_mmurelease(arm) */

/*s: function arch_putmmu(arm) */
void
arch_putmmu(uintptr va, uintptr pa, Page* page)
{
    int x;
    Page *pg;
    PTE *l1, *pte;

    x = L1X(va);
    l1 = &cpu->mmul1[x];
    if(*l1 == Fault){
        /* wasteful - l2 pages only have 256 entries - fix */
        if(up->mmul2cache == nil){
            /* auxpg since we don't need much? memset if so */
            pg = newpage(1, 0, 0);
            pg->va = VA(arch_kmap(pg));
        }
        else{
            pg = up->mmul2cache;
            up->mmul2cache = pg->next;
            memset(UINT2PTR(pg->va), 0, BY2PG);
        }
        pg->daddr = x;
        pg->next = up->mmul2;
        up->mmul2 = pg;

        /* force l2 page to memory */
        cachedwbse((void *)pg->va, BY2PG);

        *l1 = PPN(pg->pa)|Dom0|Coarse;
        cachedwbse(l1, sizeof *l1);

        if(x >= cpu->mmul1lo && x < cpu->mmul1hi){
            if(x+1 - cpu->mmul1lo < cpu->mmul1hi - x)
                cpu->mmul1lo = x+1;
            else
                cpu->mmul1hi = x;
        }
    }
    pte = UINT2PTR(KADDR(PPN(*l1)));

    /* protection bits are
     *  PTERONLY|PTEVALID;
     *  PTEWRITE|PTEVALID;
     *  PTEWRITE|PTEUNCACHED|PTEVALID;
     */
    x = Small;
    if(!(pa & PTEUNCACHED))
        x |= L2ptedramattrs;
    if(pa & PTEWRITE)
        x |= L2AP(Urw);
    else
        x |= L2AP(Uro);
    pte[L2X(va)] = PPN(pa)|x;
    cachedwbse(&pte[L2X(va)], sizeof pte[0]);

    /* clear out the current entry */
    mmuinvalidateaddr(PPN(va));

    /*  write back dirty entries - we need this because the pio() in
     *  fault.c is writing via a different virt addr and won't clean
     *  its changes out of the dcache.  Page coloring doesn't work
     *  on this mmu because the virtual cache is set associative
     *  rather than direct mapped.
     */
    if(page->cachectl[cpu->cpuno] == PG_TXTFLUSH){
        /* pio() sets PG_TXTFLUSH whenever a text pg has been written */
        cacheiinv();
        page->cachectl[cpu->cpuno] = PG_NOFLUSH;
    }
    arch_checkmmu(va, PPN(pa));
}
/*e: function arch_putmmu(arm) */

/*s: function mmuuncache(arm) */
void*
mmuuncache(void* v, usize size)
{
    int x;
    PTE *pte;
    uintptr va;

    /*
     * Simple helper for ucalloc().
     * Uncache a Section, must already be
     * valid in the MMU.
     */
    va = PTR2UINT(v);
    assert(!(va & (1*MiB-1)) && size == 1*MiB);

    x = L1X(va);
    pte = &cpu->mmul1[x];
    if((*pte & (Fine|Section|Coarse)) != Section)
        return nil;
    *pte &= ~L1ptedramattrs;
    *pte |= L1sharable;
    mmuinvalidateaddr(va);
    cachedwbinvse(pte, 4);

    return v;
}
/*e: function mmuuncache(arm) */

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
    cachedwbse(pte0, (uintptr)pte - (uintptr)pte0);
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
//old:#define   arch_kmap(p)        (Arch_KMap*)((p)->pa|kseg0)
Arch_KMap*
arch_kmap(Page *p) {
  return (Arch_KMap*)((p)->pa|KZERO);
}
/*e: function arch_kmap(arm) */

/*s: function arch_kunmap(arm) */
//old: #define   kunmap(k)
void
arch_kunmap(Arch_KMap *k)
{
    cachedwbinvse(k, BY2PG);
}
/*e: function arch_kunmap(arm) */
/*e: memory/arm/mmu.c */
