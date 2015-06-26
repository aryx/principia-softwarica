/*s: linkers/libmach/obj.c */
/*
 * obj.c
 * routines universal to all object files
 */
#include <u.h>
#include <libc.h>
#include <bio.h>
#include <ar.h>
#include <mach.h>

#include "obj.h"

/*s: function islocal */
#define islocal(t)	((t)=='a' || (t)=='p')
/*e: function islocal */

/*s: enum _anon_ (linkers/libmach/obj.c) */
enum
{
    NNAMES	= 50,
    MAXIS	= 8,		/* max length to determine if a file is a .? file */
    MAXOFF	= 0x7fffffff,	/* larger than any possible local offset */
    NHASH	= 1024,		/* must be power of two */
    HASHMUL	= 79L,
};
/*e: enum _anon_ (linkers/libmach/obj.c) */

int			/* in [$OS].c */ //$
  _is5(char*),
  _is8(char*),
  _read5(Biobuf*, Prog*),
  _read8(Biobuf*, Prog*);


typedef struct Obj	Obj;
typedef struct Symtab	Symtab;

/*s: struct Obj */
struct	Obj		/* functions to handle each intermediate (.$O) file */
{
    char	*name;				/* name of each $O file */
    int	(*is)(char*);			/* test for each type of $O file */
    int	(*read)(Biobuf*, Prog*);	/* read for each type of $O file*/
};
/*e: struct Obj */

/*s: global obj */
static Obj	obj[] =
{			/* functions to identify and parse each type of obj */
    [ObjArm]	"arm .5",	_is5, _read5,
    [Obj386]	"386 .8",	_is8, _read8,
    [Maxobjtype]	0, 0
};
/*e: global obj */

/*s: struct Symtab */
struct	Symtab
{
    struct	Sym 	s;
    struct	Symtab	*next;
};
/*e: struct Symtab */

/*s: global hash (linkers/libmach/obj.c) */
static	Symtab *hash[NHASH];
/*e: global hash (linkers/libmach/obj.c) */
/*s: global names */
static	Sym	*names[NNAMES];	/* working set of active names */
/*e: global names */

static	int	processprog(Prog*,int);	/* decode each symbol reference */
static	void	objreset(void);
static	void	objlookup(int, char *, int, uint);
static	void 	objupdate(int, int);

/*s: function objtype */
int
objtype(Biobuf *bp, char **name)
{
    int i;
    char buf[MAXIS];

    if(Bread(bp, buf, MAXIS) < MAXIS)
        return -1;
    Bseek(bp, -MAXIS, 1);
    for (i = 0; i < Maxobjtype; i++) {
        if (obj[i].is && (*obj[i].is)(buf)) {
            if (name)
                *name = obj[i].name;
            return i;
        }
    }
    return -1;
}
/*e: function objtype */

/*s: function isar */
int
isar(Biobuf *bp)
{
    int n;
    char magbuf[SARMAG];

    n = Bread(bp, magbuf, SARMAG);
    if(n == SARMAG && strncmp(magbuf, ARMAG, SARMAG) == 0)
        return 1;
    return 0;
}
/*e: function isar */

/*s: function readobj */
/*
 * determine what kind of object file this is and process it.
 * return whether or not this was a recognized intermediate file.
 */
int
readobj(Biobuf *bp, int objtype)
{
    Prog p;

    if (objtype < 0 || objtype >= Maxobjtype || obj[objtype].is == 0)
        return 1;
    objreset();
    while ((*obj[objtype].read)(bp, &p))
        if (!processprog(&p, 1))
            return 0;
    return 1;
}
/*e: function readobj */

/*s: function readar */
int
readar(Biobuf *bp, int objtype, vlong end, int doautos)
{
    Prog p;

    if (objtype < 0 || objtype >= Maxobjtype || obj[objtype].is == 0)
        return 1;
    objreset();
    while ((*obj[objtype].read)(bp, &p) && Boffset(bp) < end)
        if (!processprog(&p, doautos))
            return 0;
    return 1;
}
/*e: function readar */

/*s: function processprog */
/*
 *	decode a symbol reference or definition
 */
