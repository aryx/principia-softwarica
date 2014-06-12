/*s: segment.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */
#include "io.h"

//*****************************************************************************
// Global
//*****************************************************************************

/*s: global imagealloc */
static struct Imagealloc imagealloc;
/*e: global imagealloc */

//*****************************************************************************
// Misc
//*****************************************************************************

/*s: global physseg */
/*
 * Attachable segment types
 */
static Physseg physseg[10] = {
    { .attr= SG_SHARED, 
      .name= "shared", 
      .pa= 0,
      .size= SEGMAXSIZE, 
    },
    { .attr= SG_BSS, 
      .name= "memory", 
      .pa = 0,
      .size = SEGMAXSIZE, 
    },
    { 0, 0, 0, 0},
};
/*e: global physseg */
/*s: global physseglock */
static Lock physseglock;
/*e: global physseglock */

/*s: hook _globalsegattach */
Segment* (*_globalsegattach)(Proc*, char*);
/*e: hook _globalsegattach */

/*s: segment.c forward decl */
static void imagereclaim(void);
static void imagechanreclaim(void);
/*e: segment.c forward decl */

//*****************************************************************************
// Initialization
//*****************************************************************************

/*s: function initimage */
void
initimage(void)
{
    KImage *i, *ie;

    imagealloc.free = xalloc(conf.nimage*sizeof(KImage));
    if (imagealloc.free == nil)
        panic("initimage: no memory");
    ie = &imagealloc.free[conf.nimage-1];
    for(i = imagealloc.free; i < ie; i++)
        i->next = i+1;
    i->next = nil;
    imagealloc.freechan = malloc(NFREECHAN * sizeof(Chan*));
    imagealloc.szfreechan = NFREECHAN;
}
/*e: function initimage */

//*****************************************************************************
// Functions
//*****************************************************************************

/*s: constructor newseg */
Segment *
newseg(int type, virt_addr base, ulong size)
{
    Segment *s;
    int pagedirsize;

    if(size > (PAGEDIRSIZE*PAGETABSIZE))
        error(Enovmem);

    s = smalloc(sizeof(Segment));
    s->ref = 1;
    s->type = type;
    s->base = base;
    s->top = base+(size*BY2PG);
    s->size = size;

    /*s: [[newseg()]] sema initialization */
        // no list, just one sema
        s->sema.prev = &s->sema;
        s->sema.next = &s->sema;
    /*e: [[newseg()]] sema initialization */

    pagedirsize = ROUND(size, PAGETABSIZE)/PAGETABSIZE;
    if(pagedirsize > nelem(s->smallpagedir)){
        pagedirsize *= 2;
        if(pagedirsize > PAGEDIRSIZE)
            pagedirsize = PAGEDIRSIZE; // pad's first bugfix :)
        s->pagedir = smalloc(pagedirsize*sizeof(Pagetable*));
        s->pagedirsize = pagedirsize;
    }else{
        s->pagedir = s->smallpagedir;
        s->pagedirsize = nelem(s->smallpagedir);
    }
    return s;
}
/*e: constructor newseg */

/*s: destructor putseg */
void
putseg(Segment *s)
{
    Pagetable **pp, **emap;
    KImage *img;

    if(s == nil)
        return; // TODO: panic("putseg") instead?

    img = s->image;
    if(img != nil) {
        lock(img);
        lock(s);
        if(img->s == s && s->ref == 1)
            img->s = nil;
        unlock(img);
    }
    else
        lock(s);


    s->ref--;
    if(s->ref != 0) {
        unlock(s);
        return;
    }
    unlock(s);

    qlock(&s->lk);
    if(img)
        putimage(img);

    emap = &s->pagedir[s->pagedirsize];
    for(pp = s->pagedir; pp < emap; pp++)
        if(*pp)
            freept(s, *pp);

    qunlock(&s->lk);
    if(s->pagedir != s->smallpagedir)
        free(s->pagedir);
    /*s: [[putseg()]] free profile */
        if(s->profile != nil)
            free(s->profile);
    /*e: [[putseg()]] free profile */
    free(s);
}
/*e: destructor putseg */

