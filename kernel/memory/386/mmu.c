/*s: mmu.c */
/*
 * Memory mappings.  Life was easier when 2G of memory was enough.
 *
 * The kernel memory starts at KZERO, with the text loaded at KZERO+1M
 * (9load sits under 1M during the load).  The memory from KZERO to the
 * top of memory is mapped 1-1 with physical memory, starting at physical
 * address 0.  All kernel memory and data structures (i.e., the entries stored
 * into conf.mem) must sit in this physical range: if KZERO is at 0xF0000000,
 * then the kernel can only have 256MB of memory for itself.
 * 
 * The 256M below KZERO comprises three parts.  The lowest 4M is the
 * virtual page table, a virtual address representation of the current 
 * page table tree.  The second 4M is used for temporary per-process
 * mappings managed by kmap and kunmap.  The remaining 248M is used
 * for global (shared by all procs and all processors) device memory
 * mappings and managed by vmap and vunmap.  The total amount (256M)
 * could probably be reduced somewhat if desired.  The largest device
 * mapping is that of the video card, and even though modern video cards
 * have embarrassing amounts of memory, the video drivers only use one
 * frame buffer worth (at most 16M).  Each is described in more detail below.
 *
 * The VPT is a 4M frame constructed by inserting the pdb into itself.
 * This short-circuits one level of the page tables, with the result that 
 * the contents of second-level page tables can be accessed at VPT.  
 * We use the VPT to edit the page tables (see mmu) after inserting them
 * into the page directory.  It is a convenient mechanism for mapping what
 * might be otherwise-inaccessible pages.  The idea was borrowed from
 * the Exokernel.
 *
 * The VPT doesn't solve all our problems, because we still need to 
 * prepare page directories before we can install them.  For that, we
 * use tmpmap/tmpunmap, which map a single page at TMPADDR.
 */

/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */
#include    "io.h"

/*s: macros xxxSEGM */
/*
 * Simple segment descriptors with no translation.
 */
#define DATASEGM(p)     { 0xFFFF, SEGG|SEGB|(0xF<<16)|SEGP|SEGPL(p)|SEGDATA|SEGW }
#define EXECSEGM(p)     { 0xFFFF, SEGG|SEGD|(0xF<<16)|SEGP|SEGPL(p)|SEGEXEC|SEGR }
#define TSSSEGM(b,p)    { ((b)<<16)|sizeof(Tss),\
              ((b)&0xFF000000)|(((b)>>16)&0xFF)|SEGTSS|SEGPL(p)|SEGP }
/*s: macros other xxxSEGM */
#define EXEC16SEGM(p)   { 0xFFFF, SEGG|(0xF<<16)|SEGP|SEGPL(p)|SEGEXEC|SEGR }
/*e: macros other xxxSEGM */
/*e: macros xxxSEGM */

/*s: global gdt */
Segdesc gdt[NGDT] =
{
[NULLSEG]   { 0, 0},        /* null descriptor */
[KDSEG]     DATASEGM(0),        /* kernel data/stack */
[KESEG]     EXECSEGM(0),        /* kernel code */
[UDSEG]     DATASEGM(3),        /* user data/stack */
[UESEG]     EXECSEGM(3),        /* user code */
[TSSSEG]    TSSSEGM(0,0),       /* tss segment */
/*s: [[gdt]] other elements */
[KESEG16]       EXEC16SEGM(0),  /* kernel code 16-bit */
/*e: [[gdt]] other elements */
};
/*e: global gdt */

/*s: mmu.c forward decl */
static void taskswitch(ulong, ulong);
static void memglobal(void);
static int findhole(ulong *a, int n, int count);
static ulong vmapalloc(ulong size);
static void pdbunmap(ulong*, ulong, int);
/*e: mmu.c forward decl */


#define vpt ((ulong*)VPT)
#define VPTX(va)        (((ulong)(va))>>12)
#define vpd (vpt+VPTX(VPT))

/*s: function mmuinit0 */
void
mmuinit0(void)
{
    // cpu->gdt should be CPU0GDT, see cpu0init
    memmove(cpu->gdt, gdt, sizeof gdt); 
}
/*e: function mmuinit0 */

/*s: global didmmuinit */
static bool didmmuinit;
/*e: global didmmuinit */

