/*s: lib_graphics/libdraw/fontcache.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <draw_private.h>
#include <font.h>

static int	fontresize(Font*, int, int, int);

/*s: constant PJW */
#define	PJW	0	/* use NUL==pjw for invisible characters */
/*e: constant PJW */

/*s: function cachechars */
int
cachechars(Font *f, char **ss, Rune **rr, ushort *cp, int max, int *wp, char **subfontname)
{
    Rune *rp;
    char *sp;
    Rune r;
    int w, rw;
    int wid = 0;
    int i;
    int h; // index in Font.cache
    int sh; // rune hashcode
    Cacheinfo *c, *ec;
    /*s: [[cachechars()]] other locals */
    Rune vr;
    /*x: [[cachechars()]] other locals */
    int ld;
    /*x: [[cachechars()]] other locals */
    int th;
    Cacheinfo *tc;
    ulong a;
    /*x: [[cachechars()]] other locals */
    int nc;
    /*e: [[cachechars()]] other locals */

    /*s: [[cachechars()]] non unicode string handling part1 */
    if(ss){
        sp = *ss;
        rp = L"";
    }
    /*e: [[cachechars()]] non unicode string handling part1 */
    else{
        sp = "";
        rp = *rr;
    }
    *subfontname = nil;

    for(i=0; i<max && (*sp || *rp); sp+=w, rp+=rw){
        /*s: [[cachechars()]] non unicode string handling part2 */
        if(ss){
            r = *(uchar*)sp;
            if(r < Runeself)
                w = 1;
            else{
                w = chartorune(&vr, sp);
                r = vr;
            }
            rw = 0;
        }
        /*e: [[cachechars()]] non unicode string handling part2 */
        else{
            r = *rp;
            w = 0;
            rw = 1;
        }

        // sh = hash_code(r)
        sh = (17 * (uint)r) & (f->ncache-NFLOOK-1);

        // c,h = lookup(r, sh, f->cache)
        c = &f->cache[sh];
        ec = c+NFLOOK;
        h = sh;
        while(c < ec){
            if(c->value==r && c->age)
                goto Found;
            c++;
            h++;
        }
        // this can break out of the loop
        /*s: [[cachechars()]] when rune not in cache, loads it */
        /*s: [[cachechars()]] find oldest entry [[c]] with age a in cache */
        /*
         * Not found; toss out oldest entry
         */
        a = ~0;
        th = sh;
        tc = &f->cache[th];
        while(tc < ec){
            if(tc->age < a){
                a = tc->age;
                h = th;
                c = tc;
            }
            tc++;
            th++;
        }
        /*e: [[cachechars()]] find oldest entry [[c]] with age a in cache */
        /*s: [[cachechars()]] if age too recent then resize cache */
        if(a && (f->age - a) < 500){	/* kicking out too recent; resize */
            nc = 2*(f->ncache-NFLOOK) + NFLOOK;
            if(nc <= MAXFCACHE){
                if(i == 0)
                    fontresize(f, f->width, nc, f->maxdepth);
                /* else flush first; retry will resize */
                break;
            }
            // else, no resize
        }
        /*e: [[cachechars()]] if age too recent then resize cache */
        /*s: [[cachechars()]] if same age */
        if(c->age == f->age)	/* flush pending string output */
            break;
        /*e: [[cachechars()]] if same age */
        // else

        ld = loadchar(f, r, c, h, i, subfontname);
        /*s: [[cachechars()]] if could not load char */
        if(ld <= 0){
            /*s: [[cachechars()]] if loadchar failed */
            if(ld == 0) // when failed, but why can fail?
                continue;
            /*e: [[cachechars()]] if loadchar failed */
            break;
        }
        /*e: [[cachechars()]] if could not load char */
        // else
        c = &f->cache[h];	/* may have reallocated f->cache */
        /*e: [[cachechars()]] when rune not in cache, loads it */
    
        Found:
        wid += c->width;
        c->age = f->age;
        cp[i] = h;
        i++;
    }
    /*s: [[cachechars()]] non unicode string handling part3 */
    if(ss)
        *ss = sp;
    /*e: [[cachechars()]] non unicode string handling part3 */
    else
        *rr = rp;
    *wp = wid;

    return i;
}
/*e: function cachechars */

