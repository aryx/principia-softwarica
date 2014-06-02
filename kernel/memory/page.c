/*s: page.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

/*s: page.c forward decl */
int ispages(void*);
void portcountpagerefs(ulong*, int);
/*e: page.c forward decl */

//*****************************************************************************
// Initialization
//*****************************************************************************

/*s: function pageinit */
void
pageinit(void)
{
    int color, i, j;
    Page *p;
    Pallocmem *pm;
    ulong m, np, k, vkb, pkb;

    np = 0;
    for(i=0; i<nelem(palloc.mem); i++){
        pm = &palloc.mem[i];
        np += pm->npage;
    }
    palloc.pages = xalloc(np*sizeof(Page));
    if(palloc.pages == 0)
        panic("pageinit");

    color = 0;
    palloc.head = palloc.pages;
    p = palloc.head;
    for(i=0; i<nelem(palloc.mem); i++){
        pm = &palloc.mem[i];
        for(j=0; j<pm->npage; j++){
            p->prev = p-1;
            p->next = p+1;
            p->pa = pm->base+j*BY2PG;
            p->color = color;
            palloc.freecount++;
            color = (color+1)%NCOLOR;
            p++;
        }
    }
    palloc.tail = p - 1;
    palloc.head->prev = 0;
    palloc.tail->next = 0;

    palloc.user = p - palloc.pages; // TODO? should be np too no?
    pkb = palloc.user*BY2PG/1024;
    vkb = pkb + (conf.nswap*BY2PG)/1024;

    /* Paging numbers */
    swapalloc.highwater = (palloc.user*5)/100;
    swapalloc.headroom = swapalloc.highwater + (swapalloc.highwater/4);

    m = 0;
    for(i=0; i<nelem(conf.mem); i++)
        if(conf.mem[i].npage)
            m += conf.mem[i].npage*BY2PG;
    k = PGROUND(end - (char*)KTZERO);
    print("%ldM memory: ", (m+k+1024*1024-1)/(1024*1024));
    print("%ldM kernel data, ", (m+k-pkb*1024+1024*1024-1)/(1024*1024));
    print("%ldM user, ", pkb/1024);
    print("%ldM swap\n", vkb/1024);
}
/*e: function pageinit */

//*****************************************************************************
// Functions
//*****************************************************************************

/*s: function pageunchain */
// assumes palloc is held
static void
pageunchain(Page *p)
{
    if(canlock(&palloc))
        panic("pageunchain (palloc %p)", &palloc);

    // remove(p, palloc);
    if(p->prev)
        p->prev->next = p->next;
    else
        palloc.head = p->next;
    if(p->next)
        p->next->prev = p->prev;
    else
        palloc.tail = p->prev;
    p->prev = p->next = nil;
    palloc.freecount--;
}
/*e: function pageunchain */

/*s: function pagechaintail */
// assumes palloc is held
void
pagechaintail(Page *p)
{
    if(canlock(&palloc))
        panic("pagechaintail");

    // add_tail(p, palloc)
    if(palloc.tail) {
        p->prev = palloc.tail;
        palloc.tail->next = p;
    }
    else {
        palloc.head = p;
        p->prev = 0;
    }
    palloc.tail = p;
    p->next = nil;
    palloc.freecount++;
}
/*e: function pagechaintail */

/*s: function pagechainhead */
// assumes palloc is held
void
pagechainhead(Page *p)
{
    if(canlock(&palloc))
        panic("pagechainhead");
    // add_head(p, palloc)
    if(palloc.head) {
        p->next = palloc.head;
        palloc.head->prev = p;
    }
    else {
        palloc.tail = p;
        p->next = nil;
    }
    palloc.head = p;
    p->prev = nil;
    palloc.freecount++;
}
/*e: function pagechainhead */

