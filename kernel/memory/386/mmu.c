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
 * The VPT is a 4M frame constructed by inserting the pd into itself.
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

/*s: macros xxxSEGM(x86) */
/*
 * Simple segment descriptors with no translation.
 */
#define DATASEGM(p) { 0xFFFF, SEGG|SEGB|(0xF<<16)|SEGP|SEGPL(p)|SEGDATA|SEGW }
#define EXECSEGM(p) { 0xFFFF, SEGG|SEGD|(0xF<<16)|SEGP|SEGPL(p)|SEGEXEC|SEGR }
#define TSSSEGM(b,p) { ((b)<<16)|sizeof(Tss),\
                   ((b)&0xFF000000)|(((b)>>16)&0xFF)|SEGTSS|SEGPL(p)|SEGP }
/*s: macros other xxxSEGM(x86) */
#define EXEC16SEGM(p)   { 0xFFFF, SEGG|(0xF<<16)|SEGP|SEGPL(p)|SEGEXEC|SEGR }
/*e: macros other xxxSEGM(x86) */
/*e: macros xxxSEGM(x86) */

/*s: global gdt(x86) */
Segdesc gdt[NGDT] =
{
[NULLSEG]   { 0, 0},        /* null descriptor */
[KDSEG]     DATASEGM(0),        /* kernel data/stack */
[KESEG]     EXECSEGM(0),        /* kernel code */
[UDSEG]     DATASEGM(3),        /* user data/stack */
[UESEG]     EXECSEGM(3),        /* user code */
[TSSSEG]    TSSSEGM(0,0),       /* tss segment */
/*s: [[gdt]] other elements(x86) */
[KESEG16]       EXEC16SEGM(0),  /* kernel code 16-bit */
/*e: [[gdt]] other elements(x86) */
};
/*e: global gdt(x86) */

/*s: mmu.c forward decl(x86) */
static void taskswitch(ulong, ulong);
static void memglobal(void);
static int findhole(ulong *a, int n, int count);
static ulong vmapalloc(ulong size);
static void pdunmap(ulong*, ulong, int);
/*e: mmu.c forward decl(x86) */

/*s: global vpt and vpd(x86) */
/*e: global vpt and vpd(x86) */

/*s: function mmuinit0(x86) */
void
mmuinit0(void)
{
    // cpu->gdt should point to CPU0GDT, see cpu0init
    memmove(cpu->gdt, gdt, sizeof gdt); 
}
/*e: function mmuinit0(x86) */

/*s: global didmmuinit(x86) */
static bool didmmuinit;
/*e: global didmmuinit(x86) */

/*s: function mmuinit(x86) */
void
mmuinit(void)
{
    ulong x, *p;
    ushort ptr[3];

    didmmuinit = true;

    memglobal();
    /*s: [[mmuinit()]] vpt adjusments(x86) */
    /*e: [[mmuinit()]] vpt adjusments(x86) */
    
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
     // so now cpu->gdt is a pointer to another page (CPU0GDT <> CPUADDR)
     // but why it was slowing down things to have both data in same page?

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
        p = mmuwalk(cpu->pdproto, x, 2, false);
        if(p == nil)
            panic("mmuinit");
        *p &= ~PTEWRITE;
    }

    taskswitch(PADDR(cpu->pdproto),  (ulong)cpu + BY2PG);
    ltr(TSSSEL);
}
/*e: function mmuinit(x86) */

/*s: function memglobal(x86) */
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
    ulong *pd, *pt;

    /* only need to do this once, on bootstrap processor */
    if(cpu->cpuno != 0)
        return;

    if(!cpu->havepge)
        return;

    pd = cpu->pdproto;
    for(i=PDX(KZERO); i<1024; i++){
        if(pd[i] & PTEVALID){
            pd[i] |= PTEGLOBAL;
            if(!(pd[i] & PTESIZE)){
                pt = KADDR(pd[i]&~(BY2PG-1));
                for(j=0; j<1024; j++)
                    if(pt[j] & PTEVALID)
                        pt[j] |= PTEGLOBAL;
            }
        }
    }           
}
/*e: function memglobal(x86) */

/*s: function flushmmu(x86) */
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
/*e: function flushmmu(x86) */

/*s: function flushpg(x86) */
/*
 * Flush a single page mapping from the tlb.
 */