/*s: function agefont */
void
agefont(Font *f)
{
    /*s: [[agefont()]] locals */
    Cacheinfo *c, *ec;
    Cachesubf *s, *es;
    /*e: [[agefont()]] locals */

    f->age++;
    /*s: [[agefont()]] if age overflow */
    if(f->age == 65536){
        /*
         * Renormalize ages
         */
        c = f->cache;
        ec = c+f->ncache;
        while(c < ec){
            if(c->age){
                c->age >>= 2;
                c->age++;
            }
            c++;
        }

        s = f->subf;
        es = s + f->nsubf;
        while(s < es){
            if(s->age){
                if(s->age < SUBFAGE && s->cf->name != nil){
                    /* clean up */
                    if(display &&
                        s->f != display->defaultsubfont)
                        freesubfont(s->f);
                    s->cf = nil;
                    s->f = nil;
                    s->age = 0;
                }else{
                    s->age >>= 2;
                    s->age++;
                }
            }
            s++;
        }
        f->age = (65536>>2) + 1;
    }
    /*e: [[agefont()]] if age overflow */
}
/*e: function agefont */

/*s: function cf2subfont */
static Subfont*
cf2subfont(Cachefont *cf, Font *f)
{
    int depth;
    char *name;

    name = cf->subfontname;
    if(name == nil){
        /*s: [[cf2subfont()]] set depth */
        if(f->display && f->display->screenimage)
            depth = f->display->screenimage->depth;
        else
            depth = 8;
        /*e: [[cf2subfont()]] set depth */
        name = subfontname(cf->name, f->name, depth);
        /*s: [[cf2subfont()]] sanity check name */
        if(name == nil)
            return nil;
        /*e: [[cf2subfont()]] sanity check name */
        cf->subfontname = name;
    }
    return lookupsubfont(f->display, name);
}
/*e: function cf2subfont */