/*s: function mmuinit */
void
mmuinit(void)
{
    ulong x, *p;
    ushort ptr[3];

    didmmuinit = true;

    if(0) print("vpt=%#.8ux vpd=%#p kmap=%#.8ux\n",
        VPT, vpd, KMAP);

    memglobal();
    cpu->pdb[PDX(VPT)] = PADDR(cpu->pdb)|PTEWRITE|PTEVALID;
    
    cpu->tss = malloc(sizeof(Tss));
    if(cpu->tss == nil)
        panic("mmuinit: no memory");
    memset(cpu->tss, 0, sizeof(Tss));
    cpu->tss->iomap = 0xDFFF<<16;

    /*
     * We used to keep the GDT in the Cpu structure, but it
     * turns out that that slows down access to the rest of the
     * page.  Since the Cpu structure is accessed quite often,
     * it pays off anywhere from a factor of 1.25 to 2 on real
     * hardware to separate them (the AMDs are more sensitive
     * than Intels in this regard).  Under VMware it pays off
     * a factor of about 10 to 100.
     */

    // we already did that in mmuinit0, but mmuinit is also called by
    // the other processors which don't call mmuinit0 and which have
    // a different cpu->gdt pointer.
    memmove(cpu->gdt, gdt, sizeof gdt); 
    x = (ulong)cpu->tss;
    cpu->gdt[TSSSEG].d0 = (x<<16)|sizeof(Tss);
    cpu->gdt[TSSSEG].d1 = (x&0xFF000000)|((x>>16)&0xFF)|SEGTSS|SEGPL(0)|SEGP;

    ptr[0] = sizeof(gdt)-1;
    x = (ulong)cpu->gdt;
    ptr[1] = x & 0xFFFF;
    ptr[2] = (x>>16) & 0xFFFF;
    lgdt(ptr);

    ptr[0] = sizeof(Segdesc)*256-1;
    x = IDTADDR;
    ptr[1] = x & 0xFFFF;
    ptr[2] = (x>>16) & 0xFFFF;
    lidt(ptr);

    /* make kernel text unwritable */
    for(x = KTZERO; x < (ulong)etext; x += BY2PG){
        p = mmuwalk(cpu->pdb, x, 2, 0);
        if(p == nil)
            panic("mmuinit");
        *p &= ~PTEWRITE;
    }

    taskswitch(PADDR(cpu->pdb),  (ulong)cpu + BY2PG);
    ltr(TSSSEL);
}
/*e: function mmuinit */

/*s: function memglobal */
/* 
 * On processors that support it, we set the PTEGLOBAL bit in
 * page table and page directory entries that map kernel memory.
 * Doing this tells the processor not to bother flushing them
 * from the TLB when doing the TLB flush associated with a 
 * context switch (write to CR3).  Since kernel memory mappings
 * are never removed, this is safe.  (If we ever remove kernel memory
 * mappings, we can do a full flush by turning off the PGE bit in CR4,
 * writing to CR3, and then turning the PGE bit back on.) 
 *
 * See also mmukmap below.
 * 
 * Processor support for the PTEGLOBAL bit is enabled in devarch.c.
 */
static void
memglobal(void)
{
    int i, j;
    ulong *pde, *pte;

    /* only need to do this once, on bootstrap processor */
    if(cpu->cpuno != 0)
        return;

    if(!cpu->havepge)
        return;

    pde = cpu->pdb;
    for(i=PDX(KZERO); i<1024; i++){
        if(pde[i] & PTEVALID){
            pde[i] |= PTEGLOBAL;
            if(!(pde[i] & PTESIZE)){
                pte = KADDR(pde[i]&~(BY2PG-1));
                for(j=0; j<1024; j++)
                    if(pte[j] & PTEVALID)
                        pte[j] |= PTEGLOBAL;
            }
        }
    }           
}
/*e: function memglobal */

/*s: function flushmmu */
/*
 * Flush all the user-space and device-mapping mmu info
 * for this process, because something has been deleted.
 * It will be paged back in on demand.
 */
void
flushmmu(void)
{
    int s;

    s = splhi();
    up->newtlb = true;
    mmuswitch(up);
    splx(s);
}
/*e: function flushmmu */

/*s: function flushpg */
/*
 * Flush a single page mapping from the tlb.
 */
void
flushpg(ulong va)
{
    if(X86FAMILY(cpu->cpuidax) >= 4)
        invlpg(va);
    else
        putcr3(getcr3());
}
/*e: function flushpg */

/*s: function mmupdballoc */
/*
 * Allocate a new page for a page directory. 
 * We keep a small cache of pre-initialized
 * page directories in each cpu.
 */