void
flushpg(virt_addr va)
{
    if(X86FAMILY(cpu->cpuidax) >= 4)
        invlpg(va);
    else
        putcr3(getcr3());
}
/*e: function flushpg(x86) */

/*s: function mmupdalloc(x86) */
/*
 * Allocate a new page for a page directory.
 * We keep a small cache of pre-initialized
 * page directories in each cpu (see mmupdfree).
 */
static Page*
mmupdalloc(void)
{
    int s;
    Page *page;
    kern_addr2 mmupd;

    s = splhi();
    cpu->mmupdalloc++;
    if(cpu->mmupdpool == nil){
        spllo();
        page = newpage(false, nil, nilptr);
        splhi();
        mmupd = tmpmap(page);
        memmove(mmupd, cpu->pdproto, BY2PG);
        /*s: [[mmupdalloc()]] vpt adjustments(x86) */
        /*e: [[mmupdalloc()]] vpt adjustments(x86) */
        tmpunmap(mmupd);
    }else{
        page = cpu->mmupdpool;
        cpu->mmupdpool = page->next;
        cpu->mmupdcnt--;
    }
    splx(s);
    return page;
}
/*e: function mmupdalloc(x86) */

/*s: function mmupdfree(x86) */
static void
mmupdfree(Proc *proc, Page *page)
{
    if(islo())
        panic("mmupdfree: islo");
    cpu->mmupdfree++;
    if(cpu->mmupdcnt >= 10){ // 10??? keep small cache of mmupd page, but not too big, don't want to eat too much memory for that.
        page->next = proc->mmufree;
        proc->mmufree = page;
    }else{
        page->next = cpu->mmupdpool;
        cpu->mmupdpool = page;
        cpu->mmupdcnt++;
    }
}
/*e: function mmupdfree(x86) */

/*s: function mmuptefree(x86) */
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
    kern_addr2 mmupd;
    Page **last, *page;

    if(proc->mmupd == nil || proc->mmuused == nil)
        return; // panic? bug to be called with that?

    s = splhi();
    mmupd = tmpmap(proc->mmupd);
    last = &proc->mmuused;
    for(page = *last; page; page = page->next){
        mmupd[page->daddr] = 0; //???? use daddr??
        last = &page->next;
    }
    tmpunmap(mmupd);
    splx(s);
    *last = proc->mmufree;
    proc->mmufree = proc->mmuused;
    proc->mmuused = nil;
}
/*e: function mmuptefree(x86) */

/*s: function taskswitch(x86) */
static void
taskswitch(phys_addr mmupd, ulong stack)
{
    Tss *tss;

    tss = cpu->tss;
    tss->ss0 = KDSEL;
    tss->esp0 = stack;
    tss->ss1 = KDSEL;
    tss->esp1 = stack;
    tss->ss2 = KDSEL;
    tss->esp2 = stack;
    putcr3(mmupd);
}
/*e: function taskswitch(x86) */

/*s: function mmuswitch(x86) */
void
mmuswitch(Proc* proc)
{
    ulong *mmupd;

    if(proc->newtlb){
        mmuptefree(proc);
        proc->newtlb = false;
    }

    if(proc->mmupd){
        mmupd = tmpmap(proc->mmupd);
        mmupd[PDX(CPUADDR)] = cpu->pdproto[PDX(CPUADDR)]; // for up
        tmpunmap(mmupd);
        taskswitch(proc->mmupd->pa, (ulong)(proc->kstack+KSTACK));
    }else
        taskswitch(PADDR(cpu->pdproto), (ulong)(proc->kstack+KSTACK));
}
/*e: function mmuswitch(x86) */

/*s: function mmurelease(x86) */
/*
 * Release any pages allocated for a page directory or page-tables
 * for this process:
 *   switch to the prototype pd for this processor (cpu->pdproto);
 *   call mmuptefree() to place all pages used for page-tables (proc->mmuused)
 *   onto the process' free list (proc->mmufree). This has the side-effect of
 *   cleaning any user entries in the pdb (proc->mmupd);
 *   if there's a pd put it in the cache of pre-initialised pd's
 *   for this processor (cpu->mmupdpool) or on the process' free list;
 *   finally, place any pages freed back into the free pool (palloc).
 * This routine is only called from schedinit() with palloc locked.
 */
