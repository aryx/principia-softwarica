/*s: libregexp/regaux.c */
#include <u.h>
#include <libc.h>
#include "regexp.h"
#include "regcomp.h"


/*s: function [[_renewmatch]] */
/*
 *  save a new match in mp
 */
extern void
_renewmatch(Resub *mp, int ms, Resublist *sp)
{
    int i;

    if(mp==0 || ms<=0)
        return;
    if(mp[0].s.sp==0 || sp->m[0].s.sp<mp[0].s.sp ||
       (sp->m[0].s.sp==mp[0].s.sp && sp->m[0].e.ep>mp[0].e.ep)){
        for(i=0; i<ms && i<NSUBEXP; i++)
            mp[i] = sp->m[i];
        for(; i<ms; i++)
            mp[i].s.sp = mp[i].e.ep = 0;
    }
}
/*e: function [[_renewmatch]] */

/*s: function [[_renewthread]] */
/*
 * Note optimization in _renewthread:
 * 	*lp must be pending when _renewthread called; if *l has been looked
 *		at already, the optimization is a bug.
 */
extern Relist*
_renewthread(Relist *lp,	/* _relist to add to */
    Reinst *ip,		/* instruction to add */
    int ms,
    Resublist *sep)		/* pointers to subexpressions */
{
    Relist *p;

    for(p=lp; p->inst; p++){
        if(p->inst == ip){
            if(sep->m[0].s.sp < p->se.m[0].s.sp){
                if(ms > 1)
                    p->se = *sep;
                else
                    p->se.m[0] = sep->m[0];
            }
            return 0;
        }
    }
    p->inst = ip;
    if(ms > 1)
        p->se = *sep;
    else
        p->se.m[0] = sep->m[0];
    (++p)->inst = 0;
    return p;
}
/*e: function [[_renewthread]] */

/*s: function [[_renewemptythread]] */
/*
 * same as renewthread, but called with
 * initial empty start pointer.
 */
extern Relist*
_renewemptythread(Relist *lp,	/* _relist to add to */
    Reinst *ip,		/* instruction to add */
    int ms,
    char *sp)		/* pointers to subexpressions */
{
    Relist *p;

    for(p=lp; p->inst; p++){
        if(p->inst == ip){
            if(sp < p->se.m[0].s.sp) {
                if(ms > 1)
                    memset(&p->se, 0, sizeof(p->se));
                p->se.m[0].s.sp = sp;
            }
            return 0;
        }
    }
    p->inst = ip;
    if(ms > 1)
        memset(&p->se, 0, sizeof(p->se));
    p->se.m[0].s.sp = sp;
    (++p)->inst = 0;
    return p;
}
/*e: function [[_renewemptythread]] */

/*s: function [[_rrenewemptythread]] */
extern Relist*
_rrenewemptythread(Relist *lp,	/* _relist to add to */
    Reinst *ip,		/* instruction to add */
    int ms,
    Rune *rsp)		/* pointers to subexpressions */
{
    Relist *p;

    for(p=lp; p->inst; p++){
        if(p->inst == ip){
            if(rsp < p->se.m[0].s.rsp) {
                if(ms > 1)
                    memset(&p->se, 0, sizeof(p->se));
                p->se.m[0].s.rsp = rsp;
            }
            return 0;
        }
    }
    p->inst = ip;
    if(ms > 1)
        memset(&p->se, 0, sizeof(p->se));
    p->se.m[0].s.rsp = rsp;
    (++p)->inst = 0;
    return p;
}
/*e: function [[_rrenewemptythread]] */
/*e: libregexp/regaux.c */