/*s: function relocateseg */
void
relocateseg(Segment *s, ulong offset)
{
    Page **pg, *x;
    Pagetable *pt, **p, **endpt;

    endpt = &s->pagedir[s->pagedirsize];
    for(p = s->pagedir; p < endpt; p++) {
        if(*p == nil)
            continue;
        pt = *p;
        for(pg = pt->first; pg <= pt->last; pg++) {
            if(x = *pg)
                x->va += offset;
        }
    }
}
/*e: function relocateseg */

/*s: function dupseg */
Segment*
dupseg(Segment **seg, int segno, bool share)
{
    int i, size;
    Pagetable *pt;
    Segment *n, *s;

    SET(n); //????

    s = seg[segno];

    qlock(&s->lk);
    if(waserror()){
        qunlock(&s->lk);
        nexterror();
    }
    switch(s->type&SG_TYPE) {
    case SG_TEXT:       /* New segment shares pt set */
        goto sameseg;

    case SG_DATA:       /* Copy on write plus demand load info */

        if(segno == TSEG){ // why not SG_TEXT then?
            poperror();
            qunlock(&s->lk);
            return data2txt(s);// ????
        }

        if(share)
            goto sameseg; // threads! clone()

        n = newseg(s->type, s->base, s->size);

        incref(s->image); // how sure non nil? data always attached to an img
        n->image = s->image;
        n->fstart = s->fstart;
        n->flen = s->flen;
        break;

    case SG_BSS:        /* Just copy on write */
        if(share)
            goto sameseg; // threads! clone()

        n = newseg(s->type, s->base, s->size);
        break;

    case SG_STACK:
        n = newseg(s->type, s->base, s->size);
        break;

    case SG_SHARED:
    case SG_PHYSICAL:
        goto sameseg;

    }
    // not sameseg, we have allocated a new seg in n above

    size = s->pagedirsize;
    for(i = 0; i < size; i++)
        if(pt = s->pagedir[i])
            n->pagedir[i] = ptcpy(pt); // will actually share the pages

    if(s->ref > 1)
        procflushseg(s); // ??
    poperror();
    qunlock(&s->lk);
    return n;

sameseg:
    incref(s);
    poperror();
    qunlock(&s->lk);
    return s;
}
/*e: function dupseg */

/*s: function segpage */
void
segpage(Segment *s, Page *p)
{
    Pagetable **pt;
    ulong off;
    Page **pg;

    if(p->va < s->base || p->va >= s->top)
        panic("segpage");

    off = p->va - s->base;
    pt = &s->pagedir[off/PAGETABMAPMEM]; // PDX
    if(*pt == nil)
        *pt = ptalloc();

    pg = &(*pt)->pagetab[(off&(PAGETABMAPMEM-1))/BY2PG]; // PTX
    *pg = p;

    if(pg < (*pt)->first)
        (*pt)->first = pg;
    if(pg > (*pt)->last)
        (*pt)->last = pg;
}
/*e: function segpage */

/*s: function attachimage */
KImage*
attachimage(int type, Chan *c, ulong base, ulong len)
{
    KImage *i, **l;

    /* reclaim any free channels from reclaimed segments */
    if(imagealloc.nfreechan)
        imagechanreclaim();

    lock(&imagealloc);

    /*
     * Search the image cache for remains of the text from a previous
     * or currently running incarnation
     */
    for(i = ihash(c->qid.path); i; i = i->hash) {
        if(c->qid.path == i->qid.path) {
            lock(i);
            if(eqqid(c->qid, i->qid) &&
               eqqid(c->mqid, i->mqid) &&
               c->mchan == i->mchan &&
               c->type == i->type) {
                goto found;
            }
            unlock(i);
        }
    }

    /*
     * imagereclaim dumps pages from the free list which are cached by image
     * structures. This should free some image structures.
     */
    while(!(i = imagealloc.free)) {
        unlock(&imagealloc);
        imagereclaim();
        sched();
        lock(&imagealloc);
    }

    imagealloc.free = i->next;

    lock(i);
    incref(c);
    i->c = c;
    i->type = c->type;
    i->qid = c->qid;
    i->mqid = c->mqid;
    i->mchan = c->mchan;
    l = &ihash(c->qid.path);
    i->hash = *l;
    *l = i;
found:
    unlock(&imagealloc);

    if(i->s == 0) {
        /* Disaster after commit in exec */
        if(waserror()) {
            unlock(i);
            pexit(Enovmem, true);
        }
        i->s = newseg(type, base, len);
        i->s->image = i;
        i->ref++;
        poperror();
    }
    else
        incref(i->s);

    return i;
}
/*e: function attachimage */