/*s: function loadchar */
/* return 1 if load succeeded, 0 if failed, -1 if must retry */
error0
loadchar(Font *f, Rune r, Cacheinfo *c, int h, int noflush, char **subfontname)
{
    Rune pic; // ?? means
    Cachefont *cf;
    int i;
    int oi, wid, top, bottom;
    Fontchar *fi;
    Cachesubf *subf;
    /*s: [[loadchar()]] other locals */
    byte *b;
    /*x: [[loadchar()]] other locals */
    Cachesubf *of;
    /*e: [[loadchar()]] other locals */

    pic = r;

    // Find subfont spec cf for Rune r 
    Again:
    for(i=0; i < f->nsub; i++){
        cf = f->sub[i];
        if(cf->min <= pic && pic <= cf->max)
            goto Found;
    }
    /*s: [[loadchar()]] if rune not handled by the font */
    TryPJW:
    if(pic != PJW){
        pic = PJW;
        goto Again;
    }
    return ERROR_0;
    /*e: [[loadchar()]] if rune not handled by the font */

    Found:
    // Now let's find the loaded subfont subf with spec cf 
    /*
     * Choose exact or oldest
     */
    oi = 0;
    subf = &f->subf[0];
    for(i=0; i < f->nsubf; i++){
        if(cf == subf->cf)
            goto Found2;
        if(subf->age < f->subf[oi].age)
            oi = i;
        subf++;
    }
    // may load and find the right subf
    /*s: [[loadchar()]] when the corresponding subfont is not loaded yet */
    subf = &f->subf[oi];
    /*s: [[loadchar()]] if old subfont entry valid, free it or expand subfont cache */
    if(subf->f){
        if(f->age - subf->age > SUBFAGE || f->nsubf > MAXSUBF){
    Toss:
            /* ancient data; toss */
            freesubfont(subf->f);
            subf->cf = nil;
            subf->f = nil;
            subf->age = 0;
        }else{				/* too recent; grow instead */
            of = f->subf;
            f->subf = malloc((f->nsubf+DSUBF) * sizeof(Subfont));
            /*s: [[loadchar()]] sanity check new malloced subf */
            if(f->subf == nil){
                f->subf = of;
                goto Toss;
            }
            /*e: [[loadchar()]] sanity check new malloced subf */
            memmove(f->subf, of, (f->nsubf+DSUBF) * sizeof(Subfont));
            memset(f->subf+f->nsubf, 0, DSUBF*sizeof *subf);
            subf = &f->subf[f->nsubf];
            f->nsubf += DSUBF;
            free(of);
        }
    }
    /*e: [[loadchar()]] if old subfont entry valid, free it or expand subfont cache */
    subf->age = 0;
    subf->cf = nil;
    subf->f = cf2subfont(cf, f);
    /*s: [[loadchar()]] sanity check subfont f */
    if(subf->f == nil){
        if(cf->subfontname == nil)
            goto TryPJW;
        *subfontname = cf->subfontname;
        return -1; // caller must retry
    }
    /*e: [[loadchar()]] sanity check subfont f */
    subf->cf = cf;
    /*s: [[loadchar()]] sanity check ascents */
    if(subf->f->ascent > f->ascent && f->display){
        /* should print something? this is a mistake in the font file */
        /* must prevent c->top from going negative when loading cache */
        Image *b;
        int d, t;
        d = subf->f->ascent - f->ascent;
        b = subf->f->bits;
        draw(b, b->r, b, nil, addpt(b->r.min, Pt(0, d)));
        draw(b, Rect(b->r.min.x, b->r.max.y-d, b->r.max.x, b->r.max.y), f->display->black, nil, b->r.min);
        for(i=0; i<subf->f->n; i++){
            t = subf->f->info[i].top-d;
            if(t < 0)
                t = 0;
            subf->f->info[i].top = t;
            t = subf->f->info[i].bottom-d;
            if(t < 0)
                t = 0;
            subf->f->info[i].bottom = t;
        }
        subf->f->ascent = f->ascent;
    }
    /*e: [[loadchar()]] sanity check ascents */
    /*e: [[loadchar()]] when the corresponding subfont is not loaded yet */

    Found2:
    subf->age = f->age;

    /* possible overflow here, but works out okay */
    pic += cf->offset;
    pic -= cf->min;
    /*s: [[loadchar()]] if rune outside range */
    if(pic >= subf->f->n)
        goto TryPJW;
    /*e: [[loadchar()]] if rune outside range */
    fi = &subf->f->info[pic];
    /*s: [[loadchar()]] sanity check fontchar width */
    if(fi->width == 0)
        goto TryPJW;
    /*e: [[loadchar()]] sanity check fontchar width */
    wid = (fi+1)->x - fi->x;
    /*s: [[loadchar()]] resize cache if width too big */
    if(f->width < wid || f->width == 0 || f->maxdepth < subf->f->bits->depth){
        /*
         * Flush, free, reload (easier than reformatting f->b)
         */
        if(noflush)
            return -1;
        // else

        if(f->width < wid)
            f->width = wid;
        if(f->maxdepth < subf->f->bits->depth)
            f->maxdepth = subf->f->bits->depth;
        i = fontresize(f, f->width, f->ncache, f->maxdepth);
        if(i <= 0)
            return i;
        /* c is still valid as didn't reallocate f->cache */
    }
    /*e: [[loadchar()]] resize cache if width too big */

    c->value = r;
    top    = fi->top    + (f->ascent - subf->f->ascent);
    bottom = fi->bottom + (f->ascent - subf->f->ascent);
    c->width = fi->width;
    c->x = h * f->width;
    c->left = fi->left;

    /*s: [[loadchar()]] sanity check display */
    if(f->display == nil)
        return OK_1;
    /*e: [[loadchar()]] sanity check display */

    /*s: [[loadchar()]] marshall Cacheinfo c */
    flushimage(f->display, false);	/* flush any pending errors */

    // load character: 'l' fontid[4] srcid[4] index[2] R[4*4] P[2*4] left[1] width[1]
    b = bufimage(f->display, 37);
    /*s: [[loadchar()]] sanity check b */
    if(b == nil)
        return ERROR_0;
    /*e: [[loadchar()]] sanity check b */
    b[0] = 'l';
    BPLONG(b+1, f->cacheimage->id);
    BPLONG(b+5, subf->f->bits->id);

    // index
    BPSHORT(b+9, c - f->cache); 

    // destination coordinates in f->cacheimage
    BPLONG(b+11, c->x);
    BPLONG(b+15, top);
    BPLONG(b+19, c->x + ((fi+1)->x - fi->x));
    BPLONG(b+23, bottom);

    // source coordinate in subf->f->bits->id
    BPLONG(b+27, fi->x);
    BPLONG(b+31, fi->top);
    b[35] = fi->left;
    b[36] = fi->width;

    return OK_1;
    /*e: [[loadchar()]] marshall Cacheinfo c */
}
/*e: function loadchar */