static Page*
mmupdballoc(void)
{
    int s;
    Page *page;
    ulong *pdb;

    s = splhi();
    cpu->pdballoc++;
    if(cpu->pdbpool == 0){
        spllo();
        page = newpage(0, 0, 0);
        page->va = (ulong)vpd;
        splhi();
        pdb = tmpmap(page);
        memmove(pdb, cpu->pdb, BY2PG);
        pdb[PDX(VPT)] = page->pa|PTEWRITE|PTEVALID; /* set up VPT */
        tmpunmap(pdb);
    }else{
        page = cpu->pdbpool;
        cpu->pdbpool = page->next;
        cpu->pdbcnt--;
    }
    splx(s);
    return page;
}
/*e: function mmupdballoc */

/*s: function mmupdbfree */
//@Scheck: not dead
static void
mmupdbfree(Proc *proc, Page *p)
{
    if(islo())
        panic("mmupdbfree: islo");
    cpu->pdbfree++;
    if(cpu->pdbcnt >= 10){
        p->next = proc->mmufree;
        proc->mmufree = p;
    }else{
        p->next = cpu->pdbpool;
        cpu->pdbpool = p;
        cpu->pdbcnt++;
    }
}
/*e: function mmupdbfree */

/*s: function mmuptefree */
/*
 * A user-space memory segment has been deleted, or the
 * process is exiting.  Clear all the pde entries for user-space
 * memory mappings and device mappings.  Any entries that
 * are needed will be paged back in as necessary.
 */
static void
mmuptefree(Proc* proc)
{
    int s;
    ulong *pdb;
    Page **last, *page;

    if(proc->mmupdb == nil || proc->mmuused == nil)
        return;
    s = splhi();
    pdb = tmpmap(proc->mmupdb);
    last = &proc->mmuused;
    for(page = *last; page; page = page->next){
        pdb[page->daddr] = 0;
        last = &page->next;
    }
    tmpunmap(pdb);
    splx(s);
    *last = proc->mmufree;
    proc->mmufree = proc->mmuused;
    proc->mmuused = 0;
}
/*e: function mmuptefree */

/*s: function taskswitch */
static void
taskswitch(ulong pdb, ulong stack)
{
    Tss *tss;

    tss = cpu->tss;
    tss->ss0 = KDSEL;
    tss->esp0 = stack;
    tss->ss1 = KDSEL;
    tss->esp1 = stack;
    tss->ss2 = KDSEL;
    tss->esp2 = stack;
    putcr3(pdb);
}
/*e: function taskswitch */

/*s: function mmuswitch */
void
mmuswitch(Proc* proc)
{
    ulong *pdb;

    if(proc->newtlb){
        mmuptefree(proc);
        proc->newtlb = false;
    }

    if(proc->mmupdb){
        pdb = tmpmap(proc->mmupdb);
        pdb[PDX(CPUADDR)] = cpu->pdb[PDX(CPUADDR)];
        tmpunmap(pdb);
        taskswitch(proc->mmupdb->pa, (ulong)(proc->kstack+KSTACK));
    }else
        taskswitch(PADDR(cpu->pdb), (ulong)(proc->kstack+KSTACK));
}
/*e: function mmuswitch */

/*s: function mmurelease */
/*
 * Release any pages allocated for a page directory base or page-tables
 * for this process:
 *   switch to the prototype pdb for this processor (cpu->pdb);
 *   call mmuptefree() to place all pages used for page-tables (proc->mmuused)
 *   onto the process' free list (proc->mmufree). This has the side-effect of
 *   cleaning any user entries in the pdb (proc->mmupdb);
 *   if there's a pdb put it in the cache of pre-initialised pdb's
 *   for this processor (cpu->pdbpool) or on the process' free list;
 *   finally, place any pages freed back into the free pool (palloc).
 * This routine is only called from schedinit() with palloc locked.
 */
void
mmurelease(Proc* proc)
{
    Page *page, *next;
    ulong *pdb;

    if(islo())
        panic("mmurelease: islo");
    taskswitch(PADDR(cpu->pdb), (ulong)cpu + BY2PG);
    if(proc->kmaptable){
        if(proc->mmupdb == nil)
            panic("mmurelease: no mmupdb");
        if(--proc->kmaptable->ref)
            panic("mmurelease: kmap ref %d", proc->kmaptable->ref);
        if(proc->nkmap)
            panic("mmurelease: nkmap %d", proc->nkmap);
        /*
         * remove kmaptable from pdb before putting pdb up for reuse.
         */
        pdb = tmpmap(proc->mmupdb);
        if(PPN(pdb[PDX(KMAP)]) != proc->kmaptable->pa)
            panic("mmurelease: bad kmap pde %#.8lux kmap %#.8lux",
                pdb[PDX(KMAP)], proc->kmaptable->pa);
        pdb[PDX(KMAP)] = 0;
        tmpunmap(pdb);
        /*
         * move kmaptable to free list.
         */
        pagechainhead(proc->kmaptable);
        proc->kmaptable = 0;
    }
    if(proc->mmupdb){
        mmuptefree(proc);
        mmupdbfree(proc, proc->mmupdb);
        proc->mmupdb = 0;
    }
    for(page = proc->mmufree; page; page = next){
        next = page->next;
        if(--page->ref)
            panic("mmurelease: page->ref %d", page->ref);
        pagechainhead(page);
    }
    if(proc->mmufree && palloc.r.p)
        wakeup(&palloc.r);
    proc->mmufree = 0;
}
/*e: function mmurelease */