/*s: struct Irstats */
struct Irstats {
    int calls;          /* times imagereclaim was called */
    int loops;          /* times the main loop was run */
    uvlong  ticks;          /* total time in the main loop */
    uvlong  maxt;           /* longest time in main loop */
};
/*e: struct Irstats */
/*s: segment.c global irstats */
static struct Irstats  irstats;
/*e: segment.c global irstats */

/*s: function imagereclaim */
static void
imagereclaim(void)
{
    int n;
    Page *p;
    uvlong ticks;

    irstats.calls++;
    /* Somebody is already cleaning the page cache */
    if(!canqlock(&imagealloc.ireclaim))
        return;

    lock(&palloc);
    ticks = fastticks(nil);
    n = 0;
    /*
     * All the pages with images backing them are at the
     * end of the list (see putpage) so start there and work
     * backward.
     */
    for(p = palloc.tail; p && p->image && n<1000; p = p->prev) {
        if(p->ref == 0 && canlock(p)) {
            if(p->ref == 0) {
                n++;
                uncachepage(p);
            }
            unlock(p);
        }
    }
    ticks = fastticks(nil) - ticks;
    unlock(&palloc);
    irstats.loops++;
    irstats.ticks += ticks;
    if(ticks > irstats.maxt)
        irstats.maxt = ticks;
    //print("T%llud+", ticks);
    qunlock(&imagealloc.ireclaim);
}
/*e: function imagereclaim */

/*s: function imagechanreclaim */
/*
 *  since close can block, this has to be called outside of
 *  spin locks.
 */
static void
imagechanreclaim(void)
{
    Chan *c;

    /* Somebody is already cleaning the image chans */
    if(!canqlock(&imagealloc.fcreclaim))
        return;

    /*
     * We don't have to recheck that nfreechan > 0 after we
     * acquire the lock, because we're the only ones who decrement 
     * it (the other lock contender increments it), and there's only
     * one of us thanks to the qlock above.
     */
    while(imagealloc.nfreechan > 0){
        lock(&imagealloc);
        imagealloc.nfreechan--;
        c = imagealloc.freechan[imagealloc.nfreechan];
        unlock(&imagealloc);
        cclose(c);
    }

    qunlock(&imagealloc.fcreclaim);
}
/*e: function imagechanreclaim */

/*s: destructor putimage */
void
putimage(KImage *i)
{
    Chan *c, **cp;
    KImage *f, **l;

    if(i->notext)
        return;

    lock(i);
    if(--i->ref == 0) {
        l = &ihash(i->qid.path);
        mkqid(&i->qid, ~0, ~0, QTFILE);
        unlock(i);
        c = i->c;

        lock(&imagealloc);
        for(f = *l; f; f = f->hash) {
            if(f == i) {
                *l = i->hash;
                break;
            }
            l = &f->hash;
        }

        i->next = imagealloc.free;
        imagealloc.free = i;

        /* defer freeing channel till we're out of spin lock's */
        if(imagealloc.nfreechan == imagealloc.szfreechan){
            imagealloc.szfreechan += NFREECHAN;
            cp = malloc(imagealloc.szfreechan*sizeof(Chan*));
            if(cp == nil)
                panic("putimage");
            memmove(cp, imagealloc.freechan, imagealloc.nfreechan*sizeof(Chan*));
            free(imagealloc.freechan);
            imagealloc.freechan = cp;
        }
        imagealloc.freechan[imagealloc.nfreechan++] = c;
        unlock(&imagealloc);

        return;
    }
    unlock(i);
}
/*e: destructor putimage */