static	int
processprog(Prog *p, int doautos)
{
    if(p->kind == aNone)
        return 1;
    if(p->sym < 0 || p->sym >= NNAMES)
        return 0;
    switch(p->kind)
    {
    case aName:
        if (!doautos)
        if(p->type != 'U' && p->type != 'b')
            break;
        objlookup(p->sym, p->id, p->type, p->sig);
        break;
    case aText:
        objupdate(p->sym, 'T');
        break;
    case aData:
        objupdate(p->sym, 'D');
        break;
    default:
        break;
    }
    return 1;
}
/*e: function processprog */

/*s: function objlookup */
/*
 * find the entry for s in the symbol array.
 * make a new entry if it is not already there.
 */
static void
objlookup(int id, char *name, int type, uint sig)
{
    long h;
    char *cp;
    Sym *s;
    Symtab *sp;

    s = names[id];
    if(s && strcmp(s->name, name) == 0) {
        s->type = type;
        s->sig = sig;
        return;
    }

    h = *name;
    for(cp = name+1; *cp; h += *cp++)
        h *= HASHMUL;
    if(h < 0)
        h = ~h;
    h &= (NHASH-1);
    if (type == 'U' || type == 'b' || islocal(type)) {
        for(sp = hash[h]; sp; sp = sp->next)
            if(strcmp(sp->s.name, name) == 0) {
                switch(sp->s.type) {
                case 'T':
                case 'D':
                case 'U':
                    if (type == 'U') {
                        names[id] = &sp->s;
                        return;
                    }
                    break;
                case 't':
                case 'd':
                case 'b':
                    if (type == 'b') {
                        names[id] = &sp->s;
                        return;
                    }
                    break;
                case 'a':
                case 'p':
                    if (islocal(type)) {
                        names[id] = &sp->s;
                        return;
                    }
                    break;
                default:
                    break;
                }
            }
    }
    sp = malloc(sizeof(Symtab));
    sp->s.name = name;
    sp->s.type = type;
    sp->s.sig = sig;
    sp->s.value = islocal(type) ? MAXOFF : 0;
    names[id] = &sp->s;
    sp->next = hash[h];
    hash[h] = sp;
    return;
}
/*e: function objlookup */
/*s: function objtraverse */
/*
 *	traverse the symbol lists
 */
void
objtraverse(void (*fn)(Sym*, void*), void *pointer)
{
    int i;
    Symtab *s;

    for(i = 0; i < NHASH; i++)
        for(s = hash[i]; s; s = s->next)
            (*fn)(&s->s, pointer);
}
/*e: function objtraverse */

/*s: function _offset */
/*
 * update the offset information for a 'a' or 'p' symbol in an intermediate file
 */
void
_offset(int id, vlong off)
{
    Sym *s;

    s = names[id];
    if (s && s->name[0] && islocal(s->type) && s->value > off)
        s->value = off;
}
/*e: function _offset */

/*s: function objupdate */
/*
 * update the type of a global text or data symbol
 */
static void 
objupdate(int id, int type)
{
    Sym *s;

    s = names[id];
    if (s && s->name[0])
        if (s->type == 'U')
            s->type = type;
        else if (s->type == 'b')
            s->type = tolower(type);
}
/*e: function objupdate */

/*s: function nextar */
/*
 * look for the next file in an archive
 */
int
nextar(Biobuf *bp, int offset, char *buf)
{
    struct ar_hdr a;
    int i, r;
    long arsize;

    if (offset&01)
        offset++;
    Bseek(bp, offset, 0);
    r = Bread(bp, &a, SAR_HDR);
    if(r != SAR_HDR)
        return 0;
    if(strncmp(a.fmag, ARFMAG, sizeof(a.fmag)))
        return -1;
    for(i=0; i<sizeof(a.name) && i<SARNAME && a.name[i] != ' '; i++)
        buf[i] = a.name[i];
    buf[i] = 0;
    arsize = strtol(a.size, 0, 0);
    if (arsize&1)
        arsize++;
    return arsize + SAR_HDR;
}
/*e: function nextar */

/*s: function objreset */
static void
objreset(void)
{
    int i;
    Symtab *s, *n;

    for(i = 0; i < NHASH; i++) {
        for(s = hash[i]; s; s = n) {
            n = s->next;
            free(s->s.name);
            free(s);
        }
        hash[i] = 0;
    }
    memset(names, 0, sizeof names);
}
/*e: function objreset */
/*e: linkers/libmach/obj.c */
