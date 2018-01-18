/*s: lib_graphics/libdraw/font.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <draw_private.h>
#include <font.h>

/*s: function [[skip]] */
static char*
skip(char *s)
{
    while(*s==' ' || *s=='\n' || *s=='\t')
        s++;
    return s;
}
/*e: function [[skip]] */


/*s: function [[openfont]] */
Font*
openfont(Display *d, char *name)
{
    Font *fnt;
    fdt fd;
    int i, n;
    char *buf;
    Dir *dir;

    fd = open(name, OREAD);
    /*s: [[openfont()]] sanity check fd */
    if(fd < 0)
        return nil;
    /*e: [[openfont()]] sanity check fd */

    // n = filename_size(fd)
    dir = dirfstat(fd);
    /*s: [[openfont()]] sanity check dir */
    if(dir == nil){
    Err0:
        close(fd);
        return nil;
    }
    /*e: [[openfont()]] sanity check dir */
    n = dir->length;
    free(dir);

    buf = malloc(n+1);
    /*s: [[openfont()]] sanity check buf */
    if(buf == nil)
        goto Err0;
    /*e: [[openfont()]] sanity check buf */
    buf[n] = '\0';
    i = read(fd, buf, n);
    close(fd);
    /*s: [[openfont()]] sanity check i */
    if(i != n){
        free(buf);
        return nil;
    }
    /*e: [[openfont()]] sanity check i */

    fnt = buildfont(d, buf, name);

    free(buf);
    return fnt;
}
/*e: function [[openfont]] */

/*s: function [[buildfont]] */
Font*
buildfont(Display *d, char *buf, char *name)
{
    Font *fnt;
    char *s, *t;
    ulong min, max;
    Cachefont *c;
    int offset;
    /*s: [[buildfont()]] other locals */
    char badform[] = "bad font format: number expected (char position %d)";
    /*e: [[buildfont()]] other locals */

    s = buf;

    fnt = malloc(sizeof(Font));
    /*s: [[buildfont()]] sanity check fnt */
    if(fnt == nil)
        return nil;
    /*e: [[buildfont()]] sanity check fnt */
    memset(fnt, 0, sizeof(Font));

    fnt->display = d;
    fnt->name = strdup(name);

    /*s: [[buildfont()]] allocate cache */
    fnt->ncache = NFCACHE+NFLOOK;
    fnt->cache = malloc(fnt->ncache * sizeof(fnt->cache[0]));
    /*x: [[buildfont()]] allocate cache */
    fnt->nsubf = NFSUBF;
    fnt->subf = malloc(fnt->nsubf * sizeof(fnt->subf[0]));
    /*e: [[buildfont()]] allocate cache */
    /*s: [[buildfont()]] initialize cache */
    fnt->age = 1;
    /*x: [[buildfont()]] initialize cache */
    /*s: [[buildfont()]] sanity check fnt fields part1 */
    if(fnt->name==nil || fnt->cache==nil || fnt->subf==nil){
    Err2:
        free(fnt->name);
        free(fnt->cache);
        free(fnt->subf);
        free(fnt->sub);
        free(fnt);
        return 0;
    }
    /*e: [[buildfont()]] sanity check fnt fields part1 */
    memset(fnt->subf, 0, fnt->nsubf * sizeof(fnt->subf[0]));
    memset(fnt->cache, 0, fnt->ncache * sizeof(fnt->cache[0]));
    /*e: [[buildfont()]] initialize cache */

    fnt->height = strtol(s, &s, 0);
    s = skip(s);
    fnt->ascent = strtol(s, &s, 0);
    s = skip(s);
    /*s: [[buildfont()]] sanity check fnt fields part2 */
    if(fnt->height <= 0 || fnt->ascent <= 0){
        werrstr("bad height or ascent in font file");
        goto Err2;
    }
    /*e: [[buildfont()]] sanity check fnt fields part2 */

    fnt->width = 0;

    fnt->sub = nil;
    fnt->nsub = 0;
    do{
        /* must be looking at a number now */
        /*s: [[buildfont()]] sanity check s content */
        if(*s<'0' || '9'<*s){
            werrstr(badform, s-buf);
            goto Err3;
        }
        /*e: [[buildfont()]] sanity check s content */
        min = strtol(s, &s, 0);
        s = skip(s);
        /* must be looking at a number now */
        /*s: [[buildfont()]] sanity check s content */
        if(*s<'0' || '9'<*s){
            werrstr(badform, s-buf);
            goto Err3;
        }
        /*e: [[buildfont()]] sanity check s content */
        max = strtol(s, &s, 0);
        s = skip(s);
        /*s: [[buildfont()]] sanity check min and max */
        if(*s=='\0' || min>=Runemax || max>=Runemax || min>max){
            werrstr("illegal subfont range");
        Err3:
            freefont(fnt);
            return nil;
        }
        /*e: [[buildfont()]] sanity check min and max */

        /*s: [[buildfont()]] set optional offset */
        t = s;
        offset = strtol(s, &t, 0);
        if(t>s && (*t==' ' || *t=='\t' || *t=='\n'))
            s = skip(t);
        else
            offset = 0;
        /*e: [[buildfont()]] set optional offset */

        fnt->sub = realloc(fnt->sub, (fnt->nsub+1)*sizeof(Cachefont*));
        /*s: [[buildfont()]] sanity check fnt fields part3 */
        if(fnt->sub == nil){
            /* realloc manual says fnt->sub may have been destroyed */
            fnt->nsub = 0;
            goto Err3;
        }
        /*e: [[buildfont()]] sanity check fnt fields part3 */

        c = malloc(sizeof(Cachefont));
        /*s: [[buildfont()]] sanity check c */
        if(c == nil)
            goto Err3;
        /*e: [[buildfont()]] sanity check c */
        fnt->sub[fnt->nsub] = c;
        fnt->nsub++;

        c->min = min;
        c->max = max;
        c->offset = offset;

        t = s;
        while(*s && *s!=' ' && *s!='\n' && *s!='\t')
            s++;
        *s++ = '\0';

        c->name = strdup(t);
        c->subfontname = nil; // full filename computed lazily later
        /*s: [[buildfont()]] sanity check c name field */
        if(c->name == nil){
            free(c);
            goto Err3;
        }
        /*e: [[buildfont()]] sanity check c name field */
        s = skip(s);
    } while(*s);
    return fnt;
}
/*e: function [[buildfont]] */

/*s: function [[freefont]] */
void
freefont(Font *f)
{
    int i;
    Cachefont *c;
    Subfont *s;

    /*s: [[freefont()]] sanity check f */
    if(f == nil)
        return;
    /*e: [[freefont()]] sanity check f */

    for(i=0; i<f->nsub; i++){
        c = f->sub[i];
        free(c->name);
        free(c->subfontname);
        free(c);
    }
    free(f->sub);

    /*s: [[freefont()]] free cache */
    freeimage(f->cacheimage);
    /*x: [[freefont()]] free cache */
    free(f->cache);
    /*x: [[freefont()]] free cache */
    for(i=0; i<f->nsubf; i++){
        s = f->subf[i].f;
        if(s && display && s != display->defaultsubfont)
            freesubfont(s);
    }
    free(f->subf);
    /*e: [[freefont()]] free cache */

    free(f->name);
    free(f);
}
/*e: function [[freefont]] */
/*e: lib_graphics/libdraw/font.c */
