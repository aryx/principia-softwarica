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
Segment* data2txt(Segment *s);
/*e: segment.c forward decl */

//*****************************************************************************
// Initialization
//*****************************************************************************

/*s: function imageinit */
void
imageinit(void)
{
    KImage *i, *ie;

    imagealloc.free = xalloc(conf.nimage * sizeof(KImage));
    /*s: [[imageinit()]] sanity check [[imagealloc.free]] */
    if (imagealloc.free == nil)
        panic("imageinit: no memory");
    /*e: [[imageinit()]] sanity check [[imagealloc.free]] */
    ie = &imagealloc.free[conf.nimage-1];
    for(i = imagealloc.free; i < ie; i++)
        i->next = i+1;
    i->next = nil;
    imagealloc.freechan = malloc(NFREECHAN * sizeof(Chan*));
    imagealloc.szfreechan = NFREECHAN;
}
/*e: function imageinit */

//*****************************************************************************
// Functions
//*****************************************************************************

/*s: constructor newseg */
Segment *
newseg(int type, user_addr base, ulong size)
{
    Segment *s;
    int pagedirsize;

    /*s: [[newseg()]] sanity check size */
    if(size > (PAGEDIRSIZE*PAGETABSIZE))
        error(Enovmem);
    /*e: [[newseg()]] sanity check size */
    s = smalloc(sizeof(Segment));

    s->ref = 1; // Segment.Ref.ref
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

    /*s: [[newseg()]] if pagedirsize small */
    if(pagedirsize <= nelem(s->smallpagedir)){
        s->pagedir = s->smallpagedir;
        s->pagedirsize = nelem(s->smallpagedir);
    }
    /*e: [[newseg()]] if pagedirsize small */
    else{
        pagedirsize *= 2; // room for growing
        if(pagedirsize > PAGEDIRSIZE)
            pagedirsize = PAGEDIRSIZE; // pad's first bugfix :)
        s->pagedir = smalloc(pagedirsize * sizeof(Pagetable*));
        s->pagedirsize = pagedirsize;
    }
    return s;
}
/*e: constructor newseg */

