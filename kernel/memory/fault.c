/*s: fault.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

/*s: fault.c forward decl */
void        pio(Segment *, ulong, ulong, Page **);
/*e: fault.c forward decl */

/*s: function fault */
int
fault(virt_addr addr, bool read)
{
    Segment *s;
    char *sps;

    if(up == nil)
        panic("fault: nil up");
    if(up->nlocks.ref)
        print("fault: addr %#p: nlocks %ld\n", addr, up->nlocks.ref);

    sps = up->psstate;
    up->psstate = "Fault";
    arch_spllo();

    cpu->pfault++;
    for(;;) {
        s = seg(up, addr, /*dolock*/true); /* leaves s->lk qlocked if seg != nil */
        if(s == nil) {
            up->psstate = sps;
            return -1;
        }
        if(!read && (s->type&SG_RONLY)) {
            qunlock(&s->lk);
            up->psstate = sps;
            return -1;
        }

        if(fixfault(s, addr, read, /*putmmu*/true) == 0) /* qunlocks s->lk */
            break;

        // else? try again?
    }

    up->psstate = sps;
    return 0;
}
/*e: function fault */

/*s: function faulterror */
static void
faulterror(char *s, Chan *c, bool freemem)
{
    char buf[ERRMAX];

    if(c && c->path){
        snprint(buf, sizeof buf, "%s accessing %s: %s", s, c->path->s, up->errstr);
        s = buf;
    }
    if(up->nerrlab) {
        postnote(up, 1, s, NDebug);
        error(s);
    }
    pexit(s, freemem);
}
/*e: function faulterror */

// for debugging SG_PHYSICAL
void    (*checkaddr)(ulong, Segment *, Page *);
ulong   addr2check;

/*s: function fixfault */
int
fixfault(Segment *s, virt_addr addr, bool read, bool doputmmu)
{
    int type;
    int ref;
    Pagetable **pde, *pt;
    Page **pte, *lkp, *new;
    ulong mmupte = nilptr;
    ulong soff;

    addr &= ~(BY2PG-1);
    soff = addr - s->base;

    // walk
    pde = &s->pagedir[soff/PAGETABMAPMEM];
    if(*pde == nil)
        *pde = ptalloc();
    pt = *pde;
    pte = &pt->pagetab[(soff&(PAGETABMAPMEM-1))/BY2PG];
    if(pte < pt->first)
        pt->first = pte;
    if(pte > pt->last)
        pt->last = pte;

    type = s->type&SG_TYPE;

    switch(type) {
    case SG_TEXT:           /* Demand load */
        /*s: [[fixfault()]] page in for SG_TEXT pte if pagedout */
        if(pagedout(*pte))
            pio(s, addr, soff, pte);
        /*e: [[fixfault()]] page in for SG_TEXT pte if pagedout */

        mmupte = PPN((*pte)->pa) | PTERONLY|PTEVALID;
        (*pte)->modref = PG_REF;
        break;

    case SG_BSS:
    case SG_SHARED:         /* Zero fill on demand */
    case SG_STACK:
        if(*pte == nil) {
            new = newpage(true, &s, addr); // true so clear! zeroed BSS & heap!
            if(s == nil) //?? when can be nil at exit?
                return -1;
            *pte = new;
        }
        goto common;

    case SG_DATA:
    common:         /* Demand load/pagein/copy on write */
        /*s: [[fixfault()]] page in for SG_DATA or swapin (SG_BSS, etc) pte if pagedout */
        if(pagedout(*pte))
            pio(s, addr, soff, pte);
        /*e: [[fixfault()]] page in for SG_DATA or swapin (SG_BSS, etc) pte if pagedout */

        /*s: [[fixfault()]] if read and copy on write, adjust mmupte and break */
        /*
         *  It's only possible to copy on write if
         *  we're the only user of the segment.
         */
        if(read && conf.copymode == false && s->ref == 1) {
            mmupte = PPN((*pte)->pa)|PTERONLY|PTEVALID;
            (*pte)->modref |= PG_REF;
            break;
        }
        /*e: [[fixfault()]] if read and copy on write, adjust mmupte and break */

        lkp = *pte;

        lock(lkp);

        if(lkp->image == &swapimage)
            ref = lkp->ref + swapcount(lkp->daddr);
        else
            ref = lkp->ref;
        /*s: [[fixfault()]] if one ref and page has an image */
        if(ref == 1 && lkp->image){
            /* save a copy of the original for the image cache */
            duppage(lkp);
            ref = lkp->ref;
        }
        /*e: [[fixfault()]] if one ref and page has an image */
        unlock(lkp);

        /*s: [[fixfault()]] if write and more than one ref then copy page */
        if(ref > 1){
            new = newpage(false, &s, addr);
            if(s == nil)
                return -1;
            *pte = new;
            copypage(lkp, *pte);
            putpage(lkp); //why??
        }
        /*e: [[fixfault()]] if write and more than one ref then copy page */

        mmupte = PPN((*pte)->pa) | PTEWRITE | PTEVALID;
        (*pte)->modref = PG_MOD|PG_REF;
        break;

    /*s: [[fixfault()]] SG_PHYSICAL case */
    case SG_PHYSICAL:
        if(*pte == nil) {
            new = smalloc(sizeof(Page));
            new->va = addr;
            new->pa = s->pseg->pa+(addr-s->base);
            new->ref = 1;
            *pte = new;
        }

        if (checkaddr && addr == addr2check)
            (*checkaddr)(addr, s, *pte);
        mmupte = PPN((*pte)->pa) |PTEWRITE|PTEUNCACHED|PTEVALID;
        (*pte)->modref = PG_MOD|PG_REF;
        break;
    /*e: [[fixfault()]] SG_PHYSICAL case */

    default:
        panic("fault");
        break;
    }
    qunlock(&s->lk);

    if(doputmmu)
        arch_putmmu(addr, mmupte, *pte);

    return 0; // OK
}
/*e: function fixfault */