/*s: constructor newpage */
Page*
newpage(bool clear, Segment **s, virt_addr va)
{
    Page *p;
    KMap *k;
    uchar ct;
    int i, hw, color;
    bool dontalloc;

    lock(&palloc);
    color = getpgcolor(va);
    hw = swapalloc.highwater;
    for(;;) {
        if(palloc.freecount > hw)
            break;
        if(up->kp && palloc.freecount > 0)
            break;

        unlock(&palloc);
        dontalloc = false;
        if(s && *s) {
            qunlock(&((*s)->lk));
            *s = nil;// !!
            dontalloc = true;
        }
        qlock(&palloc.pwait);   /* Hold memory requesters here */

        while(waserror())   /* Ignore interrupts */
            ;

        kickpager();
        tsleep(&palloc.freememr, ispages, 0, 1000);

        poperror();

        qunlock(&palloc.pwait);

        /*
         * If called from fault and we lost the segment from
         * underneath don't waste time allocating and freeing
         * a page. Fault will call newpage again when it has
         * reacquired the segment locks
         */
        if(dontalloc)
            return nil;

        lock(&palloc);
    }

    /* First try for our colour */
    for(p = palloc.head; p; p = p->next)
        if(p->color == color)
            break;

    ct = PG_NOFLUSH;
    if(p == nil) {
        p = palloc.head;
        p->color = color;
        ct = PG_NEWCOL;
    }

    pageunchain(p);

    lock(p);
    if(p->ref != 0)
        panic("newpage: p->ref %d != 0", p->ref);

    uncachepage(p);
    p->ref++;
    p->va = va;
    p->modref = 0;
    for(i = 0; i < MAXCPUS; i++)
        p->cachectl[i] = ct;
    unlock(p);
    unlock(&palloc);

    if(clear) {
        k = kmap(p);
        memset((void*)VA(k), 0, BY2PG);
        kunmap(k);
    }

    return p;
}
/*e: constructor newpage */

/*s: function ispages */
int
ispages(void*)
{
    return palloc.freecount >= swapalloc.highwater;
}
/*e: function ispages */

/*s: destructor putpage */
void
putpage(Page *p)
{
    if(onswap(p)) {
        putswap(p);
        return;
    }

    lock(&palloc);
    lock(p);

    if(p->ref == 0)
        panic("putpage");

    if(--p->ref > 0) {
        unlock(p);
        unlock(&palloc);
        return;
    }

    if(p->image && p->image != &swapimage)
        pagechaintail(p);
    else 
        pagechainhead(p);

    if(palloc.freememr.p != nil)
        wakeup(&palloc.freememr);

    unlock(p);
    unlock(&palloc);
}
/*e: destructor putpage */

/*s: function auxpage */
Page*
auxpage(void)
{
    Page *p;

    lock(&palloc);
    p = palloc.head;
    if(palloc.freecount < swapalloc.highwater) {
        unlock(&palloc);
        return nil;
    }
    pageunchain(p);

    lock(p);
    if(p->ref != 0)
        panic("auxpage");
    p->ref++;
    uncachepage(p);
    unlock(p);
    unlock(&palloc);

    return p;
}
/*e: function auxpage */

/*s: global dupretries */
static int dupretries = 15000;
/*e: global dupretries */

/*s: function duppage */
/* Always call with p locked */
int
duppage(Page *p)
{
    Page *np;
    int color;
    int retries;

    retries = 0;
retry:

    if(retries++ > dupretries){
        print("duppage %d, up %p\n", retries, up);
        dupretries += 100;
        if(dupretries > 100000)
            panic("duppage\n");
        uncachepage(p);
        return 1;
    }
        

    /* don't dup pages with no image */
    if(p->ref == 0 || p->image == nil || p->image->notext)
        return 0;

    /*
     *  normal lock ordering is to call
     *  lock(&palloc) before lock(p).
     *  To avoid deadlock, we have to drop
     *  our locks and try again.
     */
    if(!canlock(&palloc)){
        unlock(p);
        if(up)
            sched();
        lock(p);
        goto retry;
    }

    /* No freelist cache when memory is very low */
    if(palloc.freecount < swapalloc.highwater) {
        unlock(&palloc);
        uncachepage(p);
        return 1;
    }

    color = getpgcolor(p->va);
    for(np = palloc.head; np; np = np->next)
        if(np->color == color)
            break;

    /* No page of the correct color */
    if(np == nil) {
        unlock(&palloc);
        uncachepage(p);
        return 1;
    }

    pageunchain(np);
    pagechaintail(np);
/*
* XXX - here's a bug? - np is on the freelist but it's not really free.
* when we unlock palloc someone else can come in, decide to
* use np, and then try to lock it.  they succeed after we've 
* run copypage and cachepage and unlock(np).  then what?
* they call pageunchain before locking(np), so it's removed
* from the freelist, but still in the cache because of
* cachepage below.  if someone else looks in the cache
* before they remove it, the page will have a nonzero ref
* once they finally lock(np).
*/
    lock(np);
    unlock(&palloc);

    /* Cache the new version */
    uncachepage(np);
    np->va = p->va;
    np->daddr = p->daddr;
    copypage(p, np);
    cachepage(np, p->image);
    unlock(np);
    uncachepage(p);

    return 0;
}
/*e: function duppage */