/*s: destructor putseg */
void
putseg(Segment *s)
{
    Pagetable **pde, **emap;
    /*s: [[putseg()]] other locals */
    KImage *img;
    /*e: [[putseg()]] other locals */

    /*s: [[putseg()]] sanity check s */
    if(s == nil)
        return; // TODO: panic("putseg") instead?
    /*e: [[putseg()]] sanity check s */
    /*s: [[putseg()]] if s has an image */
    img = s->image;
    if(img != nil) {
        lock(img);
        lock(s);
        if(img->s == s && s->ref == 1) // race?
            img->s = nil;
        unlock(img);
    }
    /*e: [[putseg()]] if s has an image */
    else
        lock(s);

    s->ref--;
    if(s->ref != 0) {
        unlock(s);
        return;
    }
    // else
    unlock(s);

    qlock(&s->lk);
    /*s: [[putseg()]] if s had an image */
    if(img)
        putimage(img);
    /*e: [[putseg()]] if s had an image */

    emap = &s->pagedir[s->pagedirsize];
    for(pde = s->pagedir; pde < emap; pde++)
        if(*pde)
            freept(s, *pde);

    qunlock(&s->lk);

    /*s: [[putseg()]] if smallpagedir */
    if(s->pagedir == s->smallpagedir) {
    }
    /*e: [[putseg()]] if smallpagedir */
    else
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
    Segment *s;
    Segment *n; // when allocate a new segment
    /*s: [[dupseg()]] other locals */
    int i;
    int size; // pagedir
    Pagetable *pt;
    /*e: [[dupseg()]] other locals */

    SET(n);

    s = seg[segno];

    qlock(&s->lk);
    if(waserror()){
        qunlock(&s->lk);
        nexterror();
    }
    switch(s->type&SG_TYPE) {
    /*s: [[dupseg()]] switch segment type cases */
    case SG_TEXT:       /* New segment shares pt set */
        goto sameseg;
    /*x: [[dupseg()]] switch segment type cases */
    case SG_STACK:
        n = newseg(s->type, s->base, s->size);
        break;
    /*x: [[dupseg()]] switch segment type cases */
    case SG_DATA:       /* Copy on write plus demand load info */
        /*s: [[dupseg()]] when SG_DATA, if Text segment */
        if(segno == TSEG){ // why not SG_TEXT then?
            poperror();
            qunlock(&s->lk);
            return data2txt(s);// ????
        }
        /*e: [[dupseg()]] when SG_DATA, if Text segment */
        if(share)
            goto sameseg; // threads! clone()
        // else
        n = newseg(s->type, s->base, s->size);
        /*s: [[dupseg()]] SG_DATA case, attach image to new segment n */
        incref(s->image); // how sure non nil? data always attached to an img
        n->image = s->image;
        n->fstart = s->fstart;
        n->flen = s->flen;
        /*e: [[dupseg()]] SG_DATA case, attach image to new segment n */
        break;
    /*x: [[dupseg()]] switch segment type cases */
    case SG_BSS:        /* Just copy on write */
        if(share)
            goto sameseg; // threads! clone()
        // else
        n = newseg(s->type, s->base, s->size);
        break;
    /*x: [[dupseg()]] switch segment type cases */
    case SG_SHARED:
    case SG_PHYSICAL:
        goto sameseg;

    /*e: [[dupseg()]] switch segment type cases */
    }
    // not sameseg, we have allocated a new seg in n above
    /*s: [[dupseg()]] when not goto sameseg, when allocated a new segment */
    size = s->pagedirsize;
    for(i = 0; i < size; i++)
        if(pt = s->pagedir[i])
            n->pagedir[i] = ptcpy(pt); // will actually share the pages

    /*s: [[dupseg()]] copy other fields */
    n->flushme = s->flushme;
    /*e: [[dupseg()]] copy other fields */
    /*s: [[dupseg()]] if original segment was shared, flush it */
    if(s->ref > 1)
        procflushseg(s); // ??
    /*e: [[dupseg()]] if original segment was shared, flush it */
    /*e: [[dupseg()]] when not goto sameseg, when allocated a new segment */

    poperror();
    qunlock(&s->lk);
    return n;

sameseg:
    /*s: [[dupseg()]] in sameseg, when share a segment */
    incref(s);
    /*e: [[dupseg()]] in sameseg, when share a segment */

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

    /*s: [[segpage()]] sanity check page in range of segment */
    if(p->va < s->base || p->va >= s->top)
        panic("segpage");
    /*e: [[segpage()]] sanity check page in range of segment */

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

/*s: constructor attachimage */
KImage*
attachimage(int type, Chan *c, virt_addr base, ulong len)
{
    KImage *img, **l;

    /* reclaim any free channels from reclaimed segments */
    if(imagealloc.nfreechan)
        imagechanreclaim();

    lock(&imagealloc);

    /*
     * Search the image cache for remains of the text from a previous
     * or currently running incarnation
     */
    for(img = ihash(c->qid.path); img; img = img->hash) {
        if(c->qid.path == img->qid.path) {
            lock(img);
            if(eqqid(c->qid, img->qid) &&
               eqqid(c->mqid, img->mqid) &&
               c->mchan == img->mchan &&
               c->type == img->type) {
                goto found;
            }
            unlock(img);
        }
    }

    /*
     * imagereclaim dumps pages from the free list which are cached by image
     * structures. This should free some image structures.
     */
    while(!(img = imagealloc.free)) {
        unlock(&imagealloc);
        imagereclaim();
        sched();
        lock(&imagealloc);
    }

    imagealloc.free = img->next;

    lock(img);
    incref(c);
    img->c = c;
    img->type = c->type;
    img->qid = c->qid;
    img->mqid = c->mqid;
    img->mchan = c->mchan;
    //add_hash(imagealloc.hash, c->qid.path, img)
    l = &ihash(c->qid.path);
    img->hash = *l;
    *l = img;

found:
    unlock(&imagealloc);

    if(img->s == nil) {
        /* Disaster after commit in exec */
        if(waserror()) {
            unlock(img);
            pexit(Enovmem, /*freemem*/true);
        }
        img->s = newseg(type, base, len);
        img->s->image = img;
        img->ref++;
        poperror();
    }
    else
        incref(img->s);

    return img;
}
/*e: constructor attachimage */

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
    uvlong ticks; // fastticks

    irstats.calls++;
    /* Somebody is already cleaning the page cache */
    if(!canqlock(&imagealloc.ireclaim))
        return;

    lock(&palloc);
    ticks = arch_fastticks(nil);
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
                uncachepage(p); // will call putimage()
            }
            unlock(p);
        }
    }
    ticks = arch_fastticks(nil) - ticks;
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
putimage(KImage *img)
{
    Chan *c, **cp;
    KImage *f, **l;

    if(img->notext)
        return;

    lock(img);
    if(--img->ref == 0) {
        l = &ihash(img->qid.path);
        mkqid(&img->qid, ~0, ~0, QTFILE);
        unlock(img);
        c = img->c;

        lock(&imagealloc);
        //remove_hash(imagealloc.hash, img)
        for(f = *l; f; f = f->hash) {
            if(f == img) {
                *l = img->hash;
                break;
            }
            l = &f->hash;
        }

        //add_list(imagealloc.free, img)
        img->next = imagealloc.free;
        imagealloc.free = img;

        /* defer freeing channel till we're out of spin lock's */

        if(imagealloc.nfreechan == imagealloc.szfreechan){
            //realloc(imagealloc.freenchan, szfreechan+NFREECHAN)
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
    unlock(img);
}
/*e: destructor putimage */

/*s: function ibrk */
long
ibrk(user_addr addr, int seg)
{
    Segment *s;
    user_addr newtop;
    ulong newsize;
    /*s: [[ibrk()]] other locals */
    int mapsize;
    Pagetable **map;
    /*x: [[ibrk()]] other locals */
    Segment *ns;
    int i;
    /*e: [[ibrk()]] other locals */

    s = up->seg[seg];
    /*s: [[ibrk()]] sanity check s */
    if(s == nil)
        error(Ebadarg);
    /*e: [[ibrk()]] sanity check s */
    /*s: [[ibrk()]] if addr nil */
    if(addr == nilptr)
        return s->base;
    /*e: [[ibrk()]] if addr nil */

    qlock(&s->lk);

    /*s: [[ibrk()]] if addr below base */
    /* We may start with the bss overlapping the data */
    if(addr < s->base) {
        if(seg != BSEG || up->seg[DSEG] == nil || addr < up->seg[DSEG]->base) {
            qunlock(&s->lk);
            error(Enovmem);
        }
        addr = s->base;
    }
    /*e: [[ibrk()]] if addr below base */

    newtop = PGROUND(addr);
    newsize = (newtop - s->base)/BY2PG; // in nb pages

    /*s: [[ibrk()]] if newtop lower than oldtop, shrink the segment */
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
        arch_flushmmu();
        return OK_0;
    }
    /*e: [[ibrk()]] if newtop lower than oldtop, shrink the segment */
    // else
    /*s: [[ibrk()]] if newtop more than oldtop, extend the segment */
    /*s: [[ibrk()]] sanity check newtop does not overlap other segments */
    for(i = 0; i < NSEG; i++) {
        ns = up->seg[i];
        if(ns == nil || ns == s)
            continue;
        if(newtop >= ns->base && newtop < ns->top) {
            qunlock(&s->lk);
            error(Esoverlap);
        }
    }
    /*e: [[ibrk()]] sanity check newtop does not overlap other segments */
    /*s: [[ibrk()]] sanity check newsize */
    if(newsize > (PAGEDIRSIZE*PAGETABSIZE)) {
        qunlock(&s->lk);
        error(Enovmem);
    }
    /*e: [[ibrk()]] sanity check newsize */

    // similar to code in newseg()
    mapsize = ROUND(newsize, PAGETABSIZE)/PAGETABSIZE;
    // realloc
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
    return OK_0;
    /*e: [[ibrk()]] if newtop more than oldtop, extend the segment */

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
    PageOrSwap *pg;
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
    /*s: [[mfreeseg()]] if segment was shared, flush it */
    /* flush this seg in all other processes */
    if(s->ref > 1)
        procflushseg(s);
    /*e: [[mfreeseg()]] if segment was shared, flush it */
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
    /*s: [[data2txt()]] initialize other segment fields */
    ps->flushme = true;
    /*e: [[data2txt()]] initialize other segment fields */

    return ps;
}
/*e: function data2txt */
/*e: segment.c */