void
mmurelease(Proc* proc)
{
    Page *page, *next;
    ulong *mmupd;

    if(islo())
        panic("mmurelease: islo");
    taskswitch(PADDR(cpu->pdproto), (ulong)cpu + BY2PG);


    /*s: [[mmurelease()]] handle kmaptable(x86) */
    /*e: [[mmurelease()]] handle kmaptable(x86) */

    if(proc->mmupd){
        mmuptefree(proc);
        mmupdfree(proc, proc->mmupd);
        proc->mmupd = nil;
    }
    for(page = proc->mmufree; page; page = next){
        next = page->next;
        if(--page->ref)
            panic("mmurelease: page->ref %d", page->ref);
        pagechainhead(page);
    }
    if(proc->mmufree && palloc.freememr.p)
        wakeup(&palloc.freememr);
    proc->mmufree = nil;
}
/*e: function mmurelease(x86) */

/*s: function upallocmmupd(x86) */
/*
 * Allocate and install pd for the current process.
 */
static void
upallocmmupd(void)
{
    int s;
    kern_addr2 mmupd;
    Page *page;
    
    if(up->mmupd != nil)
        return;
    page = mmupdalloc();
    s = splhi();
    if(up->mmupd != nil){
        /*
         * Perhaps we got an interrupt while
         * mmupdalloc was sleeping and that
         * interrupt allocated an mmupd?
         * Seems unlikely.
         */
        mmupdfree(up, page);
        splx(s);
        return;
    }
    mmupd = tmpmap(page);
    mmupd[PDX(CPUADDR)] = cpu->pdproto[PDX(CPUADDR)]; // for up
    tmpunmap(mmupd);
    up->mmupd = page;
    putcr3(up->mmupd->pa); //!!!! bootstrap! putcr3 take a PA of course
    splx(s);
}
/*e: function upallocmmupd(x86) */

/*s: function putmmu(x86) */
/*
 * Update the mmu in response to a user fault.  pa may have PTEWRITE set.
 */
void
putmmu(virt_addr va, ulong mmupte, Page*)
{
    int old, s;
    Page *page;
    kern_addr2 mmupd;
    kern_addr2 mmupt;

    if(up->mmupd == nil)
        upallocmmupd();
    /*s: [[putmmu()]] adjustments(x86) */
    // pad's code for simplified virtual memory, no VPT
    mmupd = KADDR(up->mmupd->pa);
    if(!(mmupd[PDX(va)]&PTEVALID)) {
        if(up->mmufree == nil){
            spllo();
            page = newpage(false, nil, nilptr);
            splhi();
        }
        else{
            page = up->mmufree;
            up->mmufree = page->next;
        }
        mmupd[PDX(va)] = PPN(page->pa)|PTEUSER|PTEWRITE|PTEVALID;
        mmupt = KADDR(page->pa);
        memset(mmupt, 0, BY2PG);
        page->daddr = PDX(va); // ???
        page->next = up->mmuused;
        up->mmuused = page;
    }
    mmupt = KADDR(PPN(mmupd[PDX(va)]));
    old = mmupt[PTX(va)];
    mmupt[PTX(va)] = mmupte|PTEUSER|PTEVALID;

    if(old&PTEVALID)
        flushpg(va);
    if(getcr3() != up->mmupd->pa)
         print("bad cr3 %#.8lux %#.8lux\n", getcr3(), up->mmupd->pa);
    /*e: [[putmmu()]] adjustments(x86) */
}
/*e: function putmmu(x86) */

/*s: function checkmmu(x86) */
/*
 * Double-check the user MMU.
 * Error checking only.
 */
void
checkmmu(virt_addr va, phys_addr pa)
{
    if(up->mmupd == nil)
        return;
    /*s: [[checkmmu()]] pd at pt check(x86) */
    //    if(!(vpd[PDX(va)]&PTEVALID) || !(vpt[VPTX(va)]&PTEVALID))
    //        return;
    //    if(PPN(vpt[VPTX(va)]) != pa)
    //        print("%ld %s: va=%#08lux pa=%#08lux pte=%#08lux\n",
    //            up->pid, up->text,
    //            va, pa, vpt[VPTX(va)]);
    /*e: [[checkmmu()]] pd at pt check(x86) */
}
/*e: function checkmmu(x86) */