/*s: function upallocpdb */
/*
 * Allocate and install pdb for the current process.
 */
//@Scheck: no dead, called bellow
static void
upallocpdb(void)
{
    int s;
    ulong *pdb;
    Page *page;
    
    if(up->mmupdb != nil)
        return;
    page = mmupdballoc();
    s = splhi();
    if(up->mmupdb != nil){
        /*
         * Perhaps we got an interrupt while
         * mmupdballoc was sleeping and that
         * interrupt allocated an mmupdb?
         * Seems unlikely.
         */
        mmupdbfree(up, page);
        splx(s);
        return;
    }
    pdb = tmpmap(page);
    pdb[PDX(CPUADDR)] = cpu->pdb[PDX(CPUADDR)];
    tmpunmap(pdb);
    up->mmupdb = page;
    putcr3(up->mmupdb->pa);
    splx(s);
}
/*e: function upallocpdb */

/*s: function putmmu */
/*
 * Update the mmu in response to a user fault.  pa may have PTEWRITE set.
 */
void
putmmu(ulong va, ulong pa, Page*)
{
    int old, s;
    Page *page;

    if(up->mmupdb == nil)
        upallocpdb();

    /*
     * We should be able to get through this with interrupts
     * turned on (if we get interrupted we'll just pick up 
     * where we left off) but we get many faults accessing
     * vpt[] near the end of this function, and they always happen
     * after the process has been switched out and then 
     * switched back, usually many times in a row (perhaps
     * it cannot switch back successfully for some reason).
     * 
     * In any event, I'm tired of searching for this bug.  
     * Turn off interrupts during putmmu even though
     * we shouldn't need to.        - rsc
     */
    
    s = splhi();
    if(!(vpd[PDX(va)]&PTEVALID)){
        if(up->mmufree == 0){
            spllo();
            page = newpage(0, 0, 0);
            splhi();
        }
        else{
            page = up->mmufree;
            up->mmufree = page->next;
        }
        vpd[PDX(va)] = PPN(page->pa)|PTEUSER|PTEWRITE|PTEVALID;
        /* page is now mapped into the VPT - clear it */
        memset((void*)(VPT+PDX(va)*BY2PG), 0, BY2PG);
        page->daddr = PDX(va);
        page->next = up->mmuused;
        up->mmuused = page;
    }
    old = vpt[VPTX(va)];
    vpt[VPTX(va)] = pa|PTEUSER|PTEVALID;
    if(old&PTEVALID)
        flushpg(va);
    if(getcr3() != up->mmupdb->pa)
        print("bad cr3 %#.8lux %#.8lux\n", getcr3(), up->mmupdb->pa);
    splx(s);
}
/*e: function putmmu */

/*s: function checkmmu */
/*
 * Double-check the user MMU.
 * Error checking only.
 */
void
checkmmu(ulong va, ulong pa)
{
    if(up->mmupdb == 0)
        return;
    if(!(vpd[PDX(va)]&PTEVALID) || !(vpt[VPTX(va)]&PTEVALID))
        return;
    if(PPN(vpt[VPTX(va)]) != pa)
        print("%ld %s: va=%#08lux pa=%#08lux pte=%#08lux\n",
            up->pid, up->text,
            va, pa, vpt[VPTX(va)]);
}
/*e: function checkmmu */

/*s: function mmuwalk */
/*
 * Walk the page-table pointed to by pdb and return a pointer
 * to the entry for virtual address va at the requested level.
 * If the entry is invalid and create isn't requested then bail
 * out early. Otherwise, for the 2nd level walk, allocate a new
 * page-table page and register it in the 1st level.  This is used
 * only to edit kernel mappings, which use pages from kernel memory,
 * so it's okay to use KADDR to look at the tables.
 */