/*s: function ibrk */
long
ibrk(ulong addr, int seg)
{
    Segment *s, *ns;
    ulong newtop, newsize;
    int i, mapsize;
    Pagetable **map;

    s = up->seg[seg];
    if(s == nil)
        error(Ebadarg);

    if(addr == nilptr)
        return s->base;

    qlock(&s->lk);

    /* We may start with the bss overlapping the data */
    if(addr < s->base) {
        if(seg != BSEG || up->seg[DSEG] == nil || addr < up->seg[DSEG]->base) {
            qunlock(&s->lk);
            error(Enovmem);
        }
        addr = s->base;
    }

    newtop = PGROUND(addr);
    newsize = (newtop-s->base)/BY2PG;
    if(newtop < s->top) {
        /*
         * do not shrink a segment shared with other procs, as the
         * to-be-freed address space may have been passed to the kernel
         * already by another proc and is past the validaddr stage.
         */
        if(s->ref > 1){
            qunlock(&s->lk);
            error(Einuse);
        }
        mfreeseg(s, newtop, (s->top-newtop)/BY2PG);
        s->top = newtop;
        s->size = newsize;
        qunlock(&s->lk);
        flushmmu();
        return 0;
    }

    for(i = 0; i < NSEG; i++) {
        ns = up->seg[i];
        if(ns == nil || ns == s)
            continue;
        if(newtop >= ns->base && newtop < ns->top) {
            qunlock(&s->lk);
            error(Esoverlap);
        }
    }

    if(newsize > (PAGEDIRSIZE*PAGETABSIZE)) {
        qunlock(&s->lk);
        error(Enovmem);
    }

    // similar to code in newseg()
    mapsize = ROUND(newsize, PAGETABSIZE)/PAGETABSIZE;
    if(mapsize > s->pagedirsize){
        map = smalloc(mapsize*sizeof(Pagetable*));
        memmove(map, s->pagedir, s->pagedirsize*sizeof(Pagetable*));
        if(s->pagedir != s->smallpagedir)
            free(s->pagedir);
        s->pagedir = map;
        s->pagedirsize = mapsize;
    }

    s->top = newtop;
    s->size = newsize;
    qunlock(&s->lk);
    return 0;
}
/*e: function ibrk */

/*s: function mfreeseg */
/*
 *  called with s->lk locked
 */
void
mfreeseg(Segment *s, ulong start, int pages)
{
    int i, j, size;
    ulong soff;
    Page *pg;
    Page *list;

    soff = start-s->base;
    j = (soff&(PAGETABMAPMEM-1))/BY2PG; // PTX

    size = s->pagedirsize;
    list = nil;
    for(i = soff/PAGETABMAPMEM; i < size; i++) { // PDX
        if(pages <= 0)
            break;
        if(s->pagedir[i] == nil) { // space was never accessed, good, easier
            pages -= PAGETABSIZE-j;
            j = 0;
            continue;
        }
        while(j < PAGETABSIZE) {
            pg = s->pagedir[i]->pagetab[j];
            /*
             * We want to zero s->pagedir[i]->page[j] and putpage(pg),
             * but we have to make sure other processors flush the
             * entry from their TLBs before the page is freed.
             * We construct a list of the pages to be freed, zero
             * the entries, then (below) call procflushseg, and call
             * putpage on the whole list.
             *
             * Swapped-out pages don't appear in TLBs, so it's okay
             * to putswap those pages before procflushseg.
             */
            if(pg){
                /*s: [[mfreeseg]] if pg is a swap address */
                                if(onswap(pg))
                                    putswap(pg);
                /*e: [[mfreeseg]] if pg is a swap address */
                else{
                    pg->next = list;
                    list = pg;
                }
                s->pagedir[i]->pagetab[j] = nil;
            }
            if(--pages == 0)
                goto out;
            j++;
        }
        j = 0;
    }
out:
    /* flush this seg in all other processes */
    if(s->ref > 1)
        procflushseg(s);

    /* free the pages */
    for(pg = list; pg != nil; pg = list){
        list = list->next;
        putpage(pg);
    }
}
/*e: function mfreeseg */