/*s: function pio */
void
pio(Segment *s, virt_addr addr, ulong soff, PageOrSwap **p)
{
    Page *new;
    Arch_KMap *k;
    Chan *c;
    int n, ask;
    char *kaddr;
    ulong daddr; // disk address
    Page *loadrec;

retry:
    loadrec = *p;

    if(loadrec == nil) {  /* from a text/data image */
        daddr = s->fstart+soff;
        new = lookpage(s->image, daddr);
        if(new != nil) {
            *p = new;
            return;
        }

        c = s->image->c;
        ask = s->flen-soff;
        if(ask > BY2PG)
            ask = BY2PG;
    }else{          /* from a swap image */
        daddr = swapaddr(loadrec);
        new = lookpage(&swapimage, daddr);
        if(new != nil) {
            putswap(loadrec);
            *p = new;
            return;
        }
        c = swapimage.c;
        ask = BY2PG;
    }
    qunlock(&s->lk);

    new = newpage(false, nil, addr);
    k = arch_kmap(new);
    kaddr = (char*)VA(k);

    while(waserror()) {
        if(strcmp(up->errstr, Eintr) == 0)
            continue;
        arch_kunmap(k);
        putpage(new);
        faulterror(Eioload, c, false);
    }

    // reading the Page!! slow! which is why it's done without s->lk locked
    n = devtab[c->type]->read(c, kaddr, ask, daddr);

    if(n != ask)
        faulterror(Eioload, c, false);
    if(ask < BY2PG)
        memset(kaddr+ask, 0, BY2PG-ask);

    poperror();
    arch_kunmap(k);
    qlock(&s->lk);

    if(loadrec == nil) {  /* This is demand load */
        /*
         *  race, another proc may have gotten here first while
         *  s->lk was unlocked
         */
        if(*p == nil) { 
            new->daddr = daddr;
            cachepage(new, s->image);
            *p = new;
        }
        else
            putpage(new);
    }else{          /* This is paged out */
        /*
         *  race, another proc may have gotten here first
         *  (and the pager may have run on that page) while
         *  s->lk was unlocked
         */
        if(*p != loadrec){
            if(!pagedout(*p)){
                /* another process did it for me */
                putpage(new);
    goto done; // not return, need cachectl stuff??
            } else {
                /* another process and the pager got in */
                putpage(new);
                goto retry;
            }
        }

        new->daddr = daddr;
        cachepage(new, &swapimage);
        *p = new;
        putswap(loadrec);
    }
done:
 if(s->flushme)
  memset((*p)->cachectl, PG_TXTFLUSH, sizeof((*p)->cachectl));
}
/*e: function pio */