ulong*
mmuwalk(ulong* pdb, ulong va, int level, int create)
{
    ulong *table;
    void *map;

    table = &pdb[PDX(va)];
    if(!(*table & PTEVALID) && create == 0)
        return nil;

    switch(level){

    default:
        return nil;

    case 1:
        return table;

    case 2:
        if(*table & PTESIZE)
            panic("mmuwalk2: va %luX entry %luX", va, *table);
        if(!(*table & PTEVALID)){
            /*
             * Have to call low-level allocator from
             * memory.c if we haven't set up the xalloc
             * tables yet.
             */
            if(didmmuinit)
                map = xspanalloc(BY2PG, BY2PG, 0);
            else
                map = rampage();
            if(map == nil)
                panic("mmuwalk xspanalloc failed");
            *table = PADDR(map)|PTEWRITE|PTEVALID;
        }
        table = KADDR(PPN(*table));
        return &table[PTX(va)];
    }
}
/*e: function mmuwalk */

/*
 * Device mappings are shared by all procs and processors and
 * live in the virtual range VMAP to VMAP+VMAPSIZE.  The master
 * copy of the mappings is stored in cpu0->pdb, and they are
 * paged in from there as necessary by vmapsync during faults.
 */

/*s: global vmaplock */
static Lock vmaplock;
/*e: global vmaplock */

/*s: function vmap */
/*
 * Add a device mapping to the vmap range.
 */
void*
vmap(ulong pa, int size)
{
    int osize;
    ulong o, va;
    
    /*
     * might be asking for less than a page.
     */
    osize = size;
    o = pa & (BY2PG-1);
    pa -= o;
    size += o;

    size = ROUND(size, BY2PG);
    if(pa == 0){
        print("vmap pa=0 pc=%#p\n", getcallerpc(&pa));
        return nil;
    }
    ilock(&vmaplock);
    if((va = vmapalloc(size)) == 0 
    || pdbmap(CPUS(0)->pdb, pa|PTEUNCACHED|PTEWRITE, va, size) < 0){
        iunlock(&vmaplock);
        return nil;
    }
    iunlock(&vmaplock);
    /* avoid trap on local processor
    for(i=0; i<size; i+=4*MB)
        vmapsync(va+i);
    */
    USED(osize);
//  print("  vmap %#.8lux %d => %#.8lux\n", pa+o, osize, va+o);
    return (void*)(va + o);
}
/*e: function vmap */

/*s: function findhole */
static int
findhole(ulong *a, int n, int count)
{
    int have, i;
    
    have = 0;
    for(i=0; i<n; i++){
        if(a[i] == 0)
            have++;
        else
            have = 0;
        if(have >= count)
            return i+1 - have;
    }
    return -1;
}
/*e: function findhole */

/*s: function vmapalloc */
/*
 * Look for free space in the vmap.
 */
static ulong
vmapalloc(ulong size)
{
    int i, n, o;
    ulong *vpdb;
    int vpdbsize;
    
    vpdb = &CPUS(0)->pdb[PDX(VMAP)];
    vpdbsize = VMAPSIZE/(4*MB);

    if(size >= 4*MB){
        n = (size+4*MB-1) / (4*MB);
        if((o = findhole(vpdb, vpdbsize, n)) != -1)
            return VMAP + o*4*MB;
        return 0;
    }
    n = (size+BY2PG-1) / BY2PG;
    for(i=0; i<vpdbsize; i++)
        if((vpdb[i]&PTEVALID) && !(vpdb[i]&PTESIZE))
            if((o = findhole(KADDR(PPN(vpdb[i])), WD2PG, n)) != -1)
                return VMAP + i*4*MB + o*BY2PG;
    if((o = findhole(vpdb, vpdbsize, 1)) != -1)
        return VMAP + o*4*MB;
        
    /*
     * could span page directory entries, but not worth the trouble.
     * not going to be very much contention.
     */
    return 0;
}
/*e: function vmapalloc */

/*s: function vunmap */
/*
 * Remove a device mapping from the vmap range.
 * Since pdbunmap does not remove page tables, just entries,
 * the call need not be interlocked with vmap.
 */
