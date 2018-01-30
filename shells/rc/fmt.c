/*s: rc/fmt.c */
/*s: includes */
#include "rc.h"
#include "exec.h"
#include "fns.h"
#include "getflags.h"
#include "io.h"
/*e: includes */

/*s: global [[pfmtnest]] */
int pfmtnest = 0;
/*e: global [[pfmtnest]] */

// forward decls
void pdec(io*, int);
void poct(io*, unsigned int);
void pptr(io*, void*);
void pval(io*, word*);
void pquo(io*, char*);
void pwrd(io*, char*);

// in io.c
int fullbuf(io*, int);

/*s: function [[pfmt]] */
void
pfmt(io *f, char *fmt, ...)
{
    va_list ap;
    char err[ERRMAX];

    va_start(ap, fmt);
    pfmtnest++;
    for(;*fmt;fmt++) {
        if(*fmt!='%') {
            pchr(f, *fmt);
            continue;
        }
        if(*++fmt == '\0')		/* "blah%"? */
            break;
        switch(*fmt){
        case 'c':
            pchr(f, va_arg(ap, int));
            break;
        case 'd':
            pdec(f, va_arg(ap, int));
            break;
        case 'o':
            poct(f, va_arg(ap, unsigned int));
            break;
        case 'p':
            pptr(f, va_arg(ap, void*));
            break;
        case 'Q':
            pquo(f, va_arg(ap, char *));
            break;
        case 'q':
            pwrd(f, va_arg(ap, char *));
            break;
        case 's':
            pstr(f, va_arg(ap, char *));
            break;

        case 'r':
            errstr(err, sizeof err); pstr(f, err);
            break;

        // rc specific, TODO LP split here
        case 't':
            pcmd(f, va_arg(ap, struct Tree *));
            break;
        case 'v':
            pval(f, va_arg(ap, struct Word *));
            break;

        default:
            pchr(f, *fmt);
            break;
        }
    }
    va_end(ap);
    if(--pfmtnest==0)
        flush(f);
}
/*e: function [[pfmt]] */

/*s: function [[pchr]] */
void
pchr(io *b, int c)
{
    if(b->bufp==b->ebuf)
        fullbuf(b, c);
    else *b->bufp++=c;
}
/*e: function [[pchr]] */

/*s: function [[pquo]] */
void
pquo(io *f, char *s)
{
    pchr(f, '\'');
    for(;*s;s++)
        if(*s=='\'')
            pfmt(f, "''");
        else pchr(f, *s);
    pchr(f, '\'');
}
/*e: function [[pquo]] */

/*s: function [[pwrd]] */
void
pwrd(io *f, char *s)
{
    char *t;
    for(t = s;*t;t++) if(*t >= 0 && needsrcquote(*t)) break;
    if(t==s || *t)
        pquo(f, s);
    else pstr(f, s);
}
/*e: function [[pwrd]] */

/*s: function [[pptr]] */
void
pptr(io *f, void *v)
{
    int n;
    uintptr p;

    p = (uintptr)v;
    if(sizeof(uintptr) == sizeof(uvlong) && p>>32)
        for(n = 60;n>=32;n-=4) pchr(f, "0123456789ABCDEF"[(p>>n)&0xF]);

    for(n = 28;n>=0;n-=4) pchr(f, "0123456789ABCDEF"[(p>>n)&0xF]);
}
/*e: function [[pptr]] */

/*s: function [[pstr]] */
void
pstr(io *f, char *s)
{
    if(s==0)
        s="(null)";
    while(*s) pchr(f, *s++);
}
/*e: function [[pstr]] */

/*s: function [[pdec]] */
void
pdec(io *f, int n)
{
    if(n<0){
        n=-n;
        if(n>=0){
            pchr(f, '-');
            pdec(f, n);
            return;
        }
        /* n is two's complement minimum integer */
        n = 1-n;
        pchr(f, '-');
        pdec(f, n/10);
        pchr(f, n%10+'1');
        return;
    }
    if(n>9)
        pdec(f, n/10);
    pchr(f, n%10+'0');
}
/*e: function [[pdec]] */

/*s: function [[poct]] */
void
poct(io *f, unsigned int n)
{
    if(n>7)
        poct(f, n>>3);
    pchr(f, (n&7)+'0');
}
/*e: function [[poct]] */

/*s: function [[pval]] */
void
pval(io *f, word *a)
{
    if(a){
        while(a->next && a->next->word){
            pwrd(f, (char *)a->word);
            pchr(f, ' ');
            a = a->next;
        }
        pwrd(f, (char *)a->word);
    }
}
/*e: function [[pval]] */

// was in plan9.c
/*s: function [[_efgfmt]] */
/* avoid loading any floating-point library code */
//@Scheck: weird, probably linker trick
int _efgfmt(Fmt *)
{
    return -1;
}
/*e: function [[_efgfmt]] */

/*e: rc/fmt.c */