/*s: function mmuwalk(x86) */
/*
 * Walk the page-table pointed to by pd and return a pointer
 * to the entry for virtual address va at the requested level.
 * If the entry is invalid and create isn't requested then bail
 * out early. Otherwise, for the 2nd level walk, allocate a new
 * page-table page and register it in the 1st level.  This is used
 * only to edit kernel mappings, which use pages from kernel memory,
 * so it's okay to use KADDR to look at the tables.
 */
kern_addr2
mmuwalk(kern_addr2 pd, kern_addr va, int level, bool create)
{
    ulong *table;
    void *map;

    table = &pd[PDX(va)];
    if(!(*table & PTEVALID) && create == false)
        return nil;

    switch(level){
    default:
        return nil; //todo: panic? invalid value no?
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
                map = rampage();  //when called from meminit()
            if(map == nil)
                panic("mmuwalk xspanalloc failed");
            *table = PADDR(map)|PTEWRITE|PTEVALID;
        }
        table = KADDR(PPN(*table));
        return &table[PTX(va)];
    }
}
/*e: function mmuwalk(x86) */

/*
 * Device mappings are shared by all procs and processors and
 * live in the virtual range VMAP to VMAP+VMAPSIZE.  The master
 * copy of the mappings is stored in cpu0->pdproto, and they are
 * paged in from there as necessary by vmapsync during faults.
 */

/*s: global vmaplock(x86) */
static Lock vmaplock;
/*e: global vmaplock(x86) */

/*s: function vmap(x86) */
/*
 * Add a device mapping to the vmap range.
 */