void
vunmap(void *v, int size)
{
    int i;
    ulong va, o;
    Cpu *nm;
    Proc *p;
    
    /*
     * might not be aligned
     */
    va = (ulong)v;
    o = va&(BY2PG-1);
    va -= o;
    size += o;
    size = ROUND(size, BY2PG);
    
    if(size < 0 || va < VMAP || va+size > VMAP+VMAPSIZE)
        panic("vunmap va=%#.8lux size=%#x pc=%#.8lux",
            va, size, getcallerpc(&v));

    pdbunmap(CPUS(0)->pdb, va, size);
    
    /*
     * Flush mapping from all the tlbs and copied pdbs.
     * This can be (and is) slow, since it is called only rarely.
     * It is possible for vunmap to be called with up == nil,
     * e.g. from the reset/init driver routines during system
     * boot. In that case it suffices to flush the CPUS(0) TLB
     * and return.
     */
    if(!active.main_reached_sched){
        putcr3(PADDR(CPUS(0)->pdb));
        return;
    }
    for(i=0; i<conf.nproc; i++){
        p = proctab(i);
        if(p->state == Dead)
            continue;
        if(p != up)
            p->newtlb = true;
    }
    for(i=0; i<conf.ncpu; i++){
        nm = CPUS(i);
        if(nm != cpu)
            nm->flushmmu = true;
    }
    flushmmu();
    for(i=0; i<conf.ncpu; i++){
        nm = CPUS(i);
        if(nm != cpu)
            while((active.cpus&(1<<nm->cpuno)) && nm->flushmmu)
                ;
    }
}
/*e: function vunmap */

/*s: function pdbmap */
/*
 * Add kernel mappings for pa -> va for a section of size bytes.
 */
int
pdbmap(ulong *pdb, ulong pa, ulong va, int size)
{
    int pse;
    ulong pgsz, *pte, *table;
    ulong flag, off;
    
    flag = pa&0xFFF;
    pa &= ~0xFFF;

    if((CPUS(0)->cpuiddx & Pse) && (getcr4() & 0x10))
        pse = 1;
    else
        pse = 0;

    for(off=0; off<size; off+=pgsz){
        table = &pdb[PDX(va+off)];
        if((*table&PTEVALID) && (*table&PTESIZE))
            panic("vmap: va=%#.8lux pa=%#.8lux pde=%#.8lux",
                va+off, pa+off, *table);

        /*
         * Check if it can be mapped using a 4MB page:
         * va, pa aligned and size >= 4MB and processor can do it.
         */
        if(pse && (pa+off)%(4*MB) == 0 && (va+off)%(4*MB) == 0 && (size-off) >= 4*MB){
            *table = (pa+off)|flag|PTESIZE|PTEVALID;
            pgsz = 4*MB;
        }else{
            pte = mmuwalk(pdb, va+off, 2, 1);
            if(*pte&PTEVALID)
                panic("vmap: va=%#.8lux pa=%#.8lux pte=%#.8lux",
                    va+off, pa+off, *pte);
            *pte = (pa+off)|flag|PTEVALID;
            pgsz = BY2PG;
        }
    }
    return 0;
}
/*e: function pdbmap */

/*s: function pdbunmap */
/*
 * Remove mappings.  Must already exist, for sanity.
 * Only used for kernel mappings, so okay to use KADDR.
 */
static void
pdbunmap(ulong *pdb, ulong va, int size)
{
    ulong vae;
    ulong *table;
    
    vae = va+size;
    while(va < vae){
        table = &pdb[PDX(va)];
        if(!(*table & PTEVALID)){
            panic("vunmap: not mapped");
            /* 
            va = (va+4*MB-1) & ~(4*MB-1);
            continue;
            */
        }
        if(*table & PTESIZE){
            *table = 0;
            va = (va+4*MB-1) & ~(4*MB-1);
            continue;
        }
        table = KADDR(PPN(*table));
        if(!(table[PTX(va)] & PTEVALID))
            panic("vunmap: not mapped");
        table[PTX(va)] = 0;
        va += BY2PG;
    }
}
/*e: function pdbunmap */

/*s: function vmapsync */
/*
 * Handle a fault by bringing vmap up to date.
 * Only copy pdb entries and they never go away,
 * so no locking needed.
 */
int
vmapsync(ulong va)
{
    ulong entry, *table;

    if(va < VMAP || va >= VMAP+VMAPSIZE)
        return 0;

    entry = CPUS(0)->pdb[PDX(va)];
    if(!(entry&PTEVALID))
        return 0;
    if(!(entry&PTESIZE)){
        /* make sure entry will help the fault */
        table = KADDR(PPN(entry));
        if(!(table[PTX(va)]&PTEVALID))
            return 0;
    }
    vpd[PDX(va)] = entry;
    /*
     * TLB doesn't cache negative results, so no flush needed.
     */
    return 1;
}
/*e: function vmapsync */

/*
 * KMap is used to map individual pages into virtual memory.
 * It is rare to have more than a few KMaps at a time (in the 
 * absence of interrupts, only two at a time are ever used,
 * but interrupts can stack).  The mappings are local to a process,
 * so we can use the same range of virtual address space for
 * all processes without any coordination.
 */