/*s: function copypage */
void
copypage(Page *f, Page *t)
{
    KMap *ks, *kd;

    ks = kmap(f);
    kd = kmap(t);
    memmove((void*)VA(kd), (void*)VA(ks), BY2PG);
    kunmap(ks);
    kunmap(kd);
}
/*e: function copypage */

/*s: function uncachepage */
/* Always called with a locked page */
void
uncachepage(Page *p)
{
    Page **l, *f;

    if(p->image == nil)
        return;

    lock(&palloc.hashlock);
    // remove_hash(palloc.hash, p->daddr, p)
    l = &pghash(p->daddr);
    for(f = *l; f; f = f->hash) {
        if(f == p) {
            *l = p->hash;
            break;
        }
        l = &f->hash;
    }
    unlock(&palloc.hashlock);
    putimage(p->image);
    p->image = nil;
    p->daddr = 0;
}
/*e: function uncachepage */

/*s: function cachepage */
void
cachepage(Page *p, KImage *i)
{
    Page **l;

    /* If this ever happens it should be fixed by calling
     * uncachepage instead of panic. I think there is a race
     * with pio in which this can happen. Calling uncachepage is
     * correct - I just wanted to see if we got here.
     */
    if(p->image)
        panic("cachepage");

    incref(i);
    lock(&palloc.hashlock);
    p->image = i;
    // add_hash(palloc.hash, p->daddr, p)
    l = &pghash(p->daddr);
    p->hash = *l;
    *l = p;
    unlock(&palloc.hashlock);
}
/*e: function cachepage */

/*s: function cachedel */
void
cachedel(KImage *i, ulong daddr)
{
    Page *f, **l;

    lock(&palloc.hashlock);
    l = &pghash(daddr);
    for(f = *l; f; f = f->hash) {
        if(f->image == i && f->daddr == daddr) {
            lock(f);
            // can have a race? things could have changed, so rested under lock
            if(f->image == i && f->daddr == daddr){
                *l = f->hash;
                putimage(f->image); // =~ decref
                f->image = nil;
                f->daddr = 0;
            }
            unlock(f);
            break;
        }
        l = &f->hash;
    }
    unlock(&palloc.hashlock);
}
/*e: function cachedel */

/*s: function lookpage */
Page *
lookpage(KImage *i, ulong daddr)
{
    Page *f;

    lock(&palloc.hashlock);
    for(f = pghash(daddr); f; f = f->hash) {
        if(f->image == i && f->daddr == daddr) {
            unlock(&palloc.hashlock);

            lock(&palloc);
            lock(f);
            if(f->image != i || f->daddr != daddr) {
                unlock(f);
                unlock(&palloc);
                return nil;
            }
            if(++f->ref == 1)
                pageunchain(f);
            unlock(&palloc);
            unlock(f);

            return f;
        }
    }
    unlock(&palloc.hashlock);

    return nil;
}
/*e: function lookpage */

/*s: function ptecpy */
Pagetable*
ptecpy(Pagetable *old)
{
    Pagetable *new;
    Page **src, **dst;

    new = ptealloc();
    dst = &new->pages[old->first-old->pages];
    new->first = dst;
    for(src = old->first; src <= old->last; src++, dst++)
        if(*src) {
            if(onswap(*src))
                dupswap(*src);
            else {
                lock(*src);
                (*src)->ref++;
                unlock(*src);
            }
            new->last = dst;
            *dst = *src;
        }

    return new;
}
/*e: function ptecpy */

/*s: constructor ptealloc */
Pagetable*
ptealloc(void)
{
    Pagetable *new;

    new = smalloc(sizeof(Pagetable));
    new->first = &new->pages[PTEPERTAB];
    new->last = new->pages;
    return new;
}
/*e: constructor ptealloc */

