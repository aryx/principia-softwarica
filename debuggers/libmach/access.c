/*s: libmach/access.c */
/*
 * functions to read and write an executable or file image
 */

#include <u.h>
#include <libc.h>
#include <bio.h>
#include <mach.h>

static	int	mget(Map*, uvlong, void*, int);
static	int	mput(Map*, uvlong, void*, int);
static	struct	segment*	reloc(Map*, uvlong, vlong*);

/*s: function [[geta]] */
/*
 * routines to get/put various types
 */
int
geta(Map *map, uvlong addr, uvlong *x)
{
    ulong l;
    uvlong vl;

    if (mach->szaddr == 8){
        if (get8(map, addr, &vl) < 0)
            return -1;
        *x = vl;
        return 1;
    }

    if (get4(map, addr, &l) < 0)
        return -1;
    *x = l;

    return 1;
}
/*e: function [[geta]] */

/*s: function [[get8]] */
int
get8(Map *map, uvlong addr, uvlong *x)
{
    if (!map) {
        werrstr("get8: invalid map");
        return -1;
    }

    if (map->nsegs == 1 && map->seg[0].fd < 0) {
        *x = addr;
        return 1;
    }
    if (mget(map, addr, x, 8) < 0)
        return -1;
    *x = machdata->swav(*x);
    return 1;
}
/*e: function [[get8]] */

/*s: function [[get4]] */
int
get4(Map *map, uvlong addr, ulong *x)
{
    if (!map) {
        werrstr("get4: invalid map");
        return -1;
    }

    if (map->nsegs == 1 && map->seg[0].fd < 0) {
        *x = addr;
        return 1;
    }
    if (mget(map, addr, x, 4) < 0)
        return -1;
    *x = machdata->swal(*x);
    return 1;
}
/*e: function [[get4]] */

/*s: function [[get2]] */
int
get2(Map *map, uvlong addr, ushort *x)
{
    if (!map) {
        werrstr("get2: invalid map");
        return -1;
    }

    if (map->nsegs == 1 && map->seg[0].fd < 0) {
        *x = addr;
        return 1;
    }
    if (mget(map, addr, x, 2) < 0)
        return -1;
    *x = machdata->swab(*x);
    return 1;
}
/*e: function [[get2]] */

/*s: function [[get1]] */
int
get1(Map *map, uvlong addr, uchar *x, int size)
{
    uchar *cp;

    if (!map) {
        werrstr("get1: invalid map");
        return -1;
    }

    if (map->nsegs == 1 && map->seg[0].fd < 0) {
        cp = (uchar*)&addr;
        while (cp < (uchar*)(&addr+1) && size-- > 0)
            *x++ = *cp++;
        while (size-- > 0)
            *x++ = 0;
    } else
        return mget(map, addr, x, size);
    return 1;
}
/*e: function [[get1]] */

/*s: function [[puta]] */
int
puta(Map *map, uvlong addr, uvlong v)
{
    if (mach->szaddr == 8)
        return put8(map, addr, v);

    return put4(map, addr, v);
}
/*e: function [[puta]] */

/*s: function [[put8]] */
int
put8(Map *map, uvlong addr, uvlong v)
{
    if (!map) {
        werrstr("put8: invalid map");
        return -1;
    }
    v = machdata->swav(v);
    return mput(map, addr, &v, 8);
}
/*e: function [[put8]] */

/*s: function put4 (libmach/access.c) */
int
put4(Map *map, uvlong addr, ulong v)
{
    if (!map) {
        werrstr("put4: invalid map");
        return -1;
    }
    v = machdata->swal(v);
    return mput(map, addr, &v, 4);
}
/*e: function put4 (libmach/access.c) */

/*s: function [[put2]] */
int
put2(Map *map, uvlong addr, ushort v)
{
    if (!map) {
        werrstr("put2: invalid map");
        return -1;
    }
    v = machdata->swab(v);
    return mput(map, addr, &v, 2);
}
/*e: function [[put2]] */

/*s: function [[put1]] */
int
put1(Map *map, uvlong addr, uchar *v, int size)
{
    if (!map) {
        werrstr("put1: invalid map");
        return -1;
    }
    return mput(map, addr, v, size);
}
/*e: function [[put1]] */

/*s: function [[spread]] */
static int
spread(struct segment *s, void *buf, int n, uvlong off)
{
    uvlong base;

    static struct {
        struct segment *s;
        char a[8192];
        uvlong off;
    } cache;

    if(s->cache){
        base = off&~(sizeof cache.a-1);
        if(cache.s != s || cache.off != base){
            cache.off = ~0;
            if(seek(s->fd, base, 0) >= 0
            && readn(s->fd, cache.a, sizeof cache.a) == sizeof cache.a){
                cache.s = s;
                cache.off = base;
            }
        }
        if(cache.s == s && cache.off == base){
            off &= sizeof cache.a-1;
            if(off+n > sizeof cache.a)
                n = sizeof cache.a - off;
            memmove(buf, cache.a+off, n);
            return n;
        }
    }

    return pread(s->fd, buf, n, off);
}
/*e: function [[spread]] */

/*s: function [[mget]] */
static int
mget(Map *map, uvlong addr, void *buf, int size)
{
    uvlong off;
    int i, j, k;
    struct segment *s;

    s = reloc(map, addr, (vlong*)&off);
    if (!s)
        return -1;
    if (s->fd < 0) {
        werrstr("unreadable map");
        return -1;
    }
    for (i = j = 0; i < 2; i++) {	/* in case read crosses page */
        k = spread(s, (void*)((uchar *)buf+j), size-j, off+j);
        if (k < 0) {
            werrstr("can't read address %llux: %r", addr);
            return -1;
        }
        j += k;
        if (j == size)
            return j;
    }
    werrstr("partial read at address %llux (size %d j %d)", addr, size, j);
    return -1;
}
/*e: function [[mget]] */

/*s: function [[mput]] */
static int
mput(Map *map, uvlong addr, void *buf, int size)
{
    vlong off;
    int i, j, k;
    struct segment *s;

    s = reloc(map, addr, &off);
    if (!s)
        return -1;
    if (s->fd < 0) {
        werrstr("unwritable map");
        return -1;
    }

    seek(s->fd, off, 0);
    for (i = j = 0; i < 2; i++) {	/* in case read crosses page */
        k = write(s->fd, buf, size-j);
        if (k < 0) {
            werrstr("can't write address %llux: %r", addr);
            return -1;
        }
        j += k;
        if (j == size)
            return j;
    }
    werrstr("partial write at address %llux", addr);
    return -1;
}
/*e: function [[mput]] */

/*s: function [[reloc]] */
/*
 *	convert address to file offset; returns nonzero if ok
 */
static struct segment*
reloc(Map *map, uvlong addr, vlong *offp)
{
    int i;

    for (i = 0; i < map->nsegs; i++) {
        if (map->seg[i].inuse)
        if (map->seg[i].b <= addr && addr < map->seg[i].e) {
            *offp = addr + map->seg[i].f - map->seg[i].b;
            return &map->seg[i];
        }
    }
    werrstr("can't translate address %llux", addr);
    return 0;
}
/*e: function [[reloc]] */
/*e: libmach/access.c */