#define kpt (vpt+VPTX(KMAP))
#define NKPT (KMAPSIZE/BY2PG)

/*s: function kmap */
KMap*
kmap(Page *page)
{
    int i, o, s;

    if(up == nil)
        panic("kmap: up=0 pc=%#.8lux", getcallerpc(&page));
    if(up->mmupdb == nil)
        upallocpdb();
    if(up->nkmap < 0)
        panic("kmap %lud %s: nkmap=%d", up->pid, up->text, up->nkmap);
    
    /*
     * Splhi shouldn't be necessary here, but paranoia reigns.
     * See comment in putmmu above.
     */
    s = splhi();
    up->nkmap++;
    if(!(vpd[PDX(KMAP)]&PTEVALID)){
        /* allocate page directory */
        if(KMAPSIZE > BY2XPG)
            panic("bad kmapsize");
        if(up->kmaptable != nil)
            panic("kmaptable");
        spllo();
        up->kmaptable = newpage(0, 0, 0);
        splhi();
        vpd[PDX(KMAP)] = up->kmaptable->pa|PTEWRITE|PTEVALID;
        flushpg((ulong)kpt);
        memset(kpt, 0, BY2PG);
        kpt[0] = page->pa|PTEWRITE|PTEVALID;
        up->lastkmap = 0;
        splx(s);
        return (KMap*)KMAP;
    }
    if(up->kmaptable == nil)
        panic("no kmaptable");
    o = up->lastkmap+1;
    for(i=0; i<NKPT; i++){
        if(kpt[(i+o)%NKPT] == 0){
            o = (i+o)%NKPT;
            kpt[o] = page->pa|PTEWRITE|PTEVALID;
            up->lastkmap = o;
            splx(s);
            return (KMap*)(KMAP+o*BY2PG);
        }
    }
    panic("out of kmap");
    return nil;
}
/*e: function kmap */

/*s: function kunmap */
void
kunmap(KMap *k)
{
    ulong va;

    va = (ulong)k;
    if(up->mmupdb == nil || !(vpd[PDX(KMAP)]&PTEVALID))
        panic("kunmap: no kmaps");
    if(va < KMAP || va >= KMAP+KMAPSIZE)
        panic("kunmap: bad address %#.8lux pc=%#p", va, getcallerpc(&k));
    if(!(vpt[VPTX(va)]&PTEVALID))
        panic("kunmap: not mapped %#.8lux pc=%#p", va, getcallerpc(&k));
    up->nkmap--;
    if(up->nkmap < 0)
        panic("kunmap %lud %s: nkmap=%d", up->pid, up->text, up->nkmap);
    vpt[VPTX(va)] = 0;
    flushpg(va);
}
/*e: function kunmap */

/*
 * Temporary one-page mapping used to edit page directories.
 *
 * The fasttmp #define controls whether the code optimizes
 * the case where the page is already mapped in the physical
 * memory window.  
 */
#define fasttmp 1

/*s: function tmpmap */
void*
tmpmap(Page *p)
{
    ulong i;
    ulong *entry;
    
    if(islo())
        panic("tmpaddr: islo");

    if(fasttmp && p->pa < -KZERO)
        return KADDR(p->pa);

    /*
     * PDX(TMPADDR) == PDX(CPUADDR), so this
     * entry is private to the processor and shared 
     * between up->mmupdb (if any) and cpu->pdb.
     */
    entry = &vpt[VPTX(TMPADDR)];
    if(!(*entry&PTEVALID)){
        for(i=KZERO; i<=CPU0CPU; i+=BY2PG)
            print("%#p: *%#p=%#p (vpt=%#p index=%#p)\n", i, &vpt[VPTX(i)], vpt[VPTX(i)], vpt, VPTX(i));
        panic("tmpmap: no entry");
    }
    if(PPN(*entry) != PPN(TMPADDR-KZERO))
        panic("tmpmap: already mapped entry=%#.8lux", *entry);
    *entry = p->pa|PTEWRITE|PTEVALID;
    flushpg(TMPADDR);
    return (void*)TMPADDR;
}
/*e: function tmpmap */

/*s: function tmpunmap */
void
tmpunmap(void *v)
{
    ulong *entry;
    
    if(islo())
        panic("tmpaddr: islo");
    if(fasttmp && (ulong)v >= KZERO && v != (void*)TMPADDR)
        return;
    if(v != (void*)TMPADDR)
        panic("tmpunmap: bad address");
    entry = &vpt[VPTX(TMPADDR)];
    if(!(*entry&PTEVALID) || PPN(*entry) == PPN(PADDR(TMPADDR)))
        panic("tmpmap: not mapped entry=%#.8lux", *entry);
    *entry = PPN(TMPADDR-KZERO)|PTEWRITE|PTEVALID;
    flushpg(TMPADDR);
}
/*e: function tmpunmap */

