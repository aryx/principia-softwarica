/*s: libmach/5obj.c */
/*
 * 5obj.c - identify and parse an arm object file
 */
#include <u.h>
#include <libc.h>
#include <bio.h>
#include <mach.h>

#include <5.out.h>
#include "obj.h"

typedef struct Addr	Addr;
/*s: struct [[Addr]](arm) */
struct Addr
{
    char	type;
    char	sym;
    char	name;
};
/*e: struct [[Addr]](arm) */
static Addr addr(Biobuf*);
static char type2char(int);
static void skip(Biobuf*, int);

/*s: function [[_is5]](arm) */
int
_is5(char *s)
{
    return  s[0] == ANAME				/* ANAME */
        && s[1] == N_FILE			/* type */
        && s[2] == 1				/* sym */
        && s[3] == '<';				/* name of file */
}
/*e: function [[_is5]](arm) */

/*s: function [[_read5]](arm) */
int
_read5(Biobuf *bp, Prog *p)
{
    int as, n;
    Addr a;

    as = Bgetc(bp);			/* as */
    if(as < 0)
        return 0;
    p->kind = aNone;
    p->sig = 0;
    if(as == ANAME || as == ASIGNAME){
        if(as == ASIGNAME){
            Bread(bp, &p->sig, 4);
            p->sig = leswal(p->sig);
        }
        p->kind = aName;
        p->type = type2char(Bgetc(bp));		/* type */
        p->sym = Bgetc(bp);			/* sym */
        n = 0;
        for(;;) {
            as = Bgetc(bp);
            if(as < 0)
                return 0;
            n++;
            if(as == 0)
                break;
        }
        p->id = malloc(n);
        if(p->id == 0)
            return 0;
        Bseek(bp, -n, 1);
        if(Bread(bp, p->id, n) != n)
            return 0;
        return 1;
    }
    if(as == ATEXT)
        p->kind = aText;
    else if(as == AGLOBL)
        p->kind = aData;
    skip(bp, 6);		/* scond(1), reg(1), lineno(4) */
    a = addr(bp);
    addr(bp);
    if(a.type != D_OREG || a.name != N_INTERN && a.name != N_EXTERN)
        p->kind = aNone;
    p->sym = a.sym;
    return 1;
}
/*e: function [[_read5]](arm) */

/*s: function [[addr]](arm) */
static Addr
addr(Biobuf *bp)
{
    Addr a;
    long off;

    a.type = Bgetc(bp);	/* a.type */
    skip(bp,1);		/* reg */
    a.sym = Bgetc(bp);	/* sym index */
    a.name = Bgetc(bp);	/* sym type */
    switch(a.type){
    default:
    case D_NONE:
    case D_REG:
    case D_FREG:
    case D_PSR:
    case D_FPCR:
        break;
    case D_OREG:
    case D_CONST:
    case D_BRANCH:
    case D_SHIFT:
        off = Bgetc(bp);
        off |= Bgetc(bp) << 8;
        off |= Bgetc(bp) << 16;
        off |= Bgetc(bp) << 24;
        if(off < 0)
            off = -off;
        if(a.sym && (a.name==N_PARAM || a.name==N_LOCAL))
            _offset(a.sym, off);
        break;
    case D_SCONST:
        skip(bp, NSNAME);
        break;
    case D_FCONST:
        skip(bp, 8);
        break;
    }
    return a;
}
/*e: function [[addr]](arm) */

/*s: function [[type2char]](arm) */
static char
type2char(int t)
{
    switch(t){
    case N_EXTERN:		return 'U';
    case N_INTERN:		return 'b';
    case N_LOCAL:		return 'a';
    case N_PARAM:		return 'p';
    default:		return UNKNOWN;
    }
}
/*e: function [[type2char]](arm) */

/*s: function [[skip]](arm) */
static void
skip(Biobuf *bp, int n)
{
    while (n-- > 0)
        Bgetc(bp);
}
/*e: function [[skip]](arm) */
/*e: libmach/5obj.c */