virt_addr3
vmap(phys_addr pa, int size)
{
    int osize;
    virt_addr va;
    ulong o;
    
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
    || pdmap(CPUS(0)->pdproto, pa|PTEUNCACHED|PTEWRITE, va, size) < 0){
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
/*e: function vmap(x86) */

/*s: function findhole(x86) */
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
/*e: function findhole(x86) */

/*s: function vmapalloc(x86) */
/*
 * Look for free space in the vmap.
 */
static virt_addr
vmapalloc(ulong size)
{
    int i, n, o;
    // this is really a page directory base, that spans a range of pdes
    // starting to map from VMAP to VMAP+VMAPSIZE
    kern_addr2 vpdb;
    int vpdsize;
    
    vpdb = &CPUS(0)->pdproto[PDX(VMAP)];
    vpdsize = VMAPSIZE/(4*MB);

    if(size >= 4*MB){
        n = (size+4*MB-1) / (4*MB);
        if((o = findhole(vpdb, vpdsize, n)) != -1)
            return VMAP + o*4*MB;
        return nilptr;
    }

    // size <= 4MB
    n = (size+BY2PG-1) / BY2PG;
    for(i=0; i<vpdsize; i++)
        if((vpdb[i]&PTEVALID) && !(vpdb[i]&PTESIZE))
            if((o = findhole(KADDR(PPN(vpdb[i])), WD2PG, n)) != -1)
                return VMAP + i*4*MB + o*BY2PG;

    if((o = findhole(vpdb, vpdsize, 1)) != -1)
        return VMAP + o*4*MB;
        
    /*
     * could span page directory entries, but not worth the trouble.
     * not going to be very much contention.
     */
    return nilptr;
}
/*e: function vmapalloc(x86) */

/*s: function vunmap(x86) */
/*
 * Remove a device mapping from the vmap range.
 * Since pdunmap does not remove page tables, just entries,
 * the call need not be interlocked with vmap.
 */
void
vunmap(virt_addr3 v, int size)
{
    int i;
    virt_addr va;
    ulong o;
    Cpu *nm;
    Proc *p;
    
    /*
     * might not be aligned
     */
    va = (virt_addr)v;
    o = va&(BY2PG-1);
    va -= o;
    size += o;
    size = ROUND(size, BY2PG);
    
    if(size < 0 || va < VMAP || va+size > VMAP+VMAPSIZE)
        panic("vunmap va=%#.8lux size=%#x pc=%#.8lux",
            va, size, getcallerpc(&v));

    pdunmap(CPUS(0)->pdproto, va, size);
    
    /*
     * Flush mapping from all the tlbs and copied pds.
     * This can be (and is) slow, since it is called only rarely.
     * It is possible for vunmap to be called with up == nil,
     * e.g. from the reset/init driver routines during system
     * boot. In that case it suffices to flush the CPUS(0) TLB
     * and return.
     */
    if(!active.main_reached_sched){
        putcr3(PADDR(CPUS(0)->pdproto));
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
/*e: function vunmap(x86) */

/*s: function pdmap(x86) */
/*
 * Add kernel mappings for pa -> va for a section of size bytes.
 */
int
pdmap(kern_addr2 mmupd, ulong pa, virt_addr va, int size)
{
    bool pse;
    ulong pgsz;
    kern_addr2 pte, pde;
    ulong flag, off;
    
    flag = pa&0xFFF;
    pa &= ~0xFFF;

    if((CPUS(0)->cpuiddx & Pse) && (getcr4() & 0x10))
        pse = true;
    else
        pse = false;

    for(off=0; off<size; off+=pgsz){
        pde = &mmupd[PDX(va+off)];
        if((*pde&PTEVALID) && (*pde&PTESIZE))
            panic("vmap: va=%#.8lux pa=%#.8lux pde=%#.8lux",
                va+off, pa+off, *pde);

        /*
         * Check if it can be mapped using a 4MB page:
         * va, pa aligned and size >= 4MB and processor can do it.
         */
        if(pse && (pa+off)%(4*MB) == 0 && (va+off)%(4*MB) == 0 && (size-off) >= 4*MB){
            *pde = (pa+off)|flag|PTESIZE|PTEVALID;
            pgsz = 4*MB;
        }else{
            pte = mmuwalk(mmupd, va+off, 2, true);
            if(*pte&PTEVALID)
                panic("vmap: va=%#.8lux pa=%#.8lux pte=%#.8lux",
                    va+off, pa+off, *pte);
            *pte = (pa+off)|flag|PTEVALID;
            pgsz = BY2PG;
        }
    }
    return 0;
}
/*e: function pdmap(x86) */

/*s: function pdunmap(x86) */
/*
 * Remove mappings.  Must already exist, for sanity.
 * Only used for kernel mappings, so okay to use KADDR.
 */
static void
pdunmap(kern_addr2 mmupd, virt_addr va, int size)
{
    virt_addr vae;
    kern_addr2 pde, pte;
    
    vae = va+size;
    while(va < vae){
        pde = &mmupd[PDX(va)];
        if(!(*pde & PTEVALID)){
            panic("vunmap: not mapped");
            /* 
            va = (va+4*MB-1) & ~(4*MB-1);
            continue;
            */
        }
        if(*pde & PTESIZE){
            *pde = 0;
            va = (va+4*MB-1) & ~(4*MB-1);
            continue;
        }
        pte = KADDR(PPN(*pde));
        if(!(pte[PTX(va)] & PTEVALID))
            panic("vunmap: not mapped");
        pte[PTX(va)] = 0;
        va += BY2PG;
    }
}
/*e: function pdunmap(x86) */

/*s: function vmapsync(x86) */
/*
 * Handle a fault by bringing vmap up to date.
 * Only copy pd entries and they never go away,
 * so no locking needed.
 */
int
vmapsync(virt_addr va)
{
    ulong pde;
    kern_addr2 mmupt;
    kern_addr2 mmupd;

    if(va < VMAP || va >= VMAP+VMAPSIZE)
        return 0;

    pde = CPUS(0)->pdproto[PDX(va)];
    if(!(pde&PTEVALID))
        return 0;
    if(!(pde&PTESIZE)){
        /* make sure entry will help the fault */
        mmupt = KADDR(PPN(pde));
        if(!(mmupt[PTX(va)]&PTEVALID))
            return 0;
    }
    /*s: [[vmapsync()]] va set to entry(x86) */
    // pad's code for simplified virtual memory, no VPT
    if(up == nil) {
        return 0;
    } else {
       if(up->mmupd == nil)
           upallocmmupd();
        mmupd = KADDR(up->mmupd->pa);
        mmupd[PDX(va)] = pde;
    }
    /*e: [[vmapsync()]] va set to entry(x86) */
    /*
     * TLB doesn't cache negative results, so no flush needed.
     */
    return 1;
}
/*e: function vmapsync(x86) */

/*
 * KMap is used to map individual pages into virtual memory.
 * It is rare to have more than a few KMaps at a time (in the 
 * absence of interrupts, only two at a time are ever used,
 * but interrupts can stack).  The mappings are local to a process,
 * so we can use the same range of virtual address space for
 * all processes without any coordination.
 */

/*s: global kpt(x86) */
/*e: global kpt(x86) */
/*s: constant NKPT(x86) */
/*e: constant NKPT(x86) */

/*s: function kmap(x86) */
KMap*
kmap(Page *p)
{
    if(up == nil)
        panic("kmap: up=0 pc=%#.8lux", getcallerpc(&p));
    if(p->pa < MAXKPA)
        return KADDR(p->pa);
    else
      panic("kmap: physical address too high");
}
/*e: function kmap(x86) */

/*s: function kunmap(x86) */
void
kunmap(KMap *k)
{
    ulong va;
    va = (ulong)k;
    flushpg(va); // not sure we need that

    if((ulong)va >= KZERO)
        return;
    panic("kunmap: physical address too high");
}
/*e: function kunmap(x86) */

/*
 * Temporary one-page mapping used to edit page directories.
 *
 */

/*s: function tmpmap(x86) */
virt_addr3
tmpmap(Page *p)
{
    if(islo())
        panic("tmpmap: islo");
    if(p->pa < MAXKPA)
        return KADDR(p->pa);
    else
      panic("tmpmap: physical address too high");
}
/*e: function tmpmap(x86) */

/*s: function tmpunmap(x86) */
void
tmpunmap(virt_addr3 v)
{
    if(islo())
        panic("tmpmap: islo");
    if((ulong)v >= KZERO)
        return;
    panic("tmpmap: physical address too high");
}
/*e: function tmpunmap(x86) */

/*s: function kaddr(x86) */
/*
 * These could go back to being macros once the kernel is debugged,
 * but the extra checking is nice to have.
 */
kern_addr3
kaddr(phys_addr pa)
{
    if(pa > MAXKPA)
        panic("kaddr: pa=%#.8lux", pa);
    return (kern_addr3)(pa+KZERO);
}
/*e: function kaddr(x86) */

/*s: function paddr(x86) */
phys_addr
paddr(kern_addr3 v)
{
    kern_addr va;
    
    va = (kern_addr)v;
    if(va < KZERO)
        panic("paddr: va=%#.8lux pc=%#p", va, getcallerpc(&v));
    return va-KZERO;
}
/*e: function paddr(x86) */

/*s: function countpagerefs(x86) */
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
        if(p->mmupd){
            if(print){
                if(ref[pagenumber(p->mmupd)])
                    iprint("page %#.8lux is proc %d (pid %lud) pd\n",
                        p->mmupd->pa, i, p->pid);
                continue;
            }
            if(ref[pagenumber(p->mmupd)]++ == 0)
                n++;
            else
                iprint("page %#.8lux is proc %d (pid %lud) pd but has other refs!\n",
                    p->mmupd->pa, i, p->pid);
        }
        /*s: [[countpagerefs()]] handle kmaptable(x86) */
        /*e: [[countpagerefs()]] handle kmaptable(x86) */
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
        for(pg=mm->mmupdpool; pg; pg=pg->next){
            if(print){
                if(ref[pagenumber(pg)])
                    iprint("page %#.8lux is in cpu%d mmupdpool\n",
                        pg->pa, i);
                continue;
            }
            if(ref[pagenumber(pg)]++ == 0)
                n++;
            else
                iprint("page %#.8lux is in cpu%d mmupdpool but has other refs!\n",
                    pg->pa, i);
        }
    }
    if(!print){
        iprint("%d pages in cpu mmupdpools\n", n);
        for(i=0; i<conf.ncpu; i++)
            iprint("cpu%d: %d mmupdalloc, %d mmupdfree\n",
                i, CPUS(i)->mmupdalloc, CPUS(i)->mmupdfree);
    }
}
/*e: function countpagerefs(x86) */

/*s: function cankaddr(x86) */
/*
 * Return the number of bytes that can be accessed via KADDR(pa).
 * If pa is not a valid argument to KADDR, return 0.
 */
ulong
cankaddr(phys_addr pa)
{
    if(pa >= MAXKPA)
        return 0;
    return MAXKPA - pa;
}
/*e: function cankaddr(x86) */

/*e: mmu.c */