/*s: function okaddr */
/*
 * Called only in a system call
 */
bool
okaddr(virt_addr addr, ulong len, bool write)
{
    Segment *s;

    if((long)len >= 0) {
        for(;;) {
            s = seg(up, addr, false);
            if(s == nil || (write && (s->type&SG_RONLY)))
                break;

            if(addr+len > s->top) {
                len -= s->top - addr;
                addr = s->top;
            }else{
                return true;
            }
        }
    }
    pprint("suicide: invalid address %#lux/%lud in sys call pc=%#lux\n", addr, len, arch_userpc());
    return false;
}
/*e: function okaddr */

/*s: function validaddr */
void
validaddr(virt_addr addr, ulong len, bool write)
{
    if(!okaddr(addr, len, write)){
        postnote(up, 1, "sys: bad address in syscall", NDebug);
        error(Ebadarg);
    }
}
/*e: function validaddr */

/*s: function vmemchr */
/*
 * &s[0] is known to be a valid address.
 */
void*
vmemchr(virt_addr3 s, int c, int n)
{
    int m;
    virt_addr a;
    virt_addr3 t;

    a = (virt_addr)s;
    while(PGROUND(a) != PGROUND(a+n-1)){
        /* spans pages; handle this page */
        m = BY2PG - (a & (BY2PG-1));
        t = memchr((void*)a, c, m);
        if(t)
            return t;
        a += m;
        n -= m;
        if(a < KZERO)
            validaddr(a, 1, false);
    }

    /* fits in one page */
    return memchr((void*)a, c, n);
}
/*e: function vmemchr */

/*s: function seg */
Segment*
seg(Proc *p, virt_addr addr, bool dolock)
{
    Segment **s, **et, *sg;

    et = &p->seg[NSEG];
    for(s = p->seg; s < et; s++) {
        sg = *s;
        if(sg == nil)
            continue;
        if(addr >= sg->base && addr < sg->top) {
            if(dolock == false)
                return sg;

            qlock(&sg->lk);
            // can have a race, need to check again
            if(addr >= sg->base && addr < sg->top)
                return sg;
            qunlock(&sg->lk);
        }
    }
    return nil;
}
/*e: function seg */

/*s: function checkpages */
void
checkpages(void)
{
    int checked;
    ulong addr, off;
    Pagetable *p;
    Page *pg;
    Segment **sp, **ep, *s;
    
    if(up == nil)
        return;

    checked = 0;
    // foreach(up->seg)
    for(sp=up->seg, ep=&up->seg[NSEG]; sp<ep; sp++){
        s = *sp;
        if(s == nil)
            continue;
        qlock(&s->lk);
        for(addr=s->base; addr<s->top; addr+=BY2PG){
            off = addr - s->base;
            p = s->pagedir[off/PAGETABMAPMEM];
            if(p == nil)
                continue;
            pg = p->pagetab[(off&(PAGETABMAPMEM-1))/BY2PG];
            if(pg == nil || pagedout(pg))
                continue;
            arch_checkmmu(addr, pg->pa);
            checked++;
        }
        qunlock(&s->lk);
    }
    print("%ld %s: checked %d page table entries\n", up->pid, up->text, checked);
}
/*e: function checkpages */
/*e: fault.c */
