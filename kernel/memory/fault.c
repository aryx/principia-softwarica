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
    spllo();

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
    Pagetable **p, *etp;
    phys_addr mmuphys=nilptr;
    ulong soff;
    Page **pg, *lkp, *new;

    addr &= ~(BY2PG-1);
    soff = addr - s->base;
    p = &s->pagedir[soff/PAGETABMAPMEM];
    if(*p == 0)
        *p = ptalloc();

    etp = *p;
    pg = &etp->pagetab[(soff&(PAGETABMAPMEM-1))/BY2PG];
    type = s->type&SG_TYPE;

    if(pg < etp->first)
        etp->first = pg;
    if(pg > etp->last)
        etp->last = pg;

    switch(type) {
    default:
        panic("fault");
        break;

    case SG_TEXT:           /* Demand load */
        if(pagedout(*pg))
            pio(s, addr, soff, pg);

        mmuphys = PPN((*pg)->pa) | PTERONLY|PTEVALID;
        (*pg)->modref = PG_REF;
        break;

    case SG_BSS:
    case SG_SHARED:         /* Zero fill on demand */
    case SG_STACK:
        if(*pg == 0) {
            new = newpage(1, &s, addr);
            if(s == nil) //?? when can be nil at exit?
                return -1;
            *pg = new;
        }
        goto common;

    case SG_DATA:
    common:         /* Demand load/pagein/copy on write */
        if(pagedout(*pg))
            pio(s, addr, soff, pg);

        /*
         *  It's only possible to copy on write if
         *  we're the only user of the segment.
         */
        if(read && conf.copymode == false && s->ref == 1) {
            mmuphys = PPN((*pg)->pa)|PTERONLY|PTEVALID;
            (*pg)->modref |= PG_REF;
            break;
        }

        lkp = *pg;
        lock(lkp);

        if(lkp->image == &swapimage)
            ref = lkp->ref + swapcount(lkp->daddr);
        else
            ref = lkp->ref;
        if(ref == 1 && lkp->image){
            /* save a copy of the original for the image cache */
            duppage(lkp);
            ref = lkp->ref;
        }
        unlock(lkp);
        if(ref > 1){
            new = newpage(0, &s, addr);
            if(s == nil)
                return -1;
            *pg = new;
            copypage(lkp, *pg);
            putpage(lkp);
        }
        mmuphys = PPN((*pg)->pa) | PTEWRITE | PTEVALID;
        (*pg)->modref = PG_MOD|PG_REF;
        break;

    /*s: [[fixfault()]] SG_PHYSICAL case */
        case SG_PHYSICAL:
            if(*pg == nil) {
                new = smalloc(sizeof(Page));
                new->va = addr;
                new->pa = s->pseg->pa+(addr-s->base);
                new->ref = 1;
                *pg = new;
            }

            if (checkaddr && addr == addr2check)
                (*checkaddr)(addr, s, *pg);
            mmuphys = PPN((*pg)->pa) |PTEWRITE|PTEUNCACHED|PTEVALID;
            (*pg)->modref = PG_MOD|PG_REF;
            break;
    /*e: [[fixfault()]] SG_PHYSICAL case */
    }
    qunlock(&s->lk);

    if(doputmmu)
        putmmu(addr, mmuphys, *pg);

    return 0; // OK
}
/*e: function fixfault */

/*s: function pio */
void
pio(Segment *s, virt_addr addr, ulong soff, PageOrSwap **p)
{
    Page *new;
    KMap *k;
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
    k = kmap(new);
    kaddr = (char*)VA(k);

    while(waserror()) {
        if(strcmp(up->errstr, Eintr) == 0)
            continue;
        kunmap(k);
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
    kunmap(k);
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
                return;
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
    pprint("suicide: invalid address %#lux/%lud in sys call pc=%#lux\n", addr, len, userpc());
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
            checkmmu(addr, pg->pa);
            checked++;
        }
        qunlock(&s->lk);
    }
    print("%ld %s: checked %d page table entries\n", up->pid, up->text, checked);
}
/*e: function checkpages */
/*e: fault.c */