/*s: function kaddr */
/*
 * These could go back to being macros once the kernel is debugged,
 * but the extra checking is nice to have.
 */
//todo: should return a kern_addr
kern_addr3
kaddr(phys_addr pa)
{
    if(pa > (ulong)-KZERO)
        panic("kaddr: pa=%#.8lux", pa);
    return (kern_addr3)(pa+KZERO);
}
/*e: function kaddr */

/*s: function paddr */
kern_addr
paddr(kern_addr3 v)
{
    kern_addr va;
    
    va = (kern_addr)v;
    if(va < KZERO)
        panic("paddr: va=%#.8lux pc=%#p", va, getcallerpc(&v));
    return va-KZERO;
}
/*e: function paddr */

/*s: function countpagerefs */
/*
 * More debugging.
 */
void
countpagerefs(ulong *ref, int print)
{
    int i, n;
    Cpu *mm;
    Page *pg;
    Proc *p;
    
    n = 0;
    for(i=0; i<conf.nproc; i++){
        p = proctab(i);
        if(p->mmupdb){
            if(print){
                if(ref[pagenumber(p->mmupdb)])
                    iprint("page %#.8lux is proc %d (pid %lud) pdb\n",
                        p->mmupdb->pa, i, p->pid);
                continue;
            }
            if(ref[pagenumber(p->mmupdb)]++ == 0)
                n++;
            else
                iprint("page %#.8lux is proc %d (pid %lud) pdb but has other refs!\n",
                    p->mmupdb->pa, i, p->pid);
        }
        if(p->kmaptable){
            if(print){
                if(ref[pagenumber(p->kmaptable)])
                    iprint("page %#.8lux is proc %d (pid %lud) kmaptable\n",
                        p->kmaptable->pa, i, p->pid);
                continue;
            }
            if(ref[pagenumber(p->kmaptable)]++ == 0)
                n++;
            else
                iprint("page %#.8lux is proc %d (pid %lud) kmaptable but has other refs!\n",
                    p->kmaptable->pa, i, p->pid);
        }
        for(pg=p->mmuused; pg; pg=pg->next){
            if(print){
                if(ref[pagenumber(pg)])
                    iprint("page %#.8lux is on proc %d (pid %lud) mmuused\n",
                        pg->pa, i, p->pid);
                continue;
            }
            if(ref[pagenumber(pg)]++ == 0)
                n++;
            else
                iprint("page %#.8lux is on proc %d (pid %lud) mmuused but has other refs!\n",
                    pg->pa, i, p->pid);
        }
        for(pg=p->mmufree; pg; pg=pg->next){
            if(print){
                if(ref[pagenumber(pg)])
                    iprint("page %#.8lux is on proc %d (pid %lud) mmufree\n",
                        pg->pa, i, p->pid);
                continue;
            }
            if(ref[pagenumber(pg)]++ == 0)
                n++;
            else
                iprint("page %#.8lux is on proc %d (pid %lud) mmufree but has other refs!\n",
                    pg->pa, i, p->pid);
        }
    }
    if(!print)
        iprint("%d pages in proc mmu\n", n);
    n = 0;
    for(i=0; i<conf.ncpu; i++){
        mm = CPUS(i);
        for(pg=mm->pdbpool; pg; pg=pg->next){
            if(print){
                if(ref[pagenumber(pg)])
                    iprint("page %#.8lux is in cpu%d pdbpool\n",
                        pg->pa, i);
                continue;
            }
            if(ref[pagenumber(pg)]++ == 0)
                n++;
            else
                iprint("page %#.8lux is in cpu%d pdbpool but has other refs!\n",
                    pg->pa, i);
        }
    }
    if(!print){
        iprint("%d pages in cpu pdbpools\n", n);
        for(i=0; i<conf.ncpu; i++)
            iprint("cpu%d: %d pdballoc, %d pdbfree\n",
                i, CPUS(i)->pdballoc, CPUS(i)->pdbfree);
    }
}
/*e: function countpagerefs */

/*s: function cankaddr */
/*
 * Return the number of bytes that can be accessed via KADDR(pa).
 * If pa is not a valid argument to KADDR, return 0.
 */
ulong
cankaddr(ulong pa)
{
    if(pa >= -KZERO)
        return 0;
    return -KZERO - pa;
}
/*e: function cankaddr */

/*e: mmu.c */