/*s: function isoverlap */
Segment*
isoverlap(Proc *p, ulong va, int len)
{
    int i;
    Segment *ns;
    ulong newtop;

    newtop = va+len;
    for(i = 0; i < NSEG; i++) {
        ns = p->seg[i];
        if(ns == 0)
            continue;
        if((newtop > ns->base && newtop <= ns->top) ||
           (va >= ns->base && va < ns->top))
            return ns;
    }
    return nil;
}
/*e: function isoverlap */

/*s: function addphysseg */
int
addphysseg(Physseg* new)
{
    Physseg *ps;

    /*
     * Check not already entered and there is room
     * for a new entry and the terminating null entry.
     */
    lock(&physseglock);
    for(ps = physseg; ps->name; ps++){
        if(strcmp(ps->name, new->name) == 0){
            unlock(&physseglock);
            return -1;
        }
    }
    if(ps-physseg >= nelem(physseg)-2){
        unlock(&physseglock);
        return -1;
    }

    *ps = *new;
    unlock(&physseglock);

    return 0;
}
/*e: function addphysseg */

/*s: function segattach */
ulong
segattach(Proc *p, ulong attr, char *name, ulong va, ulong len)
{
    int sno;
    Segment *s, *os;
    Physseg *ps;

    if(va != 0 && va >= USTKTOP)
        error(Ebadarg);

    validaddr((ulong)name, 1, false);
    vmemchr(name, 0, ~0);

    for(sno = 0; sno < NSEG; sno++)
        if(p->seg[sno] == nil && sno != ESEG)
            break;

    if(sno == NSEG)
        error(Enovmem);

    /*
     *  first look for a global segment with the
     *  same name
     */
    if(_globalsegattach != nil){
        s = (*_globalsegattach)(p, name);
        if(s != nil){
            p->seg[sno] = s;
            return s->base;
        }
    }

    len = PGROUND(len);
    if(len == 0)
        error(Ebadarg);

    /*
     * Find a hole in the address space.
     * Starting at the lowest possible stack address - len,
     * check for an overlapping segment, and repeat at the
     * base of that segment - len until either a hole is found
     * or the address space is exhausted.  Ensure that we don't
     * map the zero page.
     */
    if(va == 0) {
        for (os = p->seg[SSEG]; os != nil; os = isoverlap(p, va, len)) {
            va = os->base;
            if(len >= va)
                error(Enovmem);
            va -= len;
        }
        va &= ~(BY2PG-1);
    } else {
        va &= ~(BY2PG-1);
        if(va == 0 || va >= USTKTOP)
            error(Ebadarg);
    }

    if(isoverlap(p, va, len) != nil)
        error(Esoverlap);


    for(ps = physseg; ps->name; ps++)
        if(strcmp(name, ps->name) == 0)
            goto found;

    error(Ebadarg);
found:
    if(len > ps->size)
        error(Enovmem);

    attr &= ~SG_TYPE;       /* Turn off what is not allowed */
    attr |= ps->attr;       /* Copy in defaults */

    s = newseg(attr, va, len/BY2PG);
    s->pseg = ps;
    p->seg[sno] = s;

    return va;
}
/*e: function segattach */

/*s: clock callback segclock */
// called via profclock
void
segclock(ulong pc)
{
    Segment *s;

    s = up->seg[TSEG];
    if(s == nil || s->profile == nil)
        return;

    s->profile[0] += TK2MS(1);
    if(pc >= s->base && pc < s->top) {
        pc -= s->base;
        s->profile[pc>>LRESPROF] += TK2MS(1);
    }
}
/*e: clock callback segclock */

// was in another file before
/*s: function data2txt */
Segment*
data2txt(Segment *s)
{
    Segment *ps;

    ps = newseg(SG_TEXT, s->base, s->size);
    ps->image = s->image;
    incref(ps->image);
    ps->fstart = s->fstart;
    ps->flen = s->flen;

    return ps;
}
/*e: function data2txt */
/*e: segment.c */