/*s: function fontresize */
/* return whether resize succeeded && f->cache is unchanged */
static bool
fontresize(Font *f, int wid, int ncache, int depth)
{
    Cacheinfo *i;
    int ret;
    Image *new;
    uchar *b;
    Display *d;

    ret = false;
    /*s: [[fontresize()]] sanity check depth and wid */
    if(depth <= 0)
        depth = 1;
    if(wid <= 0)
        wid = 1;
    /*e: [[fontresize()]] sanity check depth and wid */
    d = f->display;
    /*s: [[fontresize()]] sanity check d */
    if(d == nil)
        goto Nodisplay;
    /*e: [[fontresize()]] sanity check d */

    new = allocimage(d, Rect(0, 0, ncache*wid, f->height), CHAN1(CGrey, depth), false, 0);
    /*s: [[fontresize()]] sanity check new */
    if(new == nil){
        fprint(2, "font cache resize failed: %r\n");
        abort();
        goto Return;
    }
    /*e: [[fontresize()]] sanity check new */
    flushimage(d, false);	/* flush any pending errors */

    b = bufimage(d, 1+4+4+1);
    /*s: [[fontresize()]] sanity check b */
    if(b == nil){
        freeimage(new);
        goto Return;
    }
    /*e: [[fontresize()]] sanity check b */
    b[0] = 'i';
    BPLONG(b+1, new->id);
    BPLONG(b+5, ncache);
    b[9] = f->ascent;
    if(flushimage(d, false) < 0){
        fprint(2, "resize: init failed: %r\n");
        freeimage(new);
        goto Return;
    }

    freeimage(f->cacheimage);
    f->cacheimage = new;

    Nodisplay:
    f->width = wid;
    f->maxdepth = depth;
    ret = true;
    /*s: [[fontresize()]] if need to resize the cache */
    if(f->ncache != ncache){
        i = malloc(ncache*sizeof f->cache[0]);
        if(i != nil){
            ret = false;
            free(f->cache);
            f->ncache = ncache;
            f->cache = i;
        }
        /* else just wipe the cache clean and things will be ok */
    }
    /*e: [[fontresize()]] if need to resize the cache */
    Return:
    memset(f->cache, 0, f->ncache*sizeof f->cache[0]);
    return ret;
}
/*e: function fontresize */
/*e: lib_graphics/libdraw/fontcache.c */