/*s: destructor freepte */
void
freepte(Segment *s, Pagetable *p)
{
    int ref;
    void (*fn)(Page*);
    Page *pt, **pg, **ptop;

    switch(s->type&SG_TYPE) {
    case SG_PHYSICAL:
        fn = s->pseg->pgfree;
        ptop = &p->pages[PTEPERTAB];
        if(fn) {
            for(pg = p->pages; pg < ptop; pg++) {
                if(*pg == 0)
                    continue;
                (*fn)(*pg);
                *pg = 0;
            }
            break;
        }
        for(pg = p->pages; pg < ptop; pg++) {
            pt = *pg;
            if(pt == 0)
                continue;
            lock(pt);
            ref = --pt->ref;
            unlock(pt);
            if(ref == 0)
                free(pt);
        }
        break;
    default:
        for(pg = p->first; pg <= p->last; pg++)
            if(*pg) {
                putpage(*pg);
                *pg = 0;
            }
    }
    free(p);
}
/*e: destructor freepte */

/*s: function pagenumber */
ulong
pagenumber(Page *p)
{
    return p-palloc.pages;
}
/*e: function pagenumber */

/*s: function checkpagerefs */
void
checkpagerefs(void)
{
    int s;
    ulong i, np, nwrong;
    ulong *ref;
    
    np = palloc.user;
    ref = malloc(np*sizeof ref[0]);
    if(ref == nil){
        print("checkpagerefs: out of memory\n");
        return;
    }
    
    /*
     * This may not be exact if there are other processes
     * holding refs to pages on their stacks.  The hope is
     * that if you run it on a quiescent system it will still
     * be useful.
     */
    s = splhi();
    lock(&palloc);
    countpagerefs(ref, 0);
    portcountpagerefs(ref, 0);
    nwrong = 0;
    for(i=0; i<np; i++){
        if(palloc.pages[i].ref != ref[i]){
            iprint("page %#.8lux ref %d actual %lud\n", 
                palloc.pages[i].pa, palloc.pages[i].ref, ref[i]);
            ref[i] = 1;
            nwrong++;
        }else
            ref[i] = 0;
    }
    countpagerefs(ref, 1);
    portcountpagerefs(ref, 1);
    iprint("%lud mistakes found\n", nwrong);
    unlock(&palloc);
    splx(s);
}
/*e: function checkpagerefs */

/*s: function portcountpagerefs */
void
portcountpagerefs(ulong *ref, int print)
{
    ulong i, j, k, ns, n;
    Page **pg, *entry;
    Proc *p;
    Pagetable *pte;
    Segment *s;

    /*
     * Pages in segments.  s->mark avoids double-counting.
     */
    n = 0;
    ns = 0;
    for(i=0; i<conf.nproc; i++){
        p = proctab(i);
        for(j=0; j<NSEG; j++){
            s = p->seg[j];
            if(s)
                s->mark = 0;
        }
    }
    for(i=0; i<conf.nproc; i++){
        p = proctab(i);
        for(j=0; j<NSEG; j++){
            s = p->seg[j];
            if(s == nil || s->mark++)
                continue;
            ns++;
            for(k=0; k<s->mapsize; k++){
                pte = s->map[k];
                if(pte == nil)
                    continue;
                for(pg = pte->first; pg <= pte->last; pg++){
                    entry = *pg;
                    if(pagedout(entry))
                        continue;
                    if(print){
                        if(ref[pagenumber(entry)])
                            iprint("page %#.8lux in segment %#p\n", entry->pa, s);
                        continue;
                    }
                    if(ref[pagenumber(entry)]++ == 0)
                        n++;
                }
            }
        }
    }
    if(!print){
        iprint("%lud pages in %lud segments\n", n, ns);
        for(i=0; i<conf.nproc; i++){
            p = proctab(i);
            for(j=0; j<NSEG; j++){
                s = p->seg[j];
                if(s == nil)
                    continue;
                if(s->ref != s->mark){
                    iprint("segment %#p (used by proc %lud pid %lud) has bad ref count %lud actual %lud\n",
                        s, i, p->pid, s->ref, s->mark);
                }
            }
        }
    }
}
/*e: function portcountpagerefs */

/*e: page.c */
